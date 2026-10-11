# Phase 8: MMC1 and battery saves - Context

**Gathered:** 2026-10-10
**Status:** Ready for planning

<domain>
## Phase Boundary

Mapper 1 (MMC1) cartridges load and run on the Phase 6 mapper seam, including
the SNROM, SOROM, SUROM and SXROM variants derived from header sizes
(BOARD-02). Battery RAM crosses the public API as a stable pointer plus a
write counter (SAVE-01). The runner loads and writes raw `.sav` files only
under `--save-dir`, refuses a wrong-sized file with status 4, and can flush
every N frames (SAVE-02 to SAVE-04). The libretro core exposes the span as
`RETRO_MEMORY_SAVE_RAM`, and the `retroarch-e2e` job shows a `.srm` written
and loaded back (SAVE-05). MMC3, the final MAP-03 rule set and the support
message (Phase 9), save states and signal flushing (STATE-01, STATE-02) stay
out.

</domain>

<decisions>
## Implementation Decisions

The owner chose all four areas (MMC1 ROMs and the save oracle, RAM sizing and
loader rows, the public save API, the write stamp and the RetroArch proof).
They asked for the same approach as Phase 7: a fan-out with an adversarial
pass, then one coherent set of recommendations followed without further
questions. Four researchers reported. The orchestrator checked their claims
against the code and the Holy Mapperel source and reconciled the one
conflict (which ROM RetroArch uses, D-20). The decisions below are the
result.

### Holy Mapperel MMC1 ROMs and the save oracle
- **D-01:** Commit five mapper 1 ROMs from the same v0.02 asset, pin and Zlib
  licence as Phase 7 D-01. They go raw and byte-identical under
  `tests/roms/hm/`, and each gets a manifest line and a THIRD-PARTY-NOTICES
  file entry. All five are NES 2.0, mapper 1, submapper 0.
  | File | Size | sha256 | Shape |
  |---|---|---|---|
  | `M1_P128K_CR8K.nes` | 131,088 | `0595f0896d71d6e54ad19b00a2cdfb6d1665e3166167a06935c5971105ed5793` | SGROM: 8 KiB CHR-RAM, no PRG-RAM, so $6000 is open bus |
  | `M1_P128K_C32K_W8K.nes` | 163,856 | `3d330d32bb45d41ce6c9666b80f580ff2912444b44f833ee89d062794abdfe32` | SJROM-like: 32 KiB CHR-ROM, 8 KiB volatile RAM, no battery |
  | `M1_P128K_C128K_S8K.nes` | 262,160 | `2263d212fe9c8e857f89c846962e7e78f3ba885e1413644b94e50cfff65ecd08` | SKROM: 128 KiB CHR-ROM, 8 KiB battery |
  | `M1_P512K_CR8K_S8K.nes` | 524,304 | `c3f239ea8f2fa1023a272b5e1b5add884b583e79ffd6487771cc53a1f4566b5c` | SUROM: 512 KiB PRG, 8 KiB battery |
  | `M1_P512K_CR8K_S32K.nes` | 524,304 | `7db5b5191ce842d44a5a8cb132a58d7116741053d4bd0f8a5def832be80e655a` | SXROM: 512 KiB PRG, 32 KiB battery; runs the save round-trip |

  The executor re-downloads the asset, re-verifies the asset hash and every
  file hash, and extracts with `bsdtar`, as in Phase 7. Following Phase 7 D-02,
  twins and shapes that add nothing are not committed: `M1_P128K`,
  `M1_P512K_S8K`, `M1_P512K_S32K`, `M1_P128K_C128K`, `M1_P128K_C128K_W8K`,
  `M1_P128K_C32K` and `M1_P128K_C32K_S8K`. The asset's `2k.sav`, `8k.sav` and
  `32k.sav` are all 0xFF and are not used. — **Reversibility:** one-way. A
  file committed to a public repository stays in its history; the zlib
  licence allows that.
