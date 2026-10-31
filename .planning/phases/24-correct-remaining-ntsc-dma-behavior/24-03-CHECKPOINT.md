# Phase24-03 abort history decision — awaiting owner

## Concrete choice

Recommend one saved `uint8_t abort_delay` appended to DMC state after enable_delay. It records a one-shot aborted halt due in1..3 future CPU clocks;0 means inactive. This supports the shared source-defined two/three-clock stop window for explicit disable and implicit completion of a non-looping last byte. It remains separate from a normal sample DMA request, which can retry on writes. An aborted halt suppressed by a write expires instead. This new history can survive an instruction/public frame return after the stopping event.

Approve together: DMC payload23->24 bytes, native format5->6, behavior12->13, ABI1 unchanged. Formats1..5, including the unreleased current format5 states, will be refused without conversion. Battery saves remain in their existing separate format. This decision covers the concrete shared abort history and its coherent tests/docs/version consumers; it does not renew fullCI/AccuracyCoin or authorize a later-G/H unexpected-reload model.

## Evidence and corrected diagnostic assumption

The independent stopped and running fixtures use real power-on/register/read history; no timer, bit, pending or GET/PUT field is injected. The running control anchors a reload halt at434 PUT and actual sample at437 GET, with10 ordinary CPU accesses plus4 stolen cycles. Therefore explicit stops431 GET/432 PUT are the source-defined three/two-clock cases and require one aborted halt at434 PUT, with no sample fetch. Stops433/434 and earlier429/430 are adjacent non-abort controls. A write at434 must suppress the one-cycle halt rather than defer it.

An earlier exploratory diagnostic guessed a436 halt and misclassified stop433. That was not a verified timing oracle. The running control replaces it; the old inference is withdrawn explicitly in the ledger. The new explicit contract is red at both correct two/three-clock windows (10 clocks rather than11, no accepted halt). It also catches a stale ordinary dma_pending after disable at434.

The independent implicit fixture starts an empty-buffer one-byte sample. Both enable half phases produce actual sample GET575, then require a one-cycle aborted halt578 PUT and resumed read579 GET. Current source has only the load halt and no aborted halt. Write578 suppresses it; looping and length17 controls retain two real sample fetches. Neighboring counter4/6 setups retain the selected early-G model with no late-G/H unexpected reload. Both historical outcomes accepted by the pinned ROM remain accepted; no classifier/scoreboard change is proposed.

Exact bounded reproduction:

```
cmake --build build/phase24-owned --target bus.unit core.apu
build/phase24-owned/tests/bus.unit --dma-trace abort-contract
ctest --test-dir build/phase24-owned -R '^(bus\.unit|core\.apu)$' --no-tests=error --timeout 30 --output-on-failure
```

The explicit red mode exits1 on the missing aborts/stale request. Default bus/APU regressions pass. Local detailed log: build/phase24-owned/phase24-abort-contract-before.txt. No ROM/campaign ran. [DMA rev23450 Bugs](https://www.nesdev.org/w/index.php?title=DMA&oldid=23450#Bugs) defines the2/3-clock window and write suppression; pinned [explicit/implicit assembly](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm#L12944-L13075) provides independent offset sweeps and the special non-retrying write case. Model-relative owned timing is compatibility evidence, not a measured physical power-on alignment.

## Exact proposed serialized change

| Field | Current format5 | Proposed format6 |
|---|---|---|
| DMC enable_delay | local offset22, u8 range0..4 | unchanged |
| DMC abort_delay | absent | local offset23, u8 range0..3 |
| DMC payload |23 bytes|24 bytes|
| DMC C struct |24 bytes including trailing padding|24 bytes, new byte uses that padding|
| Following APU fields | immediately after DMC payload | move one serialized byte; same meaning |
| BUS and all existing DMC fields | approved format5 layout/meaning | unchanged |
| Each native state |current size|one byte longer|
| Format / behavior / ABI |5 /12 /1|6 /13 /1|

No normal dma_pending enum/range expansion is proposed. Clearing a canceled ordinary request will be a behavior correction; it remains a normal sample-request flag. No global, pointer, host field, scheduler or dependency is added.

## Implementation boundary after approval

Explicit disable and non-looping last-sample completion share only the qualified stop-window helper, after their independent traces. It records2/3 future clocks when appropriate. The bus read path captures whether the countdown is due before its APU clock decrements it, then repeats the parked access for one halt and resumes normally without a mapper sample. A write naturally consumes the due countdown without retry. During synchronous OAM, the CPU is already halted; the countdown expires without stealing a source/sample slot. Stop, re-enable, ordinary pending request and load-delay interactions need independent controls before claiming completion.

The canonical visitor validates0..3. Native owned guests must reach real saved abort-history boundaries and match uninterrupted/resumed accesses, pixels, PCM, field-wise transition events and final state; a perturbed countdown must fail its continuation oracle. Public save/load remains synchronous with no fabricated mid-DMA capability. Test current-format load, old-format refusal and failed-load atomicity. Carry README/public-header comments, API/progress/reproduction docs, C/C++ version assertions, current-source version/API pins and existing six-board native hash consumers in the same coherent correction. Preserve historical revision9 evidence and all non-state hash keys. Exact implementation remains subject to these tests and review; if a genuinely additional persisted distinction is required, stop again before changing it.

## Diagnostic review

[Scoped review](24-03-DIAGNOSTIC-REVIEW.md) found two oracle gaps; both are resolved. The implicit counter5 sample is fixed to GET575/dataEA for both halves. The write control asserts the actual578 PUT event/address/data and RAM effect. Abort/resume cycles are separately pinned. The reviewer agrees the one-byte representation is sufficient for the evidenced history with the stated ordering, and leaves actual re-enable/OAM/native continuation as implementation gates. The reviewer did not run tests. Fresh local build, selected bus/APU tests, formatting and whitespace checks pass; the explicit red mode still exits1 solely for the unimplemented abort/stale-request behavior.

## Options and recommendation

1. **Approve the one-byte countdown, format6 and behavior13.** Continue the two abort corrections and their native coverage together within Plan03. Recommended: explicit history is instance-local, directly validated and small.
2. **Defer the abort correction.** Keep current format5 production behavior, committed completed slices and red diagnostics; Plan03, the phase and requirements remain incomplete.

Reusing a normal pending flag would conflate a write-retrying sample transfer with an expiring one-cycle halt. Inferring recent stop timing solely from remaining==0 loses whether the reader stopped in the qualifying window. A temporary local cannot survive a returned stopping instruction. Omitting native preservation would lose the next halt on resume. No converter or silent compatibility claim is offered.

**Owner acts next:** approve or defer this concrete additional saved history/incompatibility. The previous bus-latch approval is settled and implemented; it does not cover abort history. The existing fullCI/AccuracyCoin stop stays active without another confirmation.

After the decision: `$gsd-execute-phase 24`. Context may be cleared; read STATE.md and 24-EXECUTION-RESUME.md first.
