---
phase: 20-nmi-line-and-vblank-flag
verified: 2026-10-28T15:23:00Z
status: passed
score: 32/32 truths verified
covered_files:
  - .githooks/pre-push
  - .github/workflows/ci.yml
  - .github/workflows/nightly.yml
  - .planning/phases/20-nmi-line-and-vblank-flag/20-01-PLAN.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-01-SUMMARY.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-02-PLAN.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-02-SUMMARY.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-03-PLAN.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-03-SUMMARY.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-04-PLAN.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-04-SUMMARY.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-05-PLAN.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-05-SUMMARY.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-06-PLAN.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-06-SUMMARY.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-07-PLAN.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-07-SUMMARY.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-08-PLAN.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-08-SUMMARY.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-09-PLAN.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-09-SUMMARY.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-10-PLAN.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-10-SUMMARY.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-11-PLAN.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-11-SUMMARY.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-12-PLAN.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-12-SUMMARY.md
  - CMakePresets.json
  - README.md
  - include/nesturbator.h
  - scripts/check_push_format.py
  - src/apu.c
  - src/bus.c
  - src/cpu.c
  - src/internal.h
  - src/ppu.c
  - src/state.c
  - src/synth.c
  - tests/CMakeLists.txt
  - tests/accuracy/baseline-metadata.txt
  - tests/accuracy/measurements.md
  - tests/accuracy/migration.tsv
  - tests/accuracy/scoreboard.txt
  - tests/cmake/accuracycoin_activation.cmake
  - tests/cmake/accuracycoin_baseline_fetch.cmake
  - tests/cmake/accuracycoin_driver.cmake
  - tests/cmake/accuracycoin_nmi_targets_control.cmake
  - tests/cmake/accuracycoin_regression_controls.cmake
  - tests/cmake/ci_required_rollup.cmake
  - tests/cmake/fetch_vectors.cmake
  - tests/cmake/hash_inventory.cmake
  - tests/cmake/nightly_workflow_policy.cmake
  - tests/cmake/nmi_targets.cmake
  - tests/cmake/phase20_hosted_evidence.cmake
  - tests/cmake/phase20_hosted_evidence_controls.cmake
  - tests/cmake/runner_state.cmake
  - tests/cmake/session_inventory.cmake
  - tests/cmake/test_push_format.py
  - tests/cmake/vector_api_policy.cmake
  - tests/cmake/vector_registration_policy.cmake
  - tests/cmake/vectors_transport.cmake
  - tests/core/test_api.c
  - tests/core/test_apu.c
  - tests/core/test_nmi_poll.c
  - tests/core/test_nmi_state.c
  - tests/core/test_state.c
  - tests/core/test_state_fork.c
  - tests/core/test_state_reject.c
  - tests/core/test_state_runahead.c
  - tests/fuzz/state_loader.c
  - tests/header/header_c.c
  - tests/header/header_cxx.cpp
  - tests/libretro/libretro_host.c
  - tests/nmi_fixture.h
  - tests/ppu/test_mmc3.c
  - tests/ppu/test_nmi_races.c
  - tests/ppu/test_registers.c
  - tests/ppu/test_sprites.c
  - tests/retroarch/CMakeLists.txt
  - tests/retroarch/frame_index.c
  - tests/retroarch/test_state_session.sh
  - tests/runahead_model.h
  - tests/runner/hashes.txt
  - tests/runner/nmi_fixture.c
  - tests/runner/phase20_fake_gh.c
  - tests/state_test.h
covered_digest: "v4:sha256:4cee7999c8b76857555a18f0823a78e949b3ed3898ef1fe06160d18994e1e5ba"
behavior_unverified: 0
overrides_applied: 0
re_verification:
  previous_status: gaps_found
  previous_score: 26/32
  gaps_closed:
    - "Roadmap SC-02/SC-04 and D-22: exact-HEAD six-platform hashes, protected PASS retention, performance, conformance, and real-host evidence."
    - "D-04: production-decoder $2002 status bits are discriminated across the M2 rise and fall samples."
    - "KEEP-02 adjacency: equal-tick frame/audio/state keys remain distinct and duplicate/missing-key controls reject."
    - "KEEP-02 empty: frame-zero, empty-PCM state preserves canonical bytes and nonzero audio history without fabricating PCM."
  gaps_remaining: []
  regressions: []
