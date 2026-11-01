---
title: "Explicit DMA Abort ROM divergence diagnosis"
summary: "Read-only analysis of the pinned Explicit DMA Abort sweep, its hardware contract, and the missing owned trace needed to locate the first divergence."
read_when: "Before changing the explicit-abort fix in response to a fresh AccuracyCoin Explicit DMA Abort failure."
updated: 2026-10-31
---

# Explicit DMA Abort ROM divergence

## Finding and confidence

The fresh campaign reported `Explicit DMA Abort` as `FAIL 0x0A`, which follows the pinned ROM's second subtest: its 16 measured durations did not match the answer key. The preceding `CheckDMATiming` subtest expects four cycles and passed. The report has no per-index `$50..$5F` values, so it does not say which offset first differs or whether the cause is the abort halt, request cancellation, write deferral, or the restart measurement. Treat this as a request for a discriminating trace, not evidence that one existing field is wrong.

The strongest coverage gap is the interaction between a reload request that is already pending, a CPU write that defers its halt, and `$4015` disable/re-enable. The current owned explicit-abort probe waits for `dma_pending == 0`, then places `$4015=0` two or three clocks before the output boundary; it tests a later write on the predicted abort cycle. That validates the documented early window, but it does not validate the ROM's full sweep through a request that has already been raised. This is a hypothesis until the reconstructed 16-entry fixture identifies the first differing event.

## What the pinned sweep measures

`TEST_ExplicitDMAAbort` first checks sync timing (`CheckDMATiming`, expected 4), then repeats for `X = 0..15`:

1. `DMASync_50CyclesRemaining` establishes a looped one-byte sample source. The test changes `$4010` to `$4E` (looping, DMC rate index `$E`, 72 CPU cycles per output bit), so one eight-bit output cycle spans 576 CPU cycles.
2. Its clockslides and `Clockslide64_Minus_A` place `$4015=0` around the next normal reload event. From the source comments, after the clockslide returns there are `A` cycles to the target; `NOP`, `LDA #0`, and the four-cycle `STA $4015` place the register write at nominal offset `8 - A` from that target. Thus the sweep spans about eight cycles after through seven cycles before. The CPU guest must log actual write and halt cycles to settle the bus-cycle alignment at each end.
3. It resynchronizes, writes `$4015=$10`, and calls `CalculateDMADuration`. That routine polls `$4000` and counts clockslide intervals until a DMC GET changes open bus; the result is stored at `$50 + X`.
4. It compares all 16 values with:

   ```text
   X:    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
   key:  4  4  4  4  4  4  3  4  1  1  0  0  0  0  0  0
   ```

This is an end-to-end timing oracle after a disable/re-enable sequence. A final `0x0A` does not identify the first mismatching X. Exact `CalculateDMADuration` behavior and the observed `$50..$5F` array are needed before naming the failing offset.

## Reference contract

