# Project Research Summary

**Project:** nesturbator, milestone v2: most of the library plays
**Domain:** accurate, deterministic NES emulator core in C (library, headless runner, libretro adapter)
**Researched:** 2026-10-09
**Confidence:** MEDIUM

## Executive Summary

v2 adds the five board families after NROM, battery saves, a CI and v1-debt tune-up, and an automated proof that a game plays from boot. Together the families cover about 91% of the North American licensed library: MMC1 33.9%, MMC3 29.5%, UxROM 14.0%, CNROM 8.8% and AxROM 5.1% of 705 dumps. No new dependency is needed. The work is owned C, licensed test ROMs, a small synthetic iNES builder in the test code, and CI configuration.

The biggest finding is in the existing PPU. `src/ppu.c` draws each background pixel from `t` plus the absolute scanline. It never advances `v`: it has no coarse-X or Y increment, no dot-257 horizontal copy and no dot-280-to-304 vertical copy. It also fetches all sprite patterns at once at dot 257. As a result, MMC3's A12-clocked IRQ counter cannot work, and mid-frame `$2005`/`$2006` split scrolling renders wrongly. Status bars in many MMC1 and MMC3 games and in some discrete-board games use that split. The PPU fetch pipeline therefore has to be rebuilt before any board's hashes are pinned. That is the single behaviour-revision bump of the milestone.

Further risks:
- the timing of the MMC3 A12 filter
- the battery-save contract, which is frozen at its first release
- test ROMs that mislead about their result protocol
- boot-to-play checks that pass on attract mode or the test-frame fallback

Every risk has a prevention mapped to a phase below.

## Key Findings

### Recommended Stack

Details in STACK.md. RetroArch 1.22.2 stays the e2e host and `libretro.h` is unchanged. The action pins are current except upload-artifact v7.0.2 and download-artifact v8.0.2.

Do not add ccache. First measure how much of the slowest legs is compile time; they are 181 s on Intel macOS and 171 s under asan. Add `execution.jobs` to the test presets, because 357 tests run serially. Also add a nightly `ctest --repeat until-fail:3 --schedule-random` flake hunt and `mappers` and `games` labels.

**Test material:**
- **Holy Mapperel v0.02** (Zlib, committable). Commit 15 in-scope ROMs, about 2.4 MB, for mappers 1, 2, 3, 4, 7 and 66. The archive and file SHA-256s are in STACK.md. It reports a 4-digit on-screen code where 0000 means pass. `M1_P128K_C32K_S8K` writes `SAVEDATA` at $6100, which makes it a two-run battery test.
- **nes-runner v1** (MIT): MMC1, NES 2.0 header, 8 KiB battery RAM. This is the boot-to-play candidate. It has not been run on this core yet.
- **Stallar 0.1.0** (MIT): MMC3 with a battery and IRQ use. It is the MMC3 real-game check. Art provenance is MEDIUM confidence.
- **blargg `mmc3_test_2` and `mmc3_irq_tests`.** They state no licence, so they are not committed; they are fetched nightly at `christopherpow/nes-test-roms` @ `95d8f621ae55cee0d09b91519a8989ae0e64753b`, checked by SHA-256.
- **Synthetic iNES builder** in C test code, about 100 lines. No open-licence UxROM or AxROM game exists. This builder covers bus conflicts, the MMC1 double-write ignore, the CNROM CHR-ROM write ignore, battery persistence, soft reset and the trainer host-path debt.

### Expected Features

Details in FEATURES.md.

**Must have (table stakes):**
- **UxROM, CNROM and AxROM:**
  - bus conflicts with a per-mapper default, overridden by NES 2.0 submappers 1 and 2
  - CNROM ignores writes to CHR-ROM
  - AxROM single-screen mirroring
- **MMC1:**
  - the SNROM, SOROM, SUROM, SXROM and SZROM variants, derived from ROM and RAM sizes (deprecated submappers 1, 2 and 4 are not used)
  - consecutive-cycle writes ignored, but a reset-bit write is never ignored
  - PRG-RAM enabled at power-on
  - control register pinned to `$0C` at power-on
