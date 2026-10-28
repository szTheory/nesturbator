---
phase: 20-nmi-line-and-vblank-flag
verified: 2026-10-28T19:13:07Z
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
  - .planning/phases/20-nmi-line-and-vblank-flag/20-EVIDENCE.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-REVERIFICATION-EVIDENCE.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-REVIEW-DISPOSITION.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-REVIEW.md
  - CMakePresets.json
  - README.md
  - build/phase20-refresh-hosted-evidence/manifest.txt
  - build/phase20-refresh-hosted-evidence/verdict.txt
  - build/phase20-refresh-source-modes.tsv
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
  - tests/cmake/session_helpers.cmake
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
  - tests/retroarch/state_session.sh
  - tests/retroarch/test_state_session.sh
  - tests/runahead_model.h
  - tests/runner/hashes.txt
  - tests/runner/nmi_fixture.c
  - tests/runner/phase20_fake_gh.c
  - tests/state_test.h
covered_digest: "v4:sha256:daf4d567d99e07838b64c805d63f404bf2a21e2a433c4370f3b3129d2fd8810c"
behavior_unverified: 0
overrides_applied: 0
re_verification:
  previous_status: passed
  previous_score: 32/32
  gaps_closed: []
  gaps_remaining: []
  regressions: []
unverified_prohibitions:
  - statement: "Implementation and test evidence must never incorporate GPL/LGPL emulator source or unmanifested ROM/BIOS bytes."
    verification: judgment
    disposition: unverified
    flagged: true
    flag: "unverified-prohibition — human review recommended"
    evidence: "Fresh automated hygiene and hosted provenance checks pass, and the verification scan found no contamination. The prohibition's full source/provenance guarantee remains judgment-tier; it is prominently flagged as one unverified prohibition."
---

# Phase 20: NMI line and VBlank flag Verification Report

**Phase Goal:** A game that reads $2002 or toggles NMI around the start or end of VBlank gets the console's flag value and exactly the console's NMIs, and its save states hold through that race.

