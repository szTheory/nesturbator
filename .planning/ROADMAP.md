# Roadmap: nesturbator

## Milestones

- ✅ **v1 NROM games with sound** — Phases 1–4.2 (shipped 2026-10-09) — [archive](milestones/v1-ROADMAP.md)
- 🚧 **v2 Most of the library plays** — Phases 5–10 (in progress)

## Overview

Milestone v2 takes nesturbator from NROM to the six common board families, which cover about 91% of the North American licensed library, and adds battery saves and an automated proof that a game plays from boot. A tune-up first makes CI fast and trustworthy and closes the v1 debt. NROM then moves onto a per-board mapper interface with its hashes unchanged. The PPU fetch pipeline is rebuilt next, so split scrolling and the MMC3 counter work, and every hash is re-pinned once. The boards follow from cheapest to hardest: UxROM, CNROM and AxROM; MMC1 with battery saves through the runner and RetroArch; MMC3 with its scanline IRQ. A close-out phase proves the hashes on all six platforms and that a game plays from boot. Each phase merges by pull request into a green `main`.

## Phases

<details>
<summary>✅ v1 NROM games with sound (Phases 1–4.2) — SHIPPED 2026-10-09</summary>

- [x] Phase 1: A test frame in RetroArch (12/12 plans) — completed 2026-10-03
- [x] Phase 2: The CPU matches the public vectors (11/11 plans) — completed 2026-10-06
- [x] Phase 3: A real game in RetroArch (15/15 plans) — completed 2026-10-09
- [x] Phase 4: Sound (4/4 plans) — completed 2026-10-09
- [x] Phase 4.1: Close gap: GAME-01 — initialize accepted iNES trainers (1/1 plan, INSERTED) — completed 2026-10-09
- [x] Phase 4.2: Close gap: SND-01 — deliver single-sample libretro audio (1/1 plan, INSERTED) — completed 2026-10-09

</details>

### 🚧 v2 Most of the library plays (Phases 5–10)

**Phase Numbering:**
- Integer phases (5, 6, 7): Planned milestone work
- Decimal phases (5.1, 5.2): Urgent insertions (marked with INSERTED)

- [x] **Phase 5: Tune-up and v1 debt** - CI runs in parallel without flakes, the v1 debt is closed, and a loaded game soft-resets (completed 2026-10-10)
- [x] **Phase 6: Mapper seam and PPU fetch pipeline** - NROM moves onto a per-board mapper interface with v1 hashes unchanged, then mid-frame scroll writes render as on the console and A12 rises on the documented dots (completed 2026-10-10)
- [ ] **Phase 7: UxROM, CNROM and AxROM** - Discrete-logic board games run, with bus conflicts where the board has them
- [ ] **Phase 8: MMC1 and battery saves** - MMC1 games run, and battery saves persist through the runner and RetroArch
- [ ] **Phase 9: MMC3** - MMC3 games run with the A12-clocked scanline IRQ, and the loader accepts exactly the six v2 mappers
- [ ] **Phase 10: Close-out and boot-to-play** - Every board's hashes match on six platforms, and a game is proven to play from boot

## Phase Details

### Phase 5: Tune-up and v1 debt

**Goal**: CI is fast and trustworthy, the v1 debt is closed, and a player who presses reset in RetroArch gets the console's soft reset.
**Depends on**: Phase 4.2 (v1 shipped)
**Requirements**: TUNE-01, TUNE-02, TUNE-03, TUNE-04, TUNE-05, TUNE-06
**Success Criteria** (what must be TRUE):
  1. `cmake --workflow --preset ci` runs CTest in parallel from the test presets; a nightly job runs the suite with `ctest --repeat until-fail:3 --schedule-random` and fails on any failure; the phase verification records the slowest CI leg's time before and after, and the ccache decision taken from the measured share of compile time.
  2. The release-policy self-test fails against a publish gate carrying any extra condition, such as an appended `|| always()`, and passes against the real gate; action pins and runner labels are current, and the phase verification records the AccuracyCoin pin as reviewed.
  3. A synthetic trainer-bearing iNES image, built in C test code, runs through `nesturbator-run` and through the libretro test program, and a CTest case finds the two frames equal.
  4. A CI job fails when a test that its preset expects to run reports itself skipped; the local RetroArch tests either pass on the owner's Mac or are removed, so the hosted `retroarch-e2e` job carries the RetroArch evidence alone.
  5. A CTest case shows that `nesturbator_reset()` keeps CPU RAM and cartridge RAM, enters the reset vector with SP lowered by 3, silences the APU and makes the PPU ignore register writes for the documented interval; `retro_reset()` calls it, the libretro test program's frames after a reset equal those of a direct-API instance of the core given the same frames, reset and frames, and the README and the public header's comments describe the released behaviour.