advisory: []
unverified_prohibitions:
  - statement: "Implementation and test evidence must never incorporate GPL/LGPL emulator source or unmanifested ROM/BIOS bytes."
    verification: judgment
    disposition: unverified
    flagged: true
    evidence: "Automated hygiene checks passed in the fresh CI workflow and this scan found no contamination. No automated provenance policy fully closes this judgment-tier prohibition; it remains an explicit bounded flag, with no manual sign-off gate."
---

# Phase 20: NMI line and VBlank flag Verification Report

**Phase Goal:** A game that reads $2002 or toggles NMI around the start or end of VBlank gets the console's flag value and exactly the console's NMIs, and its save states hold through that race.

**Verified:** 2026-10-28T15:23:00Z
**Status:** passed
**Re-verification:** Yes — after gap closure in Plans 20-07/08/09/11/12/10. Plan 20-10 changed evidence and validation only.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | All six named AccuracyCoin rows and owned race reproducers pass with source-selected whole-sequence values. | ✓ VERIFIED | The immutable hosted campaign has all six exact NMI target keys passing. The clean Linux campaign records 22/22 pages, 146 scored rows, 114 PASS/32 FAIL, zero skipped/not-run/timeout, and five DRAW. Exact-key and wrong-row controls pass. |
| 2 | All six supported platforms produce identical frame/audio hashes, with every moved pin explained. | ✓ VERIFIED | Six lane inventories each contain 114 frame/PCM/transition/state rows and are byte-identical (SHA-256 1a06cbabea1fee522e023a405af4474726ad24ef798d067b5e27575cf505bc62). The measurements document causes for moved values. Hosted source identity is independently rechecked below. |
| 3 | VBlank-set, VBlank-clear, and edge-before-poll states replay through core, runner, and libretro with format 2 and format-1 refusal. | ✓ VERIFIED | Core state cutpoints, runner state commands, and libretro host serialization exercise production save/load paths. Format 2 is pinned; malformed and format-1 states reject. The fresh 687-test CI run passes the NMI state, runner roundtrip/refusal/idempotency, core state-fork, and libretro coverage. |
| 4 | No pre-existing passing main row is lost on any CI lane. | ✓ VERIFIED | Each of six hosted lanes retains all 108/108 protected PASS keys. Fresh protected-observation, protected-verdict, and no-lost-pass terminals pass. |
| 5 | D-01: Owned PPU-dot, real-bus/program, and pinned AccuracyCoin evidence use separate oracles. | ✓ VERIFIED | PPU dot fixtures, production bus/CPU guest programs, and the pinned campaign are separate checks; all are present and passing. |
| 6 | D-02: Expected values identify source, sample coordinates, starting state, and event order rather than copying measured output. | ✓ VERIFIED | Test comments, the NMI oracle research, preparation sources, and measurement records give source references, cutpoints, initial state, and event order. The independent probe output is used only within its documented finite-margin scope. |
| 7 | D-03: Start/end boundary, suppression, retained-edge, mirror, and normal-control cases are distinguished. | ✓ VERIFIED | Registered ppu.nmi_races and core.nmi_poll tests cover before/on/after edges, suppression, retained capture, mirrored $2002 addresses, and normal controls. Both named tests passed in the fresh CI log and targeted run. |
| 8 | D-04: A $2002 read samples bit 7 at M2 rise and bits 6/5 at M2 fall while preserving read side effects. | ✓ VERIFIED | The new test changes sprite overflow between dot 130 at +9 and dot 131 at +23, then reads through production bus decoding at $2002 and mirror $3FFA. It asserts result 0x35, open-bus and I/O values, status/address-latch effects, 24 ticks, and one CPU cycle. The targeted ppu.nmi_races test passes. |
| 9 | D-05: NMI enable/disable, unchanged $80, $00->$80, cleared flag, both boundaries, and STA/INC markers produce source-expected extra NMIs. | ✓ VERIFIED | Guest programs execute STA and both writes of INC, then check handler and following-instruction markers. AccuracyCoin Control 8/9 and the six exact NMI rows pass. |
| 10 | D-06: Green guards run normally and expected-red reproducers cannot pass on setup failure, crash, or unrelated assertion. | ✓ VERIFIED | The expected-red harness checks for the named assertion and normal termination; fixed targets no longer depend on expected-red runs. CI and the owned regression controls pass. |
| 11 | D-07: Bounded wrong-model controls hit their distinct intended oracle. | ✓ VERIFIED | PPU race and CPU poll tests carry controls for edge timing, capture revocation, unchanged enable, RMW write loss, early service, and table shifts; the named tests pass. |
| 12 | D-08: Derived NMI line, previous-line sample, and captured request remain separate; lowering the line cannot revoke captured work. | ✓ VERIFIED | PPU line derivation, bus edge capture, and CPU poll state are separate fields and paths. Guest-marker and save-state tests exercise deassertion after capture. |
| 13 | D-09: +9/+15 half-master-tick accesses, register side effects, APU phase, mapper stamps, and DMA accesses remain coherent. | ✓ VERIFIED | The production bus cycle keeps the ordered catch-ups and one-time register side effects. Bus, mapper, DMA, and NMI regressions pass in the fresh CI workflow. |
| 14 | D-10 amended: finite-margin circuit evidence constrains capture with one global deterministic tie policy and no unsupported physical-tie winner claim. | ✓ VERIFIED | The probe passed six independent fresh-state finite-margin pulse cases. The documented global tie order is bus side effect, detector capture, then tied PPU transition. Exact paired-chip tie precedence is an accepted evidence limit and is not claimed as a measured physical winner. |
| 15 | D-11: Default alignment is tested against complete tables before any global phase adjustment. | ✓ VERIFIED | Measurements record default alignment with no per-test offset or VBlank-dot shift; complete alternatives and absolute-dot witnesses pass. |
| 16 | D-12: Each source table accepts only complete alternatives and explicitly ignored cells, with phase/history instance-local. | ✓ VERIFIED | Full source vectors are retained, and owned controls reject mixed alternatives. Running phase/history lives in each emulator instance. |
| 17 | D-13: PPU-to-CPU line boundary and IRQ writer ownership remain at existing seams. | ✓ VERIFIED | PPU derives the line, bus samples it, and CPU consumes the captured poll state. IRQ ownership remains at its existing seam; the CI regression suite passes. |
| 18 | D-14: State format/revision transition to format 2/revision 8 preserves error precedence and refusal atomicity. | ✓ VERIFIED | The public format pin, loader checks, malformed-state controls, and state.format_pin test pass. |
| 19 | D-15: Persistent capture/poll history is serialized, validated, and reset with the causal change. | ✓ VERIFIED | state.c serializes and validates nmi_prev, nmi_pending, and nmi_polled; CPU reset clears them. State rejection and NMI replay tests pass. |
| 20 | D-16: CRC-correct malformed bounds and unreachable states reject before mutation; real histories remain loadable and canonical. | ✓ VERIFIED | Production rejection tests validate atomic refusal; NMI state tests perform canonical save/load/save and compare state bytes. |
| 21 | D-17: Exact internal cutpoints use production visitor/loader at resumable boundaries, labeling constructed origins and executed endpoints. | ✓ VERIFIED | NMI state output labels the constructed 240:333 origin separately from the instruction-reached 241:1 endpoint and other executed race cutpoints. |
| 22 | D-18: Exact set/clear states include coherent nonzero audio history, and edge-before-poll feasibility is bounded. | ✓ VERIFIED | Measurements record audio/APU values at exact coordinates and the bounded Control 8/9 sweep. The finite-margin probe independently bounds six pulse cases. |
| 23 | D-19: Restored continuation matches native state, frame/tick/audio, and independent guest markers, including omitted-history and instance controls. | ✓ VERIFIED | Core state-fork, runner, and libretro replay compare continued state and output; omitted-history and independent-instance controls pass. |
| 24 | D-20: Poll behavior, tests, state, docs, hashes, and campaign evidence move together as a causal change. | ✓ VERIFIED | Runtime, tests, README/public header comments, hash measurements, scoreboards, and baseline provenance are included in the candidate; fresh CI passes. |
| 25 | D-22: Pins, schemas, caps, CI placement, six hash lanes, conformance checks, and the +1% performance guard remain enforced. | ✓ VERIFIED | Workflow/policy tests pass, and hosted CI, Nightly, performance, vector/MMC3 conformance, boot-to-play, and real RetroArch artifacts are all tied to the tested merge and validated source bundle. |
| 26 | D-23: Changed hashes have old/new measurements and causes, with truthful campaign fingerprint and dirty-source metadata. | ✓ VERIFIED | Measurement records explain old/new frame/audio/state values and local source_dirty=1 metadata truthfully. Separate hosted clean-source campaign records source_dirty=0, with its report and 56-input fingerprint retained. |
| 27 | D-24: Running state remains owned, integer-only, allocation-free, per-instance, and within the three deliverables. | ✓ VERIFIED | Runtime state remains per-instance with integer fields; two-instance checks pass. The changed source keeps the library, runner, and libretro adapter deliverables and adds no runtime dependency or device layer. |
| 28 | KEEP-03 idempotency: save/load/save at a valid boundary yields identical canonical bytes and continuation. | ✓ VERIFIED | Production save/load/save compares canonical bytes and resumed guest/video/tick/audio state. The targeted core.nmi_state test passes. |
| 29 | KEEP-03 concurrency: interleaved instances with different NMI history remain independent. | ✓ VERIFIED | State-fork and two-instance tests interleave independent histories and compare each continuation; core.state_fork.nmi passes. |
| 30 | KEEP-02 adjacency: equal-tick frame/audio/state keys remain distinct and duplicate/missing-key controls reject. | ✓ VERIFIED | hash_inventory.cmake constructs frame 60, state 60, PCM, and transition keys at tick 42886728 with five distinct digests. All-six duplicate/missing controls and artifact-equality checks run in hash.inventory.self_test, which passes. |
| 31 | KEEP-02 empty: frame-zero/empty-PCM state preserves canonical bytes and nonzero audio history without inventing PCM. | ✓ VERIFIED | test_frame_zero_empty_pcm_preserves_history reaches 241:1 from a labeled constructed origin, saves/restores nonzero synth history with pcm_count=0 and sample_output=NULL, checks identical canonical bytes, verifies empty drains preserve a sentinel and state, then checks continuation against a history-reset negative control. The targeted core.nmi_state test passes. |
| 32 | KEEP-02 ordering: generated complete hash inventory is deterministic and rejects equal keys. | ✓ VERIFIED | runner.write_hashes, hash.inventory.self_test, and state.format_pin pass; generated inventory matches the checked-in inventory. |

