# Phase 7: UxROM, CNROM and AxROM - Research

**Researched:** 2026-10-10
**Domain:** NES discrete-logic cartridge boards (mappers 2, 3, 7) on the Phase 6 mapper seam; iNES/NES 2.0 loader profiles; Holy Mapperel as a frame-read oracle
**Confidence:** HIGH (code facts read this session; board behaviour from NESdev; Holy Mapperel screen layout traced from source at the pin, not yet run)

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

The owner chose three areas (Holy Mapperel in the tree, reading the 0000 code,
loader rules) and asked for a fan-out with an adversarial pass, followed by one
coherent set of recommendations without further questions. Board modules were
added as Claude's call so that the set stays consistent. Four researchers
reported. The orchestrator then checked their claims against the code. The
decisions below are the result.

#### Holy Mapperel in the tree
- **D-01:** Commit three ROMs from the v0.02 release asset
  `holy-mapperel-bin-0.02.7z`, raw and byte-identical. The asset is 17,964 B
  with sha256
  `70f85671e21f293599baebb662faeb06a4c04e9c9ceb283d96d4197f09e4ce7a`, from tag
  v0.02, commit `c022622274ca8b83d214dea97e4388a6b0e92d8a`, under the zlib
  licence. The ROMs go under `tests/roms/hm/`.
  | File | Size | sha256 | Header |
  |---|---|---|---|
  | `M2_P128K_CR8K_V.nes` | 131,088 | `c7e83755bd9adbb7c705ea9f29535442af7390444632f9f824fcf4c00632069b` | mapper 2, submapper 0, 128 KiB PRG, 8 KiB CHR-RAM, vertical |
  | `M3_P32K_C32K_H.nes` | 65,552 | `499891c6d8c7a1e7631bdc601d9d624938842735f92fe1fda7b19ef9fae514b7` | mapper 3, submapper 0, 32 KiB PRG, 32 KiB CHR-ROM, horizontal |
  | `M7_P128K_CR8K.nes` | 131,088 | `4aa0050f36ae17e17701506821e5147df5b843bcdf2827468fa9c66e4b7ac1ba` | mapper 7, submapper 0, 128 KiB PRG, 8 KiB CHR-RAM |

  All three are NES 2.0. The executor re-downloads the asset, re-verifies every
  hash and extracts with `bsdtar`. The ROMs are about 97% 0xFF filler, so git
  stores roughly 16 KB of packed history for them.
- **D-02:** The near-duplicate twins `M2_P128K_V.nes` and `M7_P128K.nes` are not
  committed, because they add no coverage. Phases 8 and 9 follow the same
  pattern: one ROM per distinct banking, battery or CHR shape, never twins.
- **D-03:** Each ROM gets a line in `tests/roms/manifest.txt`. The source is the
  full release asset URL, the pin is the commit above, the licence is `Zlib`,
  and the sha256 is the file's. THIRD-PARTY-NOTICES.md gets a Holy Mapperel
  section: the files, the asset sha256 and the verbatim upstream LICENSE text
  from that commit, plus a mention in the intro sentence. Rejected options: a
  packed form with an owned decoder, which breaks the sha256 chain and makes the
  decoder part of the test oracle; a ca65 source build, which adds a toolchain
  dependency; and a nightly-only fetch, which breaks DEC.20. —
  **Reversibility:** one-way. A file committed to a public repository stays in
  its history permanently, and the zlib licence allows that.

#### Reading the 0000 code
- **D-04:** The oracle is a test-only decoder, `tests/holymapperel/decode.c`,
  built as `holymapperel-decode`. It reads the P6 PPM that
  `nesturbator-run --rom R --frames N --dump-frame N:FILE` writes. The shipped
  runner gains no Holy-Mapperel mode or font. Reading RAM or the nametable is
  rejected because it is not "from the frame". A hash match alone is rejected as
  the oracle: it gives no diagnosis, and re-pinning could freeze a failing
  screen.
- **D-05:** Screen facts, traced by hand from the Holy Mapperel source
  (`src/main.s`) at the pin and not yet run:
  - Row 8 of nametable 0 reads `DETAILED TEST RESULT: ` followed by 4 hex
    digits in columns 24-27. In pixels the digits are x 192-223, y 64-71, so
    digit k starts at x = 192 + 8k.
  - The anchor is the `D` and `E` at columns 2-3 (x 16-31, y 64-71).
  - Matching is exact on an 8x8 lit/unlit mask. A pixel is lit when it differs
    from the cell's (0,0) pixel, so the decoder does not depend on the palette.

  On the first boot, the executor confirms these coordinates by reading the
  dumped frame. That is a development step, not a gate. If they are wrong, the
  executor corrects them in the decoder and in this file's successor
  (SUMMARY).
- **D-06:** The glyphs are a hand-written `static const uint8_t glyph[16][8]`
  taken once from tiles `$30`-`$39` and `$01`-`$06` of the committed M3 ROM's
  CHR, with the comment "Holy Mapperel font, zlib, (c) Damian Yerrick". A
  CTest checks the table against those tiles of the committed M3 ROM. The ROM
  is not used to build templates at test time, because the M2 and M7 ROMs have
  CHR-RAM and no font in CHR.
- **D-07:** Decoder exit statuses:
  - `0`: the anchor matched and the code is `0000`.
  - `1`: the code is non-zero. The decoder prints the code and the meaning of
    each digit, in the order WRAM, PRG window bitmask, IRQ, CHR. `C0DE` means
    the driver never finished.
  - `2`: there is no result screen, because the anchor is missing or a cell
    matches no glyph (printed as `?`). A blank frame, a pre-render frame and a
    Morse-code crash screen all land here, so none can read as a pass.
  - Any other size than 256x240 P6 is also an error.

  The pass output is `holymapperel: code 0000 (...)`, and the CTest asserts it
  with `PASS_REGULAR_EXPRESSION`.
- **D-08:** CTest chain for each ROM:
  - A dump test (`FIXTURES_SETUP hm_<name>`) runs the runner, following the
    existing `runner.dump` pattern.
  - A decode test (`FIXTURES_REQUIRED`) asserts `0000`.
  - A frame-hash row for each ROM goes in `tests/runner/hashes.txt` through the
    existing `write_hashes`/`game_movie` flow.

  The hash guards against regressions, and the decoder decides pass or fail.
  Start with N = 600 frames per ROM, then pin the smallest round N for which
  frame N and frame 2N hash the same, because the result screen is static once
  drawn. Phases 8 and 9 add CMake lines only. The battery ROM's second run is
  Phase 8's concern.

