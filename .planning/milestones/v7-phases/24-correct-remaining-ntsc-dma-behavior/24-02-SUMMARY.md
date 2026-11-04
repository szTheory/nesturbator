---
phase: 24-correct-remaining-ntsc-dma-behavior
plan: "02"
subsystem: dma-bus-state
tags: [ntsc, dmc, oam, bus-conflict, native-state]
requires:
  - phase: 24-01
    provides: Silent-phase correction and five independent divergence records
provides:
  - OAM overlap with fixed interior/final event and timing guards
  - Split internal-register/external-sample effects and independent latch history
  - Format-5 native continuation and old-format atomic refusal
  - Independent reverted-model rejection for each corrected row
key-files:
  created:
    - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-02-SUMMARY.md
  modified:
    - src/bus.c
    - src/internal.h
    - src/state.c
    - tests/cpu/test_bus.c
    - tests/core/test_state_fork.c
    - README.md
    - include/nesturbator.h
key-decisions:
  - OAM uses synchronous local stages without new saved state; behavior11 preserves format4.
  - Owner separately approved internal cpu_data/external open_bus, format5 and behavior12 before mutation.
  - Native public frame continuation and private immediate-read sensitivity are distinct evidence.
requirements-completed: []
requirements-pending: [DMA-01, DMA-02, DMA-03, DMA-04]
completed: 2026-10-31
status: complete
---

# Phase 24 Plan 02 — collision and split bus correction

DMC GET now steals only the colliding OAM source slot, and internal register effects retain separate CPU and external data histories. Both row corrections have independent event fixtures and native continuation. This completes the bounded implementation plan, not phase acceptance.

## Coherent task commits

- `8828225`: Task1 and collision-specific Task3; behavior11 / format4, owned event/timing/data controls, native continuation and required current-version consumers.
- `418da27`: OAM evidence and partial/checkpoint handoff; `e08c999`: review whitespace cleanup.
- `d4a24c8`: Task2 and latch-specific Task3; owner-approved format5 / behavior12 / ABI1, serializer, atomic old-format refusal, native latch tests, README/header/API/progress/reproduction documentation and coupled version/hash consumers.

The [amendment](24-02-AMENDMENT.md), [approved field decision](24-02-CHECKPOINT.md), [historical partial outcome](24-02-PARTIAL.md), [review](24-02-REVIEW.md) and [DMA evidence](24-DMA-EVIDENCE.md) preserve the exact rationale and bounds.

## Behavior and native state

OAM halt/dummy/alignment overlaps DMC. Only sample GET steals a source; a following idle PUT aligns the retry. Fixed matrices pin all256 destination bytes exactly once, source/resume indices and phases, original CPU-read resumption and +2 interior/+1 or +3 tail duration. Local synchronous stages add no saved state. The local stress gap G=6 preserves all existing short-run OAM/DMC/IRQ straddles without extending bounds.

DMC sample GET uses the mapper's external address/data. Internal select combines parked core A15:5 with sample A4:0. The owned matrix checks both controllers, $4015 IRQ acknowledgement and its next-GET clear timing, load/reload phases, neighboring lows and the $4020 parked boundary. Unconnected internal reads retain CPU data. $4015 does not replace external held data; controller reads merge its upper bits. This retains the existing per-cycle controller-clock profile. Pinned assembly explicitly does not qualify DMC shift-register input; that existing external-byte model is retained without a hardware claim.

BUS serialization grows2059->2060 bytes by adding cpu_data after external open_bus. C struct2061->2062 includes two unsaved host input bytes; it is not the payload size. Internal byte bounds span0..255. Formats1..4 are refused without conversion, ABI remains1. The six-board sizes each grow one byte. Removing the added byte and restoring format/revision/length/checksum produces each exact prior native hash; all12 normalized payloads and frame ticks match. Non-state hash rows and key inventory stay unchanged. Historical revision9 AccuracyCoin metadata is untouched.

The collision native cut at frame45 and silent/pending cuts at1/17 compare full pixels, PCM, RAM effects, metadata, field-wise transition events and canonical final bytes. The new owned status-reading guest reaches frame1 with internal10/external40, saves publicly, restores poisoned latches, roundtrips canonically, refuses format4 atomically, and matches the uninterrupted next frame's full outputs/state. Immediate status/controller read sensitivity is measured on disposable private-bus copies of that real boundary; each missing-latch control changes its independent expected result. The next ordinary opcode fetch can converge latches, so these immediate stimuli are expressly separate from guest frame continuation. No public mid-DMA capability or phase history is manufactured.

## Verification

Selected CTest JSON closures contain no campaign fixture. The final correction passes:

- Builds for bus.unit, core.apu/api/mapper, C/C++ headers, core.state_fork/reject and the existing state-hash runner/derived fixture targets.
- bus.unit, core.apu/api/mapper, header.c/cxx, state.format_pin, vectors.api_policy and its mutation self-test.
- docs.reproduction and its mutation self-test under existing60-second bounds.
- core.state_fork --dma; retained --short --stress with committed nesteroids prefill, including all OAM/DMC/IRQ witnesses on NROM/MMC3.
- core.state_reject --short with committed dabg/nesteroids/MMC1/MMC3 fixtures, including every older/future version and atomic refusal controls.
- Independent pre-split bus source e08c999 compiled into BOTH linked and observed bus paths rejects the default new owned fixture with exit1: missing internal select, wrong CPU value, missing IRQ/controller effects and collapsed latches. Current source passes. The earlier independent pre-OAM source control rejects its own collision fixtures.
- clang-format18 dry run and whitespace checks. Normal commit hygiene hooks passed. Scoped code review has no active finding; the reviewer did not rerun tests.

Build-tree logs: phase24-conflicts.txt, phase24-collapsed-model.txt, phase24-split-state-stress.txt, phase24-split-state-reject.txt, phase24-split-hash-normalization.txt and phase24-split-state-<board>.txt. These are local diagnostic evidence, not required campaign results.

## Remaining work

Plan03 still owns explicit/implicit abort corrections. Any new saved abort field or meaning needs its own concrete decision before mutation. Plan04 full pinned/protected/determinism/performance acceptance is missing. The interrupted full-CI/AccuracyCoin stop remains: no workflow, AccuracyCoin ROM, campaign, push or PR ran. All phase requirements remain pending; bounded tests do not replace those gates.
