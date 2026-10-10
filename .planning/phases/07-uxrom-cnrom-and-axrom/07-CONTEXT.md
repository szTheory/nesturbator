# Phase 7: UxROM, CNROM and AxROM - Context

**Gathered:** 2026-10-10
**Status:** Ready for planning

<domain>
## Phase Boundary

Mapper 2 (UxROM), mapper 3 (CNROM) and mapper 7 (AxROM) cartridges load and
run on the Phase 6 mapper seam, with bus conflicts where the board has them
(BOARD-01). Holy Mapperel's mapper 2, 3 and 7 ROMs report `0000`, read from the
runner's frame by a CTest case, with frame hashes pinned. Synthetic cases show
the submapper 0, 1 and 2 bus-conflict rules and that CNROM ignores CHR-ROM
writes. The loader widens only as far as these three boards. MMC1 (Phase 8),
MMC3 and the final MAP-03 rule set (Phase 9) stay out.

</domain>

<decisions>
## Implementation Decisions

The owner chose three areas (Holy Mapperel in the tree, reading the 0000 code,
loader rules) and asked for a fan-out with an adversarial pass, followed by one
coherent set of recommendations without further questions. Board modules were
added as Claude's call so that the set stays consistent. Four researchers
reported. The orchestrator then checked their claims against the code. The
decisions below are the result.

### Holy Mapperel in the tree
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

### Reading the 0000 code
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

### Loader rules this phase
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

### Board modules (Claude's call, folded in for coherence)
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

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Scope and requirements
- `.planning/ROADMAP.md`: the Phase 7 goal and success criteria 1-3
- `.planning/REQUIREMENTS.md`: BOARD-01 (Phase 7), MAP-03 (Phase 9; this phase must not pre-empt its final message), BOARD-04 (six-platform hashes)
- `.planning/preparation/DECISIONS.md`: DEC.19 (mapper order) and DEC.20 (committed licensed test files only)

### Hardware and test ROMs
- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md` sections 5-6: mapper shares, bus-conflict submappers [HWP.13], the loader rules and HWP sources
- `.planning/preparation/CONFORMANCE.md`: the Holy Mapperel row, CF.05 pin, the licence table
- NESdev Wiki "UxROM", "CNROM", "AxROM", "Bus conflict", "NES 2.0 submappers" and "Mirroring", cited in hardware comments
- Holy Mapperel at `c022622274ca8b83d214dea97e4388a6b0e92d8a`: `README.md` (digit meanings) and `src/main.s` (result-screen layout). Zlib licence, so it may be read.

### Seam and prior decisions
- `.planning/phases/06-mapper-seam-and-ppu-fetch-pipeline/06-CONTEXT.md`: D-01 to D-08 (ops, watch bits, page tables, modulo, hook rules)
- `.planning/phases/06-mapper-seam-and-ppu-fetch-pipeline/06-REVIEW-DISPOSITION.md`: WR-01 and WR-03 (left open; see D-19)

### Repository policy
- `tests/roms/manifest.txt`, `ASSET_POLICY.md`, `PROVENANCE.md`, `THIRD-PARTY-NOTICES.md` and `scripts/hygiene.sh`: how ROMs and seeds are admitted

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `src/mapper_nrom.c`: the template for the three board files. Its mirroring block becomes the shared helper.
- `nesturbator__map_prg` and `nesturbator__map_chr` (`src/internal.h`): wrap bank numbers by true modulo, so boards store raw register bytes.
- `src/bus.c`: already ANDs the value with the mapped PRG byte when `BUS_CONFLICT` is watched.
- `tests/ines.h`: builds synthetic headers with mapper and submapper. `tests/mapper_test.h`: the test-board pattern.
- `runner --dump-frame N:FILE` and `runner/ppm.c` (the P6 writer), and the `runner.dump` fixture pattern and `tests/runner/hashes.txt` flow in `tests/CMakeLists.txt`.

### Established Patterns
- Validator-first loading: nothing is allocated until `validate_image` passes.
- Exact-total size rule, checked multiplication, a 64 MiB cap.
- Every committed binary has a manifest line and passes the hygiene check.

### Integration Points
- `src/cartridge.c`:
  - `validate_image`: the per-board profiles (D-09).
  - `nesturbator__mapper_load`: the switch, and the error on no match (D-10).
  - The load path: the power-on PC (D-11).
- `runner/main.c`: wording only (D-12). `include/nesturbator.h` and `README.md`: documentation of the support set.
- `tests/CMakeLists.txt`: the new board tests, the decoder target, and the per-ROM dump and decode chains.

</code_context>

<specifics>
## Specific Ideas

- The owner prefers "another copy-paste is better than another dependency". That rules out 7z or zlib readers, a cc65 build, or any helper library for the decoder.
- Delivery: this phase ships a release that plays UxROM, CNROM and AxROM games.
- The Phase 6 PR (#30) is still a draft and unmerged. The Phase 7 branch must start from a `main` that includes Phase 6.

</specifics>

<deferred>
## Deferred Ideas

- Oversize CNROM (up to 128 KiB CHR), 512 KiB AxROM and 2 KiB PRG-RAM CNROM are not supported yet. Add them when a target game needs them.
- Tolerance for DiskDude and other bad headers belongs to a separate product decision. Guessing a mapper from a corrupt byte 7 can load the wrong board.
- Mapper 180 (Crazy Climber, UxROM with the fixed bank at $8000) and mapper 185 (the CNROM copy-protection diodes) belong to the next-tier mappers.
- Phase 6's WR-01 and WR-03 are to be settled before Phases 8 and 9.

</deferred>

---

*Phase: 07-uxrom-cnrom-and-axrom*
*Context gathered: 2026-10-10*
