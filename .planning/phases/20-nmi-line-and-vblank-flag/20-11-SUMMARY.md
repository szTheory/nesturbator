---
phase: 20-nmi-line-and-vblank-flag
plan: "11"
subsystem: testing
tags: [cmake, ctest, github-actions, gh-cli, accuracycoin]

# Dependency graph
requires:
  - phase: 20-09
    provides: Existing hosted evidence collector, retention contract, and source validators
provides:
  - Production collection adapter for authentic root and nested archives and executed per-job results
  - Metadata-only source-bound availability mode with offline positive and stage-specific negative controls
  - Refreshed AccuracyCoin campaign metadata and local source identity for fresh delivery
affects: [20-12, 20-10, hosted-evidence, accuracycoin]

# Actuals (#2632), measured over the realized plan diff before summary/state commits.
actuals:
  tokens: 29031
  tasks: 2
  commits: 3
plan_head_before: 282ae2fd0121bb6beac606d772ac4078eda50075
plan_head_after: 688dd103c2637808fe8c6014f528ff5b64fd8fa9

# Tech tracking
tech-stack:
  added: [offline fake-gh test executable]
  patterns:
    - Shared metadata acquisition and source/run/job/artifact validation for collect and availability
    - Production-entrypoint CTest controls with exact stage-specific refusal assertions

key-files:
  created:
    - tests/runner/phase20_fake_gh.c
  modified:
    - tests/cmake/phase20_hosted_evidence.cmake
    - tests/cmake/phase20_hosted_evidence_controls.cmake
    - tests/CMakeLists.txt
    - README.md
    - tests/accuracy/baseline-metadata.txt
    - tests/accuracy/measurements.md
    - .planning/phases/20-nmi-line-and-vblank-flag/20-EVIDENCE.md

key-decisions:
  - "Availability remains a metadata-only snapshot and cannot create hosted content-acceptance outputs."
  - "Require actual per-job workflow execution results and retain raw archive/log bytes alongside normalized mappings."
  - "Treat local dirty campaign results as preparation; keep all four Phase 20 requirements pending fresh hosted evidence and independent verification."

patterns-established:
  - "Availability and collection share the existing source/run/job/artifact validators instead of duplicating acceptance rules."
  - "Fake-gh integration tests exercise the production script offline and fail immediately on unexpected endpoints."

requirements-completed: []
coverage:
  - id: D1
    description: "Production collection and metadata-only availability are covered by the offline fake-gh adapter and stage-specific refusal controls."
    verification:
      - kind: integration
        ref: "phase20.hosted_evidence.adapter and phase20.hosted_evidence.availability"
        status: pass
    human_judgment: false
  - id: D2
    description: "Refreshed local campaign and effective source identity are ready for new hosted delivery."
    verification: []
    human_judgment: true
    rationale: "Local fixtures and dirty-source measurements cannot establish fresh hosted exact-head CI/Nightly acceptance."

# Metrics
duration: 47m
completed: 2026-10-27
status: complete
---

# Phase 20 Plan 11: Collector Repair Summary

**Hosted evidence collection now validates authentic archives, executed per-job results, and source-bound availability, with measured local preparation for fresh delivery.**

## Performance

- **Duration:** 47 minutes
- **Started:** 2026-10-27T14:48:41Z
- **Completed:** 2026-10-27T15:35:28Z
- **Tasks:** 2
- **Files modified:** 9

## Accomplishments

- Fixed the production collector's supported `gh api` invocations and archive normalization for authentic root and single-prefix ZIP layouts. Collection preserves raw bytes and maps normalized paths separately.
- Validated exact per-job workflow execution logs, including the exact Holy Mapperel M2 test name, and added metadata-only `MODE=availability` through shared source/run/job/artifact checks. Availability emits only its own record and never fetches logs or archives.
- Refreshed one full-enabled local AccuracyCoin campaign and recorded the new effective source digest. All six Phase 20 target rows passed; protected-key checks and the full local CI workflow passed.

## Task Commits

Each task was committed atomically:

1. **Collect a realistic six-lane archive and executed-result bundle through the production entrypoint** — `d111e55` (RED test), `ac8456b` (implementation and green controls).
2. **Measure and gate the repaired local source before fresh delivery** — `688dd10` (measured baseline and evidence).

The summary and execution-state/roadmap metadata are committed separately after this file is self-checked.

## Files Created/Modified

- `tests/cmake/phase20_hosted_evidence.cmake` — shared production collect and availability metadata checks, actual workflow-log validation, and safe evidence mapping.
- `tests/cmake/phase20_hosted_evidence_controls.cmake` — offline exact-reason negative matrix and full positive collector/availability scenarios.
- `tests/runner/phase20_fake_gh.c` — fake GitHub CLI with binary archive support and strict endpoint controls.
- `tests/CMakeLists.txt` — registered collector adapter and availability tests.
- `README.md` — documents supported collection, raw versus normalized evidence, and availability semantics.
- `tests/accuracy/baseline-metadata.txt` — measured campaign and fingerprint provenance.
- `tests/accuracy/measurements.md` and `.planning/phases/20-nmi-line-and-vblank-flag/20-EVIDENCE.md` — local campaign, source identity, and pending hosted evidence.

