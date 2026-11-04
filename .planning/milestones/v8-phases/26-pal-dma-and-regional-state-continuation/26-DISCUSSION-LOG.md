# Phase 26: PAL DMA and regional state continuation - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in 26-CONTEXT.md; this log preserves the alternatives considered.

**Date:** 2026-11-01
**Phase:** 26-PAL DMA and regional state continuation
**Areas discussed:** PAL DMC DMA fixture, Regional state continuation

---

## PAL DMC DMA fixture

| Option | Description | Selected |
|--------|-------------|----------|
| Bus-cycle oracle | Generated fixture distinguishes opcode-fetch/SYNC halt from any-read halt; does not claim later no-op-cycle bus behavior. | ✓ |
| Guest-visible side effect | Controller or PPU-register side effect exposes an extra read but may depend on unresolved no-op-cycle bus behavior. | |

**User's choice:** The owner asked to follow all recommendations. Adopt the bus-cycle oracle and its same-timeline any-read wrong-model control.
**Notes:** Use the production CPU/APU/bus path. Include a request during an operand/data read that waits until the next opcode fetch, plus an observable opcode-fetch halt. Label the model as source-backed; do not claim direct hardware proof or assert the later no-op-cycle bus address. The 2025 hardware-trace report supports the observed edge and SYNC interpretation, while the curated DMA reference preserves the later-cycle uncertainty.

---

## Regional state continuation

| Option | Description | Selected |
|--------|-------------|----------|
| Keep format 6 | Reuse the existing region byte for NTSC 0 / PAL 1 and preserve existing NTSC format-6 saves, provided serialized fields keep their meanings. | ✓ |
| Bump to format 7 | Reject all existing format-6 states, including NTSC states. | |
| Write format 7 and keep a format-6 NTSC reader | Preserve NTSC states with a dual-reader compatibility path. | |

**User's choice:** The owner asked to follow all recommendations. Keep format 6 because the region, sample-phase, PPU-position, and DMA fields already exist with unchanged meanings; use regional validation bounds. Advance behavior revision 16 to 17 and retain ABI 1.
**Notes:** The current validator uses NTSC bounds for audio_rem, APU sample_phase, PPU scanline, and the PPU-to-header tick gap. PAL-specific bounds must accept sample phases below 709379, scanlines through 311, and gaps below a PAL frame. Cross-region state refusal remains an identity error before apply and leaves the instance unchanged. The behavior revision is recorded but is not a loader gate.

---

## the agent's Discretion

Choose the smallest internal opcode-fetch/SYNC marker that works for the production bus and fixture; reuse existing DMA trace and state-fork tests. Do not add a public API, runtime option, dependency, or unrelated hardware assertions.

## Deferred Ideas

- Direct 2A07 hardware proof and the later no-op-cycle bus address remain unresolved.
- General property tests, mutation testing, and runner signal determinism remain outside v8.
- PAL presentation, palette completion, and final NTSC acceptance remain in Phase 27.
