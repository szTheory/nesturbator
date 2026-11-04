---
phase: 24-correct-remaining-ntsc-dma-behavior
plan: "03"
subsystem: dma-abort-state
tags: [ntsc, dmc, abort, native-state, clean-room]
requires:
  - phase: 24-02
    provides: Verified collision and split internal/external bus correction
provides:
  - Independent explicit/implicit one-cycle abort regressions and write-suppression controls
  - Instance-local expiring abort countdown with format-6 native continuation
  - Real public-frame cuts for every positive countdown value
  - Reverted no-history and wrong write-retry model rejection
key-files:
  created:
    - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-03-AMENDMENT.md
    - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-03-REVIEW.md
  modified:
    - src/apu.c
    - src/bus.c
    - src/internal.h
    - src/state.c
    - tests/cpu/test_bus.c
    - tests/core/test_state_fork.c
    - tests/core/test_state_reject.c
    - README.md
    - include/nesturbator.h
key-decisions:
  - Owner explicitly approved abort_delay0..3, format6, behavior13 and refusal1..5 before mutation.
  - Explicit and implicit stop scheduling share only their independently evidenced two/three-clock window.
  - Keep the selected early-G model and both pinned implicit acceptance outcomes; no late-G/H unexpected reload.
requirements-completed: []
requirements-pending: [DMA-01, DMA-02, DMA-03, DMA-04]
completed: 2026-10-31
status: complete
---

# Plan24-03 — one-cycle abort and native continuation

Explicit disable and non-looping last-byte completion now preserve the qualified one-cycle aborted halt. A write suppresses that halt rather than deferring it as an ordinary sample request. The approved countdown survives genuine public frame-return/native save/load boundaries. This completes the bounded correction plan; phase acceptance remains incomplete.

## Coherent correction

Commit7f31579 combines behavior Tasks1/2 with native Task3 and all necessary tests, README/header/API/progress/reproduction documentation, visitor validation and current-version/hash consumers. [Approved checkpoint](24-03-CHECKPOINT.md), [ownership amendment](24-03-AMENDMENT.md), [diagnostic review](24-03-DIAGNOSTIC-REVIEW.md), [final code/consumer review](24-03-REVIEW.md) and the [DMA ledger](24-DMA-EVIDENCE.md) retain exact evidence and limits. No further saved distinction was needed.

The shared helper records an abort due in2/3 future CPU clocks only when stopping immediately before the full sample buffer drains. The bus captures a due countdown before its APU clock expires it; the parked read occurs on the halt and again on resumption, with no mapper sample GET. Writes and synchronous OAM expire it without a retry or stolen sample/source slot. A concurrently eligible normal request still gets its first eligible read, including an abort's resumed read. A canceled unaccepted normal request no longer leaves stale pending bookkeeping; a later enable starts a fresh load. No CPU write can occur inside an already accepted synchronous transfer. The old claim that disabled pending requests still fetched was unsupported by its cited source and contradicted the old bus's remaining==0 guard; no old successful post-disable GET was removed.

The selected early RP2A03G model does not add the separate later-G/H unexpected reload. Both historical implicit results accepted by the pinned ROM remain accepted. No classifier, scoreboard, ROM pin, performance allowance or limit was changed.

## Independent oracles and wrong models

The running control fixes the explicit normal halt at434 PUT and sample437 GET. Stops431/432 are the three/two-clock window:11 total clocks for10 ordinary accesses, one halt434 and resumed read435, no sample. Earlier429/430 and later433/434 are no-abort controls. A write434 suppresses the one-cycle halt and preserves RAM. The earlier guessed436 explicit oracle was withdrawn before implementation; its correction is visible in the ledger.

The implicit setup gives sample575 GET/dataEA at both enable halves, then one halt578 PUT/resume579 GET. The actual write578 event and RAM effect are pinned at both halves. Looping and length17 controls retain two real samples; neighboring counter4/6 cases preserve the selected early-G outcome. Normal and observed production paths compare return values and whole final machine state.

