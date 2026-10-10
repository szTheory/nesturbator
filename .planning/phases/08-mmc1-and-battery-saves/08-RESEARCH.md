# Phase 8: MMC1 and battery saves - Research

**Researched:** 2026-10-10
**Domain:** MMC1 (mapper 1) board on the Phase 6 mapper seam; NES 2.0 RAM sizing; battery-save API (`nesturbator_get_memory`, `nesturbator_save_generation`); runner `.sav` files; libretro `RETRO_MEMORY_SAVE_RAM`; RetroArch two-session proof
**Confidence:** HIGH. Repo facts were read this session. MMC1 and header facts come from NESdev pages fetched this session. Holy Mapperel facts come from its source at the pin, plus its ROMs downloaded and hashed. RetroArch behaviour was measured on the pinned 1.22.2 binary.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

The owner chose all four areas (MMC1 ROMs and the save oracle, RAM sizing and
loader rows, the public save API, the write stamp and the RetroArch proof).
They asked for the same approach as Phase 7: a fan-out with an adversarial
pass, then one coherent set of recommendations followed without further
questions. Four researchers reported. The orchestrator checked their claims
against the code and the Holy Mapperel source and reconciled the one
conflict (which ROM RetroArch uses, D-20). The decisions below are the
result.

#### Holy Mapperel MMC1 ROMs and the save oracle
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

#### RAM sizing and loader rows
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
  - **NES 2.0 RAM:** V and N are each 0, 8, 16 or 32 KiB, with V + N ≤ 32 KiB.
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

#### Public save API (append-only; ABI stays 1)
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

#### MMC1 write timing (WR-01 and WR-03 settled here)
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
  1. Compute `adjacent = (stamp == last_write + 1)`, then set
     `last_write = stamp`. The update happens even when the write is ignored.
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
    `last_write` equals the stamp after an ignored write. The first-ever
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

#### Runner saves
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

#### RetroArch proof
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
  (`tests/libretro/libretro_host.c`) checks that the pointer and size equal
  the direct API's for a battery and a battery-less image. The case of a
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

### Deferred Ideas (OUT OF SCOPE)
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
</user_constraints>

## Project Constraints (from CLAUDE.md)

Directives from `./CLAUDE.md` and `./.claude/CLAUDE.md` that bind the plans:

- **Own code, clean room.** All emulation and test code is written here. `libretro.h` is the only vendored file. The core links only the C memory functions (`memcpy memmove memset memcmp malloc free`, allowlist in `tests/cmake/undefined_symbols.cmake`). Do not open GPL or LGPL emulator source (RetroArch source included). Holy Mapperel is Zlib and may be read.
- **Three deliverables only.** Library, runner, libretro adapter. No window, audio-device or input-device code.
- **Clean tree.** No ROM bytes except files listed in `tests/roms/manifest.txt`. No personal paths, emails or names. `scripts/hygiene.sh` rejects a ROM or save file name that is not listed. Tests that write `.sav` or `.srm` files write them only under the build tree at test time.
- **Deterministic, integer-only core.** All state in the instance. No float.
- **Tested and documented together.** Every behaviour change lands with a test run by `cmake --workflow --preset ci`, and with README and public-header comment updates in the same change. Checks are automated; none waits on a person.
- **One GSD step per command.** Work on a phase branch (owner creates it); merge by pull request with a Conventional Commit title.
- **Style.** C17, extensions off, fixed-width types, small modules, plain control flow. A hardware comment says what the hardware does and cites its source. Follow the ponytail ladder: remove the need, reuse, standard library, platform, then minimum new code.
- **Warnings.** `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wstrict-prototypes -Wmissing-prototypes -Wvla` on GNU/Clang/AppleClang [VERIFIED: CMakeLists.txt:20]; `CMAKE_C_EXTENSIONS OFF` [VERIFIED: CMakeLists.txt:14].

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| BOARD-02 | MMC1 games run: Holy Mapperel mapper 1 ROMs pass; synthetic tests for the consecutive-write ignore, reset bit, RAM enabled at power-on, SNROM/SOROM/SUROM/SXROM from sizes | NESdev MMC1 rules verified (Standard Stack / Patterns 1-3); five ROM hashes and headers verified; HM source shows what each ROM exercises; bank math and encoding in Code Examples; glyph-reference pitfall (Pitfall 2) |
| SAVE-01 | `nesturbator_get_memory()` span, NULL/0 otherwise, pointer stable load to unload, `nesturbator_save_generation()` counts writes that reach the span | Layout `[V][N]`, allocation order, write counting site `src/bus.c`, instance field, header comments (Patterns 4-5; Code Examples) |
| SAVE-02 | `--save-dir DIR` loads/writes `.sav` through tmp + rename only if changed; HM battery ROM passes on second run; no `--save-dir` means no I/O | `runner/save.c` design, C17 feature-macro pitfall, Windows API verified, HM two-run chain mechanics verified from `src/wram.s` |
| SAVE-03 | Wrong-length `.sav`: print both sizes, exit 4, file byte-identical | Read-only open, size check before any frame; exit-code table (4 is free: 3 and 77 reserved) |
| SAVE-04 | `--save-interval N` flushes changed span every N frames; test stops the runner after a flush and finds current bytes | CMake `execute_process(TIMEOUT)` kill test, verified locally (Pattern 8) |
| SAVE-05 | libretro core returns the span for `RETRO_MEMORY_SAVE_RAM`; `retroarch-e2e` shows `.srm` written and loaded in a second session | RetroArch 1.22.2 measured: `--max-frames` exit writes `.srm` at unload, host loads it after `retro_load_game`; probe transcript below; pointer-equality caveat for `libretro_host.c` |
</phase_requirements>

## Summary

Phase 8 is an implementation phase whose design is already locked by CONTEXT.md D-01..D-21. This research confirmed those decisions against the code, the NESdev pages, the Holy Mapperel source and the pinned RetroArch binary, and found nine places where a decision as worded would fail or mislead the executor. They are collected under "Findings that sharpen CONTEXT" below. None reopens a locked decision. Each is a detail the plans must carry, and the three most likely to cost a re-plan are: the decoder's "lit means differs from pixel (0,0)" rule breaks on the glyph `M` (it is in `PRG RAM`), the zeroed `last_write` collides with a legitimate stamp of 1, and the libretro host test cannot compare the core's pointer with a second instance's pointer.

The existing seam fits MMC1 with small additions: a `reg.mmc1` union member, a 4 KiB CHR helper, a banked PRG-RAM mapping helper, a span range check in `nesturbator__bus_write`, and cartridge fields for the RAM allocation and span. The loader work is the largest risk surface: the generic early rejects of the battery bit and of every RAM size in `validate_image` must become per-board decisions without loosening mappers 0, 2, 3 and 7. The Holy Mapperel ROMs, downloaded and hashed this session, match D-01 byte for byte and their headers match the shapes in D-01.

The RetroArch open items are closed by measurement on the pinned 1.22.2 binary with a throwaway probe core: SRAM is written once at content unload when the run ends through `--max-frames`, a `.srm` is loaded after `retro_load_game` returns and before frame 1, and a wrong-size `.srm` is silently truncated or zero-padded and rewritten at the core's size. The e2e design in D-20 is sound.

**Primary recommendation:** Plan five plans in this order: (1) seam fixes, `tests/ines.h` extension, loader profile/RAM layout and the save API with its guards; (2) the MMC1 board with hook-level and CPU-level tests and fuzz seeds; (3) the five ROMs, the decoder row reader and the CTest/hash chains; (4) `runner/save.c`, flags and tests including the kill test; (5) the libretro memory exports, `libretro_host.c` checks, the two-session e2e, `.info`, README and docs.

## Findings that sharpen CONTEXT

These are additions or corrections to how a decision should be executed. The planner should carry each into a task.

