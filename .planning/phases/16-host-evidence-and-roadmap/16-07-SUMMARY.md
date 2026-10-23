---
phase: 16-host-evidence-and-roadmap
plan: "07"
subsystem: release-delivery
tags: [github-actions, release-please, retroarch, ci]

requires:
  - phase: "16-56"
    provides: accepted and independently reviewed current source for the host evidence correction
provides:
  - Published v0.1.11 candidate on the exact successful release commit for Plan 16-08 byte validation
  - Identity-bound PR consent, merge ancestry, exact-release CI, publication and asset evidence
affects: ["Phase 16 Plan 08", EVID-04]

actuals:
  tokens: 12330 # Measured as chars/4 over the realized plan diff at summary creation.
  tasks: 3
  commits: 9
plan_head_before: cf85c8efabaff2598eaa094d58367adc50b6b5ac
plan_head_after: 788cc12123b372570080e5c8bde36861968ec1bf
commits: 9

tech-stack:
  added: []
  patterns:
    - Exact-head owner consent precedes a protected implementation merge.
    - Release automation publishes only after CI on the exact tagged commit succeeds.

key-files:
  created: [.planning/phases/16-host-evidence-and-roadmap/16-07-SUMMARY.md]
  modified:
    - .planning/phases/16-host-evidence-and-roadmap/16-RELEASE-DELIVERY.md
    - .planning/phases/16-host-evidence-and-roadmap/deferred-items.md
    - .planning/STATE.md
    - .planning/ROADMAP.md
    - .planning/WINDOWS.md
    - include/nesturbator.h
    - tests/retroarch/CMakeLists.txt
    - tests/retroarch/state_session.sh
    - tests/retroarch/test_state_session.sh

key-decisions:
  - "Use the existing protected-main release flow without selecting a version; the release PR generated v0.1.11."
  - "Keep EVID-04 open until Plan 16-08 validates the actual downloaded release bytes."

patterns-established:
  - "A release candidate is recorded only after exact-release CI and publish jobs complete successfully."

requirements-completed: [] # EVID-04 remains open until Plan 16-08 verifies released bytes.

coverage:
  - id: D1
    description: "The existing release flow published v0.1.11 on the exact successful release commit for Plan 16-08 inspection."
    verification:
      - kind: integration
        ref: "GitHub Release workflow 39993108038: CI required and publish jobs on e47feb3afc34baf1d611ef44153af199424f41a2"
        status: pass
    human_judgment: false

duration: "101m"
started: "2026-10-23T16:04:51Z" # Recorded predecessor-state checkpoint; earlier task work is in the plan commits below.
completed: "2026-10-23"
status: complete
---

# Phase 16 Plan 07: Release Delivery Summary

**The consented Phase 16 change was published as v0.1.11 on exact CI-verified commit `e47feb3afc34baf1d611ef44153af199424f41a2`.**

## Performance

- **Duration:** 101 minutes from the recorded pre-merge state checkpoint to publication; earlier Plan 07 preparation is represented by its task commits.
- **Started:** 2026-10-23T16:04:51Z (continuation checkpoint)
- **Completed:** 2026-10-23T17:45:53Z
- **Tasks:** 3
- **Files modified:** 10

## Accomplishments

