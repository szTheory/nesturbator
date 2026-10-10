---
phase: 05-tune-up-and-v1-debt
reviewed: 2026-10-10T00:00:00Z
depth: standard
files_reviewed: 24
files_reviewed_list:
  - .github/workflows/ci.yml
  - .github/workflows/nightly.yml
  - .github/workflows/release.yml
  - CMakePresets.json
  - README.md
  - include/nesturbator.h
  - libretro/libretro.c
  - src/apu.c
  - src/cpu.c
  - src/instance.c
  - src/internal.h
  - src/ppu.c
  - tests/CMakeLists.txt
  - tests/cmake/nightly_workflow_policy.cmake
  - tests/cmake/release_policy.cmake
  - tests/cmake/skip_policy.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/cmake/vector_result_policy.cmake
  - tests/core/test_reset.c
  - tests/ines.h
  - tests/libretro/libretro_host.c
  - tests/retroarch/CMakeLists.txt
  - tests/retroarch/run_retroarch.cmake
  - tests/retroarch/test.cfg.in
findings:
  critical: 1
  warning: 3
  info: 3
  total: 7
status: issues_found
---

# Phase 5: Code Review Report

**Reviewed:** 2026-10-10
**Depth:** standard
**Files Reviewed:** 24
**Status:** issues_found

## Summary

I reviewed the diff from 751f8b7 to HEAD. It adds `nesturbator_reset` (CPU, PPU and APU halves), the libretro `retro_reset` hook, the no-skip and release-gate CMake policies, the removal of the local RetroArch tests, the nightly flake job and CI action bumps. I did not read README.md or most of `tests/core/test_reset.c` line by line. The CI, policy and RetroArch driver changes are consistent, and I found no leftover references to the removed tests. The main concern is a determinism hole in the reset path: the PPU renders nothing during the 7 reset cycles.

## Critical Issues

### CR-01: First post-reset frame leaves top-left pixels unwritten, so the frame hash depends on host buffer contents

**File:** `src/instance.c:166-172` (with `src/cpu.c:1363-1380`, `src/ppu.c:297-304`, `src/frame.c:55-62`)
**Issue:** `ppu.video_output` is non-NULL only inside `nesturbator_run_frame`. `nesturbator_reset` runs between frames, so `ppu_reset` puts the PPU at scanline 0 dot 0 and `cpu_reset` then executes 7 bus cycles. That is 168 ticks, or 21 PPU dots, run while `video_output == NULL`. `ppu_run_until` only writes a pixel when `video_output != NULL`.

Scanline 0, dots 1 to 21 (x = 0 to 20, y = 0) are therefore never written. The next `run_frame` starts from dot 21. Those 21 pixels keep whatever the host's video buffer held before. That is the previous frame in the libretro and runner cases, but a fresh or zeroed buffer in any other host.

This breaks the project rule that the same inputs give the same frame hashes everywhere. The reset parity test does not catch it, because both sides reuse a buffer holding the same pre-reset frame, and the backdrop is likely identical there.

**Fix:** Make the reset cycles render into the next frame's buffer, or run them inside the frame. One option is to defer the CPU's 7 reset cycles to the start of the next `run_frame`, with a `reset_pending` flag in the instance, so `video_output` is set. Another is to have `ppu_reset` set the PPU position so the 7 cycles finish at dot 0, for example by starting at the previous scanline's end. Add a test that runs the first post-reset frame into a buffer pre-filled with a sentinel and asserts no sentinel pixel remains.

## Warnings

### WR-01: Reset parity tests compare the core with itself, so they do not check reset behaviour against hardware

**File:** `tests/libretro/libretro_host.c:~480-555` (`check_reset_frame_parity`), `tests/core/test_reset.c`
**Issue:** The libretro-versus-API frame equality check passes for any reset implementation, correct or not, because both sides call the same `nesturbator_reset`. Its strongest assertion is the RAM counter incrementing and the backdrop colour changing. That would still pass if the write-ignore window or the vblank flag handling were wrong. The libretro test does not cover the first-frame pixel gap in CR-01, since both buffers hold identical stale data.
**Fix:** Keep the parity test as an adapter wiring check. Put hardware behaviour in `test_reset.c`: assert exact S, I, PC, tick delta (168), the `$2000` write ignored before scanline 261 dot 1 and accepted after, and sentinel-buffer coverage of the first frame (see CR-01).

