---
phase: 16-host-evidence-and-roadmap
plan: "56"
subsystem: testing
tags: [shell, cmake, retroarch, lock-reservation, asan, ci]

requires:
  - phase: 16-30
    provides: Existing host-session harness, lifecycle controls, package checks and source evidence.
provides:
  - Executing-harness source identity independent of the caller-supplied driver and cwd.
  - Fixed production-port validation and close-only FD 9 lock release with current-source CR-01 evidence.
  - Final local and parent CI/package acceptance plus an independent clean exact-source review.
affects: [phase-16-delivery, retroarch-host-tests, package-ci]

tech-stack:
  added: []
  patterns:
    - Derive trusted harness identity from the executing script before importing driver-controlled source.
    - Hold lock ownership in the inherited kernel socket and release by closing the descriptor.

key-files:
  created:
    - .planning/phases/16-host-evidence-and-roadmap/16-56-SOURCE-ACCEPTANCE.md
    - .planning/phases/16-host-evidence-and-roadmap/16-56-REVIEW.md
    - .planning/phases/16-host-evidence-and-roadmap/16-56-REVIEW-DISPOSITION.md
    - .planning/phases/16-host-evidence-and-roadmap/16-56-SUMMARY.md
  modified:
    - tests/retroarch/state_session.sh
    - tests/retroarch/test_state_session.sh
    - tests/retroarch/interruption_trace.sh
    - README.md
    - include/nesturbator.h

key-decisions:
  - Production bootstrapped re-entry must validate the fixed port 55355; alternate ports remain restricted to recognized test mode.
  - CR-01 acceptance uses the same frozen oracle and harness for the pinned old driver and final current driver.
  - Passing local and parent checks establish current-source acceptance only; native consent and released-byte evidence remain separate delivery gates.

patterns-established:
  - Source-root identity is resolved from the active harness invocation, not a supplied driver path or current directory.
  - The production lock endpoint is a validated inherited TCP socket; cleanup closes FD 9 without deleting shared pathnames.

requirements-completed: [EVID-02]
requirements-retained-open: [EVID-01, EVID-03, EVID-04, DIR-01, DIR-02]

plan_head_before: af7f9d025ce42dfd18540ad031ff638da75b8eef
plan_head_after: 2c356d574290aef1d57122566f7b81beaaa10027
commits: 2
duration: ">35 minutes; exact start time not retained across recovery"
completed: 2026-10-23
status: complete

coverage:
  - id: D1
    description: The tracked harness rejects copied driver/include sources before source import, host launch or send, and exercises all twelve finalizer/cwd cases.
    requirement: EVID-02
    verification:
      - kind: integration
        ref: "build/ci/phase16-direct/source-root-final/console.log"
        status: pass
      - kind: integration
        ref: "build/phase16-direct/review-alt-port-focused-ci/ctest.log"
        status: pass
    human_judgment: false
  - id: D2
    description: Production lock re-entry requires the fixed endpoint and current cleanup preserves foreign objects under the same-oracle causal control.
    verification:
      - kind: integration
        ref: "build/ci/phase16-direct/cr01-final-candidate2/logs/old.stdout and current.stdout"
        status: pass
      - kind: other
        ref: "16-56-REVIEW.md: clean exact-source review at candidate 2c356d5"
        status: pass
    human_judgment: false
  - id: D3
    description: Final source, packages and parent-configured acceptance pass the four-stage CI workflow without skipped tests.
    verification:
      - kind: integration
        ref: "build/phase16-parent56/final-parent/ci-workflow.log; 649/649 passed, zero skips"
        status: pass
      - kind: integration
        ref: "build/phase16-parent56/final-parent/packages.log; all three archives accepted"
        status: pass
    human_judgment: false
---

# Phase 16 Plan 56: Current-source Host Evidence Acceptance Summary

**The session harness now verifies its own source identity, validates production lock re-entry on port 55355, and preserves foreign lock objects under the final-source causal test.**

