---
phase: 22-install-and-protect-progress
verified: 2026-10-31T02:08:01Z
status: passed
score: 7/7 must-haves verified
covered_files:
  - .planning/phases/22-install-and-protect-progress/22-01-PLAN.md
  - .planning/phases/22-install-and-protect-progress/22-01-SUMMARY.md
  - .planning/phases/22-install-and-protect-progress/22-02-PLAN.md
  - .planning/phases/22-install-and-protect-progress/22-02-SUMMARY.md
  - .planning/phases/22-install-and-protect-progress/22-03-PLAN.md
  - .planning/phases/22-install-and-protect-progress/22-03-SUMMARY.md
  - .planning/phases/22-install-and-protect-progress/22-04-PLAN.md
  - .planning/phases/22-install-and-protect-progress/22-04-SUMMARY.md
  - .planning/phases/22-install-and-protect-progress/22-EXECUTION-EVIDENCE.md
  - README.md
  - docs/api.md
  - docs/progress.md
  - docs/releases.md
  - docs/runner.md
  - docs/testing.md
  - tests/CMakeLists.txt
  - tests/accuracy/baseline-metadata.txt
  - tests/accuracy/measurements.md
  - tests/cmake/doc_links.cmake
  - tests/cmake/runner_exit_table.cmake
  - tests/cmake/runner_options.cmake
  - tests/cmake/status_table.cmake
covered_digest: "v4:sha256:a6db597fdfcb270e0ed5d2b71da604e1fa0f5cc3690ab8f84fc71b0b53e2dfb5"
behavior_unverified: 0
overrides_applied: 0
decision_coverage:
  honored: 22
  total: 22
  not_honored: []
re_verification:
  previous_status: gaps_found
  previous_score: 6/7
  gaps_closed:
    - "The progress guide now links to the canonical API Save states section and testing Disposable progress fixtures section."
  gaps_remaining: []
  regressions: []
---

# Phase 22: Install and Protect Progress — Verification Report

**Phase Goal:** A player can find accurate installation/control guidance and preserve progress across an identified core upgrade or rollback.
**Verified:** 2026-10-31T02:08:01Z
**Status:** passed
**Re-verification:** Yes — after gap closure

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | READY-01 / roadmap criterion 1: The README gives a direct player route through supported content, installation and identity, loading, controls, progress, troubleshooting and developer guides. | VERIFIED | README sections appear in the locked order. The original README at fc1af9c02e19994adc3550d28ebbfd00986a4551 was enumerated by heading and reconciled against the current README and five guides. An exact-line scan found 1,979 of 2,110 original non-heading, nonblank lines unchanged; the remaining 131 were the stale v3/revision-8 opening, corrected install/provenance wording, path/anchor updates, or claims rewritten for the current checkout and assigned to the documented canonical homes. The 18-test guide/package selection below includes docs.links and its missing-target/fragment controls. |
| 2 | READY-02 / roadmap criterion 2: A player can choose the existing archive for the RetroArch executable architecture, install the matching core and info files, and distinguish release, CI and local-build identity. | VERIFIED | README names Linux/macOS/Windows × x64/arm64 and the platform module extensions. Names match cmake/packaging.cmake and the 18-archive release workflow. The dated 2026-10-30 release observation records v0.1.11, 18 expected archives, SHA256SUMS and a matching macOS arm64 archive digest; local attestation verification was unavailable and is not claimed. The configured macOS arm64 archive inventory and install-line extraction checks passed. Guidance says exact identity is unknown when the loaded path/source cannot be established. No other platform was packaged or launched locally. |
| 3 | READY-03 / roadmap criterion 3: Control instructions distinguish logical player ports from host Device Index and direct the player to verify the actual frontend launch route. | VERIFIED | README instructs one-player P1 with P2 unassigned and distinct devices on two active ports; it says a port change does not diagnose rendering or slowdown. docs/testing.md records the dated P1/P2 readback and logical-port test while explicitly limiting them to one launch/configuration and not all buttons, reconnects or handheld fallback. No new host or device operation was needed or performed. |
| 4 | READY-04: The guidance distinguishes battery RAM, native core snapshots, host state containers, rewind, autoload and host-configured storage. | VERIFIED | docs/progress.md names each data type, explains that snapshots include battery RAM, points to the host's configured Save Files/Save States directories, and preserves core-specific storage. docs/api.md separates native state payload semantics from host files. No universal host path is prescribed. |
| 5 | READY-05 / roadmap criterion 4: A player has a scratch-only backup, compatible roundtrip, native refusal and rollback procedure that preserves the newest battery copy and does not promise unsupported recovery. | VERIFIED | docs/progress.md keeps the newest battery copy outside active test/save paths, hashes both copies, keeps snapshots paired with their battery data, and describes an identified fresh launch with autoload isolated. It limits native refusal to native payloads and leaves cross-build SRAM and incompatible host-container recovery unknown. The Plan 22-04 record reports the generated-fixture digest stayed equal before/after its scratch walkthrough. This verifier directly ran runner.state.reject_format, core.state_generation and core.controller (plus the runner.save.fixture setup): 4/4 passed. The full workflow record says 699/700 passed with accuracycoin.activation as its only failure; no new long state-rejection run was started. |
| 6 | READY-06 / roadmap criterion 5: Canonical API, runner, release and testing contracts remain reachable and the document/package controls reject missing or contradictory claims. | VERIFIED | The 18 focused docs/release/info/policy checks passed, including canonical status-table, runner help/exit, stale-copy, link, release-marker and archive-metadata controls. tests/CMakeLists.txt supplies canonical docs to the status and runner consumers. The three current macOS arm64 archives passed check_archives.cmake; check_install_line.cmake extracted exactly the expected core and info file. No checker assertion was removed in the scoped files. |
| 7 | Plan 22-04 key links: The progress guide links its native-state guidance and fixture evidence to their canonical API and testing homes. | VERIFIED | docs/progress.md links the supported snapshot baseline to api.md#save-states and the disposable walkthrough to testing.md#disposable-progress-fixtures. Both target headings exist, and docs.links plus docs.links.self_test passed in this re-verification. |