**Plans**: 8/8 plans complete

Plans:
**Wave 1**
- [x] 05-01-PLAN.md — Exact, job-scoped release publish-gate check with mutation self-test (TUNE-03)
- [x] 05-02-PLAN.md — Shared synthetic iNES builder; trainer image frames equal through runner and libretro (TUNE-04)
- [x] 05-03-PLAN.md — policy.no-skip, local RetroArch tests removed, hosted-only RetroArch driver (TUNE-05)

**Wave 2** *(blocked on Wave 1 completion)*
- [x] 05-04-PLAN.md — Parallel CTest with COST, nightly flake job, current action pins, CI record (TUNE-01, TUNE-02)

**Wave 3** *(blocked on Wave 2 completion)*
- [x] 05-05-PLAN.md — nesturbator_reset public API and CPU reset sequence with core.reset proof (TUNE-06, TUNE-02)

**Wave 4** *(blocked on Wave 3 completion)*
- [x] 05-06-PLAN.md — PPU write-ignore window from the top of the picture, APU silence, full D-18 coverage and docs (TUNE-06, TUNE-02)

**Wave 5** *(blocked on Wave 4 completion)*
- [x] 05-07-PLAN.md — retro_reset wired to the soft reset, libretro parity with a direct-API instance, README status (TUNE-06, TUNE-02)

**Gap closure** *(UAT G-05-2, G-05-3)*
- [x] 05-08-PLAN.md — gcc-14 conversion fix in test_reset.c with a gcc-14 sweep of phase 5 sources; suite-flake timeout 30 min (TUNE-01, TUNE-02, TUNE-06)

**Ships**: a release with parallel CI, the exact release-policy check and a working soft reset.

### Phase 6: Mapper seam and PPU fetch pipeline

**Goal**: Every cartridge loads through one per-board mapper interface, mid-frame scroll changes render as on the console, and PPU address line A12 rises on the dots where the hardware's does, so status bars and the MMC3 counter can work.
**Depends on**: Phase 5
**Requirements**: MAP-01, MAP-02
**Plan order**: the mapper seam lands first as its own plan, gated on unchanged v1 hashes; the fetch pipeline follows as a separate plan that carries the milestone's one behaviour-revision bump. No board shares a plan with either.
**Success Criteria** (what must be TRUE):
  1. NROM loads through the per-board ops table (page tables, a four-entry nametable map, a write hook stamped with the CPU cycle, mapper IRQ ORed with the APU sources); at the seam plan's commit, `cmake --workflow --preset ci` passes on all six platforms with every v1 frame and audio hash byte-identical and no pinned hash file changed in the diff.
  2. A CTest case drives a test board through the interface and sees each CPU write arrive with its CPU cycle index, and a CTest case raises the mapper IRQ source and an APU IRQ source independently and finds the CPU IRQ line equal to their OR.
  3. A synthetic split-scroll test that writes `$2005` and `$2006` mid-frame produces its expected frame, and CTest cases show `v` advancing by coarse-X and Y increments, the dot-257 horizontal copy and the dot-280-to-304 vertical copies, and background and sprite pattern fetches, with their A12 levels, on their documented dots, including fetches for empty sprite slots.
  4. The behaviour revision is bumped exactly once; the phase verification lists each re-pinned frame and audio hash with the reason it changed, the six-platform matrix agrees on every new hash, and the protected AccuracyCoin scoreboard loses no passing test.
  5. `cmake --workflow --preset asan`, `--preset nofp` and `--preset hygiene` pass, and the core still links only the C memory functions.

**Plans**: 2/2 plans complete

Plans:
**Wave 1**
- [x] 06-01-PLAN.md — Mapper seam: NROM on page tables, nametable map, cycle-stamped write hook and IRQ OR; v1 hashes unchanged, six-platform gate (MAP-01)

