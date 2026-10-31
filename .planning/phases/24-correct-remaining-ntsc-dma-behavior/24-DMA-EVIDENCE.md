# Phase 24 DMA evidence

**Scope:** Owned C fixtures and source analysis only. The landing and OAM corrections have owned evidence at native format 4 / behavior revision 11. Bus conflicts and both abort rows remain uncorrected, with independent traces. Full-CI/campaign acceptance and DMA-01 through DMA-04 remain pending. No fresh AccuracyCoin result is claimed.

## Identities, oracle and reproduction

- Pre-correction core: `7348a10c68e592b71ec1899121003bf62d6bc813` (diagnostic checkpoint). `src/apu.c` from this identity is the independently reverted frozen-output model.
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

The next split-bus slice has a new independent red latch test: `$4015` parked, GET `$FF15/$A5`, internal D5=0. It produces status20/20 and external/controller20/21, rather than source-supported00/00 andA5/A1. `--dma-trace conflict-latches` intentionally fails; the default regression does not invoke an unimplemented correction. [24-02-CHECKPOINT.md](24-02-CHECKPOINT.md) gates the proposed saved internal byte/external-latch meaning and format5/revision12 **before mutation**. No new latch or format-5 visitor exists, Plan24-02 remains incomplete, and no new campaign result is claimed.
