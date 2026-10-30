---
phase: 20-nmi-line-and-vblank-flag
verified: 2026-10-30T12:34:26Z
status: passed
score: 32/32 truths verified
covered_files:
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
  - .planning/phases/20-nmi-line-and-vblank-flag/20-CONTEXT.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-DELIVERY.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-EVIDENCE.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-RESEARCH.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-REVERIFICATION-EVIDENCE.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-REVIEW-DISPOSITION.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-REVIEW.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-EVIDENCE.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-REVIEW-DISPOSITION.md
  - README.md
  - build/ci-efficiency-quick/campaign-final/metadata.txt
  - build/ci-efficiency-quick/campaign-final/report.tsv
  - build/ci-efficiency-quick/campaign-final/summary.txt
  - build/phase20-refresh-hosted-evidence/manifest.txt
  - build/phase20-refresh-hosted-evidence/verdict.txt
  - build/phase20-refresh-source-modes.tsv
  - build/phase20-verification-refresh-ci.log
  - build/phase21-hosted-evidence/effective-inputs.txt
  - build/phase21-hosted-evidence/manifest.txt
  - build/phase21-hosted-evidence/verification/current-source-identity.json
  - build/phase21-hosted-evidence/verification/current-worktree-modes-blobs.tsv
  - include/nesturbator.h
  - src/bus.c
  - src/cpu.c
  - src/internal.h
  - src/ppu.c
  - src/state.c
  - tests/CMakeLists.txt
  - tests/accuracy/baseline-metadata.txt
  - tests/accuracy/measurements.md
  - tests/accuracy/migration.tsv
  - tests/accuracy/scoreboard.txt
  - tests/cmake/accuracycoin_activation.cmake
  - tests/cmake/accuracycoin_baseline_fetch.cmake
  - tests/cmake/accuracycoin_nmi_targets_control.cmake
  - tests/cmake/accuracycoin_regression_controls.cmake
  - tests/cmake/hash_inventory.cmake
  - tests/cmake/hash_keys.cmake
  - tests/cmake/nmi_targets.cmake
  - tests/cmake/perf_compare.cmake
  - tests/cmake/phase20_hosted_evidence.cmake
  - tests/cmake/phase20_hosted_evidence_controls.cmake
  - tests/cmake/phase21_hosted_evidence.cmake
  - tests/cmake/phase21_source_identity.cmake
  - tests/cmake/phase21_source_identity_controls.cmake
  - tests/cmake/state_format_pin.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/core/test_interrupt_entry.c
  - tests/core/test_interrupt_state.c
  - tests/core/test_irq_poll.c
  - tests/core/test_nmi_poll.c
  - tests/core/test_nmi_state.c
  - tests/core/test_state.c
  - tests/core/test_state_fork.c
  - tests/core/test_state_reject.c
  - tests/core/test_state_runahead.c
  - tests/libretro/libretro_host.c
  - tests/nmi_fixture.h
  - tests/ppu/test_nmi_races.c
  - tests/runahead_model.h
  - tests/runner/hashes.txt
  - tests/runner/nmi_fixture.c
  - tests/state_test.h
covered_digest: "v4:sha256:00216ab6f118b920a09ca3493acd9c4ceb55becccf2433e026aac22cd45c15c8"
behavior_unverified: 0
overrides_applied: 0
re_verification:
  previous_status: passed
  previous_score: 32/32
  gaps_closed: []
  gaps_remaining: []
  regressions: []
advisory:
  - finding: "README.md:29 says the behavior revision is 8; README.md:1341 and the public header identify format 3 and behavior revision 9."
    category: other
    reason: "Nonblocking documentation inconsistency found during this current-source refresh; runtime pins and tests agree on revision 9."
    evidence_status: "Directly inspected README/header; documentation mismatch is deterministic, with no behavior test failure."
unverified_prohibitions:
  - statement: "Implementation and test evidence must never incorporate GPL/LGPL emulator source or unmanifested ROM/BIOS bytes."
    verification: judgment
    disposition: unverified
    flagged: true
    flag: "unverified-prohibition — human review recommended"
    evidence: "Current automated hygiene and provenance evidence show no contamination. Full source/provenance assurance remains judgment-tier and is not asserted by this report."
---

# Phase 20: NMI line and VBlank flag Verification Report

**Phase Goal:** A game that reads `$2002` or toggles NMI around the start or end of VBlank gets the console's flag value and exactly the console's NMIs, and its save states hold through that race.

