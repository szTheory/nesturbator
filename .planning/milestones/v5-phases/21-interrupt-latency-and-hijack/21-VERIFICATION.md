---
phase: 21-interrupt-latency-and-hijack
verified: 2026-10-30T13:29:50Z
status: passed
score: "32/32 truths verified"
covered_files:
  - .github/workflows/ci.yml
  - .planning/phases/21-interrupt-latency-and-hijack/21-01-PLAN.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-01-SUMMARY.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-02-PLAN.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-02-SUMMARY.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-03-PLAN.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-03-SUMMARY.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-04-PLAN.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-04-SUMMARY.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-05-PLAN.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-05-SUMMARY.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-06-PLAN.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-06-SUMMARY.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-07-PLAN.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-07-SUMMARY.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-08-PLAN.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-08-SUMMARY.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-09-PLAN.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-09-SUMMARY.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-EVIDENCE.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-EXECUTION-EVIDENCE.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-INTERRUPT-TIMING-ANNEX.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-REVIEW-DISPOSITION.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-REVIEW-HISTORY.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-REVIEW.md
  - .planning/phases/21-interrupt-latency-and-hijack/21-STATE-MATRIX.md
  - README.md
  - include/nesturbator.h
  - src/bus.c
  - src/cpu.c
  - src/internal.h
  - src/state.c
  - tests/CMakeLists.txt
  - tests/accuracy/baseline-metadata.txt
  - tests/accuracy/measurements.md
  - tests/accuracy/migration.tsv
  - tests/accuracy/scoreboard.txt
  - tests/cmake/accuracycoin_activation.cmake
  - tests/cmake/accuracycoin_baseline_fetch.cmake
  - tests/cmake/accuracycoin_regression_controls.cmake
  - tests/cmake/accuracycoin_v5_targets_control.cmake
  - tests/cmake/phase20_hosted_evidence.cmake
  - tests/cmake/phase20_hosted_evidence_controls.cmake
  - tests/cmake/phase21_hosted_evidence.cmake
  - tests/cmake/phase21_hosted_evidence_controls.cmake
  - tests/cmake/phase21_source_identity.cmake
  - tests/cmake/phase21_source_identity_controls.cmake
  - tests/cmake/skip_policy.cmake
  - tests/cmake/v5_targets.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/core/test_api.c
  - tests/core/test_apu.c
  - tests/core/test_controller.c
  - tests/core/test_interrupt_entry.c
  - tests/core/test_interrupt_state.c
  - tests/core/test_irq_poll.c
  - tests/core/test_nmi_state.c
  - tests/core/test_state_fork.c
  - tests/core/test_state_generation.c
  - tests/core/test_state_reject.c
  - tests/core/test_state_runahead.c
  - tests/cpu/test_cpu_unit.c
  - tests/cpu/vector_bus.c
  - tests/cpu/vector_bus.h
  - tests/fuzz/state_loader.c
  - tests/header/header_c.c
  - tests/header/header_cxx.cpp
  - tests/interrupt_fixture.h
  - tests/libretro/libretro_host.c
  - tests/retroarch/state_session.sh
  - tests/retroarch/test_state_session.sh
  - tests/runahead_model.h
  - tests/runner/hashes.txt
  - tests/state_test.h
covered_digest: "v4:sha256:62f1d2999791be7a7ec890edcaad20b824da67900b731bcae69f1d95ed5281f8"
behavior_unverified: 0
overrides_applied: 0
unverified_prohibitions:
  - statement: "Implementation and tests must never incorporate GPL/LGPL emulator source or unmanifested ROM/BIOS bytes."
    status: flagged-unverified
    verification: judgment
    reason: "Plan 21-09 marks this intent constraint unresolved; it is not a human sign-off gate."
  - statement: "Accuracy claims must never substitute title/row-specific timing or selectively shifted handler tables for one source-grounded shared hardware model."
    status: flagged-unverified
    verification: judgment
    reason: "Plan 21-09 marks this intent constraint unresolved; it is not a human sign-off gate."
  - statement: "Evidence must never claim a physical exact-tie winner or source-bound hosted acceptance that was not measured."
    status: flagged-unverified
    verification: judgment
    reason: "Plan 21-09 marks this intent constraint unresolved; physical zero-margin precedence remains unmeasured."