**Verified:** 2026-10-28T19:13:07Z
**Status:** passed
**Re-verification:** Yes — current candidate `7848cc8980e8e96a1fc6f5ec223a9783fb7ef19d`, after the previous 32/32 report. This pass rechecked every truth against the current source and fresh hosted artifacts.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | All six named AccuracyCoin rows and owned race reproducers pass with source-selected whole-sequence values. | ✓ VERIFIED | Fresh CI `40748417351/1` and Nightly `40740261760/1` bind candidate `7848cc8…` to the accepted 418-file bundle. The clean campaign records 22/22 pages, 146 scored rows, 114 PASS/32 FAIL, zero skipped/not-run/timeout, and five DRAW; all six exact NMI target rows pass in the collected scoreboard. |
| 2 | All six supported platforms produce identical frame/audio hashes, with every moved pin explained. | ✓ VERIFIED | The six fresh candidate hash artifacts each contain 114 rows and have SHA-256 `1a06cbabea1fee522e023a405af4474726ad24ef798d067b5e27575cf505bc62`. Measurements document causes for moved values; the accepted bundle is source-bound to the current candidate. |
| 3 | VBlank-set, VBlank-clear, and edge-before-poll states replay through core, runner, and libretro with format 2 and format-1 refusal. | ✓ VERIFIED | The fresh 688-test local workflow passes. Named checks for NMI state replay, state format/refusal, core state-fork, runner roundtrip/idempotency/wrong-load, and libretro all pass in the workflow; `libretro.runahead` is recorded at 35.19 seconds. Format 2/revision 8 are pinned and format 1 is refused. |
| 4 | No pre-existing passing main row is lost on any CI lane. | ✓ VERIFIED | Each of the six fresh hosted lanes retains all 108/108 protected-main PASS keys. The collector's source-bound checks and CI required/hash-equality jobs pass for `40748417351/1`. |
| 5 | D-01: Owned PPU-dot, real-bus/program, and pinned AccuracyCoin evidence use separate oracles. | ✓ VERIFIED | PPU dot fixtures, production bus/CPU guest programs, and the pinned campaign are separate checks; all are present and passing. |
| 6 | D-02: Expected values identify source, sample coordinates, starting state, and event order rather than copying measured output. | ✓ VERIFIED | Test comments, the NMI oracle research, preparation sources, and measurement records give source references, cutpoints, initial state, and event order. The independent probe output is used only within its documented finite-margin scope. |
| 7 | D-03: Start/end boundary, suppression, retained-edge, mirror, and normal-control cases are distinguished. | ✓ VERIFIED | Registered `ppu.nmi_races` and `core.nmi_poll` cover before/on/after edges, suppression, retained capture, mirrored `$2002` addresses, and normal controls; both named tests passed directly and in the fresh workflow. |
| 8 | D-04: A $2002 read samples bit 7 at M2 rise and bits 6/5 at M2 fall while preserving read side effects. | ✓ VERIFIED | The test changes sprite overflow between dot 130 at +9 and dot 131 at +23, then reads through production bus decoding at `$2002` and mirror `$3FFA`. It asserts `0x35`, open-bus/I/O values, status/address-latch effects, 24 ticks, and one CPU cycle. Direct `ppu.nmi_races` passed. |
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
| 25 | D-22: Pins, schemas, caps, CI placement, six hash lanes, conformance checks, and the +1% performance guard remain enforced. | ✓ VERIFIED | Fresh CI/Nightly records and production validation bind conformance, boot-to-play, real RetroArch, six hash lanes, and performance to tested merge `716e8d4…`. All measured performance rows pass the unchanged +1% limit. |
| 26 | D-23: Changed hashes have old/new measurements and causes, with truthful campaign fingerprint and dirty-source metadata. | ✓ VERIFIED | The local metadata remains truthfully `source_dirty=1` at `6d2d1370d8fa`; fresh hosted metadata is `source_dirty=0` at tested revision `716e8d49c806`, with the same 56-input fingerprint `424db30d…` and report SHA `29ea119d…`. |
| 27 | D-24: Running state remains owned, integer-only, allocation-free, per-instance, and within the three deliverables. | ✓ VERIFIED | Runtime state remains per-instance with integer fields; two-instance checks pass. The changed source keeps the library, runner, and libretro adapter deliverables and adds no runtime dependency or device layer. |
| 28 | KEEP-03 idempotency: save/load/save at a valid boundary yields identical canonical bytes and continuation. | ✓ VERIFIED | Production save/load/save compares canonical bytes and resumed guest/video/tick/audio state. The targeted core.nmi_state test passes. |
| 29 | KEEP-03 concurrency: interleaved instances with different NMI history remain independent. | ✓ VERIFIED | State-fork and two-instance tests interleave independent histories and compare each continuation; core.state_fork.nmi passes. |
| 30 | KEEP-02 adjacency: equal-tick frame/audio/state keys remain distinct and duplicate/missing-key controls reject. | ✓ VERIFIED | `hash_inventory.cmake` constructs frame 60, state 60, PCM, and transition keys at tick 42886728 with five distinct digests. `hash.inventory.self_test` directly passes with all duplicate/missing controls and artifact-equality checks. |
| 31 | KEEP-02 empty: frame-zero/empty-PCM state preserves canonical bytes and nonzero audio history without inventing PCM. | ✓ VERIFIED | test_frame_zero_empty_pcm_preserves_history reaches 241:1 from a labeled constructed origin, saves/restores nonzero synth history with pcm_count=0 and sample_output=NULL, checks identical canonical bytes, verifies empty drains preserve a sentinel and state, then checks continuation against a history-reset negative control. The targeted core.nmi_state test passes. |
| 32 | KEEP-02 ordering: generated complete hash inventory is deterministic and rejects equal keys. | ✓ VERIFIED | runner.write_hashes, hash.inventory.self_test, and state.format_pin pass; generated inventory matches the checked-in inventory. |

**Score:** 32/32 truths verified (0 present, behavior-unverified).

### Re-verification Closure

The previous report already scored 32/32. This refresh rechecked the same 32 truths against the current source, source-bound hosted bundle, and live behavior:

1. The new collector run accepts 418 current effective files and is independently mode/blob-checked against candidate, CI, and Nightly trees.
2. The split-sample `$2002` test passes against the production decoder and asserts status values and side effects.
3. The equal-tick inventory fixture and duplicate/missing controls pass `hash.inventory.self_test`.
4. The frame-zero empty-PCM state witness remains present and passes `core.nmi_state`, proving preserved history, canonical state, and deterministic continuation.