#### Loader rules this phase
- **D-09:** Each board has its own profile in the validator in
  `src/cartridge.c`. The set accepted is exactly {0, 2, 3, 7}, and every other
  mapper (including 1 and 4 until their phases) is rejected with the existing
  `NESTURBATOR_ERR_CARTRIDGE`. There is no new status code and no ABI change.
  | Mapper | Submapper | PRG ROM | CHR | Mirroring bit |
  |---|---|---|---|---|
  | 0 | 0 only | unchanged: 16 or 32 KiB | unchanged | used |
  | 2 | 0, 1, 2 | a multiple of 16 KiB, 16 KiB-4 MiB | 8 KiB RAM, or 8 KiB ROM | used |
  | 3 | 0, 1, 2 | 16 or 32 KiB | ROM of 8, 16 or 32 KiB only (no CHR-RAM, nothing over 32 KiB) | used |
  | 7 | 0, 1, 2 | a multiple of 32 KiB, 32-256 KiB | 8 KiB RAM only | ignored |

  Unchanged for every board: a submapper of 3 or more is rejected; PRG-RAM,
  PRG-NVRAM and CHR-NVRAM must be zero (the trainer path stays as it is);
  four-screen, battery and console-type rejections stay; v1 bytes 8-15 must be
  zero, so DiskDude-style headers are still refused; bytes 12-15 of a NES 2.0
  header must be zero; and the exact-total size rule applies. The v1
  byte-7 high-nibble rule is relaxed only so that mapper numbers 2, 3 and 7
  pass. Non-power-of-two NES 2.0 sizes within these limits use the Phase 6
  true modulo. Every size is checked against the board maximum before it is
  multiplied. Rejected for now: oversize CNROM (Jogging Race), 512 KiB AxROM
  (Hot Dance 2000) and Hayauchi Super Igo's PRG-RAM. — **Reversibility:**
  reversible. Phases 8 and 9 add rows.
- **D-10:** `nesturbator__mapper_load` must fail loudly rather than leave an
  empty map. It returns an error when no board matches, and the load returns
  `ERR_CARTRIDGE` with the instance untouched. A CTest walks every accepted
  mapper id and asserts that it gets a board, so the validator and the switch
  cannot drift apart.
- **D-11:** Power-on PC through the mapped pages. Today the loader reads the
  reset vector from the last 4 bytes of the PRG file (`src/cartridge.c:218`),
  while a soft reset reads $FFFC/$FFFD through the bus (`src/cpu.c:1375`).
  That is wrong for AxROM, whose power-on bank 0 is not the file's last bank.
  After `nesturbator__mapper_load`, read $FFFC/$FFFD from `map.cpu_r` with no
  bus cycle. For NROM the result is byte-identical. A CTest builds an AxROM
  image whose banks hold different vectors and checks that the power-on PC
  comes from bank 0.
- **D-12:** Error surfaces only change wording. The runner's error message,
  the `--rom` help and the header comment at the top of `runner/main.c` stop
  saying "mapper-0" and name mappers 0, 2, 3 and 7. The public header comments
  (`include/nesturbator.h` lines 70, 163, 184, 239-242) and the README support
  text change in the same commit (rule 6). MAP-03's final message is still
  Phase 9's.
- **D-13:** Tests and fuzz seeds, built with `tests/ines.h`:
  - Accept cases for each board: mapper 2 at 8 and 16 banks with submappers
    0-2; mapper 3 with 1, 2 and 4 CHR banks; mapper 7 at 1, 2, 4 and 8 banks.
  - One reject case per rule row: mappers 1, 4, 5, 6 and 8; submapper 3 on
    each board; CNROM with CHR-RAM; CNROM with 64 KiB of CHR; AxROM with
    CHR-ROM; AxROM with odd PRG; AxROM at 512 KiB; mapper 2 above 4 MiB;
    PRG-RAM; four-screen and battery on each board; trailing and truncated
    images; DiskDude; NES 2.0 mapper 256 or above.
  - New fuzz seeds in `tests/fuzz/corpus/`: one valid image each for mappers
    2, 3 and 7, plus the key rejected shapes. Each seed gets a manifest line,
    the same as the existing seeds. Keep `mapper-bits`.

#### Board modules (Claude's call, folded in for coherence)
- **D-14:** One file per board: `src/mapper_uxrom.c`, `src/mapper_cnrom.c` and
  `src/mapper_axrom.c`. Each exports `nesturbator__mapper_<board>_ops` and has
  a one-field register struct in the `reg` union. Each adds a case to
  `nesturbator__mapper_load` and an entry in CMake. NROM's header-bit
  mirroring moves into one small shared helper that NROM, UxROM and CNROM all
  call. A single `mapper_discrete.c` that switches on the id is rejected,
  because that is the switch-in-a-hook shape the seam exists to avoid.
- **D-15:** Bus conflicts. `init` sets `CPU_WRITE`, and adds `BUS_CONFLICT`
  when `submapper == 2 || (submapper == 0 && id != 7)`. That means:
  - Mappers 2 and 3: submapper 0 ANDs (the project default fixed by BOARD-01;
    NESdev says "warn and/or enforce").
  - Mapper 7: submapper 0 does not AND (NESdev; Double Dare and Wheel of
    Fortune glitch under emulated AxROM conflicts).
  - Submapper 1 never ANDs, and submapper 2 always does.

  The AND already happens in `src/bus.c` against the banking in force before
  the write. — The source for this is NESdev and BOARD-01 only. A researcher
  mentioned another emulator's default, and that is not a source for this
  project (clean-room rule 4).
- **D-16:** Per-board behaviour. Every write the board handles is to
  $8000-$FFFF, and it uses the value after any AND. `cpu_w` for $8000-$FFFF
  stays NULL.
  - **UxROM:** an 8-bit register, not masked, wrapped by the existing modulo.
    $8000 holds 16 KiB bank `reg`, and $C000 is fixed to the last bank. CHR
    is the existing CHR-RAM path. Mirroring comes from the header.
  - **CNROM:** the raw byte selects the 8 KiB CHR bank by modulo. Bits 4-5
    (the mapper 185 diodes) are ignored. PRG is laid out as in NROM.
    CHR-ROM writes are dropped because `chr_w` is NULL; a test proves this,
    and no board code is needed.
  - **AxROM:** `value & 7` selects the 32 KiB PRG bank. Bit 4 sets all four
    `nt[]` entries to the same CIRAM bank, and the header mirroring bit is
    ignored. CHR is 8 KiB RAM.
  - **Power-on:** every register is 0, so AxROM starts in bank 0 with
    nametable 0. NESdev gives no power-on state, and the comments say this is
    an implementation choice.
- **D-17:** Synthetic behaviour cases use a distinct marker byte per bank. They
  cover:
  - The AND on mappers 2 and 3 at submapper 0, and no AND on mapper 7.
  - Submapper 1 with no AND on mappers 2 and 3, and submapper 2 with the AND
    on all three.
  - UxROM's fixed last bank, and the UxROM and CNROM modulo wrap.
  - A CNROM `$2007` write to CHR-ROM dropped, and a UxROM CHR-RAM write that
    sticks.
  - AxROM bit 4 selecting single screen 0 or 1, checked through `nt[]` and a
    `$2000`/`$2400` readback, and the AxROM PRG bank.
  - The power-on register of each board, and D-11's power-on PC.
- **D-18:** No behaviour-revision bump is expected. NROM's paths and every
  pinned frame and audio hash stay byte-identical, and the existing hash tests
  prove it. If any pinned hash changes, stop and treat it as a defect, not as
  a re-pin.
- **D-19:** Phase 6's open review warnings WR-01 (the stamp offset) and WR-03
  (the read-range guard) do not gate this phase. These boards ignore
  `cpu_cycle`, and every write that reaches a board is at $4020 or above.
  Both warnings stay open for Phases 8 and 9.

### Claude's Discretion
- File names inside `tests/` and the CTest names. Follow the existing
  `runner.*` and `core.*` naming.
- Whether the PPM reader lives in the decoder or in a small shared test
  header.
- The exact wording of messages in D-12.

