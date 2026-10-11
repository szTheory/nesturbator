---
phase: 05-tune-up-and-v1-debt
plan: 01
subsystem: release-policy
tags: [cmake, release, ci-policy, selftest]
requires: []
provides:
  - "Exact, job-scoped release publish-gate check with D-11 mutation self-test"
affects: [release.yml]
tech-stack:
  added: []
  patterns: ["comment-stripped, ;/[/]-encoded line scan with job scoping", "mutate helper that fails when a mutation changes nothing"]
key-files:
  created: []
  modified: [tests/cmake/release_policy.cmake]
key-decisions:
  - "Gate lines are compared by exact equality to canonical CMake variables; reformatting release.yml requires editing the variables too"
requirements-completed: [TUNE-03]
status: complete
duration: 15 min
completed: 2026-10-10
commits: 2
plan_head_before: 751f8b7
plan_head_after: 970ece57207b83c205cd431c1932a65dc0092be6
actuals:
  tokens: 9000
  tasks: 2
  commits: 2
coverage:
  - deliverable: "Publish gate checked by exact job-scoped line equality"
    verification:
      - kind: test
        ref: "release.nonbehavioral_policy"
        status: pass
    human_judgment: false
  - deliverable: "Self-test rejects 16 gate weakenings and proves each mutation changes the input"
    verification:
      - kind: test
        ref: "release.nonbehavioral_policy.selftest"
        status: pass
    human_judgment: false
---

# Phase 5 Plan 1: Exact release publish gate Summary

The release-policy check now compares the publish and ci jobs' `needs:`/`if:` lines to canonical text within each job's own comment-stripped lines, so an appended `|| always()` (or any other change) fails.

## Accomplishments

- Replaced the `string(FIND)` plus `SUBSTRING ... -1` publish check with `workflow_lines`, `job_lines` and `check_gate` (exact, job-scoped, `;`/`[`/`]`-safe).
- Added D-10 checks: no `continue-on-error`, no step-level `if:`, no anchors/aliases/merge keys in publish, and exactly one `--draft=false` release edit, inside publish.
- Added the `mutate` helper and 16 rejected mutations in the self-test; a no-op mutation stops it with "changed nothing" (verified by breaking one find string).

## Task Commits

1. Task 1 (tracer): 8bf8be4 - exact publish gate against the real workflow
2. Task 2: 970ece5 - D-10 checks and full D-11 mutation list

## Deviations from Plan

None - plan executed exactly as written.

## Verification

`cmake --workflow --preset ci`: 357 of 357 tests passed. Appending ` || always()` to the real publish `if:` made normal mode fail with `publish-gate`; the edit was reverted (workflow diff clean).

## Known Stubs

None.

## Self-Check: PASSED

- tests/cmake/release_policy.cmake exists; commits 8bf8be4 and 970ece5 are on the branch.