**Verified:** 2026-10-30T12:34:26Z
**Status:** passed
**Re-verification:** Yes — after Phase 21, against current format 3 / behavior revision 9 source. The earlier format 2 / revision 8 report is superseded for the current checkout.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | All six AccuracyCoin NMI rows and owned race reproducers pass with source-selected values. | ✓ VERIFIED | Current CI log records `accuracycoin.nmi_targets`, `ppu.nmi_races`, and `core.nmi_poll` passing. The current campaign is terminal at 146 scored rows: 119 PASS, 27 FAIL, no skipped/not-run/timeout rows, and five DRAW; all six NMI target rows pass. |
| 2 | The six supported platforms produce identical frame/audio hashes, with moved pins explained. | ✓ VERIFIED | The Phase 21 hosted evidence has six-platform identical hashes and the 114 protected keys for its own candidate `54c7db2` / merge `faf1b91`. That evidence is source-bound and is not represented as a hosted acceptance for current HEAD. Current runtime and hash-pin inputs match that candidate; the current source's fresh CI and canonical inventory checks pass. |
| 3 | VBlank-set, VBlank-clear, and edge-before-poll states replay through core, runner, and libretro; current format 3/revision 9 loads, and formats 1/2 are refused. | ✓ VERIFIED | Current header and `tests/runner/hashes.txt` pin format 3/revision 9. `src/state.c` rejects non-current formats before mutation; `core.state_reject`, `state.format_pin`, `core.nmi_state`, `core.state_fork.nmi`, runner NMI roundtrip/wrong-load/idempotency, and `libretro.runahead` pass in fresh CI. |
| 4 | The 108 PASS / 38 FAIL starting scoreboard is protected, including Phase 19 rows; all six target additions and any incidental changes are accounted for. | ✓ VERIFIED | Current `accuracy.scoreboard.no_lost_pass` passes with 114 protected keys. The current campaign reports 119 PASS / 27 FAIL / five DRAW over 146 scored rows. This covers D-21's baseline and protected-main decision; no lost PASS is reported. |
| 5 | D-01: PPU-dot, real-bus/program, and pinned AccuracyCoin checks use separate oracles. | ✓ VERIFIED | Owned PPU race fixtures, guest programs using production bus/CPU paths, and pinned campaign vectors are separate checks and pass in current CI. |
| 6 | D-02: Expected values identify source, cutpoint, starting state, and event order rather than copying measured output. | ✓ VERIFIED | Research, fixture comments, measurement records, and state witnesses identify sources and event order. The independent netlist probe is limited to finite-margin cases and is not used to choose a physical exact-tie winner. |
| 7 | D-03: Start/end boundary, suppression, retained-edge, mirror, and normal-control cases are distinct. | ✓ VERIFIED | `ppu.nmi_races` and `core.nmi_poll` pass against before/on/after boundaries, suppression, edge retention, mirrored `$2002`, and normal-control cases. |
| 8 | D-04: `$2002` samples bit 7 at M2 rise and bits 6/5 at M2 fall while applying read side effects once. | ✓ VERIFIED | `test_sprite_status_changes_between_m2_samples` changes sprite overflow between the production bus samples and checks `$2002`/`$3FFA`, expected `0x35`, open bus, status/address latch, and one-cycle timing. Current `ppu.nmi_races` passes. |
| 9 | D-05: NMI enable transitions, unchanged `$80`, cleared flag, both boundaries, and STA/INC markers produce expected NMIs. | ✓ VERIFIED | CPU guest programs execute STA and both INC writes and assert handler/following-instruction markers. Current `core.nmi_poll` and all six campaign NMI targets pass. |
| 10 | D-06: Green guards run normally; expected-red reproducers cannot pass on setup failure, crash, or an unrelated assertion. | ✓ VERIFIED | Regression controls require the named assertion and normal termination; current CI includes the AccuracyCoin campaign, activation, protected-observation, and regression-control targets. |
| 11 | D-07: Wrong-model controls identify distinct race and poll errors. | ✓ VERIFIED | Race/poll tests include edge timing, capture revocation, enable, RMW-write, early-service, and table-shift controls; the current named race/poll tests pass. |
| 12 | D-08: Derived NMI line, previous-line sample, and captured request are separate; line deassertion does not revoke captured work. | ✓ VERIFIED | PPU line derivation, bus edge capture, and CPU poll history use distinct state. Guest-marker and save-state tests cover deassertion after capture; `core.nmi_state` and `core.nmi_poll` pass. |
| 13 | D-09: +9/+15 half-master-tick reads, register effects, APU phase, mapper stamps, and DMA accesses remain coherent. | ✓ VERIFIED | Production bus catch-up order and one-time register side effects are retained; current race/state workflow and bus-related CI regressions pass. |
| 14 | D-10: Finite-margin evidence supports one deterministic global tie policy without claiming a physical exact-tie winner. | ✓ VERIFIED | The netlist probe passes six independent finite-margin pulse cases. Source order defines the deterministic policy; paired-chip exact ties remain unmeasured and are not claimed as hardware fact. |
| 15 | D-11: Default alignment is tested against complete source tables before any global phase adjustment. | ✓ VERIFIED | Measurements retain complete source tables, default alignment, and absolute-dot witnesses; no per-test offset or VBlank-dot shift is used. |
| 16 | D-12: A source table accepts only complete alternatives and explicitly ignored cells; phase/history is instance-local. | ✓ VERIFIED | Full tables and mixed-alternative rejection controls remain in tests; phase and history live in each emulator instance. Current state and campaign checks pass. |
| 17 | D-13: PPU-to-CPU line boundary and IRQ writer ownership stay at existing seams. | ✓ VERIFIED | PPU derives the NMI line, bus samples it, and CPU consumes poll history; IRQ ownership remains at its existing writer seam. Relevant regressions pass. |
| 18 | D-14: State format/revision transition is current format 3/revision 9, with error precedence and atomic refusal; formats 1 and 2 are rejected. | ✓ VERIFIED | Public header and runner pin agree on 3/9; loader rejects other versions before mutation. `core.state_reject` and `state.format_pin` pass. This supersedes the prior format 2/revision 8 result. |
| 19 | D-15: Persistent NMI capture/poll history is serialized, validated, and reset consistently. | ✓ VERIFIED | `state.c` serializes and validates NMI history and interrupt fields; reset clears causal state. Current NMI replay and rejection tests pass. |
| 20 | D-16: CRC-correct malformed bounds and unreachable states reject before mutation; real histories remain canonical and loadable. | ✓ VERIFIED | State rejection checks preserve the instance on failure; canonical save/load/save and continuation witnesses pass in `core.nmi_state` and `core.state_reject`. |
| 21 | D-17: Exact internal cutpoints use production save/load at resumable boundaries, distinguishing constructed origins from executed endpoints. | ✓ VERIFIED | State tests label constructed origins separately from instruction-reached endpoints. The bounded cutpoint probe also reaches complete JMP endpoints and replays exact set/clear states through the current library. |
| 22 | D-18: Exact set/clear witnesses include coherent nonzero audio history, with edge-before-poll feasibility bounded. | ✓ VERIFIED | Current `core.nmi_state` checks retained audio history and edge-before-poll replay. The separate cutpoint probe is explicitly limited: NMI is disabled and its `(261,0)` origin is constructed; the enabled behavior is covered by the named current state/poll tests. |
| 23 | D-19: Restored continuation matches native state, frame/tick/audio, and guest markers, including omitted-history and independent-instance controls. | ✓ VERIFIED | Current core, runner, and libretro tests exercise continuation, omitted-history rejection, and isolated histories; named state-fork, runner, and `libretro.runahead` checks pass. |
| 24 | D-20: Poll behavior, tests, state, docs, hashes, and campaign evidence move together as a causal change. | ✓ VERIFIED | Current implementation, public contract, race/state tests, hash pins, and campaign are covered by the current source and passing CI. |
| 25 | D-22: Pins, schemas, caps, CI placement, six hash lanes, conformance checks, and the +1% performance guard remain enforced. | ✓ VERIFIED | CI/Nightly policies and evidence validators remain present. Phase 21's source-bound six-lane evidence covers conformance and performance for its candidate; current local CI validates current source, and no threshold/pin relaxation was found. |
| 26 | D-23: Changed hashes have old/new values and causes; campaign fingerprint and dirty-source metadata remain truthful. | ✓ VERIFIED | Current campaign metadata identifies source `d4802809ff41`, `source_dirty=1`, revision 9, and the current 146-row outcome. Historical clean hosted measurements remain tied to their recorded source; the local campaign is not presented as clean hosted evidence. |
| 27 | D-24: Runtime remains integer-only, per-instance, allocation-free while running, within the three deliverables, and clean-room constrained. | ✓ VERIFIED | Runtime state is per-instance integer state; tests interleave instances. Library, runner, and libretro remain the deliverables, with no device layer or runtime dependency introduced. Clean-room provenance remains judgment-tier flagged below. |
| 28 | KEEP-03 idempotency: save/load/save at a valid boundary preserves canonical bytes and continuation. | ✓ VERIFIED | `core.nmi_state` and runner idempotency compare canonical bytes and resumed state/output; both pass in current CI. |
| 29 | KEEP-03 concurrency: interleaved instances with different NMI histories remain independent. | ✓ VERIFIED | `core.state_fork.nmi` and state tests interleave and compare independent histories; the named check passes. |
| 30 | KEEP-02 adjacency: equal-tick frame/audio/state keys stay distinct and duplicate/missing-key controls reject. | ✓ VERIFIED | `hash.inventory.self_test` passes for frame, state, PCM, and transition keys at a shared tick with duplicate/missing controls. |
| 31 | KEEP-02 empty: frame-zero/empty-PCM state preserves canonical bytes and nonzero audio history without inventing PCM. | ✓ VERIFIED | `test_frame_zero_empty_pcm_preserves_history` retains `pcm_count=0`, a null sample output, nonzero synth history, identical bytes, sentinel-safe empty drains, and matching continuation; `core.nmi_state` passes. |
| 32 | KEEP-02 ordering: complete hash inventory generation is deterministic and rejects equal keys. | ✓ VERIFIED | `runner.write_hashes`, `runner.write_hashes.content`, `hash.inventory.self_test`, and `state.format_pin` pass in the current CI log. |

