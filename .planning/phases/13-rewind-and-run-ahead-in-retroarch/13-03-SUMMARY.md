---
phase: 13-rewind-and-run-ahead-in-retroarch
plan: 03
subsystem: libretro-tests
tags: [libretro, run-ahead, tests, controls, deterministic-gate]
requires:
  - phase: 13-01
    provides: serialize entry points, libretro.host --serialize, claim guard
  - phase: 13-02
    provides: serialized claim, draft PR #44
provides:
  - tests/runahead_model.h (input LCG scripts, alignment coverage, uninterrupted oracle, two-instance and one-instance models)
  - libretro.host --runahead (single-instance run-ahead host through the module, repeat mode, four failing controls)
  - CTests libretro.runahead and core.state_runahead (the proving tests for the deterministic claim)
affects: [13-08]
key-files:
  created:
    - tests/runahead_model.h
    - tests/core/test_state_runahead.c
  modified:
    - tests/libretro/libretro_host.c
    - tests/CMakeLists.txt
    - README.md
key-decisions:
  - "Both audio mappings are asserted (real frames equal the uninterrupted run; the last speculative frame equals it shifted by N), RA_SPECULATIVE_ORDER names the order."
  - "Matrix trimmed to fit the time budget: nesteroids gets the full matrix; the other boards get one script and the depths the budget allows."
  - "TIMEOUT 600 for both tests (hosted asan runs several times slower than local)."
requirements-completed: [LR-02]
status: complete
commits: 3
plan_head_before: a543b82efbd0e036e5637f379cbde367404faf6b
plan_head_after: 2640a2d8c5828faf98660caa9199017ba7dc22ed
actuals:
  tokens: 26000
  tasks: 3
  commits: 3
completed: 2026-10-12
---

# Phase 13 Plan 03: Run-ahead host and two-instance test Summary

`libretro.host --runahead` drives the built module as a single-instance run-ahead host (run, save, run ahead N, show last, load) at depths 0 to 3 and matches an uninterrupted run on frames, both audio mappings and state bytes; four broken hosts fail the same comparator, and `core.state_runahead` proves RetroArch's two-instance mode as a library property. The `.info` is not moved to `deterministic` (Plan 08).

## Commits

- c05dcbc: tracer, depth 1 on nesteroids end to end through the module (`tests/runahead_model.h`, `--runahead` mode, CTest `libretro.runahead`).
- 5416668: full matrix, repeat mode, controls, counts.
- 2640a2d: `core.state_runahead` and the README paragraph.

## What was built

- `tests/runahead_model.h`: `ra_script_buttons` (fixed-seed integer LCG; script 1 new value every frame, script 2 holds 1 to 5 frames), `ra_alignment_mask` and `ra_alignment_covered`, `ra_run_uninterrupted` (video, audio and state bytes per frame), `ra_run_two_instance`, `ra_run_one_instance`. Public API and `check.h` only; comparisons are exact FNV-1a hash and raw byte equality, no float.
- `libretro.runahead` per tick: real frame (video hashed and compared, audio kept), `retro_serialize` R(t+1) compared with the oracle state, N speculative frames with `in[t+1..t+N]`, last frame shown, `retro_unserialize`. It asserts the visible frame equals oracle frame t+N, real audio equals oracle audio t, last speculative audio equals oracle audio t+N, bytes serialized before the load differ from R(t+1) (so the load has work), and the final state equals the oracle's. Repeat mode feeds `in[t]` to the speculative frames and compares with `ra_run_two_instance`.
- Controls (skip-load, stale-load, late-input, audio-leak) run nesteroids at N = 2, script 1, through the same `rah_run`; each tripped (694, 748, 533, 180 mismatches). A control that matches fails the test with its name.
- Minimum counts asserted: at least 7 ROMs, tick and state counts equal to what was requested, 4 controls tripped, alignment covered at depths 1 to 3 by the two scripts together. No `SKIP_RETURN_CODE` or `DISABLED` (count 0 before and after). No new pinned hash keys.
- `core.state_runahead`: A real timeline versus B loading A's state and running N with `in[t]` repeated; visible equals the one-instance model at every tick, A equals the uninterrupted run (video, audio, every state byte), and on ticks with `in[t..t+N]` all equal the visible frame equals the uninterrupted frame t+N (636 such ticks). Nine ROMs plus both Phase 12 stress images.

