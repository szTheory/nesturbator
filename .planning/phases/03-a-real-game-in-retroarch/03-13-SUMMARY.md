---
phase: 03-a-real-game-in-retroarch
plan: 13
subsystem: conformance-testing
tags: [AccuracyCoin, scoreboard, CMake, GitHub Actions]
requires:
  - phase: 03-07
    provides: RAM-derived AccuracyCoin results and the current candidate scoreboard
  - phase: 03-12
    provides: integrated PPU behavior needed by AccuracyCoin page 17
provides:
  - Protected-main scoreboard baseline handoff in all six CI build lanes
  - Fail-closed CI baseline selection and no-lost-PASS regression checks
  - Detached local baseline snapshot and documented verification contract
affects: [GAME-04, CI, AccuracyCoin]
actuals:
  tokens: 3498
  tasks: 2
  commits: 3
  plan_head_before: e93551967af364b03acd3fa1f8451ae156fcc0c2
  plan_head_after: 5ae1357598a19ccc2573e4ebf04f825905aa105f
tech-stack:
  added: []
  patterns:
    - Fetch the protected branch in a shell-independent CMake script and export its exact baseline bytes through GITHUB_ENV.
    - Require CI to use its handed-off baseline; use a committed snapshot for detached local runs.
key-files:
  created:
    - tests/cmake/prepare_scoreboard_baseline.cmake
    - tests/cmake/verify_scoreboard_regression.cmake
    - tests/accuracy/scoreboard-main.txt
  modified:
    - .github/workflows/ci.yml
    - tests/accuracy/test_scoreboard.c
    - tests/CMakeLists.txt
    - README.md
key-decisions:
  - "Use the fetched protected-main scoreboard as CI's sole production baseline; an absent path after successful lookup produces an empty baseline."
  - "Use the committed empty scoreboard-main.txt snapshot only for detached local runs while main has no scoreboard file."
patterns-established:
  - "CI baseline handoff: validate environment paths, fetch without tags, distinguish lookup absence from errors, then append the baseline path to GITHUB_ENV."
  - "Scoreboard regression: validate sorted, exact-format rows and require each previous PASS name to remain passing."
requirements-completed: [GAME-04]
coverage:
  - id: D1
    description: "CI obtains a trusted protected-main scoreboard baseline and cannot lose an existing passing row."
    requirement: GAME-04
    verification:
      - kind: unit
        ref: "ctest --test-dir build/ci -R '^accuracy[.]scoreboard([.]|$)' --output-on-failure"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci"
        status: fail
    human_judgment: false
duration: 12min
completed: 2026-10-08
status: complete
commits: 3
plan_head_before: e93551967af364b03acd3fa1f8451ae156fcc0c2
plan_head_after: 5ae1357598a19ccc2573e4ebf04f825905aa105f
---

# Phase 03 Plan 13: Protected-main Scoreboard Baseline Summary

**The six-lane CI matrix now checks AccuracyCoin results against exact protected-main scoreboard bytes, with confirmed first-merge absence treated as an empty baseline.**

## Performance

- **Duration:** 12 min
- **Started:** 2026-10-08T19:36:51Z
- **Completed:** 2026-10-08T19:48:21Z
- **Tasks:** 2
- **Files modified:** 7

## Accomplishments

- Added a CMake preparation script before the matrix workflow preset. It fetches `refs/heads/main` without tags, fails on fetch or tree-query errors, preserves exact file bytes when present, emits an empty file only after a successful absent-path lookup, and exports the path through `GITHUB_ENV`.
- Updated the scoreboard test so CI requires `NESTURBATOR_SCOREBOARD_BASELINE` and never falls back to the candidate snapshot. Detached local runs use the committed empty `scoreboard-main.txt` snapshot. Every previous `pass` must remain present and passing.
- Added CTest coverage for lost PASS rows, missing CI handoff, and an empty CI baseline. Existing page 2 and page 17 tests still compare exact scoreboard rows to live emulated RAM results.
- Documented the baseline provenance and first-merge behavior in README.

## Task Commits

1. **Task 1: Hand off a verified protected-main scoreboard baseline in all CI lanes** - `5489309` (`ci`)
2. **Task 2 RED: Add scoreboard baseline regression checks** - `3c7be4d` (`test`)
3. **Task 2 GREEN: Enforce trusted scoreboard baseline** - `5ae1357` (`feat`)