---

# Phase 21: Interrupt latency and hijack — Verification Report

**Phase Goal:** A game's IRQ starts on the same instruction as on the console, and an NMI arriving during BRK or an IRQ takes over its vector as on the console.
**Verified:** 2026-10-30T13:29:50Z
**Status:** passed
**Re-verification:** No; the prior report had no `gaps:` section, so this is an initial-mode goal audit.

## Goal Achievement

I verified the four roadmap success criteria and all 28 truths in the nine plan frontmatters. Production code keeps IRQ eligibility at its instruction poll, carries the accepted decision into interrupt entry, and selects an NMI vector at the documented entry cutoff. Production bus tests, saved-state tests, and the actual libretro module tests exercise those transitions. The current local CI workflow passes.

Evidence has two source scopes. Hosted six-platform acceptance, campaign data, and the RetroArch session are bound to the recorded candidate `54c7db29c73a66bb552e71b0887529249a41a072`, tested merge `faf1b9159ce8dae83cd2c08c155fc9b77fac32de`, and protected base `a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`. Current `HEAD` is `ab790a8b6b03`, a descendant with later workflow, documentation, measurement, policy, and session-test edits. The fresh local CI result below covers this checkout; no new hosted qualification is claimed for current HEAD. The production CPU, bus, and state files are unchanged from the recorded hosted candidate.

