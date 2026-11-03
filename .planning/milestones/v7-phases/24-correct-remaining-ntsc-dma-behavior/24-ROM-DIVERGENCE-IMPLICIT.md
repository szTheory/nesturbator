# Phase 24 implicit DMA ROM divergence

## Scope

Read-only diagnosis of the new full-campaign result reported during the
authorized local CI. The campaign completed all 22 pages without timeout; the
`Implicit DMA Abort` row now reports raw `0x12`, with the first three test
sections passing and section 4 (`TEST_ImplicitDMAAbort_Loop3`) failing. No ROM,
test, workflow, or emulator was run for this analysis, and no emulator source
was changed.

This is a compatibility regression in the looped reload timing sweep. The
pinned assembly explicitly says Loop3 does not exercise an implicit abort; it
tests whether a looping sample can DMA too soon after a previous DMC GET. Keep
this separate from the one-byte non-looping abort and write-collision cases in
Loop1 and Loop2. Do not change the result classifier, either accepted row
outcome, or the already-corrected shared abort behavior based on this row
alone.

## What the sources establish

- The pinned Loop3 runs 16 timing offsets, `$00` through `$0F`, with the DMC
  looping, then checks `$540..$54F` against its per-profile duration key. The
  early profile expects no transfer for offsets `0..9`, then four-cycle
  transfers for `10..15`. The alternate accepted profile expects no transfer
  for `0..7`, then four-cycle transfers for `8..15`. Its comment states the
  intended condition: a DMA cannot occur within two cycles of the previous
  DMC DMA. These are pinned compatibility expectations, not fresh hardware
  measurements. See [the pinned assembly, Loop1 through the answer keys](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm#L13011-L13278)
  and the [phase research contract](24-RESEARCH.md).
- The owned `run_implicit_abort_probe` covers one-byte non-looping samples at
  counters 4/5/6, both APU half phases, a write control, a looping one-byte
  control, and a 17-byte sample. The looped control starts with looping already
  enabled at one fixed counter; it checks that two sample GETs and two normal
  halts occur, but does not assert the second halt phase/cycle or the spacing
  from the preceding GET. It does not reproduce Loop3's 16 offsets or
  disable/re-enable sequence. See `tests/cpu/test_bus.c` around
  `run_implicit_abort_probe` and `test_dma_implicit_abort`.
- `src/apu.c` creates ordinary reload requests on the output-unit edge and
  chooses a PUT halt (`dma_halt_phase = 0`) unless `dma_load_waiting` marks the
  first read after re-enabling with a full sample buffer; that first read uses
  GET (`dma_halt_phase = 1`). `src/bus.c` clocks the APU, then checks and may
  accept the resulting `dma_pending` request during that same CPU read cycle.
  The acceptance predicate checks pending, enable delay, phase, remaining
  length, and cartridge availability. It has no explicit minimum interval
  from the preceding sample GET.
- `dmc_sample_get` clears the fulfilled pending request and updates the reader
  count; a looping reader reloads its length. It does not record the actual
  GET cycle for comparison with a later request. The existing `abort_delay`
  represents a one-shot aborted halt and should not be reused as a general
  loop-reload cooldown.

## Leading hypothesis, not yet a proven first divergence

The current bus ordering can accept a request on the same CPU read that clocks
the output edge which created it, when the request's target phase matches.
That is a plausible explanation for the Loop3 failure: a looped reader may
request another byte on the first output edge after the prior sample GET, and
the bus path may begin that ordinary reload before the pinned sweep permits
it. The source-selected distinction between a pending ordinary PUT reload and
a deferred fresh GET load also matters: a cooldown must not silently convert
one request kind into the other.

The available evidence does not yet identify the failing `X` index, its
request-creation cycle, or whether the first divergence is same-cycle
acceptance, a request created while a load is already in flight, or an
incorrect GET-versus-PUT target after disable/re-enable. Raw `0x12` names the
ROM section, not that cycle-level cause. Keep these as candidates until an
owned trace establishes the first mismatch.

## Definite owned regression to add after the active CI run

Extend `tests/cpu/test_bus.c` with a separate loop-reload sweep, for example
`test_dma_implicit_loop_reload_sweep`, and include it in the existing default
`bus.unit` path and focused abort trace. Build each case from a fresh powered
instance and drive the real DMC register writes and CPU bus reads through the
existing observation seam. Reproduce the pinned disable/re-enable and timing
offset schedule for all 16 `X` values; do not inject `dma_pending`, GET/PUT
phase, timer count, buffer state, or sample-read events.

For each offset, capture the last actual `NESTURBATOR_DMA_SAMPLE_GET`, every
APU output edge that changes `dma_pending`, the request's `dma_halt_phase` and
`dma_load_waiting`, every accepted halt, and the next actual sample GET. Assert
the event ordering and minimum two-CPU-cycle exclusion from the preceding GET
that the pinned key requires, plus the correct target phase: ordinary looping
reload remains PUT; only the deferred first load after re-enable with a full
buffer is GET. Assert the selected early-profile 16-byte duration key and keep
the second accepted ROM classification unchanged.

The test should first fail against the current immediate-accept path at the
same named offset as its event trace, then pass after the smallest source-backed
fix. Retain the existing non-looping one-byte, write-suppression, 17-byte, and
counter-neighbor contracts. Add a named wrong-model control that accepts a
new output-edge request before the previous-GET exclusion expires, so the
regression itself proves that it catches the hypothesized early acceptance.

Only if the trace shows that the exclusion must survive a public save/load
boundary should the implementation add saved timing history. If needed, keep
that history distinct from `abort_delay` and `dma_load_waiting`, audit state
serialization/versioning, and add the corresponding continuation test. Do not
add serialized state or claim a particular revision outcome before the trace
proves the need.

## Evidence boundary

The ROM result establishes that current owned checks miss a Loop3 timing
contract; it does not prove the early-accept hypothesis or a universal silicon
rule. The controlled fixture should establish whether the pending request is
created too early, accepted too early, or assigned the wrong phase before any
production change. Root owns implementation after the authorized full CI
finishes. The active session checks and completed campaign must remain
attributable to that invocation; this note authorizes no rerun or other runtime
work.

## Resolution from the completed owned sweep

After correcting two emitter-only instruction-order mistakes, the first two
implicit keys matched and looping offsets8/9 reproduced the actual failure:
4 instead of0. The compatibility model now retains a just-fetched byte across
the immediately following output edge; readiness is local to the synchronous
bus call, including OAM's final GET. No persistent cooldown/state field is
added. This is inferred byte-visibility ordering, not a universal general DMA
spacing claim. All48 implicit entries now match the declared early-G keys.
A forced-immediate visibility model reproduces the wrong offsets, and the
focused DAC/buffer fixture plus real public native continuation cover the
observable audio/state consequence. See24-04-CORRECTIONS.md for full evidence.