**Wave 2** *(blocked on Wave 1 completion)*
- [x] 06-02-PLAN.md — PPU fetch pipeline from v with A12 on the bus, ppu.fetch and ppu.split_scroll, revision 5 and one re-pin (MAP-02)

**Research flag**: HIGH — PPU fetch timing against the half-tick time base is the milestone's largest regression risk; measure the three NROM games' hashes before and after the pipeline plan.
**Ships**: a release whose split-scroll rendering matches the console, built on the new mapper interface.

### Phase 7: UxROM, CNROM and AxROM

**Goal**: Games on the discrete-logic boards UxROM, CNROM and AxROM run, with bus conflicts where the board has them.
**Depends on**: Phase 6
**Requirements**: BOARD-01
**Success Criteria** (what must be TRUE):
  1. Holy Mapperel's mapper 2, 3 and 7 ROMs report `0000`, read from the runner's frame by a CTest case, and their frame hashes are pinned in CI.
  2. Synthetic CTest cases show the submapper-0 bus-conflict defaults: the written value ANDed with the ROM byte on mappers 2 and 3, and no conflict on mapper 7.
  3. Synthetic CTest cases show the NES 2.0 submapper 1 and 2 overrides, and that CNROM ignores writes to CHR-ROM.

**Plans**: 2/4 plans executed

Plans:
**Wave 1**
- [x] 07-01-PLAN.md — Loader through one probed board switch with power-on PC through the pages (NROM tracer), then the UxROM board with its synthetic cases and wording (BOARD-01)

**Wave 2** *(blocked on Wave 1 completion)*
- [x] 07-02-PLAN.md — CNROM and AxROM boards, one per task with their synthetic cases and wording (BOARD-01)

**Wave 3** *(blocked on Wave 2 completion)*
- [ ] 07-03-PLAN.md — D-13 loader accept/reject matrix, D-10 id walk, six fuzz seeds, BOARD-01 edge classification (BOARD-01)

**Wave 4** *(blocked on Wave 3 completion)*
- [ ] 07-04-PLAN.md — Holy Mapperel mapper 2/3/7 ROMs read as 0000 from the frame, one frame hash per ROM pinned from one shared list, push and green CI (BOARD-01)

**Ships**: a release that plays UxROM, CNROM and AxROM games.

### Phase 8: MMC1 and battery saves

**Goal**: MMC1 games run, and their battery saves persist across runs as raw `.sav` bytes through the runner and through RetroArch.
**Depends on**: Phase 7
**Requirements**: BOARD-02, SAVE-01, SAVE-02, SAVE-03, SAVE-04, SAVE-05
**Success Criteria** (what must be TRUE):
  1. Holy Mapperel's mapper 1 ROMs report `0000`; synthetic CTest cases show that a write on the cycle after another write is ignored unless it sets the reset bit, the reset bit's effect on the control register, PRG-RAM enabled at power-on, the SNROM, SOROM, SUROM and SXROM variants derived from ROM and RAM sizes, and MMC1 registers and battery RAM kept across `nesturbator_reset()`.
  2. CTest cases show `nesturbator_get_memory()` returning the battery span sized by the NES 2.0 non-volatile size, or 8 KiB for iNES 1 and 32 KiB for mapper 1 with more than 256 KiB of PRG, and NULL with size 0 for a cartridge without a battery; the pointer stays valid from load to unload, and `nesturbator_save_generation()` increases only when a write reaches the span.
  3. Holy Mapperel's MMC1 battery ROM passes on a second `nesturbator-run --save-dir DIR` run that starts from the first run's save; the save is written through a temporary file and a rename only when the span changed; without `--save-dir` the runner reads and writes no save file; a `.sav` of the wrong length makes the runner print both sizes and exit with status 4, leaving the file byte-identical.
  4. A CTest case stops `nesturbator-run --save-interval N` after a flush and finds the span's current bytes on disk.
  5. The libretro core returns the span for `RETRO_MEMORY_SAVE_RAM`, and the `retroarch-e2e` job shows RetroArch writing a `.srm` of the span's size and a second session loading it back.

**Plans**: TBD
**Research flag**: MEDIUM — the SxROM bits, the NES 2.0 RAM sizes in header bytes 10 and 11, RetroArch's SRAM load and save ordering, and how the pinned RetroArch binary handles a `.srm` of the wrong size.
**Ships**: a release that plays MMC1 games and keeps their saves.

