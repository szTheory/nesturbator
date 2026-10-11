---
phase: 09-mmc3
plan: 12
subsystem: ci
tags: [mmc3, nightly, workflow, policy, blargg]
requires: [09-11]
provides:
  - "mmc3-oracle job in nightly.yml, in the vectors-full shape, feeding the rolling report issue"
  - "nightly_workflow_policy.cmake checks every lane on its own job text; two-lane self-test with five mmc3-oracle mutations"
affects: []
key-files:
  created: []
  modified: [.github/workflows/nightly.yml, tests/cmake/nightly_workflow_policy.cmake, README.md]
key-decisions:
  - "Job timeout is 8 minutes: the 300 s fetch allowance plus twice the 69 s cold ci build (no cold run of this lane exists yet), rounded up; the arithmetic is in a comment on the job."
  - "Failed keys strip only the mmc3.oracle. prefix before mmc3_ keys; the fetch test keeps its name mmc3.oracle.fetch."
  - "Issue title is now 'Nightly failed' and the label description 'Nightly lanes'; the policy did not pin the old title."
requirements-completed: [BOARD-03]
status: complete
duration: 20 min
completed: 2026-10-10
commits: 1
plan_head_before: 3d141fe1aa7b1b72dd93d43ff37d081cc02eaf75
plan_head_after: 7303a3a5710ae16506621176fe5e5beef67a96d0
actuals:
  tokens: 14000
  tasks: 2
  commits: 1
---

# Phase 9 Plan 12: Nightly MMC3 oracle lane Summary

The nightly now runs blargg's MMC3 oracle as a second lane with the same least-privilege shape as `vectors-full`, and the workflow policy checks each lane on its own job text.

## Accomplishments

- `mmc3-oracle` job after `vectors-full`: `contents: read` only, pinned checkout and upload SHAs, GCC 14, inventory saved before the network run, `cmake --workflow --preset mmc3-oracle`, Failed keys, Verify run evidence, upload with `if-no-files-found: error`, run identity.
- `report` needs `mmc3-oracle`, its `RESULT` requires that job's success, and the issue body carries "Failed MMC3 oracle keys". `pull_request.paths` gained `tests/mmc3/**`, the two MMC3 scripts plus the inventory script, `src/mapper_mmc3.c`, `src/mapper.h`, `src/ppu.c`, `src/cartridge.c` and `src/bus.c`.
- Policy: lane text runs from the lane's header to the next two-space job header; permissions, checkout pin, inventory-before-workflow order, evidence and upload, and `report.needs` are checked per lane; preset checks (JUnit path, label, `noTestsAction`) cover both presets; the MMC3 fetch and run scripts join the no-skip-77 scan. Whole-file checks (no cache, no 77, `issues: write` once, no secrets, no push) stay.
- Self-test fixture holds both lanes; mutations on `mmc3-oracle` alone (evidence step removed, `contents: write`, `SKIP_RETURN_CODE: 77`, dropped from `report.needs`, workflow before inventory) each fail the policy, and the test fails if a mutation changes nothing.
- README describes the second lane (rule 6).

## Phase gate

| Workflow | Result |
|---|---|
| `ci` | pass, 446 tests, about 57 s wall time cold-ish (incremental build) |
| `asan` | pass, 446 tests |
| `hygiene` | pass, 8 tests |
| `mmc3-oracle` | pass, 13 tests (2.3 s of tests) |

Invariants hold: `tests/accuracy/scoreboard.txt` equals `origin/main`, `NESTURBATOR_BEHAVIOUR_REVISION` is 5, `tests/runner/hashes.txt` has 50 rows.

## Deviations from Plan

- Task 2's mutations were written in Task 1's commit: the two-lane fixture and the mutation helper share one rewrite of the same file, and the task-1 verify runs the self-test. Task 2 therefore produced no separate commit; its work is the gate above.
- The README paragraph was added, since rule 6 requires it; the plan listed no README file.

## Known Stubs

None.

## Residual risk

The job timeout is derived from the ci cold build, not from a measured cold run of this lane. The first nightly run should be used to tighten it.

## Self-Check: PASSED

`.github/workflows/nightly.yml`, `tests/cmake/nightly_workflow_policy.cmake` and `README.md` changed; commit 7303a3a exists; acceptance greps for `^  mmc3-oracle:`, `needs.mmc3-oracle`, `tests/mmc3/**` and the policy lane list pass.