No regressions or failed truths were found. The score remains 32/32.

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
| tests/ppu/test_nmi_races.c | src/bus.c | bus read/write for `$2002` and mirrors | WIRED | Direct production-bus assertions pass in `ppu.nmi_races`. |
| tests/core/test_nmi_poll.c | src/cpu.c | `cpu_step` to guest RAM markers | WIRED | Guest poll/handler markers pass in `core.nmi_poll`. |
| src/ppu.c + src/internal.h | src/bus.c | derived NMI line from status/control | WIRED | Inline `nesturbator__ppu_nmi_line` reads PPU fields; `bus.c` samples and latches rising edges. |
| src/bus.c | src/state.c | CPU `nmi_prev`/`nmi_pending` history | WIRED | `cpu_io` serializes and validates the fields used by the bus. |
| include/nesturbator.h | tests/runner/hashes.txt | format pin and measured state digest | WIRED | Header pins format 2/revision 8; `state.format_pin` and generated hash inventory pass. |
| src/bus.c | src/cpu.c | captured NMI at instruction poll point | WIRED | Bus populates poll history; `cpu_step` consumes the sampled request. |
| tests/core/test_nmi_state.c | src/state.c | public save/load and validate/apply | WIRED | `core.nmi_state` directly passes canonical save/restore assertions. |
| tests/core/test_nmi_state.c | src/cpu.c | executed steps and NMI guest markers | WIRED | `core.nmi_state` reaches and checks the instruction-driven cutpoints. |
| tests/nmi_fixture.h | tests/core/test_state_fork.c | shared NMI-sensitive guest program | WIRED | The state-fork test includes and executes the shared fixture; `core.state_fork.nmi` passes. |
| tests/cmake/runner_state.cmake | runner/main.c | CLI state/hash flags | WIRED | Registered runner roundtrip/idempotency/wrong-load CTests exercise the production flags and pass. |
| tests/cmake/nmi_targets.cmake | tests/accuracy/scoreboard.txt | exact six-key pass gate | WIRED | Fresh rows pass; `accuracycoin.nmi_targets` passes its exact-key controls. |
| tests/cmake/accuracycoin_activation.cmake | tests/accuracy/baseline-metadata.txt | report/revision/input-fingerprint checks | WIRED | Fresh hosted campaign metadata matches the accepted report and fingerprint; activation runs in CI. |
| .github/workflows/ci.yml | tests/runner/hashes.txt | six-platform hash/equality workflow | WIRED | CI artifacts include six identical 114-row files; hash-equality and required jobs pass. |
| tests/ppu/test_nmi_races.c | src/bus.c | bus API at `$2002` and `$3FFA` | WIRED | No callback path is used; direct named race test passes. |
| src/bus.c | src/ppu.c | catch-up and status-read composition | WIRED | Production cycle calls PPU catch-up/status helpers and preserves split samples. |
| tests/cmake/hash_inventory.cmake | tests/cmake/hash_keys.cmake | full-key validation | WIRED | Inventory includes the key table; `hash.inventory.self_test` passes duplicate/missing controls. |
| tests/core/test_nmi_state.c | src/state.c | canonical state replay | WIRED | NMI state save/load/save and resumed continuation pass. |
| .github/workflows/ci.yml | tests/cmake/hash_inventory.cmake | generated hash artifacts and validator | WIRED | The workflow runs the inventory/equality path and publishes per-platform outputs. |
| tests/cmake/phase20_hosted_evidence.cmake | tests/cmake/perf_compare.cmake | raw-measurement recomputation | WIRED | Fresh collection validates the performance artifact under the unchanged +1% policy. |
| tests/cmake/phase20_hosted_evidence.cmake | 20-EVIDENCE.md and re-verification note | accepted manifest/run/artifact evidence | WIRED | Fresh manifest/verdict values and source identity are recorded in the additive note; the historical ledger remains intact. |
| 20-09-SUMMARY.md | tests/cmake/phase20_hosted_evidence.cmake | source digest and causal source identity | WIRED | Summary identities are superseded by the new accepted run; new digest invalidates the prior bundle. |
| delivery.json | tests/cmake/phase20_hosted_evidence.cmake | repository, run/attempt, candidate, PR | WIRED | Fresh one-attempt delivery is consumed by production collection and validates as `hosted-pass`. |
| tests/cmake/phase20_hosted_evidence.cmake | 20-EVIDENCE.md and re-verification note | acceptance result and evidence ledger | WIRED | The produced manifest/verdict and checksums are retained and the report points to raw artifacts. |
| tests/runner/phase20_fake_gh.c | tests/cmake/phase20_hosted_evidence.cmake | isolated collect/validate controls | WIRED | The full local workflow passes hosted-evidence controls and adapter tests using the fake transport. |
| tests/cmake/phase20_hosted_evidence.cmake | tests/cmake/perf_compare.cmake | +1% and zero-spread gate | WIRED | Production collector recomputes fresh raw measurements and accepts all rows. |
| tests/cmake/phase20_hosted_evidence.cmake | 20-12-PLAN.md | source digest invalidates older acceptance | WIRED | Fresh manifest has 418 files and a distinct current digest; the old 417-file bundle is not used. |
| 20-11-SUMMARY.md | 20-DELIVERY.md | delivery through existing branch/PR | WIRED | Delivery artifact documents the candidate and open PR #59; live PR head/base match. |
| delivery.json | 20-10-PLAN.md | new source-bound run/attempt metadata | WIRED | The accepted delivery fields match the plan's required run identities and current manifest. |
| tests/cmake/phase20_hosted_evidence.cmake | availability artifact | metadata-only `MODE=availability` | WIRED | Availability controls pass in the fresh 688-test suite; availability mode remains separate from content acceptance. |

