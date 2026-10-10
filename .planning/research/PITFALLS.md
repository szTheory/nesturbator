# Pitfalls Research

**Domain:** Adding mappers (MMC1, MMC3, UxROM, CNROM, AxROM), battery saves, CI tune-up and a boot-to-play proof to an existing deterministic, accuracy-first NES core (nesturbator v2)
**Researched:** 2026-10-09
**Confidence:** MEDIUM-HIGH. MMC1 and MMC3 behaviour was re-read on the NESdev wiki today. Bus conflicts, iNES/NES 2.0 header rules, libretro SRAM behaviour and test-ROM quirks come from the project's own preparation files plus prior knowledge of the NESdev wiki and libretro docs; items marked (unverified) were not re-fetched. No GPL or LGPL emulator source was opened.

Phase labels used below (suggested, the roadmapper may rename):
- **P1 Tune-up**: CI speed, flakiness, pins, v1 debt (release-policy gate, trainer host path, `retro_reset()`, local RetroArch tests).
- **P2 MMC1 + saves**: MMC1, PRG-RAM generalisation, battery saves in library, runner, libretro.
- **P3 MMC3**: MMC3, A12 filter, scanline IRQ.
- **P4 Discrete boards**: UxROM, CNROM, AxROM, bus conflicts.
- **P5 Conformance pins**: Holy Mapperel and per-board hashes. Recommendation: start the pin for each board in the phase that adds it, and use P5 only to close gaps. v1 lesson: late gap-closure phases cost a re-verification.
- **P6 Boot-to-play**: one game proven from boot through interactive play.

---

## Critical Pitfalls

### Pitfall 1: A stale or shared `.sav` file silently changes a "deterministic" run

**What goes wrong:**
The runner derives `<rom>.sav` next to the ROM and loads it if present. A hash test for a battery game then depends on whatever file the previous run left behind. It passes on a clean CI checkout and fails locally, or flips between platforms. Parallel `ctest -j` runs of two tests on the same ROM race on one `.sav`. Tests also write files into `tests/roms/`, dirtying the tree, which `scripts/hygiene.sh` then flags.

**Why it happens:**
v1 had no persistent state outside the instance, so every harness assumed "same inputs, same hash". A `.sav` is a new, hidden input.

**How to avoid:**
- Make persistence explicit in the runner: no implicit `<rom>.sav` in test mode. Either a `--save <path>` flag the user passes, or a default that every test overrides with `--no-save`.
- Give every battery test its own temporary directory named after the test, and never write beside committed ROMs.
- State in the README that the save file is an input to the frame hash, and hash-pin tests with the save path recorded in the manifest.
- Add a test that runs a battery ROM twice, once with no file and once with a pre-seeded file, and asserts the hashes differ for a game that reads its RAM. This proves the file path is wired.

**Warning signs:**
"Works the second time"; a hash that changes after a local run; `git status` dirty after `ctest`; different results with `-j1` and `-j8`.

**Phase to address:** P2 (design the flag), enforced in P1's CI cleanup rules.

---

### Pitfall 2: Reporting the wrong save size, or a save size that moves

**What goes wrong:**
Existing `.sav`/`.srm` files from other cores are raw bytes whose size depends on the board: 8 KiB for most MMC1 and MMC3 games, 32 KiB for SXROM-class boards, 1 KiB for MMC6 (StarTropics), 2 KiB on a few oddities (unverified). If the core allocates 32 KiB for every MMC1 game (the prep doc says 32 KiB "covers all known titles") and exposes all of it as save RAM, every ordinary MMC1 save becomes a 32 KiB file that other cores cannot read, and hosts that compare sizes complain or truncate.

**Why it happens:**
Conflating "RAM the cartridge might need" (allocation) with "bytes that are battery-backed" (what the save contract exposes). iNES 1.0 headers give only byte 8 (units of 8 KiB, 0 meaning 8 KiB for compatibility); NES 2.0 gives separate volatile and non-volatile sizes.

**How to avoid:**
- Allocate generously inside the instance, but expose only the battery-backed span: iNES 1.0 with battery flag gives byte 8 (or 8 KiB if 0); NES 2.0 gives the non-volatile field only. Volatile PRG-RAM is never part of save data.
- Freeze this rule as a public contract (README and header comment) and commit fixtures: a synthetic 8 KiB `.sav`, a 32 KiB one, an MMC6 1 KiB one.
- Define the load policy for a file of the wrong size: copy `min(file, core)` bytes, leave the rest at the documented fill, never read past either buffer, and report a warning. Define the save policy: never silently shrink a longer user file (write beside it or refuse). Test sizes 0, short, exact, long.
- Treat the policy as frozen after the first release: changing it later corrupts saves already in users' hands.

**Warning signs:**
A `.sav` that is 32768 bytes after playing Zelda; a save that "loads" but the game says "no save"; asan reports on a long file.

**Phase to address:** P2.

---

### Pitfall 3: Battery flag or RAM size wrong in the header, and no database to fix it