**Score:** 32/32 truths verified (0 present, behavior-unverified).

### Re-verification Closure

All four carried-forward gaps are closed by direct artifacts and current behavior checks:

1. Hosted exact-source evidence is present, source-validated, and independently mode/blob-checked across all 417 effective files.
2. The split-sample $2002 regression asserts values and side effects through the production decoder.
3. The equal-tick inventory fixture proves distinct full keys and rejects duplicate/missing keys.
4. The frame-zero empty-PCM state witness proves preserved audio history, canonical state, empty drain behavior, and deterministic continuation.

No regressions were found in the previously verified truths. The original score was 26/32; current score is 32/32.

### Required Artifacts

All 12 plan artifact queries passed: 41/41 artifacts exist and are substantive. Code, tests, and CMake registrations were traced to production execution; no required artifact is a placeholder.

| Plan | Artifacts | Status | Details |
|---|---:|---|---|
| 20-01 | 4/4 | ✓ VERIFIED | PPU race tests, CPU poll tests, fixture, and CTest registration. |
| 20-02 | 4/4 | ✓ VERIFIED | Bus/PPU split samples, state contract, and public header pin. |
| 20-03 | 4/4 | ✓ VERIFIED | CPU poll/history fields, serializer, and program tests. |
| 20-04 | 3/3 | ✓ VERIFIED | NMI state witnesses, rejection controls, and measurement ledger. |
| 20-05 | 4/4 | ✓ VERIFIED | Core state fork, runner state fixture/commands, and libretro host. |
| 20-06 | 3/3 | ✓ VERIFIED | Exact target gate, baseline metadata, and phase evidence ledger. |
| 20-07 | 3/3 | ✓ VERIFIED | PPU race source, public semantics, and review disposition. |
| 20-08 | 3/3 | ✓ VERIFIED | Full inventory adjacency test and combined save-state witness. |
| 20-09 | 4/4 | ✓ VERIFIED | Hosted evidence collector, controls, and CI workflow. |
| 20-10 | 2/2 | ✓ VERIFIED | Immutable hosted acceptance evidence and old/new hash measurements. |
| 20-11 | 4/4 | ✓ VERIFIED | Collector acceptance, negative controls, and fake GitHub transport. |
| 20-12 | 3/3 | ✓ VERIFIED | Delivery record and live immutable-bundle availability metadata. |

