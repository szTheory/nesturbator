# Phase 24 DMA evidence — incomplete

**Status:** Partial execution evidence only. This file does not close Plan 24-01 or establish phase acceptance. No DMA behavior correction has been made.

## Source identities and limits

- Core baseline before these edits: commit `89a042ba742ef24679666deac8606ef8e54e67a4` (`docs(state): record phase 24 planning complete`).
- Pinned test source: AccuracyCoin `6bee919529289c555803b612b8c9e4f7eaaeeabb`; `AccuracyCoin.asm` SHA256 `540b524187a97d71018bb94a1b7874e19d37894e5882861fe0c31adafe251a9f`.
- Hardware notes used: `NES-HARDWARE-CPU-APU.md` §2.2, which summarizes RDY, GET/PUT, OAM overlap, register repeats, and internal/external bus selection. These notes are not a new silicon measurement.
- No AccuracyCoin ROM, full workflow, campaign, push, or PR ran. Bounded C fixtures are compatibility/model evidence only.

## Row 1 — DMA Landing on Write

**Historical status:** `FAIL 0x26` in the checked-in scoreboard; not freshly run.

**Pinned failure-code decode:** The pinned source encodes `TEST_Fail` as `(ErrorCode << 2) | 2`; raw `0x26` therefore identifies error code 9. The Test 9 assertion sequence places that error code at the final `LDA $4000; BNE FAIL` check. This decodes the historical scoreboard entry; it is not a fresh ROM result or event trace.

**Pinned expectation:** AccuracyCoin Test 3 (assembly near lines 3160–3174) expects a one-write reload collision to retry to a GET halt and complete `[GET halt, PUT, GET]` in three DMA cycles. Test 5 (near lines 3182–3194) expects two consecutive blocked attempts followed by a four-cycle PUT-start transfer. The test also covers load timing, `$4015` effects, and resume behavior later in the row.

**Observed core trace:** The owned fixture includes production `src/bus.c` with compile-time-only event hooks and also runs the normal linked entrypoints for parity. For the two source-shaped reload setups above, it observes request creation during APU clock, each write completion and retry, the first accepted read halt, the actual `$C000/$A7` sample GET on GET, and the resumed `$0020/$5A` CPU read. One write produces the three-cycle GET-start sequence; two writes produce the four-cycle PUT-start sequence. Fresh `$4015` load setup also exercises the production write and its 3/4-cycle enable delay. Normal and observed copies match in machine bytes, return value, and continuation.

**Corrected synthetic Test 9 trace:** Parent review found and corrected two addressing-mode errors in the diagnostic sync helper: the initial result load must be zero page; the final Copy_A load must be absolute. The earlier pre-write-halt attribution is withdrawn. With those fixes, `$4015` completes on GET at cycle 44217; `$5000` correctly delays the request at 44221; the next read halts on PUT at 44222 and sample GET occurs on GET at 44225. The reconstructed later reload is nevertheless early (request snapshot 44259; halt 44260) and the final `$4000` read at 44642 returns `$40`. The approximate Test 8 frame wait means this remains a limited reconstruction, not an exact pinned row execution.

**Independent output-unit divergence:** The owned `--dma-trace silence` probe uses production reset and CPU NOP execution, without DMC/phase injection. Six timer reload edges at snapshots 1, 429, 857, 1285, 1713 and 2141 all keep `bits=0` despite advancing output clocks. The APU reference's Output Unit contract requires continuous eight-clock sample-or-silence cycles with a separately latched silence flag. Current `src/apu.c` only starts/decrements bits when a sample is available. This loses silent-cycle phase and permits the newly loaded sample to drain at the next timer tick. This is a source-supported first output-unit divergence consistent with the early reload; exact landing-row attribution still needs the tightened fixture and required acceptance.

**State gate:** Correct output-cycle phase needs continuous `bits` plus silence state. The current DMC visitor saves bits but no silence flag. Changing that meaning and adding a boolean requires a format bump under D-05. Execution stopped before mutation; see [the concrete field/version decision](24-01-CHECKPOINT.md). No correction is shared with other rows.

**Wrong-model control:** The independent silent-clock diagnostic rejects the named `freeze-at-zero-while-empty` model; it intentionally exits 1 on the uncorrected implementation. Corrected behavior and native continuation are not implemented yet.

## Row 2 — DMC DMA + OAM DMA

**Historical status:** `FAIL 0x0a` in the checked-in scoreboard; not freshly run.

**Pinned expectation:** AccuracyCoin's row comments describe DMC/OAM halt-cycle overlap and say the in-transfer DMC operation appears to add two cycles; the answer table distinguishes phases and final positions. This is a test contract, not a silicon result.

**Observed core trace:** An owned NROM probe requested the DMC load at OAM source offsets 128 and 255. It recorded the source byte, GET sample phase, resumed source offset, and complete OAM duration. Both probes measured 514 CPU cycles without DMC and 517 with DMC (+3); each sample GET was on GET and OAM resumed at the same source offset.

