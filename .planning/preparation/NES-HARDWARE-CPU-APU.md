---
title: "NES hardware: CPU, bus, DMA and APU"
summary: "Where 2A03 CPU, bus, DMA, interrupt and APU behaviour is documented, which cases are hard, and which public tests show an implementation is right."
read_when: "Open when designing or building the CPU core, bus timing, DMA, interrupts, the APU or the audio synthesiser."
updated: 2026-10-02
---

# NES hardware: CPU, bus, DMA and APU

## Why this file exists
2A03 behaviour is documented across a wiki, test-ROM sources, circuit write-ups and forum threads, which disagree in places. This file maps topics to sources, states the facts that constrain the architecture, and names the public test for each hard case. "(derived)" marks inference from cited facts; "(unverified)" marks recall.

## 1. Source map
|Topic|Sources, best first|
|---|---|
|6502 per-cycle bus|[HWC.24] cycle tables; [HWC.20] vectors: 10,000 cases per opcode, all-RAM, made by an unnamed emulator [HWC.21], no interrupts or DMA|
|Unofficial opcodes|[HWC.26] (located, not read); [HWC.12]|
|Power-on, reset|[HWC.22]; [HWC.05] (one RP2A03G console); [HWC.19]|
|Interrupts|[HWC.17]; [HWC.02]; [HWC.22]|
|Bus latch, open bus, $4016/$4017|[HWC.19] source notes; [HWC.06]; [HWC.07]|
|DMA|[HWC.25] circuit; [HWC.01] working specification|
|Clocks, regions|[HWC.25]; [HWC.03]; [HWC.04]; [HWC.13]|
|APU|[HWC.23] (tests on a 2A03G); [HWC.25]; [HWC.32]; [HWC.08]-[HWC.11]|
|Tests|[HWC.19] (144 tests, for RP2A03G and RP2C02G); [HWC.22]; [HWC.15]|

## 2. Facts that shape the architecture
### 2.1 Time granularity
- NTSC: CPU = master clock / 12, PPU dot = master / 4; each CPU cycle is one read or one write [HWC.04]. A cycle starts where M2 falls; M2 is high for its last 15/24 (17/24 on the letterless RP2A03, 19/32 on the 2A07); data moves during phi2, the second half [HWC.03].
- Sub-cycle cases: a $2002 read takes the VBL flag at M2 rise and the sprite flags at M2 fall, 7.5 master clocks later (AC $2002 Flag Timing [HWC.19]); NMI and IRQ are sampled during phi2 [HWC.02] (Lolo 2, Ms. Pac-Man and Spelunker need exact poll timing [HWC.16]).
- A cycle starts 0-3 master clocks before the next dot, fixed at power-up (at reset per [HWC.16]), moving results by up to a dot [HWC.14]. The APU get/put phase has two power-on states [HWC.01].
- Verdict: half-master-clock ticks are sufficient and the coarsest unit making every edge an integer (NTSC M2: 9 low, 15 high; PAL: 13, 19, half-dot 5) (derived). Stepping at that interval is more than needed: TriCNES steps whole master clocks [HWC.27], and 2A03-internal behaviour is per CPU cycle plus get/put phase [HWC.01].
- Recommendation: time in half-master-clock ticks, one bus call per CPU cycle, the PPU caught up to M2 rise and M2 fall inside the call. Alternative: stepping every master clock; a test passing only with rounded M2 edges, or a speed result favouring fixed steps, would change this.

### 2.2 DMA ([HWC.01] unless cited)
- RDY halts only on a read; writes delay it up to 3 cycles. The halted 2A03 repeats that read on the bus in every no-access DMA cycle, then once more when DMA ends; no interrupt is serviced meanwhile.
- DMA reads on get and writes on put cycles (halves of an APU cycle); a wrong-phase cycle is spent aligning. OAM DMA takes 513 or 514 cycles.
- DMC DMA: normally 3 cycles for a load (halt on a get cycle in the 2nd APU cycle after the $4015 write), 4 for a reload (halt on a put cycle); inside OAM DMA it costs 2, or 1 or 3 at the end. AccuracyCoin also accepts a load after 3 APU cycles [HWC.19].
- Bugs: a stop just before a reload gives a 1-cycle aborted DMA; RP2A03H and RP2A03G from 1990 can add an unexpected reload.
- Repeats re-read $2002, $2007, $4015. Controllers see one clock per run of contiguous reads (NES-001, AV Famicom) or one per cycle (RF Famicom). Register select uses address bits 4-0 from the bus and 15-5 from the core, so a DMA address ending $15-$17 hits that register while the core is parked in $4000-$401F.
- Verdict: DMA inside the read path matches. State needed: parked address, get/put phase, two address sources, internal and external data-bus latches ($4015 reads use only the internal one [HWC.08][HWC.19]), console type.

