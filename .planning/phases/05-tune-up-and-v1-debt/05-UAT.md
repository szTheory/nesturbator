---
status: testing
phase: 05-tune-up-and-v1-debt
source: [05-VERIFICATION.md]
started: 2026-10-10T13:50:59Z
updated: 2026-10-10T13:50:59Z
---

## Current Test

number: 1
name: PR CI run "After" timing recorded in 05-CI-RECORD.md
expected: |
  Slowest leg's job wall time from the phase PR's CI run (gh run view --json jobs) is written into
  05-CI-RECORD.md "After" and shows no regression against the 181 s Before figure.
awaiting: user response

## Tests

### 1. PR CI run "After" timing recorded in 05-CI-RECORD.md
expected: Slowest leg's job wall time from the phase PR's CI run is recorded and shows no regression against 181 s. Scripted gh query, not a manual test.
result: [pending]

### 2. Six build legs, asan and retroarch-e2e green on the PR run
expected: All six platform legs and asan pass with CTest jobs 4; retroarch-e2e passes with no ***Skipped line; policy.no-skip and retroarch.compare pass.
result: [pending]

### 3. First nightly suite-flake run passes inside its timeout
expected: ctest --preset ci --repeat until-fail:3 --schedule-random passes inside the job timeout (see WR-02 in 05-REVIEW.md).
result: [pending]

## Summary

total: 3
passed: 0
issues: 0
pending: 3
skipped: 0
blocked: 0

## Gaps
