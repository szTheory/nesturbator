---
phase: 14-runner-process-contract
plan: 01
subsystem: runner
tags: [exit-status, x-macro, help, readme, ctest]

requires:
  - phase: 13
    provides: runner with exit statuses 0, 1, 2, 4 and 5 already in use
provides:
  - runner/exit_status.h with RUNNER_EXIT_TABLE, the generated enum and the reserved text
  - nesturbator-run --help (stdout, exit 0) generated from the table
  - README "### Exit status" section
  - runner.exit.* tests and the exit_status label
affects: [14-02, 14-03, 14-04, 14-05]

actuals:
  tokens: 14000
  tasks: 2
  commits: 2

plan_head_before: 4a0673ba651249b8b45f9438c33cd5646908432f
plan_head_after: 8c6f58991d3d363c60088c42e3332a877b3a1e22

tech-stack:
  added: []
  patterns: ["X-macro table as the single source for enum, --help and a README check"]

key-files:
  created: [runner/exit_status.h, tests/cmake/runner_exit_table.cmake]
  modified: [runner/main.c, README.md, tests/CMakeLists.txt]

key-decisions:
  - "3 and 77 have no enumerator; they appear only in the reserved sentence"
  - "Boolean helpers keep 0/1; only values that become the process status use RUNNER_EXIT_*"

requirements-completed: [RUN-03]

coverage:
  - id: D1
    description: "Exit table defined once and equal in exit_status.h, --help and the README"
    requirement: RUN-03
    verification:
      - kind: unit
        ref: "ctest runner.exit.table"
        status: pass
    human_judgment: false
  - id: D2
    description: "--help prints usage and exit statuses to stdout and exits 0; bad command line exits 2"
    requirement: RUN-03
    verification:
      - kind: unit
        ref: "ctest runner.exit.help, runner.exit.usage, runner.usage.help_after_bad"
        status: pass
    human_judgment: false
  - id: D3
    description: "One test per status 0, 1, 2, 4, 5 under label exit_status"
    requirement: RUN-03
    verification:
      - kind: unit
        ref: "ctest -L exit_status"
        status: pass
    human_judgment: false

duration: 20min
completed: 2026-10-12
status: complete
---

# Phase 14 Plan 01: Runner exit status contract Summary

**One X-macro exit table in runner/exit_status.h drives nesturbator-run --help, is checked against the README by runner.exit.table, and names every process status in runner/main.c.**

## Performance

- **Duration:** about 20 min
- **Completed:** 2026-10-12T22:10:00Z
- **Tasks:** 2
- **Files modified:** 5 (2 created)

## Accomplishments
- `RUNNER_EXIT_TABLE(X)` with rows 0, 1, 2, 4, 5 and `RUNNER_EXIT_RESERVED_TEXT`; no enumerator for 3 or 77.
- `--help` prints usage and the generated Exit status block to stdout (exit 0); `--bogus --help` still exits 2.
- README `### Exit status` table; the two older sentences now link to it.
- `tests/cmake/runner_exit_table.cmake` compares header, `--help` and README (order, numbers, text); changing a README cell makes it fail.
- Every status site in main.c uses a constant; `ctest -L exit_status` covers all five statuses.

## Task Commits

1. **Task 1: One exit table from header to --help to README** - `852a0ad` (feat)
2. **Task 2: Named constants and one test per status** - `8c6f589` (refactor)

## Files Created/Modified
- `runner/exit_status.h` - the table, enum and reserved text
- `runner/main.c` - print_usage, --help, constants at every status site
- `README.md` - Exit status section
- `tests/cmake/runner_exit_table.cmake` - table and help checks
- `tests/CMakeLists.txt` - runner.exit.* tests and labels

## Decisions Made
None - followed plan as specified.

## Deviations from Plan

**1. [Rule 1 - Bug] Plan verify expected `Total Tests: 8` for `ctest -N -L exit_status`**
- **Found during:** Task 2
- **Issue:** ctest adds the `runner.save.fixture` setup test (FIXTURES_REQUIRED) to a label selection, so it reports 9. The eight named tests are exactly the labelled ones.
- **Fix:** Verified the list by name instead of by count; no code change.
- **Committed in:** n/a

**2. [Rule 3 - Blocking] clang-format on exit_status.h**
- **Found during:** Task 2 verification
- **Issue:** a comment line in the Task 1 header exceeded the format width.
- **Fix:** reformatted; included in the Task 2 commit (`8c6f589`).

**Total deviations:** 2 (1 verify-count, 1 formatting). **Impact:** none on behavior.

## Issues Encountered
A comment containing `runner/*.c` tripped `-Wcomment`; reworded before the first commit.

## Verification
- `cmake --workflow --preset ci`: 552/552 passed. `cmake --workflow --preset hygiene`: passed.
- GCC 14 (Docker) compile of runner/main.c with the strict flags: exit 0.
- clang-format 18 dry run on main.c and exit_status.h: exit 0.

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
Plans 02-05 can add stop paths to a named, tested, documented status table.

## Self-Check: PASSED

---
*Phase: 14-runner-process-contract*
*Completed: 2026-10-12*