### 2.3 Interrupt polling ([HWC.02])
- NMI (edge) and IRQ (level) are sampled during phi2 of every cycle and polled in an instruction's last cycle, so the line state at the end of the second-to-last cycle decides [HWC.17].
- CLI, SEI and PLP change I after their poll (one instruction late); RTI before it.
- Branches poll before cycle 2, not before cycle 3 when taken, and again before a page-cross fix-up cycle.
- Interrupt sequences do not poll. NMI beats a simultaneous IRQ.
- The vector is chosen after cycle 4 of 7: an NMI in the first four cycles of BRK or IRQ takes over (BRK still pushes B set). Reset is the same sequence with writes suppressed.

### 2.4 Stopping the CPU mid-instruction
- DMA halts at any read cycle, also mid-instruction: SHA, SHX, SHY and SHS change result when RDY falls 2 cycles before their write [HWC.19], and a DMC DMA changes the open-bus value later cycles read [HWC.06]. No fetched source gives another reason to stop the core.
- Recommendation: an instruction-level core with one bus call per cycle and a "halted in this read" flag for SHx. Alternative: a per-cycle state machine (floooh/chips, unverified). A need to pause on an exact cycle would change this.

## 3. Hard-case index
AC = AccuracyCoin [HWC.19]; other names: [HWC.15], [HWC.22].

| Behaviour | Why hard | Documented | Public test |
|---|---|---|---|
|Dummy reads and writes|Register side effects (Cobra Triangle, Ironsword [HWC.16])|[HWC.24]|AC Dummy read cycles, Dummy write cycles, Implied Dummy Reads, Branch Dummy Reads; cpu_dummy_reads|
|Open bus, internal bus|$4015 off the external bus|[HWC.06][HWC.08]|AC Open Bus, Internal Data Bus|
|I-flag latency, branch polls, hijacking|Poll order; late vector choice|[HWC.02]|AC Interrupt Flag Latency, NMI Overlap BRK, NMI Overlap IRQ; cpu_interrupts_v2 ROMs 1, 2, 3, 5|
|DMA placement|Phase, write delays|[HWC.01]|AC Delta Modulation Channel, DMA + Open Bus, INC $4014; cpu_interrupts_v2 ROM 4|
|DMA during register reads|Repeated side effects|[HWC.01]|AC DMA + $2002 Read, + $2007 Read, + $2007 Write, + $4015 Read, + $4016 Read; dmc_dma_during_read4; read_joy3|
|DMA overlap, abort, activation|Chip date; split decode|[HWC.01]|AC DMC DMA + OAM DMA, Explicit DMA Abort, Implicit DMA Abort, APU Register Activation, DMC DMA Bus Conflicts; sprdma_and_dmc_dma|
|Controller strobe, clock|Get/put phase; console type|[HWC.01][HWC.07]|AC Controller Strobing, Controller Clocking|
|Frame counter, IRQ flag, length counter|Phase-dependent delays; $4015 race; same-cycle halt and reload|[HWC.09][HWC.22]|AC Frame Counter IRQ, 4-step, 5-step; blargg_apu_2005.07.30; apu_test|
|Power, reset|$4017 acts as if written 9-12 cycles before the first instruction|[HWC.22]|apu_reset; cpu_reset|
|Unstable opcodes|Chip-dependent constants; RDY|[HWC.26]|AC pages 3-11; instr_test-v5|

## 4. APU
- The triangle timer runs every CPU cycle; pulse, noise and DMC timers every second cycle [HWC.08].
- Frame counter (NTSC): clocks on the put half of APU cycles 3728, 7456, 11185 and 14914, or 18640 in 5-step mode; IRQ flag set on three consecutive CPU cycles; period 29830 CPU cycles; $4017 writes act 3 or 4 cycles later by phase [HWC.09].
- Length counter: a halt change applies after a same-cycle clock; a reload during a clock is ignored unless the counter is zero [HWC.22].
- DMC: [HWC.10]. Fire Hawk, Mig 29 and Time Lord split the screen from its IRQ [HWC.16].
- Mixer [HWC.11]: the 203-entry table approximates the three-term formula within 4%; an exact 16 x 16 x 128 table avoids that (derived). Test: apu_mixer.
- Filters [HWC.11]: NES: first-order high-pass at 90 and 440 Hz, low-pass at 14 kHz; Famicom: only a 37 Hz high-pass before the RF path.
- Expansion audio is later work: [HWC.18].

