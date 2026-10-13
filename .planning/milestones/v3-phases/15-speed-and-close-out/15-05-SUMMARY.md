---
phase: 15-speed-and-close-out
plan: 05
subsystem: perf-ci
tags: [ci, cachegrind, valgrind, github-actions, report-mode]
requires: [15-04]
provides:
  - "perf job in ci.yml (report mode, continue-on-error) and workflow_dispatch input control"
  - "perf.roms_render ctest (D-09)"
  - "README perf paragraph"
  - "draft PR #46"
affects: [15-06]
key-files:
  created: [tests/cmake/perf_roms_render.cmake]
  modified: [.github/workflows/ci.yml, tests/CMakeLists.txt, README.md]
decisions:
  - "The perf job copies both binaries to equal-length paths bin/base and bin/head and runs them under env -i so Ir does not depend on path or environment size."
  - "Neither release_policy.cmake nor nightly_workflow_policy.cmake reads ci.yml jobs, so neither changed."
metrics:
  duration: 45m
  completed: 2026-10-13
status: complete
commits: 3
plan_head_before: 7e7250cb397a6bfc5522f2d89f11d45fc4a7dbb1
plan_head_after: 2812859ed7dcc217d2b06c78ea9304a5e72cb06c
actuals:
  tokens: 6000
  tasks: 2
  commits: 3
---

# Phase 15 Plan 05: perf job in report mode Summary

Every pull request now runs a Cachegrind comparison of its merge commit against its first parent on an NROM, an MMC1 and an MMC3 ROM, and writes the table to the job summary and log without affecting `CI required`.

## Commits

- 4cf93a3 ci(15-05): perf job comparing instruction counts in report mode
- 1a88411 test(15-05): perf ROMs draw a non-blank frame 60
- 2812859 docs(15-05): describe the perf job in report mode

## Hosted proof

- PR: https://github.com/szTheory/nesturbator/pull/46 (draft, not merged)
- First run 38340221361 (head 4cf93a3): all jobs success; perf job 2m36s.
- Final run 38341228969 (head 2812859): `perf` success (2m40s, 03:20:20 to 03:23:00), `CI required` success.
- Table from the log (6 matching rows across the log and summary echo):

```
| ROM | mapper | base Ir/frame | head Ir/frame | delta Ir/frame | delta % | status |
| nesteroids | 0 | 23743896 | 23743914 | +18 | +0.00% | ok |
| runner | 1 | 24651876 | 24651894 | +18 | +0.00% | ok |
| M4_P256K_C256K | 4 | 23319715 | 23319733 | +18 | +0.00% | ok |
valgrind-3.22.0
```

The timeout stays at the provisional 15 minutes; 15-08 sets it from the measured cold run (about 2m40s).

## W1 workflow-policy outcome

No change: neither `release_policy.cmake` nor `nightly_workflow_policy.cmake` reads ci.yml jobs or ci-required's needs. After the ci.yml edit: `release.nonbehavioral_policy` and `.selftest` pass (ci preset), `hygiene.nightly_workflow_policy` and `.selftest` pass (hygiene preset).

## Deviations from Plan

None. Task 1's PR was opened after the first commit so its run could start; Task 2 was committed locally while that run finished and pushed afterwards, so no run was cancelled.

## Verification

- `cmake --workflow --preset hygiene` passes (8 of 8) before each push; `perf.roms_render`, 19 `perf.gate_selftest.*`, `cmake.script_policy` and the release policy tests pass in the ci preset.
- No `pull_request_target` in ci.yml; no `perf` in the `ci-required` block; PR is a draft.
- PERF-02 intentionally left unchecked until 15-06 arms the gate.

## Known Stubs

None.

## Self-Check: PASSED

Commits 4cf93a3, 1a88411, 2812859 are on the branch and origin; tests/cmake/perf_roms_render.cmake exists.
