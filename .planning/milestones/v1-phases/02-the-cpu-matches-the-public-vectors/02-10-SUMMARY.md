---
phase: 02-the-cpu-matches-the-public-vectors
plan: 10
subsystem: ci
tags: [cmake, ctest, github-actions, nightly, cpu-vectors]
requires:
  - phase: 02-the-cpu-matches-the-public-vectors
    provides: "Pinned full-vector CTest preset, rolling nightly issue reporter, and exact registration checker"
provides:
  - "Hygiene policy gate for the nightly vector workflow's cold-fetch and trust rules"
  - "Main-push full-vector runs with archived registration inventory and JUnit results"
  - "Commit SHA and run URL in rolling nightly issue reports"
affects: [phase-02-verification, ci, nightly]
actuals:
  tokens: 5738.5
  tasks: 2
  commits: 2
tech-stack:
  added: []
  patterns:
    - "CMake hygiene policy checks use the live classifier against both workflow contents and targeted synthetic mutations"
    - "Full-tier registration policy accepts saved CTest json-v1 inventories without running fetch fixtures"
key-files:
  created:
    - tests/cmake/nightly_workflow_policy.cmake
  modified:
    - .github/workflows/nightly.yml
    - CMakePresets.json
    - README.md
    - tests/hygiene/CMakeLists.txt
    - tests/cmake/vector_registration_policy.cmake
decisions:
  - "Keep D-21's cold fetch behavior and add no cache; assert cache, skip, permissions and privileged-trigger rules in hygiene."
  - "Use the same Plan 09 full-tier registration checker against a saved inventory before invoking the existing workflow preset."
  - "Name the evidence artifact with the GitHub run ID and record event, head SHA, run URL and artifact name in the job summary."
requirements-completed: [CPU-02]
coverage:
  - id: D1
    description: "Hygiene rejects nightly cache, skip, permission and privileged-trigger policy regressions."
    requirement: CPU-02
    verification:
      - kind: integration
        ref: "cmake --workflow --preset hygiene (8/8; policy self-test covers cache, skip, write-permission and privileged-trigger mutations)"
        status: pass
      - kind: other
        ref: "actionlint .github/workflows/nightly.yml"
        status: pass
    human_judgment: false
  - id: D2
    description: "Main pushes and scheduled runs execute the full-vector lane and retain the exact inventory and JUnit result with run identity."
    requirement: CPU-02
    verification:
      - kind: integration
        ref: "vector_registration_policy.cmake FULL_ONLY against saved ctest --show-only=json-v1 inventory (258 tests)"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci (327/327; elevated rerun includes RetroArch)"
        status: pass
    human_judgment: false
duration: 15min
completed: 2026-10-06
status: complete
plan_head_before: 9c49da5ba3a329fe263422301b0f167692e28c08
plan_head_after: 70e73a82a952ff511691282f421b7d281929c21a
commits: 2
---

# Phase 2 Plan 10: Observe full vectors on every main commit Summary

**The nightly workflow now enforces its cold-fetch and least-privilege rules, runs full vectors on each main push, and archives the exact 258-test inventory and completed JUnit result by run ID.**

## Performance

- **Duration:** about 15 minutes
- **Started:** 2026-10-06T16:38:08Z
- **Completed:** 2026-10-06T16:52:42Z
- **Tasks:** 2
- **Files modified:** 6

## Accomplishments

- Added a hygiene policy check for empty root permissions, read-only full-run permissions, disabled checkout credentials, issue-write permission confined to the reporter, and the pinned full-vector workflow command. Cache, skip, permission, failure-suppression, secret, writeback and privileged-trigger violations fail with file/rule diagnostics; the self-test mutates cache, skip, permission and privileged-trigger rules.
- Added an unfiltered push trigger for `main`. Before the full run, the workflow configures `vectors-full`, saves its CTest json-v1 inventory, and checks all 258 tests through Plan 09's exact registration checker. The actual workflow preset runs once and emits JUnit to `build/vectors-full/vectors-full.junit.xml`.
- The workflow uploads the inventory and JUnit result together as `vectors-full-evidence-<run ID>`, records the event, commit SHA, run URL and artifact name in the run summary, and reports main-push and scheduled failures through the existing rolling issue.
- Updated the README CI section to describe the main-push run, evidence artifact and reporter behavior.

