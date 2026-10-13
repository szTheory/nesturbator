---
phase: 13-rewind-and-run-ahead-in-retroarch
reviewed: 2026-10-12T00:00:00Z
depth: standard
files_reviewed: 24
files_reviewed_list:
  - .github/workflows/ci.yml
  - .github/workflows/nightly.yml
  - README.md
  - libretro/libretro.c
  - libretro/nesturbator_libretro.info
  - tests/CMakeLists.txt
  - tests/cmake/claim_guard/CMakeLists.txt
  - tests/cmake/fuzz_registration.cmake
  - tests/cmake/info_claims.cmake
  - tests/cmake/libretro_no_quirks.cmake
  - tests/core/test_state_runahead.c
  - tests/fuzz/replay_main.h
  - tests/fuzz/rom_loader.c
  - tests/fuzz/state_loader.c
  - tests/libretro/libretro_host.c
  - tests/retroarch/CMakeLists.txt
  - tests/retroarch/frame_index.c
  - tests/retroarch/log_allowlist.txt
  - tests/retroarch/log_check.cmake
  - tests/retroarch/run_retroarch.cmake
  - tests/retroarch/state.cfg.in
  - tests/retroarch/state_session.sh
  - tests/roms/manifest.txt
  - tests/runahead_model.h
findings:
  critical: 0
  warning: 3
  info: 5
  total: 8
status: issues_found
---

# Phase 13: Code Review Report

**Reviewed:** 2026-10-12
**Depth:** standard
**Files Reviewed:** 24
**Status:** issues_found

## Summary

The shipped adapter change (`libretro/libretro.c`: `retro_serialize_size`, `retro_serialize`, `retro_unserialize`) is a correct thin wrapper over the library. It handles a NULL instance, the test card and the no-content case, and the exact-size rule matches `src/state.c`. I found no correctness or security defect in it, and no personal paths, addresses or names in the diff.

The 22 locally runnable new tests (`fuzz.state*`, `libretro.serialize`, `libretro.info_claims*`, `libretro.no_quirks`, `retroarch.log_check*`, `retroarch.frame_index*`) pass on `build/ci`.

The concerns are about test strength and robustness, not product behaviour:
- The network-command RetroArch session cannot tell some off-by-one outcomes from the correct one (WR-01).
- Its command delivery is inconsistently hardened against missed commands (WR-02).
- Two test drivers pick their special-case ROM by substring of the whole path (WR-03).

No structural (fallow) findings were supplied.

## Warnings

### WR-01: state-net frame-chain checks cannot detect a dropped or doubled FRAMEADVANCE, or a one-frame error in the run-ahead lead

**File:** `tests/retroarch/run_retroarch.cmake:571-584, 593-598, 624-628, 633-648`
**Issue:** Each screenshot is identified as a *set* of runner frame indices, because nesteroids repeats frames in adjacent pairs (`frame_index.c` header, `retroarch.frame_index.dups`). `ra_chain` accepts any `x` in the next set with `x-1` in the previous set. With pair sets, a command that did nothing, or advanced twice, still satisfies the chain.
- Example: prev = {a, a+1}. A dropped advance gives next = {a, a+1}, and x = a+1 passes because a is in prev.
- The final chains only prove that some path of consecutive frames exists. They never check the absolute offset from M0.
- The run-ahead assertion has the same slack. `want = m + 1 + k_eff` is tested for every m in `set_rew-0`, which holds two frames. With k_eff = 1, an observed lead of 0 produces a set that can still contain `m+1+1` for the other element of the pair.
- The lane-level checks (`state-base`, `state-ahead`) compare exact images and do discriminate k_eff. The net session does not, even though its comments and README present it as checking "each step must follow from the last".
- An exact anchor is already on disk. The final RASTATE holds the core's `frame_number` at offset 48, written when SAVE_STATE was taken at rew-0. `ra_state_container(state-net ... "")` is called with an empty frame, so the anchor is discarded.

**Fix:** Pass the expected frame into the container check, or read it back. For example, read `frame_number` from `${net_states}` (offset 48, as `ra_state_container` already does for state-save). Then require that `set_rew-0` contains it, and that `set_ahead-1` contains exactly `frame_number + 1 + k_eff` (apply the measured RetroArch -1 where it applies). Compare absolute values instead of membership in a pair set:
```cmake
ra_state_container(state-net "${net_states}" "")   # keep the layout check
file(READ "${net_states}" fh HEX OFFSET 48 LIMIT 8)
ra_le_value(saved_frame "${fh}")
if(NOT saved_frame IN_LIST set_rew-0)
  message(FATAL_ERROR "[state-net] saved frame ${saved_frame} is not in rew-0 {${set_rew-0}}")
endif()
math(EXPR want "${saved_frame} + 1 + ${k_eff}")
if(NOT want IN_LIST set_ahead-1) ... endif()
```
For the five advances, also assert an absolute lower bound: `adv-5` must contain no frame below `m0_min + 5`.

### WR-02: State session sends most commands once although the script itself documents that a command can be missed