**Score:** 7/7 distinct truths verified. All six READY requirements are substantively satisfied, and both plan-declared outbound links from the progress guide resolve to their canonical API and testing sections.

The 21 plan-frontmatter truths were merged with the five roadmap criteria and six requirements where they restated the same outcome. API/status and runner relocation controls map to READY-01/06; release/testing/package preservation maps to READY-01/02/06; and the install, controls and progress decisions map to READY-01 through READY-05. Re-verification checked the two formerly missing progress-guide links directly and ran their exact link checker and negative control; unchanged truths retain the evidence from initial verification.

## Flagged Assumption Review

The six fallback rows in 22-SOURCE-AUDIT.md were judged against the current artifacts. These are verifier judgments based on observed files and checks; the source-audit rows remain historical flags and were not converted to automatic green results.

| Requirement row | Evidence-supported judgment | Remaining limit |
|---|---|---|
| READY-01 — player route completeness | The final README has the requested order; all local guide links/fragments pass; every original README heading has a current home or an explicit player-facing retention. The full conservation check found no unassigned original section. | The owned link checker covers local targets/fragments, not every semantic claim. The manual conservation review supplies that part. |
| READY-02 — archive and loaded identity | README's six names match the package naming rules and CI's 18-target matrix; local macOS arm64 packages and the marked extraction command were checked. Release, CI and pinned-local identity chains are distinct. | Only macOS arm64 archives were built locally. No active user's loaded module was inspected; the guidance correctly leaves identity unknown without path and provenance. |
| READY-03 — host launch and port assignment | The README gives the actual-launch route and separate P1/P2 guidance; docs/testing.md preserves one dated launch readback and logical-port test evidence with their limits. | Host labels and unassigned-device choices vary. No all-button, reconnect, handheld-fallback or new live-host check is claimed. |
| READY-04 — storage ownership | Battery RAM, native states, host containers, rewind and autoload are separate in the progress guide, which directs players to configured host directories. | Host paths and settings remain installation-specific; no live directory was inspected. |
| READY-05 — rollback boundaries | The disposable procedure and state/source evidence distinguish native refusal from host-container behavior and retain the newest battery backup. | Cross-build SRAM reuse and incompatible host autoload recovery remain unqualified and are explicitly presented as unknown. |
| READY-06 — scratch/concurrency | New checker controls use nonce-suffixed owned scratch subdirectories; the progress recipe requires a fresh build tree per simultaneous invocation and generated/manifest-listed inputs only. | Same-build simultaneous execution of all existing fixture families was not established; the guidance requires separate build trees. No real save or other-core directory was used. |

## Descriptor-less Prohibition Judgments

The six kept prohibitions were not treated as implicit passes. Each has a concrete judgment here; the applicable evidence boundaries remain explicit.