**First divergence:** For these synthetic in-transfer positions, the first differing event is the DMC/OAM overlap duration: the core adds three cycles where the pinned row's stated overlap model adds two. The final-position answer-key profile mapping still needs independent localization.

**Wrong-model candidate:** A full standalone DMC sequence inside OAM, with no overlap of the two halt cycles. The duration probe distinguishes this candidate; it does not prove every answer-key position.

## Row 3 — DMC DMA Bus Conflicts

**Historical status:** `FAIL 0x0a` in the checked-in scoreboard; not freshly run.

**Pinned expectation:** AccuracyCoin explains that the external sample address's low five bits select an internal APU/controller register while the CPU is parked in `$4000–$401F`. Its assembly checks controller results for offsets `$16/$17` and that a `$4015` conflict clears frame IRQ. The test explicitly does not inspect the DMC shift-register input.

**Observed core trace:** With the CPU parked at `$4000`, an external GET at `$FF16` returning `$A0` left controller shift `$01` unchanged and the CPU saw `$A0`; the pinned internal `$4016` selection expects a controller bit merge, `$A1` for this fixture, and a controller clock. At `$FFD5/$A5`, the pinned internal `$4015` selection expects frame IRQ acknowledgement, but the observed frame IRQ remained set. The production DMC sample path reads the external mapper byte and updates the bus latch without the second internal register decode.

**First divergence:** The internal register selection immediately after the observed external sample GET is absent. This differs independently from the OAM overlap trace.

**Wrong-model candidate:** External-memory-only sample fetch. The observed `$FF16` and `$FFD5` cases reject it through controller and IRQ side effects.

## Row 4 — Explicit DMA Abort

**Historical status:** `FAIL 0x0a` in the checked-in scoreboard; not freshly run.

**Pinned expectation:** The assembly's explicit-abort answer key contains one-cycle results at two offsets and zero-cycle results later. The setup disables DMC around a scheduled request and separately depends on phase alignment.

**Observed core trace:** A synthetic pending request was disabled through the production `$4015` write, with and without one intervening CPU write. At the following read, the trace recorded a halt attempt but no accepted halt and no sample GET; state remained `dma_pending=1, remaining=0`.

**First divergence:** Candidate only. The fixture has not yet reproduced the exact pinned offsets, so it does not establish which answer-key entry this state corresponds to or close the row.

**Wrong-model candidate:** Cancel the pending request before the one-cycle aborted window. Exact offset and expected outcome remain to be localized.

## Row 5 — Implicit DMA Abort

**Historical status:** `FAIL 0x0a` in the checked-in scoreboard; not freshly run.

**Pinned expectation:** The assembly has one-byte non-looping and looping controls, a one-cycle window, and a separate following-write case. It accepts both early and later chip-revision answer tables; no narrower RP2A03G outcome is assumed here.

**Observed core trace:** No row-specific trace has been run.

**First divergence:** Unknown. No correction or causal grouping may rely on this row yet.

## Bounded commands and result

Read-only CTest inspection selected only `core.apu` and `bus.unit`; their generated test entries have no `FIXTURES_REQUIRED` or `FIXTURES_SETUP` properties. The selected closure did not include `accuracycoin.campaign`.

Commands run:

```text
cmake -S . -B build/phase24-owned -G Ninja -DCMAKE_BUILD_TYPE=Debug
ctest --test-dir build/phase24-owned -R '^(bus\.unit|core\.apu)$' -N
cmake --build build/phase24-owned --target bus.unit core.apu
ctest --test-dir build/phase24-owned -R '^(bus\.unit|core\.apu)$' --no-tests=error --timeout 30 --output-on-failure
build/phase24-owned/tests/bus.unit --dma-trace remaining
cmake --build build/phase24-owned --target bus.unit
build/phase24-owned/tests/bus.unit --dma-trace landing
```

The selected bounded CTest run passed both tests. The diagnostic mode completed and emitted the traces summarized above; its success means the observations were captured, not that the failing AccuracyCoin rows passed.
The latest `--dma-trace landing` run exited 1 on the expected diagnostic assertions: observed A was `0x40`, and the last halt/resume were not on the final `$4000` read because the last GET was earlier in the setup. It printed the trace above. This does not establish a fresh pinned-test result.

## Checkpoint disposition after parent review

Plan 24-01 remains incomplete. Its selected default bus/APU regressions pass; unresolved source-shaped timing and silent-clock assertions live in explicit diagnostic modes, not the default regression path. This does not skip any pre-existing test or claim a correction. The corrected current commands and proposed native-state diff are recorded in [24-01-CHECKPOINT.md](24-01-CHECKPOINT.md). No completed SUMMARY exists and no requirement is checked off. Rows 2–5 retain the partial statuses above; no grouping/fix premise is authorized from incomplete traces.