- **MMC3:**
  - IRQ clocked by real PPU A12 rises after three M2 falls with A12 low
  - sprite fetches clock the counter even for empty slots
  - Sharp (revision B) behaviour by default; NEC/MMC3A on submapper 4
- **Battery saves:**
  - NVRAM is kept apart from volatile WRAM
  - libretro `RETRO_MEMORY_SAVE_RAM` returns a stable pointer and size for battery carts only
  - the runner reads and writes `.sav` under `--save-dir`, with temp-file-plus-rename
- **`retro_reset()` is a soft reset:**
  - kept: CPU RAM, NVRAM and mapper registers
  - changed: the CPU takes the reset vector with SP lowered by 3, the APU is silenced, and the PPU ignores register writes for about 29,658 CPU cycles
- **Debt:**
  - the release-policy check matches the gate exactly
  - a trainer-bearing image runs through both hosts
  - the local RetroArch tests run or are retired; a skip fails CI

**Should have:**
- runner `--save-interval`
- an e2e assertion that RetroArch writes and reloads an 8192-byte `.srm`
- per-board frame hashes

**Defer:**
- MMC6 (submapper 1, refused in v2)
- four-screen cartridge VRAM (Gauntlet, Rad Racer 2)
- MMC3 `$A001` RAM protect: implement it only if it is cheap, otherwise mark those Holy Mapperel rows `unsupported`
- mappers 34, 71, 94, 155, 180 and 185

### Architecture Approach

Details in ARCHITECTURE.md. NROM is hard-wired in six places:
- `validate_image`
- `nesturbator__cart_read`
- `load_cartridge`
- mirroring and CHR handling in `ppu.c`
- the `$6000`–`$FFFF` handling in `bus.c`
- the APU's direct writes to `cpu.irq_line`

Replace these with a seam: a `static const` per-board ops table with `accept`, `power_on`, `reset`, `cpu_write(addr, value, cpu_cycle)` and an optional `ppu_bus(addr, ppu_ticks)`. Per-board registers live in a union inside the instance.

**Major components:**
1. **Mapper seam.** Page tables are `prg_off[32]` and `chr_off[8]` at 1 KiB, plus a four-entry `nt_page[4]` nametable source table for H, V, single-screen and four-screen. A `nesturbator__bus_conflict` helper and a `nesturbator__irq_recompute` OR together the APU and mapper IRQ sources.
2. **PPU fetch pipeline.** Real `v` increments and copies, per-dot background and sprite fetches, and A12 that is visible from `$2006`/`$2007` outside rendering.
3. **Memory API.** New calls are added to the header without breaking ABI 1:
   - `nesturbator_reset`
   - `enum nesturbator_memory`
   - `nesturbator_get_memory(inst, kind, &ptr, &size)`
   - `nesturbator_save_generation()`, a monotonic dirty counter

   The library does no file I/O; the runner and libretro own the files.

### Critical Pitfalls

Details in PITFALLS.md.
1. **The `.sav` is a hidden test input.** Use only an explicit `--save-dir`, with a temp dir per test. Expose only the battery span, never the whole 32 KiB MMC1 allocation.
2. **MMC1 double-write ignore implemented as an "RMW opcode" shortcut.** Pass the CPU cycle index into the write hook instead.
3. **MMC3 IRQ modelled as a scanline timer, or A12 taken from per-pixel reads.** Either fails Crystalis, StarTropics and Wario's Woods. Build the fetch pipeline first, and gate the hook behind a capability bit so boards without `ppu_bus` pay nothing.
4. **Misread test protocols.**
   - `$6000` suites need the `$DE $B0 $61` signature before their status byte means anything.
   - `$F8` suites use the opposite polarity.
   - Holy Mapperel must be decoded to `0000`; pinning its hash is not enough.
   - `6-MMC3_alt` targets revision A and is recorded as `unsupported`.
