---
phase: 20-nmi-line-and-vblank-flag
plan: "12"
subsystem: hosted-evidence
tags: [cmake, github-actions, git, windows, source-identity]
requires:
  - phase: 20-11
    provides: Shared source/job/artifact validators and metadata-only availability mode
provides:
  - Historical 0039 source-bound CI/Nightly availability facts, retained as superseded for current source
  - Current afba1d0 candidate CI success and required Nightly vectors acquisition failure
  - Halt record showing that no current-source availability handoff exists and 20-10 is blocked
affects: [20-10, phase-20-hosted-acceptance]
actuals:
  tokens: 13863
  tasks: 2
  commits: 10
  plan_head_before: 56a31aa4b76fab4bebc019886fc1b4c5c21528c4
  plan_head_after: 3652f8f3ff62ce3fd659c8f4059cfa6babff5462
tech-stack:
  added: []
  patterns: [Mode-aware raw Git blob validation, metadata-only hosted availability]
key-files:
  created:
    - .planning/phases/20-nmi-line-and-vblank-flag/20-12-SUMMARY.md
  modified:
    - .planning/ROADMAP.md
    - .planning/STATE.md
    - .planning/phases/20-nmi-line-and-vblank-flag/20-12-SUMMARY.md
    - .planning/phases/20-nmi-line-and-vblank-flag/20-DELIVERY.md
    - README.md
    - tests/CMakeLists.txt
    - tests/accuracy/baseline-metadata.txt
    - tests/accuracy/measurements.md
    - tests/cmake/phase20_hosted_evidence.cmake
    - tests/cmake/phase20_hosted_evidence_controls.cmake
    - tests/runner/phase20_fake_gh.c
key-decisions:
  - "Keep artifact-content acceptance exclusively in 20-10; this plan records live run and artifact availability only."
  - "Bind candidate, tested merge, protected base, and current effective source to exact Git tree and blob identity."
patterns-established:
  - "Hash regular Git blobs in batches and resolve mode 120000 entries from their stored symlink-target bytes."
  - "Use synthetic Git info attributes to preserve raw Windows fixture bytes without changing source .gitattributes."
requirements-completed: []
duration: 232min
last_attempted: 2026-10-27
status: halted
---

# Phase 20 Plan 12: Historical source availability superseded; fresh acquisition halted

**Candidate 0039 had verified availability, but the current afba1d0 source has no fresh availability handoff after its required Nightly vector-fetch job failed.**

## Current Status — Halted

The successful delivery facts below apply only to historical candidate
`0039b9cd47a00d14d2f2f62376b8dbb2ad91baaf`; they are superseded for the current
source. Candidate `afba1d091bba916a9fe803f71e18788343a16219` is now the open
draft PR #59 head on `gsd/phase-20-nmi-line-and-vblank-flag` against `main`.
Its exact-head CI run `40580594574/1` completed successfully with all 16 jobs
passing.

Nightly run `40580594558` was started for that same PR event. Attempt 1 failed
the required `vectors-full` lane; the one already-authorized all-jobs rerun,
attempt 2, failed job `122471173432` at `cpu.vectors-full.fetch`. Its log says
the promisor fetch transferred less than 1000 bytes/second for 60 seconds,
then reported a bad pack header while fetching
`e95579991e0ab08ee8fe79b57bcae7f71a2b9d02`. The captured log is
`build/phase20-hosted-evidence-diagnostics/nightly-vectors-full-40580594558-122471173432.log`
(SHA-256 `229465ea49188322d29e826dd5b662ea155ac79f32461725d314989357bec2e7`).
At the last metadata read, attempt 2 remained in progress: `mmc3-oracle`,
`rom-loader-fuzz`, `state-fuzz`, and `execution-fuzz` had passed;
`suite-flake` and `session-controls` were still running; `vectors-full` had
failed. The failed required job is sufficient to halt acquisition; no further
rerun was triggered.

There is no completed successful Nightly attempt for candidate `afba1d0`, so
the metadata-only availability gate was not run. `delivery.json` and
`availability/availability.txt` still describe only historical candidate
`0039`; they are not valid inputs for 20-10. The plan is halted and 20-10 is
dependency-blocked. Hosted content acceptance, requirements, and Phase 20
completion remain pending.

## Historical Performance (candidate 0039 delivery execution)

These metrics and task commits describe the original successful 0039
execution. The resumed afba1d0 acquisition is recorded above; it ended halted
without a new plan task commit.

- **Duration:** 232 minutes
- **Started:** 2026-10-27T16:11:28Z (approximate; the plan ledger base commit timestamp)
- **Completed:** 2026-10-27T20:03:43Z
- **Tasks:** 2
- **Files modified:** 12, excluding the preserved pre-existing `.planning/config.json` edit

## Historical Accomplishments (candidate 0039 only)

- Published candidate `0039b9cd47a00d14d2f2f62376b8dbb2ad91baaf` through the existing open draft PR #59 using one normal fast-forward push. The original supplied `3cdf753` identity maps to its formatted equivalent `56a31aa4b76fab4bebc019886fc1b4c5c21528c4`; the commit map preserves the relationship. Formatting and hygiene hooks passed.
- CI `40571402181/1` and Nightly `40571401669/1` completed successfully for that exact candidate and PR event. The final effective source digest is `99691ce668e69ac193d493ed30fe422f872d9538ef3bfe28c75a018b05ccad0e` over 416 files. The synthetic tested merge is `32076c451f9f16d76d3e0e8f8ef721a2ee5050f6`, with protected base `ab9c2e6eb822c06f0dfb0f08a0ca42ec541358be`.
- The live `MODE=availability` gate passed. `delivery.json` selects only the fresh run attempts; `availability/availability.txt` records the exact source/run/job/artifact mapping and metadata snapshots. All required CI matrix jobs, policy/performance/real-host jobs, Nightly vector/MMC3 lanes, and their required unique, unexpired, fresh artifacts passed validation.
- Final local focused collector tests passed 3/3. Final `cmake --workflow --preset ci` passed 686/686 in 371.20 seconds and packaged the library, runner, and libretro adapter. No artifact content was downloaded or accepted; 20-10 owns those checks.

