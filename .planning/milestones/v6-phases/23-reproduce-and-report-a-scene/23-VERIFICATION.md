---
phase: 23-reproduce-and-report-a-scene
verified: 2026-10-31T13:23:22Z
status: passed
score: 9/9 roadmap and plan must-haves verified
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
covered_digest: "v4:sha256:819a7ef0397e51ac6e895ac96a26b888138f026f928275c6301bcf58d6f9388d"
behavior_unverified: 0
overrides_applied: 0
re_verification:
  previous_status: passed
  previous_score: 6/6
  reason: "The previous report's fingerprint was stale after shared guide and contract changes; this refresh rechecked the roadmap truths and plan must-haves."
  gaps_closed: []
  gaps_remaining: []
  regressions: []
decision_coverage:
  honored: 22
  total: 22
  not_honored: []
prohibition_review:
  mode: scoped_source_review
  status: flagged_unverified
  authoritative: false
  owner_risk_acceptance: false
  items:
    - statement: "MUST NOT publish commercial media, saves, checkpoints, raw private logs/captures or personal identifiers as scene-report evidence."
      verification: judgment
      disposition: unverified
    - statement: "MUST NOT imply a scene was reached or an identity is known when access or identity is unavailable."
      verification: judgment
      disposition: unverified
    - statement: "MUST NOT represent movie-plus-load-state, host recording or foreign movie conversion as supported runner capabilities."
      verification: judgment
      disposition: unverified
    - statement: "MUST NOT label repeated frame/audio hashes as independent hardware correctness."
      verification: judgment
      disposition: unverified
    - statement: "MUST NOT read or write daily progress or silently substitute an unknown, mismatched or host-container checkpoint."
      verification: judgment
      disposition: unverified
    - statement: "MUST NOT report timeout, interruption, truncation or missing required output as successful replay."
      verification: judgment
      disposition: unverified
    - statement: "MUST NOT relabel commercial reports as diagnosed or fixed or equate native/adapter parity with host presentation."
      verification: judgment
      disposition: unverified
    - statement: "MUST NOT launch live hosts/devices, commercial replay, collector campaigns, publication or emulator-source inspection under this phase."
      verification: judgment
      disposition: unverified
  limits:
    - "No contrary claim was found in the repository scope reviewed; this is non-authoritative evidence, not proof of external activity or owner acceptance."
    - "The eight judgment-tier prohibitions are not mechanically proven and remain flagged."
---

# Phase 23: Reproduce and report a scene — Verification Report

