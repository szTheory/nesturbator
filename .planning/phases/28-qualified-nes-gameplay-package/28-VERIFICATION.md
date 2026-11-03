---
phase: 28-qualified-nes-gameplay-package
verified: 2026-11-03T21:08:06Z
status: gaps_found
score: 14/17 must-haves verified
covered_files:
  - ".planning/phases/28-qualified-nes-gameplay-package/28-01-PLAN.md"
  - ".planning/phases/28-qualified-nes-gameplay-package/28-01-SUMMARY.md"
  - ".planning/phases/28-qualified-nes-gameplay-package/28-02-PLAN.md"
  - ".planning/phases/28-qualified-nes-gameplay-package/28-02-SUMMARY.md"
  - ".planning/phases/28-qualified-nes-gameplay-package/28-03-PLAN.md"
  - ".planning/phases/28-qualified-nes-gameplay-package/28-03-SUMMARY.md"
  - ".planning/phases/28-qualified-nes-gameplay-package/28-ACCEPTANCE.md"
  - ".planning/phases/28-qualified-nes-gameplay-package/28-IDENTITY.json"
  - ".planning/phases/28-qualified-nes-gameplay-package/28-REVIEW-DELTA.md"
  - "build/ci/phase28-local-frame-sampling/LastTest.log"
  - "build/ci/phase28-local-frame-sampling/ctest-discovery.json"
  - "build/ci/qualified-consumer-521294fbd128/gameplay.json"
  - "docs/nrom-gameplay.md"
  - "docs/provider-ntsc.json"
  - "tests/core/test_controller.c"
covered_digest: "v4:sha256:a5ac969d8927bbf702054cd482a0bec63cad004fbbbde2306a822f02545de258"
behavior_unverified: 0
overrides_applied: 0
re_verification:
  previous_status: gaps_found
  previous_score: 14/17
  gaps_closed: []
  gaps_remaining:
    - "Hosted CI on the exact candidate and resulting main head."
    - "Published package receipt binding PKG-01 and PROF-01 to the actual release."
  regressions: []
decision_coverage:
  honored: 7
  total: 7
  not_honored: []
gaps:
  - truth: "The exact accepted candidate and resulting main head pass their required hosted CI."
    status: failed
    reason: "The acceptance record explicitly leaves exact-candidate CI and post-merge main CI pending; no hosted result is claimed."
    artifacts:
      - path: ".planning/phases/28-qualified-nes-gameplay-package/28-ACCEPTANCE.md"
        issue: "It records local readiness and release handoff, not hosted candidate or resulting-main CI."
    missing:
      - "Run the normal release path and verify required CI on the exact candidate and resulting main head."
  - truth: "A published package and receipt bind the accepted source, version, artifacts, profile, and native gameplay result."
    status: failed
    reason: "The current archives and profile are candidate-local. Publication and the published receipt are pending, so PKG-01 and PROF-01's final release binding remain incomplete."
    artifacts:
      - path: "docs/provider-ntsc-release.json"
        issue: "The required published-package receipt does not exist."
      - path: "docs/provider-ntsc.json"
        issue: "The profile is explicitly candidate-only and has release_binding pending."
      - path: ".planning/phases/28-qualified-nes-gameplay-package/28-IDENTITY.json"
        issue: "The exact source identity is local-ready; published receipt and PKG-01 remain pending."
    missing:
      - "Verify the actual published library, runner, and libretro archives, checksums, attestations, exact source/version metadata, and native relocated gameplay."
      - "Generate and verify the final receipt that binds the published package, profile, and scenario."

---

# Phase 28: Qualified NES gameplay package — Verification Report

**Phase Goal:** A LibFuturo integrator can install an identified current Nesturbator release, replay one permitted NTSC game into interactive play through its public C API, and use a provider profile bound to that exact package.

