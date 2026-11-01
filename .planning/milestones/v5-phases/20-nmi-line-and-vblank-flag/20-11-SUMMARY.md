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
  - Production collection adapter for authentic archives and timestamp-bounded per-job plain-text results
  - Metadata-only source-bound availability mode with offline positive and stage-specific negative controls
  - Refreshed AccuracyCoin campaign metadata and local source identity for fresh delivery
affects: [20-12, 20-10, hosted-evidence, accuracycoin]

# Actuals (#2632), measured as chars/4 over the realized plan diff, excluding the preserved config edit.
actuals:
  tokens: 44657
  tasks: 2
  commits: 8
plan_head_before: 282ae2fd0121bb6beac606d772ac4078eda50075
plan_head_after: 151732834c06a4b2042e43dd16bc63920d91176d

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
    - .planning/STATE.md
    - .planning/ROADMAP.md

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
duration: 69m
completed: 2026-10-27
status: complete
---

# Phase 20 Plan 11: Collector Repair Summary

**Hosted collection preserves raw timestamped job-log text and validates CTests only in the selected successful step, alongside source-bound availability and fresh local measurements.**

## Performance

- **Duration:** 69 minutes cumulative active work (initial execution plus post-review correction)
- **Started:** 2026-10-27T14:48:41Z
- **Completed:** 2026-10-27T16:08:58Z
- **Tasks:** 2
- **Files modified:** 11

## Accomplishments

- Fixed the production collector's supported `gh api` invocations and archive normalization for authentic root and single-prefix ZIP layouts; artifact ZIP bytes and normalized mappings remain separately verifiable.
- Corrected the job-log assumption after review: GitHub returns a BOM-prefixed timestamped plain-text job log. The collector preserves raw bytes, binds the raw SHA to selected job/run/attempt/step metadata, derives only the unique selected step inside its timestamp bounds, and re-derives that log during validation. Unsupported old `gh` versions fail before raw requests.
- Validated the exact Holy Mapperel M2 test name and all six required terminal CTest results, and added metadata-only `MODE=availability` through shared source/run/job/artifact checks. Availability emits only its own record and never fetches logs or archives.
- Remeasured the final local source and passed the focused protected-key checks and full local CI; all six Phase 20 target rows remained passing. Fresh hosted evidence remains required.

## Task Commits

Each task was committed atomically:

1. **Collect a realistic six-lane archive and executed-result bundle through the production entrypoint** — `d111e55` and `ac8456b` (initial RED/implementation), then `8202159` (plain-text RED) and `f3b56e0` (corrected collector and green controls).
2. **Measure and gate the repaired local source before fresh delivery** — `688dd10` (initial measured baseline), then `1517328` (corrected-source measurements and evidence).

The summary and execution-state/roadmap metadata are committed separately after this file is self-checked.

## Files Created/Modified

- `tests/cmake/phase20_hosted_evidence.cmake` — shared production collect and availability metadata checks, actual workflow-log validation, and safe evidence mapping.
- `tests/cmake/phase20_hosted_evidence_controls.cmake` — offline exact-reason negative matrix and full positive collector/availability scenarios.
- `tests/runner/phase20_fake_gh.c` — fake GitHub CLI with binary artifact ZIP and raw plain-text response support, strict endpoints, and escape-option behavior.
- `tests/CMakeLists.txt` — registered collector adapter and availability tests.
- `README.md` — documents supported collection, raw versus normalized evidence, and availability semantics.
- `tests/accuracy/baseline-metadata.txt` — measured campaign and fingerprint provenance.
- `tests/accuracy/measurements.md` and `.planning/phases/20-nmi-line-and-vblank-flag/20-EVIDENCE.md` — local campaign, source identity, and pending hosted evidence.

## Verification

- **Initial TDD RED:** `d111e55` failed on unsupported `gh api --repo` usage and the invalid case-insensitive CMake regex. The correction's realistic BOM/CRLF/ANSI plain-text fixture then failed before its fix with `Unrecognized archive format`, confirming the measured response-shape defect.
- **Task 1 GREEN:** the final collector gate passed 28/28, including controls, production collect, metadata-only availability, hash inventory, performance gates and no-skip policy. Named log controls reject outside-step-only passes, missing/near-match/skipped/failed/duplicate/contradictory terminal results; the old-CLI case rejects before raw download.
- **Task 2 focused gate:** passed 12/12 after full mode was disabled, including current campaign activation, the cold protected-main observation, all 108 protected PASS keys, no-lost-pass, exact six targets, native hashes, collector controls and CI/Nightly policy checks.
- **Full local campaign:** two full-enabled runs completed 22/22 pages and 146 scored rows (114 PASS, 32 FAIL, 0 skip/not-run/timeout, 5 DRAW); all six Phase 20 target rows passed. The second was an unintended registered-fixture rerun before disabling full mode; it matched the first report and fingerprint, and supplies the final baseline metadata. Recorded metadata has `source_revision=f3b56e003dc8`, `source_dirty=1` for the preserved config edit, `host_elapsed_guest_ns=4768711000`, 56-input fingerprint `b2cdcdd15785b590a415dac911187b8abfa0d025a1cc448d412650d1716f1240`, and report SHA-256 `29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`.
- **Effective source identity:** `d1d4a456b40332a78678fbcf1ba1bf948a82641f20dfec5ecafbe0f2f678c69e` over 416 files after refreshing baseline metadata. Planning files and `tests/accuracy/measurements.md` are excluded by the shared digest contract.
- **Full workflow:** `cmake --workflow --preset ci` passed 686/686 tests in 389.05 seconds and generated the macOS-arm64 library, runner, and libretro packages.
- `git diff --check` passed. The pre-existing `.planning/config.json` edit was preserved and remained unstaged; execution tracking updates were committed separately with the final plan metadata.

