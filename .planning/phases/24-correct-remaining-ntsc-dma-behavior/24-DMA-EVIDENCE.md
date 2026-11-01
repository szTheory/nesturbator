# Phase 24 DMA evidence

**Current scope:** Behavior14 / native format6 / ABI1 source corrections, owned C-fixture evidence, and a completed local full-CI/AccuracyCoin run. All five selected ROM rows pass locally. DMA-01 through DMA-04 remain open because cold protected-main, hosted cross-platform hash and performance evidence is still outstanding. The first row tables and state audit below are chronological snapshots; the final dated sections supersede earlier status statements.

## Identities, oracle and reproduction

- Pre-correction core: `6503f3551899a5d607d283a41a00451a7090f4de` (diagnostic checkpoint). `src/apu.c` from this identity is the independently reverted frozen-output model.
- Pinned test: AccuracyCoin `6bee919529289c555803b612b8c9e4f7eaaeeabb`; assembly SHA256 `540b524187a97d71018bb94a1b7874e19d37894e5882861fe0c31adafe251a9f`.
- [Pinned assembly](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm), [pinned README](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/README.md), [DMA hardware model at revision 23450](https://www.nesdev.org/w/index.php?title=DMA&oldid=23450), [APU reference, Output Unit](https://www.nesdev.org/apu_ref.txt). These are separate test/model sources, not fresh silicon measurements.
- Current fixtures compile the exact bus source with test-only observation and a renamed CPU copy. Release preprocessing removes observation. Ordinary linked and observed paths agree in the landing and silent-clock fixtures.
- The inherited diagnostic's sync addressing errors, approximate wait, mismatched synthetic phase history and out-of-range long DMC counter were corrected. Earlier conclusions relying on those setups are withdrawn. The synthetic timing helper now asserts its exact 30,000-cycle delay (30,012 inside the enclosing wait; 30,018 including its call).
- Synthetic lower-level matrices seed valid pending state; they do not claim those requests were created during the first write. Production reset/guest traces separately observe request creation, actual sample GET, and continuation. Synthetic phase starts keep ticks, CPU-cycle parity, bus phase and APU half-clock phase consistent.

Commands (all use the existing owned build):

```text
cmake --build build/phase24-owned --target bus.unit core.apu core.state_fork
ctest --test-dir build/phase24-owned -R '^(bus\.unit|core\.apu)$' --no-tests=error --timeout 30 --output-on-failure
build/phase24-owned/tests/bus.unit --dma-trace silence
build/phase24-owned/tests/bus.unit --dma-trace landing
build/phase24-owned/tests/bus.unit --dma-trace remaining
build/phase24-owned/tests/core.state_fork --dma
```

Generated CTest JSON selects bus.unit/core.apu and, for executable-closure inspection, core.state_fork.stress; there are no fixture dependencies on these entries. Direct --dma invokes only generated owned content. No full workflow, AccuracyCoin ROM, campaign, push or PR ran. Diagnostic process success means the required observations were captured; it does not mean the remaining failing models passed a correction oracle.

## 1. DMA Landing on Write — corrected owned tracer

Historical raw row: **FAIL 0x26**, not freshly run. Pinned `TEST_Fail` encodes `(ErrorCode << 2) | 2`, identifying Landing Test 9. Its one-write-delayed load should take four cycles, followed by a synchronized reload during the final `$4000` read.

| Evidence layer | Observation |
|---|---|
| Hardware/reference | Output Unit in HWC.23 clocks whole eight-bit audible or silent cycles, with a separate silence flag. An empty buffer must not freeze bit phase. RDY only halts CPU reads; actual DMC memory accesses use GET. |
| Pinned test | Test 9 disables/re-enables around a timed output cycle, delays the load with `$5000`, then expects the final `$4000` open-bus read to see zero after a sample GET. Tests 3/5 separately distinguish three/four-cycle write-retry shapes. |
| Baseline core | `bits` remains zero across six (now twelve in the default regression) output clocks while the buffer is empty. A later sample drains at the next timer clock, instead of its eight-clock boundary. The tightened Test 9 reconstruction fails its final A==0 and final-halt/resume assertions. |
| First causal divergence | The first empty-buffer output clock fails to reload/advance the eight-bit silent cycle. This is earlier than the premature later reload; the fresh-load write retry itself was already correct. |
| Corrected core | Silent clocks now count 0->7->6...->0->7 while preserving DAC level. In the tightened trace `$4015` completes at 37741 GET; `$5000` retries GET->PUT at 37745; halt is 37746 PUT, actual sample GET 37749 GET, resumed read 37750 PUT. The reload halts `$4000` at 38162 PUT, samples `$FFC0/00` at 38165 GET and resumes `$4000/00` at 38166 PUT. Absolute cycles are fixture-local. |
| Wrong-model control | `freeze-at-zero-while-empty`: the bit-clock oracle rejects a zeroed next count at the first empty-buffer output edge. Rebuilding the current owned executable against the baseline APU source independently fails both the silence and tightened landing probes. |
| Unverified remainder | No fresh pinned ROM result, hardware measurement, protected-key, cross-platform hash or performance acceptance. |

The default bus.unit includes the silent-clock and tightened landing probes as well as both phase starts and one/two/three-write pending-request matrices. No old test was removed or weakened.

## 2. DMC DMA + OAM DMA — independently localized

Historical raw row: **FAIL 0x0a**, not freshly run.

| Evidence layer | Observation |
|---|---|
| Hardware model | Independent DMA units overlap no-access phases; DMC memory GET wins a collision. The usual overlap costs two cycles; final two PUT positions can extend by one or three. OAM reads on GET and writes on PUT. |
| Pinned test | Its separate beginning/end sweeps compare a phase-sensitive answer table and describe overlapping halt/dummy/alignment cycles. Aggregate raw status is not the cause. |
| Owned setup | Interior and final windows at offsets 128/254/255, both consistent initial phases, load-waiting and ordinary-reload controls. The later request uses a full buffer, one remaining output bit and a valid 214-half-cycle timer, rather than counter >214. All 256 OAM destination indices/data are verified in order. |
| Core events | Ordinary PUT-target reload requests stay pending through OAM: no DMC sample GET, OAM duration unchanged, pending=1 afterward. A GET-target delayed load can fetch, but resumes the interrupted OAM source on PUT and writes its destination on GET; common interior duration adds three clocks. Counts include the caller's resumed CPU read: baseline 514/515, load collision 517/518. |
| First causal divergence | The OAM destination clock never advances a pending DMC halt/dummy state, while source reads only try the ordinary CPU-halt gate. The expected overlapping sequence is absent. When a load does run, the generic CPU-resumed read becomes an OAM source access on PUT. |
| Wrong-model candidates | `cpu-halt-gate-inside-oam` (reload starves); `standalone-dmc-resume-as-oam-read` (wrong source/destination phase). Independent controls are retained for Plan 02. |
| Unverified remainder | Exact pinned answer-table results, all final-window source-supported durations after correction, native collision continuation and campaign acceptance. |

This cause remains present with the pre-fix APU independently linked, and is not grouped with the silent-clock correction.

## 3. DMC DMA Bus Conflicts — independently localized

Historical raw row: **FAIL 0x0a**, not freshly run.

| Evidence layer | Observation |
|---|---|
| Hardware model | Parked CPU address bits 15:5 and external sample bits 4:0 select internal registers. Internal/external bus latches and register side effects are distinct. |
| Pinned test | Samples ending in $15/$16/$17 exercise status/controller selection. The assembly explicitly does not inspect the DMC shift-register input. |
| Owned setup/core | Park `$4000`; actual GET `$FF16/$A0` stays external-only: CPU sees `$A0`, controller shift remains `$01`, versus the selected controller bit yielding `$A1` and a clock. Actual GET `$FFD5/$A5` leaves frame IRQ set instead of acknowledging internally selected `$4015`. |
| First causal divergence | Immediately following the actual mapper sample GET, the internal-register decode/side effect never occurs. `dmc_dma` only updates the one existing bus latch from the external byte. |
| Wrong-model candidate | `external-memory-only-fetch`: distinct controller/IRQ effects reject this independently of any OAM timing change. |
| Unverified remainder | Corrected split-latch semantics, both-controller/alias controls, reachable native latch continuation and required acceptance. The pinned test is not evidence for bits entering the DMC shift register. |

## 4. Explicit DMA Abort — independently localized

Historical raw row: **FAIL 0x0a**, not freshly run.

| Evidence layer | Observation |
|---|---|
| Hardware model | DMA rev 23450 Bugs describes an explicit stop in the APU cycle before reload scheduling: a one-cycle aborted halt; a write at that halt suppresses it instead of deferring an ordinary DMA. |
| Pinned test | A 16-offset disable/re-enable sweep has one-cycle entries and later zero-cycle entries in its answer key. |
| Owned setup | Production reset and real register writes establish a looping one-byte sample and full buffer. Ordinary reads reach bits=0/counter=1 at the appropriate half-clock; production `$4015=0` stops immediately before rollover. No request/bit/phase field is injected. Separate no-write/write-on-halt controls preserve the write effect. |
| Core events | Stop at cycle 433 GET, ordinary reads 434 PUT / 435 GET / 436 PUT / 437 GET. The reference sequence requires a one-cycle aborted read halt before resumed execution; observed total stays ten ordinary clocks, no sample GET and no retained abort request. The write control writes RAM at 436 and is also ten clocks. |
| First causal divergence | On the required aborted-halt window, the core completes the ordinary CPU read without a halted repeat. The stop clears remaining; subsequent output-clock scheduling requires remaining !=0, so it loses the stop-window history and cannot produce an aborted DMA at all. This supersedes the older injected-pending candidate. |
| Wrong-model candidate | `forget-stop-before-reload`: zero remaining is treated as eliminating both a real memory fetch and the separate one-cycle halt. |
| Unverified remainder | The diagnostic sweeps three neighboring timer-counter offsets, both half-clock phases and separate writes. Exact corrected offset durations and the full pinned answer key remain Plan 03/04 work. |

## 5. Implicit DMA Abort — independently localized

Historical raw row: **FAIL 0x0a**, not freshly run.

| Evidence layer | Observation |
|---|---|
| Hardware model | An empty-buffer, non-looping one-byte load can finish in the APU cycle before the next output boundary and require the same one-cycle aborted halt. Writes suppress it. The later RP2A03G/H extra-reload window is a separate revision-dependent case. |
| Pinned test | Separate non-looping, following-write and looping sweeps; README retains both early and later accepted outcomes. |
| Owned setup | Production reset, second-fastest timer, empty buffer and a genuine silent output cycle. Wait for counter=5/bits=0 at the chosen half-clock, then enable via `$4015`. Separate looping and length-17 controls use the same production history; no phase injection. |
| Core events | Enable at 569 GET, halt 573 GET, actual sample GET `$C000/$EA` at 575 GET, resume 576 PUT. No second halt at the common aborted window (578 PUT); ordinary reads continue and remaining=0/pending=0. A write at 578 is preserved. Looping and length-17 controls retain remaining and produce a second real sample GET. |
| First causal divergence | Completion of the one-byte sample clears remaining at the sample GET; the following output-cycle boundary does not remember that just-completed stop and emits no one-cycle aborted halt. This is independently observed without any `$4015` disable. |
| Wrong-model candidate | `forget-one-byte-completion`: ordinary no-work behavior after remaining reaches zero, omitting the distinct aborted read. |
| Selected-profile assumption | The owned later extra-reload control will use the early RP2A03G outcome (no optional extra fetch). This common aborted window applies to both accepted profiles. No universal RP2A03G or measured chip-date claim is made; both pinned acceptable result classifications remain unchanged. |
| Unverified remainder | The diagnostic sweeps counter offsets 4/5/6, both half-clock phases, non-looping/looping/length-17 and following-write controls. Corrected per-offset oracles, native stop-history continuation and fresh pinned acceptance remain Plan 03/04 work. |

The explicit and implicit paths now both have independent records. Similar final counters do not by themselves authorize sharing a fix; Plan 03 must compare their actual stop-history events first.

## State audit and native continuation

The owner explicitly approved [the concrete state decision](24-01-CHECKPOINT.md) before mutation. The implemented DMC visitor adds `silence` at local serialized offset 15; `bits` at offset 14 now counts silent output cycles too. Later DMC fields shift by one byte without changing meaning. Serialized DMC length is 23 (was 22), while C-struct size is 24 because of padding; struct padding is never serialized. Format is 4, behavior revision 10, ABI unchanged; no converter. The visitor validates silence <=1 in both validation/apply passes. Internal pointer-offset tripwires account for alignment without adding saved bytes.

`core.state_fork --dma` reaches real public frame-return cuts at frame 1 (silent, bits=7) and frame 29 (silent, bits=5, DMA pending), using the existing owned stress guest with a mixed-bit sample. A never loads; B restores the native state. Both produce identical full pixel and PCM buffers, metadata, RAM effects, field-wise cycle/level transition lists and final canonical bytes. Each continuation includes nonzero DAC transitions. Valid-valued silence perturbation changes PCM and is rejected by the continuation oracle. Old-format-tagged input and an invalid silence byte are refused with the instance and save generation unchanged. The same fixture is called by existing --stress; no new CTest registration or public cycle API exists. Synchronous sample/transfer steps remain unexposed as public save points.

## Acceptance still missing

Owned checks do not replace the required fresh full pinned result, every protected PASS key, deterministic cross-platform hashes or performance gates. The runtime stop remains active. No requirement or phase completion may be marked from these local results. Later plans must preserve the five independent records and demonstrate their own wrong-model regressions before claiming corrections.

## Version-consumer review follow-through

The scoped ownership amendment and review resolve the format/revision consumers alongside the correction. C/C++ header checks, API version, older/future refusal tests, new silence bounds, normalized API policy and current-version docs agree on format 4/revision 10. Six boards' state hashes were regenerated at frames 0/60; every state size grew one byte and frame ticks were unchanged. No frame/PCM/transition hash row was changed. The declaration-stream normalization proves only the two approved version macros changed the API baseline. Historical revision-9 AccuracyCoin metadata and its checker remain historical; they are not relabeled as current evidence. Bounded doc contract checks passed after the documented helper targets were built, without changing existing timeouts.

## Plan 24-02 OAM correction — bounded, acceptance still pending

The OAM loop now services a pending DMC halt/dummy/alignment using three synchronous local stages. Those no-access stages overlap OAM GET/PUT; only actual DMC sample GET steals the source slot. OAM waits the following PUT, then retries the same unconsumed byte on GET. All 256 destinations retain their order/data and PUT phase. The CPU resumes its original `$8000/$A7` read with the expected final phase. Standalone DMC sample completion is shared without changing its saved fields or ordering.

The matrix pins fixed expectations independently of observed timing: interior offset128 adds2 for both phase/load cases; phase0 tail offsets254/255 add1/3; phase1 tails add2/1. Observed collision and resumed-source indices, actual sample address/data/GET, final read and all256 destination events are separately asserted. The normal and observer sources agree. Compiling **both** sources from the retained pre-OAM `313a0f3` bus rejects the same regression through missing GET, wrong OAM phase and fixed duration/index assertions. These are named `cpu-halt-gate-inside-oam` and `standalone-dmc-resume-as-oam-read` controls, not synthetic fabricated traces.

No OAM local stage can straddle a public call return: transfers finish synchronously. Saved OAM pending/page and DMC request fields retain their existing interpretation, so format stays4. Behavior revision11 and its version consumers are updated. All twelve six-board state hashes differ from the prior revision10 pins **only** in the behavior tag and CRC; normalizing those two fields reproduces the old digests exactly. State lengths, ticks, every payload byte and all other hash inventory rows remain unchanged.

The old owned stress timing missed OAM-pending cuts inside the retained 60-frame short window after this actual timing correction; both board assertions failed. A measured local gap G=6 restores all original OAM/DMC/IRQ boundary checks on NROM/MMC3, without extending bounds. Native --dma reaches silent frame1, pending-DMC frame17 and active-DMC/OAM-pending frame45; full pixel/PCM/metadata/RAM/transition/canonical-state continuation agrees. Skipping pending OAM is rejected by final-state equality. Short --stress retains all original forks/straddle assertions and passes both boards (first OAM45/41). The [scoped amendment](24-02-AMENDMENT.md) and [resolved review](24-02-REVIEW.md) explain the necessary fixture/version follow-through.

Historical pre-approval checkpoint (superseded by the correction below): the split-bus slice had a new independent red latch test: `$4015` parked, GET `$FF15/$A5`, internal D5=0. It produces status20/20 and external/controller20/21, rather than source-supported00/00 andA5/A1. `--dma-trace conflict-latches` intentionally fails; the default regression does not invoke an unimplemented correction. [24-02-CHECKPOINT.md](24-02-CHECKPOINT.md) gates the proposed saved internal byte/external-latch meaning and format5/revision12 **before mutation**. No new latch or format-5 visitor exists, Plan24-02 remains incomplete, and no new campaign result is claimed.

## Plan24-02 split-bus correction — current source

Owner approved the exact internal cpu_data/external open_bus distinction on2026-10-31 before mutation. Coherent implementation is d4a24c8, format5 / behavior12 / ABI1. BUS grows2059->2060 serialized bytes, adding cpu_data at local2049; phase moves2049->2050 and final strobe2058->2059. Host input_pending remains unsaved. C sizeof2061->2062 is separate from the payload. Formats1..4 are refused without conversion. Earlier silence and OAM decisions remain historical; this approval does not cover future abort fields.

| Hardware explanation | Pinned test expectation | Owned current-source observation |
|---|---|---|
| Internal select combines coreA15:5 and externalA4:0; status is internal-only (DMA rev23450; HWC.08/19). | TEST_DMABusConflict checks CPU-visible internal-register effects and external sample bytes; explicitly cannot qualify DMC shift-register input. | Mapper sample FF15/A5 on GET remains externalA5; status returns00 twice with internalD5=0, then controller seesA1. The pre-split model returned20/20 and external20/controller21. |
| Controller data merges with external upper bits; retained core profile clocks each read cycle. | Both controller offsets16/17 appear in the answer sheet; console-dependent floating bits are measured by the ROM. | Fixed owned matrix covers both ports, load/reload, sampleA5/C2, low14..18, parked4000/401f/4015/4016/4017/4020. Controller shifts, separate internal event and actual sample/phase, resumed value and normal/observed parity pass. No NES-001 clock-profile claim is added. |
| Status acknowledgement follows existing next-GET clear semantics. | Frame IRQ is deliberately set so alias15 can clear it. | Internal alias sets clear_pending on sampleGET; resumedPUT retains the IRQ until nextGET clears it. It does not replace external held data. Unconnected internal reads retain status data rather than recopying the sample. |

Native guest-produced frame1 has cpu_data10/open_bus40. Public load restores both poisoned latches and canonical bytes, refuses format4 atomically, and matches uninterrupted next-frame pixels/PCM/metadata/RAM/transitions/final state. Private immediate status/controller stimuli on disposable copies independently reject corrupt internalD5 or external upper bits. These are separate from the public guest continuation; a new opcode fetch can converge initial latches. No mid-DMA save boundary is fabricated.

Wrong model: e08c999 bus source compiled in BOTH linked and observer paths with current declarations/tests, preserving corrected silence/OAM, rejects the new default matrix with exit1. It misses the internal select and register/IRQ/controller effects and fails the split-latch probe. Production current source passes; this is row-specific source behavior evidence, not a compatibility claim about an old binary.

Exact bounded commands/results are in [24-02-SUMMARY.md](24-02-SUMMARY.md). All12 six-board state dumps normalize byte-for-byte to their previous pinned hashes after deleting only added cpu_data at file2147 and restoring format4/revision11/length/CRC. Sizes grow1, ticks and non-state hash inventory stay fixed. Review has no active finding. Current format5/hash/API/docs tests pass; historical revision9 campaign metadata is unchanged. No protected ROM/campaign acceptance is inferred. Explicit/implicit abort remain uncorrected.


## Plan24-03 fail-first abort contracts and corrected explicit offset

The retained running control now independently establishes ordinary reload halt434 PUT and sampleGET437, at14 clocks for10 ordinary CPU accesses. This withdraws the exploratory explicit-row inference of halt436 and the counter1/half0 stop433 abort classification above. Counter2 at either half gives stop431/432, three/two clocks before the measured running halt, which is the source-defined abort window. The new fixed oracle requires aborted halt434 with no sample and total11 clocks; current source gives10/no halt for both. Later counter1 stops433/434 and earlier counter3 stops429/430 are no-abort controls. The stop434 case exposes a stale ordinary pending flag. A write434 suppresses the aborted halt. This is a corrected diagnostic oracle, not a new hardware phase measurement or an inferred ROM pass.

The independently derived implicit case still gives sample575 and missing abort578 for both enable halves. The write control is now explicitly at cycle578 for BOTH half-phase setups; the old relative-offset write landed one cycle too late in one setup. Looping/length17 retain two actual samples, and selected early-G neighboring counter4/6 cases have no late-G/H extra sample. Both pinned accepted implicit outcomes remain untouched.

New explicit test_dma_explicit_abort and implicit test_dma_implicit_abort run ONLY under `bus.unit --dma-trace abort-contract` until the production correction. The running control is also in default bus.unit. Fresh red contract exits1, independently rejecting missing explicit/implicit aborts and the stale stopped request; default bus.unit/core.apu pass. Detailed local log: build/phase24-owned/phase24-abort-contract-before.txt. Production bus/APU/serializer/header remain at committed format5 /behavior12. No new field or meaning is implemented. [Concrete one-byte abort-history proposal](24-03-CHECKPOINT.md) gates Plan03 before mutation; it is not covered by the earlier bus approval. No AccuracyCoin ROM, full workflow or campaign ran.

Scoped diagnostic review warnings are resolved: the implicit sample anchor575/EA and actual write578 PUT/address/data/RAM effect are asserted at both enable halves. Abort and resumed-access cycles are pinned separately. The representation review is clean for the evidenced states, while re-enable/OAM/new native-cut implementation remains gated. Fresh corrected helper compilation succeeds before recording red/default results. [Review](24-03-DIAGNOSTIC-REVIEW.md).

## Plan24-03 implemented abort correction — format6 / behavior13

Owner approved the exact countdown/incompatibility before mutation; coherent source commit7f31579 includes all behavior/native/tests/docs/version consumers. New abort_delay u8 at DMC local23 records0inactive or1..3 future CPU clocks; DMC payload23->24 while its Csizeof24 is unchanged. Format6 refuses1..5 without conversion; ABI1 and prior fields/BUS meaning stay fixed. There are no added scheduler/dependency/global fields. [Final summary](24-03-SUMMARY.md) and [review](24-03-REVIEW.md).

| Row | Independent source-bound expectation | Corrected core observation | Rejected wrong model |
|---|---|---|---|
| Explicit abort | Stop two/three clocks before reload; single aborted read halt, writes suppress. DMA rev23450 Bugs; pinned16-offset sweep. | Running halt434PUT/sample437GET anchors stops431/432; each now has halt434/resume435,11 clocks for10 accesses, no sample. Earlier/later cases0extra; write434 suppresses and preservesRAM. | aa78a23 no-stop-history code gives10/no halt; due-write-retains-countdown mutation gives11 where write requires10. |
| Implicit abort | Non-looping length1 final load shortly before buffer empty has same one-cycle window; write suppresses; lateG/H unexpected reload is distinct. Pinned separate write/loop sweeps, both accepted revision outcomes retained. | Both enable halves produce sample575GET/EA then halt578PUT/resume579GET; actual write578 event/RAM suppresses. Loop/length17 retain two real samples; adjacent early-G cases noextra. | aa78a23 misses the second halt; retry mutation creates a delayed halt after write. |

Both independent wrong models compile in BOTH linked and observed bus paths; the no-history model also uses pre-abort APU source. Each focused contract exits1 and restored production passes. Sources before correction retain all three earlier row corrections. Full parity checks compare normal/observed access values and final machine bytes. New fixed controls preserve both controllers' repeated reads, status IRQ acknowledgement/external held data, ordinary request deferral, OAM256 bytes/noextra slot, re-enable and cancellation/fresh-enable history. The old source never fetched a disabled stale pending request because its remaining==0 guard already refused it; clearing that bookkeeping is now explicitly tested outside the abort window. HWC.10 Memory reader and HWC.08 Status set reader eligibility from nonzero remaining; no old successful transfer is removed. Accepted DMA is synchronous and cannot contain an intervening CPU write.

Public native frame1 cuts reach explicit delay2/cycle29782, explicit delay3/cycle29781 and implicit delay1/cycle29781 through real guest instructions. All full frame/PCM/transition/effect/final-state continuations match without A loading. Invalid4/old5 refuse atomically; clearing the valid countdown loses a real halt and changes next-frame timing/canonical state. Disposable clones execute the actual next INC in6versus5 clocks with the correct PC/RAM effect. All previous native cuts and short stress witnesses remain intact. No public mid-DMA cut is manufactured.

Six-board native sizes grow1 and all12 dumps normalize to the exact previous hashes after removing only the new byte and restoring version/length/CRC. Non-state rows/ticks/inventory stay unchanged. Final owned bus/APU/controller/entry and version/doc/format/refusal checks pass; release preprocessing erases all observer markers. No fresh ROM or protected/cross-platform/performance acceptance is implied. Literal main...HEAD whitespace check fails on inherited planning records already present before execution; correction delta89a042b..HEAD is clean. Detailed bounded evidence and remaining gates are in the summary and forthcoming acceptance record.

## Post-workflow correction of the bounded oracle

The complete behavior13 campaign exposed three remaining selected failures.
[Plan04 corrections](24-04-CORRECTIONS.md) supersede the old no-GET expectations
at explicit stops433/434 and the retained-internal-data expectation after
4015 deselection. The new source-shaped64-entry abort suite and its immediate-
visibility wrong model resolve the missing looping cases. Those later owned
results do not replace the recorded full-ROM result or its source identity.

## Plan24-04 current-candidate reconciliation

The evidence above is chronological, not one claim about one source revision.
This section records the final source and bounded build identity after the
corrections. It does not convert the historical behavior13 ROM report into a
behavior14 result.

### Candidate and build identities

- Prior complete ROM run: source `0f7171ab1960987273689653794f522276b78338`,
  behavior13 / format6; campaign report SHA256
  `aed1823fe23884583b0a25f7a5a827c3b6f51744f6c07fa0906617a34ba2dcc5`; build
  input fingerprint `2ae704c0a21b865ca861faf01b5399194c1fe411324906e85b184de3dcbef3d2`
  (56 inputs). The result was complete but failed three selected rows; see
  [24-CI-RUN.md](24-CI-RUN.md). Its one-run authority is consumed.
- Corrected source candidate: `324a769e413003cce3aa30d6a5a78a441ce558d8`,
  behavior14 / format6 / ABI1. The latest correction adds no serialized field
  or meaning: `abort_delay` remains the format6 DMC field, payload bytes remain
  format6, and `dmc_sample_ready` is a synchronous local argument that cannot
  cross a public save boundary.
- Bounded Task1 build tree: existing `build/phase24-owned`, Debug,
  `NESTURBATOR_ACCURACYCOIN_FULL=OFF`, `NESTURBATOR_VECTORS_FULL=OFF`. After
  the bounded build, SHA256 identities were: `bus.unit`
  `8a3bc05a75c9bb195e59f3bc406a88594fe4dccd44b4decf6c3cfa866650fb0b`,
  `core.apu` `7e089885aa621ce7d9d11ad59c4b4ca1807bb54fb40edf6ccc7f784807a1690d`,
  `core.controller`
  `6cfcb00af5ca610b85395808b98f587d781659af4aa5143fc930d31206db938f`, and
  `core.state_fork`
  `7a27d691ad4b2ae3bc187e08d88a377bb45cb289fbc950fe9b5b41185c18e153`.

### Five independent row records

| Row | Source lineage | First divergence and corrected trace | Independent wrong model |
|---|---|---|---|
| DMA Landing on Write | Diagnostic `6503f3551899a5d607d283a41a00451a7090f4de` -> format4/behavior10 correction `e2f3c50c9e548576c9a8a93bd1ea7f3527f0eefc` -> current candidate `324a769e413003cce3aa30d6a5a78a441ce558d8`. | First empty-buffer output edge failed to advance the silent eight-bit cycle. The corrected trace advances silent bits while retaining DAC level; the tightened landing case samples `$5000` GET->PUT at37745, halt37746 PUT, sample GET37749 and resume37750. Its synchronized reload samples `$FFC0/00` at38165 GET and resumes `$4000` at38166 PUT. | `freeze-at-zero-while-empty`; the bit-clock oracle and retained pre-correction APU source reject the zero-count model. |
| DMC DMA + OAM DMA | Pre-OAM bus `b9e635dbebb1a497bccc9b5ea683d31e5d73ae46` -> format4/behavior11 correction `8c57c7737c437612b335d257c575a0fb43f57152` -> current candidate `324a769e413003cce3aa30d6a5a78a441ce558d8`. | The OAM clock failed to advance a pending DMC halt/dummy state. Corrected interior collision durations are517/518 clocks including the resumed CPU access, versus514/515 without load; all256 OAM destinations preserve byte order and phase. | `cpu-halt-gate-inside-oam` and `standalone-dmc-resume-as-oam-read` independently reject missing arbitration and the wrong resumed source/destination phase. |
| DMC DMA Bus Conflicts | Pre-split bus `bafb374d80e1b8d01ede3d3600bb6622e6552f4f` -> format5/behavior12 split-latch correction `00402ced249cdaaba16dd76c4aee63833df1208d` -> current candidate `324a769e413003cce3aa30d6a5a78a441ce558d8`. | The first divergence is the mapper sample GET not applying the selected internal-register side effect. The current matrix checks `$FF16/$A0` external data separately from controller shift/clock effects, `$FFD5/$A5` status IRQ acknowledgement, and the external held byte becoming visible after `$4015` deselection. | `external-memory-only-fetch`; the retained pre-split bus source independently fails the status/controller/latch assertions. |
| Explicit DMA Abort | Pre-abort bus/APU `8e41fd0e879b6e55c74d963b7b80f0c084f499c6` -> format6/behavior13 correction `0b71adf06452644b9105a290279a475ddbf67fef` -> current candidate `324a769e413003cce3aa30d6a5a78a441ce558d8`. | The fixed key is `4,4,4,4,4,4,3,4,1,1,0,0,0,0,0,0`. At offset6 an already armed request completes; at offset7 disable sees counter1/GET/bits0/full buffer/remaining1/pending0 and preserves the subsequently armed ordinary GET without underflow or restart. The old unsupported no-GET expectations at stops433/434 are superseded. | `forget-stop-before-reload`; separate due-write suppression controls reject an abort countdown incorrectly retained after the write. |
| Implicit DMA Abort | Pre-abort bus/APU `8e41fd0e879b6e55c74d963b7b80f0c084f499c6` -> format6/behavior13 correction `0b71adf06452644b9105a290279a475ddbf67fef` -> current candidate `324a769e413003cce3aa30d6a5a78a441ce558d8`. | A non-looping final sample at575 GET/data`EA` was followed by no required abort halt at578 PUT. The corrected trace halts at578 and resumes579 GET; looping offsets8/9 retain the fresh byte through the immediate output edge, avoiding the extra refill. The selected early-G ordering is an inference from the pinned key, not universal hardware proof. | `forget-one-byte-completion` rejects the missing halt; `immediate-sample-visibility` independently reproduces and rejects the extra loop refill. |

The named wrong models above were compiled against the same owned fixtures as
the production source, as detailed in Plans01-04 and
[24-04-CORRECTIONS.md](24-04-CORRECTIONS.md). The current reproduction is the
bounded `bus.unit` test plus the focused build/CTest/native-fork command in
[24-ACCEPTANCE.md](24-ACCEPTANCE.md). The production/observer paths agree in
the owned row fixtures; the previously recorded warnings-as-errors Release
build and preprocessing check remove all observation markers. These results
establish source/test compatibility only, not silicon behavior.

### Current state and acceptance status

The current native contract is format6 / behavior14 / ABI1. The approved
`abort_delay` field remains serialized at DMC-local offset23 with values0..3;
the current synchronous sample-readiness argument adds no state, API or
converter. Older formats1..5 remain refused atomically. No old-format
compatibility, behavior-revision loader enforcement, or cross-platform hash
claim is added. The fresh local ROM evidence is recorded in the dated section
below; it does not establish protected-main authority, cross-platform
frame/PCM/transition equality, unchanged performance qualification, or
silicon behavior. Do not complete any requirement or phase from local results
alone.

## Full local behavior14 acceptance — 2026-11-01

The final local `cmake --workflow --preset ci` run passed all707 CTest entries
in517.82 seconds and produced the library, runner and libretro macOS arm64
packages. Its complete AccuracyCoin campaign ran22/22 pages and observed all
146 scored rows terminal:126 PASS,20 FAIL, no skip/not-run/timeout, and5
inventory-only DRAW rows. All five selected Phase24 rows passed at the pinned
ROM and classifier: Landing `0x01`/frame128, DMC+OAM `0x01`/frame46, Bus
Conflicts `0x01`/frame46, Explicit Abort `0x01`/frame46, and Implicit Abort
`0x09`/frame46 (accepted source-defined pass variant). DMA + `$4015` Read,
Misaligned OAM DMA, and INC `$4014` passed. All119 previous current-pin
protected PASS keys remain passing.

The campaign used source revision `0c8cc53b1672` with a dirty checkout;
behavior14 implementation is committed at
`324a769e413003cce3aa30d6a5a78a441ce558d8`. The input fingerprint is
`f04cdd6ec67f0d02b0a6043ede6a2c6b5b6a6c85eee6f6277f73ac75100cd606`; report
SHA256 is `f3b5bbf7ddcf4d18deabda6d875b441330dc71c97b8aca5fdcc38bf471f98f6c`.
See [the exact run record](24-CI-RUN-20261101.md) and
[campaign measurements](../../../tests/accuracy/measurements.md#phase-24-behavior14-local-acceptance-2026-11-01).

Protected observation used the checkout `scoreboard-main` fallback, not
independently prepared cold-main authority. Current-head hosted lanes,
Linux/macOS/Windows x64/arm64 frame/PCM/transition equality, and the unchanged
Cachegrind `+1%` evidence remain outstanding. This report is compatibility
evidence against its pinned tests, not a universal hardware claim.