| # | Finding | Decision touched | Action |
|---|---------|------------------|--------|
| C1 | The decoder's cell rule "a pixel is lit when it differs from the cell's (0,0) pixel" fails for `M`: tile `$0D` row 0 is `0xc6`, so bit 7 is set and pixel (0,0) is lit. The mask inverts and matches no glyph. Rows 5-7 are blank in every glyph needed. [VERIFIED: computed this session from the committed `M3_P32K_C32K_H.nes` CHR, tile `$0D` rows `c6 ee fe d6 c6 00 00 00`] | D-03 | Take the background reference from pixel (0,7) of the cell (row 7, column 0), not (0,0). Existing `0`-`F`, `D`, `E` cells still match. Add a unit case painting `M`. |
| C2 | `last_write` zeroed means "a write at stamp 0". With D-14's zero-based stamps, a first write at stamp 1 is adjacent to the zeroed field. D-17 asks for "first-ever write at a low stamp is not adjacent". | D-15, D-17 | Store `stamp + 1` in `last_write` (0 means never written). `adjacent = last_write != 0 && stamp + 1 == last_write + 1`. A zeroed block stays power-on. |
| C3 | Two instances never share a pointer, so "the pointer and size equal the direct API's" cannot be tested by comparing the core's pointer with a separately created instance's pointer. | D-21 | In `libretro_host.c` check: size equals the expected span size, the pointer is non-NULL for a battery image and stays the same across `retro_run`, and the bytes after N frames equal the bytes a direct instance has after the same N frames of the same synthetic ROM. |
| C4 | SOROM (16 KiB total RAM) must bank on CHR0 bit 3 only. A generic `(chr0 >> 2) & 3` taken modulo the bank count gives bit 2 for 2 banks. NESdev: "SOROM implements only this bit [bit 3]". [VERIFIED: nesdev.org/wiki/MMC1, fetched this session] | D-10, D-05 | Select by total RAM size: 8 KiB or less no banking; 16 KiB bank = `(chr0 >> 3) & 1`; 32 KiB bank = `(chr0 >> 2) & 3`. |
| C5 | D-08 allows V and N each in {0, 8, 16, 32} with V + N at most 32, which admits a 24 KiB total (8+16, 16+8). No board has 24 KiB and the bank formula has no case for it. | D-08 | Require V + N in {0, 8, 16, 32} KiB. Reject 24. Any split inside a legal total is accepted (the layout is `[V][N]` regardless). |
| C6 | `fsync` and `fileno` are POSIX, not ISO C. With `CMAKE_C_EXTENSIONS OFF` the compiler gets `-std=c17`, and glibc hides them unless a feature macro is defined. [ASSUMED: glibc `feature_test_macros(7)` behaviour; not run on Linux here] | D-19 | Put `#define _POSIX_C_SOURCE 200809L` at the top of `runner/save.c` before any include. Windows branch includes `<io.h>` and `<windows.h>`. The Linux CI leg is the check. |
| C7 | The validator rejects the battery bit and bit 3 together at `validate_image` (`if ((image[6] & 0x0au) != 0u ...) return 0;`) and again for NES 2.0 (`if (image[6] & 2u) return 0;`), and rejects any PRG-RAM or PRG-NVRAM size (`if (prg_ram != 0u || prg_nvram != 0u || chr_nvram != 0u) return 0;`). [VERIFIED: src/cartridge.c:98, 117, 123] | D-08 | Reject only the four-screen bit (`0x08`) generically. Carry `battery`, `prg_ram`, `prg_nvram`, `chr_nvram` and `nes2` into `board_profile_ok`. Boards 0, 2, 3, 7 keep rejecting battery and every RAM size, so the existing rows `uxrom-battery`, `cnrom-battery`, `axrom-battery` and `prg-ram` stay BAD. |
| C8 | The existing row `{"mapper-1", 1, 0, 0, 1, 1, ...BAD_}` has 16 KiB PRG. Under D-08 it stays BAD for the PRG-size reason. [VERIFIED: tests/core/test_cartridge.c:353] | D-09 | Rename it `mmc1-prg-16k` and add real accept rows. `test_board_switch_matches_profiles` wants `{0,1,2,3,7}` and `boarded == 5` (today `want = id == 0u || id == 2u || id == 3u || id == 7u;` and `CHECK_EQ_U64(boarded, 4u);`). [VERIFIED: tests/core/test_cartridge.c, function `test_board_switch_matches_profiles`] |
| C9 | `vector_api_policy.cmake` pins a SHA-256 of the comment-stripped, whitespace-stripped declaration stream. Any header edit that adds a declaration changes it, and any later edit to a declaration changes it again. [VERIFIED: tests/cmake/vector_api_policy.cmake:3-28] | D-13 | Finish the header declarations first, run the policy test, copy the printed `found` hash into the constant, and rename the constant (it says "Phase 6"). Comment-only edits do not move the hash. |
| C10 | `libretro/nesturbator_libretro.info` still has `libretro_saves = "false"` and a description that says "Phase 1 shows a built-in test card". [VERIFIED: libretro/nesturbator_libretro.info:18 `libretro_saves = "false"`, :23 `description = "Accuracy-focused NES core. Phase 1 shows a built-in test card."`] No test pins either line; `version_consistency.cmake` reads only `display_version`. | D-21 | Set `libretro_saves = "true"` and refresh the description in the same change. What RetroArch does with the flag is [ASSUMED] to be display only; the probe wrote and loaded a `.srm` without any `.info`. |

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| MMC1 registers, banking, mirroring, write-ignore | Core library (`src/mapper_mmc1.c`) | — | Board behaviour; deterministic, integer only, no I/O |
| PRG-RAM allocation, `[V][N]` layout, span | Core library (`src/cartridge.c`) | — | Loader owns the allocation; the span is a view into it |
| Header validation and board profiles | Core library (`src/cartridge.c`) | Fuzz corpus, cartridge tests | Rejects before any allocation |
| Write stamp and span write counting | Core library (`src/bus.c`) | — | The bus sees every CPU write; hooks must not touch time |
| Save span handout and generation | Public API (`include/nesturbator.h`, `src/instance.c`) | README | Core never does file I/O |
| `.sav` read, size check, atomic write, interval | Runner (`runner/save.c`, `runner/main.c`) | CTest | The runner may use the platform; the core may not |
| Save RAM exposure to a host | libretro adapter (`libretro/libretro.c`) | `.info` | The host owns `.srm` file I/O |
| `.srm` write/load behaviour | RetroArch (host) | `retroarch-e2e` job | Measured, not implemented here |
| Result-screen oracle | Test code (`tests/holymapperel/`) | — | Frame is the only input; the runner knows nothing of Holy Mapperel |

## Standard Stack

No external package is added. Everything is in-repo C, CMake and the tools already used by Phases 1-7.

### Core
| Library / tool | Version | Purpose | Why Standard |
|----------------|---------|---------|--------------|
| C17, extensions off | `CMAKE_C_STANDARD 17` | Core, runner, adapter | Project rule [VERIFIED: CMakeLists.txt:12-14] |
| CMake / CTest | 3.25 or newer (4.4.3 present locally) | Presets, tests, fixtures, `execute_process` kill test | Project build [VERIFIED: `cmake --version`] |
| `tests/check.h`, `tests/ines.h`, `tests/mapper_test.h` | in-repo | Test macros, synthetic images, hook test board | Existing pattern; `ines.h` needs the NES 2.0 RAM nibbles and battery bit |
| Holy Mapperel v0.02 ROMs | commit `c022622274ca8b83d214dea97e4388a6b0e92d8a`, Zlib | MMC1 oracle | D-01 |
| RetroArch | 1.22.2, DMG sha256 `81b79121ba26d539064ae13b4d0419a120c3d165afbe656cf5f5412b15fdb434` | e2e host | Already pinned in `ci.yml`; sha256 re-verified this session |

### Supporting
| Tool | Purpose | When to Use |
|------|---------|-------------|
| `bsdtar` | Extract the 7z asset | Re-verification step of D-01; worked on this Mac |
| `MoveFileExA`, `_commit` | Windows atomic replace and flush | `runner/save.c` Windows branch |
| `fsync`, `rename` | POSIX flush and atomic replace | `runner/save.c` POSIX branch |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Span range check in `bus.c` | A 48-bit per-page "save page" mask in `struct nesturbator__map`, set by rebuild | The mask costs a rebuild-time loop and a field but no pointer compares; the range check costs two compares per RAM write. Both are allowed by Discretion. Recommended: range check, guarded by `save_size != 0`, because every `cpu_w` page points into the single `cart.bytes` allocation so the compare is well defined. |
| Kill test via CMake `TIMEOUT` | A C driver using `fork`/`TerminateProcess` | CMake `execute_process(TIMEOUT)` terminates the child on every platform and was verified locally; no new C code. |

