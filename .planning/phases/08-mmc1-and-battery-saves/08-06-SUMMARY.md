---
phase: 08-mmc1-and-battery-saves
plan: 06
subsystem: runner
tags: [runner, battery-save, save-dir, atomic-write, exit-status-4]
requires:
  - phase: 08-mmc1-and-battery-saves
    provides: plan 01 (nesturbator_get_memory, nesturbator_save_generation), plan 05 (MMC1B board the fixture runs on)
provides:
  - runner --save-dir and --save-interval, exit status 4 for a wrong-sized .sav
  - runner/save.c (stem rule, read-only load, temp-sync-rename flush)
  - runner.save_rom fixture writer and runner.save.* CTests
affects: [phase-08-plan-07, phase-08-plan-08]
tech-stack:
  added: []
  patterns: [generation-then-memcmp change gate with a shadow copy, per-case scratch directory under ctest -j, kill test through execute_process TIMEOUT]
key-files:
  created:
    - runner/save.c
    - runner/save.h
    - tests/runner/save_rom.c
    - tests/runner/test_save.c
    - tests/cmake/runner_save.cmake
  modified:
    - runner/main.c
    - runner/CMakeLists.txt
    - tests/CMakeLists.txt
    - README.md
key-decisions:
  - "The exit flush sits after the frame loop, so it runs after a normal end, a frame failure and a JAM; the interval flush sits at the end of each loop body, after hash and dump steps."
  - "A battery-less cartridge (NULL span) does no save I/O at all, even with --save-dir."
  - "Save path longer than the 4096-byte buffer is an exit 1 error, not a truncation."
requirements-completed: [SAVE-02, SAVE-03, SAVE-04]
status: complete
commits: 3
plan_head_before: 53db1f08960e7cdb68317b659309f1ab25bca3c0
plan_head_after: 6ddf68d5ec008762ba06b6e1d2818a521981af3d
actuals:
  tokens: 9500
  tasks: 3
  commits: 3
duration: 30 min
completed: 2026-10-10
---

# Phase 8 Plan 06: Runner battery saves Summary

`nesturbator-run --save-dir DIR` now loads `DIR/<stem>.sav` into the cartridge's battery RAM before frame 1 and writes a changed span back through a synced temp file and a rename. A wrong-sized `.sav` is refused with status 4 and left byte-identical, and `--save-interval N` flushes during the run.

## Accomplishments

- **Save module (D-18, D-19).** `runner/save.c` has the stem rule (last component split at `/` or `\`, minus the last extension unless the dot is first), a read-only load that distinguishes missing (fresh), wrong size (mismatch) and I/O error, and a flush using `.tmp`, fwrite, fflush, fsync (`_commit` on Windows), checked fclose and rename (`MoveFileExA` with replace and write-through on Windows). A failed rename names the kept temp file.
- **Runner flags.** `--save-dir` and `--save-interval` with the D-18 usage errors (exit 2). The change gate is generation `!=` and then `memcmp` against a shadow copy, shared by interval and exit flushes. Status 4 returns before the frame loop and before the shadow is allocated.
- **Tests.** `runner.save.fixture` plus roundtrip, fresh, mismatch, mismatch_empty, unchanged (bytes and microsecond mtime), nodir, batteryless, unwritable, interval (killed by a 4 s timeout), interval_long and jam; `runner.save_path` unit cases; six `runner.usage.save_*` cases. The roundtrip proves the load happens before frame 1 through the seeded `$42` arriving at offset 4.
- **Docs.** Runner top comment, `usage()` and README describe both flags, the name rule, the atomic write, the shared-file caveat, statuses 0, 1, 2 and 4 (3 and 77 reserved) and mappers 0, 1, 2, 3 and 7. No stale "0, 2, 3 and 7" text remains.

## Task Commits

1. Task 1 (tracer): `e70b238` feat(08-06): runner --save-dir round trip with atomic flush
2. Task 2: `c2a712d` test(08-06): save refusals, no-touch guarantees, usage errors and stem rule
3. Task 3: `6ddf68d` feat(08-06): runner --save-interval, JAM exit flush and save docs

## Deviations from Plan

None - plan executed as written. Task 2's tests passed on first run against Task 1's implementation, so no defect fix was needed there.

## Verification

`cmake --workflow --preset ci` (398 tests), `hygiene` (8) and `asan` (398) all pass. The tracer was re-verified end to end before expansion.

## Known Stubs

None.

## Threat Flags

None. T-08-06 (path from ROM name), T-08-07 (wrong-sized .sav) and T-08-08 (lost save on crash) are mitigated as planned and covered by `runner.save_path`, `runner.save.mismatch*` and `runner.save.interval`.

## Self-Check: PASSED

Files runner/save.c, runner/save.h, tests/runner/save_rom.c, tests/runner/test_save.c and tests/cmake/runner_save.cmake exist; commits e70b238, c2a712d and 6ddf68d are on the branch.