**What goes wrong:**
Dumps with the battery bit missing lose saves with no error. Dumps with it set but byte 8 = 0 or NES 2.0 non-volatile size = 0 contradict each other. iNES 1.0 images that use WRAM without declaring it (common for MMC1 and MMC3) get no RAM at `$6000`, and the game hangs or loses its state. Conversely, giving NES 2.0 images that declare no RAM a readable `$6000` hides the open-bus behaviour Holy Mapperel checks.

**Why it happens:**
The project decided not to ship a header database (nes20db has no stated licence), so the header is the only truth.

**How to avoid:**
- iNES 1.0: always provide 8 KiB WRAM at `$6000-$7FFF` for mappers that can have it (1, 4 and, from v1, 0). The battery bit only decides whether it is persisted. NES 2.0: exactly what the header says; absent RAM reads as open bus.
- Battery bit set with zero battery bytes: treat as 8 KiB iNES 1.0 style, log a warning, add to a loader test.
- Expose the existing header-override path (host-supplied) so a user can fix a bad dump; document it for the runner (`--header-override` or equivalent).
- Reject unsupported mappers with a distinct error that the runner prints and libretro reports through its log callback. Do not fall through to a black screen. Examples that look like supported mappers but are not: 155 (MMC1A), 118/119 (TxSROM, TQROM, which take mirroring or CHR-RAM from CHR bit 7), 185 (CNROM with copy protection), 180 (UxROM with the fixed bank first), 30 (UNROM 512), 34 and 66.
- Keep the DiskDude! garbage-header case in the loader corpus: a mapper-1 image with text in bytes 7-15 must not become mapper 65.

**Warning signs:**
Games with a title screen but "save failed" messages; black screen for an unsupported board; Holy Mapperel WRAM rows failing only in NES 2.0 variants.

**Phase to address:** P2 (RAM rules), P4 (unsupported-mapper error covers 180/185), loader fuzz corpus in every mapper phase.

---

### Pitfall 4: MMC1 consecutive-write ignore implemented as a hack

**What goes wrong:**
On hardware the serial port ignores the second write when two writes land on consecutive CPU cycles. This happens with read-modify-write instructions (`INC $8000` writes the old value, then the new value one cycle later). Bill & Ted's Excellent Adventure, among others, depends on it. Implementations either omit it (the game resets banks wrongly and crashes) or approximate it with "previous opcode was RMW" or "ignore a write if the previous write was within N ticks" using the project's half-master-clock units. Both misfire on DMA-stolen cycles, on two ordinary `STA` writes separated by a stolen cycle, or on the bit-7 reset, which is never ignored.

**Why it happens:**
The mapper write callback has no CPU-cycle argument; the wiki notes the effect may apply even if the first write does not target the serial port, so a "last mapper write" timestamp is subtly wrong too.

**How to avoid:**
- Pass the CPU cycle index (cycles, not half-ticks) to the mapper write hook, and track "cycle of the most recent CPU write of any address".
- Count cycles stolen by DMC and OAM DMA as ordinary cycles so they break adjacency.
- Reset (bit 7) bypasses the ignore; only the data write is ignored.
- Soft reset must not clear the shift register's adjacency state in a way the cycle counter contradicts; if the cycle counter restarts at zero, restart the "last write" marker with it.
- Prove it with a synthetic hand-assembled test ROM generated in test code (no licence issues): `INC` on ROM vs two `STA`s; plus Bill & Ted locally if available.

**Warning signs:**
A mapper hook with no cycle parameter; "if opcode is RMW" anywhere in mapper code; a bank register changing twice from one `INC`.

**Phase to address:** P2 (change the mapper interface before MMC1 code lands; later mappers reuse it).

---

### Pitfall 5: MMC1 reset bit, power-on state and register decode errors

**What goes wrong:**
- Bit 7 clears the shift register and ORs control with `$0C` (PRG mode 3, last bank fixed at `$C000`). It does not zero control, does not touch CHR or PRG registers, and does not change CHR mode or mirroring. Zeroing control instead selects 32 KiB mode and a wrong reset vector.
- The fifth write selects the register by address bits 14-13 of that fifth write, not the first.
- Power-on state is not guaranteed on hardware. Starting control at 0 breaks the reset vector when the last bank is not at `$8000-$FFFF` in 32 KiB mode.
- PRG modes 0/1 (32 KiB) ignore the low bank bit; CHR 8 KiB mode ignores the low bit of CHR bank 0 and all of bank 1; with 8 KiB CHR the bank is effectively ANDed with 1.
- Mirroring encoding: MMC1 control bits 0-1: 0 and 1 are one-screen (lower, upper), 2 vertical, 3 horizontal. MMC3's `$A000` bit 0 is the opposite sense (0 vertical, 1 horizontal). Mixing them inverts every title screen.
- PRG-RAM enable: MMC1A (mapper 155) keeps RAM always on; MMC1B honours bit 4 of the PRG register as a disable; on SNROM the CHR register bit 4 acts as the RAM disable. iNES cannot distinguish A, B and C, so choose the B behaviour for mapper 1, 155 for A, default RAM enabled at power-on, and let tests pin the choice. Honouring disable bits too eagerly breaks games that set stray bits.
- SUROM, SOROM, SXROM reuse CHR bank bits for the 256 KiB PRG half and the RAM bank; in 4 KiB CHR mode the two CHR registers should hold matching high bits, otherwise PRG can switch with PPU A12 mid-frame. Use CHR register 0 and document that choice.