| # | Observable truth | Status | Evidence |
|---|---|---|---|
| R1 | The Interrupt flag latency, NMI Overlap BRK, and NMI Overlap IRQ targets pass, and owned CLI/SEI/PLP/RTI/branch and BRK/IRQ cycle cases run in CI. | VERIFIED | Current `core.irq_poll`, `core.interrupt_entry`, and `cpu.unit` pass in the fresh workflow; the source-bound hosted 11-row gate also passed. |
| R2 | All eleven v5 rows pass and previously passing keys remain protected in each accepted CI lane. | VERIFIED | The accepted hosted candidate records 11/11 targets and all 114 protected keys across six lanes; current local exact-target and no-lost-pass checks pass. No current-HEAD hosted rerun is asserted. |
| R3 | CPU vectors, nightly MMC3, Holy Mapperel, and boot-to-play remain green with the shared entry path. | VERIFIED | Hosted acceptance records vectors 258/258, MMC3 13/13, Holy Mapperel, and boot-to-play across six lanes; current `runner.boot_to_play` passes. |
| R4 | Mid-entry native state and libretro run-ahead resume the same run, with matching moved hashes across the six accepted platforms. | VERIFIED | Current native and actual-module run-ahead tests pass; the accepted hosted evidence records six equal hash inventories and a complete RetroArch session for its bound source. |
| 1.1 | A real APU IRQ traverses production source update, bus capture, legal poll, entry accesses, and an observable guest handler. | VERIFIED | `test_real_apu_irq_reaches_handler` uses the real APU and production bus path; `core.irq_poll` passes. |
| 1.2 | Old-I CLI/SEI/PLP, restored-I RTI, and every branch opcode exercise the legal poll timing and pushed state. | VERIFIED | Owned exact access/stack tables and production guest cases cover the flag transitions and eight branch opcodes; `cpu.unit` and `core.irq_poll` pass. |
| 1.3 | The existing integer timeline and sole IRQ writer remain; owned tests add no device code, dependency, ROM asset, or public API, and the preexisting config edit is excluded. | VERIFIED | Source and test wiring show owned fixtures and the production IRQ writer; the current config edit is outside phase inputs as Plan 21-01 states. No API or dependency was added. |
| 2.1 | IRQ eligibility is committed at legal polls with correct old/restored-I semantics and survives later line/I changes. | VERIFIED | `src/cpu.c` stores accepted history per instance; poll, CPU, entry, and state replay tests exercise the transition. |
| 2.2 | State format 3 and behavior revision 9 carry bounded accepted history, preserve ABI 1, and reject formats 1/2 before mutation. | VERIFIED | Header constants are format 3, revision 9, ABI 1; visitor validation runs before APPLY and state rejection/generation tests pass. |
| 2.3 | Poll changes, README/header behavior documentation, campaign accounting, hash pins, and protected keys are covered. | VERIFIED | The README save-state section and public header describe revision 9; measured outputs and pins are covered. A stale README intro line and WR-02 attribution discrepancy are reported below as documentation warnings. |
| 3.1 | Source-derived finite-margin schedules constrain origin, vector cutoff, and coherent paired results without claiming a physical exact tie. | VERIFIED | Production bus tables and finite-margin schedules pass; the annex explicitly leaves physical zero-margin precedence unmeasured. |
| 3.2 | BRK/IRQ cycle arrivals, held and missed requests, paired tables, and wrong-model controls are exercised. | VERIFIED | `core.interrupt_entry` covers C1–C7 arrivals, complete BRK/IRQ tables, request lifetime, and negative controls. |
| 4.1 | BRK, IRQ, and NMI share instance-owned staged entry with origin semantics, coherent vector choice, and one-time request consumption. | VERIFIED | Entry begin/action/drain code is exercised by origin, overlap, simultaneous-request, and consumption cases. |
| 4.2 | Only interrupt entry is resumable; ordinary CPU/frame execution drains it synchronously without a new stepping API or runtime snapshot callback. | VERIFIED | Public synchronous execution drains entry actions; API inspection and ordinary continuation tests confirm no new public stepping API. |
| 4.3 | Persistent fields use the canonical visitor and reachable bounds before APPLY; state format and serialized hashes remain coherent. | VERIFIED | `src/state.c`, the state matrix, CRC-correct rejection tests, header, and current state tests agree; `core.interrupt_state` passes. |
| 5.1 | Only source-reachable entry combinations load, and valid states remain canonical across save/load/save. | VERIFIED | State rejection tests cover round trips and invalid entry combinations; `core.state_reject` passes. |
| 5.2 | Reset ends partial entry; invalid states and old formats preserve error order, instance bytes, and generation. | VERIFIED | `core.state_generation` and `core.state_reject` pass reset and refusal controls. |
| 6.1 | Every resumable BRK/IRQ/NMI action saves, loads, and continues through ordinary execution with exact trace and future output equality. | VERIFIED | Native tests cover 20 production cutpoints and compare traces, ticks, CPU state, markers, pixels, PCM, frame data, and transitions; `core.interrupt_state` passes. |
| 6.2 | Nonzero audio history, interleaved instances, state forks, and omitted-history controls rule out boundary-only substitutes. | VERIFIED | Interleaving and synthesis comparisons, entry state-fork coverage, and expected-divergence controls pass. |
| 7.1 | The actual libretro module's `retro_unserialize` resumes every entry origin/stage under run-ahead and matches committed/speculative output. | VERIFIED | Host dynamically loads the production module and compares state, ticks, video, PCM, frame metadata, and markers at depths 1–3; `libretro.runahead` passes. |
| 7.2 | Stale/skipped loads and omitted history fail adapter comparisons while rewind, run-ahead, and board-family cases remain. | VERIFIED | Adapter negative controls exercise intended rejection paths; the current libretro and state-fork suites pass. |
| 8.1 | Fresh authority protects all 114 prior PASS keys, and all eleven exact v5 target rows pass without aggregate-score substitution. | VERIFIED | Historical hosted acceptance is source-bound and exact-key; current `accuracycoin.v5_targets` and `accuracy.scoreboard.no_lost_pass` pass locally. |
| 8.2 | Six-platform equality, performance, sanitizer, host, vector, MMC3, Holy Mapperel, and boot gates remain present and pass. | VERIFIED | The accepted hosted evidence records these outcomes and the six matching inventories; current workflow regression passes. It does not requalify current HEAD remotely. |
| 8.3 | Local preparation is not represented as hosted acceptance; deferred collector debt and branch/config boundaries remain explicit. | VERIFIED | Evidence ledger distinguishes local preparation from the accepted run; review history and disposition retain deferred CR/WR items. Current HEAD is not newly hosted-qualified. |
| 8.4 | Candidate, tested merge, and protected base remain distinct; source mapping is limited to matching effective inputs. | VERIFIED | The ledger binds candidate, tested merge, base, and the recorded 428-path inventories and digests. |
| 8.5 | Empty or missing authorities, lane results, artifacts, and inventories fail closed. | VERIFIED | Phase21 collector negative controls cover absent/empty and mismatch cases and pass in current CI. |
| 8.6 | Full-key and sorted hash comparisons reject duplicates, omissions, and platform-order changes. | VERIFIED | Collector and target negative controls pass; accepted six-platform inventories are complete and equal. |
| 9.1 | Actual hosted CI/Nightly contents and provenance pass for the source they accepted. | VERIFIED | CI `40929011059/1` passed 18/18; Nightly `40929010997/1` had seven work jobs pass and its declared PR-only report job skipped; 16 artifact ZIPs matched API digests. This evidence is for the recorded candidate only. |
| 9.2 | Acceptance binds candidate, tested merge, base, inputs/modes/blobs, run/attempt/artifact IDs, without delivery mutation. | VERIFIED | `21-EVIDENCE.md` and its manifest distinguish identities; recorded validation exited 0. The current verification made no remote mutations. |
| 9.3 | Finite-margin ordering follows the global integer tie policy without asserting the physical zero-margin winner. | VERIFIED | Timing annex and tests distinguish finite-margin observations from deterministic model policy; no physical exact-tie claim is made. |
| 9.4 | Without a new eligible NMI edge, BRK/IRQ retain their vector/origin; a held consumed level is not another request. | VERIFIED | Empty-request, held-consumed-level, and cancellation cases pass in `core.interrupt_entry`. |
| 9.5 | Simultaneous eligible requests prioritize NMI, preserve origin PC/B semantics, and consume each request once. | VERIFIED | Arbitration, origin, stack, consumption, and new-edge controls pass in `core.interrupt_entry`. |