## 5. Integer band-limited synthesis
- Literature: [HWC.28] impulse trains from a windowed-sinc table; [HWC.29] band-limited steps, minimum-phase variant, DC offset of floating-point sums; [HWC.30] fixed-point tables, Kaiser windows, interpolation bounds; [HWC.31] tutorial (code archive unopened: clean room).
- Recommendation: on each change of the mixed level, add delta times an interpolated kernel row to an integer buffer; a running sum is the output; three first-order filters follow. Kernel: symmetric Kaiser-windowed sinc (beta 8, cutoff at Nyquist, support 15 samples), 16 taps, 32 phases, linear interpolation between rows. Rows hold one-sample integrals of the kernel in Q15 and sum to exactly 32768 (the interpolation residue goes into one tap), so the running sum is exact with no leak. The table is a checked-in integer array (libm differs between platforms). The NTSC CPU clock is 236.25 MHz / 132 [HWC.04], so 48 kHz is exactly 352 samples per 13125 CPU cycles (derived). Alternative: the minimum-phase step [HWC.29], if latency under 8 samples is needed.
- Prototype (pure Python, this session): worst non-harmonic component below 16 kHz -87 dB against the fundamental; harmonics within 0.01 dB of ideal to 14 kHz. Traps: point-sampled rows boost 12 kHz by 0.9 dB; a kernel as wide as the tap count leaves a -74 dB floor; lookup without interpolation needs about 1024 phases.
- Spectral test: render pulses at timer periods 100, 40, 12, 8 and a triangle at period 1; from 32768 windowed samples report the largest non-harmonic bin below 16 kHz, harmonic amplitudes against 1/k, exact return of the integer level, and output hashes across platforms.

## 6. Regions
||NTSC 2A03|PAL 2A07|Dendy UA6527P|
|---|---|---|---|
|Master clock; CPU divider|21.477272 MHz; 12|26.601712 MHz; 16|as PAL; 15|
|M2 high|15/24|19/32|not found|
|Frame counter steps|3728, 7456, 11185, 14914|4156, 8313, 12469, 16626|rate about 59 Hz|
|Noise, DMC tables|NTSC|own tables|NTSC DMC table|
|DMA extra reads|present|removed|DMC DMA bugs present, 1 APU cycle later|
|Pulse duty|normal|normal|swapped on many clones|

From [HWC.01], [HWC.03], [HWC.04], [HWC.08]-[HWC.10], [HWC.13]. Unofficial opcodes behave alike on the 2A03 and 2A07 [HWC.04].

## 7. Unofficial and unstable opcodes
- ANE ($8B), LXA ($AB): the first 1,200 vectors of each fit only constant $EE [HWC.20]; an open issue says test ROMs need $FF for LXA [HWC.21]; AccuracyCoin tests only operands where the constant cannot matter [HWC.19]. If that report holds, no constant passes both vectors and ROMs.
- SHA, SHX, SHY, SHS: sampled vectors store register AND (high byte + 1) and on a page cross use that value as the target high byte [HWC.20]; AccuracyCoin accepts three high-byte behaviours for SHA and SHS and adds the RDY case, which vectors cannot express [HWC.19].
- AccuracyCoin's names ASR, ANE, LXA, SHA, SHS, LAE are the wiki's ALR, XAA, LAX #i, AHX, TAS, LAS.
- Games use NOP variants, SLO $07, LAX $B3 and, in one multicart, XAA $8B [HWC.12].

## 8. Power-on state
- CPU: A, X, Y = 0, S = $FD, I set; reset sets I and lowers S by 3 without writing [HWC.05][HWC.22]. Consoles show noise in these values [HWC.19].
- APU: $4000-$4013, $4015, $4017 are 0 at power; reset clears $4015, ANDs $4011 with 1 and re-applies the last $4017 value (not on the letterless RP2A03) [HWC.05][HWC.13][HWC.22].
- RAM is unreliable; observed fills: F0/0F blocks, 00/FF blocks, all 00, all FF [HWC.19].
- Games [HWC.16]: Final Fantasy and others seed RNGs from RAM; Minna no Taabou needs $11 non-zero; Huang Di cheats when $0100 is 0; Dancing Blocks fails when $EC and $ED are $FF; Terminator 2 skips its copyright screen on all-zero RAM.
- Recommendation: a fixed non-uniform default fill meeting those four constraints, with all-00, all-FF and seeded fills selectable. Alternative: all-00. A compatibility run over these games would change this.