**How to avoid:**
Table-driven unit tests of every register (feed 5 bits LSB first through the port, assert the decoded banks, modes and mirroring); reset-bit test asserts PRG mode becomes 3 and CHR registers unchanged; a test per mirroring value that checks all four nametable mappings.

**Warning signs:**
Title screens mirrored wrongly; a game boots only after pressing reset; Final Fantasy or Zelda lose RAM.

**Phase to address:** P2.

---

### Pitfall 6: PPU nametable mapping is H/V-only, latched, or cached per frame

**What goes wrong:**
NROM needed only a fixed horizontal or vertical choice. MMC1 (one-screen lower/upper), AxROM (one-screen with a page bit), MMC3 (`$A000`; ignored with four-screen), and four-screen carts (Gauntlet, Rad Racer II class, mapper 4 with the header four-screen bit) need a general four-entry nametable map. A change made by a register write mid-frame must apply on the very next nametable fetch, including `$2007` accesses. Implementations that latch the mapping at frame or scanline start, or keep an enum {H, V, FOUR}, miss single-screen modes and split-screen effects.

**How to avoid:**
Replace the mirroring enum with a four-entry source table (CIRAM page 0, CIRAM page 1, extra RAM 0-3) that the mapper updates at write time and the PPU reads at fetch time. Add a four-screen RAM block in the instance only when the header says so. Test: write `$A000`/`$8000`/`$8000` AxROM bit between two nametable reads on consecutive dots and assert the second read sees the new page.

**Warning signs:**
`enum mirroring`; a cached `nt_base[4]` copied at vblank; AxROM games showing both screens identical for the wrong reason.

**Phase to address:** P2 (the interface), P4 (AxROM exercise).

---

### Pitfall 7: MMC3 clocked without the A12 M2 filter, or with a PPU that skips unused-slot fetches

**What goes wrong:**
The MMC3 counter is clocked by a rising edge of PPU A12 only after A12 has been low for three falling edges of M2 (roughly three CPU cycles). Without the filter, the garbage nametable fetches between sprite pattern fetches (A12 low for about 4 dots) clock the counter up to eight times per line. The result is IRQs at the wrong lines and games like Mega Man 3-6, Kirby's Adventure and SMB3 splitting wrongly. The filter must be measured in M2 cycles derived from the project's half-tick base; an off-by-one rounding at 3 versus 2.67 CPU cycles is exactly the kind of boundary mmc3_test_2 probes.

Related traps:
- The PPU must actually perform the sprite-slot fetches of dots 257-320 whenever rendering is enabled, including empty slots (tile `$FF`) and the 8x16 case where the slot's pattern table comes from bit 0 of tile `$FF`. If v1 skips empty slots for speed or accuracy shortcuts, the counter never clocks on lines without sprites.
- With 8x16 sprites, A12 depends on each sprite's tile index bit 0 (BG at `$0000`, sprites mixed between tables), and the number of clocks per line depends on the sprite set; IRQs arrive early or late by multiples of 8 pixels.
- A12 also changes outside rendering through `$2006` writes and `$2007` accesses; games clock the counter by hand this way, so A12 tracking must observe the VRAM address bus at all times, not only during rendering. Rendering disabled means no fetches and no clocks.
- Dummy fetch addresses must be shown to the mapper (as a cheap `a12` notification or flag) not only the pattern fetches the renderer uses.

**How to avoid:**
Expose a single "PPU bus address changed" hook gated by a mapper capability bit so NROM pays nothing and v1 hashes cannot move. Keep the last-A12-low timestamp in the instance in a unit derived from the time base, with named constants. Write boundary tests that hold A12 low for 2, 3 and 4 M2 cycles and assert clocked / not clocked.

**Warning signs:**
IRQ count per frame is a multiple of expected; status bars wobble by a few lines; mmc3_test_2 tests 1-3 fail; v1 NROM hashes change after the hook lands.

**Phase to address:** P3. Its plan must start with the hook and a v1-hash-unchanged check.

---

### Pitfall 8: MMC3 IRQ revision, reload and ack semantics