**Verified:** 2026-11-03T21:08:06Z  
**Status:** gaps_found  
**Re-verification:** Yes — refreshed after acceptance and summary updates

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | An integrator can obtain the published library, runner, and libretro artifacts, verify checksums, and identify one consistent accepted source, version, ABI, behavior revision, and state format. | ✗ FAILED | Identity 08a0d1ed2cf738c428e81dead9407b9b695a8608 and three local archives are consistent, but they are not published release artifacts. Hosted CI and the published receipt are pending. |
| 2 | A clean C consumer builds and runs using only a relocated install prefix, loads permitted content, advances frames with input, receives video/audio, and continues the active game after a refused replacement. | ✓ VERIFIED | Exact-candidate install.gameplay passed in the retained 726-test workflow. The hash-bound report records 110 frames, NTSC video/audio, refusal status 7, preserved state, and 30 equal continuation frames. The driver moves the prefix, rebuilds without the old consumer build, and confines package/header/library lookup to that prefix. |
| 3 | A manifest-permitted NTSC NROM reaches a source-defined interactive transition with input-caused progress, visible response, and non-silent gameplay audio. | ✓ VERIFIED | The positive runtime report records the DABG state transition, X=40→70 under Right input, all four gameplay predicates true, 12,647 nonzero PCM samples, and 238 changed native pixels. The exact-candidate install.gameplay test passed. |
| 4 | An integrator can use a version-bound profile to identify the tested package/source and its NTSC contract, with unsupported capabilities marked unqualified. | ✗ FAILED | The contract is checked against the current header and runtime, and the acceptance identity binds profile and scenario digests to the exact local source. The profile remains candidate-only; final package/source/version binding awaits the published receipt. |
| 5 | The accepted local gameplay run combines input-caused RAM progress, visible response and non-silent PCM; no-input and Start-only controls fail the complete gameplay verdict. | ✓ VERIFIED | The owned consumer and controls are part of the exact-candidate workflow; install.gameplay passed. The report's source-backed game-state, progress, pixel, and PCM predicates are true. |
| 6 | A malformed replacement is refused while the original game continues identically to a twin receiving the same later inputs. | ✓ VERIFIED | The report records refusal status 7, refusal_preserved=true, continuation_equal=true, 30 continuation frames, and continued player movement. |
| 7 | Package discovery, public-header compilation, and library linkage stay inside the moved prefix after the original prefix and earlier consumer build are removed. | ✓ VERIFIED | The consumer driver relocates the package, removes the earlier build, reconfigures, checks imported include/library/link paths, and uses prefix-only package discovery. install.gameplay passed. |
| 8 | An NTSC preference preserves explicit PAL metadata, Dendy is refused, and only effective NTSC content enters this first qualified route. | ✓ VERIFIED | Region matrix assertions and profile checks cover explicit metadata, preference, and Dendy refusal. The retained exact-candidate workflow passed. |
| 9 | The profile states size/null/empty behavior and one-below/exact output-buffer bounds without treating an error as a completed frame. | ✓ VERIFIED | The profile documents these cases, and retained core.frame plus installed-consumer checks passed in the bounded workflow. |
| 10 | Exact NTSC frame/sample fractions, remainder carry, cumulative ticks, and valid audio counts define output cadence without rounded-rate drift. | ✓ VERIFIED | Profile values match source arithmetic and runtime rates/counts; retained core frame and provider profile checks passed. |
| 11 | Adjacent public frame calls retain one controller-input snapshot each, with no mixed mask across a crossing instruction. | ✓ VERIFIED | The new test_public_frames_snapshot_inputs_across_instruction_overrun uses consecutive public nesturbator_run_frame calls with distinct masks. It asserts the first instruction completes past the nominal target with A=1, then the next call samples the new mask with A=0. core.controller passed in the exact-candidate retained log. |
| 12 | The publication checker rejects absent, draft, stale, or identity-inconsistent evidence and distinguishes local archives from published inventory. | ✓ VERIFIED | Offline receipt controls and the local/handoff gates passed in the retained workflow. This verifies checker behavior, not that a live release exists. |
| 13 | The candidate has a single source/input identity, consistent package/header metadata, and fresh local workflow and relocated-gameplay evidence. | ✓ VERIFIED | Candidate source and tree match the accepted identity. All 423 frozen inputs match their recorded SHA-256 values. CTest discovery, LastTest.log, and gameplay report hashes match the identity. |
| 14 | The release lineage preserves v0.1.12 as the observed published baseline and lets Release Please calculate the next version. | ✓ VERIFIED | Version metadata and Release Please configuration preserve the v0.1.12 baseline without claiming a subsequent published version. |
| 15 | Final acceptance requires a non-draft release with exact tag/source/version, compatibility metadata, 18 verified archives, SHA256SUMS, and attestations. | ✓ VERIFIED | The published-mode receipt gate enforces these requirements; offline controls passed. No live publication is inferred from those checks. |
| 16 | The final provider receipt binds a published native package's relocated gameplay/refusal run to the fixed profile and independently supported scenario. | ✗ FAILED | The writer and validation path are wired, but docs/provider-ntsc-release.json is absent because no published package has been replayed through the receipt gate. |
| 17 | Local readiness, exact-candidate CI, resulting-main CI, and published acceptance remain separate; absent publication leaves PKG-01 pending. | ✓ VERIFIED | The refreshed identity and acceptance distinguish local-ready from pending candidate CI, main CI, publication, and PKG-01. |

