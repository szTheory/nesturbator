# Domain Pitfalls

**Domain:** NMI/VBlank/sprite-0 timing fixes in an existing cycle-accurate NES core (v5)
**Researched:** 2026-10-25
**Overall confidence:** MEDIUM-HIGH. Hardware facts are from the NESdev wiki and the ppu_vbl_nmi readme (T1/T2, fetched). Which alignment AccuracyCoin assumes, and how this repo's frame boundary sits against VBL, are not read yet and are flagged.

Repo facts used: CPU keeps `nmi_prev`, `nmi_pending`, `irq_line`, `poll_latch` (internal.h:46); PPU keeps `vblank_suppress` (internal.h:125); `$2000` NMI-edge rule is behaviour revision 6 (moved 24 hash rows, Phase 10); NMI is currently sampled "at an instruction boundary" (cpu.c:494).

## Critical Pitfalls

### 1. Patching each AccuracyCoin row separately instead of one NMI-line model
**What goes wrong:** A tweak (suppress NMI when `$2002` read at dot N, delay edge by one dot) fixes `NMI Suppression` and breaks `NMI Timing` or `NMI at VBlank end`, or the reverse. Passes one test, breaks its neighbour.
**Why it happens:** The rows share one mechanism: /NMI = vblank_flag AND NMI_output, edge-latched, sampled during phi2, polled by the CPU at the second-to-last cycle [NESdev NMI, CPU interrupts]. Separate flags (`nmi_pending`, `vblank_suppress`) hold fragments of it.
**Consequences:** Whack-a-mole, off-by-one dots, plus ppu_vbl_nmi 05-08 regressions that the AccuracyCoin rows do not show.
**Prevention:** Model the physical line: compute `nmi_line = vblank_flag & nmi_output` after every PPU dot and every `$2000` write; the CPU edge detector latches a high-to-low (active) transition at a fixed sub-cycle point; polling reads the latch from the previous cycle. Delete `vblank_suppress` as a separate concept if the line model covers it. Fix the order inside a CPU cycle (PPU catch-up, then flag change, then line sample, then poll) once, in a comment, and never reorder per test.
**Detection:** Fixing a row changes the result code of another NMI row; ppu_vbl_nmi lines move.

### 2. The one-dot windows: wrong edge of the `$2002` race
**What goes wrong:** Hardware: a read one PPU clock before VBL set returns 0 and the flag never sets (no NMI that frame); a read on the set clock or one clock after returns 1 but no NMI; two or more clocks away behaves normally [NESdev PPU frame timing]. ppu_vbl_nmi 02 expects flag-suppressed on exactly line 04; 06 expects flag set with NMI suppressed on lines 05-06, NMI normal from line 07 [readme, fetched]. Implementations get the "no flag" case right and the "flag set, no NMI" two-dot case wrong, or the reverse.
**Prevention:** Encode both windows as a table of dot offsets (-1, 0, +1) in a unit test that reads `$2002` at each offset and asserts (value, flag after, NMI). Run ppu_vbl_nmi 02, 03, 05-08 directly if their ROMs are in `tests/roms/manifest.txt` (check; add if licence allows), since AccuracyCoin only reports pass/fail.
**Detection:** Existing code at ppu.c:564 handles reads "at/after dot 1"; check the dot-0 and dot+1 cases separately.

### 3. NMI sampled at instruction boundary instead of polled at the second-to-last cycle
**What goes wrong:** Current NMI check sits at the instruction boundary (cpu.c:494). AccuracyCoin NMI Control 8 and 9 need NMI polled before the write cycle of STA (gap after enabling) and between the two writes of INC (NMI right after INC) [AC README]. Interrupt Flag Latency 9-E need branch polls (before cycle 2, not before cycle 3, again before page fix-up). Boundary sampling cannot do these.
**Prevention:** Poll in each instruction's penultimate cycle with a per-opcode-class table (RMW, branch taken, page-cross, implied); store the poll result in `poll_latch`. The sequence itself does not poll, so one handler instruction always runs.
**Detection:** NMI Control 8/9 and IFL 9-E fail together.