### Deferred Ideas (OUT OF SCOPE)
- Oversize CNROM (up to 128 KiB CHR), 512 KiB AxROM and 2 KiB PRG-RAM CNROM are not supported yet. Add them when a target game needs them.
- Tolerance for DiskDude and other bad headers belongs to a separate product decision. Guessing a mapper from a corrupt byte 7 can load the wrong board.
- Mapper 180 (Crazy Climber, UxROM with the fixed bank at $8000) and mapper 185 (the CNROM copy-protection diodes) belong to the next-tier mappers.
- Phase 6's WR-01 and WR-03 are to be settled before Phases 8 and 9.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| BOARD-01 | UxROM, CNROM and AxROM games run (Holy Mapperel mapper 2/3/7 report `0000`; bus-conflict defaults and submapper overrides shown by synthetic CTest cases; CNROM ignores CHR-ROM writes) | Board sketches (Code Examples), loader profile changes (Pitfalls 1-4), Holy Mapperel oracle (Pattern 3), hash-inventory changes (Pitfall 6), requirement-to-test map (below) |
</phase_requirements>

## Summary

The Phase 6 seam is small and does almost everything these three boards need. A board fills five function pointers, sets `map.watch` in `init`, and rebuilds the 1 KiB page tables (`cpu_r[48]`, `chr_r[8]`, `chr_w[8]`) and `nt[4]` in `rebuild`. `src/bus.c` already stores the write, then (if `CPU_WRITE` is watched) calls the hook with the value ANDed against the mapped ROM byte when `BUS_CONFLICT` is also watched. The AND uses the banking in force before the write because the hook runs before the board rebuilds. Each board is therefore about 40 lines: a one-byte register, a `cpu_write` that ignores addresses below $8000 and then rebuilds, and a rebuild that points pages through `nesturbator__map_prg` / `nesturbator__map_chr` (both do a true modulo, so raw register bytes are safe).

The real work is in the loader and the test infrastructure, and CONTEXT.md has four gaps that the planner must close (details under Common Pitfalls): (1) `src/cartridge.c:201` hard-codes `cart.chr_size = 8192u`, so CNROM with 16 or 32 KiB of CHR-ROM would silently wrap to the first 8 KiB; (2) D-10's "instance untouched on no match" cannot be met if the board lookup happens in `nesturbator__mapper_load` at the end of the load, because by then the old cartridge has been freed and every subsystem reset, so the lookup has to be probed before the commit; (3) the power-on PC (D-11) is currently computed before the mapper is loaded and must move after it; (4) the cross-platform hash inventory is hard-coded to 36 rows in `write_hashes.cmake`, `hash_inventory.cmake` (twice, plus a fixture), so adding Holy Mapperel rows needs those scripts changed, not just `hashes.txt`.

Holy Mapperel's result-screen layout in D-05 checks out against `src/main.s` at the pin: the "DETAILED TEST RESULT: " line is printed at `txt_y` = 8, which `start_line` turns into PPUADDR `$2102` (nametable 0, row 8, column 2), so the four digits sit at columns 24-27, pixels x 192-223, y 64-71. The 16 hex glyphs in the committed M3 ROM are 5 rows tall in 8x8 cells, use pixel value 1 only, and every glyph's (0,0) pixel is unlit, so "lit = differs from the cell's (0,0) pixel" is sound. I extracted the 16 glyph byte rows below so the decoder table needs no guessing. The asset hash and all three ROM hashes in D-01 were re-verified this session.

**Primary recommendation:** Two plans. 07-01 (loader + boards + synthetic tests + fuzz seeds + doc wording): profile table in `validate_image`, `chr_size` fix, board probe before commit, PC after mapper load, three board files plus one shared helper, `core.*` tests. 07-02 (Holy Mapperel): ROMs, manifest, notices, `holymapperel-decode`, the dump/decode chain, and the hash-inventory script changes driven by one shared ROM list so Phases 8 and 9 add one line.

## Architectural Responsibility Map

This is a C library with a host-owned shell; tiers are the library's own layers rather than web tiers.

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Header parse + per-board size/CHR rules | Loader (`src/cartridge.c` `validate_image`) | — | Validator-first loading: nothing is allocated until the image passes |
| Board selection | Loader (`nesturbator__mapper_load`) | Validator (probe) | One place chooses a board; validator must use the same lookup |
| Bank registers, page tables, `nt[]` | Board module (`src/mapper_*.c`) | Seam (`src/mapper.h`) | Boards own `nes->mapper.reg`; bus/PPU never name a board |
| Bus-conflict AND | Bus (`src/bus.c`) | Board `init` (sets the watch bit) | Already implemented generically; board only requests it |
| CHR-ROM write drop | Seam data (`chr_w[i] == NULL`) | — | No board code; PPU write to a NULL page is dropped |
| Power-on PC | Loader (after board load) | CPU (`src/cpu.c` soft reset reads through the bus) | PC must follow mapped pages, not file layout |
| Result-screen reading | Test-only decoder (`tests/holymapperel/decode.c`) | Runner `--dump-frame` | Shipped runner gains nothing |
| Cross-platform hash pinning | CMake scripts (`write_hashes.cmake`, `hash_inventory.cmake`) | CI `hash-equality` job | Six hosted runners must produce byte-identical rows |

## Standard Stack

No external packages are installed by this phase: C17 in the existing core, CMake/CTest in the existing tree, `bsdtar` (present at `/usr/bin/bsdtar`, version 3.5.3) only to unpack the Holy Mapperel asset during the executor's one-time fetch.

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| (in-repo) `src/mapper.h` seam | Phase 6 | Board ops, watch bits, page tables | Locked by Phase 6 D-01..D-08 |
| (in-repo) `tests/ines.h`, `tests/check.h` | current | Synthetic images, assertions | Project convention |
| Holy Mapperel v0.02 (test data) | commit `c022622274ca8b83d214dea97e4388a6b0e92d8a` | Public board test, zlib | Locked by D-01 |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Per-ROM hard-coded rows in the hash scripts | One shared ROM list included by the scripts and `tests/CMakeLists.txt` | Recommended: Phases 8 and 9 then add one line, as D-08 intends |
| Probe-before-commit in the validator | Call `mapper_load` and roll back on failure | Rollback must restore a freed cartridge; rejected |

**Installation:** none. **Package Legitimacy Audit:** not applicable, no external packages. `gsd-tools package-legitimacy` was not run because nothing is installed.

**Version verification (tools actually present on this machine):** cmake 4.4.3, ninja 1.13.2, Apple clang 21.0.0, Python 3.14.4, bsdtar 3.5.3. `clang-format` is not installed locally, so the format check cannot be run on this Mac; CI runs it. Write code in the existing `.clang-format` style by hand.

## Architecture Patterns

### System Architecture Diagram