### Phase 9: MMC3

**Goal**: MMC3 games run with the scanline IRQ clocked by real PPU A12 rises, and the loader accepts exactly the six v2 mappers.
**Depends on**: Phase 8
**Requirements**: BOARD-03, MAP-03
**Success Criteria** (what must be TRUE):
  1. Synthetic CTest cases show the IRQ counter clocked by an A12 rise only after A12 has been low for three M2 falls, sprite fetches clocking it even for empty slots, and the NEC behaviour on submapper 4.
  2. Holy Mapperel's mapper 4 ROMs report `0000`, including `$A001` PRG-RAM enable and write protect.
  3. A nightly job fetches blargg's `mmc3_test_2` and `mmc3_irq_tests` at their pinned commit, checks their SHA-256s and passes the Sharp-revision tests; `6-MMC3_alt` is recorded as unsupported.
  4. The loader accepts mappers 0, 1, 2, 3, 4 and 7, and `nesturbator-run` rejects any other mapper, MMC6 (mapper 4, submapper 1) and four-screen boards with a message and a non-zero exit status; the fuzz corpus and the cartridge tests reflect the new rules.

**Plans**: TBD
**Research flag**: HIGH — the A12 M2-filter threshold, counter reload and IRQ semantics, the `$A000` mirroring polarity, and submapper 4.
**Ships**: a release that plays MMC3 games.

### Phase 10: Close-out and boot-to-play

**Goal**: Every board's output is the same on all six platforms, and a game is proven by an automated check to play from boot through interactive play.
**Depends on**: Phase 9
**Requirements**: BOARD-04, PLAY-01
**Success Criteria** (what must be TRUE):
  1. Frame and audio hashes for every board's committed ROMs are identical on Linux, macOS and Windows on x64 and arm64 in the CI matrix, and the libretro test program receives frames equal to the runner's for each of them.
  2. A CI test replays a recorded input movie on nes-runner from boot into gameplay: a RAM byte with meaning in the game differs between runs with and without input, the scene's frames change visibly, and the audio keeps varying past a threshold.
  3. The same check fails, in the same test, for each negative control: no input, an attract-mode-only run, and a failed load that falls back to the test frame.

**Plans**: TBD
**Research flag**: MEDIUM — confirm nes-runner reaches gameplay on this core and writes its save RAM.
**Ships**: the milestone's final release.

## Progress

**Execution Order:**
Phases execute in numeric order: 5 → 6 → 7 → 8 → 9 → 10

| Phase | Milestone | Plans Complete | Status | Completed |
|-------|-----------|----------------|--------|-----------|
| 1. A test frame in RetroArch | v1 | 12/12 | Complete | 2026-10-03 |
| 2. The CPU matches the public vectors | v1 | 11/11 | Complete | 2026-10-06 |
| 3. A real game in RetroArch | v1 | 15/15 | Complete | 2026-10-09 |
| 4. Sound | v1 | 4/4 | Complete | 2026-10-09 |
| 4.1. Close gap: GAME-01 | v1 | 1/1 | Complete | 2026-10-09 |
| 4.2. Close gap: SND-01 | v1 | 1/1 | Complete | 2026-10-09 |
| 5. Tune-up and v1 debt | v2 | 8/8 | Complete    | 2026-10-10 |
| 6. Mapper seam and PPU fetch pipeline | v2 | 2/2 | Complete    | 2026-10-10 |
| 7. UxROM, CNROM and AxROM | v2 | 2/4 | In Progress | - |
| 8. MMC1 and battery saves | v2 | 0/TBD | Not started | - |
| 9. MMC3 | v2 | 0/TBD | Not started | - |
| 10. Close-out and boot-to-play | v2 | 0/TBD | Not started | - |

## Backlog

### Phase 999.1: Follow-up — Phase 05 deferred UAT follow-up: Test 3 (BACKLOG)

**Goal:** Resolve the UAT checkpoint deferred during Phase 05 verification
**Source phase:** 05
**Deferred at:** 2026-10-10 during /gsd-verify-work 05 session completion
**Follow-ups:**
- [ ] Test 3: Lower nightly suite-flake timeout-minutes from the 30-minute ceiling to about twice the measured cold run (172 s job wall, so 6 to 10 minutes) (deferred 2026-10-10)
