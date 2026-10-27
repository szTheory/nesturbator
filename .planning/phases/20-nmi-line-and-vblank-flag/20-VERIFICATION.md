---
phase: 20-nmi-line-and-vblank-flag
verified: 2026-10-27T02:15:32Z
status: gaps_found
score: 26/32 must-haves verified
covered_files:
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
  - .planning/phases/20-nmi-line-and-vblank-flag/20-EVIDENCE.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-NMI-ORACLE-RESEARCH.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-REVIEW-DISPOSITION.md
  - .planning/phases/20-nmi-line-and-vblank-flag/20-REVIEW.md
  - README.md
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
  - tests/cmake/accuracycoin_driver.cmake
  - tests/cmake/accuracycoin_nmi_targets_control.cmake
  - tests/cmake/accuracycoin_regression_controls.cmake
  - tests/cmake/ci_required_rollup.cmake
  - tests/cmake/nightly_workflow_policy.cmake
  - tests/cmake/nmi_targets.cmake
  - tests/cmake/runner_state.cmake
  - tests/cmake/session_inventory.cmake
  - tests/cmake/test_push_format.py
  - tests/cmake/vector_api_policy.cmake
  - tests/cmake/vector_registration_policy.cmake
  - tests/core/test_api.c
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
  - tests/ppu/test_nmi_races.c
  - tests/ppu/test_registers.c
  - tests/ppu/test_sprites.c
  - tests/retroarch/CMakeLists.txt
  - tests/retroarch/frame_index.c
  - tests/retroarch/test_state_session.sh
  - tests/runahead_model.h
  - tests/runner/hashes.txt
  - tests/runner/nmi_fixture.c
  - tests/state_test.h
covered_digest: "v3:sha256:8ab3afb1c66a171efcc035a34e713e6497647f73413be58348ecec5296f72c5e"
behavior_unverified: 1
overrides_applied: 0
gaps:
  - truth: "Roadmap SC-02/SC-04 and D-22: exact-HEAD hosted evidence establishes six-platform hash equality, protected PASS retention, performance, and configured hosted checks."
    status: partial
    reason: "Local code and gates pass, but no exact-HEAD hosted run or artifact identity exists; Plan 20-06 records these outcomes as outstanding."
    artifacts:
      - path: ".github/workflows/ci.yml"
        issue: "Six-platform and protected-main workflows are wired, but their exact-HEAD results are not available in this phase evidence."
      - path: "tests/accuracy/baseline-metadata.txt"
        issue: "The measured campaign records source_dirty=1; there is no clean-source exact-HEAD hosted campaign artifact."
    missing:
      - "Exact-HEAD six-platform frame/audio/state hash artifacts and protected PASS results."
      - "Exact-HEAD hosted performance and configured vector/MMC3/Holy Mapperel/boot-to-play/real-host results."
      - "Identified clean-source campaign evidence."
  - truth: "D-04: a production-decoder $2002 regression test distinguishes the bit-7 M2-rise sample from sprite bits 6/5 at M2 fall."
    status: partial
    reason: "The production implementation has separate samples, but current bus-read tests leave sprite status constant across both samples."
    artifacts:
      - path: "tests/ppu/test_nmi_races.c"
        issue: "No case changes sprite bits between M2 samples, so reversed sprite-bit sampling can pass."
    missing:
      - "An automated production-decoder test that changes bit 5 or bit 6 between the early and late samples and asserts the composed status and single-read side effects."
  - truth: "KEEP-02 adjacency: equal-tick frame/audio/state keys remain distinct and duplicate/missing-key controls reject."
    status: partial
    reason: "The plan identifies this as a backstop, and the evidence ledger leaves the independent property unconfirmed."
    artifacts:
      - path: "tests/cmake/session_inventory.cmake"
        issue: "Current inventory checks do not provide the required equal-tick lane-adjacency witness."
    missing:
      - "A concrete automated equal-tick adjacency case with duplicate and missing-key negative controls."
  - truth: "KEEP-02 empty: frame-zero/empty-PCM state retains canonical state and nonzero saved audio history without fabricating PCM."
    status: partial
    reason: "The plan identifies this as a backstop, and the combined witness is explicitly unconfirmed."
    artifacts:
      - path: "tests/core/test_nmi_state.c"
        issue: "Existing checks do not establish the combined frame-zero, empty-PCM, nonzero-history property."
    missing:
      - "A concrete automated combined witness that checks canonical state and preserved history without a fabricated PCM sample."