## Verification

- **TDD RED:** `phase20.hosted_evidence.adapter` failed before implementation on unsupported `gh api --repo` usage and the invalid case-insensitive CMake regex. The captured evidence at `build/ci/Testing/Temporary/20-11-red-evidence.json` was checked as `RED_EVIDENCE_OK`.
- **Task 1 GREEN:** the plan's collector gate passed 28/28, including controls, the production adapter, availability, hash inventory, performance gates, and no-skip policy.
- **Task 2 focused gate:** passed 12/12, including all six target rows, the cold protected-main observation, all 108 protected PASS keys, no-lost-pass, native hashes, collector controls, and CI/Nightly policy checks.
- **Full local campaign:** 22/22 pages, 146 scored rows (114 PASS, 32 FAIL, 0 skip/not-run/timeout, 5 DRAW); all six Phase 20 target rows passed. `source_revision=ac8456b60272`, `source_dirty=1`, `host_elapsed_guest_ns=5017232000`, 56-input fingerprint `b2cdcdd15785b590a415dac911187b8abfa0d025a1cc448d412650d1716f1240`, and report SHA-256 `29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`.
- **Effective source identity:** `b07b42ac26cd92f698eec7a643bc77d5c3c4193ae9432b6f80032affdfabdede` over 416 files. Planning files and `tests/accuracy/measurements.md` are excluded by the shared digest contract.
- **Full workflow:** `cmake --workflow --preset ci` passed 686/686 tests in 394.85 seconds and generated the library, runner, and libretro packages.
- `git diff --check` passed. The pre-existing `.planning/config.json` edit was preserved and remained unstaged; the orchestrator's dirty `.planning/STATE.md` edit was not staged with either task commit.

### Named refusal coverage

| Path | Negative inputs asserted at their intended reason |
|---|---|
| Collection | Root-plus-nested duplicates, duplicate members, unsafe entries, missing content, empty or registration-only logs, missing/skipped/failed exact tests, wrong lane/job/run/attempt, substituted near-match M2 name, ambiguous step logs, stale-success retry, missing/colliding performance or Nightly files, invalid campaign path, authentication and other CLI errors. |
| Availability | Failed/incomplete/wrong-workflow runs; missing, duplicate, substituted, or wrong-attempt matrix jobs; missing, expired, duplicate, wrong-run, old-attempt, or freshness-less artifacts; incomplete inventories; wrong candidate/base/merge parents; truncated or changed source trees; local source drift; authentication/CLI errors; stale-success retry. Unexpected archive/log requests fail immediately, and availability failure leaves no success record or content-acceptance outputs. |

The positive collection fixtures cover authentic root and nested archive layouts. All offline controls use the fake CLI and make no real network calls.

## Decisions Made

- Availability is an early source-bound metadata snapshot; only collection and content validation can produce acceptance evidence.
- Preserve raw archives and logs and record their normalized mappings rather than extracting over provenance.
- Keep the prior successful CI `40514060876/1` and Nightly `40513950000/1` as historical availability for candidate `e6e83a31e413e12abe9dee142e3c0f7143d63eb4` / merge `11533e456d41f740751c8353f947d65017aae692` and digest `de6d4355222063e019c7a3a94e30a404b52041f3d994659d445f12b4a23318ab`. They do not certify the repaired effective source. The prior failed collection remains diagnostic; there is no new hosted acceptance manifest.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Serialized controls that temporarily mutate source inputs**
- **Found during:** Task 1
- **Issue:** Source-drift controls deliberately alter fixture inputs while running; concurrent execution could make adjacent controls observe another case's temporary state.
- **Fix:** Marked the related collector control test `RUN_SERIAL` so its intended refusal reasons remain deterministic.
- **Files modified:** `tests/CMakeLists.txt`
- **Verification:** The 28-test Task 1 gate, 12-test Task 2 gate, and full 686-test workflow passed.
- **Committed in:** `ac8456b`

**Total deviations:** 1 auto-fixed (Rule 3 - Blocking)
**Impact on plan:** Serialization protects the offline matrix from test interference; the collector, workflow policy, thresholds, runtime/API, ROMs, and native hashes remain unchanged.

## Issues Encountered

The required full CI workflow includes a long `retroarch.session.controls` integration test. It passed in 247.22 seconds; the overall workflow completed successfully.

## User Setup Required

None - no external service configuration is required for the offline tests.

## Next Phase Readiness

The local collector repair and evidence handoff are ready for Plan 20-12. Fresh source-bound hosted CI/Nightly evidence and later content acceptance remain outstanding. All four Phase 20 requirements, KEEP-02/KEEP-03, and phase completion stay pending independent verification. No Plan 20-10 or 20-12 execution occurred in this plan.

---
*Phase: 20-nmi-line-and-vblank-flag*
*Completed: 2026-10-27*

## Self-Check: PASSED

- Summary and primary created artifacts exist.
- Task commits `d111e55`, `ac8456b`, and `688dd10` are ancestors of the current branch head.
- `git diff --check` passed.