## Performance

- **Duration:** At least 35 minutes; the exact initial start time was not retained through the recovery handoff.
- **Started:** Before the first retained source commit at 2026-10-22 20:39:20 EDT.
- **Completed:** 2026-10-23 01:14 UTC (2026-10-22 21:14 EDT).
- **Tasks:** 3 completed.
- **Files modified:** 9 (five behavior/test/documentation paths and four Plan 16-56 metadata paths).

## Accomplishments

- The harness resolves its trusted identity from its own invocation before importing the driver/include. A copied driver/include pair is refused before sourcing, host launch or send. Repository-root, CTest-binary and space-bearing cwd runs passed, including all four finalizer faults in each cwd with wait 143, the retained first reason, `cleanup_anomaly=0`, no premature oracle result and exact-port re-entry.
- Production lock re-entry now requires port 55355 outside test mode. A new negative control creates a genuine non-listening alternate-port FD 9 socket with matching PID/start/device/inode metadata and verifies early refusal. Cleanup releases the validated socket by closing FD 9 and does not unlink or rename the shared legacy pathnames.
- The final-source CR-01 pair used the same final harness and frozen oracle: the pinned old driver returned 143 and the oracle classified `FOREIGN_OBJECT_CHANGED`; the final driver returned 143 and the oracle classified `FOREIGN_OBJECT_PRESERVED`. Both five-target suites passed their evidence checks; current cleanup also demonstrated held-port refusal and exact-port re-entry after owned reap.
- The final current source passed focused CI and focused ASan/UBSan (2/2 each), then the four-stage CI workflow with 649/649 tests, zero skips and four CTest jobs. Parent acceptance independently passed the configured build, full workflow and package validator on the same candidate. Three fresh packages passed archive checks and matched source metadata.
- The standard independent review covered exactly five candidate paths and is clean with zero findings. The original CR-01 is closed; the production alternate-port finding from the first review was corrected and the exact final candidate was reviewed again.
- The existing 39-row behavior audit and established trace, capture, sidecar, field, cleanup and reacquisition controls remain in place. D-01–D-17 and all six requirement areas remain represented; no test was removed, skipped, weakened or given a larger timeout.

## Task Commits

1. **Tasks 1–2: paired source, test and documentation acceptance** — `2e9928b` (`fix(16-56): enforce harness identity and kernel lock lifecycle`).
2. **Task 3 follow-up: resolve the review finding for forged production re-entry** — `2c356d5` (`fix(16-56): reject forged production lock reentry`).

The four metadata files in this summary commit record acceptance and review results for those two measured plan source commits.

## Files Created/Modified

- `tests/retroarch/state_session.sh` — independent source-root setup, kernel reservation lifecycle, production fixed-port re-entry validation and bounded cleanup.
- `tests/retroarch/test_state_session.sh` — cwd/finalizer/source-root and valid alternate-port controls, with same-oracle causal evidence.
- `tests/retroarch/interruption_trace.sh` — validates the inherited socket reservation rather than filesystem-path occupancy.
- `README.md` and `include/nesturbator.h` — paired documentation for the session behavior and controls.
- `16-56-SOURCE-ACCEPTANCE.md` — initial and final source hashes, retained failures, causal evidence, local and parent gate results.
- `16-56-REVIEW.md` and `16-56-REVIEW-DISPOSITION.md` — exact-source standard review and actual finding dispositions.
- `16-56-SUMMARY.md` — this execution record.

## Decisions Made

- Production bootstrapped re-entry trusts only a validated FD 9 socket bound to port 55355; ephemeral guard ports are limited to test mode.
- CR-01 closure requires an actual old-source RED and current-source GREEN under the same harness and frozen oracle; the older pre-review pair is retained as historical evidence only.
- Current-source and parent acceptance do not imply native-host consent, publication approval or successful released-byte CI.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Restore creation of a fresh lock-root directory before endpoint preflight**
- **Found during:** Task 2 focused CI.
- **Issue:** A production-prefix control used a fresh lock root that the driver no longer created, so it failed before reaching endpoint behavior.
- **Fix:** Create the configured lock root with bounded `mkdir -p` and refuse on failure.
- **Files modified:** `tests/retroarch/state_session.sh`.
- **Verification:** Focused CI and the final 649-test workflow passed.
- **Committed in:** `2e9928b`.