**Score:** 32/32 truths verified (0 present, behavior-unverified).

### Re-verification scope and source identities

The previous report was 32/32 for format 2/revision 8. The current checkout is format 3/revision 9 and rejects formats 1 and 2; prior format-2 acceptance is not asserted as current compatibility.

- The historical Phase 20 hosted bundle accepted candidate `7848cc8` / merge `716e8d4`, with 418 effective inputs and 114 PASS / 32 FAIL. It remains valid for that exact candidate only; this refresh does not claim it qualifies current HEAD.
- The Phase 21 hosted bundle accepted candidate `54c7db2` / merge `faf1b91`, with 428 effective inputs. Its cross-platform hash and protected-key evidence remains bound to that source. Covered runtime, runner, libretro, and hash-pin inputs match the current candidate; the public-header change is comment-only and the current format/revision pins remain 3/9.
- The fresh local campaign reports source `d4802809ff41`, `source_dirty=1`, revision 9, 22/22 pages, 146 terminal scored rows, 119 PASS / 27 FAIL / 0 skipped, not-run, or timed-out, and five DRAW. All six NMI target rows pass. Its report SHA-256 is `56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`.
- No new hosted delivery was authorized or required. Current-source runtime behavior is verified by the fresh full CI and current data above; historical hosted evidence is kept source-bound.

