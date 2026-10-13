---
phase: 16-host-evidence-and-roadmap
plan: "02"
subsystem: testing
tags: [retroarch, progression, evidence, deterministic]

requires:
  - phase: 16-host-evidence-and-roadmap
    provides: Qualified marker images and retained native anchors from Plan 16-01
provides:
  - Bounded versioned progression evidence records and one reason-coded production verdict
  - Core-generated complete and prefix fixtures using original and rewound native origins
  - Independent semantic controls for exact progression, ambiguity, origin and process failures
affects: [Phase 16 Plan 03 host driver, EVID-01]

actuals:
  tokens: 23267
  tasks: 2
  commits: 4
plan_head_before: 3055f53c33c1d4da70d2f62c78ba6875edb1ee7f
plan_head_after: 7dcf5a1c048c51b62c796a79b73a7a9588660654

tech-stack:
  added: []
  patterns:
    - Shared reason-coded verdict for CLI observations and independent controls
    - Bounded versioned evidence records with immutable native branch anchors

key-files:
  created: [tests/retroarch/progression.h]
  modified:
    - tests/retroarch/frame_index.c
    - tests/retroarch/CMakeLists.txt
    - README.md
    - include/nesturbator.h

key-decisions:
  - "Keep the evidence parser and verdict private to the test harness; no public API or state format changes."
  - "Treat multiple absolute-frame candidates as AMBIGUOUS and zero or mismatched candidates as IDENTITY."
  - "EVID-01 remains pending until Plan 16-03 proves the real-host network sequence."

patterns-established:
  - "Real marker images and native state anchors enter the same verdict exercised by independent controls."
  - "A compound load-plus-probe is counted as one load and one planned frame advance; run-ahead display lead is separate."

requirements-completed: [] # EVID-01 remains pending real-host acceptance in Plan 16-03.
coverage:
  - id: D1
    description: "A core-generated native continuation passes the shared prefix verdict, while incomplete evidence cannot pass complete mode."
    verification:
      - kind: integration
        ref: "cmake --workflow --preset ci: retroarch.progression.tracer, retroarch.progression.prefix, retroarch.progression.complete_required"
        status: pass
    human_judgment: false
  - id: D2
    description: "The complete 16-boundary path and 44 independent exact-reason controls pass, including A,A,B ambiguity and immutable origins."
    verification:
      - kind: integration
        ref: "retroarch.progression.controls (44 cases); cmake --workflow --preset ci"
        status: pass
    human_judgment: false

duration: 34min
completed: 2026-10-13
status: complete
---

# Phase 16 Plan 02: Shared progression verdict Summary

**A single bounded verdict now validates real core-generated progression evidence and rejects ambiguous or contradictory histories by reason.**

## Performance

- **Duration:** 34 minutes from the first persisted RED run (plan loading preceded that recorded run).
- **Started:** 2026-10-13T21:48:23Z (first persisted RED evidence).
- **Completed:** 2026-10-13T22:22:02Z.
- **Tasks:** 2 complete.
- **Files modified:** 5 planned production and documentation files.

## Accomplishments

- Added a bounded, versioned evidence format and one reason-coded verdict shared by the CLI and controls. Prefix validation reports `PREFIX_OK`; complete mode requires all prescribed boundaries and a clean terminal exit.
- Generated actual decoded marker frames and native saves on isolated core instances. The complete fixture uses one retained original anchor for pause, five advances, load continuation and resume, and one retained rewound anchor for coarse rewind, its three advances and run-ahead load/probe.
- Added 44 independent controls for dropped, doubled and compensated steps; load/probe counts; same-image A,A,B ambiguity; attribution and anchor errors; branch relations; display lead; deadlines; process state; malformed input; and CLI I/O/parse exit status.
- Updated README and public-header comments to describe the evidence boundary while retaining the broader portable machine-state and audio guarantees.

## Task Commits

1. **Task 1 RED:** `d751650` — add failing progression tracer test.
2. **Task 1 GREEN:** `71d77b8` — implement shared progression tracer verdict.
3. **Task 2 RED:** `d8194af` — add failing progression ambiguity control.
4. **Task 2 GREEN:** `7dcf5a1` — implement comprehensive progression controls.