- **D-02:** The `0000` code alone cannot prove a save came back, because it is
  `0000` with or without a save. The proof is Holy Mapperel's PRG RAM line.
  `src/wram.s` checks for `SAVEDATA` at $6100 of WRAM bank 0 before its
  pattern fill and writes the signature back afterwards. `src/main.s`
  (lines 279-317 and 651) appends ` + BATTERY` to that line only when the
  signature survived. The decoder therefore also reads the PRG RAM text row,
  which is row 6 (y 48-55) when the result is on row 8. The executor confirms
  the row on the first boot, as with Phase 7 D-05. Matching is exact text by
  column, for example `32K PRG RAM OK` or `32K PRG RAM OK + BATTERY`; a
  single `B` is not enough, because ` PROBLEM` contains one. The same row
  check also catches 32 KiB banking that falls back to 8 KiB, which the
  four-digit code hides.
- **D-03:** The decoder's glyph table grows by the letters, space and `+`
  that the PRG RAM line uses. Tile = ASCII & $3F, taken from the committed M3
  ROM's CHR and checked by the existing glyph CTest (Phase 7 D-06). The
  decoder stays the only frame oracle, and its exit statuses (Phase 7 D-07)
  are unchanged. It gains a way to print or assert the row-6 text, and the
  CTest asserts it with `PASS_REGULAR_EXPRESSION`.
- **D-04:** The SXROM CTest chain:
  1. A clean step removes and recreates the save directory
     (`FIXTURES_SETUP`).
  2. Run 1 is the runner with `--save-dir DIR`, dumping frame N.
  3. Run 1's decode asserts `0000` and `32K PRG RAM OK` with no ` + BATTERY`.
     This catches a stale save directory.
  4. A `cmake -P` script checks that the run-1 `.sav` is 32,768 bytes with
     `SAVEDATA` at offset 0x100.
  5. Run 2 uses the same command and starts from run 1's save.
  6. Run 2's decode asserts `0000` and `32K PRG RAM OK + BATTERY`.

  Both runs' frame hashes are pinned, because they differ. The other four ROMs
  use the Phase 7 dump/decode/hash pattern, plus the row-6 text: `8K PRG RAM
  OK` where there is RAM, and the missing-RAM text for SGROM. Following
  Phase 7 D-08, start at N = 600 and pin the smallest round N for which frame
  N equals frame 2N. Expect about 200 for the 8 KiB ROMs and 400-600 for SXROM.
  Run 2's `.sav` content is not asserted, because the ROM re-signs the same
  bytes.
- **D-05:** Shapes with no Holy Mapperel ROM are covered by synthetic CTests
  built with `tests/ines.h`:
  - SNROM: 8 KiB CHR-RAM and 8 KiB RAM, with CHR0 bit 4 as RAM disable.
  - SOROM: 8 KiB work RAM plus 8 KiB battery RAM, selected by CHR0 bit 3.
  - SXROM bank order: tag each of the four 8 KiB banks through CHR0 bits 3-2
    and assert span offset = bank × 8 KiB. Holy Mapperel cannot see this,
    because its fill overwrites the tags.
  - iNES 1 sizing (D-07).
  - The write rules (D-14), the reset bit, PRG-RAM enabled at power-on, and
    $E000 bit 4.

### RAM sizing and loader rows
- **D-06:** Layout. Each cartridge has one PRG-RAM allocation, made at load
  and laid out in CPU bank order: work RAM first, then battery RAM, as
  `[V bytes work][N bytes NVRAM]`. The span is `prg_ram + V` with size N.
  This follows NESdev MMC1 on SOROM: "The first RAM chip will not retain its
  data, but the second one will." It corrects
  `.planning/research/ARCHITECTURE.md` (lines 127 and 192), whose
  `[ nvram | work ]` order is wrong for SOROM. The trainer path keeps working:
  a trainer with no declared RAM still gets 8 KiB of work RAM at $7000, and a
  `.sav` copied in after load overwrites trainer bytes inside the span. —
  **Reversibility:** costly. The offset of the span decides where every
  user's `.sav` bytes land, so changing it later moves existing saves.