## Measured wall time and trim (budget rule)

| Test | Before trim (ci) | After trim ci | After trim asan (60 ticks) |
|---|---|---|---|
| libretro.runahead | 81 s (all 7 ROMs, both scripts, real and repeat mode, depths 1 to 3) | 22 s | 114 s |
| core.state_runahead | about 95 s estimated (not run untrimmed) | 22.6 s (29 s inside the full workflow, other tests sharing CPUs) | 106 s |

Trim for `libretro.runahead`: full matrix kept on nesteroids only (script 1 at depths 0 to 3 with repeat mode, script 2 at depths 1 to 3); the other six boards and both stress images run one script (alternating) at depths 1 and 3 in real-input mode; repeat mode only on nesteroids (the other boards are covered in repeat mode by `core.state_runahead`). Trim for `core.state_runahead`: nesteroids runs both scripts at depths 1 to 3; every other image runs one script at one depth, rotating with its index. Both are above the "about 10 s" target but under the hard bounds (ci 30 s, asan 150 s); the registrations carry TIMEOUT 600 because hosted asan runs several times slower (libretro.serialize needed the same).

Stress images: they run in both `libretro.runahead` (`--stress`; `state_test.h` compiles in the host once `src` is on its include path) and `core.state_runahead` (D-14).

## Verification

- `cmake --workflow --preset ci` passes (65 s). `libretro.runahead` and `core.state_runahead` pass under the asan preset.
- Local edit (reverted): removing the `retro_unserialize` call in the normal path made `libretro.runahead` fail; setting `savestate_features = "deterministic"` configured successfully with both proving tests registered (reverted, `.info` untouched).
- Pre-push gates: clang-format 18 (local and `ubuntu:24.04`) clean on the three files, GCC 14 `-Werror` syntax pass with the project warning set on both programs, `cmake --workflow --preset hygiene` passes. Pushed to the branch for PR #44.

## Deviations from Plan

**1. [Rule 3 - Blocker] Test matrix trimmed to the plan's time budget**
- **Found during:** Task 2 and Task 3 (untrimmed libretro.runahead took 81 s in ci; the plan's rule caps ci at 30 s and asan at 150 s).
- **Fix:** trimmed as in the table above (plan rule: script 2 first, then ROM list; the ROM list is kept whole, depth 2 and repeat mode are dropped on non-nesteroids boards). The minimum counts were lowered to match the new totals.
- **Files modified:** tests/libretro/libretro_host.c, tests/core/test_state_runahead.c
- **Commits:** 5416668, 2640a2d

**2. [Rule 3 - Blocker] TIMEOUT 600 instead of 300**
- **Issue:** locally the asan run takes 106 to 114 s and hosted runners were already seen to run libretro.serialize about 2x slower than local under asan.
- **Fix:** TIMEOUT 600 with a comment on `libretro.runahead` and `core.state_runahead`.
- **Commits:** 5416668, 2640a2d

**3. [Rule 3 - Blocker] `src` added to the libretro.host include path**
- **Issue:** `state_test.h` includes `internal.h` for the stress images in the host.
- **Commit:** 5416668

**Total deviations:** 3 auto-fixed. **Impact:** the claim's coverage is the full matrix on nesteroids and a reduced matrix on other boards in the module host, plus library-level coverage of every board in `core.state_runahead`.

## Issues Encountered

None open. The tests are slow under asan; their hosted timing is unknown until the next CI run on PR #44.

## Known Stubs

None.

## Self-Check: PASSED

tests/runahead_model.h, tests/core/test_state_runahead.c exist; commits c05dcbc, 5416668 and 2640a2d are ancestors of HEAD and pushed; `grep -c "core.state_runahead" tests/CMakeLists.txt` and `add_test(NAME libretro.runahead` present; README mentions both tests.