**Score:** 14/17 truths verified (0 present, behavior-unverified)

Plan 28-01's general consumer/gameplay truths and Plan 28-02's profile truth that restate roadmap criteria are deduplicated in this score. The detailed plan truths remain represented above.

### Deferred Items

None. The milestone roadmap has no later phase that explicitly completes these release gates; they are the separate shipping step.

### Advisory (New Scope, Unevidenced)

None. This re-verification found no new-scope blocker.

### Required Artifacts

| Artifact | Expected | Status | Details |
|---|---|---|---|
| tests/cmake/qualified_consumer.cmake | Install, relocate, configure, build, and run driver | ✓ VERIFIED | Present, substantive, registered as install.gameplay, and passed in the retained exact-candidate workflow. |
| tests/consumer/main.c | Public-header consumer with gameplay and refusal controls | ✓ VERIFIED | Uses the installed public API; positive route, controls, buffer bounds, and refusal continuation are exercised. |
| docs/nrom-gameplay.md | Pinned gameplay evidence and bounded recipe | ✓ VERIFIED | Its changed-pixel count is 238, matching the identity-bound runtime report. Its SHA-256 matches the identity and profile scenario digest. |
| docs/provider-ntsc.json | Machine-readable NTSC provider contract | ✓ VERIFIED, candidate-only | Profile controls passed and the identity binds its digest. Exact release binding remains intentionally pending. |
| tests/core/test_controller.c | Public frame-boundary input regression | ✓ VERIFIED | The crossing-instruction test is substantive, called from the test entry point, registered as core.controller, and passed in the retained run. |
| tests/cmake/provider_profile.cmake | Offline profile/runtime consistency checks | ✓ VERIFIED | Present and registered; profile checks passed in the retained workflow. |
| tests/cmake/provider_receipt.cmake | Offline controls and fail-closed publication gate | ✓ VERIFIED | Present and registered; receipt controls passed. Published mode has not run against a release. |
| .planning/phases/28-qualified-nes-gameplay-package/28-IDENTITY.json | Exact source, build, profile, and scenario identity | ✓ VERIFIED | Source commit 08a0d1ed2cf738c428e81dead9407b9b695a8608, 423 input hashes, report hash, and local archive digests are internally consistent. |
| .planning/phases/28-qualified-nes-gameplay-package/28-ACCEPTANCE.md | Separation of local, hosted CI, and publication evidence | ✓ VERIFIED | Records local acceptance for this source and explicitly leaves hosted CI and publication pending. |
| docs/provider-ntsc-release.json | Published package/profile/scenario receipt | ✗ MISSING | Required final artifact; absent until the published release and native replay gate pass. |

The candidate code review and the three-file review delta are clean. The current exact-candidate acceptance report states the full bounded workflow passed all 726 registered tests, including core.controller, install.gameplay, and provider checks. The three local archives do not satisfy the published artifact requirement.

### Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| tests/CMakeLists.txt | tests/cmake/qualified_consumer.cmake | install.gameplay CTest registration | WIRED | Registered and passed in the exact-candidate workflow. |
| tests/consumer/CMakeLists.txt | relocated nesturbator::nesturbator | Prefix-only find_package | WIRED | Discovery, imported paths, public headers, and library linkage are checked under the moved prefix. |
| tests/consumer/main.c | docs/nrom-gameplay.md | RAM allowlist and input recipe | PARTIAL | Runtime evidence agrees with the documented recipe, but no automated source check compares all consumer constants to guide prose. |
| docs/provider-ntsc.md | include/nesturbator.h | Public API contract citations | WIRED | The profile describes the relevant API, status, frame, video, audio, and lifecycle behavior. |
| tests/cmake/provider_profile.cmake | docs/provider-ntsc.json | Header, scenario, and runtime consistency | WIRED | Contract and installed-runtime profile validation passed. |
| tests/cmake/provider_receipt.cmake | docs/provider-ntsc-release.json | Published receipt output | WIRED; output pending | Writer is gated by live release, archive, checksum, attestation, CI, and native replay evidence. |
| 28-IDENTITY.json | docs/provider-ntsc.json | Frozen profile/scenario digests | WIRED | Identity digests match the current profile and guide; this is candidate-local binding. |
| 28-ACCEPTANCE.md | .github/workflows/release.yml | Hosted CI and publication gates | WIRED; results pending | Handoff names the required checks and does not claim they ran on this candidate or resulting main. |

### Data-Flow Trace (Level 4)