### Decision coverage

The current decision-coverage query reports 24/24 honored decisions from `20-CONTEXT.md`. D-01 through D-20, D-22 through D-24 are represented in truths 5–27; D-21's starting scoreboard/protected-main policy is represented in truth 4. The five KEEP truths are 28–32.

### Required Artifacts

All 12 plan artifact queries passed: 41/41 declared artifacts exist and are substantive. Source, test, and CMake registrations were checked. The static key-link matcher only literally matched 6 of 31 declared links; its C/CMake and document-to-evidence false negatives were traced manually through implementation, test registration, current CI, and the retained source-bound evidence.

| Area | Expected | Status | Details |
|---|---|---|---|
| PPU, bus, and CPU NMI behavior | Split `$2002` sampling, NMI-line capture, CPU poll state | ✓ VERIFIED | `src/ppu.c`, `src/bus.c`, `src/cpu.c`; exercised by race/poll tests. |
| State persistence | Current serializer, validation, atomic refusal, replay | ✓ VERIFIED | `src/state.c`; format3/revision9 pin and current state tests pass. |
| Core/runner/libretro tests | Real race, save/load, fork, and host tests | ✓ VERIFIED | 41/41 plan artifacts and current CI targets pass. |
| Campaign and inventory | Six exact NMI rows, protected keys, deterministic inventory | ✓ VERIFIED | Current campaign and hash inventory tests pass; hosted cross-platform evidence remains source-bound. |