**Installation:** none.

## Package Legitimacy Audit

This phase installs no external package. `gsd-tools query package-legitimacy check` was not run because there is nothing to check. The only downloaded artifacts are the Holy Mapperel asset (sha256 re-verified, below) and the RetroArch DMG (sha256 re-verified); both are pinned and hash-checked by existing project policy.

| Package | Registry | Age | Downloads | Source Repo | Verdict | Disposition |
|---------|----------|-----|-----------|-------------|---------|-------------|
| (none) | — | — | — | — | — | — |

**Packages removed due to [SLOP] verdict:** none
**Packages flagged as suspicious [SUS]:** none

## Verified Inputs (this session)

### Holy Mapperel asset and the five ROMs [VERIFIED: downloaded, hashed and parsed this session]

Asset `holy-mapperel-bin-0.02.7z`: 17,964 bytes, sha256 `70f85671e21f293599baebb662faeb06a4c04e9c9ceb283d96d4197f09e4ce7a` (matches Phase 7 D-01). `bsdtar -xf` extracts to `testroms/`. All five D-01 hashes and sizes match.

First 16 bytes of each committed ROM, verbatim:

| File | Header bytes 0-15 | Reading |
|------|-------------------|---------|
| `M1_P128K_CR8K.nes` | `4e45 531a 0800 1008 0000 0007 0000 0000` | PRG 8×16 KiB, CHR units 0 (RAM), byte 6 `10`, byte 7 `08` (NES 2.0), byte 10 `00`, byte 11 `07` (8 KiB CHR-RAM) |
| `M1_P128K_C32K_W8K.nes` | `4e45 531a 0804 1008 0000 0700 0000 0000` | CHR 4×8 KiB, byte 10 `07`: 8 KiB volatile PRG-RAM, no battery |
| `M1_P128K_C128K_S8K.nes` | `4e45 531a 0810 1208 0000 7000 0000 0000` | CHR 16×8 KiB, byte 6 `12` (battery), byte 10 `70`: 8 KiB NVRAM |
| `M1_P512K_CR8K_S8K.nes` | `4e45 531a 2000 1208 0000 7007 0000 0000` | PRG 32×16 KiB, battery, byte 10 `70`, byte 11 `07` |
| `M1_P512K_CR8K_S32K.nes` | `4e45 531a 2000 1208 0000 9007 0000 0000` | byte 10 `90`: NVRAM shift 9 = `64 << 9` = 32,768 bytes |

All five have submapper nibble 0 (byte 8 = `00`) and no trainer. The 8 KiB battery ROMs and the volatile ROM would be rejected by today's validator at `prg_ram != 0u || prg_nvram != 0u`. The unselected twins in D-01 were also present in the asset with the expected sizes.

### Holy Mapperel screen and WRAM protocol [VERIFIED: src/main.s, src/wram.s, src/mmcdrivers.s read at the pin this session]

- Row numbering: `lda #4; sta txt_y` then one `start_line` per line. Line order is mapper (row 4), PRG ROM (row 5), PRG RAM (row 6), CHR (row 7), detailed result (row 8). `start_line` sets the PPU address low byte with `ora #$02`, so text starts at column 2 (x = 16). Row 6 is y 48-55. D-02's row is confirmed by source; the executor still confirms it on the first dump.
- Strings, verbatim from `src/main.s` lines 646-654: `k_prg_rom: .byte "K PRG ROM",0`, `k_prg_ram: .byte "K "`, `msg_prg_ram: .byte "PRG RAM",0`, `msg_missing: .byte " MISSING",0`, `k_chr: .byte "K CHR R",0`, `msg_battery: .byte " + BATTERY",0`, `msg_ok: .byte " OK",0`, `msg_problem: .byte " PROBLEM",0`, `msg_detailed: .byte "DETAILED TEST RESULT: ",0`.
- The PRG RAM line is the decimal size with no padding (`write_decimal_in_0` skips leading zeros), then `K PRG RAM`, then ` OK` or ` PROBLEM`, then ` + BATTERY` only if `has_savedata`. With no RAM it prints `PRG RAM MISSING` and no number. So the exact lines are: `PRG RAM MISSING`, `8K PRG RAM OK`, `32K PRG RAM OK`, `32K PRG RAM OK + BATTERY`. The size is `(last_wram_bank + 1) * 8`.
- `wram_test`: select RAM bank 0, `verify_savedata` compares `SAVEDATA` at `$6100`; `has_savedata` is that result. It then tests that `$60FF` holds `$AA`/`$55` (else `has_wram = 0`), sprays bank tags 15 down to 0 into `$60FF` (so a 4-bank RAM reports `last_wram_bank = 3`), fills and verifies all of `$6000-$7FFF` in every bank for 8 rounds, then selects bank 0 and writes `SAVEDATA` back to `$6100`. After a first run the RAM therefore holds the last fill pattern plus the signature; run 2 re-signs identical bytes, so its flush finds the span unchanged.
- No-RAM detection reads `$60FF` after writing `$AA`; on open bus the read returns the high operand byte `$60`, so `$AA ^ $60 != 0` and `has_wram = 0`. The core's `open_bus` latch is updated by every read, so a NULL page returns `$60` here.
- MMC1 driver (`src/mmcdrivers.s`): the RAM-select helper writes `((bank & 3) << 1)`, and `mmc1_set_chr_8k` ORs `$08` for 512 KiB PRG, masks to 4 bits and shifts once more, so the value written to CHR0 has the RAM bank in bits 3-2 and `$10` (bit 4) set. This is consistent with D-10 (bits 3-2 RAM bank, bit 4 PRG half).
- Protection test (`mmc1_test_wram_protection`): `$E000` bit 4 (`$1F` to the PRG register) must drop a write to `$6000` on every board with RAM. The SNROM test writes `$10` to CHR0 and then `$80` to `$6000`: it expects the write to be dropped only for CHR-RAM boards with PRG at or below 256 KiB (`ldy is_chrrom; bne ...; ldy last_prg_bank; cpy #64; bcs ...` then expected `$6B`), and expects it to land on CHR-ROM boards and on 512 KiB boards. So CHR0 bit 4 must not disable RAM on `M1_P128K_C32K_W8K`, `M1_P128K_C128K_S8K` or the two 512 KiB ROMs; the committed set never exercises the SNROM disable itself, so the synthetic SNROM test (D-05) is the only proof.
- PRG window test covers modes `$0C, $08, $04, $00` with CHR0 bit 4 stepped through the 512 KiB range; it checks that the outer bit applies to the fixed bank too (mode 2 and 3). The CHR test sets control `$1C` (4 KiB CHR mode, PRG mode 3) and writes CHR0 and CHR1 with indirect stores.

### NESdev MMC1 [VERIFIED: nesdev.org/wiki/MMC1 fetched this session; quotes verbatim]

- "When the serial port is written to on consecutive cycles, it ignores every write after the first." "This restriction only applies to the data being written on bit 0; the bit 7 reset is never ignored."
- Bit 7 write: clears the shift register and writes Control with (Control OR `$0C`); "The CHR-ROM bank mode and nametable arrangement are not altered."
- Fifth write copies into the register "selected by bits 14 and 13 of the address."
- Revision notes: MMC1A "PRG-RAM is always enabled at $6000-$7FFF"; MMC1B "PRG-RAM is enabled by default but can by disabled by bit 4 of $E000."
- CHR0 outer bits: bit 4 "Select 256 KB PRG-ROM bank" on SOROM/SUROM/SXROM and the PRG-RAM disable on SNROM; bits 3-2 "Select 8 KB PRG-RAM bank"; "SOROM implements only this bit [bit 3]"; on SXROM bit 3 "selects the SRAM's A14", bit 2 "selects A13". The 256 KB selection "applies to all the PRG area, including the normally 'fixed' bank."
- Mirroring bits 1-0: 0 one-screen A, 1 one-screen B, 2 vertical mirroring (PPU A10), 3 horizontal mirroring (PPU A11). The wiki's "horizontal arrangement"/"vertical arrangement" wording is the reverse of the mirroring name; D-10's table (2 vertical, 3 horizontal) matches the mirroring names.
- SOROM: "The first RAM chip will not retain its data, but the second one will." SZROM is "8 KiB PRG-RAM, 8 KiB PRG-NVRAM, and 16 KiB or more of CHR" in a NES 2.0 header. Without NES 2.0 the page suggests assuming 32 KiB of PRG-RAM.