**Plan metadata:** Summary, state, roadmap, and verification ledger are recorded in the final documentation commit.

## Files Created/Modified

- `tests/cmake/prepare_scoreboard_baseline.cmake` - Fetches, validates, and hands off the protected-main baseline.
- `tests/accuracy/scoreboard-main.txt` - Empty trusted snapshot for detached local runs at this first merge.
- `tests/accuracy/test_scoreboard.c` - Validates scoreboard rows and prevents any baseline PASS from disappearing or changing status.
- `tests/CMakeLists.txt` and `tests/cmake/verify_scoreboard_regression.cmake` - Register no-lost-PASS, missing-baseline, and empty-baseline checks.
- `.github/workflows/ci.yml` - Runs baseline preparation before each of the six build-lane workflow presets.
- `README.md` - Describes CI and local baseline behavior.

## Decisions Made

- A successful lookup that confirms no scoreboard path on protected main is the only condition that permits an empty CI baseline.
- CI requires the workflow handoff variable. It does not select the candidate-controlled local snapshot.
- The first-merge detached local snapshot is empty because protected main currently has no scoreboard file.

## TDD Gate Compliance

- **RED:** The no-lost-PASS CTest was run against the pre-implementation scoreboard executable. It failed on the intended assertion because the old executable accepted the removed `accuracycoin/Removed` PASS. The JUnit report passed `gsd-tools check tdd-red-evidence` with `RED_EVIDENCE_OK`; semantic assessment confirmed the target test executed and failed for the planned behavior. Local evidence: `build/ci/scoreboard-red-evidence.json`.
- **GREEN:** The implementation rejects a lost PASS and the focused scoreboard CTest selection passes all four tests.
- **REFACTOR:** No separate refactor was needed.
- **Commits:** RED `3c7be4d` precedes GREEN `5ae1357`.

## Verification

- `cmake -P tests/cmake/prepare_scoreboard_baseline.cmake` with temporary `RUNNER_TEMP` and `GITHUB_ENV`: passed; protected main was fetched, its scoreboard path was confirmed absent, the resulting baseline was zero bytes, and the handoff variable was appended.
- `ctest --test-dir build/ci -R '^accuracy[.]scoreboard([.]|$)' --output-on-failure`: passed 4/4, including lost-PASS regression, missing CI handoff, and empty CI baseline.
- AccuracyCoin `page2` and `page17` remained passing against the live RAM-derived candidate scoreboard in the full workflow.
- `cmake --workflow --preset ci`: configure and build passed; 344/345 tests passed. The sole failure was the known local `retroarch.testframe` GUI-session abort (`Subprocess aborted`, empty stdout and stderr). This local run is not fully green.
- The six hosted runner lanes were not independently exercised from this local run; the same preparation step is in the six-entry build matrix before each preset.
- `scripts/hygiene.sh --tree` and `git diff --check`: passed before the task commits.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Compare CMake process results as strings to fail closed on launch errors**
- **Found during:** Task 1 final review
- **Issue:** Numeric comparison could treat a non-numeric `execute_process` launch-error string as zero.
- **Fix:** Require the exact result string `0` for fetch, tree lookup, and content retrieval.
- **Files modified:** `tests/cmake/prepare_scoreboard_baseline.cmake`
- **Verification:** Re-ran the finalized preparation script successfully against protected main.
- **Committed in:** `5ae1357`

**Total deviations:** 1 auto-fixed (Rule 1)
**Impact on plan:** The fix closes a fail-open edge in the required error handling without changing scope.

## Issues Encountered

- The local RetroArch test abort remains an environment-specific GUI-session failure already recorded in project state; all other local workflow tests passed.
- Hosted execution on all six runners remains to be observed in GitHub Actions.

## User Setup Required

None - no external service configuration is required.

## Next Phase Readiness

The scoreboard baseline gate and GAME-04 regression checks are implemented. Phase 4 can proceed; the six-lane hosted run should be observed in CI.

---
*Phase: 03-a-real-game-in-retroarch · Plan: 13 · Completed: 2026-10-08*

## Self-Check: PASSED

- Summary and all three implementation/test artifacts exist; the local baseline snapshot is zero bytes as intended.
- Task commits `5489309`, `3c7be4d`, and `5ae1357` are ancestors of the plan head.
- The persisted pre-plan base `e93551967af364b03acd3fa1f8451ae156fcc0c2` measures three plan commits through `5ae1357598a19ccc2573e4ebf04f825905aa105f`.
