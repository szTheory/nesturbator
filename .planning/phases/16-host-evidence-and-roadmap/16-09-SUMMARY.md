---
phase: 16-host-evidence-and-roadmap
plan: "09"
subsystem: testing
tags: [readiness, timing, monotonic-clock, ctest, libretro]

requires:
  - phase: 16
    provides: "Existing host progression driver, readiness supervision, fixture checks, and full session control matrix"
provides:
  - "Coherent monotonic journal-row sampling with the existing TSV schema and deadlines intact"
  - "Focused delayed and stalled readiness tests that traverse the full prefix and cleanup path"
  - "Measured timing evidence, fresh source-bound review, and a safe Plan 07 handoff"
affects: [16-07, 16-08, EVID-02, EVID-04]

actuals:
  tokens: 11674
  tasks: 3
  commits: 5
commits: 5
plan_head_before: 5458a6733ad15185521e86fa1bd8aa4becead7c2
plan_head_after: dfc4f66e9b92aa4395096916b5aefaaff20176c3

tech-stack:
  added: []
  patterns: ["One Perl process samples and serializes each journal row", "Private CTest selector shares focused cases with the full matrix"]

key-files:
  created:
    - .planning/phases/16-host-evidence-and-roadmap/16-TIMING-REPAIR.md
  modified:
    - tests/retroarch/state_session.sh
    - tests/retroarch/test_state_session.sh
    - tests/retroarch/CMakeLists.txt
    - README.md
    - include/nesturbator.h
    - .planning/phases/16-host-evidence-and-roadmap/16-REVIEW.md
    - .planning/phases/16-host-evidence-and-roadmap/16-REVIEW-DISPOSITION.md

key-decisions:
  - "Use one monotonic observation and one Perl serialization process per row while preserving the eleven-column schema, sanitization, failure reason, and all timing bounds."
  - "Treat the measured local process reduction as a bounded repair; native Intel slowdown causation remains unproven and EVID-04 stays pending publication evidence."

patterns-established:
  - "Focused readiness cases use the same shared full-prefix controls as the complete session matrix."
  - "Attempt-local timing and enclosing test-process timing are reported as separate measurements."

requirements-completed: [EVID-02]
coverage:
  - id: D1
    description: "Journal rows retain their schema and report coherent monotonic attempt accounting through cleanup."
    requirement: EVID-02
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^retroarch.session.readiness$' --output-on-failure"
        status: pass
      - kind: other
        ref: "Fresh deep review at e2bed6aeb85b773e44597e8f5d27de94d2dc383f"
        status: pass
    human_judgment: false
  - id: D2
    description: "Delayed and stalled exact readiness responses traverse the full prefix with unchanged timing and cleanup controls."
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^retroarch.session.readiness$' --output-on-failure"
        status: pass
      - kind: integration
        ref: "ctest --preset ci -R '^retroarch.session.controls$' --output-on-failure"
        status: pass
    human_judgment: false
  - id: D3
    description: "Timing evidence distinguishes measured journal work from unresolved native host causation and preserves the Plan 07 handoff."
    verification:
      - kind: other
        ref: ".planning/phases/16-host-evidence-and-roadmap/16-TIMING-REPAIR.md"
        status: pass
      - kind: other
        ref: "Fresh review and disposition: status clean, open findings 0"
        status: pass
    human_judgment: false

duration: 38min
completed: 2026-10-15
status: complete
---

# Phase 16 Plan 09: Readiness timing repair summary

**A single Perl journal pass removed redundant readiness-row subprocesses while focused and full-prefix tests kept every original timing and cleanup bound.**

## Performance

- **Duration:** 38 min
- **Started:** 2026-10-15T15:00:51Z
- **Completed:** 2026-10-15T15:38:45Z
- **Tasks:** 3
- **Files modified:** 8

## Accomplishments

- Consolidated each journal row's monotonic accounting and sanitization into one Perl process while retaining all eleven fields, first-failure reporting, sanitization, child supervision, cleanup, and existing budgets.
- Added a focused `retroarch.session.readiness` CTest for both delayed and response-stalled exact reads; each executes the same full prefix and cleanup assertions used by the complete session matrix.
- Recorded the failed native run, bounded local measurements, full workflow results, and a fresh review at the final repair source hash. The native root cause remains unproven; Plan 07/08 and EVID-04 remain open.

## Task Commits

1. **Task 1: Prove the response-stall prefix through a cheaper, equivalent journal path** — `c5459a0` (RED test), `526b1e0` (implementation and docs).
2. **Task 2: Require both readiness timing controls and journal integrity** — `ff6f058` (RED test), `e2bed6a` (implementation and docs).
3. **Task 3: Record source-bound timing evidence and review** — `dfc4f66`.