### NES 2.0 header [VERIFIED: nesdev.org/wiki/NES_2.0 and /INES fetched this session]

- Byte 10 low nibble PRG-RAM (volatile) shift, high nibble PRG-NVRAM/EEPROM shift; byte 11 the same for CHR. "If the shift count is non-zero, the actual size is `64 << shift count` bytes." This matches `nes2_ram_size` in `src/cartridge.c:46-56` and the nibble use at lines 112-115.
- "When the upper nibble (PRG-NVRAM/EEPROM) has a non-zero value ... the Battery bit ... must always be set" and "if the Battery bit is set, the upper nibble must have a non-zero value" unless the memory is chip-internal or similar. D-08's equality rule holds for MMC1.
- Trainer: 512 bytes "to be loaded into CPU memory at $7000". iNES byte 8: "Value 0 infers 8 KB for compatibility."

### RetroArch 1.22.2 measurements [VERIFIED: measured this session on the pinned binary]

Setup: the pinned DMG (sha256 `81b79121ba26d539064ae13b4d0419a120c3d165afbe656cf5f5412b15fdb434`, version 1.22.2) was downloaded into its own directory, mounted read-only and copied. A throwaway libretro core (kept outside the repo, built from the vendored `libretro.h`) exposes a 32,768-byte `RETRO_MEMORY_SAVE_RAM`, writes `SAVEDATA` at 0x100 on frame 3, and logs what it sees at `retro_load_game`, frames 1-2 and unload. RetroArch ran with the repo's `tests/retroarch/test.cfg.in` plus the six D-20 keys, a redirected `HOME`, and `-L core content/game.nes --max-frames=N`. The user's real RetroArch directory was not modified (no file there changed in the period).

| Probe | Result |
|-------|--------|
| Session 1, empty `saves/`, `--max-frames=10` | exit 0; `saves/game.srm` created, 32,768 bytes, `SAVEDATA` at 0x100. Stem rule confirmed: `content/game.nes` gives `game.srm`. Log order: `load_game`, `unload_game`, `deinit`. |
| Ordering | In session 2 the core saw an empty buffer right after `retro_load_game` returned and the saved bytes at frame 1. The host copies the `.srm` in after `retro_load_game` and before the first `retro_run`. |
| Session 2 | Loaded bytes visible at frame 1; at exit the `.srm` was rewritten (new sha256) with the frame-3 change. |
| Wrong size, 8,192-byte `.srm` | Loaded without a message. The first 8,192 bytes were copied (the `0xAABB` marker and `SAVEDATA` at 0x100 were visible), the rest stayed as the core had it, and at exit the file was rewritten at 32,768 bytes. |
| Wrong size, 40,000-byte `.srm` | Loaded without a message. The first 32,768 bytes were used and at exit the file was rewritten at 32,768 bytes, so the tail was dropped. |
| Core size 0 | No `.srm` created. |
| Time | A 600-frame session took about 6 s wall clock including startup, so the 50 s `TIMEOUT` in `run_retroarch.cmake` has room for the SXROM frame number. |

This answers the Phase 8 research flag. RetroArch never refuses a wrong-sized `.srm`; it truncates or pads silently and rewrites at the core's size. That is host behaviour and, per D-21, not gated. The runner's refusal (SAVE-03) is stricter by design.

The six D-20 keys were not needed for these results (defaults gave the same files), and RetroArch ignores unknown keys, so a misspelled key would be silent. The e2e assertions (saves empty before session 1, exactly one `.srm`, at `saves/<stem>.srm`) are the real guard. The key names come from CONTEXT D-20 and the existing `test.cfg.in` convention [CITED: CONTEXT D-20].

### Repo facts relied on [VERIFIED: read this session]

- `nesturbator__bus_write` calls `cycle(nes)` first, which increments `nes->cpu_cycle`, then passes `nes->cpu_cycle` to the hook: `nes->map.ops.cpu_write(nes, addr, v, nes->cpu_cycle);` [src/bus.c:175]. The fix is `nes->cpu_cycle - 1u`.
- `nesturbator__map_cpu_read` is `const uint8_t *page = nes->map.cpu_r[(addr - 0x4000u) >> 10];` with no range guard [src/internal.h:260-264]. WR-03 fix: return `nes->bus.open_bus` when `addr < 0x4020u`.
- Writes: the page store happens first, then the hook is called for any address at or above `$4020` when `watch & NESTURBATOR_WATCH_CPU_WRITE` [src/bus.c:163-177]. A RAM write to `$6000-$7FFF` therefore reaches the MMC1 hook with the stamp, as D-15 needs.
- `cpu_reset` uses only `nesturbator__bus_read` for its seven cycles, so a soft reset produces no hook calls [src/cpu.c:1367-1376]. `rmw()` writes the old value and then the new value, both through `nesturbator__bus_write` [src/cpu.c:369-372].
- `nesturbator_load_cartridge` allocates the new image first and frees the old one after it succeeded [src/cartridge.c:227-234], so D-11's pointer rule holds already. Trainer RAM is `copy + size`, 8 KiB, zeroed, with the trainer at `+0x1000` [src/cartridge.c:241-246]. The allocation is one block holding image, trainer RAM, and CHR-RAM, so every `cpu_w` page pointer lies inside `cart.bytes`.
- `nesturbator_unload_cartridge` memsets `cart`, `mapper`, `map`, and zeroes `cpu_cycle` [src/cartridge.c:151-169]. `nesturbator_reset` does not touch `cart`, `mapper` or `map` [src/instance.c:158-176]. A new `save_generation` field on `struct nesturbator` is zeroed at create by the instance memset [src/instance.c:103] and is not touched by load or unload.
- Mapper hook comment: "cpu_cycle is the index of the write's cycle" [src/mapper.h:33]. `board_profile_ok(mapper, submapper, prg_size, chr_size, chr_is_ram)` has the signature at [src/cartridge.c:60-61]. `nesturbator__mapper_ops_for` is the only board switch [src/cartridge.c:173-191].
- The runner's `main.c` already parses `--rom` and `--accuracycoin-page`, loads the ROM with `fopen`/`fread`, and runs frames in a `uint64_t` loop [runner/main.c:630-737]; there is no save code. `usage()` and the top comment list "mappers 0, 2, 3 and 7" [runner/main.c:7, 46, 659-660].
- libretro adapter: `retro_get_memory_data`/`retro_get_memory_size` return NULL/0 [libretro/libretro.c:306-317]; `retro_load_game` creates the instance and loads the image [libretro/libretro.c:257-283].
- Speed: 600 frames of an existing Holy Mapperel ROM take 0.58 s with the `ci` build and 9.9 s with the `asan` build, and frame 600 of `M7` hashes equal to frame 100 (static result screen) [VERIFIED: ran `build/ci/runner/nesturbator-run` and `build/asan/runner/nesturbator-run`].

## Architecture Patterns

### System Architecture Diagram

```
 ROM file (.nes)                                   .sav / .srm  (raw bytes = the span)
      |                                                  ^   |
      v                                                  |   | read once, after load,
 nesturbator_load_cartridge ----> validate_image ----+   |   | before the first frame
      |             (board_profile_ok: per-board rows;  |   |
      |              RAM sizes, battery bit, submapper) |   v
      |   allocate [image][trainer/PRG-RAM = V+N][CHR-RAM]   copy into span
      v
 struct cart { prg_ram, prg_ram_size, save = prg_ram+V, save_size = N }
      |
      v
 CPU bus write ($6000-$FFFF) --> nesturbator__bus_write
      |       |                       |
      |       |  page != NULL --> store byte; if page inside [save, save+N): save_generation++
      |       v
      |   ops.cpu_write(addr, value, cpu_cycle - 1)   <- only if watch bit set
      |       |
      |       v   mapper_mmc1: adjacent? reset bit? shift 5 bits? commit register
      |       v   rebuild(): PRG pages, CHR 4K/8K pages, nt[], PRG-RAM page (NULL when disabled)
      v
 CPU bus read ($4020-$FFFF) --> map.cpu_r[page] or open bus (NULL page)

 Public API:  nesturbator_get_memory(inst, SAVE_RAM, &p, &n)   nesturbator_save_generation(inst)
      |                         |
      v                         v
 runner/save.c            libretro/libretro.c
  flush if gen changed     retro_get_memory_data/size -> same span
  and memcmp differs       (host reads/writes .srm itself)
  tmp + fsync + rename
```