- **D-07:** Sizes by header case (SAVE-01 wording):
  | Header | PRG-RAM allocated | Span |
  |---|---|---|
  | iNES 1, mapper 1, PRG ≤ 256 KiB, no battery | 8 KiB work | NULL, 0 |
  | iNES 1, mapper 1, PRG ≤ 256 KiB, battery | 8 KiB | all 8 KiB |
  | iNES 1, mapper 1, PRG > 256 KiB | 32 KiB, banked by CHR0 bits 3-2 | battery: all 32 KiB; else NULL, 0 |
  | NES 2.0, V work + N NVRAM | V + N, work first | `prg_ram + V`, N; NULL, 0 when N = 0 |
  | NES 2.0, V = N = 0 | none; $6000-$7FFF is open bus | NULL, 0 |

  iNES 1 byte 8 stays unused, and v1 bytes 8-15 must stay zero, so
  DiskDude-style headers are still refused. Known gap, documented in the
  README: iNES 1 SOROM dumps get 8 KiB and need an NES 2.0 header. iNES 1
  SUROM saves are 32 KiB here, so an 8 KiB file from elsewhere is refused by
  SAVE-03, never padded.
- **D-08:** The mapper 1 row in the validator, with the RAM sizes and the
  battery flag passed to the profile check:
  - **Submapper:** 0, plus 5 (SEROM/SHROM) only with 32 KiB of PRG, treated as
    0. NESdev says it is compatible when $8000-$BFFF starts at the low bank,
    which the power-on state gives. Reject 1-4 (deprecated, and 3 is MMC1A),
    6 (2ME, EEPROM) and 7-15.
  - **Mapper 155** (MMC1A) has no row and is rejected.
  - **PRG:** a power of two from 32 to 512 KiB; 512 KiB only with 8 KiB of
    CHR.
  - **CHR:** ROM of 8-128 KiB in powers of two, or exactly 8 KiB of RAM.
    CHR-NVRAM is rejected.
  - **NES 2.0 RAM:** V and N are each 0, 8, 16 or 32 KiB, with V + N in {0, 8,
    16, 32} KiB *(amended during plan-phase 2026-10-10: a 24 KiB total is
    rejected, because no MMC1 board has it and the RAM bank select has no case
    for it; research C5)*.
    Above 8 KiB only with 8 KiB of CHR. Non-multiples of 8 KiB (byte 10 =
    `0x01`) are rejected. 16 KiB of RAM with CHR-ROM of 16 KiB or more
    (SZROM) is rejected.
  - **NES 2.0 battery bit:** must equal N ≠ 0, per the "must" wording of the
    NES 2.0 page. There is no log channel to warn through.
  - **Other boards:** mappers 0, 2, 3 and 7 keep rejecting the battery bit
    and every RAM size, as in Phase 7 D-09. Only the trainer path gives them
    RAM.

  The `nesturbator__mapper_ops_for` switch, the board-profile set test
  (Phase 7 D-10) and the error code (`NESTURBATOR_ERR_CARTRIDGE`) extend to
  {0, 1, 2, 3, 7}. MAP-03's final message stays Phase 9's.
- **D-09:** New fuzz seeds, each with a manifest line:
  - `valid-mmc1`: NES 2.0, 128K, CR8K, S8K.
  - `valid-mmc1-sorom`.
  - `mmc1-submapper-1` and `mmc1-submapper-3`.
  - `mapper-155`.
  - `mmc1-512k-chr-rom` and `mmc1-prg-16k`.
  - `mmc1-ram-64k` and `mmc1-ram-128b`.
  - `mmc1-battery-no-nvram` and `mmc1-nvram-no-battery`.
  - `mmc1-szrom`, `mmc1-chr-nvram` and `mmc1-sub5-128k`.

  Each reject also gets a cartridge-test case.
