---
phase: 11-tune-up-and-state-readiness
plan: 04
subsystem: ci
tags: [retroarch, mmc3, save, ci, docs]
requires: [11-03]
provides: [SAVE_SIZE, retroarch-e2e MMC3 save round trip]
affects: [tests/retroarch/run_retroarch.cmake, .github/workflows/ci.yml, README.md]
key-files:
  created: []
  modified: [tests/retroarch/run_retroarch.cmake, .github/workflows/ci.yml, README.md]
decisions:
  - "D-14 run_retroarch.cmake takes -DSAVE_SIZE (default 32768, positive decimal or FATAL_ERROR); MMC3 step uses derived m4tkrom.nes, frame 600, 8192 bytes, no sleeps, no committed bytes"
requirements: [TUNE-08]
requirements-completed: [TUNE-08]
status: complete
commits: 2
plan_head_before: ba7a9e38d4ae699840e474ff28c03f29acbf04c1
plan_head_after: 9f2c0d65595991e589df0e00d6f63875a9004c72
actuals:
  tokens: 3000
  tasks: 2
  commits: 2
---

# Phase 11 Plan 04: MMC3 RetroArch save round trip Summary

`run_retroarch.cmake` now takes `-DSAVE_SIZE` and the hosted `retroarch-e2e` job round-trips the derived MMC3 battery ROM `m4tkrom` (8192-byte `.srm`) across two RetroArch sessions, alongside the MMC1 one.

## Done
- Task 1 (081b754): `SAVE_SIZE` validated right after the frame check (default 32768, `^[1-9][0-9]*$` else FATAL_ERROR "needs a positive -DSAVE_SIZE"); `check_one_srm` compares against it; comment made board-neutral; new ci.yml step "Compare the RetroArch MMC3 battery save round trip" (RA_DIR run-save-mmc3, frame 600, size 8192, SAVE_ROM build/ci/tests/holymapperel/derived/m4tkrom.nes) and seven artifact paths. No sleep, no new action.
- Task 2 (9f2c0d6): README describes the MMC3 step, `-DSAVE_SIZE`, and both `.srm` files in the artifact.

## Verification
- Task 1 automated verify: OK (abc and 0 rejected; 8192 and omitted pass through to the version-plist check; ci.yml greps pass). `EQUAL 32768` count 0, no `sleep`, YAML parses.
- Action pins: `tests/cmake/action_pins.cmake -DROOT=$PWD/.github` exit 0 (21 uses checked). Note: the plan's `-DROOT="$PWD"` scans the intentional `tests/hygiene/fixtures/pins_bad.yml` and fails; the check must be pointed at `.github`.
- `cmake --workflow --preset ci` and `asan` pass (asan 483/483, ci log shows 0 failures); `scripts/hygiene.sh` exit 0.
- Local RetroArch round trip not run (no logged-in GUI session assumed). The gate is the hosted `retroarch-e2e` job on the phase pull request: `gh pr checks` must show it passing before merge.

## Deviations from Plan
None in code. Plan advisory confirmed: no `hygiene.action_pins` CTest exists; the script was run directly with `-DROOT=.github`.

## Known Stubs
None.

## Self-Check: PASSED