### Recommended Project Structure
```
src/
├── mapper_mmc1.c        # new: MMC1 board, one file per board (Phase 7 D-14)
├── mapper.h             # + reg.mmc1, + nesturbator__mapper_mmc1_ops
├── internal.h           # + cart fields, + save_generation, + map_chr_4k, + banked PRG-RAM helper, WR-03 guard
├── bus.c                # stamp fix, span write counting
├── cartridge.c          # profile signature, RAM layout, switch case 1
└── instance.c           # nesturbator_get_memory, nesturbator_save_generation
include/nesturbator.h    # enum nesturbator_memory + two functions + comments (end of header, after nesturbator_reset)
runner/
├── save.c / save.h      # new: path, load, flush (atomic)
└── main.c               # flags, exit 4, interval, exit flush, comment and usage
libretro/libretro.c      # memory exports
libretro/nesturbator_libretro.info
tests/
├── ines.h               # + prg_ram/prg_nvram shifts, battery, chr nvram, iNES byte tweaks
├── core/test_mapper_mmc1.c   # hook-level and CPU-level MMC1 cases (new file)
├── core/test_cartridge.c     # MMC1 accept/refuse rows, switch set {0,1,2,3,7}
├── core/test_save.c          # span sizes, stability, generation (new)
├── holymapperel/        # decode.c / hm_decode.h row reader and glyphs; roms.cmake entries
├── roms/hm/             # five new ROMs
├── runner/              # save-dir CTest chains, kill test, synthetic save ROM generator
├── libretro/libretro_host.c  # memory checks
└── retroarch/           # run_retroarch.cmake factored into a launch function; test.cfg.in keys
```

### Pattern 1: MMC1 board on the seam
**What:** One file, `reg.mmc1` plain integers, `init` sets only `map.watch = NESTURBATOR_WATCH_CPU_WRITE` (no bus-conflict bit: MMC1 has none), `rebuild` derives every page from the registers, `cpu_write` is the serial port.
**When to use:** Always for mapper 1.
**Registers (zero is power-on):** `shift`, `count`, `control_x` (stored as `control ^ 0x0C`), `chr0`, `chr1`, `prg`, `last_write` (stored as `stamp + 1`). All fields are integers (C2, D-16).

### Pattern 2: Bank math (NESdev MMC1 behaviour, derived)
- PRG outer half: `outer = (prg_size > 256 KiB) ? (chr0 >> 4) & 1 : 0`. It offsets both windows by 16 banks of 16 KiB, including the "fixed" one.
- PRG mode `(control >> 2) & 3` with `p = prg & 0x0F`:
  - 0, 1: 32 KiB switch, `lo = (outer << 4) | (p & 0x0E)`, `hi = lo + 1`.
  - 2: `lo = outer << 4` (first bank of the half), `hi = (outer << 4) | p`.
  - 3: `lo = (outer << 4) | p`, `hi = (outer << 4) | 0x0F`.
  Convert to 1 KiB pages with `bank16 * 16 + i` and `nesturbator__map_prg`, whose true modulo makes `0x0F` the last bank on any power-of-two PRG of 32 KiB or more.
- CHR: control bit 4 clear is 8 KiB mode using `chr0 & 0x1E` as the 4 KiB bank pair; set is two 4 KiB banks, `chr0` at `$0000` and `chr1` at `$1000`. Add `nesturbator__map_chr_4k(nes, half, bank_4k)` next to the existing `nesturbator__map_chr_8k`. `chr_w` is non-NULL only for CHR-RAM, as the existing helper does.
- Mirroring: `nt[]` = `{0,0,0,0}` (one-screen lower), `{1,1,1,1}` (upper), `{0,1,0,1}` (vertical), `{0,0,1,1}` (horizontal). The header mirroring bit is ignored.
- PRG-RAM page (8 KiB at `$6000`):
  - none if `prg_ram` is NULL, or `prg & 0x10` (MMC1B disable), or SNROM disable (`chr_is_ram && prg_size <= 256 KiB && ram_total <= 8 KiB && (chr0 & 0x10)`).
  - bank = 0 for total at most 8 KiB; `(chr0 >> 3) & 1` for 16 KiB; `(chr0 >> 2) & 3` for 32 KiB (C4).
  - A disabled window leaves `cpu_r[8..15]` and `cpu_w[8..15]` NULL: reads give open bus, writes drop, and the generation does not count them.

### Pattern 3: Write-ignore (D-15 with C2)
Compute adjacency and update `last_write` on every hook call before the address test, then reset bit, then adjacency drop, then shift. `nesturbator_reset` leaves `reg.mmc1` alone and the CPU reset path makes no hook calls, so adjacency state stays consistent with the kept `cpu_cycle`.

### Pattern 4: Loader layout (D-06, D-07, D-08)
1. Extend `struct cartridge_layout` with `ram_work`, `ram_nv`, `battery`.
2. Header rules: NES 2.0 takes `V = image[10] & 0x0F` size, `N = image[10] >> 4` size. iNES 1 for mapper 1 follows the D-07 table; every other mapper keeps zero RAM.
3. `board_profile_ok` gets the extra arguments; cases 0, 2, 3, 7 additionally require `battery == 0 && ram_work == 0 && ram_nv == 0 && chr_nv == 0`.
4. Allocation: `ram_total = max(V + N, trainer ? 8192 : 0)`; allocate once after the image: `[image][PRG-RAM][CHR-RAM]`, memset PRG-RAM to zero, then the trainer at `+0x1000`. Set `cart.prg_ram`, `cart.prg_ram_size`, and `cart.save = prg_ram + V`, `cart.save_size = N` (both 0/NULL when N is 0).
5. Existing trainer-only boards keep the flat 8 KiB mapping through `nesturbator__map_prg_ram`.

### Pattern 5: Write counting and the save API
- Count in `nesturbator__bus_write` right after the page store: `if (nes->cart.save_size != 0u && page >= nes->cart.save && page < nes->cart.save + nes->cart.save_size) nes->save_generation++;`. `save_generation` is a `uint64_t` field of `struct nesturbator`, never reset.
- `nesturbator_get_memory` and `nesturbator_save_generation` in `src/instance.c`. The header comment states every D-11 rule, the count rule of D-12, and that the host copies the save in after load and before the first frame.
- `BEHAVIOUR_REVISION` stays 5 only if no existing frame or audio hash moves; the existing `tests/runner/hashes.txt` rows must be byte-identical after the stamp fix and the loader change [ASSUMED: no existing output changes; the hash test proves it].

### Pattern 6: Decoder row reader (D-02, D-03, C1)
Reference pixel (0,7). New glyph table indexed by character: space, `+`, `A B E G I K L M N O P R S T Y`, plus the existing digits. Read row 6 from x = 16 as 8 px cells until 26 cells or a run of blanks, map to text, trim trailing spaces, then compare with the expected string passed on the command line, or print `holymapperel: prg ram "<text>"`. Unknown cell prints `?`, which cannot match. The CTest asserts `PASS_REGULAR_EXPRESSION "prg ram \"32K PRG RAM OK\""`-style text; take care that `32K PRG RAM OK` is a prefix of `32K PRG RAM OK + BATTERY`, so anchor with the closing quote.