### 4. Interrupt flag latency: CLI/SEI/PLP late, RTI early, and the pushed P
**What goes wrong:** CLI, SEI, PLP change I after the poll (effect one instruction later); RTI changes I before it [NESdev CPU interrupts]. AC IFL 3-4: an IRQ one cycle after SEI's last cycle must push I set. IFL 5-6: an unacknowledged IRQ re-fires after RTI when I was clear when pushed. Wrongly using the new I for the current poll gives off-by-one-instruction.
**Prevention:** Keep two I values: the one used by this instruction's poll (old for CLI/SEI/PLP, new for RTI) and the architectural one. Test with the cpu_interrupts_v2 ROMs 1, 2, 3, 5 in addition to AC.
**Detection:** IFL 2/7 (2 instructions after CLI/PLP) pass while 3/4/6 fail.

### 5. Hijack window: vector chosen too early or NMI edge cleared at the wrong cycle
**What goes wrong:** An NMI asserted within the first four ticks of BRK/IRQ makes the sequence take the NMI vector; BRK still pushes B set; the IRQ is later re-detected or the BRK is effectively cancelled [NESdev CPU interrupts]. Common bugs: choose vector at cycle 1; clear the NMI latch at sequence start (a hijacked or later-arriving NMI is then lost or fires twice); an NMI landing after the choice point must stay latched for the next instruction (after one handler instruction).
**Prevention:** Choose the vector at the vector-low fetch, clear the latch only when an NMI vector is actually taken. Unit-test NMI edge at each of cycles 1-7 of BRK and of IRQ (return address, pushed B, final PC). Cross-check `NMI Overlap BRK/IRQ` and cpu_interrupts_v2 ROMs 2, 3.
**Detection:** AC BRK code 1 (wrong return address) or code 2.

### 6. Hidden state missing from save states, and run-ahead divergence
**What goes wrong:** New latches (NMI line level, edge latch, `poll_latch`, delayed-I, hijack decision, sprite-0 pipeline bits, any CPU/PPU alignment phase) are added to the struct but not to the state visitor. Run-ahead (libretro replays frames after a load) then diverges, and RetroArch's `deterministic` claim becomes false.
**Prevention:** Add every new field to the one visitor (SIZE/WRITE/VALIDATE/APPLY/RECORD), bump the format version, add range checks for the rejection sweep, and extend `core.state_fork` to save at every PPU dot and CPU sub-cycle across the VBL set, VBL clear, and mid-BRK/IRQ/NMI sequence, then compare the continued run to an uninterrupted one. Also check where the frame boundary lies against line 241 dot 1: if a frame ends inside the race window, saves land inside the race (unverified; read `frame.c`).
**Detection:** `core.state_fork` mismatch; RetroArch state-ahead e2e hash differs; nothing else shows it.

### 7. Hash drift treated as a nuisance, or re-pinned blindly
**What goes wrong:** Shifting NMI by one CPU cycle moves register writes by one cycle, so frame and audio hashes of most game runs and the boot-to-play movie change (Phase 10 moved exactly 24 rows for a smaller change). Blind re-pin hides a real regression.
**Prevention:** Bump the behaviour revision (7); make the fix commit and the re-pin commit separate; for every moved row, state why (which NMI/poll edge) and view the frame diff (a one-line scroll jitter is expected, a hung game is not); require the same hash on all six platforms; run the 65x02 vectors and MMC3 blargg (10/12 unchanged, same two failing) before re-pinning. The boot-to-play `judge()` must still pass.
**Detection:** A moved hash that no reasoned timing change predicts.

### 8. Protected PASS set regresses through shared plumbing
**What goes wrong:** AccuracyCoin uses sprite-0 hits and VBL sync as measurement tools: Attributes as Tiles, INC $4014 (code 2) and Stale BG Shift Registers (code 3) read results through sprite-0 hits; many rows need sprite-0 hits "working" as code 1 (Suddenly Resize Sprite, Misaligned OAM DMA, t Register Quirks, $2007 Read w/ Rendering, Stale Sprite Shift Registers, Frozen OAM2 Increment, Misaligned OAM2 Address); OAM Corruption, $2004 Stress and $2007 Stress fail with "failed to sync CPU to VBlank" if the VBL sync moves [AC README, fetched]. Changing VBL/NMI/sprite-0 therefore can flip rows that pass today without touching their topic.
**Prevention:** Run the full 146-row campaign (Linux x64 lane) and diff against the 101-pass baseline after every behaviour commit, not only at phase end; give the diff output row names. Land sprite-0 changes (Arbitrary Sprite Zero, edge cases) in a separate plan from NMI work so a regression is attributable.
**Detection:** Campaign diff shows a previously passing row; the 1 timeout row changing.