```
 ROM bytes --> nesturbator_load_cartridge
                 |
                 v
           validate_image  --(profile row for mapper id: PRG/CHR/mirroring/submapper)--> reject => ERR_CARTRIDGE, instance untouched
                 |            \__ board probe: nesturbator__mapper_ops_for(id) must succeed  (no drift, no late failure)
                 v
           commit: copy image, set cart.prg/prg_size/chr/chr_size/chr_is_ram, id + submapper, zero mapper regs
                 |
                 v
           nesturbator__mapper_load: ops_for(id) -> init (watch bits) -> rebuild (pages, nt[])
                 |
                 v
           power-on PC = cpu_r[$FFFC],[$FFFD] (read through pages, no bus cycle)

 CPU write $4020-$FFFF --> bus.c: store to cpu_w page (NULL on ROM)
        |  watch & CPU_WRITE ?
        |     v  yes
        |  v = value [& map_cpu_read(addr) if watch & BUS_CONFLICT]     <- ROM byte under the OLD banking
        |     v
        |  ops.cpu_write(addr, v, cycle): addr < $8000 ignored; else reg = v; rebuild
        v
 PPU fetch / CPU fetch use cpu_r[], chr_r[], nt[]  (bank number % page count, so any reg byte is safe)
```

### Recommended Project Structure
```
src/
├── mapper.h              # + three *_ops declarations, three reg structs, ops_for probe, int mapper_load
├── mapper_common.c       # NEW shared helpers: header mirroring, $6000 trainer RAM, identity CHR (D-14)
├── mapper_nrom.c         # calls the helpers (byte-identical behaviour)
├── mapper_uxrom.c        # NEW
├── mapper_cnrom.c        # NEW
├── mapper_axrom.c        # NEW
└── cartridge.c           # profiles, chr_size, probe, PC
tests/
├── core/test_mapper_discrete.c   # NEW synthetic behaviour (boards)
├── core/test_cartridge.c         # + accept/reject rows
├── holymapperel/decode.c         # NEW test-only decoder
├── cmake/holymapperel_roms.cmake # NEW shared list (name;file;N) for scripts and tests
└── roms/hm/                      # three ROMs
```

### Pattern 1: A discrete board on the seam
**What:** `init` sets watch bits only; `cpu_write` updates one register and rebuilds; `rebuild` maps all pages from the register. The board never touches the bus or time.
**When to use:** all three boards.
**Source:** structure of `src/mapper_nrom.c` and the hook rules in `src/mapper.h` [VERIFIED: src/mapper.h:8-12, src/mapper_nrom.c:1-50, read this session]. Hardware facts [CITED: nesdev.org/wiki/UxROM, /AxROM, /INES_Mapper_003, /Bus_conflict].

### Pattern 2: Bus-conflict selection in `init`
Verbatim from the seam: `#define NESTURBATOR_WATCH_CPU_WRITE 1u`, `#define NESTURBATOR_WATCH_BUS_CONFLICT 2u` [VERIFIED: src/mapper.h:23-24]. `bus.c` does `v = (uint8_t)(value & nesturbator__map_cpu_read(nes, addr));` when the second bit is watched [VERIFIED: src/bus.c:171-175]. So `init` only computes the mask: `watch = CPU_WRITE | ((sub == 2 || (sub == 0 && id != 7)) ? BUS_CONFLICT : 0)`.

### Pattern 3: Holy Mapperel as an oracle without a display
`nesturbator-run --rom R --frames N --dump-frame N:FILE` writes a 256x240 P6 (184,335 bytes, header `P6\n256 240\n255\n`) [VERIFIED: tests/cmake/check_ppm.cmake:9-11, tests/CMakeLists.txt:74-84]. `check_ppm.cmake` asserts the first pixel is white, which is true of the test card and false of Holy Mapperel, so it cannot be reused for these dumps. Use `nesturbator_runner_test(... EXIT 0 IGNORE_STDOUT)` for the dump (exit status only) and let the decoder validate size and header (D-07 already requires that). Make the CTest chain: `hm.<name>.dump` (FIXTURES_SETUP) then `hm.<name>.decode` (FIXTURES_REQUIRED, `PASS_REGULAR_EXPRESSION "holymapperel: code 0000"`). Note: a `PASS_REGULAR_EXPRESSION` overrides the exit status, so also assert in the decoder's own output that nothing else prints `code 0000` on failure (it will not, because exit 1 and 2 print other text); optionally add `FAIL_REGULAR_EXPRESSION "holymapperel: (no result|code [0-9A-F]{4} \\(mismatch)"` wording of your choice.

### Anti-Patterns to Avoid
- **Switching on `mapper.id` inside a hook:** the seam exists to avoid it (D-14). Submapper handling lives only in `init`.
- **Masking the UxROM register:** NESdev says treat it as a full 8-bit register; let the modulo wrap (D-16).
- **Calling the hook for the whole $4020-$FFFF range without a guard:** `bus.c` calls `cpu_write` for every write at or above $4020 [VERIFIED: src/bus.c:163-176]. Each board must `return` when `addr < 0x8000u`.
- **Zeroing registers in `init`:** `nesturbator__mapper_load` is also the state-load path (comment at src/cartridge.c:142-144), so a zeroing `init` would wipe restored registers. Registers are zero at load because `nesturbator_load_cartridge` does `memset(&inst->mapper, 0, ...)` before setting id and submapper (src/cartridge.c:226). Keep `init` to watch bits. This fits D-16's "every register is 0" and needs a comment saying so. (The `mapper.h` text "Power-on registers; sets map.watch" is loose and could be tightened in the same change.)

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Bank wrap for oversized registers | A mask per board | `nesturbator__map_prg` / `nesturbator__map_chr` true modulo | Already handles non-power-of-two NES 2.0 sizes [VERIFIED: src/internal.h:209-218] |
| Bus-conflict AND | AND inside each board | `BUS_CONFLICT` watch bit | Done in `bus.c` against the pre-write banking |
| CHR-ROM write drop | A guard in CNROM | `chr_w[i] = NULL` | PPU write to NULL page is dropped (src/mapper.h:55-57) |
| PPM writing/reading helpers beyond a tiny reader | A library | ~30-line P6 reader inside the decoder | Owner rule: copy-paste over dependency; the format is fixed |
| Hex-glyph recognition | OCR, ROM-derived templates at test time | The 16-row table below, checked once against the M3 ROM | M2/M7 have CHR-RAM so no font in CHR |
| 7z extraction in the repo | A 7z reader | `bsdtar` once, by the executor | Decided in D-03 |

## Runtime State Inventory

Not a rename/refactor/migration phase. The one adjacent concern: nothing stores mapper state outside the instance, and `nesturbator_reset` keeps `inst->mapper` (so bank registers survive a soft reset, as on real discrete boards) while `inst->map` pointers persist [VERIFIED: src/instance.c:158-176 reset touches ppu, apu, cpu, not mapper/map]. None found in other categories: no databases, services, OS registrations or env vars are involved.

## Common Pitfalls

### Pitfall 1: `cart.chr_size` is hard-coded to 8192
**What goes wrong:** CNROM images with 16 or 32 KiB CHR-ROM load, but `nesturbator__map_chr` takes `bank % (cart.chr_size / 1024)`, so every CHR bank maps to the first 8 KiB. Holy Mapperel's M3 (32 KiB CHR) would then fail its CHR bank-tag test.
**Evidence:** `inst->cart.chr_size = 8192u;` [VERIFIED: src/cartridge.c:201]; modulo in [VERIFIED: src/internal.h:215-218]; the struct comment says "CHR size the loader validated (8192)" [VERIFIED: src/internal.h:165]. The validator also hard-requires 8192 at src/cartridge.c:89 (`else if (layout->chr_size != 8192u || chr_ram != 0u)`), :106-107 (v1 `image[5] != 1u`) and :111.
**How to avoid:** Set `cart.chr_size = layout.chr_is_ram ? 8192u : layout.chr_size`. Replace the three 8192 checks with the board profile. Update the struct comment. A CTest loads a 32 KiB CHR CNROM with a different marker per 8 KiB bank and checks `chr_r[0]` after writing 0..3 (and 4 wraps to 0).

