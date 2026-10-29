---
status: verifying
trigger: "Corroborate Phase21 review CR01/CR02 against primary hardware sources"
created: 2026-10-29
updated: 2026-10-29
---

## Current Focus

hypothesis: CR01 queue preservation is rejected; CR02 sticky promotion was the root cause and is repaired with separate inhibited-candidate history.
test: Completed before-fix reproduction, CR01 proposal mutant, CR02 exact revert/reapply and19-test focused/native/adapter/campaign verification.
expecting: Preserve all119 current PASS keys, all11 targets and427-source inventory without changing state bytes/format/revision or proven DMC/open-bus behavior.
next_action: Return source ownership/disposition to parent for coherent hashes/corpus/fullCI/ASAN/vectors/MMC3/source-candidate refresh and re-review; keep this causal repair uncommitted.
reasoning_checkpoint:
  hypothesis: Unconditional raw-edge capture promotes entry-inhibited pulses to sticky recognized NMI history, so release before T1 cannot cancel a request that hardware never recognized.
  confirming_evidence:
    - New production C5/C6/C7 sampled-pulse cases for BRK/IRQ/NMI all retain nmi_polled1 and enter8134 instead of handlerRTI return8001/8003 on unchanged runtime.
    - The first-handler branch pulse sibling also enters NMI unexpectedly; held sibling remains correct. Primary node-chain analysis explicitly documents loss before T1 recognition.
  falsification_test: Qualifying held/released candidates loses an already recognized edge, duplicates a consumed held input, changes Phase20 ordinary capture, or loses any119 current campaign PASS.
  fix_rationale: Preserve raw nmi_prev and recognized pending1, represent an inhibited candidate as value2 in the existing byte, cancel release and promote only held candidates at the first handler opening before its legal poll.
  blind_spots: C7 close-to-T1 release is deterministic model policy with zero-margin physical uncertainty; finite-margin C5/C6 cases provide the hardware discriminator. Native/adapter replay and state validation must cover candidate2 histories.
  candidate_causes: [code/sticky promotion, data/unsampled short-pulse fixture, environment/stale runner]
  and_gate: no; unconditional promotion is sufficient for the spurious service.
bug_class: Bohrbug
candidate_causes: [code/recognition inhibition, data/reviewer IRQ-queue assumption, data/existing unsampled short-pulse fixture]
and_gate: no for CR02; sticky promotion alone can cause the spurious service. CR01 is independently judged.

## Symptoms

expected: One NMI-priority entry consumes the arbitration event; only held IRQ can be reaccepted after I is restored. NMI pulses wholly within post-choice inhibition and released before recognition do not queue service; held late and already recognized fresh edges survive appropriately.
actual: Review reports discarded IRQ acceptance and retained inhibited NMI pulse; production code clears IRQ arbitration and captures every sampled NMI edge into one sticky byte.
errors: Two unproven blocker claims in committed 21-REVIEW.md.
reproduction: Production bus fixtures schedule simultaneous IRQ/NMI and finite-margin inhibited NMI pulses around staged entry.
started: Source-review followup after candidate1d27a0f.

## Eliminated

- hypothesis: Preserve a separately queued accepted IRQ through NMI entry.
  evidence: Primary shared-entry request-flop analysis conflicts with that model. Unchanged-runtime production ack/live controls pass: first NMI handler instruction runs with I1; ack suppresses later IRQ, held level is freshly accepted afterRTI.

## Evidence

- checked: Visual6502 State Machine, Interrupt Hijacking, MOS IRQ/NMI contract and actual source.
  found: Shared entry request bookkeeping is cleared after vector handling; IRQ remains level-sensitive. Post-choice stage1 NMI recognition is inhibited and released pulses can be lost.
  implication: Do not implement proposed independent IRQ queue; distinguish raw inhibited candidates from recognized NMI history.