The query checked 31 declared links and matched 6 by literal source pattern. Its other 25 “not referenced” results are static-matcher false negatives across C/CMake call edges and document-to-evidence links. Each was traced through source, test registration, current metadata, or fresh collector output as recorded above; none is orphaned.

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
| Fresh whole-project CI workflow | `cmake --workflow --preset ci` | Exit 0; 688/688 tests in 434.48 s. Library, runner, and libretro packages generated. Log: `build/phase20-refresh-regression.log`. | ✓ PASS |
| Named race, poll, state, runner, and inventory checks | Named `ctest --test-dir build/ci --output-on-failure -R '^TEST$'` invocations | `ppu.nmi_races`, `core.nmi_poll`, `core.nmi_state`, `core.state_fork.nmi`, `runner.state.nmi_roundtrip`, `runner.state.nmi_idempotent`, `runner.state.nmi_wrong_load`, `core.state_reject`, `state.format_pin`, `hash.inventory.self_test`, and `accuracycoin.nmi_targets` pass. | ✓ PASS |
| libretro run-ahead and serialization | Fresh full-workflow log, `libretro.runahead` | Passes in 35.19 s. Its separate 10-second bounded spot-check invocation was stopped at the limit. | ✓ PASS |
| Production collection | `cmake -DMODE=collect -DROOT=/tmp/nesturbator-phase20-refresh-single-attempt-evidence -DSOURCE_REF=/tmp/nesturbator-phase20-refresh-source -P tests/cmake/phase20_hosted_evidence.cmake .` | Exit 0; fresh one-attempt CI/Nightly bundle accepted as `hosted-pass`. | ✓ PASS |
| Current checkout against fresh hosted bundle | `cmake -DMODE=validate -DROOT=build/phase20-refresh-hosted-evidence -DSOURCE_REF="$PWD" -P tests/cmake/phase20_hosted_evidence.cmake .` | Exit 0; current working checkout revalidated against the accepted bundle. | ✓ PASS |
| NMI circuit finite-margin probe | `gsd_run run-with-timeout 30 -- python3 .planning/phases/20-nmi-line-and-vblank-flag/probes/20-nmi-netlist-oracle-fetch.py` | Exit 0; six independent fresh-state pulse cases passed; no exact clock ties were claimed. | ✓ PASS |
| Fresh hosted target, hash, and conformance evidence | CI `40748417351/1`, Nightly `40740261760/1`; accepted bundle under `build/phase20-refresh-hosted-evidence/` | Six NMI rows pass; every lane retains 108 protected PASS keys and 114 identical hash rows; performance, vectors, MMC3, and real RetroArch checks pass. | ✓ PASS |