unverified_prohibitions:
  - statement: "Implementation and test evidence must never incorporate GPL/LGPL emulator source or unmanifested ROM/BIOS bytes."
    verification: judgment
    disposition: unverified
    flagged: true
    evidence: "No contamination found in this scan and scripts/hygiene.sh --tree exited 0, but no wired enforcement evidence resolves the judgment-tier prohibition."
behavior_unverified_items:
  - truth: "D-04: Both flag-clear and flag-set $2002 reads take bit 7 at M2 rise and sprite bits 6/5 at M2 fall, with one read's low bits and side effects."
    test: "Exercise the production $2002 bus decoder with sprite status changing between its early and late M2 samples."
    expected: "Returned bit 7 reflects the early sample, bits 6/5 reflect the late sample, and the read's low-bit/open-bus and status side effects occur once."
    why_human: "The implementation has distinct early and late samples and is wired, but the existing bus-read regressions keep sprite bits constant across both samples, so no test distinguishes rise sampling from fall sampling."
---

# Phase 20: NMI line and VBlank flag Verification Report

**Phase Goal:** A game that reads `$2002` or toggles NMI around the start or end of VBlank gets the console's flag value and exactly the console's NMIs, and its save states hold through that race.

**Verified:** 2026-10-27T02:15:32Z  
**Status:** gaps_found  
**Re-verification:** No

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | All six named AccuracyCoin rows and owned race reproducers pass with the source-selected whole-sequence values. | ✓ VERIFIED | `build/ci/accuracycoin/full/report.tsv` contains all six passing target rows; its SHA-256 is `29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`. The exact-key gate and wrong-row controls pass. Actual stored vectors and source acceptance are recorded in `20-EVIDENCE.md`. |
| 2 | All six supported platforms produce identical frame/audio hashes, with every moved pin explained. | ? UNCERTAIN | Hash inventory generation, comparison wiring, local measured pins, and causes exist. The required exact-HEAD six-platform hosted results are absent; only local native evidence is available. |
| 3 | VBlank-set, VBlank-clear, and edge-before-poll states replay through core, runner, and libretro with format 2 and format-1 refusal. | ✓ VERIFIED | `tests/core/test_nmi_state.c`, `tests/core/test_state_fork.c`, runner state CTests, and `tests/libretro/libretro_host.c` exercise production save/load paths. The 683-test CI workflow passes; state format is pinned to 2, and malformed/format-1 loads are rejected. |
| 4 | No pre-existing passing main row is lost on any CI lane. | ? UNCERTAIN | Local cold protected-main observation and 108-key no-lost-pass controls pass. No exact-HEAD hosted lane results/artifacts were obtained, so the roadmap's every-lane claim remains unresolved. |
| 5 | D-01: Owned PPU-dot, real-bus/program and pinned AccuracyCoin evidence use separate oracles. | ✓ VERIFIED | `tests/ppu/test_nmi_races.c`, `tests/core/test_nmi_poll.c`, and the pinned campaign report use separate owned checks. |
| 6 | D-02: Expected values identify source, sample coordinates, starting state, and event order rather than copying measured output. | ✓ VERIFIED | Test comments, `20-NMI-ORACLE-RESEARCH.md`, preparation references, and measurement records identify the external source tables and cutpoints. |
| 7 | D-03: Start/end boundary, suppression, retained-edge, mirror, and normal-control cases are distinguished. | ✓ VERIFIED | `ppu.nmi_races` and `core.nmi_poll` are registered and passed in CI; the source contains before/on/after and mirrored-address cases. |
| 8 | D-04: A `$2002` read independently takes bit 7 at M2 rise and bits 6/5 at M2 fall while preserving read side effects. | ⚠️ PRESENT_BEHAVIOR_UNVERIFIED | `src/bus.c` captures and composes at distinct points and is reached by the decoder test. However, `tests/ppu/test_nmi_races.c` seeds sprite bits identically before and after M2, so the behavior is not discriminated by an automated test. See actionable gaps. |
| 9 | D-05: NMI enable/disable, unchanged `$80`, `$00->$80`, cleared flag, both boundaries, and STA/INC markers produce the source-expected extra NMIs. | ✓ VERIFIED | Guest program tests execute actual STA and INC writes, including both RMW writes, and inspect handler/next-instruction markers. AccuracyCoin Control 8/9 pass. |
| 10 | D-06: Green guards run normally and expected-red reproducers cannot pass on setup failure, crash, or unrelated assertion. | ✓ VERIFIED | `nmi_expect_red` checks the named assertion and normal termination; expected-red cases were removed for fixed targets. CI passes the owned cases. |
| 11 | D-07: Seven bounded wrong-model controls hit their distinct intended oracle. | ✓ VERIFIED | Race/poll tests include bounded mutations for edge timing, capture revocation, unchanged enable, RMW write loss, early service, and table shifts; they pass with the intended checks. |
| 12 | D-08: Derived NMI line, previous-line sample, and captured request remain separate; lowering the line cannot revoke captured work. | ✓ VERIFIED | `src/ppu.c`, `src/bus.c`, and `src/cpu.c` maintain distinct line/capture/poll state; state and guest-marker tests cover deassertion after capture. |
| 13 | D-09: The +9/+15 half-master-tick accesses, register side effects, APU phase, mapper stamps, and DMA accesses remain coherent. | ✓ VERIFIED | Production bus cycle code retains the ordered catch-ups and performs accesses once; relevant bus, mapper, DMA, and NMI regressions pass in CI. |
| 14 | D-10 amended: finite-margin circuit evidence constrains capture, with one global deterministic tie policy and no unsupported physical-tie winner claim. | ✓ VERIFIED | Six independent finite-margin waveforms are checked against production close samples; the selected global order is documented as bus side effect, detector capture, tied PPU transition. Exact paired-chip tie precedence is expressly retained as a source limit, not claimed as measured. |
| 15 | D-11: Default alignment is tested against complete tables before any global phase adjustment. | ✓ VERIFIED | Measurements state no per-test offset or VBlank-dot shift was applied; complete table checks and absolute-dot witnesses pass. |
| 16 | D-12: Each source table accepts only complete alternatives and explicitly ignored cells, with phase/history instance-local. | ✓ VERIFIED | The six-row report includes executed full vectors. Owned controls reject mixed alternatives; instance state holds the running fields. |
| 17 | D-13: PPU-to-CPU line boundary and IRQ writer ownership remain at existing seams. | ✓ VERIFIED | `src/ppu.c` derives the line, `src/bus.c` samples it, and CPU NMI state is consumed in `src/cpu.c`; existing IRQ ownership and vector tests pass. |
| 18 | D-14: The state format/revision transition to format 2/revision 8 preserves error precedence and refusal atomicity. | ✓ VERIFIED | Public pin, state loader checks, malformed-state controls, and `state.format_pin` pass. |
| 19 | D-15: Persistent capture/poll history is serialized, validated, and reset with the causal change. | ✓ VERIFIED | `src/state.c` visits and validates `nmi_prev`, `nmi_pending`, and `nmi_polled`; CPU reset clears them; state rejection and replay tests pass. |
| 20 | D-16: CRC-correct malformed bounds and unreachable states reject before mutation; real histories remain loadable and canonical. | ✓ VERIFIED | `tests/core/test_state_reject.c` and NMI state controls exercise production validation, atomic refusal, and canonical save/load/save. |
| 21 | D-17: Exact internal cutpoints use production visitor/loader at resumable boundaries, labeling constructed origins and executed endpoints. | ✓ VERIFIED | `tests/core/test_nmi_state.c` reports constructed origins separately from instruction-reached VBlank set/clear and edge/poll cutpoints. |
| 22 | D-18: Exact set/clear states include coherent nonzero audio history, and edge-before-poll feasibility is bounded. | ✓ VERIFIED | `20-EVIDENCE.md` records exact coordinates, nonzero synth/APU values, and a 20-case bounded Control 8/9 sweep. |
| 23 | D-19: Restored continuation matches native state, frame/tick/audio, and independent guest markers, including omitted-history and instance controls. | ✓ VERIFIED | Core state fork, runner, and libretro replay tests compare continuation outputs and markers; omitted-state and independent-instance controls pass. |
| 24 | D-20: Poll behavior, tests, state, docs, hashes, and campaign evidence move together as a causal change. | ✓ VERIFIED | Source, tests, README/header, measurements, scoreboards, and baseline provenance are present; the final local CI passes. |
| 25 | D-22: Pins, schemas, caps, existing CI placement, six hash lanes, conformance checks, and the +1% performance guard remain enforced. | ? UNCERTAIN | Local policy tests and workflow wiring exist and pass. Actual exact-HEAD six-platform, performance, and hosted conformance evidence is not recorded. |
| 26 | D-23: Changed hashes have old/new measurements and causes, with truthful campaign fingerprint and dirty-source metadata. | ✓ VERIFIED | `tests/accuracy/measurements.md` explains changed frame/audio/state values; metadata matches the campaign report and records `source_dirty=1` and the pre-existing config edit. |
| 27 | D-24: Running state remains owned, integer-only, allocation-free, per-instance, and within the three deliverables. | ✓ VERIFIED | Production state is in the emulator instance; two-instance tests pass; no runtime dependency or device layer was added. `AGENTS.md` ownership constraints remain satisfied. |
| 28 | KEEP-03 idempotency: save/load/save at a valid boundary yields identical canonical bytes and continuation. | ✓ VERIFIED | NMI state test executes production save/load/save and compares canonical bytes and resumed guest/video/tick/audio state. |
| 29 | KEEP-03 concurrency: interleaved instances with different NMI history remain independent. | ✓ VERIFIED | State fork and two-instance controls interleave independent instances and compare each continuation. |
| 30 | KEEP-02 adjacency: equal-tick frame/audio/state keys remain distinct and duplicate/missing key controls reject. | ? UNCERTAIN | Plan 20-06 explicitly says this backstop remains unconfirmed; current test evidence does not establish the held-out property. |
| 31 | KEEP-02 empty: frame-zero/empty-PCM state retains canonical state and nonzero audio history without inventing PCM. | ? UNCERTAIN | Plan 20-06 explicitly says this backstop remains unconfirmed; the required combined witness has not been demonstrated. |
| 32 | KEEP-02 ordering: generated complete hash inventory is deterministic and rejects equal keys. | ✓ VERIFIED | `runner.write_hashes`, inventory self-test, and `state.format_pin` pass; generated inventory matches the checked-in inventory. |