### Key Link Verification

| From | To | Via | Status | Evidence |
|---|---|---|---|---|
| tests/ppu/test_nmi_races.c | src/bus.c | nesturbator__bus_read on $2002 and $3FFA | WIRED | Production decoder returns composed status and applies one-read side effects; split-sample test asserts both addresses. |
| src/ppu.c | src/bus.c | nesturbator__ppu_nmi_line | WIRED | Bus cycle samples the derived PPU line and captures rising edges. |
| src/bus.c | src/cpu.c | nmi_pending/nmi_polled fields | WIRED | Poll reads captured request state; CPU consumes it at instruction boundary. |
| src/internal.h | src/state.c | instance history serializer and validator | WIRED | nmi_prev, nmi_pending, and nmi_polled are serialized and constrained. |
| tests/core/test_nmi_state.c | src/state.c, src/cpu.c | public save/load then resumed execution | WIRED | Canonical state and continued output are compared through production code. |
| tests/cmake/runner_state.cmake | tests/runner/nmi_fixture.c | CLI save/load fixture | WIRED | Runner test compares restored suffix and guest markers. |
| tests/libretro/libretro_host.c | libretro adapter | retro_serialize/retro_unserialize | WIRED | Host interface exercises restore and continued frame/audio output. |
| tests/cmake/nmi_targets.cmake | tests/accuracy/scoreboard.txt | exact six-key gate and negative controls | WIRED | Missing, duplicate, mistyped, or failed keys reject. |
| .github/workflows/ci.yml | platform hash artifacts | six-lane artifacts and hash inventory comparison | WIRED | Raw hosted lane evidence and equality validators are present in the bundle. |
| tests/cmake/phase20_hosted_evidence.cmake | immutable run/artifact records | collect then validate source-bound manifest | WIRED | MODE=validate rechecks current checkout against the retained bundle. |

