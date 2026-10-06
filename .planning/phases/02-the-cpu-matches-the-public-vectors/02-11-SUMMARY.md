---
phase: 02-the-cpu-matches-the-public-vectors
plan: 11
subsystem: ci
tags: [github-actions, gh, jq, cmake, ctest, junit, release-evidence]
requires:
  - phase: 02-the-cpu-matches-the-public-vectors
    provides: "Release workflow and nightly full-vector evidence artifact with exact CTest inventory and JUnit output"
provides:
  - "Post-publication assertion for the exact 18 release archives plus SHA256SUMS"
  - "Read-only command to collect release, exact-merge main-push and first post-merge scheduled vector evidence"
  - "Reusable automation-first UAT rule for later phases"
affects: [phase-02-verification, release, nightly, gsd-handoff]
actuals:
  tokens: 4833.25
  tasks: 2
  commits: 3
tech-stack:
  added: []
  patterns:
    - "GitHub evidence is tied to explicit SHAs; release-please's later tag commit is checked as a descendant of the phase merge"
    - "Archived CTest registration and JUnit results must match the exact 258 full-vector tests"
key-files:
  created:
    - scripts/phase2_outcomes.sh
    - tests/cmake/vector_result_policy.cmake
  modified:
    - .github/workflows/release.yml
    - README.md
    - .planning/preparation/GSD-HANDOFF.md
key-decisions:
  - "Keep the evidence command read-only and noninteractive; report untriggered hosted events as PENDING."
  - "Validate registration and completed JUnit results independently; never infer a pass from test registration alone."
  - "Treat clean-room source provenance as an irreducible judgment because repository automation cannot establish what sources a person or agent opened."
requirements-completed: [CPU-02, CPU-01]
coverage:
  - id: D1
    description: "Publishing asserts the release tag is published with the exact 18 archives and nonempty SHA256SUMS, then records the tag URL."
    requirement: CPU-02
    verification:
      - kind: other
        ref: "actionlint .github/workflows/release.yml"
        status: pass
      - kind: other
        ref: "scripts/phase2_outcomes.sh --self-test (missing-asset fixture)"
        status: pass
    human_judgment: false
  - id: D2
    description: "Read-only command classifies release and main-push/scheduled vector runs by commit and verifies all 258 archived results."
    requirement: CPU-02
    verification:
      - kind: integration
        ref: "scripts/phase2_outcomes.sh --self-test (SHA, artifact, pending, success and result mutations)"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci (327/327; elevated execution)"
        status: pass
    human_judgment: false
  - id: D3
    description: "Future GSD handoffs run objective verification first and reserve owner prompts for consent or irreducible judgment."
    requirement: CPU-01
    verification:
      - kind: other
        ref: "sh scripts/hygiene.sh --tree"
        status: pass
    human_judgment: false
  - id: D4
    description: "Clean-room source provenance remains explicitly identified as a judgment-tier item."
    verification: []
    human_judgment: true
    rationale: "A repository scanner cannot determine which external sources a person or agent opened."
metrics:
duration: 22min
completed: 2026-10-06
status: complete
plan_head_before: d3aba4ae5ac29b92a360b7dce3aff9daf93b9a8b
plan_head_after: c1f1c3c7d37cf13a410af16cabe6e206d5c175b5
commits: 3
---

# Phase 2 Plan 11: Automate release and post-merge evidence Summary

**Release publication now checks the exact remote asset set, while a read-only command validates commit-bound release and 258-test nightly evidence and leaves future events pending.**

## Performance

- **Duration:** 22 minutes
- **Started:** 2026-10-06T16:53:00Z
- **Completed:** 2026-10-06T17:14:54Z
- **Tasks:** 2
- **Files modified:** 5

## Accomplishments

- Added a bounded post-publication metadata check that requires the expected tag, a non-draft release, exactly the version's 18 platform archives plus SHA256SUMS, and nonzero asset sizes; the job summary records the release URL.
- Added a read-only, noninteractive command that queries the published release and hosted workflow runs by supplied merge SHA and tag. It verifies the release-please tag commit descends from the supplied merge commit, requires a successful release workflow run on the tag commit, validates exact-merge main-push and first later scheduled run status, downloads only the named evidence artifact to a temporary directory, and checks exact registration and JUnit results.
- Documented the evidence command and added a durable handoff rule that runs command-verifiable checks before owner UAT while keeping external-source provenance as a judgment item.