**Score:** 26/32 truths verified (1 present, behavior-unverified; 5 uncertain)

### Required Artifacts

The six plan artifact queries report **22/22 passed**. I inspected substantive behavior and wiring as well; no artifact is missing or a placeholder.

| Plan | Artifacts | Status | Details |
|---|---|---|---|
| 20-01 | `tests/ppu/test_nmi_races.c`, `tests/core/test_nmi_poll.c`, `tests/nmi_fixture.h`, `tests/CMakeLists.txt` | ✓ VERIFIED | Production decoder/CPU paths are exercised and CTests are registered. |
| 20-02 | `src/bus.c`, `src/ppu.c`, `src/state.c`, `include/nesturbator.h` | ✓ VERIFIED | Split status samples, captured line, state format 2, and public state contract are substantive. |
| 20-03 | `src/cpu.c`, `src/internal.h`, `src/state.c`, `tests/core/test_nmi_poll.c` | ✓ VERIFIED | Poll and instance fields are consumed, serialized, reset, and exercised. |
| 20-04 | `tests/core/test_nmi_state.c`, `tests/core/test_state_reject.c`, `tests/accuracy/measurements.md` | ✓ VERIFIED | Exact state witnesses, rejection controls, and measured cutpoints exist. |
| 20-05 | `tests/core/test_state_fork.c`, `tests/cmake/runner_state.cmake`, `tests/runner/nmi_fixture.c`, `tests/libretro/libretro_host.c` | ✓ VERIFIED | Real core, runner and libretro save/load interfaces are exercised. |
| 20-06 | `tests/cmake/nmi_targets.cmake`, `tests/accuracy/baseline-metadata.txt`, `20-EVIDENCE.md` | ✓ VERIFIED | Exact-six row gate, authenticated local provenance, and hosted limits are documented. |

### Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| `tests/ppu/test_nmi_races.c` | `src/bus.c` | Production `nesturbator__bus_read/write`, ordinary and mirrored PPU addresses | WIRED | Source calls production decoder; sprite-between-samples assertion coverage remains the D-04 test gap. |
| `tests/core/test_nmi_poll.c` | `src/cpu.c` | `nesturbator__cpu_step` and guest RAM markers | WIRED | Actual STA/INC programs and handler ordering are asserted. |
| `src/ppu.c` | `src/bus.c` | `nesturbator__ppu_nmi_line` consumed by bus-cycle sampling | WIRED | Derived status/enable line is not a static return. |
| `src/bus.c` | `src/cpu.c` | Captured request and prior sample feed CPU poll state | WIRED | Bus cycle sequence captures line; poll reads captured history. |
| `src/internal.h` | `src/state.c` | Instance latch fields visited, validated, and serialized | WIRED | Format-2 fields are included in production state handling. |
| `tests/core/test_nmi_state.c` | `src/state.c`, `src/cpu.c` | Production save/load plus resumed instructions | WIRED | Exact cutpoints use completed instructions and compare resumed output. |
| `tests/cmake/runner_state.cmake` | `tests/runner/nmi_fixture.c` | Runner CTest executes owned NMI fixture through CLI state commands | WIRED | Real runner path checks restored suffix and guest markers. |
| `tests/libretro/libretro_host.c` | libretro adapter state API | `retro_serialize`/`retro_unserialize` and run-ahead host loop | WIRED | Host boundary replay is distinct from internal race cutpoints. |
| `tests/cmake/nmi_targets.cmake` | `tests/accuracy/scoreboard.txt` | Exact six full-key pass checks and mutated controls | WIRED | Gate rejects missing, duplicated, mistyped, or failed row. |
| `.github/workflows/ci.yml` | `tests/runner/hashes.txt` | Six matrix artifacts are downloaded and compared by hash-inventory script | WIRED | Workflow wiring exists; exact-HEAD hosted outcome remains unobserved. |

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
|---|---|---|---|---|
| AccuracyCoin scoreboard | Guest result rows | Pinned campaign executes ROM and writes actual guest RAM; report hash/provenance is checked by activation | Yes | ✓ FLOWING locally; target rows pass |
| NMI poll/state results | Guest markers and serialized fields | PPU status/control → bus capture → CPU poll → state visitor/loader → resumed CPU | Yes | ✓ FLOWING |
| Native hash inventory | Frame, PCM, transition, state digests | Runner produces hashes from emulated output and state; CMake validates exact keys | Yes | ✓ FLOWING locally; hosted equality is uncertain |
| Host save-state replay | Frame and PCM suffixes | Actual runner/libretro serialization calls and subsequent core execution | Yes | ✓ FLOWING |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|---|---|---|---|
| Whole-project CI contract and three deliverables | `cmake --workflow --preset ci` | Parent run: exit 0, 683/683, 450.68s; all three packages produced (`/private/tmp/nesturbator-20-regression.log`) | ✓ PASS |
| NMI/save-state tests and all CI regressions | Included in workflow above (`core.nmi_state`, `runner.state.nmi_roundtrip`, `runner.state.nmi_wrong_load`, `core.state_fork.nmi`, `libretro.runahead`, `ppu.nmi_races`, `core.nmi_poll`) | Passing in 683/683 run | ✓ PASS |
| Full AccuracyCoin report | Final local full campaign, recorded in `20-EVIDENCE.md` | 22/22 pages; 146 scored, 114 pass/32 fail, 5 DRAW; all six NMI target keys pass | ✓ PASS |
| Exact six-key gate | `ctest --preset ci -R '^accuracycoin\\.nmi_targets$' --no-tests=error --output-on-failure` | 1/1 with changed/absent/duplicate/mistyped negative controls rejected | ✓ PASS |
| Sanitizer workflow | `cmake --workflow --preset asan` | 675/675 passed (local plan evidence) | ✓ PASS |
| Source hygiene tree check | `bash scripts/hygiene.sh --tree` | Exit 0 | ✓ PASS |
| Independent finite-margin circuit probe | `python3 .planning/phases/20-nmi-line-and-vblank-flag/probes/20-nmi-netlist-oracle-fetch.py` | Exit 0; six independent fresh-state finite-margin pulse cases pass; no exact clock ties tested | ✓ PASS |
| Dynamic sprite-bit split behavior | No existing test varies sprite bits across the M2 samples | Implementation is present and wired but the discriminating transition is not exercised | ? UNCERTAIN |