## Files Created/Modified

- `tests/retroarch/state_session.sh` — one coherent monotonic sampling and serialization pass per journal row.
- `tests/retroarch/test_state_session.sh` — focused selector, shared delayed/stalled cases, full-prefix, timing, journal, and cleanup assertions.
- `tests/retroarch/CMakeLists.txt` — registered the focused readiness CTest with a 60-second guard; retained the full matrix's 900-second guard.
- `README.md`, `include/nesturbator.h` — documented the bounded host-proof timing behavior.
- `.planning/phases/16-host-evidence-and-roadmap/16-TIMING-REPAIR.md` — native and local measurements, test identities, causal limits, and Plan 07 handoff.
- `.planning/phases/16-host-evidence-and-roadmap/16-REVIEW.md`, `.planning/phases/16-host-evidence-and-roadmap/16-REVIEW-DISPOSITION.md` — fresh source-bound review and retained prior dispositions.

## Decisions Made

- Sample and serialize each row in one Perl invocation; preserve the TSV contract, sanitization, operation timestamps, original failure reason, and all bounds.
- Keep local process-count and timing results separate from the unresolved native Intel slowdown cause. Do not claim that local CI establishes a platform-specific root cause or closes EVID-04.

## TDD Gate Compliance

- **Task 1 RED:** the focused CTest ran the real response-stall prefix, reached `PREFIX_OK`, and failed the intended journal invariant: `5756 + 24238 = 29994`, six milliseconds short of the configured 30,000 ms. The ignored classifier returned `RED_EVIDENCE_OK`. **GREEN:** `526b1e0` consolidates sampling and serialization; the focused tracer then passed.
- **Task 2 RED:** the stalled full-prefix case passed, while the planned case-count assertion correctly failed because the selector still ran only one case. **GREEN:** `e2bed6a` added the delayed case and coherent controls; both focused cases passed.

## Verification

- Final focused `retroarch.session.readiness` passed both cases in 19.38 seconds. Delayed operation: 1,624 ms; response-stall operation: 5,106 ms. Both later prefixes and cleanup completed under the existing strict `<25000 ms` limit.
- Final standalone `retroarch.session.controls` passed the full matrix in 284.90 seconds, including both readiness cases and the existing progression, send-count, lock, interruption, failure, and cleanup controls.
- `cmake --workflow --preset ci` exited 0: all 649 tests passed in 296.60 seconds, then the package step generated the library, runner, and libretro archives.
- Fresh deep source review at `e2bed6aeb85b773e44597e8f5d27de94d2dc383f`: clean, zero actionable findings; disposition `open: 0`.
- `git diff --check 30a009838b741a429863e7cb5143526fbaad8ed7 HEAD` and the documentation whitespace checks passed before the evidence commit.

## Auto-fixed Issues

**1. [Rule 1 - Bug] Corrected the startup-boundary count for a headerless fixture file.**
- **Found during:** Task 2
- **Issue:** The test expected four boundary rows although the file contains three rows and no header.
- **Fix:** Corrected the expected row count to three.
- **Files modified:** `tests/retroarch/test_state_session.sh`
- **Verification:** Focused two-case test, complete session matrix, and normal CI workflow passed.
- **Committed in:** `e2bed6a`

**2. [Rule 1 - Bug] Compared operation timing against the final non-cleanup failure row.**
- **Found during:** Task 2
- **Issue:** A wrong-readiness control emits both a malformed live row and a stopped terminal row with the same reason; reason-only matching incorrectly treated those rows as a timing regression.
- **Fix:** Compare operation fields from the final non-cleanup row and assert cleanup preserves the terminal reason.
- **Files modified:** `tests/retroarch/test_state_session.sh`
- **Verification:** Complete session matrix and normal CI workflow passed.
- **Committed in:** `e2bed6a`

These were inline correctness fixes within the planned test work; no scope or timing-bound deviation was made.

## User Setup Required

None - no external service configuration is required for this local repair.

## Next Phase Readiness

Plan 16-07 can resume at Task 1 by reconciling existing preparation commit `30a0098` and reusing PR #50. It must obtain fresh native PR CI on the final pushed head before any exact-head merge-consent request and stop on terminal CI failure. Plan 16-08 still owns the actual publication-byte evidence. EVID-04 and Phase 16 remain incomplete; this plan did not mutate PR or release state.

---
*Phase: 16-host-evidence-and-roadmap*
*Completed: 2026-10-15*

## Self-Check: PASSED

All listed plan artifacts exist, all five task commits are ancestors of the current branch head, and the modified source/docs scan found no stubs or whitespace errors.