**2. [Rule 1 - Bug] Preserve the O_EXCL reservation observation before retrying a setup-failure fixture**
- **Found during:** Task 2 focused CI.
- **Issue:** The test retried with an existing exclusive-create record and failed for fixture setup rather than intended driver behavior.
- **Fix:** Assert and move the first reservation observation to the failure-case record before retry.
- **Files modified:** `tests/retroarch/test_state_session.sh`.
- **Verification:** Focused CI and ASan/UBSan focused tests passed.
- **Committed in:** `2e9928b`.

**3. [Rule 1 - Bug] Bind the interruption fixture to its defined lock root**
- **Found during:** Task 2 focused CI.
- **Issue:** `interrupt_lock_root` was unset under `set -u`, preventing the intended interruption control.
- **Fix:** Define `$interrupt_dir/locks` once and use it for fixture setup and driver environment.
- **Files modified:** `tests/retroarch/test_state_session.sh`.
- **Verification:** Focused CI and ASan/UBSan focused tests passed.
- **Committed in:** `2e9928b`.

**4. [Rule 1 - Bug] Require fixed production port for bootstrapped re-entry**
- **Found during:** Task 3 independent standard review.
- **Issue:** A valid environment-matched FD 9 socket on an alternate port could otherwise bypass the shared production reservation.
- **Fix:** Reject alternate ports outside test mode and add a valid-socket negative control that checks no reservation record, host launch or transport launch occurs.
- **Files modified:** `tests/retroarch/state_session.sh`, `tests/retroarch/test_state_session.sh`.
- **Verification:** Direct PID preflight, focused CI, focused ASan/UBSan, final full workflow and fresh exact-candidate review passed.
- **Committed in:** `2c356d5`.

**Total deviations:** 4 correctness fixes, all within the authorized current-source scope. No architectural change or dependency was added.

## Issues Encountered

Three early focused CI failures exposed the lock-root setup, reservation-observation retry and unbound interruption fixture issues; each failure log is retained in `build/ci/phase16-direct/` and each affected check passed after correction. During the final preflight rerun, a zsh wrapper attempted to assign the read-only `$status` variable after the test itself had passed; a subsequent invocation reused the existing scratch root and refused re-entry. The final rerun used a fresh root and passed. The first exact-source review found the alternate-port re-entry blocker; it was corrected, all affected checks and the full workflow were rerun, and the final review reported zero findings. A separate review concern about busy-port error precedence was withdrawn after the reviewer confirmed input and budget checks precede the bind.

## User Setup Required

None. No external service or manual configuration is required for this current-source acceptance.

## Next Phase Readiness

Plan 16-56 is complete and ready to hand off to the separately invoked Plan 16-07 delivery step. Plan 16-07 must still perform native CI, obtain affirmative consent for the concrete PR/head before protected merge or publication, and preserve the required performance evidence. Plan 16-08 must still validate every downloaded release archive and exact released bytes. These limits leave EVID-01, EVID-03, EVID-04, DIR-01 and DIR-02 open in the phase-level requirements record; this summary does not waive them or update STATE/ROADMAP.

## Self-Check: PASSED

All four metadata files exist. Source commits `2e9928b7eb68604e6d11ecb5b240e7915b28c4e3` and `2c356d574290aef1d57122566f7b81beaaa10027` are ancestors of the recorded candidate. The five tested source blobs match the recorded manifest, and `.planning/config.json` retains its original SHA-256 and remains unstaged.

---
*Phase: 16-host-evidence-and-roadmap*
*Completed: 2026-10-23*