### Key Link Verification

| From | To | Via | Status | Evidence |
|---|---|---|---|---|
| `src/ppu.c` / `src/internal.h` | `src/bus.c` | Derived NMI line sampled and edge captured | WIRED | Production line helper and bus capture path; race/poll checks pass. |
| `src/bus.c` | `src/cpu.c` | Captured request consumed at instruction poll | WIRED | Bus poll history is consumed by `cpu_step`; guest-marker tests pass. |
| `src/bus.c` | `src/state.c` | NMI/interrupt history serialized and restored | WIRED | Current canonical save/load and edge-before-poll replay pass. |
| `include/nesturbator.h` | `src/state.c` / `tests/runner/hashes.txt` | Public format/revision contract | WIRED | All current pins say format 3/revision 9; format1/2 refusal tests pass. |
| PPU/core fixtures | `tests/CMakeLists.txt` | Registered executable checks | WIRED | `ppu.nmi_races`, `core.nmi_poll`, `core.nmi_state`, and state-fork tests ran in current CI. |
| runner/libretro fixtures | production host interfaces | Public save/load and resumed execution | WIRED | Runner NMI roundtrip/idempotency/wrong-load and `libretro.runahead` pass. |

### Data-Flow Trace

| Artifact | Real source | Status |
|---|---|---|
| `$2002` status and NMI poll | PPU status/control → bus samples/capture → CPU poll → guest markers | ✓ FLOWING |
| Save-state NMI history | Per-instance fields → format3 serializer/validator → production restore → continuation | ✓ FLOWING |
| AccuracyCoin NMI rows | Pinned guest execution → produced markers/report → six-key target gate | ✓ FLOWING |
| Frame/audio/state inventory | Runner outputs → key validation → compatibility inventory | ✓ FLOWING |
| Hosted evidence | Raw jobs/artifacts → provenance and content checks → source-bound manifest | ✓ FLOWING |

No checked output is a static response or hard-coded pass value.

### Behavioral Spot-Checks

| Behavior | Command or evidence | Result | Status |
|---|---|---|---|
| Full current-source workflow | Parent's fresh `cmake --workflow --preset ci`; `build/phase20-verification-refresh-ci.log` | Exit 0; 695/695 passed, 100%, 401.09 seconds. All three v0.1.11 macOS arm64 deliverables generated. No source edits followed this run. | ✓ PASS |
| NMI status, poll, and state history | Named CTests in that log: `ppu.nmi_races`, `core.nmi_poll`, `core.nmi_state`, `core.state_fork.nmi` | All pass. | ✓ PASS |
| Format and rejection atomicity | `core.state_reject`, `state.format_pin` | Formats 1/2 and invalid versions reject; instance remains unchanged. Both pass. | ✓ PASS |
| Core/runner/libretro continuation | Runner NMI roundtrip, wrong-load, idempotency; `libretro.runahead` | All pass in the current workflow. | ✓ PASS |
| Current campaign and protected keys | `accuracycoin.nmi_targets`, `accuracy.scoreboard.no_lost_pass`; campaign `metadata.txt` and `report.tsv` | Six NMI target rows pass; 114 protected keys pass; current campaign has 119/27/5 terminal results. | ✓ PASS |
| Current cutpoint fixture | `cc -std=c17 -Wall -Wextra -Wpedantic -Iinclude -Isrc -Itests .planning/phases/20-nmi-line-and-vblank-flag/probes/20-cutpoint-probe.c build/ci/libnesturbator.a -o /tmp/phase20-cutpoint-probe-verify && /tmp/phase20-cutpoint-probe-verify` | Exit 0. Exact set/clear states replay through the current library with complete JMP endpoints and nonzero audio history. The probe's NMI-disabled, constructed-origin limits are stated in truth 22. | ✓ PASS |
| Independent NMI finite-margin oracle | `gsd_run run-with-timeout 30 -- python3 .planning/phases/20-nmi-line-and-vblank-flag/probes/20-nmi-netlist-oracle-fetch.py` | Exit 0; six independent finite-margin cases pass. Exact paired-chip ties are not measured. | ✓ PASS |

### Probe Execution