**What goes wrong:**
- Counter logic: on each clock, if the counter is 0 or the reload flag is set, reload from the latch (`$C000`), otherwise decrement. The IRQ fires when the counter is zero and IRQs are enabled. The "new" (Sharp) behaviour fires whenever the result is zero, so latch 0 gives an IRQ every line; the "old" (NEC) behaviour fires on the 1 to 0 transition only, giving one IRQ for latch 0, plus another if `$C001` is written while the latch is 0. NES 2.0 submapper 4 selects NEC; MC-ACC (submapper 3) differs again.
- `$E000` disables and acknowledges; the counter keeps running while IRQs are disabled. `$E001` enables and does not touch the counter. `$C001` clears the counter and sets the reload flag, so the reload happens on the next clock.
- IRQ must be a source OR-ed into the CPU's IRQ line with the APU frame IRQ and DMC IRQ. A single `irq_pending` bool that APU code overwrites or clears loses MMC3 IRQs.
- Register decode uses only A0 and the upper bits of the range (even/odd pairs mirrored across `$8000-$FFFF`); decoding the full address makes `$8002` a no-op.
- Recompute banks on writes to `$8000` too, because bits 6 and 7 (PRG swap, CHR inversion) change the mapping without a new `$8001` write. R6/R7 use 6 bits; R0/R1 ignore the low bit.
- Default revision choice: Sharp/"new" for plain mapper 4 (most games run on either; a few are sensitive); NEC for submapper 4. Document the choice.
- `$A001` RAM protect (bit 7 enable, bit 6 write-protect): default RAM enabled and writable at power-on; honour the bits only after a write. Treat MMC6 (submapper 1, 1 KiB, different `$A001` meaning) as separate or unsupported; do not blend.
- After the IRQ line goes active the CPU takes it after the current instruction under the project's existing polling rules; an A12 edge landing one dot early or late relative to the CPU's M2 cycle shifts the IRQ by a CPU cycle, which is visible as one-line jitter.

**How to avoid:**
Model the counter as a small struct with a `rev` field; unit-test the table of (latch, reload flag, enabled, revision) to expected IRQ pattern; test line-sharing with the APU frame IRQ (both asserted, ack one, other persists).

**Phase to address:** P3.

---

### Pitfall 9: Bus conflicts applied to the wrong boards, or read from the wrong bank

**What goes wrong:**
On discrete boards with ROM and CPU both driving the data bus, the written value is ANDed with the ROM byte at that address (read through the currently selected bank). Wrong choices:
- Applying AND to every UxROM, CNROM and AxROM image. Boards without conflicts (ANROM variants; some AxROM dumps) then select wrong banks when a game writes a value that differs from the ROM byte. The wiki's counts: all licensed UxROM and CNROM dumps in nes20db are submapper 2 (conflicts); AxROM splits roughly 27 without to 31 with.
- Never applying it to any, so the conflict-dependent behaviour Holy Mapperel and a few titles probe is wrong.
- Reading the ROM byte from the wrong bank (the pre-write mapping is the right one, since the write has not taken effect).
- Treating submapper 0 as "conflicts" by default. Unspecified headers are everywhere in the wild; a no-conflict default cannot crash games that wrote matching values, while a conflict default can crash games written for non-conflict boards.

**How to avoid:**
Submapper 2: AND. Submapper 1: none. Submapper 0 (iNES 1.0): choose and document a default, with a recommendation of no conflicts for AxROM and conflicts for UxROM/CNROM only if Holy Mapperel and the manifest show it is safe (decide in the P4 plan; this is an open decision, see Gaps). Test the AND with a hand-built ROM in test code, reading through both bank positions.

**Phase to address:** P4.

---

### Pitfall 10: Battery flush timing, atomicity and platform I/O

**What goes wrong:**
- The runner writes only at exit; a crash, kill or Ctrl-C loses the session. Writing on every RAM write thrashes the disk.
- A truncated write (power loss, kill during `fwrite`) leaves a zero-length or partial `.sav`, which the next run loads as a valid, nearly empty save, then overwrites the only good copy.
- C `rename()` on Windows fails if the target exists, so the usual "write temp then rename" pattern is not portable with the standard library alone. The platform call (`MoveFileEx` with replace) is needed, or a documented fallback.
- Text mode on Windows corrupts bytes `$0A`/`$1A`: always `"rb"`/`"wb"`.
- Writing a save that was never modified, or when load failed, wastes writes and can replace a good file with default-filled RAM after a failed read.

**How to avoid:**
Core: no I/O (rule: core links only memory functions). Core exposes the span, size, and a dirty indicator (instance counter incremented when a battery byte actually changes). Runner: flush on exit and on signal, only when dirty and only when the load phase succeeded; write temp, sync, replace; never overwrite on a size-policy refusal. Libretro: expose the span; the host flushes (RetroArch autosaves on an interval and at close).

**Warning signs:**
`.sav` of size zero; save missing after `kill`; Windows CI failing the "replace" test; hosts that cannot tell when to save (Playstead needs "safe battery saves").

**Phase to address:** P2.

---

### Pitfall 11: Libretro SRAM contract violations