| Prohibition | Judgment | Evidence and remaining limit |
|---|---|---|
| READY-06 / Plan 22-01: Contract relocation must not drop unique assertions or let a stale README satisfy a missing canonical guide. | Satisfied within the reviewed scope. | Status and runner self-tests keep stale README copies while removing or changing the canonical guide; README and the marked support/install claims were compared with the original. The controls cover their stated parser contracts, not every future prose edit. |
| READY-01 / Plan 22-04: New gameplay, device operation or owner sign-off must not become acceptance prerequisites. | Satisfied. | The phase's test and package acceptance is automated and the instructions do not require a play session or sign-off. None was performed. |
| READY-02 / Plan 22-04: Current-source capabilities must not be attributed to an unidentified or older binary. | Satisfied. | README labels support as current-source information; release guidance requires module path, digest and matching release/CI/local provenance, with unknown as the result when evidence is missing. The local Sigstore check did not verify an attestation. |
| READY-03 / Plan 22-04: Port assignment must not be presented as a diagnosis for unclassified rendering/slowdown reports. | Satisfied. | README explicitly says a port change does not diagnose those reports; the dated observations keep the cause open. |
| READY-04 / Plan 22-04: Private content, daily progress or another core's storage must not be disposable test input. | Satisfied. | The walkthrough uses a generated battery fixture and manifest-listed dabg.nes state fixture in a unique scratch/build tree. No private content, daily save or other-core directory was used. |
| READY-05 / Plan 22-04: Rollback must not overwrite the separate newest battery copy or promise unqualified cross-build/host-container recovery. | Satisfied within the stated boundary. | The newest backup stays outside active paths; the walkthrough's recorded digests matched. The guides preserve both sets when compatibility is unknown and state that native refusal does not prove host-container recovery. |

## Required Artifacts

| Artifact | Exists | Substantive | Wired | Data flow | Status / evidence |
|---|---:|---:|---:|---|---|
| README.md | Yes | Yes | Yes | Static documentation | VERIFIED — player route, supported-content summary, marked install command and guide links are present. |
| docs/api.md | Yes | Yes | Yes | Static documentation | VERIFIED — API, battery, state and status contracts; the status consumer reads only this canonical guide. |
| docs/runner.md | Yes | Yes | Yes | Static documentation | VERIFIED — options compare against real --help; exit/stop contract compares against runner/exit_status.h and help output. |
| docs/releases.md | Yes | Yes | Yes | Static documentation | VERIFIED — archive inventory, marker policy and identity guide are linked from README and match package/release sources. |
| docs/testing.md | Yes | Yes | Yes | Static documentation | VERIFIED — build, evidence limits and host/test procedures are linked from README. |
| docs/progress.md | Yes | Yes | Partial | Static documentation | PARTIAL — README links to it and it is substantive, but the two outbound links in Plan 22-04 are absent. |
| tests/CMakeLists.txt and tests/cmake/{doc_links,status_table,runner_options,runner_exit_table}.cmake | Yes | Yes | Yes | No rendered data | VERIFIED — all plan-declared artifact paths exist; gsd-tools artifact checks passed for all four plans. Static link-query false negatives for relative CMake script paths were checked manually at their registrations. |
| cmake/packaging.cmake, tests/cmake/check_archives.cmake and check_install_line.cmake | Yes | Yes | Yes | Build outputs | VERIFIED — unchanged package contract; current local archive inventory and install-line extraction passed. |
| tests/accuracy/baseline-metadata.txt and measurements.md | Yes | Yes | Yes | Campaign report/provenance | VERIFIED — this bookkeeping refreshes the existing fingerprint/source measurement. The report SHA-256 matches the stored value exactly; scoreboard, pins and runtime sources are unchanged. |

## Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| README.md | docs/api.md | Relative Markdown guide link | WIRED | docs.links and its negative controls passed. |
| tests/CMakeLists.txt | tests/cmake/status_table.cmake | state.status_table registration passes canonical docs/api.md | WIRED | The CMake registration invokes cmake/status_table.cmake with DOC=docs/api.md; the generic link query missed the relative script path. |
| tests/cmake/status_table.cmake | include/nesturbator.h | Bidirectional enum/table comparison | WIRED | The checker parses the public enum and compares every row in both directions. |
| README.md | docs/runner.md | Relative Markdown guide link | WIRED | docs.links passed. |
| tests/CMakeLists.txt | docs/runner.md | Canonical DOC input to runner consumers | WIRED | runner options and exit consumers receive docs/runner.md; focused tests passed. |
| tests/cmake/runner_exit_table.cmake | runner/exit_status.h | Header/help/guide equality | WIRED | The production checker reads the header and the real runner's --help output. |
| README.md | docs/releases.md | Developer/package guide links | WIRED | Local README link and fragment checks passed. |
| README.md | docs/testing.md | Build/test/evidence guide links | WIRED | Local README link and fragment checks passed. |
| tests/cmake/release_markers.cmake | release-please-config.json | README remains a version-marker-owned file | WIRED | The marker configuration and checker still include README; the marked block is byte-identical to the original. |
| cmake/packaging.cmake | tests/cmake/check_archives.cmake | Package output inventory | WIRED | CI runs check_archives after packaging; direct local inventory check accepted all three current macOS arm64 archives. |
| README.md | cmake/packaging.cmake | Documented OS/architecture archive names match CPack naming | WIRED | Manual source comparison confirms matching component, OS and architecture values. README is not intended to link players to the CMake implementation. |
| README.md | docs/progress.md | Progress/troubleshooting links | WIRED | README points to the progress guide; docs.links passed. |
| docs/progress.md | docs/api.md#save-states | Native support baseline versus loader enforcement | WIRED | Direct Markdown link follows Supported snapshot baseline; exact target heading exists. |
| docs/progress.md | docs/testing.md#disposable-progress-fixtures | Scratch fixtures and reused evidence | WIRED | Direct Markdown link follows the fixture explanation; exact target heading exists. |

## Data-Flow Trace (Level 4)

Not applicable to rendered values: Phase 22 delivers static Markdown guides, not a UI that renders query results or fetched values. Package identity values were checked against packaging sources and current generated local archives.

## Behavioral Spot-Checks

| Behavior | Command / evidence | Result | Status |
|---|---|---|---|
| Repaired progress-guide links | ctest --test-dir build/ci --output-on-failure --no-tests=error -FA accuracycoin_full_report -R '^(docs\.links|docs\.links\.self_test)$' | Both exact link/fragment validation and its negative control passed (2/2). | PASS |
| Canonical document, link, runner, release and info checks | Initial-verification CTest selection recorded above | 18/18 passed in initial verification; unaffected checks were not rerun. | PASS (prior evidence) |
| Native format refusal, save-generation invariant and separate logical input ports | ctest --test-dir build/ci --output-on-failure --no-tests=error -FA accuracycoin_full_report -R '^(runner.state.reject_format|core.state_generation|core.controller)$' | 4/4 passed including runner.save.fixture setup. No campaign fixture was enabled. | PASS |
| Archive component inventory | cmake -DDIR=build/ci -P tests/cmake/check_archives.cmake | Accepted library, libretro and runner macOS arm64 archives for v0.1.11. | PASS |
| Marked macOS install extraction | cmake -DREADME=README.md -DPACKAGES=build/ci/packages -DOUT=build/ci/install-line -P tests/cmake/check_install_line.cmake | Extracted exactly cores/nesturbator_libretro.dylib and info/nesturbator_libretro.info from the local archive. | PASS |
| Scratch battery/state procedure | Plan 22-04 execution record | Generated-fixture roundtrip/refusal selections passed; its before/after battery backup digests matched. This verification did not repeat the full scratch procedure. | PASS (recorded evidence) |
| AccuracyCoin migration/activation/protected-observation/scoreboard/no-lost-pass/baseline-fetch controls | Existing build/ci/Testing/Temporary/LastTest.log at 2026-10-30 21:58 EDT | 6/6 passed. The activation fixture was excluded; the log contains no campaign invocation. | PASS (existing run record) |
| Full configured CI workflow | 22-EXECUTION-EVIDENCE.md and the preserved failure record | The initial run was 699/700; accuracycoin.activation failed because the registered build-input fingerprint was stale after tests/CMakeLists.txt changed. The later activation group passed after metadata refresh. Execution evidence records the owner's authorization boundary for a bounded native refresh; existing evidence met it, so no additional campaign ran. The full workflow was not rerun. | NOT A FULL-WORKFLOW PASS |

The report file in build/ci/accuracycoin/full/report.tsv has SHA-256 56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3, matching both build metadata and tests/accuracy/baseline-metadata.txt. The refresh preserves the 119 PASS / 27 FAIL measured rows, five DRAW rows and historical measurements. It repairs current-input provenance; it does not establish emulator correctness or a hosted result.

