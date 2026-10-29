---
phase: 21-interrupt-latency-and-hijack
reviewed: 2026-10-29T16:10:21Z
depth: standard
files_reviewed: 44
files_reviewed_list:
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
  - tests/runahead_model.h
  - tests/runner/hashes.txt
  - tests/state_test.h
findings:
  critical: 0
  warning: 0
  info: 0
  total: 0
status: clean
---

# Phase 21: Code Review Report

**Reviewed:** 2026-10-29T16:10:21Z
**Depth:** standard
**Files Reviewed:** 44

## Summary

Independently re-reviewed all 12 changed code, test, and documentation files in the original 44-file scope, plus the stated disposition and state matrix. The prior CR-01 is rejected by primary interrupt-arbitration evidence. CR-02 was valid and is fixed in source commit `43d260a`. The frontmatter counts active findings; none remain. Post-review local qualification passed full CI 695/695, ASAN 686/686, vectors-full 258/258, and MMC3 13/13; the fresh 114-key authority and all eleven targets also passed on the current source. Hosted Plan 21-09 delivery remains pending, so this does not claim phase or requirement completion.

## Narrative Findings (AI reviewer)

No active issues found in the reviewed working candidate after these dispositions.

### CR-01: NMI priority entry discards an already accepted IRQ (BLOCKER — REJECTED)

**File:** src/cpu.c:465
**Issue:** The prior report treated clearing irq_accepted when NMI wins dispatch as dropping an independently queued IRQ. Primary circuit evidence instead describes IRQ/NMI/reset as one forced-BRK entry with shared request bookkeeping cleared after vector handling. The IRQ is level-sensitive; after an NMI-priority entry, the still-asserted source can be accepted after RTI restores I. If the NMI handler acknowledges the source, no IRQ entry should follow.
**Fix:** No code change is warranted. Keep the current one-event priority arbitration and clear irq_accepted when NMI entry is selected. Do not retain it as an independent queue.
**Disposition:** Rejected. The [Visual6502 State Machine analysis](https://www.nesdev.org/wiki/Visual6502wiki/6502_State_Machine) describes the shared forced-BRK entry and request-flop clearing after VEC1. The [Visual6502 Interrupt Hijacking analysis](https://www.nesdev.org/wiki/Visual6502wiki/6502_Interrupt_Hijacking) says an IRQ hijacked by NMI gets attention after NMI RTI. The new production-path cases at tests/core/test_interrupt_entry.c:942-983 cover acknowledgment and held-source behavior; the recorded preservation mutant takes IRQ before the first NMI-handler opcode. Ordinary accepted-IRQ behavior before arbitration remains intact.

### CR-02: Post-choice NMI pulses were retained without recognition (BLOCKER — FIXED AND LOCALLY QUALIFIED)

**File:** src/bus.c:35-49, 66-100
**Issue:** Before this fix, every sampled low-to-high NMI transition became sticky recognized history, including C5-C7 pulses after vector selection. A pulse released before the next T1 was consequently serviced after the lower-priority handler instruction, contrary to the documented recognition-inhibition window.
**Fix:** The current code records post-choice input as NESTURBATOR_NMI_INHIBITED, cancels it on release, and promotes it only when still held at the first handler opening before the normal poll. Recognized requests remain sticky; src/cpu.c:528-535 permits only recognized history to hijack the vector.
**Disposition:** Fixed in source commit `43d260a` and locally qualified. The [Visual6502 Interrupt Hijacking analysis](https://www.nesdev.org/wiki/Visual6502wiki/6502_Interrupt_Hijacking) describes stage-1 inhibition after vector choice: a pulse released before T1 can be missed, while a sufficiently held request is recognized after the first lower-priority handler instruction. The C5/C6 tests have a complete later entry cycle before T1; C7 release is explicitly only a deterministic integer-model case because the physical tie is unmeasured.

State validation at src/state.c:108-143 bounds candidate history to reachable active C6/C7 or idle-after-C7 states and rejects inconsistent sampled-line, poll, accepted-IRQ, JAM, and vector-stage combinations before APPLY. Production tests at tests/core/test_interrupt_entry.c:987-1115 cover released and held sampled pulses across BRK, IRQ, and NMI origins, the first-handler branch poll, and consumed held input. tests/core/test_interrupt_state.c and tests/core/test_state_reject.c exercise candidate serialization, release, and CRC-correct invalid neighbors. The candidate creator at tests/libretro/libretro_host.c:2952-3015 feeds the actual-adapter replay at lines 2824-2850, which checks retro_unserialize/serialize byte identity and continuation. The disposition records the before-fix reproduction, exact revert/reapply control, and focused/campaign results. Local qualification is recorded in Plan 21-08; hosted evidence remains pending.

---

_Reviewed: 2026-10-29T16:10:21Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