Tiles needed, all present in the committed M3 ROM CHR (tile = ASCII & `$3F`; bit planes OR-ed; verified): `' '`=`$20` (all zero), `'+'`=`$2B` (`18 18 7e 18 18`), `A`=`$01` (`3c 66 7e 66 66`), `B`=`$02`, `E`=`$05`, `G`=`$07` (`3e 60 6e 66 3e`), `I`=`$09`, `K`=`$0B` (`66 6c 78 6c 66`), `L`=`$0C`, `M`=`$0D` (`c6 ee fe d6 c6`), `N`=`$0E`, `O`=`$0F`, `P`=`$10`, `R`=`$12`, `S`=`$13`, `T`=`$14`, `Y`=`$19`. `C`, `D`, `F` already exist. Add each to the glyph CTest, which compares the table with the ROM's CHR.

### Pattern 7: Runner save module (D-18, D-19)
`runner/save.c` exposes: `save_path(buf, cap, dir, rom)` (stem rule, `/` and `\` split on every platform, leading dot kept); `save_load(path, span, size)` returning OK, FRESH, MISMATCH (prints both sizes) or ERROR; `save_flush(path, span, size, shadow)` doing temp + fwrite + fflush + fsync/`_commit` + checked `fclose` + rename/`MoveFileExA`. `main.c` calls load after `nesturbator_load_cartridge` and before the frame loop, flushes after frame f when `f % N == 0`, and flushes at exit on every path that ran frames (including JAM and frame failure). Status 4 must be returned before any frame and must leave the file untouched, since the file is opened read-only. Every `.sav` and temp file is created only under the given directory.

### Pattern 8: Tests that need care
- **Hook-level MMC1 tests** use `nesturbator_load_cartridge` on a synthetic MMC1 image (32 KiB PRG) and call `nes->map.ops.cpu_write` directly with chosen stamps, as `tests/core/test_mapper.c` does. Feed values LSB first through five calls at distinct non-adjacent stamps.
- **CPU-level tests** use `INC $80xx` on bytes placed at `$8010` etc. (not the code byte). Reset vector comes from the fixed last bank; with MMC1's 32 KiB minimum use `prg_16k = 2`.
- **SAVE-04 kill test:** a synthetic MMC1 battery ROM writes a known pattern to `$6000` and loops; a `cmake -P` script runs `nesturbator-run --rom R --frames 4294967295 --save-dir D --save-interval 1` through `execute_process(... TIMEOUT 4 RESULT_VARIABLE r)`, expects `r` to contain "timeout", then reads the `.sav` with `file(READ ... HEX)` and compares it. CMake terminates the child on timeout [VERIFIED: ran `execute_process(COMMAND sleep 30 TIMEOUT 1)`; result `Process terminated due to timeout`, child gone]. It proves a flush that is not the exit flush. A leftover `<sav>.tmp` must not be treated as the save.
- **SAVE-03:** write a `.sav` of the wrong length into a fresh directory, run, expect exit 4 and the two sizes in stderr, then compare the file byte for byte (hash before and after).
- **SXROM chain:** unique save directory per test to stay safe under `ctest -j`; `FIXTURES_SETUP`/`FIXTURES_REQUIRED` order run 1 then run 2.
- **Hash inventory:** run 1 and run 2 need separate keys in `write_hashes.cmake`/`roms.cmake` (today a key is `<key>:<file>:<N>` and one run). Phase 7 WR-02 (`hash_inventory.cmake` compares only the last platform's keys) is open, so decide whether the first plan folds it in before relying on the cross-platform equality of new rows.
- **libretro host:** see C3.

### Pattern 9: Two-session RetroArch proof (D-20)
Factor the RetroArch launch (steps 5-9 of the script) into a CMake `function(run_session name saves_state)`. Session 1: `saves/` empty, runner reference without save. Session 2: copy `saves/<stem>.srm` to `<scratch>/<stem>.sav`, runner reference with `--save-dir <scratch>`. Assert both screenshots equal their references, the two differ, and exactly one `.srm` of 32,768 bytes with `SAVEDATA` at 0x100 exists. `ci.yml` changes `-DROM` and `-DFRAME` for the e2e job (`FRAME` must be at least the pinned N for run 2). Keep the user-directory snapshot check around each session.

### Anti-Patterns to Avoid
- **Keying adjacency off the CPU opcode.** Rejected by D-15 and PITFALLS.
- **Resetting `last_write` or the shift register in `nesturbator_reset`.** The cartridge sees no reset line.
- **Reading `nes->ticks` or allocating inside the hook.** Seam rule.
- **A host-specific branch in the bus** for disabled RAM. NULL pages already give open bus and dropped writes.
- **Padding or truncating a `.sav`.** Out of scope; refuse with status 4.
- **Writing the `.sav` beside the ROM.** Out of scope; only `--save-dir`.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Page-table banking | A new access path in the bus | `cpu_r`/`cpu_w` pages rebuilt from registers, NULL for open bus | Existing seam; no bus change except the stamp and the span counter |
| Test cartridges | Hand-written byte arrays | `tests/ines.h`, extended with RAM nibbles and the battery bit | One builder keeps header encoding in one place |
| Hook probing | A second recording board | `tests/mapper_test.h` | Already records `(addr, value, cpu_cycle)` |
| Killing a child | `fork`/`TerminateProcess` driver | CMake `execute_process(TIMEOUT)` | Works on all six legs, verified locally |
| Result screen reading | OCR or a hash only | The decoder with a reference pixel at (0,7) | Diagnosable; a hash cannot say why |
| Save format | A header, checksum or padding | Raw span bytes, same as `.srm` | Interchangeable with RetroArch (D-18, requirement text) |
| Atomic replace | `remove` then `rename` | `rename` (POSIX), `MoveFileExA(... REPLACE_EXISTING|WRITE_THROUGH)` (Windows) | A crash between remove and rename loses the save [CITED: CONTEXT D-19] |
| Header database | Bundled DB for SOROM/SUROM | NES 2.0 sizes only | Deferred by CONTEXT |

**Key insight:** the phase adds almost no new mechanism. Every new behaviour is a page-table fill, a counter, or a file helper; the risk is in header rules and in the two places where timing and identity matter (the write stamp and the save offset).

## Runtime State Inventory

Not a rename or migration phase. The one runtime-state concern is that `.sav` files written by this release fix the span layout (D-06, costly to reverse): `[V work][N NVRAM]`, span at `prg_ram + V`. Stored data: none exists yet. Live service config, OS-registered state, secrets and build artifacts: none (verified: no installed service, no env var, no packaged artifact references mapper 1).

## Common Pitfalls

### Pitfall 1: Battery span moves between releases
**What goes wrong:** Saves written by one release load into the wrong bytes of the next.
**Why it happens:** The span offset depends on V, and V depends on header interpretation.
**How to avoid:** Freeze D-06/D-07 in the header comment and README, with fixture tests for iNES 1 (8 KiB, 32 KiB) and NES 2.0 SOROM (span at +8 KiB). Test the SOROM bank-1 byte lands at span offset 0.
**Warning signs:** A SOROM `.sav` whose first 8 KiB are work-RAM garbage.

### Pitfall 2: Glyph matching breaks on `M` (C1)
**What goes wrong:** `PRG RAM` reads `PRG RA?` and every row-6 assertion fails or, worse, passes on a prefix.
**How to avoid:** Reference pixel (0,7); unit test paints `M`; anchor expected strings with a terminator; assert `OK` and `+ BATTERY` explicitly.

### Pitfall 3: Zeroed `last_write` equals a real stamp (C2)
**What goes wrong:** A write at stamp 1 right after load looks adjacent to nothing and is dropped.
**How to avoid:** Store `stamp + 1`.

### Pitfall 4: SOROM bit selection by modulo (C4)
**What goes wrong:** SOROM games that write bit 3 get the same RAM bank for both settings of that bit; the battery half never sees data.
**How to avoid:** Select by total size; the D-05 SOROM test writes bit 3 and checks span versus work half.

### Pitfall 5: Hook sees `cpu_cycle + 1` (WR-01)
**What goes wrong:** The documented "index of the write's cycle" is off by one; a test written to the comment fails or, worse, a future board that uses the absolute value (the Phase 9 IRQ counters) is off by one.
**How to avoid:** Fix first, then write the MMC1 tests against the corrected stamp. Hashes cannot move because Uxrom, Cnrom and Axrom discard the stamp [CITED: CONTEXT D-14; `(void)cpu_cycle;` in src/mapper_uxrom.c:40, src/mapper_cnrom.c:35, src/mapper_axrom.c:41].

### Pitfall 6: Generic header rejects survive (C7)
**What goes wrong:** The battery ROMs still fail to load because the early check in `validate_image` rejects `image[6] & 0x02`.
**How to avoid:** Change that check and the NES 2.0 branch together; keep the four-screen bit rejected for all boards. Keep every existing BAD row for mappers 0, 2, 3, 7.

### Pitfall 7: Fixed bank moves with the outer bit
**What goes wrong:** SUROM/SXROM boot crashes after the first CHR0 write that changes bit 4, because the code at `$C000` is no longer mapped.
**How to avoid:** Apply `outer` to both windows, including the fixed one, in every PRG mode. Holy Mapperel's PRG window test checks exactly this.

### Pitfall 8: `fsync`/`fileno` invisible under `-std=c17` (C6)
**How to avoid:** `_POSIX_C_SOURCE 200809L` first line of `runner/save.c`; Windows uses `_commit(_fileno(f))`.

### Pitfall 9: Unflushed save on the expected path
**What goes wrong:** The exit flush is skipped on a JAM or a frame failure.
**How to avoid:** One exit label that flushes whenever a span exists and the frame loop started; the status stays the first failure.

### Pitfall 10: Run 2 of the SXROM chain passes without the save
**What goes wrong:** A stale directory makes run 1 look like run 2.
**How to avoid:** D-04 steps 1 and 3 (clean step, run 1 must show no `+ BATTERY`).

### Pitfall 11: RetroArch session 2 reads session 1's screenshot
**How to avoid:** Remove `shot.png`, `shot_raw.png`, `shot.bmp` between sessions or use per-session names; the "screenshots must differ" assertion would otherwise pass trivially.

### Pitfall 12: Pointer equality across instances (C3)
See C3.

## Code Examples

Patterns for the executor. Values below that mirror in-repo facts are cited; the rest is design.

### MMC1 serial port (design; satisfies D-15, D-16, C2)
```c
/* Source: NESdev Wiki "MMC1": consecutive-cycle writes after the first are ignored, the bit-7 reset
   is never ignored and sets Control |= $0C, the fifth write picks the register by address bits
   14-13. control_x holds Control ^ $0C so a zeroed block is the power-on Control of $0C. */
static void mmc1_cpu_write(struct nesturbator *nes, uint16_t addr, uint8_t value, uint64_t cpu_cycle)
{
    struct nesturbator__mmc1 *m = &nes->mapper.reg.mmc1;
    const uint64_t stamp1 = cpu_cycle + 1u;                      /* 0 means never written */
    const int adjacent = m->last_write != 0u && stamp1 == m->last_write + 1u;
    m->last_write = stamp1;                                      /* even when ignored */
    if (addr < 0x8000u)
        return;
    if ((value & 0x80u) != 0u) {
        m->shift = 0u;
        m->count = 0u;
        m->control_x = (uint8_t)(m->control_x & ~0x0Cu);         /* Control |= $0C */
        nes->map.ops.rebuild(nes);
        return;
    }
    if (adjacent)
        return;
    m->shift = (uint8_t)((m->shift >> 1) | ((value & 1u) << 4));
    if (++m->count < 5u)
        return;
    switch ((addr >> 13) & 3u) {
    case 0u: m->control_x = (uint8_t)(m->shift ^ 0x0Cu); break;
    case 1u: m->chr0 = m->shift; break;
    case 2u: m->chr1 = m->shift; break;
    default: m->prg = m->shift; break;
    }
    m->shift = 0u;
    m->count = 0u;
    nes->map.ops.rebuild(nes);
}
```

### Bus change (design on [VERIFIED: src/bus.c:163-177])
```c
    } else if (addr >= 0x4020u) {
        uint8_t *page = nes->map.cpu_w[(addr - 0x4000u) >> 10];
        if (page != NULL) {
            page[addr & 0x3ffu] = value;
            /* All cpu_w pages lie inside cart.bytes, so the range compare is within one object. */
            if (nes->cart.save_size != 0u && page >= nes->cart.save &&
                page < nes->cart.save + nes->cart.save_size)
                nes->save_generation++;
        }
        if ((nes->map.watch & NESTURBATOR_WATCH_CPU_WRITE) != 0u) {
            /* ... existing AND for bus conflicts ... */
            nes->map.ops.cpu_write(nes, addr, v, nes->cpu_cycle - 1u);   /* D-14 */
        }
    }