### 9. Sprite-0: tracking the OAM index instead of the first sprite processed
**What goes wrong:** Hit logic tied to OAM index 0 fails `Arbitrary Sprite Zero` (2: the first processed sprite on a line counts as sprite zero; 3: misaligned OAM can hit) [AC README]. It must be a per-line "sprite 0 is in secondary OAM slot 0" latch set during evaluation, used on the next line's render; it is state to serialize.
**Other row traps (each its own AC code):** no hit at X=255 but hit at 254; left-8-pixel mask rules; hit at Y=238 but not >=239; transparent sprite 0; BG or sprites disabled; "set too early" (code E), where the flag must appear on the right dot after the overlapping pixel, not at sprite-fetch time. `$2002` reads sample sprite flags at M2 fall, 7.5 master clocks after the VBL flag point [AC `$2002 Flag Timing`, HWC.19], which needs the half-dot ordering from ARCHITECTURE.
**Prevention:** One table-driven test per code. Do not "fix" by setting the flag on any sprite-0 pixel; that is the known diagnostic hack for Battletoads, not hardware [nesdev forum report].

### 10. Fixing in a vacuum: alignment assumptions
**What goes wrong:** The CPU start can be 0-3 master clocks off the PPU tick, so more than one alignment exists, and the console powers up with a different divider phase [NESdev PPU frame timing; ppu_vbl_nmi readme says some resets fail tests]. Tests tuned to one alignment pass for one and fail for another. Wario's Woods depends on alignment (IRQ gets an extra clock every second frame on some resets) [NESdev tricky games].
**Prevention:** Determinism forces one fixed power-on and reset phase. Read AccuracyCoin's commented assembly (MIT) to learn which alignment its race tests assume; do not guess from the README, which does not say. Make the phase explicit saved state, set it in one place, and keep soft reset (libretro `retro_reset`) from reshuffling it unless that is deliberately modelled. If a row passes only for a non-default alignment, record it as a documented limitation rather than adding random alignment (breaks Rule 5).
**Detection:** A row passes after reset and fails after first power-on, or vice versa; ppu_vbl_nmi 02 prints "- -" instead of "V -" after some resets.

## Moderate Pitfalls

### 11. Cross-row deadlines and prerequisites
IFL C/D require open-bus tests to pass first; NMI Control 8/9 rely on IFL polling; IFL 1/8 use a DMC DMA IRQ, so DMA halt cycles feed the poll point (DMC timing rows must stay passing). A failing row can leave NMI enabled, I clear, or a stale latch for later rows, so a correct fix can turn a downstream row from fail to pass or the reverse, and a run that now takes longer can hit the per-test frame budget (one row already times out). Re-measure budgets; do not read a changed neighbour as a regression without checking the guest's leaked state.

### 12. Odd-frame skip and VBL set share a clock edge
ppu_vbl_nmi 09/10 check when the skipped clock occurs relative to enabling/disabling BG (expected 00 01 01 02 and 08 08 09 07). Re-ordering PPU work inside a dot to fix VBL timing can move the skip decision at (339,261). Keep 09/10 in the check list. Battletoads keeps BG off into the skip window [NESdev PPU frame timing], so both effects hit it.

### 13. MMC3 IRQ interaction
MMC3 counts A12 rises after three M2 falls low [HWP.12]. NMI/VBL changes do not alter fetch addresses, but altering dot ordering, sprite fetch or `$2001` effect delay shifts IRQ cycles (StarTropics flickers if the IRQ is a cycle or two late; Jurassic Park is IRQ-delay sensitive [NESdev tricky games]). Phase 9's filter (counts from the A12 fall, no extra delay) passes blargg scanline timing; do not touch PPU fetch/A12 code while fixing sprite flags. Run blargg MMC3 (10/12) and Holy Mapperel after each PPU edit.

### 14. `$2000` write semantics regress
Bases Loaded II is reported to glitch when writing $00 then $80 to PPUCTRL during VBL does not cause an extra NMI [NESdev tricky games]; this is NMI Control 7 and is already in revision 6. A model rewrite must keep "0-to-1 only, while flag set" and the "immediate NMI is serviced after one more instruction" delay (Lolo 2, Tengen Ms. Pac-Man, Spelunker per the same page). `core.nmi_edge` and `ppu.registers` must stay green.