- PR [#50](https://github.com/szTheory/nesturbator/pull/50), head `82e4686ece0aa8535aa39bbfc06e24d6d6b86af1`, passed fresh native `CI required`, `perf`, and pinned RetroArch checks before the owner explicitly approved that exact head with `yes i approve`.
- The protected squash merge produced `b39a0106e4cccfe9922e051d5680f1f7f84f5efa`. Release PR [#52](https://github.com/szTheory/nesturbator/pull/52) then passed native CI and auto-merged as `e47feb3afc34baf1d611ef44153af199424f41a2`, the direct target of tag `v0.1.11`.
- Exact-release workflow [39993108038](https://github.com/szTheory/nesturbator/actions/runs/39993108038) passed `CI required`, hash equality, all six platform builds, ASan, no-float, hygiene, title, and pinned RetroArch. Its `perf` job was skipped on the push event as permitted by the rollup; fresh PR `perf` had passed before consent.
- The exact-release `publish` job succeeded. It verified the exact 18 archives, created and attested `SHA256SUMS`, verified every archive, rejected a modified archive, published the release, and checked the release metadata. [v0.1.11](https://github.com/szTheory/nesturbator/releases/tag/v0.1.11) is non-draft with 19 nonempty assets.
- Updated the delivery record with the exact consent, PR and release merge identities, successful checks, published tag target, release URL and remaining acceptance boundary.

## Task Commits

1. **Task 1: Prepare the actual Phase 16 PR through native required CI** — eight commits: `450ab3b`, `d11ec88`, `583cb29`, `3e6e674`, `b8da121`, `478581a`, `bf46a54`, `82e4686`.
2. **Task 2: Obtain owner consent for the exact phase PR merge and normal release consequence** — approved checkpoint; no separate code commit.
3. **Task 3: Merge the consented head and observe the normal release handoff** — `788cc12` (docs).

The plan metadata commit is separate from the nine measured task commits above.

## Files Created/Modified

- `.planning/phases/16-host-evidence-and-roadmap/16-07-SUMMARY.md` — exact plan outcome, merge and publication identities.
- `.planning/phases/16-host-evidence-and-roadmap/16-RELEASE-DELIVERY.md` — current release outcome appended while retaining historical failures and reviews.
- `.planning/phases/16-host-evidence-and-roadmap/deferred-items.md` and `.planning/STATE.md` — plan execution tracking.
- `.planning/ROADMAP.md` — Wave 11/12 progress and Plan 16-08 readiness; `.planning/WINDOWS.md` — both resolved deviations recorded with zero open entries.
- `include/nesturbator.h`, `tests/retroarch/CMakeLists.txt`, `tests/retroarch/state_session.sh`, and `tests/retroarch/test_state_session.sh` — bounded test/comment corrections made during Task 1.

## Decisions Made

- The owner’s exact-head approval authorized the ordinary protected squash merge and the existing release flow; no version or tag was preselected.
- The published release is a candidate for Plan 16-08. EVID-04 remains pending until downloaded archives, checksums, attestations and metadata copies are validated.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Removed a no-float scan false positive in a public-header comment.**
- **Found during:** Task 1, native CI.
- **Issue:** The explanatory IPv4 address and port were read as a floating-point token.
- **Fix:** Described the loopback TCP port without the address literal; executable behavior and scanner rules were unchanged.
- **Files modified:** `include/nesturbator.h`.
- **Committed in:** `d11ec88`.

**2. [Rule 1/3 - Test measurement and blocking CI issues] Corrected the host timing controls without relaxing their bounds.**
- **Found during:** Task 1, native CI and local acceptance.
- **Issue:** Earlier failures exposed a cleanup-matrix timing measurement affected by scheduling/probe cost and a TERM-to-exit interval that included wrapper startup.
- **Fix:** Retained timing diagnostics, isolated the timing lane, shortened same-port reacquisition probes, isolated readiness timing, and started TERM-to-exit timing at the signal boundary. Existing thresholds and assertions were preserved.
- **Files modified:** `tests/retroarch/CMakeLists.txt`, `tests/retroarch/state_session.sh`, `tests/retroarch/test_state_session.sh`.
- **Verification:** Local `cmake --workflow --preset ci` passed 649/649 tests without skips; `asan` passed 644/644 without skips; fresh PR CI passed all 15 jobs including `CI required`, `perf` and pinned RetroArch.
- **Committed in:** `583cb29`, `3e6e674`, `b8da121`, `478581a`, `bf46a54`.

**Total deviations:** 2 auto-fixed issue groups (Rule 1: 1; Rule 3: 1, combined with Rule 1).
**Impact on plan:** Corrections stayed within the source/test delivery scope; no CI thresholds, release workflow, dependencies, or product behavior were changed.

## Issues Encountered

- Independent Nightly run [39993108154](https://github.com/szTheory/nesturbator/actions/runs/39993108154) on the published commit reported `vectors-full` failure while `suite-flake` was still running at final observation. Nightly was not a release gate; no rerun or repair was triggered. The exact Release workflow had successful `CI required` and `publish` jobs.
- The release push’s `perf` job was skipped by workflow design. The exact PR head’s native `perf` job passed before the merge, and the release push’s `CI required` rollup succeeded with the configured push-event exception.

## User Setup Required

None — the existing GitHub App and release workflow completed publication without a manual service configuration step.

## Next Phase Readiness

Plan 16-08 is ready to validate the actual published v0.1.11 bytes. EVID-04 and Phase 16 remain open until that plan verifies the downloaded artifacts and metadata. This completion ends at Wave 11; no Plan 08 execution or phase-level verification was started.

## Self-Check: PASSED

The summary and all listed task files exist; all nine measured plan commits are ancestors of `HEAD`; `plan_head_before` and `plan_head_after` match the persisted ledger and current task commit. Both deviation-ledger entries are fixed with `open_count: 0`. The pre-existing `.planning/config.json` SHA-256 remains unchanged and unstaged.

---
*Phase: 16-host-evidence-and-roadmap*
*Completed: 2026-10-23*