**File:** `tests/retroarch/state_session.sh:104, 111, 123, 131, 142, 144, 182, 184, 61`
**Issue:** The header says commands are per-loop flags. Step 6 resends `SAVE_STATE` every second for that reason (lines 163-166). Every other step is sent once: the first `SAVE_STATE`, `PAUSE_TOGGLE` (three times), `LOAD_STATE_SLOT`, `FRAMEADVANCE`, and `SCREENSHOT` inside `shot()`. The effects on this required macOS check:
- A missed `SCREENSHOT` fails the session after 5 s.
- A missed first `SAVE_STATE` fails after 5 s.
- A missed `PAUSE_TOGGLE` produces a confusing "screen moved while paused" failure in cmake.

`FRAMEADVANCE` is not idempotent and cannot be resent safely. The two idempotent commands, `SCREENSHOT` and the first `SAVE_STATE`, can reuse the step-6 resend pattern.
**Fix:** In `shot()`, resend `SCREENSHOT` on the same 1 s cadence:
```sh
while [ "$(png_count)" -le "$before" ]; do
    i=$((i + 1))
    [ "$i" -le 50 ] || fail "no screenshot for $1 within 5 s"
    [ $((i % 10)) -ne 1 ] || [ "$i" -eq 1 ] || cmd SCREENSHOT
    sleep 0.1
done
```
Apply the same loop to step 2's `SAVE_STATE`. The existing `rm -f shots/*.png` already collapses duplicate screenshots.

### WR-03: Special-case ROM selection matches a substring of the full path

**File:** `tests/libretro/libretro_host.c:2395, 2029`; `tests/core/test_state_runahead.c:130`
**Issue:** `strstr(argv[i], "nesteroids")` and `strstr(path, "M1_P128K_C128K_S8K")` run on the full path CMake passes (`${PROJECT_SOURCE_DIR}/tests/roms/...`). A checkout under a directory whose name contains `nesteroids` (a fork or worktree, for example) makes every ROM take the "full matrix" branch. The count guards then fail: `control_roms == 1` and `full_images == 1` in `libretro.runahead` and `core.state_runahead`. The failure is environment dependent and unrelated to the code under test.
**Fix:** Match the basename:
```c
static int path_has_name(const char *path, const char *name)
{
    const char *b = strrchr(path, '/');
    const char *w = strrchr(path, '\\');
    if (w != NULL && (b == NULL || w > b)) b = w;
    return strstr(b != NULL ? b + 1 : path, name) != NULL;
}
```

## Info

### IN-01: Duplicate hash helpers across the new test code

**File:** `tests/libretro/libretro_host.c:1377-1407` (`SER_FNV_BASIS`, `ser_hash_words`), `tests/libretro/libretro_host.c:710` (`fnv_add`), `tests/retroarch/frame_index.c:60`
**Issue:** `ser_hash_words` and `SER_FNV_BASIS` repeat `ra_hash_words` and `RA_FNV_BASIS` from `tests/runahead_model.h`. `fnv_add` repeats `ra_fnv_bytes`. The two copies can drift apart.
**Fix:** Use the `runahead_model.h` helpers in `--serialize` mode as well.

### IN-02: `replay_main.h` depends on its includer for standard headers

**File:** `tests/fuzz/replay_main.h:14-49`
**Issue:** The header uses `FILE`, `fopen`, `malloc` and `uint8_t` but includes nothing. It compiles only because `rom_loader.c` and `state_loader.c` include `<stdio.h>`, `<stdlib.h>` and `<stdint.h>` first.
**Fix:** Add `#include <stdint.h>`, `<stdio.h>` and `<stdlib.h>` inside the `NESTURBATOR_FUZZ_REPLAY` branch.

### IN-03: `state-fuzz` timeout is a placeholder

**File:** `.github/workflows/nightly.yml:238`
**Issue:** `timeout-minutes: 3 # not yet measured`. The job does a cold ASan/UBSan build, then a 60 s fuzz run with up to 4 MB synthetic carts built per input. This departs from the repo's own "twice the measured run" rule (ENGINEERING §5) and risks a spurious cancellation that the rolling issue reports as a fuzz failure.
**Fix:** Measure the first scheduled run and set twice that.

### IN-04: `log_check` does not report allowlist entries that matched nothing

**File:** `tests/retroarch/log_check.cmake:138-146`
**Issue:** Used entries are printed, but an entry that never matches (a stale measurement) is neither flagged nor listed. The allowlist can only grow silently.
**Fix:** List the unused entries in the final STATUS message, or fail on unused entries in the lanes where they were measured.

### IN-05: `.info` description is stale

**File:** `libretro/nesturbator_libretro.info:24`
**Issue:** The description lists "NROM, MMC1, UxROM, CNROM and AxROM". The new fuzz carts and the run-ahead tests use mapper 4 (MMC3). The string is user-visible in RetroArch's core information. Also, `info_claims.cmake`'s parser builds a CMake list with `file(STRINGS)`, so a future description containing `;` would be split and rejected as "cannot parse line".
**Fix:** Add MMC3 to the description, and read the file with `file(READ)` plus per-line splitting, as `log_check.cmake` does.

---

_Reviewed: 2026-10-12_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