**What goes wrong:**
- `retro_get_memory_data(RETRO_MEMORY_SAVE_RAM)` returns a pointer that moves or dangles (reallocated at reload, freed before the host's close-time save), or returns a non-zero size for a game without battery.
- Hosts restore the `.srm` after `retro_load_game()` and before the first `retro_run()`. If the core zeroes or re-initialises PRG-RAM on the first frame, on reset, or in a late power-on step, restored data disappears.
- `retro_reset()` is a v1 no-op for loaded games. The naive fix, rebuilding the instance, wipes volatile and battery RAM and clears mapper registers. A console reset leaves PRG-RAM and mapper state alone; MMC1's shift register is exactly why games write `$80` at startup. Reset must preserve cartridge RAM and not reset mapper registers other than any adjacency markers tied to the cycle counter.
- The size changes between `retro_get_memory_size` calls (derived from state rather than header).
- The save span includes volatile WRAM (see Pitfall 2) so `.srm` files from other cores load wrongly.

**How to avoid:**
Pointer stable from `retro_load_game()` until `retro_unload_game()` returns; size constant; both zero for no battery; `retro_reset()` calls the core's soft reset, not reload; a libretro host test (extend `tests/libretro/libretro_host.c`) that sets SRAM after load, runs frames, resets, runs frames and reads SRAM back. Document in the adapter's comments.

**Phase to address:** P1 (`retro_reset()`), P2 (SRAM).

---

### Pitfall 12: Boot-to-play predicates that pass without the game playing

**What goes wrong:**
Predicates that look like proof but can pass on a hung, idle or fallback run:
- "Frame hash changed after input": attract-mode animation, a blinking cursor, palette cycling or timer text also change it. Fix: run the same timeline with and without the input and require the first divergence to occur after the input frame; repeat with two different inputs and require different results.
- "Game-state byte changed": a frame counter or RNG byte always changes. Pick a semantic byte from the game's symbol map (open-source game: position, lives, score, selected menu item) and assert before and after values, not only inequality.
- "Audio non-silent": any non-zero sample passes, including DMC DC offset, a stuck noise channel or APU power-on click. Require sustained variation (min and max over a window above a threshold, or distinct sample counts) across at least N frames.
- Cartridge not actually loaded: the runner's no-cartridge path emits the built-in test frame and audio, so a failed or unsupported load can pass a lax predicate. Assert exit status, mapper id and a non-test-frame hash.
- Stuck in a loop with NMI still running: the picture is static but status counters tick. Require scene transitions (title to gameplay).
- Input latched too late: hold input over several frames and across controller strobes; verify bit order and that an unpressed run differs.
- Passing on the runner but not through libretro: v1's flows show both; require both to agree.
- Choosing a licensed game that cannot be committed, so the check is local-only. The only committable open-licence game on a v2 mapper in the prep list is nes-runner (MMC1, MIT); Nesteroids, DABG and RHDE are mapper 0 (already supported). Do not use Battletoads or other timing-sensitive titles as the proof game.

**How to avoid:**
Define the predicate as a short script (movie file) with three assertions and one negative control, stored with the manifest. Mutation self-test: replace input with neutral input, and the check must fail; point the runner at a missing ROM, and the check must fail.

**Phase to address:** P6, with the negative controls added in the same plan.

---

### Pitfall 13: Test ROMs that mislead

**What goes wrong:**
- Blargg `$6000` protocol: `$6000` is 0 before the ROM writes `$80`. A harness reading it at frame 1 sees 0 and reports pass. Require the `$DE $B0 $61` signature at `$6001-$6003` and an observed `$80` first. Before v2, mapper-0 ROMs had no RAM there, so these harness paths were never exercised on real hardware semantics.
- Mixed polarities: `$6000` code 0 is pass; the older screen-text suites using zero-page `$F8` mean 1 is pass. A scoreboard that decodes both with one rule marks half of them wrongly.
- mmc3_test_2 and mmc3_irq_tests: the last two ROMs of each target different chip revisions (Sharp versus NEC), so no single implementation passes both. Record the other as `unsupported` or a distinct revision row; do not call it a regression and do not "fix" the chip logic to pass both.
- Holy Mapperel: a PCB test with no memory result channel; the result is a screen code (0000 normal) plus beeps. A hash pinned from the current output without decoding the code can pin a failing screen. Decode the code from the nametable and require it, then also pin the hash. Variants include NES 2.0 RAM-size combinations; the loader must honour them. It also covers mappers not in v2 (9, 10, 11, 28, 34, 66, 69, 118, 180 and more); those rows are `unsupported`, not failing. Verify the digit semantics at the pinned v0.02 README (unverified here).
- Holy Mapperel and the blargg ROMs differ in redistribution: Holy Mapperel is zlib and committable; the blargg suites have no licence, so they are fetched nightly at the pin and checked by SHA-256. Per-PR coverage of fine MMC3 IRQ timing is therefore thin, which makes hand-built test ROMs in the repo's own tests important.
- ROMs that need a reset (`$81`) require the P1 `retro_reset()` and runner reset to be real first.

**How to avoid:**
Per-protocol decoders in the harness, each with a known-fail and known-pass fixture; scoreboard keys per revision; unsupported mappers listed by id.

**Phase to address:** P1 (harness decoders, reset), P3 and P5 (scoreboard rows).

---

### Pitfall 14: Pinned hashes that are fragile or taken from unverified output

**What goes wrong:**
- Hashes recorded from the first output and never visually checked.
- Hashes taken in a transitional frame (fade, scroll, attract animation) that change with any legitimate timing fix and force a pin storm.
- Mapper hooks added to the shared PPU/CPU path move v1 NROM hashes (extra dots, changed fetch order, an uninitialised field).
- Uninitialised mapper state, struct padding or pointer values entering a hashed or serialised blob make hashes differ per platform. Power-on register values of MMC1/MMC3 are undefined on hardware; pick fixed values in the instance and write them down.
- CHR-RAM and volatile PRG-RAM power-on pattern: choose a fixed fill (zero) rather than random or sanitiser-dependent memory. First-frame hashes depend on it.
- Non-power-of-two PRG or CHR sizes with a mask (`& (size-1)`) instead of a modulo give wrong banks and out-of-range reads.

**How to avoid:**
Pin at a stable idle frame after a scripted input, record the frame count, input script and save-file state in the manifest; re-run the v1 hash inventory on every mapper phase and require it unchanged; run `asan` and `nofp` on each new mapper; compare the six-platform inventories byte for byte as v1 did.

**Phase to address:** P2-P5, each with the v1-unchanged check.

---

### Pitfall 15: Mapper state not reachable by fuzzing and sanitisers

**What goes wrong:**
v1 fuzzes the ROM loader only. New attack surface: NES 2.0 RAM size fields (64 << n, up to 2 MiB each), bank calculations on tiny or odd-sized PRG/CHR (a 16 KiB PRG with MMC3 or a 24 KiB CHR), the `.sav` loader (size taken from a file), and mapper register writes combined with PPU fetches. Allocation before comparing sizes with the file length is the known hostile-input trap.

**How to avoid:**
Per-mapper minimum sizes or modulo bank math validated in the loader; save-file reads bounded by the instance span; extend the corpus with one image per mapper id and odd sizes; add a fuzz target that loads a header-valid image, writes random bytes to `$4020-$FFFF`, and steps a few frames.

**Phase to address:** P2 (first), each mapper phase adds seeds.

---

## Moderate Pitfalls

### Pitfall 16: CI flakiness sources the tune-up should remove

**What goes wrong:**
- Floating runner labels (`macos-latest`, `windows-latest`) change images under the gate; pin labels or record the image version in logs.
- Sleep-based waits in RetroArch end-to-end screenshots; replace with polled conditions and bounded timeouts.
- A local test that self-skips with exit 0 hides rot. Make the skip return code explicit and make CI fail when any test skips; retire what cannot run (matches the v1 debt item).
- Parallel tests sharing temp paths, `.sav` files, ports or screenshot locations (see Pitfall 1).
- Cache keys that omit the compiler, preset or pin, so a stale cache hides a build break; caching test results.
- Speed-ups that quietly drop coverage: moving `asan`, `nofp` or the six-platform hash comparison off the required path. Keep them on PR, trim by other means (ccache, fetch only changed pins).
- Downloaded vector sets (1 GB) on nightly: network flake. Retry with the checksum and keep the nightly separate from the required check.
- Release-policy gate: the substring check can be defeated by an appended `|| always()`. A stronger regex can be defeated again; add several mutation cases (appended condition, moved condition, a comment containing the string) and parse the `needs:` and `if:` lines of the publish job structurally in CMake script.
- Windows: `long` is 32-bit, shifts of signed values, bitfields, path separators and `fopen` text mode; the MSVC preset catches some of these.

**Phase to address:** P1.

### Pitfall 17: Trainer path generalised badly

**What goes wrong:**
v1 allocates PRG-RAM for trainer-bearing mapper-0 images. With MMC1/MMC3 the RAM comes from the new unified allocation; two allocation paths mean double free or a missed free on reload. The trainer lands at `$7000-$71FF`; a host loading `.sav` after load overwrites it for battery games (RetroArch restores SRAM after `retro_load_game()`). The v1 audit notes no trainer image goes through the runner and libretro host path.

**How to avoid:** One RAM allocation function used by all mappers; reload test under asan; define precedence (trainer first, `.sav` over it) and test with a trainer + battery fixture through both hosts.

**Phase to address:** P1 (host-path fixture) and P2 (precedence).

### Pitfall 18: Cached CHR/PRG pointers and mid-scanline bank swaps

**What goes wrong:**
MMC3 games change CHR banks in the IRQ handler between background and sprite fetches. A PPU that resolves the CHR page once per scanline reads the old bank. v1's per-dot PPU is probably safe; a mapper that caches an eight-entry CHR pointer table is safe only if every register write, including `$8000`, rebuilds it.

**How to avoid:** Resolve CHR addresses at fetch time from the table; add a test with a bank write between the two pattern fetches of a tile.

**Phase to address:** P3.

### Pitfall 19: Public API and declaration baseline drift

**What goes wrong:**
The v1 public API baseline guard fails when the header changes; a quick refresh hides an accidental ABI change. Save access added to the opaque-instance API must stay compatible with the sibling core's conventions, and the README and header comments must change in the same commit (rule 6).

**How to avoid:** Add save span access, size and dirty indicator as one change, refresh the baseline deliberately, document ownership and lifetime of the pointer.

**Phase to address:** P2.

### Pitfall 20: Release-please versioning for behaviour changes

**What goes wrong:**
Each mapper merges as `feat:`; a save-format change is a user-visible compatibility change and belongs in the changelog. A `fix:` for something that alters hashes of existing games is invisible to pinned-hash users.

**How to avoid:** Conventional Commit titles that say when hashes or save compatibility change; keep `release.one_version_per_line` guarded.

**Phase to address:** all.

---

## Minor Pitfalls

### Pitfall 21: Unused-bit and mask details
**What goes wrong:** CNROM CHR bank uses only the low bits and wraps by CHR size; AxROM bank bits 0-2 plus bit 4; UxROM fixes the last bank at `$C000` (mapper 180 fixes the first); MMC3 PRG fixed banks swap with the mode bit.
**Prevention:** Mask/modulo against actual bank counts; table-driven tests for each board.

### Pitfall 22: Games that are known to expose timing bugs used as acceptance
**What goes wrong:** Choosing Battletoads, Kirby, Marble Madness or Bill & Ted as the headline proof couples the mapper milestone to PPU/CPU alignment accuracy that is v3 scope.
**Prevention:** Use these locally as bug finders, not as gates.

### Pitfall 23: Non-battery PRG-RAM treated as save data
**What goes wrong:** Persisting volatile RAM makes games load stale data.
**Prevention:** Persist only the battery span (Pitfall 2).

### Pitfall 24: Copying code or logic from reference emulators
**What goes wrong:** Breaks the clean-room rule.
**Prevention:** Implement from the wiki and test behaviour; reference emulators only as released binaries; keep wiki citations in hardware comments.

---

## Technical Debt Patterns

| Shortcut | Immediate Benefit | Long-term Cost | When Acceptable |
|----------|-------------------|----------------|-----------------|
| "Previous opcode was RMW" for MMC1 write ignore | No cycle plumbing | Wrong with DMA and non-RMW adjacency | Never |
| Pin hash from first output | Fast green CI | Pins a wrong picture | Never; decode result or inspect |
| Mirroring enum | Smaller change | Single-screen and four-screen require a rewrite | Never past P2 |
| 32 KiB save for all MMC1 | One code path | Incompatible `.sav` files | Never |
| Ignore MMC3 `$A001` entirely | Matches common practice | Fails Holy Mapperel RAM-protect rows; MMC6 conflict | Acceptable in P3 if documented and pinned as `unsupported` |
| Per-frame blanket skip of mapper hook | Speed | A12 misses | Only via capability bit |
| Default conflicts on for submapper 0 | Looks accurate | Can break non-conflict dumps | Only if evidence in P4 supports it |
| Reload instance to implement reset | Easy | Wipes RAM and registers | Never |

## Integration Gotchas

| Integration | Common Mistake | Correct Approach |
|-------------|----------------|------------------|
| RetroArch SRAM | Pointer moves; RAM re-initialised after the host restored it | Stable pointer, constant size, init only at load |
| RetroArch reset | Rebuild instance | Soft reset keeps RAM and mapper registers |
| Runner `.sav` | Implicit file beside ROM | Explicit path; tests use temp dirs |
| Windows file replace | `rename` over existing | Platform replace call or documented fallback |
| Windows file mode | Text mode | Binary mode always |
| Hosted macOS/Windows runners | Floating labels | Pin or log image version |
| Playstead-style host | Pointer only, no dirty signal | Dirty counter in the instance |
| Scoreboard | One decode rule for all protocols | Per-protocol decoders |

## Performance Traps

| Trap | Symptoms | Prevention | When It Breaks |
|------|----------|------------|----------------|
| Per-fetch mapper function pointer for every board | Slower NROM, hash-gate time up | Capability bit; no hook for NROM/UxROM/CNROM/AxROM | Visible from the first MMC3 merge |
| Per-write save-file I/O | Stutter | Dirty flag; host flushes | Games that write RAM every frame |
| Fetching full vector sets on PR | CI minutes | Keep nightly | Already a v1 split |

## Security Mistakes

| Mistake | Risk | Prevention |
|---------|------|------------|
| Allocating from NES 2.0 RAM fields before checking limits | Memory exhaustion | Cap and compare with file length |
| Reading `.sav` into a fixed buffer without bound | Overflow | Bounded copy of `min` bytes |
| Writing `.sav` through a predictable temp name in a shared directory | Clobber or symlink | Temp file in the same directory with a unique name |
| Paths and user names in saved metadata or test artifacts | Privacy rule | Hygiene script covers new artifacts |

## UX Pitfalls

| Pitfall | User Impact | Better Approach |
|---------|-------------|-----------------|
| Unsupported mapper shows black screen | User thinks core is broken | Clear error message through runner/libretro log |
| Save loaded with wrong size silently | Lost progress | Warning naming expected and actual size |
| Reset deletes save | Lost progress | Soft reset preserves RAM |

## "Looks Done But Isn't" Checklist

- [ ] **MMC1:** consecutive-write ignore tested with an RMW and a DMA-separated pair — verify bank unchanged after `INC`.
- [ ] **MMC1:** reset bit leaves CHR/PRG registers untouched — verify PRG mode 3 only.
- [ ] **MMC3:** IRQ per line count over a frame with 8x8 and 8x16 sprites; empty-slot fetches present.
- [ ] **MMC3:** both revisions selectable; the test table includes latch 0.
- [ ] **Battery:** round trip across two instances, wrong-size files, no-battery game returns size 0.
- [ ] **Battery:** `retro_reset()` leaves SRAM intact; first `retro_run()` does not clear restored data.
- [ ] **Bus conflicts:** submapper 1 vs 2 differ in a test; ROM byte read from pre-write bank.
- [ ] **Mirroring:** all four-entry modes tested; mid-frame write seen on the next fetch.
- [ ] **Holy Mapperel:** result code decoded and required, not only the hash; unsupported mappers listed.
- [ ] **Hashes:** v1 inventory unchanged; six-platform comparison identical.
- [ ] **Boot-to-play:** negative controls fail (no input, missing ROM).
- [ ] **CI:** no test self-skips with success; no test writes into the source tree.
- [ ] **Docs:** README and header comments describe save contract, mapper list and defaults in the same change.

## Recovery Strategies

| Pitfall | Recovery Cost | Recovery Steps |
|---------|---------------|----------------|
| Wrong save size policy released | HIGH | Add a migration rule for old files, publish in release notes, keep both-size fixtures |
| MMC3 off-by-one IRQ timing | MEDIUM | Adjust constants with boundary tests; re-pin only the affected rows with reasons |
| Mirroring enum too small | MEDIUM | Introduce the source table, migrate NROM first, compare v1 hashes |
| Pinned a wrong picture | LOW | Decode and fix, re-pin in its own change |
| Flaky test | LOW | Quarantine with an issue, fix root cause, do not retry-loop |

## Pitfall-to-Phase Mapping

| Pitfall | Prevention Phase | Verification |
|---------|------------------|--------------|
| 1 Stale/shared `.sav` | P2, P1 rules | Clean-tree check after `ctest -j`; seeded vs unseeded hash differ |
| 2 Save size | P2 | Fixture sizes 8K/32K/1K, short/long |
| 3 Header errors | P2, P4 | Loader corpus; unsupported error test |
| 4 MMC1 consecutive writes | P2 | Synthetic RMW ROM |
| 5 MMC1 reset/decode | P2 | Register table test |
| 6 Mirroring table | P2 | Mid-frame mirroring test |
| 7 A12 filter | P3 | M2 boundary test; v1 hashes unchanged |
| 8 MMC3 IRQ revisions | P3 | Latch/reload table; shared IRQ line test |
| 9 Bus conflicts | P4 | Submapper 1 vs 2 test |
| 10 Flush atomicity | P2 | Kill-during-write test; Windows replace |
| 11 Libretro SRAM | P1, P2 | Host test with restore/reset |
| 12 Boot-to-play | P6 | Negative controls |
| 13 Misleading ROMs | P1, P3, P5 | Decoder fixtures |
| 14 Fragile hashes | P2-P5 | Inventory diff |
| 15 Fuzz/sanitiser | P2-P4 | Corpus seeds per mapper; asan green |
| 16 CI flakiness | P1 | Skips fail CI; mutation self-tests |
| 17 Trainer path | P1, P2 | Trainer + battery fixture through both hosts |

## Sources

- NESdev Wiki MMC1 and MMC3 (fetched 2026-10-09): consecutive-write ignore, bit-7 reset, `$0C` OR, RAM-enable differences, SUROM/SOROM/SXROM bits, A12 filter of three M2 falling edges, Sharp versus NEC reload, 8x16 notes, `$E000/$E001`, `$A001` (HIGH). Note that a summariser garbled the MMC3 mirroring polarity; confirm 0 = vertical on the wiki page before coding.
- Project files: `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md` (mapper table, header rules), `CONFORMANCE.md` (suites, protocols, licences, scoreboard), `NES-ECOSYSTEM.md` (user complaints on mapper coverage and saves), `.planning/RETROSPECTIVE.md`, `.planning/milestones/v1-MILESTONE-AUDIT.md` (HIGH for project facts).
- NESdev Wiki Bus conflict, INES, NES 2.0 and submapper pages, Tricky-to-emulate games, Holy Mapperel README, blargg mmc3_test_2 readmes (MEDIUM, from prior reading and the project's prep files; not re-fetched).
- libretro documentation for `RETRO_MEMORY_SAVE_RAM` and `retro_get_memory_data` (MEDIUM, from memory; RetroArch's exact load/save ordering and size-mismatch handling should be confirmed with the existing libretro host test).

---
*Pitfalls research for: NES emulator mapper and battery-save additions (nesturbator v2)*
*Researched: 2026-10-09*