```
The existing line is `nes->map.ops.cpu_write(nes, addr, v, nes->cpu_cycle);` [src/bus.c:175].

### Public API (D-11 verbatim signature; body is design)
```c
nesturbator_status nesturbator_get_memory(nesturbator *inst, nesturbator_memory kind, uint8_t **data, size_t *size)
{
    if (inst == NULL || data == NULL || size == NULL || kind != NESTURBATOR_MEMORY_SAVE_RAM)
        return NESTURBATOR_ERR_ARGUMENT;
    *data = inst->cart.save_size != 0u ? inst->cart.save : NULL;
    *size = inst->cart.save_size;
    return NESTURBATOR_OK;
}
```

### WR-03 guard (design on [VERIFIED: src/internal.h:260-264])
```c
static inline uint8_t nesturbator__map_cpu_read(struct nesturbator *nes, uint16_t addr)
{
    const uint8_t *page;
    if (addr < 0x4020u)
        return nes->bus.open_bus;
    page = nes->map.cpu_r[(addr - 0x4000u) >> 10];
    return page != NULL ? page[addr & 0x3ffu] : nes->bus.open_bus;
}
```

### SAVE-04 kill test (CMake script; design, mechanism verified locally)
```cmake
execute_process(COMMAND "${RUNNER}" --rom "${ROM}" --frames 4294967295
                        --save-dir "${DIR}" --save-interval 1
                TIMEOUT 4 RESULT_VARIABLE result OUTPUT_QUIET ERROR_QUIET)
if(NOT result MATCHES "timeout")
  message(FATAL_ERROR "runner should still have been running: ${result}")
endif()
file(SIZE "${DIR}/${STEM}.sav" size)
file(READ "${DIR}/${STEM}.sav" bytes HEX)
if(NOT size EQUAL ${EXPECT_SIZE} OR NOT bytes STREQUAL "${EXPECT_HEX}")
  message(FATAL_ERROR ".sav does not hold the bytes written before the kill")
endif()
```

### SXROM chain pieces (D-04)
```cmake
add_test(NAME holymapperel.sxrom.clean
  COMMAND ${CMAKE_COMMAND} -E rm -rf ${SXROM_SAVEDIR})
