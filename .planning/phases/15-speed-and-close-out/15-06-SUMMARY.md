---
phase: 15-speed-and-close-out
plan: 06
subsystem: perf-ci
tags: [ci, cachegrind, perf-gate, ci-required, jq]
requires: [15-05]
provides:
  - "15-PERF-PROOF.md: two zero-spread control runs"
  - "perf gate armed: MODE=gate, perf in ci-required needs, event-aware roll-up"
  - "hygiene.ci_required_rollup"
affects: [15-07, 15-08]
key-files:
  created: [.planning/phases/15-speed-and-close-out/15-PERF-PROOF.md, tests/cmake/ci_required_rollup.cmake]
  modified: [.github/workflows/ci.yml, tests/hygiene/CMakeLists.txt, CMakeLists.txt, README.md]
decisions:
  - "The gate is armed: both control runs showed delta 0 on all three ROMs, no UNSTABLE rows."
  - "CI required accepts a skipped perf only when the event is not pull_request."
metrics:
  duration: 50m
  completed: 2026-10-13
status: complete
commits: 2
plan_head_before: af7fc66ee36327c217639ab7b11363ab378f07db
plan_head_after: bed1f64af403cde80dcef5dd536acf9fa7b96743
actuals:
  tokens: 9000
  tasks: 2
  commits: 2
---

# Phase 15 Plan 06: Arm the perf gate Summary

Two hosted control runs showed zero Cachegrind spread on all three ROMs, so the +1% per-ROM perf gate is now part of `CI required`, with a hygiene test that owns the roll-up.

## Commits

- 5bed970 docs(15-06): record two zero-spread perf control runs (proof file only)
- bed1f64 ci(15-06): arm the perf gate after recorded zero spread

## Evidence (D-15)

- Control run 1: 38342366027 (head af7fc66), perf success, delta 0 on nesteroids, runner, M4_P256K_C256K.
- Control run 2: 38343146470 (head af7fc66), perf success, same counts.
- Image ubuntu-24.04 version 20261004.327.1, valgrind-3.22.0, gcc-14 14.2.0.
- Verdict: zero spread. Proof committed separately and before the arming commit.

## Arming (D-16)

- perf job: job-level `continue-on-error` removed, compare step runs `-DMODE=gate`, comment rewritten (disarm by reverting bed1f64; missing base shows `n/a` and does not fail; missing head or UNSTABLE base fails).
- `ci-required`: `perf` added to `needs`; the jq roll-up takes `--arg event` and accepts `perf` skipped only off `pull_request`.
- `hygiene.ci_required_rollup` (`NESTURBATOR_JQ` via `find_program REQUIRED`): checks ci-required needs list every other job, then runs the jq expression extracted from ci.yml on 8 synthetic cases (push/dispatch skip passes; pull_request skip fails; failures fail). Mutation check by hand: dropping `perf` from needs, or removing the event condition, makes it fail.
- README perf paragraph states the gate, the allow file, unstable base and `n/a` rules.

## Hosted proof after arming

PR run 38344323347 on head bed1f64: `perf` success, `CI required` success (PERF-02 criterion 3; the deliberate-slowdown path is `perf.gate_selftest.allow_ok` from Plan 04).

## W1 workflow-policy outcome

`release_policy.cmake` and `nightly_workflow_policy.cmake` unchanged (neither reads ci.yml jobs or ci-required needs). `release.nonbehavioral_policy` and `.selftest` pass (ci preset); `hygiene.nightly_workflow_policy` and `.selftest` pass (hygiene preset).

## Verification

`cmake --workflow --preset hygiene` (9 of 9) and `cmake --workflow --preset ci` pass locally.

## Deviations from Plan

None - plan executed as written.

## Known Stubs

None.

## Self-Check: PASSED

Commits 5bed970 and bed1f64 are on the branch and origin; both new files exist.
