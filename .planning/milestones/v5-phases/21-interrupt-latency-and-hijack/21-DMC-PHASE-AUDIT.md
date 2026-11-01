# Bounded DMC phase-contract audit

**Continuation:** The initial read-only findings below were followed by an
authorized implementation continuation. Its proven resolution is recorded at
the end; it supersedes the initial inconclusive status, not the retained audit
observations. No STATE/ROADMAP or source commit was made by this continuation.

Date: 2026-10-29. Read-only source audit during Plan 21-04. No runtime/test
changes, builds, test executions, campaign executions or commits were performed
by this audit. The executor owns measurements and the live source. Source line
numbers below describe the inspected working tree; concurrent edits can move them.

## Conclusions

The original observed `$40` has a direct mechanism: a DMA on JSR's final mapped
operand read is followed by the resumed mapped read driving `$40`; `$4013` then
retains `$40`. Keeping the GET byte over the resumed ROM read would violate the
independent mapped-read control. The repair must make the correct CPU access own
the halt, rather than alter ordinary open-bus replacement.

Two distinct boundaries need separating: when an output-unit event requests DMA,
and which bus cycle RDY actually halts. A pending-at-opening guard by itself is
not established as the complete hardware correction. In particular, the new owned
JSR fixture uses a different relationship between the two phase fields from
ordinary cold execution. It can pass while the canonical guest still fails.

There is also an independently visible `$4010` timer-reload ordering hazard.
The protected DMC Reload Timing row deliberately measures that boundary; its
first wrong byte must be interpreted as one member of a timed pair, not as
generic DMA duration evidence.

## Source authority

- Preparation index and NES-HARDWARE-CPU-APU.md were read, along with Plan 21-04's
  approved prerequisite and checkpoint. Existing KB entries have no matching DMC
  diagnosis. No project-local skills or configured debugger skills were emitted.