**Phase Goal:** A developer can repeat a precisely reported scene without disturbing daily progress and choose the next evidence source without overclaiming a diagnosis.
**Verified:** 2026-10-31T13:23:22Z
**Status:** passed
**Re-verification:** Yes — refreshed after the prior report became stale

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | A developer can reach a complete sanitized report form and permitted-fixture example from README troubleshooting, with exact-scene access, identity, expectation provenance, outcomes and unknown conditions distinguished. | ✓ VERIFIED | [docs/reproduction.md](../../../../docs/reproduction.md#report-a-scene) contains every report field, permits unknown/withheld/not-tried/unavailable values, and separates title-level observation from exact-scene access. [README.md](../../../../README.md:120) links directly to it. I recomputed the permitted fixture's SHA-256 from the checked-in bytes; it matches the manifest and example. |
| 2 | The public source-build cold route runs two complete 600-frame movies under nested deadlines and compares every ordered native frame record plus transition and PCM output. | ✓ VERIFIED | [tests/cmake/reproduction.cmake](../../../../tests/cmake/reproduction.cmake:275) creates a fresh nonce directory, invokes the existing movie generator and public runner twice, checks child results/stops, requires frames 1–600 and both 64-hex audio hashes, and compares the records. The current bounded CTest run passed runner.reproduction.cold; its output reports matching frame-record, transition and PCM hashes. |
| 3 | The movie route retains two-port NMOVIE1 behavior and the existing independently chosen input-sensitive controls. | ✓ VERIFIED | [tests/runner/game_movie.c](../../../../tests/runner/game_movie.c) writes the two-port format, Start at indexes 60–63, generator-owned jump spans and neutral port 1. [tests/runner/test_boot_to_play.c](../../../../tests/runner/test_boot_to_play.c) still checks positive play separately from no-input, attract-only and Start-only controls; CTest registrations remain in [tests/CMakeLists.txt](../../../../tests/CMakeLists.txt:1570). Those played-input controls passed in the earlier Phase 23 run (four fixture cases plus predicate, 2.73 s); they are outside the current 31-test selection. The current selection did pass core.controller. |
| 4 | The separate neutral checkpoint route saves at absolute frame K=1, continues to N=4, compares audio frames 2–4, and retains stale/omitted-load and idempotence controls. | ✓ VERIFIED | [docs/reproduction.md](../../../../docs/reproduction.md#native-checkpoint-continuation) describes the absolute frame window and neutral input. [tests/cmake/runner_state.cmake](../../../../tests/cmake/runner_state.cmake) and its [CTest registrations](../../../../tests/CMakeLists.txt:325) connect the guide's runner.state.nmi_roundtrip, nmi_idempotent and nmi_wrong_load cases to the runner. All three passed in the current bounded selection. |
| 5 | Checkpoint support is qualified only after runner, content and state identities match; unknown, mismatched and host-container origins stop before replay. | ✓ VERIFIED | [tests/cmake/reproduction.cmake](../../../../tests/cmake/reproduction.cmake:23) validates digest shape, hashes the current runner/ROM/state and refuses unsupported origin/container cases. runner.reproduction.preflight and runner.reproduction.preflight_self_test passed in the current selection; the latter exercises seven independent invalid cases. The guide and code both state that the caller-declared origin is not authentication and that native loading does not enforce exact build pairing. |
| 6 | Every documented route has explicit frame/process/CTest bounds, isolated scratch and incomplete-output handling, with daily progress outside the example paths. | ✓ VERIFIED | [docs/reproduction.md](../../../../docs/reproduction.md#scratch-storage-and-bounds) specifies the route bounds and keeps the newest battery backup outside active paths. The wrapper writes generated files under fresh build-tree roots, has finite execute_process timeouts, and treats interruption, missing output and partial frames as failure. The current document contract and self-test passed; neither route is given a host save/config path. No daily save operation was performed. |
| 7 | The guide selects a next evidence source while separating native output, adapter conversion, host presentation and original-game correctness; commercial reports remain unclassified. | ✓ VERIFIED | [docs/reproduction.md](../../../../docs/reproduction.md#choose-the-next-evidence-source) maps each comparison to its inference, unknowns and next bounded source. Its evidence-limits section requires an independent claim-specific oracle and explicitly leaves commercial observations unclassified. The current document self-test includes mutations for false hash correctness, screenshot equivalence, host-pacing attribution and relabeling commercial observations. |
| 8 | README and topic guides resolve to one maintained guide, and executable checks protect report claims, commands, generated CTest metadata and safety boundaries. | ✓ VERIFIED | README plus runner, testing and progress guides link to [docs/reproduction.md](../../../../docs/reproduction.md). Current docs.links, docs.links.self_test, docs.reproduction and docs.reproduction.self_test checks passed in the bounded run. The contract reads generated CTest metadata and mutates required fields, targets, test names and safety/evidence statements. |
| 9 | Phase evidence distinguishes measured, inherited, unsupported, unavailable and untried work, without expanding runtime, live-device or campaign scope. | ✓ VERIFIED | [23-EXECUTION-EVIDENCE.md](23-EXECUTION-EVIDENCE.md) separates Plan 01/02 measurements from Plan 03 checks, records identity/window limits and keeps PowerShell, other platforms, host/device and private-content routes untried. The phase changed documentation and CMake/CTest orchestration; no emulator runtime or public API changes are in scope. The supplied current regression run explicitly excluded the AccuracyCoin campaign fixture. |

**Score:** 9/9 roadmap and plan must-haves verified (0 present, behavior-unverified)

The nine rows consolidate the four roadmap success criteria with all plan-specific truths. Plan artifacts and declared links are checked separately below. Current commercial access and an independent hardware oracle remain unestablished by design; the goal requires an honest triage path, not a diagnosis.

### Re-verification and Current Regression Evidence

The previous report's frontmatter status was passed (6/6), but its fingerprint was stale after shared Phase 22 guide and contract changes. I rechecked the roadmap criteria and all plan must-haves against the current source.

The orchestrator-supplied bounded CTest output completed **31/31 tests in 33.11 seconds** with the accuracycoin_full_report fixture excluded. The current build/ci/Testing/Temporary/LastTest.log contains 31 selected test entries and ends with exit_status = 0. The selection covers the reproduction cold run/comparator/preflight, NMI state cases, documentation/link/policy checks, runner/API/status/release/info contracts, state generation, controller input, and required fixture setup. It includes docs.reproduction and its self-test. The current cold-run output reports matching frame-record SHA-256 a90cc60caeea1fc4e4f159e3341b5f1b446022281c018b6d609eebf702175d9b, transition SHA-256 33e4bce709976dca002689e4cc164f694a677d9949fdfa325b7dd6cfa5cc4006, and PCM SHA-256 255636674903d03dd9cb7ca632246c3375dff8c88300155b052ac5c273d4dcca. These are evidence of repeatability on the measured build/platform only.

The current 31-test selection did not include runner.boot_to_play; that check is supported by its earlier recorded Phase 23 execution result and unchanged source/CTest wiring. The current run's core.controller check covers controller behavior but is not represented as a substitute for the boot-to-play predicate.

The selection is bounded regression evidence, not a full-workflow result. The configured full workflow would enable accuracycoin_full_report; no AccuracyCoin campaign, full workflow, provenance refresh, live host/device, commercial replay or daily-save operation was performed.

### Advisory (New Scope, Unevidenced)

None. No new-scope defect with missing deterministic evidence was raised in this refresh.

### Flagged Unresolved Assumptions

The plans' five heuristic fallback edges remain unresolved and are not converted into requirement failures or acceptance claims: REPRO-01 idempotency and concurrency, plus the unclassified REPRO-02, REPRO-03 and REPRO-04 probe classifications.

### Judgment-Only Prohibition Review

All eight plan prohibitions are retained in frontmatter as flagged_unverified. A scoped read-only source review found no contradictory report claim or path in the files inspected, but that review is non-authoritative. It cannot prove activity outside this repository/session and does not constitute owner risk acceptance. This verification records no human signoff and does not wait on one.

## Required Artifacts

| Artifact | Expected | Status | Details |
|---|---|---|---|
| docs/reproduction.md | Complete report/replay/safety/triage guide | ✓ VERIFIED | Exists and contains the report form, permitted example, cold movie, checkpoint, identity, bounds, outcome and triage sections. Direct links and report contract are active. |
| tests/cmake/reproduction.cmake | Cold replay, strict parser/comparator and checkpoint identity preflight | ✓ VERIFIED | Substantive CMake modes share the same output validation. CTest invokes cold, comparator, positive preflight and preflight self-test modes. |
| tests/cmake/reproduction_contract.cmake | Scoped guide/command/metadata assertions and negative controls | ✓ VERIFIED | Contains report-field, safety, target/test, timeout, fixture and triage assertions plus independent document mutations. Both check modes are registered with 60-second bounds. |
| tests/CMakeLists.txt | Bounded CTest registrations and setup dependencies | ✓ VERIFIED | Registers cold replay at 180 s; comparator, preflight, documentation, and NMI checks with finite bounds; the current generated metadata is inspected by the document contract. |
| README.md, docs/runner.md, docs/testing.md, docs/progress.md | Entry links to canonical guide and progress policy | ✓ VERIFIED | Direct guide links resolve; progress guidance retains its separate newest-battery backup policy. |
| tests/runner/game_movie.c, tests/runner/test_boot_to_play.c, tests/cmake/runner_state.cmake | Existing permitted movie and continuation controls | ✓ VERIFIED | Scenario writers, input-sensitive predicates and neutral NMI continuation are substantive and registered. This refresh did not modify those authorities. |
| 23-EXECUTION-EVIDENCE.md | Sanitized run record and evidence limits | ✓ VERIFIED | Distinguishes inherited measurements from Plan 03 and refresh checks, plus untried/unsupported scope and review history. |

The verify.artifacts query passed all declared artifacts in each of the three plans (3/3 per plan).

## Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| README.md | docs/reproduction.md | Troubleshooting link | ✓ WIRED | Direct Markdown link; current guide-link checks pass. |
| tests/CMakeLists.txt | tests/cmake/reproduction.cmake | runner.reproduction.* | ✓ WIRED | CTest commands invoke the script with cold, compare and preflight modes. |
| docs/reproduction.md | tests/runner/game_movie.c | runner.game_movie / nesrunner-pos | ✓ WIRED | Guide links the source and names the public source-build target; CTest passes its target path to the cold wrapper. |
| docs/reproduction.md | tests/cmake/runner_state.cmake | NMI test names | ✓ WIRED | Guide names the NMI cases; runner_state_case() registers them to runner_state.cmake. |
| tests/cmake/reproduction.cmake | docs/api.md | Exact-build support policy vs. loader checks | ✓ WIRED (policy boundary) | This is a semantic boundary, not a file include: the wrapper comment separates preflight from loader validation; docs/api.md states exact pairing is support guidance, and the reproduction guide links to that contract and states the loader limit. |
| docs/reproduction.md | docs/progress.md | Backup/container policy | ✓ WIRED | Direct link plus explicit newest-battery and host-container boundary. |
| docs/reproduction.md | docs/testing.md | Adapter/callback evidence limits | ✓ WIRED | Direct link; the guide distinguishes host.convert and libretro.host from frontend/device behavior. |
| docs/reproduction.md | .planning/playtesting/POSTMORTEM.md | Commercial observations | ✓ WIRED | Direct dated-evidence link; observations remain unclassified. |
| docs/runner.md, docs/testing.md, docs/progress.md | docs/reproduction.md | Topic-guide navigation | ✓ WIRED | All three guides link directly to the canonical reproduction guide; docs.links checks passed. |

The generic verify.key-links query reported false negatives for CMake -P relationships and Markdown links whose names are represented by CTest cases. Each was manually traced through its source registration and target. No declared key link remains unwired.

## Data-Flow Trace (Level 4)

There is no rendered UI or database-backed page in this documentation/tooling phase. The executable evidence path consumes actual runner and fixture data:

| Artifact | Data variable | Source | Produces real data | Status |
|---|---|---|---|---|
| tests/cmake/reproduction.cmake | run_1_output, run_2_output | Two actual invocations of the built public runner on the generated NMOVIE1 fixture | Yes; current cold CTest output confirms 600 records and matching audio hashes | ✓ FLOWING |
| tests/cmake/reproduction.cmake | current and recorded runner/ROM/state digests | file(SHA256) over the actual runner, manifest fixture and invocation-created state | Yes; current preflight checks passed | ✓ FLOWING |
| docs/reproduction.md | Fixture digest and measured output hashes | Checked-in tests/roms/runner.nes, manifest and bounded cold-run output | Yes; fixture hash recomputed and output hashes observed in the current test log | ✓ FLOWING |

## Behavioral Spot-Checks

| Behavior | Command/source | Result | Status |
|---|---|---|---|
| Repeat two complete cold movies and compare frame/transition/PCM output | runner.reproduction.cold in supplied bounded CTest selection | Passed; log reports 600 ordered frame records and all three matching identities | ✓ PASS |
| Reject altered/incomplete output representations | runner.reproduction.compare_self_test | Passed in current 31-test selection; independent synthetic controls target changed frame/audio, missing audio, duplicate/reordered and truncated records | ✓ PASS |
| Accept the positive identity path and reject bad identity/origin/container cases | runner.reproduction.preflight, runner.reproduction.preflight_self_test | Both passed; seven independent unsupported cases stop before replay | ✓ PASS |
| Resume neutral NMI state, preserve idempotence and detect stale/omitted load | runner.state.nmi_roundtrip, runner.state.nmi_idempotent, runner.state.nmi_wrong_load | Passed in current bounded selection, including required save-fixture setup | ✓ PASS |
| Protect guide fields, commands, evidence boundaries and local links | docs.reproduction, docs.reproduction.self_test, docs.links, docs.links.self_test | Passed in current bounded selection | ✓ PASS |
| Preserve controller behavior | core.controller in current selection; runner.boot_to_play source/registration and earlier Phase 23 run record | Controller check passed currently; boot-to-play controls passed in the prior 2.73 s run and were not included in this 31-test refresh | ✓ PASS (prior test; source unchanged) |

The test-quality audit found no disabled linked tests or circular correctness oracle. The cold comparator intentionally establishes repeatability by comparing two real runs, not hardware correctness. The synthetic positive pair is separately authored for comparator sensitivity; the NMI guest-marker and boot-to-play predicates retain their own behavioral claims.

## Probe Execution

No probe-*.sh file or probe/PASS-marker check is declared or implied in the phase plans/summaries, and no conventional scripts/*/tests/probe-*.sh file exists. Not applicable.

## Requirements Coverage

| Requirement | Source plan | Description | Status | Evidence |
|---|---|---|---|---|
| REPRO-01 | 23-01, 23-03 | Sanitized scene report with identity, scene/access, inputs, behavior, repeatability and untried conditions | ✓ SATISFIED | Complete form/example, README entry link, fixture digest and current document/link checks. |
| REPRO-02 | 23-01, 23-02, 23-03 | Bounded cold movie and neutral checkpoint replay with repeatability plus independent input/continuation controls | ✓ SATISFIED | Current cold/comparator/preflight/NMI tests and preserved boot-to-play controls; measurement windows remain separate. |
| REPRO-03 | 23-01, 23-02, 23-03 | Isolated scratch, explicit limits, privacy and unsupported identity/container refusal | ✓ SATISFIED | Source paths and generated CTest metadata verified; current document and preflight controls passed. |
| REPRO-04 | 23-03 | Triage separates native, adapter, host and original-game evidence and keeps correctness claims oracle-bound | ✓ SATISFIED | Decision/evidence-limit sections, current document mutations and unclassified commercial reports. |

The plans claim all four requirements. REQUIREMENTS.md maps no additional IDs to Phase 23; no orphaned requirement was found.

### Decision Coverage

The current decision-coverage query found all 22/22 trackable CONTEXT decisions honored, with none unhonored. This gate is non-blocking.

## Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---|---|---|---|

No blocking debt markers, stubs, empty implementations, hardcoded empty outputs or personal identifiers were found in the phase implementation/evidence scope. The “Linux and Windows were not available” text records platform limits; it is not a placeholder. Current CTest output paths remain in ignored build scratch, not tracked phase artifacts.

## Test Quality Audit

| Test File | Linked Requirement | Active | Skipped | Circular | Assertion Level | Verdict |
|---|---|---|---|---|---|---|
| tests/cmake/reproduction.cmake | REPRO-02, REPRO-03 | Yes | None | No correctness oracle; cold run compares repeatability | Behavioral/value | Adequate |
| tests/cmake/reproduction_contract.cmake | REPRO-01, REPRO-03, REPRO-04 | Yes | None | No; independent required claims and mutations plus generated CTest metadata | Value/behavioral | Adequate |
| tests/cmake/runner_state.cmake | REPRO-02 | Yes | None | No; continuation equivalence is limited to the named fixture and guest-marker controls | Behavioral | Adequate |
| tests/runner/test_boot_to_play.c, tests/runner/game_movie.c | REPRO-02 | Yes | None | No hardware claim; owned input-sensitive fixture predicate | Behavioral | Adequate |

**Disabled tests on requirements:** 0. **Circular patterns detected:** 0. **Insufficient assertions:** 0.

## Human Verification Required

None. This is a documentation and CTest tooling phase with no visual UI, live service or authorized host/device flow. Exact commercial-scene access and hardware correctness remain explicitly unclassified; they are not acceptance conditions for this phase.

## Historical Test-Time Anomalies and Limits

The previous verification recorded document-check timeout anomalies. They remain historical failures, with the cause unknown:

- docs.reproduction timed out after its script printed success; CTest reported 935.24 seconds while observed wall time was about 20 seconds.
- docs.reproduction.self_test hit its 60.03-second CTest limit after the positive check and first mutation; output also reported /bin/ps permission denied.
- A later review-fix self-test reported 1,086 seconds against its 60-second limit; a narrow retry reported 921 seconds.
- A separate prior-phase libretro.info_claims.self_test reported 1,071 seconds.

Subsequent evidence recorded an exact fresh-scratch route passing in about 21 seconds, a verbose document self-test passing in about 20 seconds, and later positive/self-test passes with all 14 then 17 document mutations. No deadline was raised and no cause was established. The current supplied 31/31 bounded run, including both reproduction document checks, completed in 33.11 seconds. This current pass does not explain the earlier timing anomalies.

Linux and Windows were unavailable; PowerShell was inspected by document controls but not executed. No live host/device, commercial content, private capture, campaign, full workflow, publication, release, push or PR was part of this verification. The full CI workflow was intentionally not run because the configured AccuracyCoin fixture would exceed the authorized scope.

## Gaps Summary

No requirement or plan must-have gap was found. The report/replay/checkpoint/triage path is present, connected and backed by the supplied bounded regression result plus direct source evidence. Repeatability remains explicitly separate from hardware correctness. The eight judgment-tier prohibitions and five fallback edges remain flagged/unresolved; they are not accepted risks, owner approvals or correctness claims. No user action is needed to complete this phase verification.

---

_Verified: 2026-10-31T13:23:22Z_
_Verifier: the agent (gsd-verifier)_