The query key-link heuristic under-reports some C/CMake call edges because its static pattern matcher does not trace these internal symbols. Each listed connection was checked directly at the call site and through the invoked test or artifact. This was not treated as evidence of an orphan.

### Data-Flow Trace

| Artifact | Real source | Status |
|---|---|---|
| AccuracyCoin rows | Pinned guest execution, produced marker/report bytes, exact six-key gate | ✓ FLOWING |
| NMI status and poll results | PPU status/control → bus sample/capture → CPU poll → guest markers | ✓ FLOWING |
| Save-state history | Instance NMI fields → format-2 serializer/validator → production restore → continued execution | ✓ FLOWING |
| Native frame/audio/state inventory | Runner outputs → full-key inventory validation → six platform artifacts | ✓ FLOWING |
| Hosted acceptance bundle | Raw run/job/artifact data → provenance validators → immutable manifest/evidence digest | ✓ FLOWING |

None of these chains ends in a static response, mock, or hardcoded pass value.

### Behavioral Spot-Checks

| Behavior | Command or artifact | Result | Status |
|---|---|---|---|
| Fresh whole-project CI workflow | cmake --workflow --preset ci | Exit 0; 687/687 tests in 399.45 s; library, runner, and libretro packages generated. Log: build/phase20-execute-regression.log. | ✓ PASS |
| Named NMI and state checks | ctest --test-dir build/ci --output-on-failure -R '^(ppu.nmi_races|core.nmi_poll|core.nmi_state|hash.inventory.self_test|core.state_fork.nmi|runner.state.nmi_roundtrip)$' | All six intended tests passed; CTest also selected runner.save.fixture; total 7/7 passed in 0.48 s. | ✓ PASS |
| Current checkout against hosted bundle | cmake -DMODE=validate -DROOT=build/phase20-hosted-evidence -DSOURCE_REF="$PWD" -P tests/cmake/phase20_hosted_evidence.cmake | Exit 0; hosted phase20 evidence revalidated against the working checkout. | ✓ PASS |
| NMI circuit finite-margin probe | python3 .planning/phases/20-nmi-line-and-vblank-flag/probes/20-nmi-netlist-oracle-fetch.py | Exit 0; six independent fresh-state pulse cases passed; no exact clock ties were claimed. | ✓ PASS |
| Exact hosted target/conformance evidence | Immutable bundle build/phase20-hosted-evidence | Six NMI rows pass; all six lanes retain 108 protected PASS keys and 114 identical hash rows; performance, vectors, MMC3, boot-to-play, and real host checks pass. | ✓ PASS |