## Task Commits

1. **Task 1: Gate the workflow's fetch and trust contract** - `7aea01b` (`test`)
2. **Task 2: Observe full vectors on each main commit** - `70e73a8` (`ci`)

## Files Created/Modified

- `tests/cmake/nightly_workflow_policy.cmake` - live workflow, preset and script policy check with targeted mutation self-tests.
- `tests/hygiene/CMakeLists.txt` - registers live and self-test policy checks in the hygiene preset.
- `.github/workflows/nightly.yml` - main-push trigger, pre-run inventory check, evidence artifact, run identity and issue reporter conditions.
- `tests/cmake/vector_registration_policy.cmake` - accepts a saved CTest inventory in full-only mode.
- `CMakePresets.json` - writes vectors-full JUnit output to a stable build-tree path.
- `README.md` - documents full-vector main-push evidence and issue reporting.

## Decisions Made

- The nightly remains cache-free under D-21; cache behavior is guarded by the new hygiene policy.
- The exact existing full-tier checker validates the same inventory saved for the artifact, before any fetch or test runs.
- The run ID is part of the artifact name, and the job summary records it with the event, SHA and URL for later queries.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Extended the full-only registration checker to read the saved inventory**
- **Found during:** Task 2
- **Issue:** Plan 09's checker only loaded a live CTest inventory from a build directory, so the workflow could not prove that the archived `registered-tests.json` was the inventory it validated.
- **Fix:** Added `INVENTORY_FILE` support to full-only mode while preserving the existing build-directory path for other callers.
- **Files modified:** `tests/cmake/vector_registration_policy.cmake`
- **Verification:** Saved json-v1 inventory passed full-only validation with all 258 required tests and properties.
- **Committed in:** `70e73a8`

**Total deviations:** 1 auto-fixed (Rule 3). **Impact:** required to connect the workflow's saved evidence to the existing registration policy; no scope expansion.

## Verification

- `actionlint .github/workflows/nightly.yml` - passed.
- `cmake --workflow --preset hygiene` - passed 8/8, including the new live policy and mutation self-test.
- `cmake --preset vectors-full`, `ctest --test-dir build/vectors-full --show-only=json-v1`, and full-only registration check against the saved JSON - passed; exact inventory is 258 tests.
- `cmake --workflow --preset ci` in the default sandbox - 326/327; the existing `retroarch.testframe` aborted under the sandbox.
- The same `cmake --workflow --preset ci` outside the sandbox - passed 327/327, including `retroarch.testframe`; packaging completed.
- The hosted Nightly workflow was not triggered because this step did not push or write to GitHub. Its first run will create the run-bound artifact after the workflow commit is pushed.

## Issues Encountered

The existing RetroArch smoke test aborts in the default shell sandbox. Rerunning the unchanged full CI workflow with elevated execution passed all 327 tests; no test skip or workaround was introduced.

## User Setup Required

None.

## Next Phase Readiness

Plan 02-10 is complete locally. After the branch is pushed, the existing path-filtered PR run and subsequent main-push run can exercise the new hosted workflow and publish its run-specific artifact. No issue or other external GitHub object was written by this executor.

---
*Phase: 02-the-cpu-matches-the-public-vectors*
*Completed: 2026-10-06*

## Self-Check: PASSED

- Summary file exists and both task commits are ancestors of the phase branch head.
- The source, hygiene and CI checks listed above completed successfully (the hosted GitHub run remains post-push).