## Open questions
- $4017 write delay: 3 or 4 cycles [HWC.09] against 2 or 3 [HWC.08]; AC Frame Counter IRQ codes A-D settle it.
- Noise LFSR at power: 0 [HWC.05] against 1 [HWC.08]; a Visual 2A03 reset trace [HWC.32] settles it.
- 2A07 DMA fix: unknown [HWC.01] against halt on an instruction's first cycle [HWC.10]; a PAL hardware test settles it.
- Whether whole-master-clock M2 edges [HWC.27] change any test result: run both.

## Sources
Wiki = NESdev Wiki, https://www.nesdev.org/w/index.php?oldid=REV. All accessed 2026-10-02.

| ID | Tier | Source | Pin or access date | Supports |
|---|---|---|---|---|
|HWC.01|T2|Wiki: DMA|rev 23450|DMA|
|HWC.02|T2|Wiki: CPU interrupts|rev 21632|Polling|
|HWC.03|T2|Wiki: CPU pinout|rev 22943|M2, phi2|
|HWC.04|T2|Wiki: Cycle reference chart; CPU|rev 22030; 24046|Clocks|
|HWC.05|T2|Wiki: CPU power up state|rev 22091|Power, reset|
|HWC.06|T2|Wiki: Open bus behavior|rev 23814|Bus latch|
|HWC.07|T2|Wiki: Controller reading; Standard controller|rev 23772; 23466|Ports|
|HWC.08|T2|Wiki: APU and unit pages (Pulse 24334, Triangle 24340, Noise 24339, Envelope 23446, Sweep 24338, Length Counter 24331)|rev 23811|Channels|
|HWC.09|T2|Wiki: APU Frame Counter|rev 23448|Sequencer|
|HWC.10|T2|Wiki: APU DMC|rev 24164|DMC|
|HWC.11|T2|Wiki: APU Mixer|rev 23451|Mixer|
|HWC.12|T2|Wiki: CPU unofficial opcodes|rev 23975|Opcodes|
|HWC.13|T2|Wiki: CPU variants|rev 24230|Revisions|
|HWC.14|T2|Wiki: PPU frame timing|rev 19961|Alignment|
|HWC.15|T2|Wiki: Emulator tests|rev 23774|Test names|
|HWC.16|T2|Wiki: Tricky-to-emulate games; Game bugs|rev 23875; 24031|Games|
|HWC.17|T1|Wiki mirror of Visual6502: 6502 Interrupt Recognition Stages and Tolerances|rev 19267|Interrupts|
|HWC.18|T2|Wiki: Category:Expansion audio (not read)|rev 18360|Pointers|
|HWC.19|T1|AccuracyCoin README.md and .asm, https://github.com/100thCoin/AccuracyCoin|commit 673ef55|Tests|
|HWC.20|T1 files, T3 content|READMEs, nes6502/v1 samples, https://github.com/SingleStepTests/65x02|commit 2f6980a|Vectors|
|HWC.21|T4|Same repository, issues 2, 11|2026-10-02|Divergences|
|HWC.22|T1|blargg readmes and file tree, https://github.com/christopherpow/nes-test-roms|commit 95d8f62|Test intent|
|HWC.23|T1|blargg, APU reference (header read), https://www.nesdev.org/apu_ref.txt|2026-10-02|APU model|
|HWC.24|T2|64doc v1.8 (contents read), https://www.nesdev.org/6502_cpu.txt|2026-10-02|Cycle tables|
|HWC.25|T1|BreakingNESWiki_DeepL/APU, https://github.com/emu-russia/breaks|commit 5a55bac|Circuits|
|HWC.26|T2|No More Secrets V1.0, https://csdb.dk/release/?id=258111|2026-10-02|Opcodes|
|HWC.27|T3|TriCNES Emulator.cs, https://github.com/100thCoin/TriCNES|commit 94f1b11|Stepping|
|HWC.28|T1|Stilson and Smith, ICMC 1996, https://ccrma.stanford.edu/~stilti/papers/blit.pdf|2026-10-02|Impulse trains|
|HWC.29|T1|Brandt 2001, https://www.cs.cmu.edu/~eli/papers/icmc01-hardsync.pdf|2026-10-02|Steps|
|HWC.30|T1|J. O. Smith, resampling pages, https://ccrma.stanford.edu/~jos/resample/|2026-10-02|Sinc tables|
|HWC.31|T3|blargg, Band-Limited Sound Synthesis, http://www.slack.net/~ant/bl-synth/|2026-10-02|Tutorial|
|HWC.32|T1|Visual 2A03 (not run), https://www.qmtpro.com/~nes/chipimages/visual2a03/|2026-10-02|Simulator|