### Probe Execution

| Probe | Command | Result | Status |
|---|---|---|---|
| Phase 20 netlist oracle probe | python3 .planning/phases/20-nmi-line-and-vblank-flag/probes/20-nmi-netlist-oracle-fetch.py | Exit 0; six finite-margin pulse cases passed. | PASS |
| Conventional scripts/*/tests/probe-*.sh probes | Discovery | None found or declared by the phase. | N/A |

### Requirements Coverage

| Requirement | Source Plans | Status | Evidence |
|---|---|---|---|
| NMI-01 | 20-01, 20-03, 20-06 | SATISFIED | Pinned $2002 flag/timing/suppression rows and owned production-bus tests pass. |
| NMI-02 | 20-01, 20-03, 20-06 | SATISFIED | NMI control, VBlank-end, and disabled rows pass; STA/INC guest markers verify the extra interrupt sequence. |
| KEEP-02 | 20-04, 20-06, 20-08, 20-10 | SATISFIED | Old/new causes, six-platform identity, equal-tick adjacency controls, and frame-zero empty-PCM history witness all pass. |
| KEEP-03 | 20-04, 20-05, 20-06 | SATISFIED | Core/runner/libretro replay, format-2 pin, format-1 refusal, and canonical continuation checks pass. |

REQUIREMENTS.md maps exactly these four IDs to Phase 20; there are no orphaned Phase 20 requirements. Its checkboxes and mapping statuses were left untouched as instructed.

### Anti-Patterns and Deferred Review Debt

| Finding | Severity | Current assessment |
|---|---|---|
| CR-01: hosted collector does not enforce Git file modes in its recurring source identity comparison | WARNING, explicitly deferred | Independent comparison matched all 417 current index (path, mode, blob SHA) entries with candidate, CI-tested, and Nightly-tested trees. The retained manifest is build/phase20-source-modes.tsv, SHA-256 d799defc0b6be54d5e9ff8e00beae68fb003fe06e78bb4d6c240afd055117818. This closes the current-run evidence question; collector enforcement remains maintenance debt. |
| WR-03: collector recursively scans source root and can encounter ignored build artifacts | WARNING, explicitly deferred | Successful collection used absolute SOURCE_REF to a clean equivalent checkout. Current checkout MODE=validate passed against the immutable bundle. The source-root scan weakness remains a future robustness concern. |
| WR-04: nightly validation function has a duplicate identical definition | WARNING, explicitly deferred | The later definition replaces the earlier CMake function with the same body; the tested Nightly workflow and bundle validation pass. Remove the duplication during future collector maintenance. |

No unresolved debt-marker comments or placeholder implementations were found in changed functional code. The lowercase xxxM text in a mapper mask comment is hardware bit notation; return-NULL branches in tests are fixture/allocation failure handling, not no-op implementations. The fresh CI workflow includes hygiene checks. Decision coverage is 24/24: all trackable CONTEXT.md decisions are honored by shipped artifacts.

### Accepted Evidence Limits and Human Verification

No manual behavior or provenance sign-off is required. The judgment-tier clean-room prohibition remains visibly flagged in frontmatter and is not silently resolved by this verification. D-10 accepts a deterministic global tie policy because the exact paired-chip physical tie is not directly measured; the report makes no physical winner claim. These accepted bounds do not leave an observable phase truth unverified.

### Gaps Summary

The four gaps carried from the 26/32 report are closed by new test and hosted evidence. All 32 observable truths are verified. CR-01, WR-03, and WR-04 remain non-blocking, explicitly deferred collector-maintenance debt; they do not invalidate this source-bound acceptance because current content/mode identity and validation were independently reproduced.

---

_Verified: 2026-10-28T15:23:00Z_
_Verifier: gsd-verifier agent_
