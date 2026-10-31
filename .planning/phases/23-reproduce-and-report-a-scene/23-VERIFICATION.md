---
phase: 23-reproduce-and-report-a-scene
verified: 2026-10-31T04:56:27Z
status: passed
score: 6/6 must-haves verified
covered_files:
  - .planning/phases/23-reproduce-and-report-a-scene/23-01-PLAN.md
  - .planning/phases/23-reproduce-and-report-a-scene/23-01-SUMMARY.md
  - .planning/phases/23-reproduce-and-report-a-scene/23-02-PLAN.md
  - .planning/phases/23-reproduce-and-report-a-scene/23-02-SUMMARY.md
  - .planning/phases/23-reproduce-and-report-a-scene/23-03-PLAN.md
  - .planning/phases/23-reproduce-and-report-a-scene/23-03-SUMMARY.md
  - .planning/phases/23-reproduce-and-report-a-scene/23-EXECUTION-EVIDENCE.md
  - .planning/phases/23-reproduce-and-report-a-scene/23-REVIEW-DISPOSITION.md
  - .planning/phases/23-reproduce-and-report-a-scene/23-REVIEW.md
  - README.md
  - docs/progress.md
  - docs/reproduction.md
  - docs/runner.md
  - docs/testing.md
  - tests/CMakeLists.txt
  - tests/cmake/reproduction.cmake
  - tests/cmake/reproduction_contract.cmake
covered_digest: "v4:sha256:eeae4d642555e3e78b34b66531cfbf88f9f62ed1c8b90c8d22867e14b90d5afb"
behavior_unverified: 0
overrides_applied: 0
decision_coverage:
  honored: 22
  total: 22
  not_honored: []
prohibition_review:
  mode: scoped_source_review
  findings: none_in_phase_artifacts
  items:
    - statement: "MUST NOT publish commercial media, saves, checkpoints, raw private logs/captures or personal identifiers as scene-report evidence."
      verification: judgment
      result: no_violation_found_in_scope
    - statement: "MUST NOT imply a scene was reached or an identity is known when access or identity is unavailable."
      verification: judgment
      result: no_violation_found_in_scope
    - statement: "MUST NOT represent movie-plus-load-state, host recording or foreign movie conversion as supported runner capabilities."
      verification: judgment
      result: no_violation_found_in_scope
    - statement: "MUST NOT label repeated frame/audio hashes as independent hardware correctness."
      verification: judgment
      result: no_violation_found_in_scope
    - statement: "MUST NOT read or write daily progress or silently substitute an unknown, mismatched or host-container checkpoint."
      verification: judgment
      result: no_violation_found_in_scope
    - statement: "MUST NOT report timeout, interruption, truncation or missing required output as successful replay."
      verification: judgment
      result: no_violation_found_in_scope
    - statement: "MUST NOT relabel commercial reports as diagnosed or fixed or equate native/adapter parity with host presentation."
      verification: judgment
      result: no_violation_found_in_scope
    - statement: "MUST NOT launch live hosts/devices, commercial replay, collector campaigns, publication or emulator-source inspection under this phase."
      verification: judgment
      result: no_violation_found_in_scope
  limits:
    - "These are scoped source-review judgments over the changed artifacts and recorded phase evidence, not mechanical proofs or owner acceptance."
    - "The execution record reports no live, device, commercial, campaign or publication work; this review makes no claim about activity outside that recorded scope."
---

# Phase 23: Reproduce and report a scene — Verification Report