### Pitfall 2: D-10 "instance untouched" versus a late board lookup
**What goes wrong:** `nesturbator_load_cartridge` frees the old cartridge (src/cartridge.c:186-188) and resets bus, APU, PPU, CPU and mapper (:189-231) before `nesturbator__mapper_load` runs (:233). If that function fails at the end, the instance is no longer the old one.
**How to avoid (stays inside D-10):** Add `int nesturbator__mapper_ops_for(uint16_t id, struct nesturbator__mapper_ops *out)` (returns 0 for no board) as the single switch. `nesturbator__mapper_load` calls it and returns int; the loader calls it once in the validation phase with a scratch `ops` before allocating. Then the validator profile (what is legal) and the switch (what exists) can still disagree, but only toward rejection, and the D-10 CTest that walks accepted ids stays useful. It also resolves Phase 6 review finding IN-01 ("Unknown mapper id loads successfully as an empty cartridge") [VERIFIED: 06-REVIEW-DISPOSITION.md].
**Warning signs:** a test that loads a good image, then a bad one, and finds the first image's reset vector gone.

### Pitfall 3: Power-on PC is set before the mapper exists
**What goes wrong:** `inst->cpu.pc` is computed from `cart.prg[prg_size - 4]` at src/cartridge.c:218-219 (verbatim: `inst->cpu.pc = (uint16_t)(inst->cart.prg[layout.prg_size - 4u] | ...`), earlier than `nesturbator__mapper_load` at :233. After the change the PC must be read from `map.cpu_r[(0xfffc - 0x4000) >> 10]` after the load. Use `nesturbator__map_cpu_read(inst, 0xfffcu)` (no bus cycle, no time) [VERIFIED: src/internal.h:221-225]. Note also `memset(&inst->cpu, 0, ...)` at :212 precedes the PC set, so the new assignment goes after `mapper_load` and after the `cpu.s`/`cpu.p` lines remain intact.
**Existing test:** `test_32k_reset_vector_comes_from_upper_prg_bank` expects `0x8123` for 32 KiB NROM [VERIFIED: tests/core/test_cartridge.c:171-194]; it must stay green and is the NROM byte-identity check for D-11.
**For AxROM:** bank 0 at power-on, so the vector comes from the first 32 KiB, not the file's end. Holy Mapperel puts a trampoline every 16 KiB for exactly this reason (README, "Because several mappers don't guarantee what bank will be visible at power on").

### Pitfall 4: The validator edits are bigger than "relax the byte-7 rule"
Verified line by line [VERIFIED: src/cartridge.c:60-118]:
- :67 `(image[6] & 0x0au) != 0u || (image[7] & 0xf0u) != 0u` — mappers 2, 3, 7 have byte-7 high nibble 0 (mapper = `(b6>>4) | (b7 & 0xf0)`), so this line needs **no** relaxation for this phase's ids; it keeps rejecting mappers of 16 or more in v1. CONTEXT D-09's "v1 byte-7 high-nibble rule is relaxed only so that mapper numbers 2, 3 and 7 pass" is therefore a no-op for byte 7. The planner should not weaken :67 at all.
- :75 `if (mapper != 0u || (image[8] >> 4) != 0u || ...` — must become a profile lookup on `mapper` and a submapper check `<= 2`.
- :94 v1: `if ((image[6] >> 4) != 0u || (image[7] & 0xf3u) != 0u)` — the `image[6] >> 4` half is the v1 mapper check and must be replaced by the profile lookup; the `0xf3` half (VS/PlayChoice bits) stays.
- :102 PRG-bank rule (`image[4] != 1u && image[4] != 2u`) and :109 (`prg_size != 16384 && != 32768`) become per-board PRG rules.
- :104-107 CHR rule and :111 become per-board CHR rules.
- v1 images carry no submapper, so submapper is 0 for every v1 image (src/cartridge.c:228-231 sets it only for NES 2.0). A v1 mapper 2/3 image therefore gets AND conflicts and a v1 mapper 7 image none (D-15).
- NES 2.0 CHR-RAM: the NES 2.0 path requires `chr_size == 0 && chr_ram == 8192` (src/cartridge.c:87-88). The `tests/ines.h` builder never writes byte 11, so a synthetic NES 2.0 CHR-RAM image needs `image[11] = 0x07` patched after `ines_build` (or add a field to `struct ines_spec`). The ines.h top comment ("mapper 0 only") should be updated.

Suggested shape: a small `static const` table of integers (mapper id, max submapper, prg_unit, prg_min, prg_max, chr allowance flags) is fine under `tests/cmake/global_symbols.cmake` because it forbids only writable data and tables of pointers [VERIFIED: tests/cmake/global_symbols.cmake:1-15]. The board-specific CHR and mirroring cases are small enough for a `switch (mapper)` returning a profile struct by value.

### Pitfall 5: Source lists that name every library file
`CMakeLists.txt:55` lists `src/mapper_nrom.c` for the library, and `tests/CMakeLists.txt:520` lists it again for `bus.unit`, which compiles `bus.c`, `cartridge.c`, `ppu.c` and others directly [VERIFIED: grep, both lines read]. Once `cartridge.c` references the new `*_ops` and the helper, `bus.unit` fails to link unless all four new files are added there too.

### Pitfall 6: The hash inventory is hard-coded to 36 rows
`write_hashes.cmake` enforces `output_count EQUAL 36` and a fixed `required_manifest`, and loops over `nesteroids dabg rhde` only; `hash_inventory.cmake` hard-codes the key list, `row_count EQUAL 36` and the message "36-key inventory", and its self-test fixture builds the same rows [VERIFIED: tests/cmake/write_hashes.cmake, tests/cmake/hash_inventory.cmake read in full]. The CI step runs `write_hashes.cmake` on all six runners and `hash_inventory.cmake` compares them [VERIFIED: .github/workflows/ci.yml:112, :171]. `write_hashes.cmake`'s regex `^frame (1|30|60|120|180) ` will not pick up frame 600 or 1200 rows.
**How to avoid:** put the Holy Mapperel list (`name;rom path;N`) in one `tests/cmake/holymapperel_roms.cmake` that `write_hashes.cmake`, `hash_inventory.cmake` (real check and fixture) and `tests/CMakeLists.txt` all `include()`. Row key e.g. `holymapperel/m2/frame 600` and `.../frame 1200` (the existing second regex in hash_inventory accepts `key N ticks T sha256 H` with any `[^ ]+` key). Replace the literal 36 with 36 plus 2 times the list length (42 for three ROMs). Add the three manifest lines to `required_manifest`. Row counts, the 6-artifact rule and the sorted-rows rule stay. Do not change existing rows (D-18).
**Audio rows:** not required for Holy Mapperel; do not add `--hash-audio` rows (beeps are not a behaviour under test and would add 6 more keys for nothing).

