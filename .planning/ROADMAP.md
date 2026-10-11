# Roadmap: nesturbator

## Milestones

- ✅ **v1 NROM games with sound** — Phases 1–4.2 (shipped 2026-10-09) — [archive](milestones/v1-ROADMAP.md)
- ✅ **v2 Most of the library plays** — Phases 5–10 (shipped 2026-10-11) — [archive](milestones/v2-ROADMAP.md)
- 🚧 **v3 Save states and rewind** — Phases 11–15 (in progress)

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

<details>
<summary>✅ v2 Most of the library plays (Phases 5–10) — SHIPPED 2026-10-11</summary>

- [x] Phase 5: Tune-up and v1 debt (8/8 plans) — completed 2026-10-10
- [x] Phase 6: Mapper seam and PPU fetch pipeline (2/2 plans) — completed 2026-10-10
- [x] Phase 7: UxROM, CNROM and AxROM (4/4 plans) — completed 2026-10-10
- [x] Phase 8: MMC1 and battery saves (8/8 plans) — completed 2026-10-10
- [x] Phase 9: MMC3 (12/12 plans) — completed 2026-10-10
- [x] Phase 10: Close-out and boot-to-play (7/7 plans) — completed 2026-10-11

</details>

### 🚧 v3 Save states and rewind (Phases 11–15)

Players save and load states, rewind and use run-ahead in RetroArch. Playstead can launch nes-runner and rely on its saves and exit statuses. No frame or audio hash changes, and speed is measured on every pull request. Each phase ends in a release.

- [ ] **Phase 11: Tune-up and state readiness** - One atomic-write helper with no stale `.tmp`, MMC3 `.srm` round trip in RetroArch, a caller and README text for every public function
- [ ] **Phase 12: Save states in the library** - Fixed-size, checked, all-or-nothing states on six boards, pinned on six platforms through the runner's state options
- [ ] **Phase 13: Rewind and run-ahead in RetroArch** - libretro serialize, staged `.info` claims, RetroArch e2e for states, rewind and run-ahead, and a state-loader fuzz target
- [ ] **Phase 14: Runner process contract** - Signal-safe save flush and one documented exit-status table for launchers
- [ ] **Phase 15: Speed and close-out** - Frame-time report, per-PR instruction-count comparison, measured nightly timeout and a final pin and docs review

## Phase Details

### Phase 11: Tune-up and state readiness
**Goal**: Battery saves can never be left torn or stale, MMC3 saves survive RetroArch sessions, and every public function has a caller and README text, so state work inherits no v2 debt.
**Depends on**: Phase 10 (v2 shipped)
**Requirements**: TUNE-07, TUNE-08, TUNE-09
**Success Criteria** (what must be TRUE):
  1. A failure-injection test in `cmake --workflow --preset ci`, with an unwritable directory and with a directory at the target path, finds the old `.sav` intact and no `.tmp` left behind.
  2. The `retroarch-e2e` job round-trips an MMC3 battery game's `.srm` across two RetroArch sessions, as it does for MMC1, waiting on markers rather than sleeps.
  3. The runner or the libretro adapter calls `nesturbator_get_version` and `nesturbator_unload_cartridge`, a `ci` test exercises each call, and the README describes both.
**Plans**: TBD
**Notes**: State readiness from research rides here without its own requirement: white-box checks that frame-scoped output pointers are NULL and the PCM ring is empty at every frame boundary, the existing status numbers and the absence of exit 3 confirmed, and the runner's `save_state` struct renamed `battery_save`.

### Phase 12: Save states in the library
**Goal**: A host can save the whole machine at a frame boundary and load it back, getting exactly the run it would have had, on every board and every platform.
**Depends on**: Phase 11
**Requirements**: STATE-01, STATE-02, STATE-03, STATE-04, STATE-05, RUN-01
**Success Criteria** (what must be TRUE):
  1. A `ci` fork test saves, loads into a fresh instance pre-filled with another game, and continues, for a committed ROM on each of the six boards at several frames including 0. Every later frame hash, audio hash, tick count and the final state bytes equal an uninterrupted run, and saving every frame changes no pinned hash.
  2. `nesturbator-run --state-at-frame N:FILE`, `--load-state FILE` and `--hash-state N` work, a rejected state exits 5, and the state hashes in `tests/runner/hashes.txt` match on all six platforms in the CI matrix.
  3. Loads of a state from another game or board, another format version, cut short at every length, with any flipped byte, or with a recomputed CRC over out-of-range fields return distinct status codes and leave the instance byte-identical, under `ci` and the ASan and UBSan presets.
  4. A test shows the state size fixed from cartridge load to unload and 0 with no cartridge, and a save that never allocates and leaves the instance unchanged; the status codes are appended to the public header and the README has a "Save states" section.
  5. A test loads the same state 600 times with no change to `nesturbator_save_generation()`, and a load that changes battery bytes raises it exactly once; the README says rewinding past an in-game save reverts the save RAM.