- checked: Fresh build/entry-review-repair before-fix production regressions; red identity/log retained under build/entry-review-evidence.
  found: All nine post-choice pulse cases retain a spurious request; first-handler branch distinguishes held from released incorrectly. CR01 ack/live controls pass on unchanged runtime.
  implication: CR02 requires a minimal recognition-history correction; CR01 proposed behavior is rejected.
- checked: Exact CR01 proposed queue-preservation mutant.
  found: Both new ack/live regressions fail at retained irq_accepted1 and missing first NMI handler marker; unintended IRQ enters despite I1.
  implication: The proposed fix introduces an interrupt-priority defect and was removed.
- checked: CR02 recognition-site revert and reapplication.
  found: Revert restores sampled-pulse spurious service; final reapplication passes. Identity and output retained in cr02-revert-identity.json and cr02-revert.log.
  implication: Recognition separation is causal; hardware vector decision and ordinary capture timing remain unchanged.
- checked: Fresh final focused run and full campaign.
  found: 19/19 passed in68.11s;119PASS/27FAIL with no status/raw/frame row changes,all119 oldPASS retained,all11 targetsPASS,146terminal5DRAW. Source checker reports427 inputs.
  implication: Focused source repair is accepted; broader closeout and re-review remain pending.
- checked: Idle candidate handoff transition and CRC-correct controls requested by parent.
  found: Entry clears accepted/poll markers and C7 produces idle candidate2 before any opcode can JAM. New legal sampled-high/live-released candidate round-trips; IRQ accepted1, JAM1, raw IRQ poll1, NMI poll1 and sampled-low neighbors are rejected before APPLY.
  implication: Idle loader arbitration cannot bypass qualification via an accepted IRQ or JAM; live input/history divergence remains legal.
- checked: Explicit actual retro_unserialize/runahead candidate2 extension.
  found: Four production-created active-C6/idle-after-C7 held/released histories echo all saved bytes through the actual adapter, then compare committed/speculative output and absolute held1/released0 NMI markers at both scripts/depths1-3. The extended test passes in26.63s with24 additional cases;20 basecuts/60depth/120script/fourcontrols remain intact.
  implication: The new serialized semantic state is exercised directly through actual adapter APIs, not merely inferred from the old stage matrix. Runtime/module/runner identities and all campaign inputs remained unchanged during this test-only addition.

## Resolution

root_cause: Raw NMI assertions sampled after vector choice were stored as already recognized sticky requests, so releasing an inhibited pulse before the first handler T1 could not cancel it.
fix: Use value2 in the existing pending byte for an inhibited candidate, cancel release and qualify a held input before the first handler poll; retain recognized history1 and raw close samples. Enforce corresponding visitor range/history constraints without adding bytes. Reject CR01 proposed IRQ queue behavior.
oracle_type: derived from primary Visual6502 node/state analysis and specified existing phase crosswalk; exact physical ties remain unmeasured
verification:
  target_test: { result: pass, suite: core.interrupt_entry before-fix-red and final-green }
  mutation_check: { result: skipped, reason_if_skipped: No Stryker configured for owned C; exact CR02 source revert and proposed CR01 mutant exercised }
  no_op_deletion: { result: pass, deletion_justified_by_rca: true, detail: Replace unconditional sticky promotion with source-qualified candidate recognition; preserve raw sampling and recognized history }
  adjacent_tests: { result: pass, suites_run: 19-test native/adapter/Phase20/CPU/bus/state/campaign selection, log: build/entry-review-evidence/final-focused.log }
  revert_and_reconfirm: { result: pass, bug_returned_on_revert: true, fixed_on_reapply: true }
  guardrail_verdict: accepted
files_changed: [src/bus.c, src/cpu.c, src/internal.h, src/state.c, tests/core/test_interrupt_entry.c, tests/core/test_interrupt_state.c, tests/core/test_state_reject.c, tests/libretro/libretro_host.c, README.md, include/nesturbator.h]