- [Pinned AccuracyCoin assembly](https://raw.githubusercontent.com/100thCoin/AccuracyCoin/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm)
  was fetched directly in memory. SHA256:
  `540b524187a97d71018bb94a1b7874e19d37894e5882861fe0c31adafe251a9f`.
  The local permitted assembly's relevant sections agree. Do not trust line
  positions returned by a stale web cache for the short commit URL.
- [MOS hardware manual](https://ftpmirror.your.org/pub/misc/bitsavers/components/mosTechnology/6500-10A_MCS6500hwMan_Jan76.pdf),
  section 1.4.1.2.8, establishes phi1 RDY changes, phi2 recognition and the write
  exception. This does not itself locate a 2A03 DMC output event within phi1/phi2.
- [Breaking NES DPCM circuit analysis](https://raw.githubusercontent.com/emu-russia/breaks/master/BreakingNESWiki_DeepL/APU/dpcm.md)
  identifies ACLK-controlled counters/control, CPU PHI1 read detection and sample
  loading, and separate DMCRDY/address-mux outputs. The prose alone does not prove
  a one-CPU-cycle request-to-halt delay.
- [NESdev DMA](https://www.nesdev.org/wiki/DMA) is the preparation file's curated
  working specification: first load initially attempts GET, reload initially
  attempts PUT, dummy then optional alignment then data GET, and failed halt
  attempts retry on the next CPU cycle. It is secondary hardware evidence, not
  an independent transistor-level proof.

## Exact phase invariant

Observed source: `cycle()` toggles `bus.apu_get_put_phase` before APU work
(`src/bus.c:64`); `apu_clock()` independently toggles `pulse_clock_phase`
(`src/apu.c:209`). Both fields start zero (`src/instance.c:81-82`,
`src/cartridge.c:344-345`), and soft reset does not change either independently.
Every production APU clock is reached through that bus cycle seam.

Derived invariant for ordinary cold execution, excluding deliberately injected
fixtures and loaded histories: the fields remain equal. DMC output work currently
runs when `pulse_clock_phase == 0` (`src/apu.c:260`), thus on bus PUT. The comment
at apu.c:255 describing an offset from the pulse edge does not match the two
identical `pulse_clock_phase == 0` conditions at apu.c:210 and :260.

Observed fixture: `test_dmc_reload_supplies_jsr_target_opcode` sets bus phase 1,
pulse phase 0 (`tests/core/test_interrupt_entry.c:328-329`). Six CPU cycles later,
the event occurs on bus GET, not PUT. This XOR-one phase history is not obtained
from the ordinary cold path. A green result here cannot establish canonical E's
request timing. This is a fixture limitation, not permission to reject historical
native states or change state format/revision.

With the inspected post-clock qualification (`src/bus.c:175-178`), the first
`cycle()` has already toggled the field. It describes the cycle just performed;
the argument named `phase_at_read_open` is actually that post-toggle value.

| Halt cycle | Dummy | Alignment | Data access | Resumed CPU read | Total bus-call cycles |
|---|---|---|---|---|---|
| GET (load) | PUT | omitted | GET | PUT | 4 |
| PUT (reload) | GET | PUT | GET | PUT | 5 |

This follows directly from `dmc_dma()` at bus.c:134-152 and the final resumed
`cycle()` at :182. Under these conditions adding `|| dma_halt_phase == 0` to
the alignment condition is redundant: the reload dummy is already GET. With
opening-phase qualification, the same added condition can force data onto PUT.
Therefore cycle count alone is insufficient: assert the phase at the actual
sample-buffer read, not merely the final phase or elapsed ticks.

A further existing limitation is visible: qualifying every retry by
`dma_halt_phase` prevents a reload delayed by one write from halting on the next
GET for a three-cycle DMA. The working specification permits that retry. This
audit does not establish that this separate limitation causes canonical E; do
not expand into a general DMA cluster on that basis.

## Protected timer-window oracle

Pinned source lines 8677-8778 describe six writes around one timer reload and
two open-bus probes per iteration. The physical test's expected byte sequence is
`40 00 40 00 40 00 40 00 00 40 00 40`.

| Addresses | Write relative to timer reload | Expected fast/slow probe bytes |
|---|---|---|
| 0500/0501 | -3 CPU cycles | 40 / 00 |
| 0502/0503 | -2 | 40 / 00 |
| 0504/0505 | -1 | 40 / 00 |
| 0506/0507 | same cycle | 40 / 00 |
| 0508/0509 | +1 | 00 / 40 |
| 050A/050B | +2 | 00 / 40 |

The source expressly warns that its internal request-to-DMA delay was unknown;
the programmed offsets and hardware answer key remain the oracle. A moved DMA
probe can change one byte without changing which timer value won. Measure both
the timer reload and both probe accesses before assigning cause.

Observed code: a production write runs `cycle()` before decode
(`src/bus.c:222`); DMC reload copies `dmc.timer` into `counter`
(`src/apu.c:269-273`); only later does `$4010` update `timer`
(`src/apu.c:476-481`). Thus a reload performed inside that same modeled write
cycle necessarily selects the old period. Pinned iteration 3 expects the new
period. This is a definite model-order contradiction when the two events are
in that same internal cycle. Whether a particular retained 0506 discrepancy
actually exercises that exact conjunction requires the executor's edge trace.

## Historical falsifiable checks (superseded by the final source-bound evidence below)

1. Repeat the owned JSR case with the production-reachable equal phase pair and
   schedule request creation specifically on C6. Record opening/closing bus and
   pulse phase, pending before/after, counter, bits, actual DMA address/phase and
   resumed CPU address/value. Distinguish request eligibility from GET retention.
2. For both a GET and PUT start, assert the full phase sequence in the table,
   including the actual mapper sample read. Add one-write-delayed reload only if
   the canonical trace proves retry is part of this bounded prerequisite.
3. Construct a narrow production `$4010` write with old half-period 27, new 214,
   and counter at the same-edge reload boundary, plus neighbor writes one cycle
   earlier/later. Observe the counter's loaded period separately from output and
   DMA. The tie should select 214; the late neighbor should retain 27 for the
   current interval. Do not flip global APU phase merely to shift this window.
4. For each canonical protected iteration retain loaded timer value, next
   buffer-empty event, initial request phase, actual halt, actual GET, and both
   `$4000` probe values. If timer selection stays correct while one probe moves,
   the fault is landing, not timer reload ordering. If tie loads 27, the timer
   write order is independently wrong.

No complete minimum runtime repair is proven by this read-only audit. Preserve
all 114 protected PASS keys, pin/cap/tables, prepared entry work and the single
green commit boundary. Historical variant labels and withdrawn stale-runner
claims are not evidence. The executor's fresh source-bound matrix must decide
which small correction is justified.

## Historical implementation snapshot: root causes and bounded repair

The first audit's request-edge alternatives were not the causal repair.
Fresh baseline tracing found six eligible reload attempts blocked by CPU stack
writes in E's timed prefix (cycles 2381138, 2382866, 2391938, 2400146,
2401010, 2402738). The old core then waited for the original PUT phase instead
of retrying on the next CPU cycle. That shifted progress through the guest's
clockslides and put the eventual DMA on C6's mapped `$40` operand. Advancing
`dma_halt_phase` after each eligible write implements the documented retry
without changing the initial load/reload phases, timer clock or GET alignment.

The corrected prefix makes canonical latency, BRK overlap and IRQ overlap pass.
It also lets Implied Dummy Reads advance past its original `0x0e` failure.
That newly reached path exposes a second existing decoder defect: controller
D6 was forced high. Pinned source lines 11660-11815 explicitly depend on
JSR `$4015` reading `$21` and `$00` controller operands from stack-driven open
bus. The old decoder forms `$4061` instead of `$0021`; its unintended RTI
corrupts the guest stack and eventually resets page15, erasing protected rows.
Retaining all three upper bits fixes that adjacent open-bus prerequisite. The
owner confirmed this exact one-line correction remains within the bounded scope.

No APU implementation, timer phase, `$4010` write ordering, `$4015` latch
behavior, open-bus GET retention, pin, cap or source answer table changed.
The final runtime changes are the write-side retry and controller D6 retention,
alongside the preexisting prepared shared-entry change.

Owned regressions cover initial GET and PUT attempts delayed by one, two and
three writes; both controller ports, both serial data values and all eight
upper-bit patterns; equal bus/APU phase history for the source-shaped JSR; and
the actual branch C1 DMA followed by C3 `$4015` acknowledgement and C4 clear.
The original mapped-resume `$5A` versus `$EA` control remains passing.

Independent reverts are retained in `build/dmc-repair-evidence/`:

| Exact change removed | Owned regression | Canonical observation |
|---|---|---|
| Retry only, retain controller repair | core.apu fails | page12 latency03/frame3; overlaps unstarted |
| Controller retention only, retain retry | core.controller fails | page15 guest resets; seven rows unstarted at cap1800 |
| Both fixes reapplied | focused four suites pass | complete campaign119PASS/27FAIL;146terminal;5DRAW |

The campaign retains every one of the starting114 PASS keys and passes all11
named v5 targets. Exactly five keys improve: DMA + `$4015` Read, Implied Dummy
Reads, Interrupt flag latency, NMI Overlap BRK and NMI Overlap IRQ. Page12's three
rows are raw01/frame148; DMC Reload Timing remains raw01 (page15 frame128);
Delta Modulation Channel remains raw01 (page14 frame122).

Fresh build: `build/dmc-repair`, configured Release/Ninja with warnings as
errors and `NESTURBATOR_ACCURACYCOIN_FULL=ON`. Canonical launch arguments are
owned `tests/cmake/accuracycoin_campaign.cmake`, boot32/navigation5/execution1800.
The complete command is registered as `accuracycoin.campaign` in that build.
Final campaign report SHA256:
`56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`.
Its56-input fingerprint at this intermediate refresh was
`7936186b09988b79b502e9e6382d2895343d6f35adc7f8a9782a99efa8079e91`.
Final source/binary/report digests and exact-key gates are retained in
`build/dmc-repair-evidence/final-campaign/identity.json` and `gates.json`.
Every intermediate diagnostic/variant directory retains exact source copies,
runner hash, command arguments and raw/report outputs. All diagnostic code was
removed; `src/apu.c` and `runner/accuracycoin.c` are unchanged.

After the independent reverts and final reapplication,
`ctest --test-dir build/dmc-repair -j 1 -R '^(core\..*|cpu\.unit|bus\.unit|ppu\.nmi_races|fuzz\.state.*|accuracycoin\.campaign)$' --output-on-failure`
passed 42/42 in 173.81 seconds. This includes state-fork stress, state rejection,
run-ahead, native seeds, CPU/bus and interrupt controls, plus a fresh full
campaign. `build/dmc-repair-evidence/final-focused.log` retains the result.
Every final source/binary/report identity was rechecked afterward: zero
mismatches. The repair remains uncommitted for the executor's coherent closeout.

At this investigation checkpoint, Plan21-04 still needed its coherent closeout.
The final refresh and validation are recorded below.

## Final source-bound closeout evidence (2026-10-29)

The candidate-count fixture update was followed by a fresh full campaign on
source revision `a23394036f71`, behavior revision 9, pin
`6bee919529289c555803b612b8c9e4f7eaaeeabb`, and caps `32/5/1800`. It completed
22/22 pages and 146/146 terminal scored rows: 119 PASS, 27 FAIL, zero
not-run/skip/timeout, and five inventory-only DRAW rows. All eleven v5 target
rows and all 114 protected starting PASS keys pass. The report SHA-256 is
`56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`; the
current 56-input fingerprint is
`c1aeeddca9b0b5a2641d66ffa9309998dfb67bb620cb1cbb94d6c59534865283` and
guest elapsed time is 4.917281 seconds. The final run artifacts are retained in
`build/dmc-repair/accuracycoin/full/` and
`build/dmc-repair-evidence/final-campaign/`.

Fresh protected-main authority is commit
`a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`; its projected baseline has exactly
114 PASS keys. Independent source/binary/report identity checks covered all 12
entries in `identity.json` with zero mismatches. `runner.write_hashes` and its
content comparison passed, so frame, PCM and APU-transition hashes are stable;
state hash and size changes remain the four-byte format3/revision9 continuation
movement measured above. Regeneration confirmed all 17 state seed entries and
manifest rows unchanged. Six page-14 migration links were refreshed to the new
measured frame-122 rows. The retry and controller fixes retain their independent
revert evidence; no APU timer order, global phase, GET retention, ROM pin, cap or
answer table changed.

The final `cmake --workflow --preset ci` passed 691/691 tests in 358.64 seconds
and generated all three packages. The local Plan21-04 gates are complete;
hosted all-lane evidence remains pending the later Phase21 acceptance work.