## Task Commits

1. **Task 1: Verify published assets and collect commit-bound GitHub evidence** - `fe21b93` (`ci`)
2. **Task 2: Record the automation-first UAT rule once** - `bbc3f57` (`docs`)
3. **Task 1 follow-up: Fix release retry bound and distinguish pending exit status** - `c1f1c3c` (`fix`)

## Files Created/Modified

- `scripts/phase2_outcomes.sh` - Read-only GitHub evidence collection and pending/failure/pass reporting.
- `tests/cmake/vector_result_policy.cmake` - Fail-closed comparison of the exact 258-test inventory and completed JUnit names.
- `.github/workflows/release.yml` - Bounded remote release metadata assertion and job summary link.
- `README.md` - Post-merge command usage and pending-state behavior.
- `.planning/preparation/GSD-HANDOFF.md` - Automation-first UAT operating rule.

## Decisions Made

- Require the tag commit to be a descendant of the supplied Phase 2 merge SHA; release-please creates a later commit, so the two SHAs are not equated.
- Keep external events pending until the hosted run exists and its archived results validate successfully.
- Preserve clean-room provenance as a judgment-tier check since repository automation cannot prove browsing or source-opening history.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Corrected an unused retry-loop variable flagged by actionlint**
- **Found during:** Task 1
- **Issue:** The initial bounded retry loop triggered actionlint's embedded shellcheck warning for an unused variable.
- **Fix:** Replaced it with an explicit bounded counter.
- **Files modified:** `.github/workflows/release.yml`
- **Verification:** `actionlint .github/workflows/release.yml` passed.
- **Committed in:** `fe21b93`

**2. [Rule 1 - Bug] Kept metadata retries bounded and exposed pending as a distinct exit status**
- **Found during:** Final review after Task 2
- **Issue:** Asset mismatch could skip retry-counter advancement, and a pending hosted event returned the same nonzero exit code as a failure.
- **Fix:** Let every failed metadata attempt reach the bounded counter increment; return exit 2 for pending and exit 1 for failures.
- **Files modified:** `.github/workflows/release.yml`, `scripts/phase2_outcomes.sh`
- **Verification:** `actionlint`, shell syntax, and self-tests passed.
- **Committed in:** `c1f1c3c`

**Total deviations:** 2 auto-fixed (Rule 1). **Impact:** Kept bounded retries finite and made pending machine-distinguishable from failure.

## Verification

- `sh -n scripts/phase2_outcomes.sh` - passed.
- `scripts/phase2_outcomes.sh --self-test` - passed synthetic wrong-SHA, missing-artifact, missing-asset, pending-schedule and success checks, plus wrong/duplicate/missing test, failure and skip result mutations.
- `actionlint .github/workflows/release.yml` - passed.
- `sh scripts/hygiene.sh --tree` - passed.
- `cmake --workflow --preset hygiene` - passed 8/8.
- `cmake --workflow --preset ci` - passed 327/327; RetroArch test passed and packages generated. Elevated execution was required because RetroArch aborts under the default shell sandbox.
- No real hosted outcome was queried: the owner-authorized merge has not happened. The command reports those release and nightly events as pending until they exist. No remote state was changed.

## Issues Encountered

The default sandbox denied writes to the local Git index and plan ledger. The authorized local commits were created with escalated execution; no GitHub write or release publication was attempted.

## User Setup Required

None.

## Next Phase Readiness

The release assertion and post-merge evidence collector are ready for the authorized merge. Run `scripts/phase2_outcomes.sh <phase-2-merge-sha> <release-tag>` after merge and keep any not-yet-triggered scheduled run pending. No publishing, pushing or merging was performed.

---
*Phase: 02-the-cpu-matches-the-public-vectors*
*Completed: 2026-10-06*

## Self-Check: PASSED

- Summary file exists; task commits `fe21b93`, `bbc3f57` and `c1f1c3c` are ancestors of the plan head.
- Existing owner-reserved changes `.planning/HANDOFF.json` and `.planning/config.json` were preserved and excluded from commits.