| Artifact | Data variable | Source | Produces real data | Status |
|---|---|---|---|---|
| build/ci/qualified-consumer-521294fbd128/gameplay.json | Game state, input progress, pixels, PCM, refusal continuation | Relocated installed Nesturbator library called by the public C consumer | Yes; 110-frame DABG replay and output checks are recorded | FLOWING |
| docs/provider-ntsc.json | Contract and scenario fields | Public header/source arithmetic, guide digest, and installed-runtime validation | Yes; profile checks compare values to implementation and report | FLOWING |
| docs/provider-ntsc-release.json | Published release identity and replay result | Live published assets and published-mode native replay | No; receipt is absent pending shipping | DISCONNECTED |

### Behavioral Spot-Checks

| Behavior | Retained command/evidence | Result | Status |
|---|---|---|---|
| Public frame input snapshot across an instruction overrun | core.controller in build/ci/phase28-local-frame-sampling/LastTest.log | Passed; exact-candidate log hash matches identity. No test was run during this verification. | ✓ PASS |
| Relocated installed gameplay and refused-load continuation | install.gameplay in the same retained log and hash-bound gameplay report | Passed; report records positive predicates, refusal preservation, and equal continuation. | ✓ PASS |
| Profile and receipt offline controls | provider.profile, provider.profile.controls, provider.receipt.controls in the retained log | All passed; receipt controls validate gates, not a published release. | ✓ PASS |

### Probe Execution

No phase-declared probe and no conventional scripts/*/tests/probe-*.sh probe exists for this phase.

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|---|---|---|---|---|
| PKG-01 | 28-02, 28-03 | Published exact-source package with library, runner, libretro artifacts, and checksums | ✗ BLOCKED | Hosted checks and published receipt are pending; local archives are not a release. |
| INST-01 | 28-01, 28-03 | Relocated installed consumer with permitted load, input, video/audio, and refusal continuation | ✓ SATISFIED | Exact-candidate install.gameplay passed and its report is hash-bound to the 423-input identity. |
| PROF-01 | 28-02, 28-03 | Accurate profile bound to exact package/source, with unsupported capabilities unqualified | ✗ BLOCKED | Contract facts and candidate profile are verified, but exact published package/source binding remains pending the receipt. |
| GAME-01 | 28-01, 28-03 | Permitted game reaches source-backed progress, visible response, and non-silent audio | ✓ SATISFIED | DABG manifest identity, runtime predicates, install.gameplay, and matching 238-pixel count are verified. |

All four phase requirements appear in the plans and in REQUIREMENTS.md; none is orphaned. The project requirement tracker correctly leaves PKG-01 and PROF-01 pending.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---:|---|---|---|
| tests/cmake/provider_profile.cmake | 1042 | placeholder in deliberately invalid self-test JSON | ℹ️ INFO | Test-only malformed input; it does not flow to a user-visible result. |

No blocking debt markers, empty implementations, or misleading static output were found in the changed implementation and documentation. The previous 119-versus-238 metric discrepancy is closed: the guide says 238 of 960, the hash-bound gameplay report says 238, and the profile's guide digest matches both the guide and accepted identity. Review delta status is clean with no findings.

### Plan-Marked Prohibitions

Plans 28-02 and 28-03 leave their transparency prohibitions unresolved and descriptor-less. The current profile explicitly marks host playback, PAL, device pacing, and Playstead as unqualified, and no contradictory claim was found. These plan descriptors remain flagged-unverified; this judgment is not owner acceptance or a declaration that the descriptors are resolved. Human review is recommended when the release handoff is reviewed.

### Human Verification Required

No separate gameplay UAT is required by this phase. This verification does not claim hardware correctness or LibFuturo consumer/host playback; those outcomes are explicitly outside Nesturbator's acceptance boundary.

### Gaps Summary

The refreshed candidate is 08a0d1ed2cf738c428e81dead9407b9b695a8608 (tree 51e8787f052cb5658dacff3ab49445b9f0430b33). All 423 frozen implementation inputs match their identity hashes, and the retained 401-second local workflow passed all 726 tests. The new controller regression closes the frame-boundary behavior finding, and the corrected 238-pixel documentation value matches the bound runtime report and profile digest.

Phase 28's local gameplay and profile contract evidence is verified, but the release goal is not complete. Exact-candidate hosted CI and resulting-main CI remain pending. No published receipt exists, so PKG-01 and PROF-01's exact package/source binding remain pending. Local archives are not treated as a published release. The updated handoff routes the next step through $gsd-discuss-phase 28 to reconcile the verification gaps with the phase's shipping boundary before any gap plan; preserve unrelated dirty work. No tests were run during this verification.

---

_Verified: 2026-11-03T21:08:06Z_  
_Verifier: the agent (gsd-verifier)_