set_tests_properties(holymapperel.sxrom.clean PROPERTIES FIXTURES_SETUP hm_sxrom_clean)
# run 1: nesturbator-run --rom ... --frames N --save-dir ${SXROM_SAVEDIR} --dump-frame N:run1.ppm
# check: size 32768 and file(READ ... HEX OFFSET 256 LIMIT 8) == "534156454441 5441" without the space
# run 2: identical command, dump run2.ppm; decode asserts text with the BATTERY suffix
```
`SAVEDATA` in hex is `5341564544415441` [VERIFIED: probe `.srm` bytes at 0x100 were `5341 5645 4441 5441`].

### Runner atomic write (POSIX branch, design)
```c
#define _POSIX_C_SOURCE 200809L   /* before every include: fileno and fsync under -std=c17 */
/* tmp = path + ".tmp"; fopen(tmp,"wb"); fwrite; fflush; fsync(fileno(f)); fclose checked; rename(tmp,path) */
```
Windows branch: `_commit(_fileno(f))`, then `MoveFileExA(tmp, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)`. [CITED: learn.microsoft.com/windows/win32/api/winbase/nf-winbase-movefileexa: REPLACE_EXISTING "replaces its contents"; WRITE_THROUGH matters for copy-and-delete moves; the page does not promise atomicity.]

## State of the Art

| Old Approach | Current Approach | Impact |
|--------------|------------------|--------|
| Planning doc layout `[ nvram \| work ]` | `[V work][N NVRAM]` (D-06) | `.planning/research/ARCHITECTURE.md` lines 127 and 192 are wrong for SOROM; fix them in the same change. |
| `void **` out parameter | `uint8_t **` (D-11) | Avoids an incompatible-pointer error in strict C17 and C++. |
| Stamp = cycles done (one high) | Stamp = zero-based cycle index | Matches `mapper.h` wording; no hash change. |

**Deprecated/outdated:**
- `libretro_saves = "false"` and the Phase 1 description in the `.info` (C10).
- The "mappers 0, 2, 3 and 7" wording in `include/nesturbator.h:70-71, 240-260`, `runner/main.c:7, 46, 659-660, 663` and the README sections "The runner" and API text. Every site lists the supported set and must say mapper 1 too.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | glibc hides `fsync`/`fileno` under `-std=c17` unless a feature macro is defined | C6, Pitfall 8 | Low. The macro is harmless where unnecessary; a missed need shows as a Linux build error. |
| A2 | `.info` `libretro_saves` is display-only in RetroArch | C10 | Low. The probe saved without any `.info`; setting `"true"` is the documented meaning. |
| A3 | No existing frame or audio hash changes, so `NESTURBATOR_BEHAVIOUR_REVISION` stays 5 | Pattern 5 | Medium. If any `hashes.txt` row moves, the revision must rise; the hash test shows it. |
| A4 | Round N for the Holy Mapperel ROMs is about 200 for 8 KiB ROMs and 400-600 for SXROM | Pattern 8 | Low. D-04 pins N by measurement; 600 frames cost 0.6 s (ci) and about 10 s (asan). |
| A5 | The six D-20 config key names are accepted by RetroArch 1.22.2 | Verified Inputs | Low. The e2e asserts the outcome (empty `saves/`, one `.srm` at the stem), so a wrong key shows as a failure; the probe worked with defaults. |
| A6 | Windows `MoveFileExA` with the D-19 flags replaces an existing file on the CI runners | Code Examples | Low. Documented to replace; Windows legs of `ci` run the save tests. |

## Open Questions (RESOLVED)

1. **Total RAM of 24 KiB (C5).**
   - What we know: D-08 as worded admits V + N = 24 KiB; no board has it.
   - What's unclear: whether the owner wants it rejected.
   - Recommendation: reject any total outside {0, 8, 16, 32} KiB and add a fuzz seed row `mmc1-ram-24k` and a cartridge-test row. This is within the spirit of D-08 and does not touch a locked value.
   - RESOLVED: rejected. 08-01 records the decision and implements the sum rule in `board_profile_ok`; 08-02 Task 2 adds the `mmc1-ram-24k` cartridge row; 08-04 Task 1 commits the `mmc1-ram-24k` seed.

2. **Fold Phase 7 WR-01/WR-02 into plan 1?**
   - What we know: they are open, PR #32 is a draft, and Phase 8 adds hash rows that depend on `hash_inventory.cmake` (WR-02).
   - What's unclear: the owner's preference (CONTEXT leaves it open).
   - Recommendation: fold WR-02 into plan 1 only if the planner finds the new SXROM keys cannot be compared across platforms without it; otherwise leave both for `/gsd-quick`.
   - RESOLVED: not folded. CONTEXT.md lists both under Deferred Ideas, and the byte-equality check against the first artifact already compares the new rows on all six platforms. 08-07 records the decision and only adds keys and counts to `hash_inventory.cmake`.

3. **Trainer plus mapper 1 plus no declared RAM.**
   - What we know: the trainer path allocates 8 KiB regardless; D-06 says it still does.
   - What's unclear: whether MMC1 should map that 8 KiB at `$6000` when the NES 2.0 header declares none.
   - Recommendation: yes (the allocation exists, so the page maps); add a one-line test. It is a corner no real dump has.
   - RESOLVED: yes, the trainer's 8 KiB maps at `$6000` as work RAM and the span is NULL/0. 08-01 records the decision and allocates `max(V + N, trainer ? 8 KiB : 0)`; 08-02 Task 2 adds the `core.save` case.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| CMake, CTest | Build and tests | yes | 4.4.3 | — |
| Ninja, C toolchain | Build | yes (build dirs exist) | — | — |
| `bsdtar` | Extract the 7z asset (D-01) | yes | — | — |
| Network (GitHub, buildbot.libretro.com) | Asset and RetroArch download | yes | — | — |
| RetroArch 1.22.2 (pinned DMG) | `retroarch-e2e` | yes, on this Mac and in the macos-15 job | 1.22.2 | none; the job is required |
| A logged-in GUI session | RetroArch window | yes on this Mac | — | — |
| Linux, Windows legs | `fsync`, `MoveFileExA`, `_POSIX_C_SOURCE` | only in CI | — | CI is the check |

**Missing dependencies with no fallback:** none.

## Validation Architecture

Skipped: `workflow.nyquist_validation` is `false` in `.planning/config.json` [VERIFIED: config read]. The requirement-to-test map lives in the `<phase_requirements>` table above and in Pattern 8. The test command is `cmake --workflow --preset ci` [VERIFIED: config `test_command`]; `asan` and `nofp` presets must also stay green.

## Security Domain

Skipped: `security_enforcement` is `false` in `.planning/config.json`. Two points still shape the plan. The runner writes only `<save-dir>/<stem>.sav` and its `.tmp` sibling and reads only that path, and the stem comes from the last path component, so a ROM path cannot direct a write outside the save directory. The fuzz corpus must gain the D-09 seeds, because the loader gains more header branches.

## Sources

### Primary (HIGH confidence)
- Repo files read this session: `CLAUDE.md`, `.claude/CLAUDE.md`, `.planning/phases/08-mmc1-and-battery-saves/08-CONTEXT.md`, `.planning/REQUIREMENTS.md`, `.planning/preparation/README.md`, `.planning/research/PITFALLS.md` (Pitfalls 2-5), `src/internal.h`, `src/mapper.h`, `src/cartridge.c`, `src/bus.c`, `src/instance.c`, `src/mapper_uxrom.c`, `src/mapper_axrom.c`, `include/nesturbator.h`, `runner/main.c`, `runner/CMakeLists.txt`, `libretro/libretro.c`, `libretro/nesturbator_libretro.info`, `tests/ines.h`, `tests/mapper_test.h`, `tests/core/test_cartridge.c`, `tests/core/test_mapper.c`, `tests/core/test_reset.c`, `tests/CMakeLists.txt`, `tests/holymapperel/*`, `tests/retroarch/*`, `tests/cmake/write_hashes.cmake`, `tests/cmake/vector_api_policy.cmake`, `tests/cmake/undefined_symbols.cmake`, `tests/runner/hashes.txt`, `.github/workflows/ci.yml`, `CMakeLists.txt`.
- Holy Mapperel at `c022622274ca8b83d214dea97e4388a6b0e92d8a` (Zlib): `src/main.s`, `src/wram.s`, `src/mmcdrivers.s`, `README.md`; release asset `holy-mapperel-bin-0.02.7z` downloaded, hashed and extracted.
- https://www.nesdev.org/wiki/MMC1 (fetched this session).
- https://www.nesdev.org/wiki/NES_2.0 and https://www.nesdev.org/wiki/INES (fetched this session).
- https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexa (fetched this session).
- RetroArch 1.22.2 pinned DMG: behaviour measured on the binary with a throwaway probe core (no RetroArch source opened).

### Secondary (MEDIUM confidence)
- Arch Wiki RetroArch page via web search: SRAM written on clean exit; `autosave_interval` in seconds (consistent with the measurement).

### Tertiary (LOW confidence)
- glibc feature-test-macro behaviour (A1), from general knowledge.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH. Nothing new; all in-repo.
- Architecture: HIGH. Seam read in full; the only judgement calls are the span range check (either option allowed) and the zero-encodings (C2, D-16).
- Pitfalls: HIGH for C1-C9 (each traced to a read file or a computation). MEDIUM for the Linux `-std=c17` point (A1).
- RetroArch behaviour: HIGH. Measured on the pinned binary, three sessions plus size probes.

**Research date:** 2026-10-10
**Valid until:** 2026-11-09 for repo and Holy Mapperel facts (stable); the RetroArch results hold while the pin stays 1.22.2.