### Pitfall 7: Frame count and the "static screen" assumption
Holy Mapperel runs CHR tests with buzzing (CHR-RAM ROMs do eight pattern passes) before printing, then plays beep codes with the screen on [CITED: Holy Mapperel README, "CHR test"; VERIFIED: main.s:199-215 and :457-467 read this session]. 600 frames (10 s NTSC) may not be enough for M2/M7. Process: run N=600, decode; on exit 2 double N until the decoder passes, then confirm `hash(N) == hash(2N)`. Pin the smallest round N where both hold. Because each emulated frame of 128 KiB ROMs costs little, 2N = 1200-2400 frames per ROM is acceptable, but `write_hashes` already carries `COST 50`; keep an eye on the asan leg time and raise `COST`/TIMEOUT if needed.
The frame hash line is `frame N ticks T sha256 H` (see `runner.hash` expectation) so the hash pin includes the tick count, which makes the pin sensitive to any timing change (good, intended).

### Pitfall 8: Submapper-0 AND on mapper 2 and 3 is a project choice, not NESdev's
NESdev's UxROM page says emulators implementing mapper 2 "treat this as a full 8-bit bank select register, without bus conflicts" and calls submapper 0 unknown; the Bus_conflict page says "an emulator should use the bitwise AND of the value from the CPU and the value from the ROM" and CNROM "is always subject to AND-type bus conflicts" [CITED: nesdev.org/wiki/UxROM, /Bus_conflict, /INES_Mapper_003 fetched this session]. D-15 picks AND for mappers 2 and 3 because BOARD-01 says so. The comment in `mapper_uxrom.c` should say "project default", not "hardware fact". Risk: a mapper-2 submapper-0 game that writes a non-matching value would bank wrongly under AND; every retail mapper-2 dump in NESdev's snapshot is submapper 2 anyway (project file NES-HARDWARE-PPU-CARTRIDGE.md line 97), so this affects homebrew only.

### Pitfall 9: Trainer images on the new boards
`nrom_rebuild` maps `cart.prg_ram` at $6000-$7FFF when the loader allocated it for a trainer [VERIFIED: src/mapper_nrom.c:14-19]. The loader still allocates and fills `prg_ram` for any trainer image (src/cartridge.c:195-200), and D-09 keeps the trainer path. If the three new boards do not repeat that loop, a mapper-2/3/7 trainer image loads its trainer into nothing. Put the $6000 loop in the shared helper with the mirroring block so all four boards behave the same. (A trainer test for one new board is cheap: copy `test_trainer_is_visible_through_cpu_bus`.)

### Pitfall 10: Warning flags and MSVC
`-Wconversion -Wsign-conversion -Wshadow` and `/W4` are fatal in presets [VERIFIED: CMakeLists.txt:17-22]. Cast narrowing explicitly (`(uint8_t)(value & 7u)`, `(uint32_t)reg * 16u`). Keep register types `uint8_t`, page indexes `uint32_t`, as `nrom_rebuild` does.

### Pitfall 11: Public header edits must be comment-only
`tests/cmake/vector_api_policy.cmake` pins a SHA-256 over the header after stripping comments and the version macros; any declaration change fails it [VERIFIED: tests/cmake/vector_api_policy.cmake:1-20]. D-12's header edits at lines 70, 163, 184, 239-247 are comments, which is safe. Do not add a declaration or touch a macro. Also keep D-18: do not bump the behaviour revision.

### Pitfall 12: Holy Mapperel specifics worth knowing before the first boot (all unverified until run)
- Open bus on $6000-$7FFF: these boards have no WRAM, so `cpu_r` there is NULL and reads return the open-bus latch. HM's WRAM test writes then reads $6000; if it detects "RAM" from open-bus echo the first digit will be non-zero. Phase 6 behaviour is correct hardware behaviour; treat a non-zero first digit as a bug to investigate, not something to mask [ASSUMED].
- The M2 and M7 ROMs are tested as mapper 2 and 7 by HM's own detection (mirroring writes), so a wrong `nt[]` for AxROM fails at `MIR` Morse code, which the decoder reports as exit 2.
- HM's drivers write bank numbers to ROM addresses that already contain the same value (it was built for UNROM 7432 boards), so the submapper-0 AND on mapper 2/3 should pass [ASSUMED; executor confirms on first boot].

## Code Examples

### Board sketch (UxROM), valid against the seam as read
```c
/* Source: pattern of src/mapper_nrom.c and src/mapper.h (read this session);
   hardware per NESdev Wiki "UxROM" and "Bus conflict". Illustrative; not compiled. */
#include "internal.h"

static void uxrom_init(struct nesturbator *nes)
{
    uint8_t conflict = (nes->mapper.submapper == 2u || nes->mapper.submapper == 0u);
    /* Project default for submapper 0 is the AND (BOARD-01); registers are zero from load. */
    nes->map.watch = (uint8_t)(NESTURBATOR_WATCH_CPU_WRITE |
                               (conflict ? NESTURBATOR_WATCH_BUS_CONFLICT : 0u));
}

static void uxrom_rebuild(struct nesturbator *nes)
{
    struct nesturbator__map *map = &nes->map;
    uint32_t last = (uint32_t)(nes->cart.prg_size / 1024u) - 16u; /* last 16 KiB bank, in 1 KiB pages */
    uint32_t sel = (uint32_t)nes->mapper.reg.uxrom.bank * 16u;
    nesturbator__map_trainer_ram(nes);            /* shared helper: $6000-$7FFF */
    for (uint32_t i = 16u; i < 32u; ++i) {        /* $8000-$BFFF switchable */
        map->cpu_r[i] = nesturbator__map_prg(nes, sel + (i - 16u));
        map->cpu_w[i] = NULL;
    }
    for (uint32_t i = 32u; i < 48u; ++i) {        /* $C000-$FFFF fixed to the last bank */
        map->cpu_r[i] = nesturbator__map_prg(nes, last + (i - 32u));
        map->cpu_w[i] = NULL;
    }
    nesturbator__map_chr_identity(nes);           /* chr_w only when chr_is_ram */
    nesturbator__map_header_mirroring(nes);
}

static void uxrom_cpu_write(struct nesturbator *nes, uint16_t addr, uint8_t value, uint64_t cpu_cycle)
{
    (void)cpu_cycle;
    if (addr < 0x8000u)
        return;
    nes->mapper.reg.uxrom.bank = value;
    nes->map.ops.rebuild(nes);
}
```
Notes: `watch` for submapper 1 must have no `BUS_CONFLICT`; the expression above includes only submappers 0 and 2, which for UxROM and CNROM is exactly D-15. AxROM's `init` uses `submapper == 2` only. The `cart.prg_size / 1024` is at least 16 because the validator enforces a multiple of 16 KiB.

### AxROM write
```c
/* NESdev "AxROM": xxxM xPPP. Source: nesdev.org/wiki/AxROM (fetched this session). */
static void axrom_cpu_write(struct nesturbator *nes, uint16_t addr, uint8_t value, uint64_t cpu_cycle)
{
    (void)cpu_cycle;
    if (addr < 0x8000u)
        return;
    nes->mapper.reg.axrom.bank = value;     /* store raw; rebuild uses (reg & 7), (reg >> 4) & 1 */
    nes->map.ops.rebuild(nes);
}
/* rebuild: pages 16..47 = map_prg(nes, (reg & 7) * 32 + (i - 16)); all nt[k] = (reg >> 4) & 1 */
```