### Probe Execution

| Probe | Command | Result | Status |
|---|---|---|---|
| `.planning/phases/20-nmi-line-and-vblank-flag/probes/20-nmi-netlist-oracle-fetch.py` | `python3 .planning/phases/20-nmi-line-and-vblank-flag/probes/20-nmi-netlist-oracle-fetch.py` | Exit 0; six finite-margin pulse cases passed; explicit source limit remains for exact physical ties | PASS |
| Conventional `scripts/*/tests/probe-*.sh` | Discovery | No phase-declared or conventional shell probe found | N/A |

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|---|---|---|---|---|
| NMI-01 | 20-01, 20-03, 20-06 | `$2002` around VBlank start reports the console flag/NMI behavior; flag timing, timing and suppression rows pass. | SATISFIED locally | All three pinned rows pass and owned bus/status cases are in CI. Existing checkbox is not edited by this verification. |
| NMI-02 | 20-01, 20-03, 20-06 | NMI enable/disable around VBlank start/end matches console behavior, including the extra Bases Loaded II NMI; target rows pass. | SATISFIED locally | Control, end, and disabled rows pass; STA/INC guest marker tests exercise the extra NMI. Existing checkbox is not edited by this verification. |
| KEEP-02 | 20-04, 20-06 | Re-pinned frame/audio hashes are explained and identical on all six platforms. | PARTIAL — evidence outstanding | Local measured changes and reasons exist; exact-HEAD six-platform artifacts and two declared automated backstops remain outstanding. Requirement remains unchecked. |
| KEEP-03 | 20-04, 20-05, 20-06 | Save state at any frame/race/interrupt resumes identically through rewind/run-ahead; older format is refused. | PARTIAL — evidence outstanding | Local exact cutpoints, runner/libretro replay, canonical save/load/save, and format-1 refusal pass. Exact-HEAD hosted run/performance/host evidence is pending. Requirement remains unchecked. |