**Score:** 32/32 roadmap and plan truths verified (0 present but behavior-unverified).

## Required Artifacts

The artifact query passed all 29/29 plan-declared artifacts. I also traced their wiring to production C entry points, CMake/CTest registration, state visitor use, or the evidence collector as applicable.

| Plan | Artifacts checked | Status |
|---|---|---|
| 21-01 | `test_irq_poll.c`, `test_cpu_unit.c`, `interrupt_fixture.h` | VERIFIED, 3/3 |
| 21-02 | `cpu.c`, `state.c`, public header, `measurements.md` | VERIFIED, 4/4 |
| 21-03 | `test_interrupt_entry.c`, CPU test, fixture | VERIFIED, 3/3 |
| 21-04 | CPU entry, internal state, visitor, entry tests | VERIFIED, 4/4 |
| 21-05 | State rejection/generation tests, state matrix | VERIFIED, 3/3 |
| 21-06 | Native interrupt replay, fixture, state fork | VERIFIED, 3/3 |
| 21-07 | Libretro host, run-ahead model, public run-ahead test | VERIFIED, 3/3 |
| 21-08 | Exact targets, hosted wrapper/controls, preparation evidence | VERIFIED, 4/4 |
| 21-09 | Hosted ledger and measurement record | VERIFIED, 2/2 |

## Key Link Verification