### Probe Execution

| Probe | Command | Result | Status |
|---|---|---|---|
| Phase 20 netlist oracle probe | `gsd_run run-with-timeout 30 -- python3 .planning/phases/20-nmi-line-and-vblank-flag/probes/20-nmi-netlist-oracle-fetch.py` | Exit 0; six fresh-state finite-margin pulses passed; exact ties are not claimed. | PASS |
| Conventional scripts/*/tests/probe-*.sh probes | Discovery | None found or declared by the phase. | N/A |

### Requirements Coverage

| Requirement | Source Plans | Status | Evidence |
|---|---|---|---|
| NMI-01 | 20-01, 20-03, 20-06 | SATISFIED | Fresh source-bound `$2002` flag/timing/suppression scoreboard rows pass, as do the production-bus race tests. |
| NMI-02 | 20-01, 20-03, 20-06 | SATISFIED | Fresh NMI control, VBlank-end, and disabled rows pass; STA/INC guest markers and named poll tests verify interrupt sequences. |
| KEEP-02 | 20-04, 20-06, 20-08, 20-10 | SATISFIED | Current hosted campaign and six-platform identity, equal-tick adjacency controls, and frame-zero empty-PCM history witness all pass. |
| KEEP-03 | 20-04, 20-05, 20-06 | SATISFIED | Fresh core/runner/libretro replay, format-2 pin, format-1 refusal, and canonical continuation checks pass. |

REQUIREMENTS.md maps exactly these four IDs to Phase 20; there are no orphaned Phase 20 requirements. Its checkboxes and mapping statuses were left untouched as instructed.

### Anti-Patterns and Deferred Review Debt

| Finding | Severity | Current assessment |
|---|---|---|
| CR-01: hosted collector does not enforce Git file modes in its recurring source identity comparison | WARNING, explicitly deferred | The independent 418-entry `(path, mode, blob SHA)` set matches the current index subset and candidate, CI-tested, and Nightly-tested trees. TSV: `build/phase20-refresh-source-modes.tsv`, SHA-256 `98f1978ae7a3ead9f8ce22b62a4f6d5950165abeddb03523e57bfe696e4fce7f`. This closes the current-run evidence question; recurring collector enforcement remains maintenance debt. |
| WR-03: collector recursively scans source root and can encounter ignored build artifacts | WARNING, explicitly deferred | Successful collection used absolute `SOURCE_REF` to a clean equivalent checkout. Current checkout `MODE=validate` passed against the fresh immutable bundle. The source-root scan weakness remains a future robustness concern. |
| WR-04: nightly validation function has a duplicate identical definition | WARNING, explicitly deferred | The later definition replaces the earlier CMake function with the same body; the tested Nightly workflow and bundle validation pass. Remove the duplication during future collector maintenance. |

No unresolved debt-marker comments or placeholder implementations were found in changed functional code. Grep hits were planning backlog references, probe logging, and fixture/error handling; none is a functional stub or unreferenced debt marker. Fresh CI includes hygiene checks. Decision coverage is 24/24: all trackable CONTEXT.md decisions are honored by shipped artifacts.

### Accepted Evidence Limits and Human Verification

No manual behavior test is needed for this headless core phase. The judgment-tier clean-room prohibition remains explicitly flagged as `unverified-prohibition — human review recommended`; fresh automated hygiene/provenance checks do not claim complete source provenance. This flagged limitation is not a phase behavior gate under the project workflow. D-10 accepts a deterministic global tie policy because the exact paired-chip physical tie is not directly measured; the report makes no physical winner claim. Neither limitation falsifies an observable phase truth.

### Gaps Summary

All 32 observable truths are verified against the current candidate. The fresh production collector and independent mode/blob comparison close the current-source evidence requirement; targeted state/race/inventory tests and the 688-test workflow pass. CR-01, WR-03, and WR-04 remain non-blocking, explicitly deferred collector-maintenance debt. The clean-room prohibition and exact paired-chip tie remain disclosed evidence limits.

---

_Verified: 2026-10-28T19:13:07Z_
_Verifier: gsd-verifier agent_