All PLAN requirement IDs are represented above. REQUIREMENTS.md maps exactly these four IDs to Phase 20; no orphaned Phase 20 requirement was found.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---:|---|---|---|
| `include/nesturbator.h` | 258-261 | Public comment says clearing NMI enable suppresses a pending NMI | WARNING (WR-01) | Contradicts the later API/state description and implementation: deasserting the line does not revoke an already captured request. |
| `tests/ppu/test_nmi_races.c` | 136-185, 195-221 | Sprite bits are unchanged across the two `$2002` samples | WARNING (WR-02) | Tests do not fail if bits 5/6 are sampled at M2 rise; recorded as D-04 behavior-unverified, not as an absent implementation. |

No unreferenced `TBD`, `FIXME`, or `XXX` debt markers, placeholder implementations, or disabled requirement-linked tests were found in the phase's changed implementation/test files. Test-quality audit found no generated oracle sourced from the candidate output and no circular test-data path. The deterministic decision gate reports 24/24 project decisions honored.

### Actionable Evidence Gaps

1. **Exact-HEAD hosted artifacts**

   **Action:** In the separately authorized delivery step, collect the hosted six-platform hash, protected-main, per-ROM performance, and configured host-check artifacts for exact HEAD `46e2af6458fd`.
   **Expected:** Identical frame/audio/state inventories, all 108 original protected PASS rows retained, no performance regression above +1%, and hosted vector/MMC3/Holy Mapperel/boot-to-play/real-host checks passing. Record source cleanliness and campaign identity truthfully.
   **Why this is a gap:** No hosted job or artifact identity was obtained. Local controls pass, but cannot establish what the hosted platforms and performance lane produced. Plan 20-06 excludes triggering push/PR/merge activity in this verification step.