5. **False-passing boot-to-play.** Attract mode, a frame counter, any non-zero sample, the test-frame fallback and an NMI-alive hang can all pass a weak check. Require a with/without-input A/B divergence on a semantic RAM byte, sustained audio variation, and negative controls in the same plan.

## Resolved Conflicts

1. **Bus-conflict default for submapper 0 and iNES 1.**
   - Mappers 2 and 3: AND, because every licensed UxROM and CNROM dump is submapper 2 and the games are written around the conflict.
   - Mapper 7: none, because Double Dare and Wheel of Fortune glitch when AxROM conflicts are emulated. The dump count does not support "none", since 31 of 58 dumps have conflicts; the glitch reports do.
   - Confirm the board Cybernoid uses before citing it.
   - Holy Mapperel's mapper 2 and 7 codes confirm the final default.
2. **Phase order.** Tune-up, then mapper seam, then PPU fetch pipeline, then discrete boards, then MMC1 and saves, then MMC3, then close-out and boot-to-play.
   - The pipeline goes before every board, because it changes split-scroll output for all of them. Otherwise board hashes would be pinned twice.
   - MMC1 does not need to come first, because the cycle-stamped write hook lands in the seam phase.
   - The cost is that MMC1 saves arrive one phase later.
3. **A `.sav` whose length differs from the core's span.** The length must match exactly, or the file is refused: the runner does not load or write it, leaves it byte-identical, prints both sizes and exits with status 4.
   - Padding and truncating would lose the tail of a longer save at the first write-back.
   - The span is the NES 2.0 non-volatile size when declared. For iNES 1 with a battery it is 8 KiB, or 32 KiB for mapper 1 with more than 256 KiB of PRG.
   - RetroArch's handling of a mismatched `.srm` is measured with the pinned binary, not taken from its source.
4. **Clean room.** No GPL or LGPL emulator source; reference emulators run only as released binaries. Unlicensed test ROMs are fetched nightly at a pin and never committed.

Other choices:
- The runner uses `--save-interval` instead of signal handlers.
- The header selects the MMC3 revision; it is not a user option.

## Implications for Roadmap

Phase numbering continues from v1, which ended at 4.2.

### Phase 5: Tune-up and v1 debt
**Rationale:** Every later phase adds hashes, and a slow or flaky matrix would hide mapper regressions.
**Delivers:**
- parallel CTest and the nightly flake hunt
- action bumps
- a compile-time measurement and a recorded ccache decision
- an exact release-policy check
- a trainer fixture through both hosts
- a decision on the local RetroArch tests, with any skip failing CI
- `nesturbator_reset` and a soft `retro_reset()` for NROM

**Avoids:** Pitfalls on self-skips hiding rot and the reset that wipes saves.

### Phase 6: Mapper seam
**Rationale:** Every board depends on it.
**Delivers:**
- the ops table, page tables, `nt_page[4]`, the IRQ OR and the cycle-stamped write hook
- NROM routed through the seam, with byte-identical v1 hashes as the gate

### Phase 7: PPU fetch pipeline
**Rationale:** MMC3 needs it, and split-scroll games on every board need it.
**Delivers:**
- real `v` increments and copies, per-dot fetches and A12 events
- one behaviour-revision bump, with NROM hashes re-pinned once and explained

It does not share a plan with any board.

### Phase 8: UxROM, CNROM, AxROM
**Rationale:** The cheapest boards prove the seam, submapper plumbing, single-screen mirroring and CHR-RAM.
**Delivers:** Holy Mapperel 2, 3, 66 and 7; synthetic bus-conflict tests; per-board hashes.

### Phase 9: MMC1 and battery saves end to end
**Delivers:**
- the MMC1 variants
- `nesturbator_get_memory` and the save generation counter
- runner `--save-dir` and `--save-interval`, with exit status 4 on a size mismatch
- libretro SRAM, and a `.srm` round trip in `retroarch-e2e`
- the Holy Mapperel M1 two-run battery test
- nes-runner committed