Further controls assert repeated side effects at both controller ports and status/IRQ, retained internal/external data, no normal write retry for an abort, all256 OAM bytes with unchanged515-clock continuation, re-enable during a countdown, and real pending load/reload cancellation followed by a fresh enable. These are bounded model guards, not new silicon measurements.

Wrong model1: compile BOTH linked/observed bus and APU sources from aa78a23 with current tests/declarations. This preserves preceding silence/OAM/split-bus corrections but omits stop history; the explicit/implicit contracts independently fail with missing halt/extra-clock assertions and stale pending. Wrong model2: compile BOTH current bus copies with due writes retaining abort_delay1 for the next clock. It incorrectly retries an aborted halt and fails the write-suppression cases. Both exit1; restored production passes. Logs are phase24-no-stop-history-model.txt and phase24-retry-aborted-halt-model.txt in build/phase24-owned.

## Native state

DMC appends abort_delay at serialized local23, bound0..3: payload23->24. Its C size remains24 by using trailing padding. Following serialized APU fields move one byte; existing meanings and BUS2060 are unchanged. Format6 refuses formats1..5 without conversion, behavior13 records output/timing changes, ABI stays1. Every six-board state grows1 byte. Deleting file byte2236 and restoring format5/revision12/length/CRC reproduces all12 exact prior native hashes; ticks and every non-state row/key remain unchanged. No historical revision9 campaign record was relabeled.

Owned guest instructions, initial/later rate writes and precisely counted NOP/BIT padding produce public frame1 returns with explicit countdown2 at cycle29782, explicit countdown3 at29781 and implicit countdown1 at29781. No timer/phase/bit/request/CPU state is injected. Reference A never loads; B restores and produces identical full pixels, PCM, metadata, RAM effects, field-wise transition cycle/level events and canonical final bytes. Each continuation has nonzero transitions. Current-format roundtrip preserves the field; old format5 and out-of-range4 refuse atomically. Removing valid-valued countdown history changes next-frame timing/final state. Disposable copies also execute the actual next guest INC instruction:6 clocks with the halted read versus5 without it, with the expected PC/RAM effect. This private instruction audit is separate from the uninterrupted public-frame comparison; no public mid-DMA API is invented.

All prior silent/pending/collision/latch native cuts and retained NROM/MMC3 short-stress OAM/DMC/IRQ witnesses still pass.

## Verification and limits

Read-only CTest inventories show no campaign fixture in the selected closures. Fresh bounded builds and checks pass:

- bus.unit/core.apu/core.api/core.mapper, C/C++ header checks, state.format_pin, vectors.api_policy and its mutation self-test.
- docs.reproduction and mutation self-test within existing60-second limits.
- core.state_fork --dma, --short --stress with committed nesteroids prefill, and core.state_reject --short on dabg/nesteroids/MMC1/MMC3. The new abort_delay upper-bound row loads and continues; invalid and all older/future versions refuse atomically.
- Final Plan04-owned controller/entry check: bus.unit, core.apu, core.controller, core.state_fork.entry and --dma.
- Production preprocessing erases every DMA observer marker. clang-format18 and the execution delta's whitespace check pass. Normal commit hygiene hooks ran. Reviews have no active production or consumer finding.

The required literal git diff --check main...HEAD exposes159 inherited planning whitespace diagnostics in50 files. The same check already failed before execution at89a042b; the correction delta89a042b..HEAD is clean. Local main is older than the recorded remote main ref. These inherited planning/patch-file diagnostics are not silently waived or bulk-edited in this correction; the branch-wide guard remains unresolved for acceptance.

No full workflow, AccuracyCoin ROM, campaign, push or PR ran. build/ci still has FULL=ON. Full pinned/protected observations, cross-platform output equality and unchanged+1% Cachegrind acceptance remain required in Plan04 after separate matching runtime authority. All four phase requirements remain pending.
