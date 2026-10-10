# Requirements: nesturbator

**Defined:** 2026-10-09
**Core Value:** Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.

## v2 Requirements

Milestone v2: most of the library plays. These replace the v1 future requirements LIB-01 and LIB-02.

A platform is one of six: Linux, macOS and Windows, each on x64 and arm64. "Holy Mapperel passes" means the result code read from the runner's frame is `0000`; pinning the frame's hash is not enough. Synthetic tests build iNES images in C test code, so they need no entry in `tests/roms/manifest.txt`.

### Tune-up and v1 debt

- [x] **TUNE-01**: The test presets run CTest in parallel. A nightly job runs the suite with `--repeat until-fail:3 --schedule-random` and fails on any failure. The phase verification records the slowest CI leg's time before and after the change, and decides on ccache from the measured share of compile time.
- [x] **TUNE-02**: Action pins and runner labels are current, the phase verification records the AccuracyCoin pin as reviewed, and the README and the public header's comments describe the released code.
- [x] **TUNE-03**: The release-policy self-test fails when the publish gate has any extra condition, such as an appended `|| always()`.
- [x] **TUNE-04**: A synthetic trainer-bearing image runs through the runner and through the libretro test program, and both deliver equal frames.
- [x] **TUNE-05**: The local RetroArch tests either run on the owner's Mac or are removed, so the hosted `retroarch-e2e` job carries that evidence alone. A CI job fails when a test that its preset expects to run reports itself skipped.
- [x] **TUNE-06**: `nesturbator_reset()` soft-resets a loaded game. A test shows that CPU RAM, battery RAM and mapper registers are kept. It also shows that the CPU enters the reset vector with SP lowered by 3, that the APU is silenced, and that the PPU ignores register writes for the documented interval. `retro_reset()` calls it, and the libretro test program's frames after a reset equal those of a direct-API instance of the core given the same frames, reset and frames.

### Mapper interface and PPU

- [ ] **MAP-01**: NROM loads through the per-board mapper interface (page tables, a four-entry nametable map, a write hook stamped with the CPU cycle, mapper IRQ ORed with the APU sources), and every v1 frame and audio hash stays byte-identical.
- [ ] **MAP-02**: The PPU advances `v` with coarse-X and Y increments and the dot-257 and dot-280-to-304 copies, and fetches background and sprite patterns on their documented dots. A synthetic split-scroll test that writes `$2005` and `$2006` mid-frame matches its expected frame. The behaviour revision is bumped once, and each re-pinned hash is listed with the reason it changed.
- [ ] **MAP-03**: The loader accepts mappers 0, 1, 2, 3, 4 and 7. It rejects other mappers, MMC6 (mapper 4, submapper 1) and four-screen boards with a message and a non-zero exit status. The fuzz corpus and the cartridge tests reflect the new rules.

### Boards

- [ ] **BOARD-01**: UxROM, CNROM and AxROM games run:
  - Holy Mapperel's mapper 2, 3 and 7 ROMs pass, and their frame hashes are pinned.
  - Synthetic tests show the submapper-0 bus-conflict defaults: the written value ANDed with the ROM byte on mappers 2 and 3, and no conflicts on mapper 7.
  - Synthetic tests also show the NES 2.0 submapper 1 and 2 overrides, and that CNROM ignores writes to CHR-ROM.
- [ ] **BOARD-02**: MMC1 games run:
  - Holy Mapperel's mapper 1 ROMs pass.
  - Synthetic tests show a write on the cycle after another write is ignored, unless it sets the reset bit.
  - They also show the reset bit's effect on the control register, PRG-RAM enabled at power-on, and the SNROM, SOROM, SUROM and SXROM variants derived from ROM and RAM sizes.
- [ ] **BOARD-03**: MMC3 games run:
  - The IRQ counter is clocked by PPU A12 rises after A12 has been low for three M2 falls, and sprite fetches clock it even for empty slots.
  - Holy Mapperel's mapper 4 ROMs pass, including `$A001` PRG-RAM enable and write protect.
  - A synthetic test shows the NEC behaviour on submapper 4.
  - A nightly job fetches blargg's `mmc3_test_2` and `mmc3_irq_tests` at their pinned commit and passes the Sharp-revision tests. `6-MMC3_alt` is recorded as unsupported.
- [ ] **BOARD-04**: Frame and audio hashes for every board's committed ROMs are identical on all six platforms, and the libretro test program receives equal frames.

### Battery saves

- [ ] **SAVE-01**: `nesturbator_get_memory()` returns the battery span of a battery cartridge and NULL with size 0 otherwise.
  - The span is the NES 2.0 non-volatile size. For iNES 1 it is 8 KiB, or 32 KiB for mapper 1 with more than 256 KiB of PRG.
  - The pointer stays valid from load to unload.
  - `nesturbator_save_generation()` increases only when a write reaches the span.
