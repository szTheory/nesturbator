---
title: "Phase 20: independent NMI capture oracle"
summary: "Published die-netlist pulse experiments establish nonzero-margin detector behavior, separate M2 from internal phi2, and expose the limit of the old exact-tie A/C comparison."
read_when: "Before resuming Plan 20-02's detector-order checkpoint."
updated: 2026-10-26
status: researched
---

# Independent NMI capture oracle

## Verdict

An independent, executable **switch-level circuit oracle** is available and
was exercised successfully: six fresh-state pulse cases against the published
Visual2A03 die network. It establishes the detector's behavior with nonzero
setup/hold margin and supplies ordinary expected outcomes for owned tests.
It does **not** establish which chip wins a zero-margin simultaneous PPU/CPU
edge on a physical console. The old A/C experiment supplies no retained
candidate source and its distinguishing start point places VBlank exactly
at the integer M2-fall boundary. Consequently, this research does not select
A or C merely from that comparison. No acceptance rule is relaxed here.

The useful replacement question is: what signal does the CPU input latch
retain at the end of its internal phi2 window, when was the PPU's physical
output valid, and what is latched by the next phi1 stage? A generic extra
whole-cycle delay and an arbitrary same-timestamp execution priority are
neither an exhaustive set of hardware models nor equivalent descriptions
of these circuit stages. This is an inference from the circuit and measured
waveforms below, not an observed NES console result.

## Primary circuit evidence