## Historical Task Commits

**Task 1: Publish the gated repair through PR 59 and acquire automatic runs**

- `f256e747` — record repaired-source publication and run candidates
- `c3a64f0` — allow MSVC secure CRT for the evidence helper
- `8a2e101` — refresh measured provenance after the MSVC fix
- `8f0504f` — handle CRLF fake GitHub routes
- `c2d34f6` — diagnose Windows fixture route paths
- `1d7cdb6` — batch Windows adapter fixture setup
- `7b0ba99` — batch hosted tree hashes and preserve manifest checksums
- `221916c` — preserve symlink blob identity in source checks
- `0039b9c` — preserve raw bytes in fixture tree checks

**Task 2: Acquire completed fresh CI/Nightly facts and replace the delivery input**

- `3652f8f` — record final hosted availability

## Files Created/Modified

- `.planning/phases/20-nmi-line-and-vblank-flag/20-DELIVERY.md` — final source/run history, required job and artifact identities, and the live availability record hashes.
- `tests/cmake/phase20_hosted_evidence.cmake` — batched exact raw blob identity checks with correct symlink mode handling.
- `tests/cmake/phase20_hosted_evidence_controls.cmake` — Windows byte-preservation, CRLF, and real Git symlink controls.
- `tests/runner/phase20_fake_gh.c` — robust CRLF route parsing and fixture diagnostics.
- `README.md`, `tests/CMakeLists.txt`, and AccuracyCoin metadata files — collector contract, controls, and measured source metadata.
- `build/phase20-hosted-evidence/delivery.json` and `availability/availability.txt` — ignored local handoff inputs and proof (not committed).

## Decisions Made

- Availability confirms only live metadata, exact source identity, required successful jobs, and artifact availability. Content validation and hosted acceptance remain pending in 20-10.
- Candidate, synthetic tested merge, protected base, and source digest stay separate identities. Old run attempts and failed collection artifacts remain historical.
- Keep all four Phase 20 requirements pending until 20-10 content acceptance and independent phase verification.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug / Rule 3 - Blocking] Repaired Windows fixture and Git tree identity controls**

- **Found during:** Task 1 hosted verification
- **Issue:** Windows jobs first exposed terminal CR bytes in route fixture paths, then CTest setup cost and restoration edge cases. The production tree batch check also dereferenced mode `120000` symlinks, and Windows fixture Git repositories normalized raw source bytes under inherited `eol=lf` attributes.
- **Fix:** Trim repeated route terminators, assert route response files before subprocess execution, reuse immutable fixtures and batch Git operations, restore exact file bytes, hash stored symlink-target blobs by Git mode, and disable text conversion only in synthetic Git metadata while asserting raw source blobs match `git ls-tree`.
- **Files modified:** `README.md`, `tests/CMakeLists.txt`, `tests/accuracy/baseline-metadata.txt`, `tests/accuracy/measurements.md`, `tests/cmake/phase20_hosted_evidence.cmake`, `tests/cmake/phase20_hosted_evidence_controls.cmake`, `tests/runner/phase20_fake_gh.c`.
- **Verification:** Focused controls 3/3; full local CI 686/686; final Windows x64 and ARM64 hosted builds and availability CTests passed. Test matrix and timeouts were retained.
- **Committed in:** `c2d34f6`, `1d7cdb6`, `7b0ba99`, `221916c`, `0039b9c` (with earlier route/MSVC commits listed above).

**Total deviations:** 1 grouped evidence-driven repair
**Impact on plan:** Fixed platform-specific delivery blockers within the authorized collector scope. No acceptance policy, timeout, workflow, or test-matrix relaxation.

## Issues Encountered

Earlier candidate attempts remain recorded as failed historical diagnostics and do not satisfy this handoff. The final candidate's exact PR-triggered CI and Nightly attempts both passed, so no retry or workflow dispatch was needed. No new authorization gate occurred.

## User Setup Required

None.

## Next Phase Readiness

Plan 20-10 cannot consume the historical 0039 handoff for current candidate
`afba1d0`. Fresh successful Nightly availability has not been established; the
single permitted rerun failed its required full-vector fetch, so the plan is
halted and 20-10 is blocked by its dependency. The existing branch remains on
open draft PR #59. No additional retry, workflow dispatch, merge, or source
change occurred in this closeout.

## Historical Self-Check (0039 candidate): PASSED

- The original 0039 summary recorded ten plan task commits and the matching historical availability record.
- The `.planning/config.json` edit remained unchanged and unstaged.

## Current Self-Check: HALTED

- Candidate `afba1d0` CI passed, but its only authorized Nightly rerun failed the required `vectors-full` fetch.
- No current-source `delivery.json` handoff or `MODE=availability` record exists; the existing 0039 files remain historical.
- Plan 20-12 is halted and plan 20-10 is blocked; no completion or acceptance is claimed.

---
*Phase: 20-nmi-line-and-vblank-flag*
*Completed: 2026-10-27*