### Phase 10: MMC3 with the scanline IRQ
**Delivers:**
- the A12 filter, Sharp by default and NEC on submapper 4
- Holy Mapperel M4 and Stallar
- the nightly blargg fetch, which must pass `5-MMC3` and records `6-MMC3_alt` as unsupported

### Phase 11: Close-out and boot-to-play
**Delivers:**
- remaining per-board pins
- the nes-runner boot-to-play check, with A/B input divergence, a semantic RAM byte, sustained audio and negative controls

Each board's own pins begin in its own phase; this phase only closes gaps.

### Phase Ordering Rationale

- Each step depends on the one before: seam, then pipeline, then boards. The cycle stamp and the IRQ OR are in the seam, so the boards' order among themselves is free.
- Putting the pipeline before any board's pins means one behaviour-revision bump and no double re-pinning.
- Cheap boards first prove the seam before the two hard chips.

### Research Flags

These phases need deeper research during planning:
- **Phase 7 (HIGH):** the PPU fetch timing against the half-tick time base. This is the largest regression risk.
- **Phase 10 (HIGH):** the A12 M2 filter threshold, reload and IRQ semantics, and submapper 4.
- **Phase 9 (MEDIUM):** the SxROM bits, the NES 2.0 RAM sizes in bytes 10 and 11, RetroArch's SRAM load/save ordering, and its handling of a mismatched size.
- **Phase 11 (MEDIUM):** confirm nes-runner reaches gameplay and writes its save RAM.

These phases follow standard patterns: Phase 6 and Phase 8.

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | Licences, pins and SHA-256s fetched and computed on 2026-10-09; CI timings come from the latest green `main` run. |
| Features | MEDIUM-HIGH | Board facts from the NESdev Wiki and the Holy Mapperel README; the save-flush and complaint ranking are partly from search summaries. |
| Architecture | HIGH on integration, MEDIUM on hardware | Integration points read from the code; hardware details from the preparation docs. |
| Pitfalls | MEDIUM-HIGH | MMC1 and MMC3 re-read on the wiki; bus conflicts and libretro SRAM ordering not re-fetched. |

**Overall confidence:** MEDIUM

### Gaps to Address

These remain unverified; resolve each during its phase:
- **How much `v1` hash output changes** under the pipeline. Measure the three NROM games before and after in Phase 7.
- **Which MMC3 revision games need.** Test ROMs, not games, prove the revision.
- **MMC3 `$A000` mirroring polarity.** Confirm on the wiki before coding.
- **Holy Mapperel's result layout and the frames it needs.** Measure in Phase 8.
- **Exact soft-reset state list and MMC1 power-on value.** Confirm in the Phase 5 and Phase 9 plans.
- **Intel macOS leg.** It is supported until the macOS 15 image retires, around fall 2027. Record a demotion plan in Phase 5.

## Sources

Full source tables are in STACK.md, FEATURES.md, ARCHITECTURE.md and PITFALLS.md.

### Primary (HIGH confidence)
- NESdev Wiki: MMC1, MMC3, UxROM, CNROM, AxROM, NES 2.0 submappers, bus conflicts, PPU rendering.
- The vendored `libretro.h`: `RETRO_MEMORY_SAVE_RAM`, `retro_reset`, memory maps.
- Holy Mapperel v0.02 release and README (Zlib), the nes-runner v1 and Stallar 0.1.0 repositories (MIT), and `christopherpow/nes-test-roms` @ `95d8f62`.
- This repository: `src/ppu.c`, `src/bus.c`, `src/apu.c`, `include/nesturbator.h`, `libretro/libretro.c`, and the v1 audit and retrospective.

### Secondary (MEDIUM confidence)
- `.planning/preparation/` reference files: dump counts, conformance suites and the ecosystem report.
- GitHub Actions release and runner-image notices.

### Tertiary (LOW confidence)
- User-complaint ranking from search summaries.
- nes-runner's save and gameplay behaviour, which has not been run on this core.

---
*Research completed: 2026-10-09*
*Ready for roadmap: yes*
