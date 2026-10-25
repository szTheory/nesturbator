# Feature Landscape: v5 "NMI, VBlank and sprite-0 timing"

**Domain:** cycle-accurate NTSC NES core; the 11 AccuracyCoin rows at pin `6bee919` plus the JAM tune-up
**Researched:** 2026-10-25
**Overall confidence:** MEDIUM-HIGH. Test intent and error codes are read from the pinned AccuracyCoin assembly (HIGH). Hardware rules are from NESdev pages (MEDIUM, T2) and the project's own prep notes. "What nesturbator gets wrong" is derived from reading `src/cpu.c`, `src/bus.c`, `src/ppu.c` against those tests, not from a trace (MEDIUM; the sprite-0 root cause is not yet isolated, see Group 5).

## How to read an AccuracyCoin error code

`TEST_Fail` returns `(ErrorCode << 2) | 2`; `ErrorCode` starts at 1 and is incremented after each passing subtest. So raw `0x06` = subtest 1 failed, `0x0a` = subtest 2, `0x22` = subtest 8, `0x03` = still running (no verdict). Many NMI sweep tests have only one subtest (a sweep over PPU-dot offsets compared to an answer key), so `0x06` there says only "the sweep table differs", not where. (AccuracyCoin `TEST_Fail`, line 3377 of the asm at the pin.)

Almost every row here syncs to the PPU with `VblSync_Plus_A` (each loop is one full frame, 1 PPU dot later per frame) and compares results to a table that may be shifted by exactly one PPU dot (CPU/PPU clock alignment). nesturbator must produce a table equal to the expected one or the one shifted by one dot.

## Groups (requirement categories, named by visible outcome)

### Group 1: The $2002 read sees each flag at the right instant

| Row | Code | Meaning |
|---|---|---|
| `$2002 flag timing` | `0x06` = subtest 1 | Four reads on consecutive PPU dots around the pre-render clear (dot 1 of scanline 261). Expected top-3-bit results `E0,E0,80,00` (or alt `E0,80,80,00`): the sprite-0 and overflow bits already read clear while bit 7 still reads set. |

**Rule.** A `$2002` read latches the VBL flag when M2 rises, but returns the sprite-0 and overflow bits as they are when M2 falls. On a revision-G CPU M2 duty is 15/24, so the sprite bits are read 7.5 master clocks (about 1.875 PPU dots) after the VBL bit. All three flags clear on the same dot (dot 1, pre-render). Source: AccuracyCoin asm comment at `TEST_2002FlagTiming` (lines 1776-1793); prep note NES-HARDWARE-CPU-APU 2.1.
**What nesturbator gets wrong (derived).** `bus.c cycle()` already catches the PPU up to M2 rise and M2 fall, but `ppu_register_read` builds the whole status byte at one instant (after the M2-fall catch-up), so bits 7/6/5 clear together. Fix: snapshot bit 7 at M2 rise, take bits 6/5 at M2 fall.
**Subtest 2 (not yet reached):** sprite-0/overflow flag set timing, read at dot 0 of scanline 130-ish; expects `$53 == 00`, `$54 == 00 or 40` (alignment), `$55 == 60`. Expect a second round of failures after subtest 1 passes.
**Fixes:** `$2002 flag timing`. Also unblocks the shared `Sync_ToSpriteFlagsClearing` routine used by `$2004 Stress Test` and the sprite page (adjacent rows, not in v5 scope).
**Complexity:** Low for subtest 1, Medium for subtest 2. **Games:** none credibly named; this is precision, not game compatibility.

### Group 2: NMI is an edge that is latched, polled one cycle early, and not revocable