- **D-10:** MMC1 board behaviour lives in `src/mapper_mmc1.c` (one file per
  board, Phase 7 D-14). It is MMC1B: RAM is enabled at power-on, and $E000
  bit 4 disables it. Variants come from sizes only. The outer bits come from
  CHR register 0, the PITFALLS line 114 choice, documented in a comment:
  - SNROM: CHR0 bit 4 disables RAM.
  - SOROM: CHR0 bit 3 selects the RAM bank.
  - SUROM: CHR0 bit 4 selects the 256 KiB PRG half, which also moves the
    fixed bank.
  - SXROM: CHR0 bits 3-2 drive RAM A14:A13, and bit 4 selects the PRG half.

  Disabled RAM reads as open bus and drops writes. `rebuild` leaves those
  `cpu_r`/`cpu_w` pages NULL, so the bus needs no new branch. Mirroring
  control bits 0-1 are: 0 one-screen lower, 1 one-screen upper, 2 vertical,
  3 horizontal.

### Public save API (append-only; ABI stays 1)
- **D-11:** The new public declarations, after `nesturbator_reset`:
  ```c
  enum nesturbator_memory { NESTURBATOR_MEMORY_SAVE_RAM = 0 };
  typedef enum nesturbator_memory nesturbator_memory;
  nesturbator_status nesturbator_get_memory(nesturbator *inst, nesturbator_memory kind,
                                            uint8_t **data, size_t *size);
  uint64_t nesturbator_save_generation(const nesturbator *inst);
  ```
  - The out-param is `uint8_t **`, not the research draft's `void **`. A
    caller's `uint8_t *p` passed as `&p` is an incompatible pointer type in
    strict C17 and an error in C++.
  - A NULL `inst`, `data` or `size`, or an unknown kind, gives
    `NESTURBATOR_ERR_ARGUMENT` and writes nothing.
  - No cartridge, or a cartridge with no span, gives OK with NULL and 0.
  - The span keeps its address from a successful load until unload, the next
    successful load or destroy. `nesturbator_reset` and a refused load keep
    it; `src/cartridge.c` allocates the new cartridge before it frees the old
    one.
  - A SOROM exposes only its battery half.
  - The header comment states each of these rules, and also that the host
    copies a save in after load and before the first frame.

  `NESTURBATOR_MEMORY_SYSTEM_RAM` is not shipped: nothing in this phase tests
  or uses it, and adding a value later is only an append. —
  **Reversibility:** one-way. These are public symbols of a released library;
  once shipped, their names, argument types and enum values are fixed.
- **D-12:** `nesturbator_save_generation` counts CPU bus writes that reach
  the span while the board has that RAM enabled and writable, whether or not
  the byte changes. A read-modify-write counts both of its writes. The count
  runs from `nesturbator_create` and never resets or decreases, so a host
  cache cannot miss a change after a reload. A `uint64_t` counting writes
  cannot wrap in practice, and hosts compare with `!=`. These do not count:
  - loading, including trainer bytes;
  - `nesturbator_reset`;
  - unloading;
  - writes through the pointer, which the host made itself (documented).

  Writes to SOROM's work half and writes while RAM is disabled do not count.
  `nesturbator_save_generation(NULL)` returns 0. The counter is an instance
  field, so the core gains no file I/O.
- **D-13:** The guards change in the same change (rule 6):
  - `tests/core/test_api.c`
  - `tests/header/header_c.c` and `header_cxx.cpp`, so the new declarations
    compile in C and C++
  - the header baseline in `tests/cmake/vector_api_policy.cmake`
  - the README API section

  `global_symbols` and `undefined_symbols` must still pass unchanged.

### MMC1 write timing (WR-01 and WR-03 settled here)
- **D-14:** Fix Phase 6 WR-01 first. The stamp passed to `ops.cpu_write`
  becomes the zero-based index of the write's own cycle (`nes->cpu_cycle - 1`
  in `nesturbator__bus_write`). The comments in `src/bus.c`,
  `src/internal.h` and `src/mapper.h` are reconciled with it (IN-02).
  - **Tests:** `tests/core/test_mapper.c` lines 71-73 change to the
    pre-increment index. A new case asserts that the first write after load
    carries the documented absolute stamp.
  - **Hashes:** cannot move. UxROM, CNROM and AxROM all discard the stamp,
    which the orchestrator checked.
  - **WR-03:** fixed in the same plan. `nesturbator__map_cpu_read` returns
    open bus below $4020.
  - **Ledger:** both rows in `06-REVIEW-DISPOSITION.md` become `fixed`.