The [published Visual2A03 site](https://www.qmtpro.com/~nes/chipimages/visual2a03/)
provides a transistor network, node names, pin controls and a general
switch-level simulator. The site identifies its upstream source as
[Visual6502](https://github.com/trebonian/visual6502), whose README describes
the original rev-D 6502 die data. The experiment below used the **2A03 site's
own data**, not the standalone 6502 data or another NES emulator's behavior.

The site's [node names](https://www.qmtpro.com/~nes/chipimages/visual2a03/nodenames.js)
distinguish external `nmi` pad 10331, core `c_nmi` 1297, external `phi2`
alias 10743 (M2), internal `cclk` 943 (phi2), and `cp1` 710 (phi1).
Its detector names distinguish `NMIP` 1032, its complement 297, active-low
`~NMIG` 264, and `NMIL` 1374. This matters: the alias `phi2` in the site's
UI is the widened output M2, whereas `cclk` names the internal sampling
phase. Those two rising edges are not simultaneous in the measurement.

The [published transistor data](https://www.qmtpro.com/~nes/chipimages/visual2a03/transdefs.js)
contains the core input inverter at t2588, phase-qualified NMIP latch paths
through t1187/t14 and t1281/t1229, cross-coupled storage at t2151/t2610, and
the phi1 path to the next NMI stage at t1653. This is a circuit-level basis
for checking separate input-latch and retained-request stages. The concrete
outcomes below are measured using the unmodified published simulator;
they are not inferred from the current Nesturbator implementation.

The [Visual6502 interrupt-hijacking analysis](https://www.nesdev.org/wiki/Visual6502wiki/6502_Interrupt_Hijacking)
is also primary circuit research: it describes drawing the transistor
network and conducting simulator experiments, including vector-fetch
inhibition of NMI recognition. That is why this probe runs an ordinary
NOP/JMP loop and waits for startup to finish before injecting pulses;
its conclusions do not cover interrupt-entry/hijack windows. That page's
direct fetch was refused, but its indexed full text was retrievable.

## Executed experiment

Execution date: 2026-10-26. Runtime: Node v26.10.0. Each case initializes
a fresh VM and the site's original network. Six published files are fetched
over HTTPS into memory, passed over stdin as JSON, and evaluated without
editing their contents. No third-party source/data is copied into the
repository. The original `setupNodes`, `setupTransistors`, `initChip`,
`setLow`, `setHigh`, `handleBusRead` and `handleBusWrite` perform circuit
work. Graphics/status methods are replaced with no-ops only. The owned
driver toggles the published master-clock pin and services its bus helpers
when the published divider changes `clk0`.

The owned program is two NOPs and a JMP loop at `$8000`. Its NMI handler
increments RAM `$02` and executes RTI at `$9000`; reset/NMI vectors point
there. IRQ remains deasserted. The trace starts after 2400 master half-ticks
following the site's initialization. The counter at 3700 is checked, so
this tests eventual NMI delivery independently of a tight CPU poll cutoff.

Measured edges in the reference interval:

| Event | Master half-tick |
|---|---:|
| Internal phi2/M2 fall, phi1 begins | 2428 |
| M2 rises while internal phi2 is still low | 2437 |
| Internal phi2 rises | 2440 |
| Internal phi2/M2 fall, phi1 begins | 2452 |

One CPU period is 24 half-ticks. M2 is high for 15, internal phi2 for 12.
In this network's settled values, M2 falls with internal phi2, but rises
three half-ticks earlier. This is a measured digital-model result, not a
scope measurement of analog pad delay.

Pulse changes happen **after** the specified clock half-tick settles.
All selected pin transitions have nonzero margin from the relevant internal
phase boundaries; the exact coincident boundary is excluded.

| Case | NMI low -> high, master half-ticks | Expected and measured handler count | Stage observation |
|---|---|---:|---|
| Before fall | 2424 -> 2432 | 1 | NMIP sets during phi2; `~NMIG` asserts at 2428, with four half-ticks setup and four hold |
| After fall, short | 2432 -> 2436 | 0 | Pulse lies wholly in phi1; NMIP and `~NMIG` stay unasserted |
| Phi2-only short | 2420 -> 2424 | 0 | NMIP sets then clears before phi2 closes; `~NMIG` never asserts |
| Widened-M2-only | 2438 -> 2439 | 0 | M2 is high, internal phi2 remains low; no capture |
| After fall, long | 2432 -> 2456 | 1 | NMIP sets when phi2 opens at 2440; `~NMIG` asserts at 2452 |
| No pulse | none | 0 | Negative control |

All six assertions passed. In the first case, pin release in phi1 does not
clear NMIP immediately; NMIP clears on the following phi2 while `~NMIG`
retains the recognized request and the handler eventually executes once.
The transient-phi2 case shows why asynchronous pin-edge detection alone is
too strong. The widened-M2 control shows why sampling throughout M2-high
is not the same circuit as sampling throughout internal phi2.

The corresponding code-level test oracle is thus precise for a waveform
with margin: low across the internal phi2-to-phi1 transition produces one
eventual request, a pulse entirely in phi1 produces none, and a pulse
that returns high before the end of phi2 produces none. The local core
must map its PPU output timing into this window before these outcomes can
classify its A/C candidates.

## Limits and next discriminating check

The parent mapped the old fixture's clocks: PPU events advance every eight
half-ticks; the start at `240:330` reaches `241:1` after 96 half-ticks,
exactly the fourth bus cycle's integer M2 fall. This is a project-code
derivation supplied by the parent, not a circuit measurement. Re-running
that same logical tie with an imposed event order would measure the imposed
order, not independently settle real setup/hold or inter-chip propagation.

The published simulator is a settled switch-level model. It supplies no
analog setup/hold time or board-wire propagation delay for a PPU output
changing exactly as CPU phi2 closes. Do not turn its deliberate pin-before-
clock versus pin-after-clock evaluation order into a physical console claim.
It does not include a PPU in this experiment. No PPU flag, `$2002` read,
register side effect, or A/C production variant was simulated here.

The next useful owned check can preserve the ordinary core bus contract:
start from a documented waveform with a four-half-tick margin, distinguish
pin/NMIP/request/poll histories, and verify eventual delivery after a roomy
instruction sequence. A wrong model that misses this nonzero-margin pulse
or captures either short negative control is rejected independently by this
netlist oracle. If A and C differ only at the deliberately exact tie, these
cases correctly leave that tie unresolved; the remaining claim needs a
source-backed PPU-output-to-CPU-input schedule or an actual paired-chip
measurement. Do not invent an unmeasured universal timing shift.

## Independent real-console test route and its correction

The pinned [VBL/NMI test readme](https://raw.githubusercontent.com/christopherpow/nes-test-roms/95d8f621ae55cee0d09b91519a8989ae0e64753b/vbl_nmi_timing/readme.txt)
reports tests passing on an actual NES, with a divider-alignment caveat,
and documents NMI suppression/disable cases. However, the test author's
[2006-09-12 correction](https://forums.nesdev.org/viewtopic.php?start=15&t=1892)
states that the disable error descriptions are off by one PPU clock while
the tests themselves remain valid. Therefore the older readme's absolute
zero/one/two-clock descriptions are not a safe independent cutoff oracle.
This correction was retrieved from the primary forum post's indexed text.

The newer pinned [PPU VBL/NMI readme](https://raw.githubusercontent.com/christopherpow/nes-test-roms/95d8f621ae55cee0d09b91519a8989ae0e64753b/ppu_vbl_nmi/readme.txt)
defines exact self-synchronized output tables for `05-nmi_timing`,
`06-suppression`, `07-nmi_on_timing` and `08-nmi_off_timing`, including
status-seen/NMI-missed outcomes. These can support an independently derived
owned guest test only after mapping their synchronization and bus-sampling
coordinates; table indices are not absolute `241:dot` values. No source or
ROM from those suites was vendored or run in this research.

## Reproduction and provenance

The owned [driver](probes/20-nmi-netlist-oracle.cjs) and exact
[raw trace](probes/20-nmi-netlist-oracle.raw.txt) are archived with this
research. The raw trace mixes tab-separated waveform rows with case labels and source
hashes; the `.txt` extension preserves that original mixed-format output.
These owned artifacts are distinct from the remotely loaded third-party
simulator/data. The library has no new
dependency and no implementation source was changed.

| Artifact | SHA-256 |
|---|---|
| Owned driver `nesturbator-nmi-netlist-oracle.cjs` | `9643f6f1d6574ac82250f2d0fd34f56db87b4de4efd39e900070a231aa5299d2` |
| Full output `20-nmi-netlist-oracle.raw.txt` | `5d1abd0c34ccd0aafd47988a74bcd5dfdeb11c8841e37f22e7e87fa6d3a02bc1` |

Published file SHA-256 values, fetched from the Visual2A03 URL prefix above:

| File | SHA-256 |
|---|---|
| `segdefs.js` | `fe34a098ec64ee9049a0e083ec337e4d5228f0d338e1c64859c6bf2b1b2e7197` |
| `transdefs.js` | `b10ce2c7c5b9774bb4f27476cb3e44a1ba1b290e8a463df6957803950cf44607` |
| `nodenames.js` | `e88eeec604234012a82beb172145426c2391a06cf618c8a5f8ab32c872840679` |
| `wires.js` | `dded17b4e9e0752a450e1f684925c2bf67ece24977006a75eaaa43325b5d6387` |
| `chipsim.js` | `3fe3fa48a003704ef66346e042adb016c0b3bbee018e267edb4ef921ba9cc2a1` |
| `macros.js` | `c8ec996fbad9434f84b4b829f20797df0cceae0828d87e298ca28352dfd1e8e5` |

The simulator/helper source headers contain the MIT permission notice. The
data files did not expose an equivalent top-of-file notice in the inspected
headers; no redistribution right for those data files is asserted. They
are consumed in memory from the existing public simulator, not retained as
project assets. No GPL/LGPL emulator source was opened. Direct NESdev fetches
were often HTTP 403; primary indexed text was used where stated. No actual
console or analog scope measurement was performed.

### Confidence

- High: the six recorded outcomes of the pinned-input die simulation and
  the external-M2/internal-phi2 distinction in its waveform.
- High: the existing exact-tie owned comparison cannot establish analog
  inter-chip order by observing only this emulator's candidate outputs.
- Medium: mapping the measured circuit stages to the current core's two
  synchronous bus callbacks without additional persistent input history.
- Unresolved: physical paired-chip precedence at the old zero-margin tie.

### Owned reproduction driver

Run the hash-checked research command from the repository root:

```sh
python3 .planning/phases/20-nmi-line-and-vblank-flag/probes/20-nmi-netlist-oracle-fetch.py
```

The owned [fetch wrapper](probes/20-nmi-netlist-oracle-fetch.py) verifies
all six input hashes in memory before running the archived driver, then
requires the exact output hash above. It fails on a changed source, driver,
output, timeout or assertion. Python performs the HTTPS fetch because this
environment's Node fetch could not reach the site. No third-party source or
netlist is saved to disk. Node and Python are research tools, not new library
or CI dependencies. The driver and raw trace hashes remain those of the
original experiment.
