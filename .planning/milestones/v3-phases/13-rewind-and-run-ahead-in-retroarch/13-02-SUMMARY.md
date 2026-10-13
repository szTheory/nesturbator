---
phase: 13-rewind-and-run-ahead-in-retroarch
plan: 02
subsystem: libretro
tags: [libretro, savestate, info-claims, ci, draft-pr]
requires:
  - phase: 13-01
    provides: serialize entry points, libretro.serialize, claim guard
provides:
  - draft PR #44 (feat: rewind and run-ahead in RetroArch)
  - .info claims savestate = "true", savestate_features = "serialized"
affects: [13-03]
key-files:
  modified:
    - libretro/nesturbator_libretro.info
    - README.md
    - tests/CMakeLists.txt
key-decisions:
  - "Serialized claim committed only after all six build jobs were green on the head that carried the serialize work (ce77da3)."
requirements-completed: [LR-02]
status: complete
commits: 2
plan_head_before: c57a7ac89b29bd9b1abe1699eb194ad561d43363
plan_head_after: 568f766fa0cabcfea78286a4fed50638ecee030b
actuals:
  tokens: 1500
  tasks: 2
  commits: 2
completed: 2026-10-12
---

# Phase 13 Plan 02: Serialized claim after six-platform proof Summary

The phase branch is pushed with draft PR #44, the six-platform build matrix passed the serialize round trip, and the `.info` now claims `savestate = "true"` with `savestate_features = "serialized"` in one revertible commit (568f766).

- PR: https://github.com/szTheory/nesturbator/pull/44 (draft, title `feat: rewind and run-ahead in RetroArch`). Not merged.
- CI run for the proving head ce77da3: https://github.com/szTheory/nesturbator/actions/runs/38280621807 (attempt 4, conclusion success; all six build jobs success, plus asan, nofp, hygiene, hash-equality, retroarch-e2e).
- Task 1 verify (six build jobs success for HEAD) printed OK. Pre-push gates (hygiene preset, clang-format 18 local, GCC 14 -Werror in Docker, clang-format-18 in ubuntu:24.04) passed. origin/main had not moved past the merge base.
- Task 2: guard, `libretro.info_claims`, `.self_test`, `libretro.no_quirks` and `libretro.serialize` pass; `cmake --workflow --preset ci` passes on the claim commit. README states the serialized level, the cores-directory condition and that run-ahead (deterministic) is not claimed.

## Deviations from Plan

**1. [Rule 1 - Bug] libretro.serialize timed out under asan on the hosted runner**
- **Found during:** Task 1 (first CI run on c57a7ac; six build jobs were green, asan was red)
- **Issue:** the asan job hit the 120 s CTest TIMEOUT (about 57 s locally; the runner shares CPU across tests).
- **Fix:** TIMEOUT raised to 600 with a comment. Pushed as ce77da3; asan then passed. The serialized claim waited for this head.
- **Files modified:** tests/CMakeLists.txt
- **Commit:** ce77da3

**Total deviations:** 1 auto-fixed. **Impact:** the test's wall time is unchanged; only the ceiling moved.

## Issues Encountered

The macOS Intel build job (timeout-minutes 7 in ci.yml) was cancelled at the limit on three reruns of the ce77da3 run, with every test passing but the runner about twice as slow as on the first run (ctest 218 s then, longer later); attempt 4 passed in 6 min 38 s. This is runner variance on tests not touched by this phase, so ci.yml was not changed. The margin on that job is thin and may need a larger timeout later.

## Known Stubs

None.

## Self-Check: PASSED

Commits ce77da3 and 568f766 are on the branch and pushed (HEAD equals upstream); `.info` has one `savestate = "true"` and one `savestate_features = "serialized"`; the last commit lists exactly the `.info` and README.