- [ ] **SAVE-02**: `nesturbator-run --save-dir DIR` loads `DIR/<rom name>.sav` when it exists. On exit it writes the battery span there, through a temporary file and a rename, only if the span changed. Holy Mapperel's MMC1 battery ROM passes on a second run that starts from the first run's save. Without `--save-dir` the runner reads and writes no save file.
- [ ] **SAVE-03**: When the `.sav` length differs from the span, the runner prints both sizes and exits with status 4, and leaves the file byte-identical.
- [ ] **SAVE-04**: `nesturbator-run --save-interval N` writes a changed span every N frames. A test stops the runner after a flush and finds the current bytes on disk.
- [ ] **SAVE-05**: The libretro core returns the span for `RETRO_MEMORY_SAVE_RAM`. The `retroarch-e2e` job shows RetroArch writing a `.srm` of the span's size, and a second session loading it back.

### Playability

- [ ] **PLAY-01**: A CI test proves nes-runner plays from boot through interactive play:
  - A recorded input movie reaches gameplay.
  - A RAM byte that has meaning in the game differs between runs with and without input.
  - The scene visibly changes.
  - The audio keeps varying past a threshold.
  - Negative controls fail the same check: no input, an attract-mode-only run, and a failed load that falls back to the test frame.

## Future Requirements

Deferred. Tracked, but not in the v2 roadmap.

### Library coverage

- **LIB-03**: A committed MMC3 real-game hash from Stallar, once the provenance of its art is verified.
- **LIB-04**: Mapper 66 (GxROM), mappers 206, 5, 19, 16, 18, 69 and 210, four-screen cartridge VRAM, and MMC6.

### Carried from v1

- **STATE-01**: A player uses save states, rewind and run-ahead in RetroArch; a property test shows that saving, loading and continuing equals an uninterrupted run.
- **STATE-02**: The runner honours the process contract Playstead needs: a save directory, a flush on signal, and distinct exit statuses.
- **ACC-01**: Every AccuracyCoin test passes at the pinned commit.
- **ACC-02**: PAL and Dendy games run at their own timing.
- **PERF-01**: Property tests and long fuzz runs cover the loader, the state loader and bounded runs of arbitrary ROM bytes.
- **PERF-02**: CI compares instructions per frame between a pull request and `main`.
- **PERF-03**: `nesturbator-run --bench ROM` prints frames per second and frame-time percentiles.
- **HW-01**: Expansion audio, the Famicom Disk System and peripherals beyond the standard controller.

## Out of Scope

| Feature | Reason |
|---------|--------|
| Submitting the core to the libretro buildbot | An outward-facing step the owner decides on separately |
| Committing blargg's MMC3 ROMs | They state no licence; they are fetched nightly at a pinned commit instead |
| Padding or truncating a `.sav` of the wrong length | Truncating would lose the tail of a longer save at the first write-back; the runner refuses the file instead (SAVE-03) |
| A save file beside the ROM | It would make hash tests depend on leftover files; saves use only `--save-dir` |
| Signal-driven flushing in the runner for v2 | `--save-interval` covers a killed process; signal handling is part of STATE-02 |
| A core option for the MMC3 revision | The header's submapper selects it |
| ccache or another cache action without measurement | A new dependency on a pipeline of about 3 minutes; TUNE-01 decides from data |
| Code from GPL or LGPL emulators | The project is MIT and clean-room |

## Traceability

Which phases cover which requirements. Updated during roadmap creation.

| Requirement | Phase | Status |
|-------------|-------|--------|
| TUNE-01 | Phase 5 | Complete |
| TUNE-02 | Phase 5 | Complete |
| TUNE-03 | Phase 5 | Complete |
| TUNE-04 | Phase 5 | Complete |
| TUNE-05 | Phase 5 | Complete |
| TUNE-06 | Phase 5 | Complete |
| MAP-01 | Phase 6 | Pending |
| MAP-02 | Phase 6 | Pending |
| MAP-03 | Phase 9 | Pending |
| BOARD-01 | Phase 7 | Pending |
| BOARD-02 | Phase 8 | Pending |
| BOARD-03 | Phase 9 | Pending |
| BOARD-04 | Phase 10 | Pending |
| SAVE-01 | Phase 8 | Pending |
| SAVE-02 | Phase 8 | Pending |
| SAVE-03 | Phase 8 | Pending |
| SAVE-04 | Phase 8 | Pending |
| SAVE-05 | Phase 8 | Pending |
| PLAY-01 | Phase 10 | Pending |

**Coverage:**
- v2 requirements: 19 total
- Mapped to phases: 19
- Unmapped: 0

---
*Requirements defined: 2026-10-09*
*Last updated: 2026-10-09 after v2 roadmap creation*
