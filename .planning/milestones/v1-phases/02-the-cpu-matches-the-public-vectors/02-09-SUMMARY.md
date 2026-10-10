---
phase: 02-the-cpu-matches-the-public-vectors
plan: 09
subsystem: testing
tags: [cmake, ctest, hygiene, cpu-vectors]
requires:
  - phase: 02-the-cpu-matches-the-public-vectors
    provides: "Phase 2 vector parser, public header and sample/full CTest tiers"
provides:
  - "Parser policy gate scoped to vector conversion and fetch sources"
  - "Temporary Phase 2 API declaration baseline and tracked vector JSON size limit"
  - "CTest metadata policy for exact sample/full opcode inventories and fixture wiring"
affects: [phase-02-verification, phase-03-public-api]
actuals:
  tokens: 4771
  tasks: 3
  commits: 4
tech-stack:
  added: []
  patterns: ["CTest json-v1 policy checks", "Mutation-sensitive CMake policy self-tests"]
key-files:
  created:
    - tests/cmake/vector_source_policy.cmake
    - tests/cmake/vector_api_policy.cmake
    - tests/cmake/vector_registration_policy.cmake
    - .planning/phases/02-the-cpu-matches-the-public-vectors/deferred-items.md
  modified:
    - tests/CMakeLists.txt
    - scripts/hygiene.sh
    - tests/hygiene/scan_selftest.sh
decisions:
  - "Pin the normalized Phase 1 public declaration stream for Phase 2, with a documented Phase 3 retirement/update path."
  - "Use CTest json-v1 metadata for registration checks; configure the full preset without running its fetch fixture."
requirements-completed: [CPU-01, CPU-02]
coverage:
  - id: D1
    description: "CI rejects parser policy violations and API declaration drift, with mutation-sensitive checks."
    requirement: CPU-01
    verification:
      - kind: unit
        ref: "ctest --preset ci -R '^vectors.source_policy'"
        status: pass
      - kind: unit
        ref: "ctest --preset ci -R '^vectors.api_policy'"
        status: pass
    human_judgment: false
  - id: D2
    description: "Hygiene rejects tracked vector JSON totals above 64 KiB in staged, tree and history modes."
    verification:
      - kind: integration
        ref: "cmake --workflow --preset hygiene (6/6 tests)"
        status: pass
    human_judgment: false
  - id: D3
    description: "CTest inventories enforce 256 active opcode tests in both tiers with full-tier fixtures and labels."
    requirement: CPU-02
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^vectors.registration_policy' (live inventories and mutations)"
        status: pass
    human_judgment: false
duration: 15 min
completed: 2026-10-06
status: complete
plan_head_before: c8ab31b212e4eadb6f91fb55e4a117e1097419d2
plan_head_after: 08c1df4596068f18a26bffe09aca2cdb4e88a255
commits: 4
---

# Phase 2 Plan 9: Enforce vector policy gates Summary

**Phase 2 parser, API, vector-size and exact opcode registration policies now fail through CI and hygiene with mutation checks.**

## Performance

- **Duration:** about 15 minutes
- **Started:** 2026-10-06T14:37:00Z
- **Completed:** 2026-10-06T14:52:00Z
- **Tasks:** 3
- **Files modified:** 6 plan source/test files

## Accomplishments

- Added a scoped, dependency-free parser policy gate for the owned vector converter and conversion/fetch scripts. Its self-test exercises the same classifier used by the live scan.
- Pinned the normalized Phase 1 public declarations for this temporary Phase 2 API boundary and added an added-declaration mutation check.
- Extended hygiene to limit tracked vector JSON to 64 KiB across staged, tree and history modes, with an oversized synthetic case in its self-test.
- Added CTest json-v1 inventory checks for all 256 sample and full opcode tests, required full-tier labels/fixtures, and no disabled or skipped opcode cases. The CI check configures `vectors-full` but never runs its fetch fixture.

## Task Commits

1. **Task 1: Run one parser policy violation through the CI lane** - `f008fc5` (`test`)
2. **Task 2: Guard the Phase 2 API and tracked JSON budget** - `d2878eb` (`test`)
3. **Task 3: Enforce exactly 256 active vector tests in each tier** - `e933c10` (`test`)
4. **Correctness fix: share the parser classifier with its mutation self-test** - `08c1df4` (`fix`)

## Files Created/Modified

- `tests/cmake/vector_source_policy.cmake` - scoped parser and dependency policy plus mutation self-test.
- `tests/cmake/vector_api_policy.cmake` - normalized Phase 1 public declaration baseline and mutation self-test.
- `tests/cmake/vector_registration_policy.cmake` - exact CTest inventory, fixture, label, disabled and skip checks for both tiers.
- `tests/CMakeLists.txt` - registers all policy checks in ordinary CI.
- `scripts/hygiene.sh` - checks tracked vector JSON size in staged, tree and history scans.
- `tests/hygiene/scan_selftest.sh` - generates an oversized JSON file in an isolated scratch repository and proves all three modes reject it.

## Decisions Made

- The public API guard is explicitly temporary: Phase 3 must update or retire the baseline as part of its API work.
- The registration checker reads CTest json-v1 metadata. Its ordinary CI check configures the full preset only; fetch remains a test-time fixture and is not executed.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing critical verification] Reused the parser classifier in the mutation self-test**
- **Found during:** Final review of Task 1
- **Issue:** The first self-test duplicated the violation regex, so a change to the live classifier could leave the self-test green.
- **Fix:** Extracted a shared classifier and made clean, CMake JSON, Python and common external JSON dependency fixtures pass through it.
- **Files modified:** `tests/cmake/vector_source_policy.cmake`
- **Verification:** `ctest --preset ci -R '^vectors.source_policy' --output-on-failure` passed 2/2.
- **Committed in:** `08c1df4`

**Total deviations:** 1 auto-fixed (Rule 2). **Impact:** strengthened the planned mutation-sensitive gate without changing scope.

## Verification

- `ctest --preset ci -R '^vectors.source_policy' --output-on-failure` - passed 2/2.
- `ctest --preset ci -R '^vectors.api_policy' --output-on-failure` - passed 2/2.
- `ctest --preset ci -R '^vectors.registration_policy' --output-on-failure` - passed 2/2; both live inventories and all missing/extra/disabled/skip mutations were checked.
- `cmake --workflow --preset hygiene` - passed 6/6, including the oversized JSON staged/tree/history self-test.
- `cmake --workflow --preset ci` - 326/327 passed. All 256 CPU vector tests and all policy checks passed; unrelated `retroarch.testframe` aborted with `RetroArch exited with Subprocess aborted` after 5.15 seconds. Recorded in [deferred-items.md](./deferred-items.md).
- `ctest --preset ci -N` showed no `cpu.vectors-full.*` tests, so the ordinary CI run did not execute the vector network fetch.

## Issues Encountered

The full CI workflow's existing RetroArch smoke test aborted on this host. It does not touch the changed policy files and is deferred separately; no emulator or build changes were made to address it.

## User Setup Required

None.

## Next Phase Readiness

The repository now has executable gates for the four policy areas assigned to this plan. Phase 3 must deliberately retire or revise the API baseline if its public declarations change. The unrelated RetroArch abort remains recorded for separate diagnosis.

---
*Phase: 02-the-cpu-matches-the-public-vectors*
*Completed: 2026-10-06*

## Self-Check: PASSED

- All declared created files exist.
- All four task and correctness-fix commits are ancestors of the current branch head.
- Required plan checks completed; the unrelated RetroArch failure is recorded under Verification and in `deferred-items.md`.