### 15. Silent JAM during experiments
A mis-vectored hijack or wrong return address can send a guest to a JAM opcode. AccuracyCoin observe currently records JAM only as a generic frame failure (v4 audit), so a JAM looks like a timeout. The tune-up (JAM reported as JAM, run-ahead across a JAM test) should land first so later NMI-work failures are diagnosable. Run-ahead after a JAM: the replayed frames must stay halted and not diverge.

### 16. Writing tests that mirror the implementation
A test built from the emulator's own output cannot detect a wrong dot. Pin expected values to ppu_vbl_nmi or cpu_interrupts_v2 readmes, AC error codes or Visual2C02 reasoning; label a derived expectation "(derived)".

## Minor Pitfalls

- Early-emulator `$2002` behaviours (not clearing the flag on read) are relied on by some defective homebrew; do not add compatibility hacks (NESdev NMI).
- Do not apply the "shift VBL to dot 17" workaround reported for Marble Madness and Battletoads [community report, LOW]; it hides, not fixes, timing.
- Read-modify-write on `$2002`/`$2000` produces double accesses; a fix that special-cases RMW polls (INC) must still run the dummy write.
- README and `nesturbator.h` comments must change with the behaviour (Rule 6), including the revision number.

## Game Sensitivity (verification status)

| Game | Dependency | Status |
|------|-----------|--------|
| Super Mario Bros. | Sprite-0 detection or title screen freezes | Verified, NESdev tricky games |
| Battletoads | Precise CPU/PPU timing, robust sprite 0, odd-frame skip with BG off; stage 2 freezes if NMI or sprite-0 is slightly off | Verified (wiki); forum detail MEDIUM |
| Bases Loaded II | `$00` then `$80` to PPUCTRL in VBL must re-NMI | Verified (wiki); "NMI at VBL end" claim for Bases Loaded not found: UNVERIFIED |
| Lolo 2, Ms. Pac-Man (Tengen), Spelunker | Interrupt poll timing; NMI enable with flag set delays one instruction | Verified (wiki) |
| Gradius | Handlers avoid missing sprite 0 | Verified (NESdev NMI) |
| Wario's Woods | CPU/PPU alignment, MMC3 IRQ extra clock | Verified (wiki) |
| Marble Madness | Mid-scanline CHR switch, fine timing | Verified (wiki); dot-17 note LOW |

## Phase-Specific Warnings

| Phase topic | Likely pitfall | Mitigation |
|-------------|---------------|------------|
| 0: Tune-up (JAM label, run-ahead under JAM) | #15 | First, so later failures are diagnosable |
| 1: Baseline and alignment decision | #10, #16 | Read AC assembly; fix power-on/reset phase; add direct ppu_vbl_nmi and cpu_interrupts_v2 checks; record baseline campaign diff tool |
| 2: CPU poll model (IFL, NMI Control 8/9, hijack) | #3, #4, #5 | Per-cycle poll latch, vector choice at fetch; BRK/IRQ cycle-sweep tests |
| 3: PPU VBL/NMI line (flag timing, Timing, Suppression, VBL end, disabled) | #1, #2, #12, #14 | Single line model; dot-offset table tests; keep ppu_vbl_nmi 09/10 |
| 4: Sprite-0 and Arbitrary Sprite Zero | #8, #9, #13 | Separate plan; per-code tests; campaign diff after each commit |
| 5: State, run-ahead, hashes | #6, #7 | State fork at race points, revision 7, reasoned re-pins on six platforms |
| Deeper research flag | #10, #6 | Alignment assumed by AC; frame boundary vs VBL set point |

## Sources

- NESdev wiki NMI, CPU interrupts, PPU frame timing, PPU power up state, Tricky-to-emulate games (fetched 2026-10-25; HIGH for wiki facts)
- ppu_vbl_nmi readme, christopherpow/nes-test-roms (fetched; HIGH)
- AccuracyCoin README (MIT, fetched; HIGH for error codes, silent on alignment)
- Preparation files NES-HARDWARE-CPU-APU 2.3, NES-HARDWARE-PPU-CARTRIDGE 3; v4 audit; PROJECT.md decisions table
- NESdev forum Battletoads reports (via search summary; MEDIUM-LOW)