The literal-pattern query found 2/23 links. I manually traced the remaining C calls, visitor fields, CMake registrations, and evidence paths; all 23 are connected. The query undercounts these relationships because they cross C/test fixtures and generated evidence contracts.

| Plan | Manual trace |
|---|---|
| 21-01 | Production APU/bus IRQ guest and separate CPU sequence fixture are wired to registered tests. |
| 21-02 | CPU acceptance uses instance fields; state visitor persists them; measurements map to scoreboard keys. |
| 21-03 | Entry fixture delegates bus operations to production code; timing tables trace to the pinned annex. |
| 21-04 | Each staged action calls production bus methods; state reload continues at the next action. |
| 21-05 | CRC-repaired test inputs reach production state validation; reset cases call CPU reset and ordinary execution. |
| 21-06 | Shared fixture reaches production actions; public save/load resumes them; state-fork registers entry cases. |
| 21-07 | Host loads the actual module and resolves `retro_unserialize`; fixture bytes feed the adapter cases. |
| 21-08 | Phase21 wrapper uses shared validators; CMake registers exact target and evidence controls; fresh authority flows to key comparison. |
| 21-09 | Evidence wrapper, delivery identities, and effective-input inventory feed the source-bound ledger. |

## Data-Flow Trace

| Data | Source and flow | Real observations | Status |
|---|---|---|---|
| IRQ eligibility | APU source → bus capture → legal poll → accepted CPU history → shared entry/guest marker | Real APU guest and CPU access trace | FLOWING |
| NMI takeover | PPU edge history → bus recognition/inhibition → staged vector choice → production vector reads | Finite-margin schedules and paired source tables | FLOWING |
| Native continuation | Entry fields → state visitor/validation → loader → next ordinary action → output | Trace and future frame/audio comparisons | FLOWING |
| Libretro continuation | Native cutpoint bytes → production `retro_unserialize` → run-ahead → committed/speculative state/output | Video, PCM, ticks, frame data, and markers | FLOWING |
| Cross-platform acceptance | Actual hosted artifacts and protected authority → collector → exact rows and sorted hashes | Six source-bound platform inventories and provenance | FLOWING for the accepted source |

## Behavioral Spot-Checks

No suite was rerun during this report refresh. The parent-provided fresh current checkout run is in `build/phase21-verification-refresh-ci.log`.

| Behavior | Evidence | Result |
|---|---|---|
| Current integrated build and regression | `cmake --workflow --preset ci` | Exit 0; 695/695 passed in 459.85 seconds; library, runner, and libretro packages generated. |
| Real APU IRQ, poll timing, branch cases | `core.irq_poll`, `cpu.unit` in the current workflow | Passed. |
| BRK/IRQ cycles, vector selection, request lifetime | `core.interrupt_entry` in the current workflow | Passed. |
| Native state replay, rejection, reset, entry fork | `core.interrupt_state`, `core.state_reject`, `core.state_generation`, `core.state_fork.entry` | Passed. |
| Actual libretro module run-ahead | `libretro.runahead` | Passed. |
| Exact target and no-lost-pass gates | `accuracycoin.v5_targets`, `accuracy.scoreboard.no_lost_pass` | Passed. |
| Boot-to-play | `runner.boot_to_play` | Passed. |

## Probe Execution

No Phase21 plan or summary declares a shell probe, and no `scripts/*/tests/probe-*.sh` file exists. Probe execution is not applicable; registered CTest targets provide the automated checks.

## Requirements Coverage

| Requirement | Source plans | Description | Status | Evidence |
|---|---|---|---|---|
| INT-01 | 21-01, 21-02, 21-08, 21-09 | IRQ begins on the console-equivalent instruction after CLI, SEI, PLP, RTI, and branches; Interrupt flag latency passes. | SATISFIED | Poll tables, real source guest, exact target gate, protected-key gate, and source-bound hosted record. |
| INT-02 | 21-03, 21-04, 21-06, 21-07, 21-08, 21-09 | NMI during BRK/IRQ takes over the vector; both overlap rows pass. | SATISFIED | Finite-margin arrival tests, paired vector tables, native and adapter replay, and accepted hosted target rows. |
| KEEP-01 | All nine plans | Existing passing AccuracyCoin rows and CPU vectors, MMC3, Holy Mapperel, and boot-to-play stay green. | SATISFIED | 114/114 protected keys and six hosted lanes on the accepted source; current local regression and boot test pass. |