### Holy Mapperel glyph table (verified from the committed M3 ROM CHR, this session)
Extracted from `M3_P32K_C32K_H.nes` (CHR starts at file offset 16 + 32768); each tile's two bitplanes were OR-ed per row and plane 1 turned out to be all zero (pixel value 1 only). Rows 5-7 are 0x00 in every glyph; bit 7 is the leftmost pixel. [VERIFIED: computed from the downloaded asset, this session]
```
tile $30 '0': 3c 66 66 66 3c 00 00 00      tile $01 'A': 3c 66 7e 66 66 00 00 00
tile $31 '1': 18 38 18 18 18 00 00 00      tile $02 'B': 7c 66 7c 66 7c 00 00 00
tile $32 '2': 7c 06 3c 60 7e 00 00 00      tile $03 'C': 3e 60 60 60 3e 00 00 00
tile $33 '3': 7c 06 3c 06 7c 00 00 00      tile $04 'D': 7c 66 66 66 7c 00 00 00
tile $34 '4': 1e 36 66 7f 06 00 00 00      tile $05 'E': 7e 60 7c 60 7e 00 00 00
tile $35 '5': 7e 60 7c 06 7c 00 00 00      tile $06 'F': 7e 60 7c 60 60 00 00 00
tile $36 '6': 3c 60 7c 66 3c 00 00 00
tile $37 '7': 7e 06 0c 18 18 00 00 00
tile $38 '8': 3c 66 3c 66 3c 00 00 00
tile $39 '9': 3c 66 3e 06 3c 00 00 00
```
The decoder's `glyph[16][8]` indexes 0-9 then A-F. The "DE" anchor is `glyph['D']` then `glyph['E']` at cells x=16 and x=24. Every glyph's (0,0) pixel is unlit, so the "lit = differs from the cell's (0,0)" rule holds. The CTest that checks the table against the M3 ROM reads bytes at `16 + 32768 + tile*16 + row` (plane 0) and `+8` (plane 1) and ORs them.

### Where the screen coordinates come from (re-derived this session from `src/main.s` at the pin)
- `start_line` loads `txt_y`, increments it, and builds PPUADDR with `sec; ror a; ror a; ror a` for the high byte and `ror a; and #$E0; ora #$02` for the low byte [VERIFIED: main.s:491-503]. For `txt_y = 8` this gives high `$21`, low `$02`, i.e. `$2102` = nametable 0, row 8, column 2.
- `txt_y` is set to 4 at main.s:203-204 and `start_line` is called unconditionally at main.s:205, 272, 279, 333, 355, so the detailed-result line is row 8 on every ROM (no branch skips a `start_line` between them) [VERIFIED: main.s:204-355 read, branch scan].
- "DETAILED TEST RESULT: " is 22 characters, so columns 2-23; `puthex` for `driver_prg_result` then `driver_chr_result` writes 4 digits at columns 24-27 [VERIFIED: main.s:354-362, 538-552, 654].
- Tile numbers: `puts` masks ASCII with `$3F` (D = `$04`, E = `$05`); `puthex` emits `'0'|nibble` for 0-9 (tiles `$30-$39`) and for A-F the `adc #<('A'-'9'-2-64)` path gives `$01`-`$06` [VERIFIED: arithmetic, main.s:538-555].
- Screen on with `VBLANK_NMI|BG_0000|OBJ_1000`, scroll (0,0) [VERIFIED: main.s:464-467]. BG palette index 0 entries are `$FF,$38,$38,$20`, all other entries `$FF` [VERIFIED: main.s:479-482] — `$FF` is masked to `$3F` by the PPU, which is why the decoder must not assume a colour.
- `driver_prg_result` is preset to `$C0` and `driver_chr_result` to `$DE` before the driver runs (`C0DE`), main.s:191-195 [VERIFIED].
- Digit meaning (README, "Displayed result"): "The detailed code is 4 digits: WRAM, PRG ROM, IRQ, and CHR ROM/RAM. Zero is normal" [CITED: Holy Mapperel README at the pin]. The first two digits are the `driver_prg_result` byte (WRAM, PRG window bitmask) and the last two the `driver_chr_result` byte (IRQ, CHR), consistent with D-07's order.
Remaining unverified: that the CHR-RAM ROMs load the same glyph shapes into RAM (README says "loads the small font into CHR RAM"); the executor confirms on first boot.

### Synthetic test technique
Build with `ines_build` then patch markers in place: PRG bank b begins at file offset `16 + b * 16384` (no trainer). Put the marker at `$8000` of each bank (e.g. `0x10 + b`) and at a second known offset; the AND test writes a value and checks `nes->map.cpu_r[16][0]` or reads via `nesturbator__map_cpu_read`. Use `nesturbator__bus_write(nes, 0x8000u, v)` (as `core.mapper` does) so the real path through `bus.c` is covered. Because the AND uses the ROM byte under the **old** banking, build the test as: ROM[$8000 in bank 0] = 0x03, write 0x07 -> reg 0x03 (AND), confirm bank 3 visible; same write at submapper 1 gives bank 7. For mapper 7: write 0x17 at bank 0 with ROM byte 0x03 -> expect bank 7 and `nt[]` all 1 (no AND); with submapper 2 expect `0x17 & 0x03 = 0x03`, bank 3, `nt[]` all 0. CNROM CHR drop: `CHECK(nes->map.chr_w[0] == NULL)`, write `$2006/$2007`, compare `nes->cart.chr[0]` before and after. UxROM CHR-RAM: write persists (`chr_w[0] != NULL` and the byte reads back).

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Reset vector read from the file's last 4 bytes | Read through the mapped pages | This phase (D-11) | Correct power-on for AxROM and later banked boards |
| `chr_size` fixed at 8192 | Loader-validated size | This phase | CNROM CHR-ROM beyond 8 KiB |
| Empty map for unknown ids | Hard failure before commit | This phase (D-10, closes IN-01) | No silently blank cartridges |

**Deprecated/outdated:** the text "mapper-0" in `runner/main.c:3-7,46,659`, `include/nesturbator.h:70,163,184,239-247`, `libretro/libretro.c:5`, `tests/libretro/libretro_host.c:929` (test comment), `README.md` lines 7, 11, 22-25, 33, 44, 99, 145, 624, 636. D-12 covers the runner and header and README; `libretro/libretro.c:5` is a comment in the same family and should change in the same commit. The README line 25 sentence "with NROM the only board so far" and line 44 ("three redistributable mapper-0 games") also need to name the new boards and the Holy Mapperel pins (rule 6).

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | The M2/M7 CHR-RAM ROMs show the same glyphs at the same row/columns as the M3 ROM | Code Examples | Decoder reports exit 2; executor corrects table/coordinates (D-05 already allows this) |
| A2 | N=600 frames is enough, then doubling finds a stable N | Pitfall 7 | Pin N differs; costs CI time only |
| A3 | HM drivers write conflict-safe values, so submapper-0 AND on mappers 2/3 still gives `0000` | Pitfall 12 | If `mapper 2`/`3` fail only under AND, the BOARD-01 default is wrong and needs the owner |
| A4 | Open-bus reads at $6000-$7FFF do not fake WRAM in HM's test | Pitfall 12 | First digit non-zero; investigate open-bus latch behaviour |
| A5 | A row of hash text per ROM at N and 2N is what the owner wants in `hashes.txt` | Pitfall 6 | Rename keys; trivial |

## Open Questions