## Probe Execution

Not applicable. None of the four plans declares a probe script, and no conventional scripts/*/tests/probe-*.sh probe was found.

## Requirements Coverage

| Requirement | Source plan | Status | Evidence |
|---|---|---|---|
| READY-01 | 22-01 through 22-04 | SATISFIED | README player order, full original-README conservation review, and passing local-guide target/fragment checks. |
| READY-02 | 22-03 and 22-04 | SATISFIED | Six archive choices match package naming; release/CI/local identity paths and unknown handling are documented; local macOS arm64 archives and extraction checked. |
| READY-03 | 22-04 | SATISFIED | Actual-launch host-owned controls, P1/P2 device distinction and dated evidence limits are documented. No new host operation is required by the approved boundary. |
| READY-04 | 22-04 | SATISFIED | Save/state/container/rewind/autoload distinctions and configured host storage are documented. |
| READY-05 | 22-04 | SATISFIED | Backup, digest, roundtrip, native refusal and rollback procedure is present; cross-build SRAM and incompatible host-container recovery remain expressly unknown. |
| READY-06 | 22-01 through 22-04 | SATISFIED | Canonical contracts and independent relocation controls pass; archive and install-line checks pass. The progress-guide links to the canonical API and testing sections now resolve and pass link checks. |

## Test Quality Audit

| Test file / check | Linked requirement | Active | Skipped | Circular | Assertion level | Verdict |
|---|---|---:|---:|---:|---|---|
| tests/cmake/doc_links.cmake | READY-01, READY-06 | Yes | No | No | Behavioral: missing target and fragment controls | Valid; 2 focused checks passed. |
| tests/cmake/status_table.cmake | READY-06 | Yes | No | No | Value: enum equality, duplicate/extra/missing rows and canonical-path rejection | Valid; production check and self-test passed. |
| tests/cmake/runner_options.cmake and runner_exit_table.cmake | READY-06 | Yes | No | No | Value/behavioral: real help/header equality plus stale-copy/mutation controls | Valid; production comparisons and self-tests passed. |
| tests/cmake/check_archives.cmake and check_install_line.cmake | READY-02, READY-06 | Yes | No | No | Value: exact package inventory and extraction operands/members | Valid; current local packages accepted. |
| tests/cmake/runner_save.cmake and runner_state.cmake | READY-05 | Yes | No | No | Behavioral: generated battery save, native roundtrip and deliberate format refusal | Valid; Plan 22-04 record reports the focused selections passed. |
| tests/core/test_state_generation.c and test_state_reject.c | READY-05 | Yes | No | No | Behavioral/value: battery generation and byte-identical state after refusal | Source asserts the invariant; full workflow aggregate reports these among the 699 passing tests. The 60-second state-rejection check was not rerun. |
| tests/core/test_controller.c | READY-03 | Yes | No | No | Behavioral: independent logical port state | Valid for the documented logical-port scope; it does not prove host Device Index routing. |

**Disabled tests on requirements:** 0 found in the linked document checker files.
**Circular patterns detected:** 0.
**Insufficient assertions:** 0 for the documented requirements. Logical-port tests are not treated as host-routing proof; that limitation is explicit.

## Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---:|---|---|---|
| Scoped implementation files | — | No debt markers, placeholders, empty implementations or disabled checker tests found. A broad XXX search only matched the mktemp XXXXXX template in docs/progress.md. | None | No implementation stub or unresolved marker found. |

## Decision Coverage

The configured decision-coverage gate reports 22/22 adopted CONTEXT.md decisions honored, with no unhonored decisions. It is a non-blocking gate.

## Human Verification Required

None. The approved acceptance boundary does not require owner gameplay, a device operation, host setup or sign-off. The absence of incompatible host-autoload recovery and cross-build SRAM evidence is stated as a limit, not as a verified behavior.

## Deferred Items

None. No open verification item is deferred to a later phase.

## Gaps Summary

The previous blocking gap is closed: docs/progress.md now links to docs/api.md#save-states and docs/testing.md#disposable-progress-fixtures, both exact headings exist, and docs.links plus docs.links.self_test pass. All six READY requirements and the five roadmap success criteria have evidence. The initial full CI failure remains recorded as a stale-fingerprint activation failure; the repaired activation check passed, but the full workflow was not rerun and must not be described as having passed.

---

_Verified: 2026-10-31T02:08:01Z_
_Verifier: the agent (gsd-verifier)_