- **D-15:** The MMC1 register is `reg.mmc1`. It holds `shift`, `count`,
  `control`, `chr0`, `chr1`, `prg` and `uint64_t last_write`: plain integers,
  so they serialise as they are. On every hook call, including $6000-$7FFF
  RAM writes (NESdev: the ignore applies "even if that first write does not
  target the serial port"):
  1. Compute `adjacent = (last_write != 0u && stamp == last_write)`, then
     store `last_write = stamp + 1`. The update happens even when the write
     is ignored. A zeroed `last_write` means "never written".
     *(Amended during plan-phase 2026-10-10: the earlier form
     `adjacent = (stamp == last_write + 1); last_write = stamp` would drop a
     first write at stamp 1 against the zeroed field, which D-17 forbids;
     both forms agree on every later write.)*
  2. Below $8000, stop.
  3. If bit 7 is set: clear the shift register and set `control |= 0x0C`.
     This is never ignored, and the other registers, mirroring and CHR mode
     are left alone.
  4. Else, if `adjacent`, drop the write.
  5. Else shift the bit in. On the fifth write, bits 14-13 of that write's
     address choose the register.

  On a 6502, only an RMW's two writes land on consecutive cycles, and DMA
  steals cycles only on reads, so it cannot split them. The board reads no
  opcode or CPU state; a "previous opcode was RMW" shortcut is rejected
  (PITFALLS line 377).
- **D-16:** Power-on state is `control = $0C`: PRG mode 3 with the last bank
  at $C000, one-screen lower, 8 KiB CHR mode. The shift register is empty
  and PRG bit 4 is 0, so RAM is enabled. The seam rule that the loader's
  zeroed registers are the power-on state, and that `init` never writes
  registers, stays. So the stored control register is encoded so that
  zero means $0C, for example storing `control ^ 0x0C`, with a comment citing
  NESdev MMC1. The exact encoding is Claude's discretion. The rule is that a
  zeroed `reg.mmc1` is power-on and no new op is added. `nesturbator_reset`
  keeps the registers, the shift register and `last_write`; the cartridge
  sees no reset line.
- **D-17:** Write-rule tests run both through the hook and through the CPU:
  - **Hook level:** stamps (n, n+1) are ignored and (n, n+2) are accepted.
    `last_write` equals the stamp plus one after an ignored write
    *(amended during plan-phase 2026-10-10 to match D-15's stored form)*.
    The first-ever
    write at a low stamp is not adjacent to the zeroed `last_write`. A $6000
    RAM write followed by a $8000 write on the next cycle is ignored.
  - **CPU level:** synthetic PRG runs `INC $8000` three ways:
    - On a byte with bit 7 clear: the shift count rises by 1, not 2.
    - On $FF: the reset applies, then the $00 data write is ignored.
    - On $7F: the data bit is shifted in, then the $80 reset is honoured.
  - **Reset bit:** with a CHR mode and mirroring set, writing $80 leaves
    them and the CHR/PRG registers unchanged, and PRG mode becomes 3.
  - **Across `nesturbator_reset()`:** registers and battery RAM are kept, as
    success criterion 1 requires.

### Runner saves
- **D-18:** The runner flags:
  - **`--save-dir DIR`:** given at most once, not empty, and requires
    `--rom`. It is a usage error (2) with `--accuracycoin-page`.
  - **`--save-interval N`:** N ≥ 1 through the existing `parse_count`, so 0 is
    a usage error. It requires `--save-dir`.
  - **Battery-less cartridge:** `--save-dir` reads and writes nothing.
  - **Save name:** the last path component of `--rom` (split at `/` or `\` on
    every platform), minus the last extension unless the dot is the first
    character, plus `.sav`. This is RetroArch's `.srm` stem rule, so saves
    move across. Two ROMs with the same stem share a save, which is
    documented, and SAVE-03 catches it when sizes differ.
  - **Exit statuses:** 0 done, 1 failure (including any save I/O failure),
    2 usage, 4 `.sav` size mismatch. 3 and 77 stay reserved by the runner
    contract.
  - **Docs:** the comment at the top of `runner/main.c`, `usage()` and the
    README are updated together.
- **D-19:** Save I/O lives in a new `runner/save.c`. The runner may use the
  platform; the core may not.
  - **Load:** `fopen(path, "rb")` returning NULL with `ENOENT` means start
    fresh, and any other failure exits 1. If the length differs from the
    span, print `nesturbator-run: <path> is <a> bytes; the cartridge's
    battery RAM is <b> bytes`, run no frames and exit 4. The file is only
    opened read-only, so it stays byte-identical. Otherwise copy it into the
    span after `nesturbator_load_cartridge`, then keep a shadow copy and the
    generation.
  - **Flush:** after frame f when `--save-interval N` divides f (frames
    counted from 1), and once at exit, including after a JAM. A flush happens
    only if the generation changed **and** a memcmp against the shadow
    differs, so an unchanged run leaves the file and its mtime untouched.
  - **Write:** `<path>.tmp` in the same directory, then fwrite, fflush and
    fsync (`_commit` on Windows), then a checked fclose. Then `rename` on
    POSIX, or `MoveFileExA(tmp, path, MOVEFILE_REPLACE_EXISTING |
    MOVEFILE_WRITE_THROUGH)` on Windows. Rejected: `remove` then `rename`,
    which can lose the save between the two calls, and `ReplaceFile`, which
    fails on the first save.
  - **Failure:** any write or rename failure prints a message and exits 1. A
    failed rename keeps the temp file and names it.
  - **Tests:** CTest cases cover:
    - a missing file, then a created file;
    - an unchanged run leaving the file untouched;
    - a wrong size giving status 4 with the file byte-identical;
    - no `--save-dir` touching nothing;
    - `--save-interval` stopped after a flush with the current bytes on disk
      (SAVE-04), by bounding `--frames` or killing after a flush the test
      observes.

### RetroArch proof
- **D-20:** The `retroarch-e2e` proof uses the same SXROM ROM as the runner
  round-trip (D-04), so both prove the same thing with the same oracle. It
  runs two RetroArch sessions through one factored launch function in
  `tests/retroarch/run_retroarch.cmake`:
  - Each session's screenshot must equal the runner's frame for the matching
    save state: the runner with no save for session 1, and with session 1's
    `.srm` for session 2.
  - The two screenshots must differ.
  - `saves/` must be empty before session 1.
  - Exactly one `.srm` must exist afterwards, at `saves/<stem>.srm`, with
    32,768 bytes and `SAVEDATA` at 0x100.

  `test.cfg.in` gains `sort_savefiles_enable = "false"`,
  `sort_savefiles_by_content_enable = "false"`,
  `savefiles_in_content_dir = "false"`, `autosave_interval = "0"`,
  `savestate_auto_load = "false"` and `savestate_auto_save = "false"`.
  RetroArch then writes SRAM only at content unload. The planner confirms
  once that a `--max-frames` exit goes through unload; if it does not, the
  job fails loudly, not vacuously. Only the documentation (docs.libretro.com,
  `libretro/libretro.h` L498-517, the config docs) and the released binary's
  behaviour are used. RetroArch's source is GPL and is not opened (rule 4).
- **D-21:** In `libretro/libretro.c`, `retro_get_memory_data` and
  `retro_get_memory_size` with `RETRO_MEMORY_SAVE_RAM` return the
  `nesturbator_get_memory` span. Every other id, no game and no battery
  return NULL and 0. The span is filled inside `retro_load_game` and never
  again in `retro_run` or `retro_reset`, because the host copies the `.srm`
  in after `retro_load_game` returns. The libretro test program
  (`tests/libretro/libretro_host.c`) checks, for a battery image, that the
  size equals the direct API's; the pointer is non-NULL and stable across
  `retro_run` and `retro_reset`; and the span bytes equal a direct
  instance's after the same frames. For a battery-less image both sides
  give NULL and 0. No new public export is added.
  *(Amended during plan-phase 2026-10-10: the adapter's instance lives
  inside the loaded module and `abi.global_symbols` keeps its exports to
  `retro_*`, so literal pointer equality with the direct API cannot be
  observed without a new export.)* The case of a
  `.srm` with the wrong size is not part of the required e2e, because that
  is host behaviour. The researcher may measure it once with the pinned
  binary and record the result in RESEARCH.md as a non-gating note.

### Claude's Discretion
- File names inside `tests/` and the CTest names, following `holymapperel.*`,
  `runner.*` and `core.*`.
- The exact encoding that makes a zeroed MMC1 control register mean $0C
  (D-16).
- How the bus recognises a write into the span for D-12, for example a page
  range check or a flag in the derived map. It must not touch `nes->ticks`
  inside a hook or allocate.
- The exact wording of runner messages, apart from the two sizes SAVE-03
  requires.
- Whether the decoder's row-6 reader prints the text for a regex or takes an
  expected string.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Scope and requirements
- `.planning/ROADMAP.md`: the Phase 8 goal, success criteria 1-5 and research flag
- `.planning/REQUIREMENTS.md`:
  - BOARD-02 and SAVE-01 to SAVE-05 (locked wording)
  - MAP-03 (Phase 9's final message; do not pre-empt it)
  - Out of Scope: no padding or truncating `.sav` files, no save beside the
    ROM, no signal flushing
- `.planning/preparation/DECISIONS.md`: DEC.20, committed licensed test files only

### Hardware and test ROMs
- NESdev Wiki "MMC1", "SxROM", "NES 2.0", "NES 2.0 submappers" and "INES", cited in hardware comments
- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md`: MMC1 and header sections
- Holy Mapperel at `c022622274ca8b83d214dea97e4388a6b0e92d8a` (Zlib; may be read):
  - `src/wram.s`: the `SAVEDATA` signature at $6100 and the pattern fill
  - `src/main.s` lines 279-317 and 651: the PRG RAM line and ` + BATTERY`
  - `src/mmcdrivers.s` lines 25-190: the MMC1 driver and WRAM protection
  - `README.md`: what each digit means
- `.planning/research/PITFALLS.md`:
  - Pitfalls 3, 4 and 5 (lines 43, 64, 83-114)
  - lines 221 and 318 (reset, trainer)
  - lines 377-380 and 426

### API, runner and libretro
- `.planning/research/ARCHITECTURE.md`, "Battery RAM Across the API" and
  "Public Header Changes". These are corrected by D-06 (layout order) and
  D-11 (`uint8_t **`, no SYSTEM_RAM).
- `.planning/research/STACK.md` lines 80-160: libretro SRAM and the adapter
  doing no file I/O
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` section 5: runner exit statuses
- `libretro/libretro.h` L498-517: `RETRO_MEMORY_SAVE_RAM` semantics

### Seam and prior decisions
- `.planning/phases/06-mapper-seam-and-ppu-fetch-pipeline/06-REVIEW.md` and
  `06-REVIEW-DISPOSITION.md`: WR-01, WR-03 and IN-02, fixed by D-14
- `.planning/phases/07-uxrom-cnrom-and-axrom/07-CONTEXT.md`:
  - D-01 to D-10: ROM admission, the decoder, CTest chains, validator rows,
    and the switch/profile set test
  - D-14: one file per board

### Repository policy
- `tests/roms/manifest.txt`, `ASSET_POLICY.md`, `PROVENANCE.md`,
  `THIRD-PARTY-NOTICES.md` and `scripts/hygiene.sh`: how ROMs and seeds are
  admitted

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `src/mapper_uxrom.c`, `src/mapper_axrom.c`: board templates. AxROM already
  rewrites `nt[]` for single-screen mirroring.
- `nesturbator__map_prg`, `nesturbator__map_chr` and
  `nesturbator__map_prg_ram` (`src/internal.h`):
  - The first two wrap banks by true modulo.
  - `map_prg_ram` assumes one flat 8 KiB and must take a bank offset and
    allow NULL pages for disabled RAM.
- `tests/holymapperel/` (`decode.c`, `hm_decode.h`, `roms.cmake`): the decoder
  and per-ROM chain to extend.
- `tests/ines.h`: synthetic headers, which need the NES 2.0 RAM nibbles and
  the battery bit.
- `tests/mapper_test.h` and `tests/core/test_mapper.c`: hook-level tests.
- `tests/retroarch/run_retroarch.cmake`, `compare_frame.c` and `bmp_ppm.c`:
  the e2e steps to factor into a launch function.

### Established Patterns
- Validator-first loading: nothing is allocated until `validate_image`
  passes, and the new cartridge is allocated before the old one is freed.
- A zeroed register block is the power-on state, and `init` sets only
  `map.watch`.
- Hooks receive time as an argument, never read `nes->ticks` and never
  allocate.
- Every committed binary has a manifest line, and every behaviour change
  ships with its tests and its README and header text.

### Integration Points
- `src/bus.c:31,175`: the stamp fix (D-14). The write path also bumps the
  generation (D-12).
- `src/cartridge.c`: `board_profile_ok`, the NES 2.0 RAM rejections at about
  line 103-124, the RAM allocation at about line 220-246, and the switch.
- `src/instance.c`: `nesturbator_get_memory` and
  `nesturbator_save_generation`. `nesturbator_reset` keeps the span.
- `include/nesturbator.h`: the declarations and the support-set comments
  (lines around 70, 163, 184 and 239-254).
- `runner/main.c` and the new `runner/save.c`, `libretro/libretro.c:306-318`,
  `.github/workflows/ci.yml` (`retroarch-e2e` at about line 175).

</code_context>

<specifics>
## Specific Ideas

- The owner prefers "another copy-paste is better than another dependency".
  There is no 7z or zlib reader, no cc65 build, and no helper library for the
  save I/O or the decoder.
- The runner's `.sav` and RetroArch's `.srm` for the same cartridge are the
  same raw bytes, so they can be used interchangeably. D-20 proves it by
  feeding session 1's `.srm` to the runner.
- Delivery: this phase ships a release that plays MMC1 games and keeps their
  saves.
- PR #32 (Phase 7) is still a draft and unmerged. The Phase 8 branch must
  start from a `main` that includes Phase 7.

</specifics>

<deferred>
## Deferred Ideas

- `NESTURBATOR_MEMORY_SYSTEM_RAM` and `RETRO_MEMORY_SYSTEM_RAM`, for cheats
  and achievements. Append it when a host needs it.
- SZROM (16 KiB RAM with large CHR-ROM), MMC1A (mapper 155), 2ME (submapper
  6, EEPROM) and the deprecated submappers 1-4. These are next-tier.
- iNES 1 SOROM heuristics, such as a database or a 32 KiB fallback. These
  belong to a header-repair or database decision. NES 2.0 covers SOROM now.
- RetroArch's handling of a wrong-sized `.srm`: measure it only, never gate
  on it.
- A host acknowledgement call for saves (SEED-002), flushing on a signal and
  more exit statuses (STATE-02).
- Phase 7 WR-01 (the AxROM power-on wording and a soft-reset test) and WR-02
  (the hash inventory keys) are still open. They can be fixed with
  `/gsd-quick` before PR #32 merges, or folded into this phase's first plan if
  the owner prefers.

</deferred>

---

*Phase: 08-mmc1-and-battery-saves*
*Context gathered: 2026-10-10*