| Row | Code | Meaning |
|---|---|---|
| `NMI Control` | `0x22` = subtest 8 | Subtests 1-7 pass today (no NMI when off, NMI at VBlank, enabling during vblank with flag set, no NMI with flag cleared, no double NMI on `$80` rewrite, re-enable gives a second NMI). Subtest 8: `STA $2000` (enable) then `LDX #$10`; NMI must run after the LDX, not before. Subtest 9: `INC $2000` must give an NMI before the next instruction (polled between the RMW's two writes). |
| `NMI Timing` | `0x06` | Sweep the VBL set point by 1 dot over 10 frames; count how many `INY` run before the NMI handler. Expected `03,02,02,02,02,02,02,01,01,01,01` (or the same shifted one entry). |
| `NMI Suppression` | `0x06` | NMI already enabled; `LDA $2002` is placed on successive dots around the VBL set. Result `(NMI ran<<1)|flag read`. Expected `.,02,02,.,00,01,.,03,03,03` (`.` = either). Reading one dot before the set gives flag 0 and no NMI; on the set dot (and about one dot later) flag 1 and no NMI; two or more dots later flag 1 and the NMI still runs. |
| `NMI at VBlank end` | `0x06` | `$2000=$80` written on dots before/after VBL clears (scanline 261 dot 1). Expected `01,01,01,00,00,00,00`: an enable that lands 3 or fewer dots before the clear still produces an NMI; later ones do not. Done twice, with the NMI taken after a 4-cycle `PLA` and after a 2-cycle `NOP`; both must give the same table. |
| `NMI disabled at VBlank` | `0x06` | `$2000` bit 7 cleared on dots around VBL set. Expected `00,00,00,.,01,01,01`: a disable just after the set still lets the already-latched NMI through. |

**Rules (cycle level).**
- /NMI is low while `vblank_flag AND ctrl.7`. The CPU edge detector samples the line during phi2 of every cycle and raises its internal signal in the next cycle's phi1; once set it stays until serviced. Source: NESdev "CPU interrupts" (T2); visual6502 interrupt-recognition analysis [HWC.17].
- Interrupts are polled at the end of the second-to-last cycle of an instruction, so a signal that appears in an instruction's last cycle waits one full instruction. A `STA $2000` write lands in its last cycle, so the NMI follows the next instruction (this is what NMI Control subtest 8 tests; Lolo 2, Ms. Pac-Man and Spelunker depend on it, NESdev "Tricky-to-emulate games"). An RMW's dummy write happens in the second-to-last cycle, so `INC $2000` can be polled in time (subtest 9).
- A later write that clears ctrl.7, or the VBL clear at dot 1 of 261, cannot revoke an edge already latched. Only a `$2002` read landing at or within about two dots of the set suppresses it, because the line comes back high before phi2 samples it: read one dot before the set returns 0 and never sets the flag; read on the same dot or one later returns 1, clears it and suppresses the NMI; two or more dots later behaves normally. Source: NESdev "PPU frame timing"; "PPU registers" ("may even be suppressed by reads landing on the following dot or two").
- Enabling ctrl.7 (0 to 1) while the flag is set raises the line at once (PPU registers); toggling 1-0-1 in vblank gives a new edge (NMI Control subtest 7; Bases Loaded II relies on the extra NMI, NESdev "Tricky-to-emulate games").

**What nesturbator gets wrong (derived from `ppu.c`, `cpu.c`).**
1. `nmi_pending` is a single flag, taken at the next instruction start. There is no separate edge latch and no poll point, so a `$2000` write in the last cycle is taken one instruction too early (NMI Control 8). Fix: line level -> edge latch (phi2 sample) -> poll latch (end of second-to-last cycle) -> acted on at opcode fetch.
2. `$2002` reads clear `nmi_pending` unconditionally (`ppu.c` register read), and any `$2000` write with bit 7 clear clears it, and dot 1 of 261 clears it. All three revoke an NMI that real hardware has already latched (Suppression, VBL end, Disabled). Only the read at the set dot or within the ~2-dot window may suppress.
3. NMI Timing and Suppression depend on where inside a CPU cycle the VBL set becomes visible to phi2 sampling, so they need the half-cycle ordering already available in `bus.c cycle()` (set at dot 1, sampled at M2 rise).
**Fixes:** `NMI Control`, `NMI Timing`, `NMI Suppression`, `NMI at VBlank end`, `NMI disabled at VBlank` (the last four only if the dot-exact ordering of set, sample and read is right; allow a one-dot shifted table).
**Complexity:** High. This is the central change; it reworks the interrupt seam shared with Groups 3 and 4.
**Games (credible source):** Lolo 2, Ms. Pac-Man (Tengen), Spelunker: late NMI after enabling during vblank. Bases Loaded II: extra NMI on `$00` then `$80` writes. Gradius/Micronics-style code that counts cycles to NMI also benefit. Suppression-window dot accuracy: no named game.

### Group 3: CLI, SEI, PLP and branches poll with the old I flag

| Row | Code | Meaning |
|---|---|---|
| `Interrupt flag latency` | `0x0a` = subtest 2 | A DMC-DMA IRQ is raised ~50 cycles before. Subtest 1 (IRQ happens) passes. Subtest 2: `INX; CLI; INX; (IRQ here); INX` must store X=2 in the handler: the IRQ is taken after the instruction following CLI. nesturbator takes it right after CLI (X=1). |

Later subtests (not yet reached): 3 `CLI; SEI` still takes the IRQ already polled (and pushed P has I set); 4 pushed I flag; 5 IRQ again immediately after RTI in `SEI CLI`; 6 RTI updates I before polling (no IRQ when P popped has I set); 7 PLP delayed like CLI; 8 DMA-to-IRQ cycle exactness; 9 not-taken branch polls before cycle 2; A taken branch does not poll again before cycle 3; B taken page-crossing branch polls before the PCH fix-up cycle (code at `$FE00`); C/D prerequisites (PPU open bus, DMC DMA updating the data bus); E a first poll that saw the IRQ stays valid even if the flag is cleared by the branch's dummy `$4015` read before the second poll (IRQ at PC `$3FA5`, pushed P `$20`).

**Rules.** CLI, SEI and PLP change I in their second cycle, after the poll, so the following instruction runs before the interrupt. RTI restores I before polling. Branches poll before cycle 2; a taken branch does not poll before cycle 3; a taken page-crossing branch polls again before the fix-up cycle; detection at either poll triggers. Interrupt sequences do not poll. NMI beats a simultaneous IRQ. Source: NESdev "CPU interrupts"; AccuracyCoin asm `TEST_IFlagLatency` header comment (lines 9403-9475).
**What nesturbator gets wrong (derived).** `cpu.c` checks `(p & FLAG_I)` at the next instruction start, after `flag_op` has already updated I, so CLI/SEI/PLP take effect with no delay; the poll sample (`poll_latch`) is one boolean taken at the start of the last cycle. Fix: sample the I flag together with the IRQ line at the poll point (keep a polled-I copy), and make branches poll at both points with an OR-latch.
**Fixes:** `Interrupt flag latency`. Passing subtest 2 will expose 3-E; plan for the later ones to need DMC-DMA/IRQ cycle precision (DMC DMA rows still fail elsewhere, but the DMC pretest bytes report `0x01` today).
**Complexity:** Medium (I-flag snapshot) to High (all of A, B, E). **Games:** NESdev lists none for latency itself; correctness matters to any IRQ-driven code (MMC3, DMC, frame IRQ) but no title is credibly named.

### Group 4: An NMI arriving during BRK or IRQ takes over the vector

| Row | Code | Meaning |
|---|---|---|
| `NMI Overlap BRK` | `0x0a` = subtest 2 | Subtest 1 (32-step sweep of NMI against a `BRK`, which returns past the signature byte) completes. Subtest 2 compares flags the handlers pushed with an answer key: NMI handler sees `$A4` (BRK first), then `$36` for ~15 steps (NMI hijacks BRK: B set, BRK handler never runs, BRK slot stays `$00`), then `$26` (NMI first, normal), with the BRK slot `$36`. |
| `NMI Overlap IRQ` | `0x06` | Same with a DMC-IRQ (64 steps). Expected NMI `$A4` then `$26`; IRQ slot `$26` then `$00` for ~27 steps (IRQ hijacked by NMI, handler never runs), then `$26` again. Single compare, so `0x06` says only that the table differs. |

**Rules.** The vector is chosen after cycle 4 of 7 (after the PCL push, before the P push). An NMI asserted during the first four cycles sends BRK or IRQ to `$FFFA`; BRK still pushes P with B set and advances PC, IRQ/NMI push B clear; an NMI can hijack an IRQ, and an IRQ can hijack a BRK. Reset is the same sequence with writes suppressed. Source: NESdev "CPU interrupts"; prep NES-HARDWARE-CPU-APU 2.3.
**What nesturbator gets wrong (derived).** `cpu.c` NMI and IRQ entry (lines 496-526) and the BRK case do a fixed vector read; there is no hijack, so the hijack window (the ~15/27 steps) is absent. It also requires the Group 2 latch to exist so an NMI that appears mid-sequence is visible.
**Fixes:** `NMI Overlap BRK`, `NMI Overlap IRQ` (after Groups 2 and 3). **Complexity:** Medium once Group 2 lands (a vector decision after the PCL push). **Games:** none credibly named (NESdev lists no title for hijacking). Value is test conformance and CPU correctness.

### Group 5: Sprite-0 hit appears when and only when the console sets it

| Row | Code | Meaning |
|---|---|---|
| `Sprite 0 Hit behavior` | `0x06` = subtest 1 | Basic case fails: sprite 0 (tile `$FC` at x=8,y=0) over a solid BG tile at `$2001` must set bit 6 within about one frame. Later subtests (unreached): no hit with BG off, sprites off, both off, transparent sprite; hit at x=254, none at x=255; left-8 mask rules (x=0 masked, x=1 not); hit at y=238, none at y>=239; opaque-pixel overlap only; and E: the flag is not set until the PPU dot where the pixel is drawn (read 1 CPU cycle before = 0, 2 cycles after = 1). |
| `Arbitrary Sprite zero` | `0x03` timeout in the scoreboard; this is a budget artifact | With a larger execution cap (8000 frames, run today) the row ends `fail 0x06`, subtest 1: the same missing basic hit. With the 1800-frame cap, the earlier rows of page 19 consume the budget (`$2002 flag timing` alone is a long sync sweep) and this row is merely still active. Subtests 2-3 (unreached): writing `$2003` after the OAM-address reset (dots 257-320) and before dot 65 makes the sprite evaluated first at dot 66 act as sprite zero, including a misaligned OAM address (`$81`, tile bits ANDed with `$E3`). |

**Rules.** The flag sets when an opaque sprite-0 pixel overlaps an opaque BG pixel, only where both layers are enabled, never at x=255, not in the left 8 pixels when the matching mask bit is clear, and it clears at dot 1 of the pre-render line. The sprite counted as "zero" is the first sprite found in range at dot 66 of the previous line's evaluation, not necessarily OAM index 0. Sources: NESdev "PPU registers" (PPUSTATUS, OAMADDR notes); AccuracyCoin asm `TEST_Sprite0Hit_Behavior`, `TEST_ArbitrarySpriteZero` (lines 6908-7326).
**What nesturbator gets wrong.** Not yet isolated. `compose_pixel` (`ppu.c` 395-415) has the right shape (opaque overlap, x!=255, mask checks) and unit tests in `tests/ppu/test_sprites.c` pass, yet the real ROM sees no hit at all. Derived suspects: the sprite-zero flag is `eval_n == 0` (index based), which is wrong for the `$2003` cases; sprite scan/render may not run in the ROM's configuration (mid-VBlank rendering enable, `STX $4014` while rendering is enabled); or hit logic runs only in a path the ROM does not take. **A first-plan probe is required**: run `TEST_Sprite0Hit_Behavior` subtest 1 with a PC/`$2002` trace, then fix. Subtests D and E additionally need OAM DMA alignment (513/514 cycles) and PPU catch-up to be exact; Group 1's sub-cycle read rule matters for E.
**Fixes:** `Sprite 0 Hit behavior`, `Arbitrary Sprite zero`. **Complexity:** Medium (basic hit) to High (subtest 2-3 sprite-zero-by-position, E dot-exact).
**Games (credible source, NESdev "Tricky-to-emulate games"):** Super Mario Bros. (freezes on the title screen without sprite-0 detection), Battletoads (rendering disabled for scanlines; mistimed hit can hang), Devil World (sprite-0 for the status area), StarTropics and Crystalis (MMC3 splits, already covered by the MMC3 IRQ work). Sprite 0 is the standard status-bar split, so this group is the highest game impact of the milestone.

## Table stakes vs differentiators

| Group | Game impact | Test count it fixes | Complexity |
|---|---|---|---|
| 5 Sprite-0 hit | High (SMB, Battletoads, Devil World) | 2 | Medium-High |
| 2 NMI edge/latch/poll | High (Lolo 2, Ms. Pac-Man, Spelunker, Bases Loaded II) | 5 (plus enables 4) | High |
| 3 I-flag latency | Low-Medium (IRQ code) | 1 | Medium-High |
| 4 Hijacking | Low (no named title) | 2 | Medium |
| 1 $2002 split sampling | Low | 1 | Low-Medium |

## Anti-features (do not build in v5)

| Anti-Feature | Why | Instead |
|---|---|---|
| Per-row special cases in the PPU/CPU to satisfy a table | The answer keys are 1-dot-shifted-tolerant on purpose; hacks drift | Model the sub-cycle events (M2 rise/fall, phi2 sample, second-to-last-cycle poll) once |
| Raising the 1800-frame cap to erase the page-19 timeout | `tests/accuracy/budgets.txt` rejects this; the true cause is a budget artifact that disappears only when earlier rows pass and become shorter, or must be re-measured | Re-measure page 19 after Group 1 and 5 land, then decide the cap with evidence |
| Fixing neighbours (`Misaligned OAM behavior`, `Suddenly Resize Sprite`, DMA rows, `$2004 Stress Test`) | Out of the 11-row scope; DMA cluster is the next planned milestone | Note interactions only |
| A separate "NMI-hack" for specific games | Rule-based model covers Lolo 2, Ms. Pac-Man, Spelunker, Bases Loaded II | Same edge/poll mechanism |

## Dependencies between groups

```
Interrupt-poll seam (second-to-last-cycle poll, polled-I snapshot, edge latch)
  -> Group 2 (NMI Control 8/9, Timing, Suppression, VBL end, Disabled)
  -> Group 3 (CLI/SEI/PLP/branch latency)
  -> Group 4 (hijack; needs Group 2 latch visible in BRK/IRQ cycles 1-4)
Group 1 (split $2002 sampling)  independent; helps Group 2 (the same M2-rise sample feeds the line level) and Group 5 test E
Group 5 (sprite 0) independent of the CPU; rows share page 19 with Group 1, so cap/budget re-measure goes last
```

State impact: a new CPU latch (edge, polled NMI, polled I) is new state; `tests/accuracy` hashes and save-state coverage must be re-pinned (PRODUCT-ROADMAP exit condition).

## JAM: what AccuracyCoin does, and how a harness recognises it

- The pinned asm has no JAM test (a search for JAM/KIL finds nothing; the unofficial-opcode pages cover SLO/RLA/SRE/... only). A guest JAM is therefore always an accident: a mis-synced `Clockslide` or a bad jump runs into data. On hardware, a JAM stops the CPU until reset (no NMI is serviced either), so the menu freezes and the running test's result byte stays `0x03` forever.
- Raw bytes cannot tell a JAM from a slow test: both leave `0x03` at the cap, and the observer today reports the row as `guest-timeout` / `execution-cap-active-test` (or `execution-frame-failure` if the library returns an error). No ROM-side signal exists.
- The only discriminator is the library: `nesturbator_run_frame` returns `NESTURBATOR_STOP_JAM` (latched, `include/nesturbator.h`; `cpu.c jam()` sets `cpu.jammed`; further frame calls do not advance). `runner/accuracycoin.c` (`frame_and_sample`, line 561) passes status through, and its execution loop (line ~1195) folds every non-OK status into `stop_cause="execution-frame-failure"`, `verdict="guest-failure"`. The tune-up: map `STOP_JAM` to a distinct cause (for example `guest-jam`) in observe mode, the report/metadata and the campaign collector, and add a run-ahead test that saves/loads state across a JAM (state already stores `jammed`; `nesturbator.h` lines 651-652 define the expected behaviour). `runner/main.c` already prints "JAM stopped replay at frame N" for plain replay; mirror that attribution.
- Cheap extra evidence: after a JAM the CPU reads `$FFFF` every cycle, so PC is stuck; the frame hash is constant from then on.

## MVP recommendation (ordering for the roadmap)

1. Interrupt-poll seam plus Group 3 subtest 2 and NMI Control 8/9 (smallest visible wins, largest enabling change).
2. Group 2 remaining sweeps (Timing, Suppression, VBL end, Disabled): latch and suppression window.
3. Group 4 hijack.
4. Group 1 split `$2002` sampling (can run in parallel with 1-3).
5. Group 5 sprite 0: start with a trace probe of why subtest 1 sees no hit; then positional sprite-zero and dot-exact set.
6. JAM tune-up (independent, small); then re-measure page 19 and the budget, re-pin hashes.

Defer: DMA cluster rows, `Misaligned OAM behavior` (still timing out at 8000 frames; likely a hang in a sync loop, flagged for the next milestone), PAL.

## Sources

- AccuracyCoin asm and README at pin `6bee919529289c555803b612b8c9e4f7eaaeeabb` (https://github.com/100thCoin/AccuracyCoin): `TEST_2002FlagTiming`, `TEST_NMI_*`, `TEST_NmiAndBrk`, `TEST_NmiAndIrq`, `TEST_IFlagLatency`, `TEST_Sprite0Hit_Behavior`, `TEST_ArbitrarySpriteZero`, `TEST_Fail`, `VblSync_Plus_A`, `New_VBL_Sync` (HIGH, T1).
- NESdev Wiki: CPU interrupts, NMI, PPU frame timing, PPU registers, Tricky-to-emulate games (MEDIUM, T2): https://www.nesdev.org/wiki/CPU_interrupts, /wiki/NMI, /wiki/PPU_frame_timing, /wiki/PPU_registers, /wiki/Tricky-to-emulate_games. Fetched through a summarising reader; the NMI page gives no polling offsets, the dot-level suppression numbers come from PPU frame timing.
- Project: `tests/accuracy/scoreboard.txt`, `measurements.md`, `budgets.txt`; `.planning/preparation/NES-HARDWARE-CPU-APU.md` 2.1 and 2.3; `src/cpu.c`, `src/bus.c`, `src/ppu.c`; observe runs on pages 12, 18, 19 at 1800 and 8000 frames (local, 2026-10-25).
