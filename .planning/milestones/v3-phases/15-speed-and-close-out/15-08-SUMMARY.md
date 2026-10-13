---
phase: 15-speed-and-close-out
plan: 08
subsystem: ci-timeouts
tags: [timeouts, hygiene, nightly, close-out]
requires: [15-07]
provides:
  - hygiene.timeout_comments lint with bad and good fixtures
  - measured timeout-minutes with run IDs in nightly.yml, ci.yml and release.yml
  - 15-CLOSEOUT.md Timeouts and Allow file sections
affects: []
key-files:
  created:
    - tests/cmake/timeout_comments.cmake
    - tests/hygiene/fixtures/timeouts_bad.yml
    - tests/hygiene/fixtures/timeouts_good.yml
  modified:
    - tests/hygiene/CMakeLists.txt
    - .github/workflows/nightly.yml
    - .github/workflows/ci.yml
    - .github/workflows/release.yml
    - .planning/phases/15-speed-and-close-out/15-CLOSEOUT.md
decisions:
  - Every timeout-minutes line carries a measured run ID, enforced by hygiene.timeout_comments
metrics:
  tasks: 3
  files: 9
status: complete
commits: 5
plan_head_before: ea29198
plan_head_after: 03cc5df9a89dfb034377c6efaad132172497530f
actuals:
  tokens: 12000
  tasks: 3
  commits: 5
---

# Phase 15 Plan 08: Measured timeouts Summary

Every workflow timeout is now twice a measured run of the final tree with its run ID in a comment, and a hygiene test fails any `timeout-minutes` line without one.

## Results

- Measurement-ceiling commit: f200df9 (suite-flake 60, mmc3-oracle 20, rom-loader-fuzz 15, state-fuzz 15) before the first dispatch.
- Dependabot: no PR open on 2026-10-13, nothing merged; origin/main already an ancestor. Allow file: no entries, `Justified entries: 0`.
- Counted nightly runs (head 4fa0c23, all jobs success under ceiling): 38346572174, 38347773803, 38349314234. Excluded runs: none. report job from schedule run 38347599457.
- New timeouts (minutes): suite-flake 38 (was 30), vectors-full 19 (16), mmc3-oracle 6 (8), rom-loader-fuzz 3, state-fuzz 3, report 1, asan 12 (14), retroarch-e2e 11 (30), perf 9 (15), release-please 1 (30), publish 3 (30; run 38093301725, the latest where publish ran). CI samples from run 38346573845.
- Final head 03cc5df9a89dfb034377c6efaad132172497530f: PR run https://github.com/szTheory/nesturbator/actions/runs/38357342480 green on every job including CI required. PR https://github.com/szTheory/nesturbator/pull/46 stays in draft.

## Deviations from Plan

None in code. A local `ctest --preset ci -j8` rerun timed out seven long state tests from CPU contention (the earlier `cmake --workflow --preset ci` run and the hosted run passed); the change was workflow YAML and hygiene only.

## Known Stubs

None.

## Self-Check: PASSED
