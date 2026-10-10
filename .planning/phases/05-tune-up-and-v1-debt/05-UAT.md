---
status: diagnosed
phase: 05-tune-up-and-v1-debt
source: [05-VERIFICATION.md]
started: 2026-10-10T13:50:59Z
updated: 2026-10-10T14:06:00Z
---

## Current Test

[testing paused — 3 items outstanding]

## Tests

### 1. PR CI run "After" timing recorded in 05-CI-RECORD.md
expected: Slowest leg's job wall time from the phase PR's CI run is recorded and shows no regression against 181 s. Scripted gh query, not a manual test.
result: blocked
blocked_by: other
reason: "PR #25 CI run 38058075941: both Linux build legs failed to compile in under 10 s, so the run has no complete set of leg times to measure. Re-measure after G-05-2 is fixed."

### 2. Six build legs, asan and retroarch-e2e green on the PR run
expected: All six platform legs and asan pass with CTest jobs 4; retroarch-e2e passes with no ***Skipped line; policy.no-skip and retroarch.compare pass.
result: issue
reported: "PR #25 CI run 38058075941 failed: build (ubuntu-24.04) and build (ubuntu-24.04-arm), plus both nofp legs, fail to compile tests/core/test_reset.c:32:19: error: conversion from 'unsigned int' to 'uint8_t' may change value [-Werror=conversion] (gcc-14). macOS, Windows, asan and retroarch-e2e pass."
severity: blocker

### 3. First nightly suite-flake run passes inside its timeout
expected: ctest --preset ci --repeat until-fail:3 --schedule-random passes inside the job timeout (see WR-02 in 05-REVIEW.md).
result: issue
reported: "Nightly run 38058075997 failed: suite-flake and vectors-full stop at the same gcc-14 -Werror=conversion error in tests/core/test_reset.c:32; the suite never ran, so the timeout is unmeasured."
severity: blocker

## Summary

total: 3
passed: 0
issues: 2
pending: 0
skipped: 0
blocked: 1

## Gaps

- gap_id: G-05-2
  truth: "Six build legs, asan and retroarch-e2e green on the PR run"
  status: failed
  reason: "PR #25 CI: gcc-14 rejects tests/core/test_reset.c:32 under -Werror=conversion on both Linux build legs and both nofp legs"
  severity: blocker
  test: 2
  root_cause: "tests/core/test_reset.c:32 assigns `chr_ram ? 0u : 1u` (unsigned int) to `ines_spec.chr_8k` (uint8_t, tests/ines.h:23). gcc-14 -Wconversion flags the narrowing; Apple clang, MSVC and the asan build do not, so the local `ci` gate passed 358/358 and only the hosted Linux gcc legs caught it."
  artifacts:
    - path: "tests/core/test_reset.c"
      issue: "line 32 narrows unsigned int to uint8_t without a cast"
  missing:
    - "Make the assignment conversion-clean for gcc-14 -Wconversion (e.g. an explicit (uint8_t) cast or a uint8_t-typed ternary)"
    - "Confirm no other phase 5 test or src file has the same gcc-only narrowing (every Linux leg stopped at the first error)"
  debug_session: ""

- gap_id: G-05-3
  truth: "First nightly suite-flake run passes inside its timeout"
  status: failed
  reason: "Nightly run 38058075997: suite-flake and vectors-full fail at the same compile error as G-05-2; the suite never ran"
  severity: blocker
  test: 3
  root_cause: "Same as G-05-2: the ci build tree does not compile under gcc-14, so suite-flake stops at `cmake --build --preset ci`. Its 10-minute timeout (WR-02 in 05-REVIEW.md) remains unmeasured."
  artifacts:
    - path: "tests/core/test_reset.c"
      issue: "line 32, the same narrowing as G-05-2"
    - path: ".github/workflows/nightly.yml"
      issue: "suite-flake timeout-minutes: 10 is unmeasured (WR-02)"
  missing:
    - "Fix the compile error (shared with G-05-2)"
    - "Make sure suite-flake's timeout cannot fail a correct run on time alone (WR-02)"
  debug_session: ""