### Named refusal coverage

| Path | Negative inputs asserted at their intended reason |
|---|---|
| Collection | Root-plus-nested duplicates, duplicate members, unsafe entries, missing content, empty or registration-only logs, missing/skipped/failed/duplicate/contradictory exact results, outside-step-only output, wrong lane/job/run/attempt, substituted near-match M2 name, duplicate step groups, stale-success retry, missing/colliding performance or Nightly files, invalid campaign path, authentication and other CLI errors. |
| Availability | Failed/incomplete/wrong-workflow runs; missing, duplicate, substituted, or wrong-attempt matrix jobs; missing, expired, duplicate, wrong-run, old-attempt, or freshness-less artifacts; incomplete inventories; wrong candidate/base/merge parents; truncated or changed source trees; local source drift; authentication/CLI errors; stale-success retry. Unexpected archive/log requests fail immediately, and availability failure leaves no success record or content-acceptance outputs. |

The positive collection fixtures cover authentic root and nested archive layouts. All offline controls use the fake CLI and make no real network calls.

## Decisions Made

- Availability is an early source-bound metadata snapshot; only collection and content validation can produce acceptance evidence.
- Preserve raw artifact archives and job logs byte for byte; derive timestamp-bounded job-step output and record separate raw/derived digests.
- Use the locally resolved GitHub CLI consistently and require its documented escape-sequence option before requesting raw log or ZIP bytes.
- Keep the prior successful CI `40514060876/1` and Nightly `40513950000/1` as historical availability for candidate `e6e83a31e413e12abe9dee142e3c0f7143d63eb4` / merge `11533e456d41f740751c8353f947d65017aae692` and digest `de6d4355222063e019c7a3a94e30a404b52041f3d994659d445f12b4a23318ab`. They do not certify the repaired effective source. The prior failed collection remains diagnostic; there is no new hosted acceptance manifest.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Serialized controls that temporarily mutate source inputs**
- **Found during:** Task 1
- **Issue:** Source-drift controls deliberately alter fixture inputs while running; concurrent execution could make adjacent controls observe another case's temporary state.
- **Fix:** Marked the related collector control test `RUN_SERIAL` so its intended refusal reasons remain deterministic.
- **Files modified:** `tests/CMakeLists.txt`
- **Verification:** The initial 28-test Task 1 gate, 12-test Task 2 gate, and full workflow passed.
- **Committed in:** `ac8456b`

**2. [Rule 1 - Bug] Parse real GitHub per-job logs as timestamped plain text**
- **Found during:** Post-review correction before Plan 20-12
- **Issue:** The Actions job `/logs` endpoint returns a UTF-8-BOM timestamped text log; the collector treated it as a ZIP, so the prior fake positive did not model hosted behavior.
- **Fix:** Preserve and hash the raw response, feature-detect/pass `--allow-escape-sequences`, bind the unique expected workflow-step group to the selected job metadata time bounds, and derive/revalidate the exact step log before parsing CTest terminals.
- **Files modified:** `tests/cmake/phase20_hosted_evidence.cmake`, `tests/cmake/phase20_hosted_evidence_controls.cmake`, `tests/runner/phase20_fake_gh.c`, `README.md`
- **Verification:** Plain-text RED failed with `Unrecognized archive format`; final 28/28 gate passed. The captured historical job log also passed the exact parser as data-only shape verification.
- **Committed in:** `8202159` (RED), `f3b56e0` (fix)

**Total deviations:** 2 auto-fixed (Rule 3 - Blocking; Rule 1 - Bug)
**Impact on plan:** Serialization protects the offline matrix from test interference; raw job-log handling now matches the measured API response. Workflow policy, thresholds, runtime/API, ROMs, and native hashes remain unchanged.

## Issues Encountered

The required full CI workflow includes long RetroArch isolated-session controls. `retroarch.session.controls` passed in 249.04 seconds, and the overall final-source workflow passed 686/686 in 389.05 seconds.

## User Setup Required

None - no external service configuration is required for the offline tests.

## Next Phase Readiness

The local collector repair and evidence handoff are ready for Plan 20-12. Execution tracking is 10/12 plans complete; the remaining order is 20-12 then 20-10. Fresh source-bound hosted CI/Nightly evidence and later content acceptance remain outstanding. All four Phase 20 requirements, KEEP-02/KEEP-03, and phase completion stay pending independent verification. No Plan 20-10 or 20-12 execution occurred in this plan.

---
*Phase: 20-nmi-line-and-vblank-flag*
*Completed: 2026-10-27*

## Self-Check: PASSED

- Summary and primary created artifacts exist.
- Plan commits `d111e55`, `ac8456b`, `688dd10`, `57515b3`, `56a7477`, `8202159`, `f3b56e0`, and `1517328` are ancestors of the current branch head.
- `git diff --check` passed.