| Probe | Command | Result | Status |
|---|---|---|---|
| Phase 20 netlist oracle | `gsd_run run-with-timeout 30 -- python3 .planning/phases/20-nmi-line-and-vblank-flag/probes/20-nmi-netlist-oracle-fetch.py` | Exit 0; six finite-margin cases passed, no exact-tie claim. | PASS |
| Phase 20 cutpoint probe | Compile and run command in Behavioral Spot-Checks | Exit 0; bounded exact set/clear save/load/continuation behavior passed. | PASS |
| Conventional `scripts/*/tests/probe-*.sh` | Phase directory discovery | None found or declared. | N/A |

### Requirements Coverage

| Requirement | Source plans | Description | Status | Evidence |
|---|---|---|---|---|
| NMI-01 | 20-01, 20-03, 20-06 | `$2002` VBlank flag timing and suppression | SATISFIED | Current race test and `$2002` flag campaign row pass. |
| NMI-02 | 20-01, 20-03, 20-06 | Correct NMI behavior around VBlank boundaries | SATISFIED | Poll/race tests and five other current NMI campaign target rows pass. |
| KEEP-02 | 20-04, 20-06, 20-08, 20-10 | Preserve deterministic frame/audio/state evidence | SATISFIED | Current inventory and empty-PCM history witness pass; protected-key gate passes. |
| KEEP-03 | 20-04, 20-05, 20-06 | Preserve state behavior and replay through public surfaces | SATISFIED | Current core, runner, and libretro replay tests pass with format3/revision9. |

`REQUIREMENTS.md` maps exactly these four IDs to Phase 20; no orphaned Phase 20 requirement was found. The file was not edited.

### Anti-Patterns and Deferred Review Debt

No unresolved `TBD`, `FIXME`, or `XXX` markers, placeholder implementations, or disconnected data paths were found in the inspected implementation and tests. Empty-return scan results are allocation/error handling paths, not stubs.

| Finding | Disposition | Current evidence and limit |
|---|---|---|
| CR-01: hosted source validation omits recurring Git file-mode enforcement | Deferred | Independent run-specific `(path, mode, blob)` comparison covers all 418 inputs and matches candidate/CI/Nightly trees. This does not repair recurring collector enforcement. |
| WR-03: recursive collector scan can encounter ignored build fixtures | Deferred | The accepted-run workaround uses a clean equivalent source checkout; recurring scan behavior remains maintenance debt. |
| WR-04: Nightly validator is defined twice | Deferred | Current source contains identical duplicate function bodies; tested behavior passes, but cleanup remains deferred. |
| WR-05: Phase 21 identity helper misses filesystem mode changes (same issue as Phase 21 WR-01) | Deferred | Independent run-specific mode/blob comparison exists for the accepted 428-input candidate; recurring enforcement remains debt. |
| WR-01: public NMI API comment misstated captured-request behavior | Fixed | Current public comment and README contract describe the retained request after line deassertion. |
| WR-02: split `$2002` test lacked changing sprite-flag coverage | Fixed | Production-bus test changes sprite overflow between M2 samples and checks `$2002` and `$3FFA`. |

These are disclosed maintenance and evidence-enforcement limits, not newly observed NMI runtime failures. The current refresh adds no manual sign-off requirement.

### Accepted Evidence Limits and Human Verification

No human behavior check is required for this headless core phase. The clean-room prohibition is judgment-tier and remains explicitly flagged as `unverified-prohibition — human review recommended`; automated hygiene/provenance evidence does not claim complete source provenance. The deterministic finite-margin policy remains documented; exact physical paired-chip tie ordering is unmeasured and not claimed.

### Advisory (New Scope, Unevidenced)

| # | Finding | Category | Why Advisory |
|---|---|---|---|
| 1 | `README.md:29` says behavior revision 8 while `README.md:1341` and `include/nesturbator.h` identify format 3/revision 9. | other | Direct, deterministic documentation mismatch; no current runtime test fails. This refresh records it without changing source or expanding maintenance scope. |

### Gaps Summary

All 32 original observable truths are verified for current revision 9 behavior; all 24 context decisions and four mapped requirements are covered. Current CI provides fresh behavior evidence. Historical Phase 20 and Phase 21 hosted bundles remain bound to their own source identities, while this report makes no new hosted acceptance claim for current HEAD. The four deferred review findings, clean-room judgment flag, physical exact-tie limit, and README revision advisory remain visible and do not contradict the verified runtime truths.

---

_Verified: 2026-10-30T12:34:26Z_
_Verifier: gsd-verifier agent_