**Plans**: TBD
**Notes**: RUN-01 lands here as test tooling, so the six-platform state hash pins run in this phase. Research flag: short spike on instruction boundaries in `src/frame.c` and the save and load cost.

### Phase 13: Rewind and run-ahead in RetroArch
**Goal**: Players save and load states, rewind and use run-ahead in RetroArch, and every `.info` claim is backed by a test on the same commit.
**Depends on**: Phase 12
**Requirements**: LR-01, LR-02, LR-03, LR-04
**Success Criteria** (what must be TRUE):
  1. The libretro host test shows `retro_serialize_size` equal to the library size, 0 with no content and constant across frames; short, long and corrupt buffers return false and the next frame equals one with no load.
  2. A simulated host that runs ahead one to three frames, with a save and load per frame, gives visible frame and audio hashes equal to an uninterrupted run on all six platforms, and a test checks that the `.info` file claims `serialized` and `deterministic`, and nothing else.
  3. The `retroarch-e2e` job saves and loads a state, rewinds, and runs with run-ahead enabled through RetroArch's network commands, and ends with no core error and the expected screenshot.
  4. A nightly libFuzzer target feeds the state loader, and its committed corpus is replayed as a test in `ci`.
**Plans**: TBD
**Notes**: `serialized` is claimed first, after the round trip is green on six platforms; `deterministic` only in the last commit, after the run-ahead host is green.

### Phase 14: Runner process contract
**Goal**: A launcher such as Playstead can stop nes-runner at any time without losing a save, and can read every outcome from one documented exit status.
**Depends on**: Phase 12 (exit 5); runs after Phase 13 under the one-step rule
**Requirements**: RUN-02, RUN-03
**Success Criteria** (what must be TRUE):
  1. A frame-indexed signal test on all six platforms shows the runner finishing the current frame, flushing the battery save and exiting 0, or 1 when the flush fails.
  2. On Linux and macOS an external SIGTERM test sees exit 0 within 2 s with the whole `.sav` and no `.tmp`, and a second signal ends the runner at once.
  3. The exit-status table (0, 1, 2, 4, 5; 3 and 77 reserved) is the same in one header, `--help` and the README, a test checks that, and each status has its own test.
  4. The README says Ctrl+Break and console close are handled on Windows and that a forced termination cannot be.
**Plans**: TBD
**Notes**: Research flag: Windows console-event delivery on hosted runners and fsync latency against 2 s need a look before planning.

### Phase 15: Speed and close-out
**Goal**: Speed is visible on every pull request, a slowdown cannot land unseen, and the milestone closes with a measured nightly timeout and docs that describe the released code.
**Depends on**: Phase 14
**Requirements**: PERF-02, PERF-03, TUNE-10
**Success Criteria** (what must be TRUE):
  1. `nesturbator-run --bench N` prints frames per second and p50, p90, p99, p99.9 and max frame times, `--bench-state` adds the save and load cost, and a `ci` test shows benchmarking changes no hash.
  2. Every pull request runs a Cachegrind job that compares instructions per frame between head and merge base on the committed NROM, MMC1 and MMC3 ROMs and writes the table to the job summary, without failing the CI roll-up while in report mode.
  3. The +1% per-ROM gate is armed only after identical builds show zero spread, and a deliberate slowdown passes with a reviewed line in the allow file.
  4. The nightly suite-flake timeout is about twice a measured cold run of the v3 suite, action pins, runner labels and the AccuracyCoin pin are reviewed, and the README and public header describe the released code.
**Plans**: TBD
**Notes**: TUNE-10 sits last so its cold run includes every v3 test. This absorbs Backlog 999.1 (nightly suite-flake timeout).

## Backlog

Phase 999.1 (lower the nightly suite-flake timeout to about twice a measured cold run) moved into Phase 15 as TUNE-10.

## Progress

| Phase | Milestone | Plans Complete | Status | Completed |
|-------|-----------|----------------|--------|-----------|
| 11. Tune-up and state readiness | v3 | 0/TBD | Not started | - |
| 12. Save states in the library | v3 | 0/TBD | Not started | - |
| 13. Rewind and run-ahead in RetroArch | v3 | 0/TBD | Not started | - |
| 14. Runner process contract | v3 | 0/TBD | Not started | - |
| 15. Speed and close-out | v3 | 0/TBD | Not started | - |