**Phase Goal:** A developer can repeat a precisely reported scene without disturbing daily progress and choose the next evidence source without overclaiming a diagnosis.
**Verified:** 2026-10-31T04:56:27Z
**Status:** passed
**Re-verification:** No — initial verification

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | A player can reach a sanitized report form and completed permitted-fixture example that distinguishes exact-scene access, identity, expected-behavior provenance, repeatability and unknown conditions. | ✓ VERIFIED | [docs/reproduction.md](../../../docs/reproduction.md#report-a-scene) contains all report fields, permits unknown/withheld/not-tried/unavailable values, distinguishes title-level observations and exact-scene access, and keeps private artifacts local. [README.md](../../../README.md:120) links directly to the guide. The permitted fixture digest was independently recomputed and matches `tests/roms/manifest.txt`; the report identities and output hashes are source-bound in the execution record. |
| 2 | A developer can repeat the permitted 600-frame cold movie from the public runner and compare complete native frame and audio output, while retaining independent input controls. | ✓ VERIFIED | [tests/cmake/reproduction.cmake](../../../tests/cmake/reproduction.cmake:276) generates a fresh movie, invokes the public runner twice, requires 600 ordered frame records plus transition and PCM hashes, and rejects incomplete/stopped output. `runner.reproduction.cold` passed in this verification (1.80 s); `runner.reproduction.compare_self_test` passed (0.23 s). `runner.boot_to_play` and its four movie fixtures passed (2.73 s). |
| 3 | A developer can use a separate neutral native checkpoint route, verify exact recorded runner/content/state digests before claiming support, and detect wrong or omitted continuation. | ✓ VERIFIED | [docs/reproduction.md](../../../docs/reproduction.md#native-checkpoint-continuation) specifies K=1 through absolute N=4 and compares the audio suffix at frames 2–4. The preflight computes file digests and stops unknown, changed or host-container cases before replay; current `runner.reproduction.preflight`, `runner.reproduction.preflight_self_test`, `runner.state.nmi_roundtrip`, `runner.state.nmi_idempotent` and `runner.state.nmi_wrong_load` runs passed. The guide explicitly says the caller-declared origin is not authentication and that loader build/revision checks are separate. |
| 4 | Every documented replay has finite frame/time bounds and isolated scratch storage, with unsupported identity/container cases and incomplete output handled explicitly without touching daily progress. | ✓ VERIFIED | [docs/reproduction.md](../../../docs/reproduction.md#scratch-storage-and-bounds) lists frame, child-process and CTest limits; [tests/CMakeLists.txt](../../../tests/CMakeLists.txt:1618) registers them. The scripts use fresh build scratch and no host save/config directory. Review fixes close the POSIX `mktemp` failure path and check PowerShell native exits. Earlier post-review evidence records injected failure checks and a successful exact fresh-scratch route. No ROM/movie/state artifact was added to the tracked changes. |
| 5 | A developer can choose the next bounded evidence source while keeping native output, adapter conversion, host presentation and original-game correctness claims separate. | ✓ VERIFIED | [docs/reproduction.md](../../../docs/reproduction.md#choose-the-next-evidence-source) names each comparison, inference limit, unknown and next evidence source. It requires a claim-specific independent oracle for correctness, treats screenshots qualitatively without aligned transforms, and keeps the cited commercial observations unclassified. The contract mutations target the false hash, screenshot, host-pacing and commercial-status claims. |
| 6 | README and topic guides reach one maintained reproduction guide, and executable controls protect its claims and the phase evidence distinguishes verified work from untried scope. | ✓ VERIFIED | README, runner, testing and progress guides link to `docs/reproduction.md`; the current `docs.links` and `docs.links.self_test` checks passed. The document contract checks generated CTest names, targets, fixture dependencies and timeouts. The execution record distinguishes earlier measured fixture results from Plan 03 checks and records platform and scope limits. **Local test limitation:** both current attempts to run the document contract timed out; details and earlier post-review successful reruns are recorded below. |

**Score:** 6/6 truths verified (0 present, behavior-unverified)

The six truths consolidate the four non-negotiable roadmap criteria with the additive plan details. The three plan-specific CMake artifacts and all declared links were checked separately below.

### Judgment-Only Prohibition Review

All eight plan prohibitions were explicitly source-reviewed; none is silently counted as a test result. Their plans classify them as `verification: judgment`, and the execution evidence retains that verification limit. In the changed documentation, scripts, CTest registrations and phase evidence, I found no violating claim, artifact or authorized operation. This is a scoped review of the supplied source and phase record, not proof of unseen external actions or an owner acceptance.

| Plan prohibition | Scoped source-review result | Evidence and retained limit |
|---|---|---|
| No commercial media, saves, checkpoints, raw private logs/captures or personal identifiers in public scene-report evidence. | No violation found. | The guide prohibits these artifacts; tracked phase changes are documentation and CMake only. The permitted ROM is manifest-listed; no new binary artifact is tracked. This does not attest to material outside the repository. |
| No claim that an unavailable scene or identity was reached or known. | No violation found. | The form allows explicit unknown/unavailable values; the worked example is a permitted test fixture, not a commercial scene report. |
| No claim that movie-plus-load-state, host recording or foreign movie conversion is supported. | No violation found. | The guide labels these routes unsupported; the test wrapper only reuses the owned NMOVIE1 generator and native-state fixture. |
| Repeated hashes are not independent hardware correctness. | No violation found. | Guide and comparator contract separate repeatability from correctness; an independent oracle is required for correctness. |
| Daily progress is not read/written and an unknown/mismatched/host-container checkpoint is not silently substituted. | No violation found. | Examples use build/scratch paths; CTest has no daily save path; preflight checks the three recorded digests and origin before replay. This review is scoped to those paths and commands. |
| Timeout, interruption, truncation or missing output is not reported as successful replay. | No violation found. | The wrapper checks child status/stopped output and the full ordered frame/audio records. The current CTest timeouts are retained as failures, not passed replays. |
| Commercial reports are not diagnosed/fixed, and adapter parity is not equated with host presentation. | No violation found. | The guide keeps commercial observations unclassified and states the host/device evidence limits; five contract mutations target those claims. |
| No live host/device, commercial replay, collector campaign, publication or emulator-source inspection under this phase. | No violation found in the scoped phase record. | The execution record says these did not run and this verification used local bounded CTests only. No push/PR/release or campaign was started. This is a session-record judgment, not an independently observable guarantee about activity outside that record. |

## Required Artifacts

| Artifact | Expected | Status | Details |
|---|---|---|---|
| `docs/reproduction.md` | Canonical report/replay/safety/triage guide and oracle table | ✓ VERIFIED | 280 lines; full report form, fixture example, replay paths, bounds, preflight, triage and correctness limits; linked from all four entry guides. |
| `tests/cmake/reproduction.cmake` | Bounded cold replay, output comparison and checkpoint preflight | ✓ VERIFIED | 501 lines; substantive CMake implementation. CTest invokes it for cold, comparator and preflight checks; current named checks passed. |
| `tests/cmake/reproduction_contract.cmake` | Scoped documentation/command assertions and mutation controls | ✓ VERIFIED | 603 lines; verifies report, route, bounds, target/test metadata, guide links and negative claims. CTest registers both check and self-test. The latest local attempts timed out; earlier post-review runs are preserved in the evidence record. |
| `docs/progress.md` | Link to reproduction guide and separate battery backup policy | ✓ VERIFIED | Direct guide link and existing independent-backup guidance; no replay path points to daily storage. |
| `tests/CMakeLists.txt` | CTest registrations with finite bounds/fixture setup | ✓ VERIFIED | Registers cold 180 s, comparator/preflight 60 s, document checks 60 s, NMI cases 60 s, and fixture/generator bounds. |
| `README.md`, `docs/runner.md`, `docs/testing.md` | Direct route to the canonical guide | ✓ VERIFIED | README and all topic guides link to the same maintained document; guide-link CTests passed. |
| `23-EXECUTION-EVIDENCE.md` | Sanitized actual commands/results and evidence limits | ✓ VERIFIED | Distinguishes inherited and current results, lists fixture/build identities, comparison windows, timeouts, platform limits and untried host/device paths. |

GSD `verify.artifacts` returned `all_passed: true` for each of the three plans (3/3 artifacts each). The generic key-link query did not recognize some CMake `-P` relationships and links represented by CTest names; those were traced manually below.

## Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| `README.md` | `docs/reproduction.md` | Troubleshooting link | ✓ WIRED | Direct Markdown link at README line 120; reproduced by document contract and `docs.links`. |
| `tests/CMakeLists.txt` | `tests/cmake/reproduction.cmake` | `runner.reproduction.*` | ✓ WIRED | Each named CTest command uses `-P tests/cmake/reproduction.cmake` at lines 1620–1648. |
| `docs/reproduction.md` | `tests/runner/game_movie.c` | Generator source link and public build target | ✓ WIRED | Guide links the source and names `runner.game_movie`; CTest uses the target as `MOVIE_WRITER`. |
| `docs/reproduction.md` | `tests/cmake/runner_state.cmake` | Existing NMI test names | ✓ WIRED | Guide names `runner.state.nmi_roundtrip`, `nmi_idempotent` and `nmi_wrong_load`; CTest registration passes each through `runner_state_case()` to `-P runner_state.cmake`. |
| `docs/reproduction.md` | `docs/api.md` | Save-state format/build policy | ✓ WIRED | Direct save-state contract link; guide correctly distinguishes loader checks from exact-build policy. |
| `docs/reproduction.md` | `docs/progress.md` | Backup/container policy | ✓ WIRED | Direct progress-guide link and existing independent battery-backup guidance. |
| `docs/reproduction.md` | `docs/testing.md` | Conversion/callback evidence limits | ✓ WIRED | Direct guide link; triage names `host.convert` and `libretro.host` with their limits. |
| `docs/reproduction.md` | `.planning/playtesting/POSTMORTEM.md` | Existing commercial observations | ✓ WIRED | Direct dated-evidence link; observations remain explicitly unclassified. |
| `docs/runner.md`, `docs/testing.md`, `docs/progress.md` | `docs/reproduction.md` | Direct topic-guide links | ✓ WIRED | Each guide contains the direct Markdown link; `docs.links` checks passed. |

## Data-Flow Trace (Level 4)

There is no rendered UI or dynamic page in this documentation/CMake phase. The executable evidence path does consume real runtime data:

| Artifact | Data variable | Source | Produces real data | Status |
|---|---|---|---|---|
| `tests/cmake/reproduction.cmake` | `run_1_output`, `run_2_output` | Two actual invocations of the built public runner on a locally generated NMOVIE1 movie | Yes; current `runner.reproduction.cold` passed | ✓ FLOWING |
| `tests/cmake/reproduction.cmake` | recorded/current runner, ROM and state SHA-256 values | `file(SHA256)` over the actual runner executable and invocation-generated fixture/state | Yes; current positive preflight and negative controls passed | ✓ FLOWING |
| `docs/reproduction.md` | Permitted fixture digest and measured hashes | Manifest-listed `tests/roms/runner.nes` and the bounded run recorded in phase execution evidence | Yes; current ROM digest independently matches the manifest; replay check passed | ✓ FLOWING |

## Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|---|---|---|---|
| Two cold movies compare complete frame/transition/PCM output | `ctest --test-dir build/ci ... -R '^runner.reproduction.cold$'` | 1/1 passed, 1.80 s | ✓ PASS |
| Comparator rejects altered/incomplete outputs | `ctest --test-dir build/ci ... -R '^runner.reproduction.compare_self_test$'` | 1/1 passed, 0.23 s | ✓ PASS |
| Positive checkpoint identity preflight | `ctest --test-dir build/ci ... -R '^runner.reproduction.preflight$'` | 1/1 passed, 0.08 s | ✓ PASS |
| Missing/mismatched/unknown/host identity controls stop before replay | `ctest --test-dir build/ci ... -R '^runner.reproduction.preflight_self_test$'` | 1/1 passed, 0.06 s; seven negative cases | ✓ PASS |
| Neutral NMI checkpoint continuation and same audio suffix | `ctest --test-dir build/ci ... -R '^runner.state.nmi_roundtrip$'` | Save fixture plus test passed, 0.10 s | ✓ PASS |
| Idempotent checkpoint | `ctest --test-dir build/ci ... -R '^runner.state.nmi_idempotent$'` | Test and save-fixture setup passed, 0.07 s | ✓ PASS |
| Stale/omitted-load controls | `ctest --test-dir build/ci ... -R '^runner.state.nmi_wrong_load$'` | Test and save-fixture setup passed, 0.11 s | ✓ PASS |
| Existing input-sensitive boot-to-play controls | `ctest --test-dir build/ci ... -R '^runner.boot_to_play$'` | Four movie fixture tests and predicate test passed, 2.73 s | ✓ PASS |
| README/topic guide links | `ctest --test-dir build/ci ... -R '^docs\.links(\.self_test)?$'` | 2/2 passed, 0.05 s | ✓ PASS |
| Positive reproduction document/CTest-metadata check | `ctest --test-dir build/ci ... -R '^docs.reproduction$'` | **This attempt failed:** CTest reported timeout at 935.24 s after the script printed its success message; observed wall time was about 20 s. Cause unknown. | ✗ FAILED ATTEMPT |
| Fourteen document mutations, including review-fix controls | `ctest --test-dir build/ci ... -R '^docs.reproduction.self_test$'` | **This attempt failed:** CTest timed out at its 60.03 s bound after the positive check and first mutation passed; output also reported `/bin/ps` permission denied. Cause unknown. | ✗ FAILED ATTEMPT |

The failed document-test attempts are not represented as passes. Before these verification attempts, the post-review execution record for the same phase head reports successful CI and ASan document-check/self-test runs, all fourteen mutation controls, injected POSIX scratch/configure failures, `git diff --check`, and an exact fresh-scratch route. That record also preserves earlier timeouts and labels their cause unknown. No timeout was raised here, and these results are not a full CI claim.

The verifier used `-FA accuracycoin_full_report` for focused CTest runs. No full workflow or collector campaign ran: the existing `build/ci` cache has `NESTURBATOR_ACCURACYCOIN_FULL=ON`, and the full configured CI command would launch the campaign excluded by the approved scope. The earlier execution evidence records the bounded cross-phase selection; it is not relabeled as full CI.

## Probe Execution

No phase-declared `probe-*.sh`, PASS-marker probe or migration probe was found or implied by the plans/summaries. The phase delivers documentation and CMake/CTest contracts, not a migration or campaign.

## Requirements Coverage

| Requirement | Source plan | Description | Status | Evidence |
|---|---|---|---|---|
| REPRO-01 | 23-01, 23-03 | Sanitized scene report with content/core/host identity, access, inputs, expected/observed behavior, repeatability and untried conditions | ✓ SATISFIED | Complete form and permitted fixture example; README link and current link checks. |
| REPRO-02 | 23-01, 23-02, 23-03 | Bounded cold movie and supported neutral checkpoint replay with repeatability and independent input/continuation controls | ✓ SATISFIED | Current cold, comparator, preflight, NMI and boot-to-play tests passed; same-window and guest-marker controls remain separately identified. |
| REPRO-03 | 23-01, 23-02, 23-03 | Isolated scratch, explicit bounds, private-artifact limits and unsupported identity/container stop | ✓ SATISFIED | Source and CTest properties verified; positive and seven negative preflight cases passed; earlier post-review scratch injection and exact recipe results retained. |
| REPRO-04 | 23-03 | Triage separates native output, adapter, host presentation and original-game behavior; correctness requires independent evidence | ✓ SATISFIED | Decision table and evidence-limit sections; the record leaves commercial reports unclassified and rejects false-claim mutations in prior post-review runs. |

All four Phase 23 requirement IDs are present in the plans and map to the four approved Phase 23 requirements. No additional requirement is mapped to Phase 23, so there are no orphaned requirements.

### Decision Coverage

The decision-coverage query reports: “All trackable CONTEXT.md decisions are honored by shipped artifacts.” **22/22** decisions are accounted for; none is unhonored.

## Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---|---|---|---|

No true debt markers, stubs, hardcoded empty outputs, or console-log-only implementations were found in the changed implementation files. The `XXX` scan matched `XXXXXX` in the `mktemp` template; that is a required scratch-name pattern, not a debt marker. The review disposition records all three code-review findings as fixed. The tracked diff contains no ROM/movie/state fixture output or personal path/identifier.

## Human Verification Required

None. The approved requirements exclude device/commercial play and owner UAT; the judgment-only prohibitions above received explicit scoped source review, with the evidence limits preserved and no owner acceptance inferred.

## Gaps Summary

The phase goal is achieved: public docs provide a sanitized report and bounded cold/checkpoint reproduction paths, and the triage route directs developers to evidence suited to the claim without turning repeatability into correctness. The current cold, comparator, preflight, checkpoint and input-control checks passed. The current document-contract attempts timed out despite earlier post-review successful reruns; the cause remains unknown and is retained here and in the execution-evidence record. This is a local verification limitation, not a demonstrated source or product-acceptance defect. Full CI, live host/device work and collector campaigns were not run or claimed. No requirement gap or owner action remains.

---

_Verified: 2026-10-31T04:56:27Z_  
_Verifier: the agent (gsd-verifier)_