The four task commits were measured from `plan_head_before` through `plan_head_after`.

## Files Created/Modified

- `tests/retroarch/progression.h` — bounded records, strict parser, reason codes and shared verdict.
- `tests/retroarch/frame_index.c` — real-core evidence generation, CLI adapter, and semantic controls.
- `tests/retroarch/CMakeLists.txt` — normal-CI tracer, prefix, complete-mode and controls tests.
- `README.md` — host evidence boundaries, required branches and control coverage.
- `include/nesturbator.h` — public comments clarify the scope of displayed continuation evidence.

## Decisions Made

- Kept evidence parsing and progression semantics private to test tooling; no dependency, public API, native state format or emulator behavior changed.
- Rejected multiple frame candidates as `AMBIGUOUS`, and zero or mismatched candidates as `IDENTITY`; no candidate selection can make an observation fit a relation.
- Kept `EVID-01` pending for Plan 16-03, which must connect the exact shared verdict to bounded real-host network actions.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Removed output-only CTest pass regexes from progression tests.**
- **Found during:** Task 2 RED.
- **Issue:** CTest `PASS_REGULAR_EXPRESSION` can mark a test passed based on matching output even when the process exits unsuccessfully, weakening exit-status verification.
- **Fix:** Removed those pass regexes; the executable asserts the expected verdict and CTest uses the real process exit status.
- **Files modified:** `tests/retroarch/CMakeLists.txt`.
- **Verification:** `retroarch.progression.controls` and the full 642-test CI workflow passed.
- **Committed in:** `d8194af` (test gate setup), with final test definitions in `7dcf5a1`.

**Total deviations:** 1 auto-fixed test-gate issue.
**Impact on plan:** Strengthened the planned acceptance checks without changing scope.

## TDD Gate Compliance

- **Task 1 RED:** `build/ci/progression-red.json` records the named `retroarch.progression.tracer` assertion failing with `INCOMPLETE` where `PREFIX_OK` was expected; the target executed against decoded frame 151 and a native-valid origin at frame 150. `gsd_run check tdd-red-evidence` returned `RED_EVIDENCE_OK` (`target_test_failed`).
- **Task 1 GREEN:** `71d77b8` added the shared verdict and real-core tracer; the prefix and reason-specific wrong-origin checks passed.
- **Task 2 RED:** `build/ci/progression-controls-red.json` records `retroarch.progression.controls` failing its planned zero-candidate assertion (`AMBIGUOUS` instead of `IDENTITY`) after the actual core-backed prefix passed. The evidence classifier returned `RED_EVIDENCE_OK` (`target_test_failed`).
- **Task 2 GREEN:** `7dcf5a1` implemented the intended reason and independent control suite; the controls report 44 passing cases.
- **REFACTOR:** No separate refactor was needed after GREEN; the full suite and workflow passed on the committed result.

## Verification

- `ctest --preset ci -R '^retroarch[.]progression[.]' --no-tests=error` — all four progression tests passed.
- `cmake --workflow --preset ci` — all 642 tests passed and all three CI packages were created.
- Direct `frame_index --progression-selftest ... --controls` — 44 controls passed; the intended fixture returned `VERDICT_OK`, the A,A,B fixture returned `AMBIGUOUS`, and malformed/missing-input CLI controls returned exit 2.
- `git diff --check` — passed before the Task 2 commit.

## Issues Encountered

No unresolved implementation issue. The local suite does not launch RetroArch; real-host network acceptance remains Plan 16-03 work.

## User Setup Required

None.

## Next Phase Readiness

Plan 16-03 can connect host observations to this exact parser and verdict. It must preserve the original and rewound origins, one load plus one probe advance, bounded command outcomes and measured display lead. `EVID-01` remains pending until that real-host path passes.

## Self-Check: PASSED

- Summary file exists at the required phase path.
- All four task commits are ancestors of the recorded plan head.
- No added stub or placeholder patterns were found in the five changed files.

---
*Phase: 16-host-evidence-and-roadmap*
*Plan: 02*
*Completed: 2026-10-13*