2. **Missing split-sample regression case**

   **Action:** Add an automated production-decoder `$2002` case where bit 5 or 6 changes after M2 rise and before M2 fall.
   **Expected:** Bit 7 comes from the early sample, sprite bits 6/5 from the late sample, and open-bus/status side effects happen once.
   **Why this is a gap:** No current test exercises this transition; symbol presence and ordinary constant-bit reads cannot establish the sample ordering.

3. **KEEP-02 equal-tick adjacency backstop**

   **Action:** Add a concrete automated equal-tick inventory case with duplicate and missing-key controls.
   **Expected:** Lane identity remains distinct and invalid controls are rejected.
   **Why this is a gap:** The plan marks this a non-inferable backstop and the evidence ledger leaves it unconfirmed.

4. **KEEP-02 empty-PCM/frame-zero backstop**

   **Action:** Add a concrete automated combined frame-zero, empty-PCM, nonzero-audio-history continuation witness.
   **Expected:** Canonical state and history are retained without fabricating a PCM sample.
   **Why this is a gap:** This declared backstop is explicitly unconfirmed; existing separate fixture coverage does not prove the combined property.

The clean-room prohibition remains flagged as `unverified-prohibition — human review recommended`. The local hygiene command passed and no contamination was found, but this is not an authoritative judgment or enforcement result. It is not converted into a separate sign-off request here.

### Gaps Summary

The local implementation and owned NMI/state tests work, including the six measured AccuracyCoin rows. The required hosted exact-HEAD results are missing, two KEEP-02 automated witnesses remain unconfirmed, and the split-sample implementation lacks a discriminating regression test. These are actionable acceptance-evidence gaps, so the verdict is `gaps_found`. The clean-room prohibition remains visibly flagged but unverified; no contamination was found.

The amended D-10 tie policy is not a gap: one globally applied source-consistent tie convention is implemented and documented, while physical paired-chip tie precedence remains an explicit source limit.

---

_Verified: 2026-10-27T02:15:32Z_  
_Verifier: the agent (gsd-verifier)_