The pinned [NESdev DMA revision 23450, “Bugs”](https://www.nesdev.org/w/index.php?title=DMA&oldid=23450#Bugs) describes a stop in the APU cycle before a reload would schedule: two or three CPU clocks before the halt attempt, the DMA starts and aborts after one cycle. If a CPU write delays that halt, this aborted DMA does not occur. The normal reload path targets a PUT; ordinary failed RDY halts retry on the next CPU cycle. This separates a one-shot abort from a normal sample-read request and its write-retry behavior.

The pinned [APU DMC reference, revision 24164](https://www.nesdev.org/w/index.php?title=APU_DMC&oldid=24164) describes the sample buffer and output unit separately: output bits finish their eight-clock cycle; the sample buffer is moved into the shift register at a boundary; an emptied buffer with bytes remaining causes the memory reader to fetch; clearing D4 in `$4015` sets bytes remaining to zero; setting it restarts only when the reader is stopped, and already-buffered bits finish first. This supports canceling the old reader request while preserving any separately qualified aborted halt. It does not justify a sample GET merely because `dma_pending` remains nonzero after disable.

## Current code path to observe

- `src/apu.c` advances the APU before decoding a bus write. On an output edge it may empty the sample buffer and set `dma_pending`; the same `$4015=0` write then calls the stop-window helper, sets `remaining=0`, clears `dma_pending`, and clears the enable/load-waiting state.
- `src/bus.c` also examines an existing pending request after advancing that write cycle. If its target phase matches, it records the failed halt and toggles `dma_halt_phase` before `$4015` register decode cancels the request. Therefore a request created on this write's APU edge and an older request deferred by an earlier write may take different paths even though both are cleared by the register handler.
- The read-side DMC transfer requires `remaining != 0`; retaining only a stale pending bit after disable cannot fetch a sample. The already-reviewed “leave pending” model is therefore not an explanation by itself.
- `nesturbator__dmc_stop_window` currently recognizes `bits == 0`, a full sample buffer, and a timer-derived two/three-clock window. The bus read path consumes the one-shot `abort_delay` on the due read; a write advances APU time and lets the countdown expire without ordinary retry. That matches the reference rule for the tested early window, but the current probe does not establish how this history composes with a request already pending or its failed write attempt.

## Minimal owned fixture to locate the first divergence

Add a source-shaped CPU guest fixture alongside the existing `test9_emit_dmasync` helpers; do not open or execute another emulator. Drive all APU and bus state through real guest writes and bus calls. Do not inject `bits`, timer, phase, `dma_pending`, or abort state.

1. Recreate the ROM's known sync precondition and looped one-byte source, including `$4010=$4E`, `$4012=$FF`, `$4013=0`, and the `$4000` open-bus polling used by `CalculateDMADuration`. Arrange the test-owned PRG sample byte and parked reads to make each actual GET observable.
2. Reproduce the documented clockslide counts and sweep all 16 values of X. Preserve the re-enable and duration-counting suffix, rather than reducing the fixture to one stop phase. For each X, emit a compact expected-versus-observed `$50+X` result and stop at the first key mismatch.
3. Record the first causal event with CPU cycle and GET/PUT phase: output edge/buffer transfer, request creation, halt attempt/acceptance, write retry, `$4015` register write completion, request/remaining/buffer/timer state, any one-cycle abort repeat, actual sample GET/address/data, re-enable state, and the first `$4000` read that detects the restarted transfer.
4. Include controls for a nearby non-abort stop, the documented two/three-clock abort window, a write on the candidate halt cycle (which suppresses the aborted cycle), and a normal non-aborted reload. Assert that disabling prevents an obsolete reader GET, buffered bits retain their documented behavior, and restart fetches at the address and time predicted by the full reference sequence.
5. Use wrong-model controls that independently keep an obsolete sample request alive after disable, erase a qualifying one-cycle abort, or retry an abort after a write. They should fail on an observed bus event or duration entry, not only a final status code.

If the ROM sweep first diverges at the same output edge that creates a request before `$4015` decode, compare the required halt attempt with the current same-cycle request path. If it first diverges after a write retry, isolate that delayed-request case. If per-offset cycle/access traces match and only the restart duration differs, investigate the re-enable/clockslide model instead of changing abort qualification. The failure record does not currently choose among these branches.

Do not add another saved field based on the aggregate ROM status. The current `abort_delay` already represents the observed one-shot history. Change its qualification or the existing pending/retry ordering only after the full sweep names a first divergence. Add state only if the production trace proves a distinct history cannot be reconstructed from current state; if needed, include production-reachable save/load continuation and the documented format/refusal consequences.

## Evidence limits

This diagnosis uses the pinned AccuracyCoin assembly and repository source, plus the project-indexed hardware references. It ran no tests, ROMs, or campaign and makes no new silicon or chip-revision claim. The campaign's answer-key failure is fresh compatibility evidence; only the owned all-offset event trace can identify the core's first causal divergence.

## Resolution after source-faithful guest tracing

See24-04-CORRECTIONS.md. Offset6 was an already armed request; offset7 stops
one clock before the output edge with no pending flag yet. The latter narrow
normal-reload boundary is qualified before reader clear. Both now complete
ordinary GETs without remaining underflow/restart. Every explicit16-entry key
matches. Old no-GET expectations at these two stops are superseded; the one-
cycle abort, write suppression and implicit early-G guards remain intact.