1. **Does the phase branch start from a `main` that contains Phase 6?**
   - Known: the current branch is `gsd/phase-6-mapper-seam-and-ppu-fetch-pipeline`; PR #30 is a DRAFT (`gh pr list`, this session); CONTEXT says the Phase 7 branch must start from a `main` that includes Phase 6; the owner creates each phase branch.
   - Unclear: whether #30 has merged by execution time.
   - Recommendation: the plan's first task checks `git merge-base --is-ancestor` for the Phase 6 merge commit and stops with a one-line report if not.
2. **ROM-fetch step: network at execution.** The executor needs `curl` and `bsdtar` and network access once. Both exist here and the asset URL `https://github.com/pinobatch/holy-mapperel/releases/download/v0.02/holy-mapperel-bin-0.02.7z` downloaded and hashed to `70f85671...ce7a` this session [VERIFIED]. If the executor's sandbox has no network, this is the one step that needs the owner; the hashes in D-01 make the files checkable offline afterwards.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| cmake / ctest | all tests | yes | 4.4.3 (project minimum 3.25) | — |
| ninja | presets | yes | 1.13.2 | — |
| C compiler | build | yes | Apple clang 21.0.0 | — |
| curl | fetch HM asset | yes | /usr/bin/curl | — |
| bsdtar (7z reader) | extract HM asset | yes | 3.5.3, libarchive 3.7.4 | — |
| python3 | one-off glyph extraction only (not in the tree) | yes | 3.14.4 | do the extraction in C or CMake `file(READ ... HEX)` in the CTest |
| clang-format | `format_check` test | no (not found) | — | CI runs it; hand-format to `.clang-format` |
| network to github.com | HM asset download | yes (this session) | — | owner downloads the asset |

**Missing dependencies with no fallback:** none. **Missing with fallback:** clang-format (local only).

## Requirement to Test Map (BOARD-01)

`workflow.nyquist_validation` is false in `.planning/config.json`, so the formal Validation Architecture section is omitted; this table is the planner's checklist. Framework: CTest with the in-repo `tests/check.h`; quick run `cmake --workflow --preset dev` or `ctest -R "core.mapper|core.cartridge|hm\." ` in a built tree; full gate `cmake --workflow --preset ci` (`test_command` in config), plus `--preset asan`.

| Success criterion | Behaviour | Test | Notes |
|---|---|---|---|
| SC1 | HM M2/M3/M7 read `0000` from the runner's frame | `hm.<name>.dump` then `hm.<name>.decode` (PASS_REGULAR_EXPRESSION), `holymapperel.glyphs` checks the table against the M3 ROM, `holymapperel.decode.negative` feeds a blank PPM and a wrong-size PPM and expects exit 2 | Without the negative tests the decoder is untested against D-07 |
| SC1 | frame hashes pinned | `runner.write_hashes` + `runner.write_hashes.content` vs `tests/runner/hashes.txt`; CI `hash-equality` | Needs script changes (Pitfall 6) |
| SC2 | submapper 0: AND on 2 and 3, none on 7 | `core.mapper_discrete` cases | Pre-write banking (Technique above) |
| SC3 | submapper 1 never ANDs, 2 always; CNROM ignores CHR-ROM writes | same file | `chr_w == NULL` plus unchanged `cart.chr` |
| D-09 / D-13 | every accept and reject row | extended `core.cartridge` | Includes NES 2.0 mapper >= 256 and DiskDude |
| D-10 | every accepted id gets a board | `core.cartridge` or `core.mapper_discrete` loop over ids {0,2,3,7} calling `mapper_ops_for` | Also assert id 1, 4, 5 not found |
| D-11 | AxROM power-on PC from bank 0 | `core.mapper_discrete` | Banks carry different vectors; `ines_build` only writes vectors in the last bank, so patch the bank-0 vector bytes by hand |
| D-13 | fuzz | `fuzz.regress` replays new seeds | Seeds need manifest lines (pin: reuse `d813db3ed6bbda1f7b1c102bc74ebdecb8f5b58e`, the pin the existing seeds carry, or the current repository HEAD at creation time) |
| D-18 | NROM unchanged | all existing `runner.write_hashes.content` rows, `core.cartridge`, `core.reset` | If any existing hash changes: stop, defect |

Wave 0 gaps: `tests/core/test_mapper_discrete.c`, `tests/holymapperel/decode.c` and its negative-input fixtures, `tests/cmake/holymapperel_roms.cmake`, an `ines.h` option for NES 2.0 CHR-RAM byte 11. No framework install.

## Security Domain

`security_enforcement` is false in `.planning/config.json`, so no ASVS mapping is required. The one hostile-input surface is the ROM loader, already covered by the fuzz target (`fuzz.rom_loader`, replayed in CI, libFuzzer nightly). The relevant controls this phase keeps: size checks before multiplication, validate-before-allocate, the 64 MiB cap, and no board lookup after commit (Pitfall 2). The new UxROM maximum (4 MiB PRG) must stay within the 64 MiB cap and the `uint32_t` page arithmetic (`255 * 16 + 15` pages fits trivially).

## Sources

### Primary (HIGH confidence)
- This repository, read this session: `src/mapper.h`, `src/mapper_nrom.c`, `src/cartridge.c`, `src/bus.c:120-178`, `src/internal.h:150-240`, `src/instance.c:158-176`, `tests/ines.h`, `tests/mapper_test.h`, `tests/CMakeLists.txt` (lines 60-125, 195-240, 300-340, 505-530), `tests/cmake/{write_hashes,hash_inventory,check_ppm,expect_output,manifest_sha256,global_symbols,vector_api_policy}.cmake`, `tests/core/test_cartridge.c`, `.github/workflows/ci.yml`, `ASSET_POLICY.md`, `PROVENANCE.md`, `scripts/hygiene.sh`, `.planning/phases/06-*/06-REVIEW-DISPOSITION.md`, `.planning/preparation/README.md`, `.planning/preparation/CONFORMANCE.md` (Holy Mapperel row)
- Holy Mapperel at `c022622274ca8b83d214dea97e4388a6b0e92d8a`: `src/main.s`, `LICENSE` (zlib text, 22 lines), downloaded and read; release asset `holy-mapperel-bin-0.02.7z` sha256 recomputed and equal to D-01; the three ROM sha256 values and headers recomputed and equal to D-01

### Secondary (MEDIUM confidence, official wiki, fetched this session)
- https://www.nesdev.org/wiki/UxROM, https://www.nesdev.org/wiki/AxROM, https://www.nesdev.org/wiki/INES_Mapper_003, https://www.nesdev.org/wiki/Bus_conflict (summaries returned by the fetch tool; quotes above are as returned)

### Tertiary (LOW confidence)
- None used for recommendations. Items marked [ASSUMED] in the Assumptions Log depend on running the ROMs.

## Metadata

**Confidence breakdown:**
- Standard stack / seam usage: HIGH, read from the code.
- Architecture and loader changes: HIGH, with the four corrections to CONTEXT.md noted (Pitfalls 1-4, 6).
- Holy Mapperel decoding: MEDIUM-HIGH, traced from source and glyphs verified from ROM bytes, not yet run through this emulator.
- Pitfalls: HIGH for 1-11, MEDIUM for 12.

**Research date:** 2026-10-10
**Valid until:** 2026-11-09 (stable; the pins are fixed commits)
