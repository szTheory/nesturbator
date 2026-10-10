# Phase 04: Sound - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-10-08
**Phase:** 04-Sound
**Areas discussed:** Phase boundary, audio output contract, synthesis target, automated acceptance

---

## Phase boundary, audio output contract, synthesis target, automated acceptance

| Option | Description | Selected |
|--------|-------------|----------|
| Carry forward documented project decisions | Keep NTSC/mapper-0 scope, mono 16-bit 48 kHz PCM, integer deterministic synthesis, both hash layers, licensed ROM fixtures, and automated CI evidence as already specified. | ✓ |
| Reopen product and API choices | Change the output format, audio-device ownership, dependencies, or human verification expectations despite existing requirements and project decisions. | |

**User's choice:** Applied the owner's standing instruction to accept the documented recommendations and automate recurring verification.
**Notes:** The roadmap, requirements, preparation references, and prior phase decisions already settle all user-facing choices relevant to this phase. No unresolved gray area required a question. Internal timing, data structures, and test construction remain for phase research and planning.

---

## the agent's Discretion

- APU module boundaries, instance state layout, event scheduling, and detailed test implementation, constrained by the canonical hardware references and fixed output contract.
- Reproducible generation and storage details for the integer mixer and synthesis tables, using the smallest owned approach that fits existing CMake/CTest patterns.

## Deferred Ideas

None.