### WR-02: The `suite-flake` timeout of 10 minutes is below the documented cost of the suite

**File:** `.github/workflows/nightly.yml:~160`
**Issue:** The job configures, builds cold and then runs the `ci` suite three times. The comment in `tests/CMakeLists.txt` says `runner.write_hashes` and the two `vectors.registration_policy` tests alone took 48 s, 107 s and 21 s in one run. Three repeats plus a cold build can exceed 10 minutes. A timeout then reports as a failure and opens the rolling nightly issue as a false flake. The comment admits the value is a placeholder.
**Fix:** Set 30 minutes until a measured cold run exists, then tighten it.

### WR-03: APU reset applies the `$4017` re-apply delay from the phase before the 7 reset cycles

**File:** `src/apu.c:357`
**Issue:** `frame_reset_delay` is chosen from `apu_get_put_phase` at the moment `apu_reset` runs, before `cpu_reset` flips the phase 7 times. The comment ties the 2-or-1 value to the 7 reset cycles plus the 10-clock delay. On either phase the delay expires within the first 2 reset cycles, so the frame counter restarts 5 or more cycles before the first instruction. NESdev says this restart happens about 10 clocks before the first code runs, so the frame-counter phase at the first instruction is off by several CPU cycles. A game that syncs to the APU frame IRQ right after reset would see different timing. It is a deterministic discrepancy rather than a crash, and it is flagged only because the header makes an accuracy claim.
**Fix:** Either set the delay so the counter restarts in the last cycles of the reset sequence (about 7 + 10/2 cycles after the call), or state in the header and the hardware comment that the phase is approximated.

## Info

### IN-01: `cpu_reset` replaces the two dummy PC reads with stack reads

**File:** `src/cpu.c:1366-1372`
**Issue:** The comment says five reads touch the stack page. The first three read the same address (`$0100 | S`) and the S decrements follow. The hardware sequence is two reads at PC, then three stack-page reads at S, S-1 and S-2. There is no internal side effect today, but the read addresses differ from the hardware trace if a bus-trace test is ever added.
**Fix:** Two reads at `nes->cpu.pc`, then three stack reads with decrements, or say in the comment that the read addresses are approximate.

### IN-02: `ines_build` silently lets PRG code overwrite the vectors

**File:** `tests/ines.h:~52-72`
**Issue:** The guard `prg_code_len > prg_size` allows code that reaches the last 6 bytes, which the vector writes then overwrite. A test author gets no error. Mapper values above 0xfff are also silently truncated.
**Fix:** Reject `prg_code_len > prg_size - 6u` and `mapper > 0xfffu`.

### IN-03: `policy.no-skip` is not applied to the `vectors-full` preset

**File:** `tests/CMakeLists.txt:340-355`
**Issue:** The policy inspects the build directory of the preset it runs in, normally `ci`. The nightly `vectors-full` inventory has its own result policy, and the only skip entry there is a synthetic self-test case. A skip-capable test added only under `NESTURBATOR_VECTORS_FULL` would escape the no-skip rule.
**Fix:** Run `skip_policy.cmake` against the `vectors-full` build directory in the nightly job, or from `vector_registration_policy.cmake`, which already loads that inventory.

---

_Reviewed: 2026-10-10_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_

## Re-review after gap plan 05-08 (2026-10-10)

Incremental, standard depth, scope `tests/core/test_reset.c` and `.github/workflows/nightly.yml` (commits 9ed3041 and 1a8fc55 since 6e7cebc). No new findings.

- `tests/core/test_reset.c:32` is now `spec.chr_8k = (uint8_t)(chr_ram ? 0u : 1u);`. `chr_8k` is `uint8_t` (`tests/ines.h:23`) and both values are 0 or 1, so the cast clears the gcc-14 `-Wconversion` error without changing behaviour. The file's other narrowings were already cast.
- `.github/workflows/nightly.yml:161` raises the `suite-flake` timeout from 10 to 30 minutes and keeps the comment that it is re-set from the first measured cold run. No policy test asserts the value. **WR-02 is resolved.**
- CR-01, WR-01, WR-03, IN-01, IN-02 and IN-03 are unchanged by 05-08 and stay open.