Plans declare these three requirements. REQUIREMENTS.md maps exactly INT-01, INT-02, and KEEP-01 to Phase 21; there is no orphan Phase21 requirement. All three are marked complete in the requirements register.

## Decision Coverage

The GSD decision-coverage owner reports 26/26 trackable CONTEXT.md decisions honored, with no unhonored decisions. This includes the explicit source-grounded finite-margin rule, one global integer equal-timestamp policy, no physical exact-tie claim, and no judgment-tier manual gate.

## Anti-Patterns Found

| File | Pattern | Severity | Impact |
|---|---|---|---|
| None | No unreferenced TBD/FIXME/XXX, TODO/HACK/PLACEHOLDER, empty production stub, or disabled phase test was found. | None | No blocker. |

The `WILL_FAIL` property belongs to the named negative `holymapperel.derive.refuse` case; serialized integration tests are not disabled. Fixture-generated synthetic input is owned test data, not an expected-output oracle.

## Review Findings and Evidence Limits

- **WR-02 remains an open warning.** The historical session-control paragraph in `tests/accuracy/measurements.md` attributes the `2940249e30ef` capture, input digest `573904875fa3641acdc4936c2f116e2594c3366d2abe4eec70841af7d5ee4ac5`, and elapsed guest time `5577072000` to the canonical baseline. Current `tests/accuracy/baseline-metadata.txt` instead records source `d4802809ff41`, `source_dirty=1`, input digest `ab0bdf5142426482fa18fecae5c2c7272c4957b808f6c7aa8feaaf3b767cec80`, and `host_elapsed_guest_ns=5091122000`. These are separate captures. The current 56-input digest matches its metadata; the inaccurate attribution does not change the Phase21 runtime test results or create a claim of new hosted acceptance.
- **README revision summary needs reconciliation.** `README.md` line 29 still calls the behavior revision 8, while the public header and README save-state section say behavior revision 9. The detailed interrupt and state documentation is present, but the introductory revision sentence is stale.
- WR-01 remains deferred: the committed source-identity helper does not enforce chmod-only worktree mode changes. For the historical accepted run, independent comparison recorded matching mode/blob inventories for all 428 inputs; this mitigates that run without fixing the recurring helper.
- Phase20 CR-01/WR-03/WR-04 collector debt remains preserved in review history and disposition; their run-specific evidence does not close those maintenance items.
- Physical exact-tie CPU/PPU timestamp precedence remains unmeasured. The verified model claim is finite-margin ordering under deterministic integer timestamps.
- The three judgment-tier prohibitions are flagged-unverified in frontmatter. They are retained as intent/provenance limits and are not human testing or sign-off gates.
- Hosted CI `40929011059/1` passed 18/18. Nightly `40929010997/1` had seven work jobs pass and one declared PR-only report job skipped. The acceptance identities remain historical and source-bound to the recorded candidate.

## Human Verification Required

None. All phase behavior truths have automated transition and replay coverage. The judgment-tier prohibitions stay flagged without adding a manual gate.

## Gaps Summary

No blocking implementation, behavior, wiring, artifact, requirement, or accepted-source evidence gap was found. The phase goal is met under the documented finite-margin contract. The open campaign-attribution warning, stale README revision sentence, deferred collector items, and absence of new hosted qualification for current HEAD remain explicit limits; they do not alter the recorded implementation verdict.

---

_Verified: 2026-10-30T13:29:50Z_
_Verifier: the agent (gsd-verifier)_
