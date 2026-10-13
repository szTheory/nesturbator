# Milestones

## v3 Save states and rewind (Shipped: 2026-10-13)

**Delivered:** Hosts save and load the whole machine through the library, players save and load states, rewind and use run-ahead in RetroArch, and a launcher can stop nes-runner at any time without losing a save and read every outcome from one documented exit-status table. No frame or audio hash moved, and speed is measured on every pull request.

**Phases completed:** 5 phases (11 to 15), 38 plans, merged as PRs #42 to #46
**Timeline:** 2026-10-11 → 2026-10-13 (3 days); the release is v0.1.10 in release PR #35, which is blocked (see below)
**Code:** 32,949 lines of C (8,804 in the core library and public header), excluding the vendored `libretro.h`; 633 CTest tests in `ci`
**Requirements:** 18/18 v3 requirements satisfied ([audit](milestones/v3-MILESTONE-AUDIT.md), status tech_debt)

**Key accomplishments:**
- Battery saves go through one atomic-replace helper that leaves no stale `.tmp`, `retroarch-e2e` round-trips an MMC3 `.srm` beside MMC1, and `nesturbator_get_version` and `nesturbator_unload_cartridge` have callers and README text.
- A fixed-size save state (64-byte header, CRC-32, one visitor for size, write, validate, apply and record) covers all six boards; every field is checked before anything is applied, and forks at every frame of eight ROMs continue byte for byte, pinned on six platforms through `--state-at-frame`, `--load-state` and `--hash-state`.
- RetroArch saves and loads states, rewinds and runs ahead with the core under a `deterministic` `.info` claim that a configure-time guard ties to its proving tests, and the state loader is fuzzed nightly.
- SIGINT, SIGTERM, Ctrl+C, Ctrl+Break and console close finish the frame, flush the save and exit 0, proven with real signals on six platforms; every exit status is one table that `--help` prints and a test holds to the README.
- `--bench` prints frames per second and frame-time percentiles, a Cachegrind `perf` job gates instructions per frame at +1% per ROM against the merge base, and every workflow timeout is twice a measured run whose ID sits beside it.

**Closed after the audit:** none; the audit ran on the merged head.

**Closeout:** override closeout. All five phase verifications passed, but GSD marked Phases 11 to 14 stale because later phases changed files they cover. CI, Nightly and Release passed on every job at `b0ec7fb`, and the milestone audit covers the phases.

Known verification overrides: 0 newly acknowledged, 9 carried forward from the v1 close (see STATE.md Deferred Items)

**Release blocker found at close:** release PR #35 (v0.1.10) has not merged since v2 because its `hygiene.format` check fails on `include/nesturbator.h:43-44`. The bumped patch number shifts the aligned `x-release-please` version comments, so no release has gone out since v0.1.9 and none of v3 has been published.

**Deferred tech debt:**
- Phase 13 WR-01: the state-net frame-chain check cannot detect a dropped or doubled FRAMEADVANCE.
- Phase 13 WR-02: `state_session.sh` sends most commands once, though RetroArch can miss one.
- Phase 13 WR-03: special-case ROM selection matches a substring of the full path.
- The RetroArch state, rewind and run-ahead e2e runs only on macOS; other platforms run `libretro.serialize` and `libretro.runahead`.
- The libretro adapter does not read `nesturbator_save_generation`; libretro has no such signal and the host writes the `.srm`.
- `runner/save.h:36` says state writes will use the atomic helper "later"; they already do.
- Open info findings in Phases 11, 13, 14 and 15 (listed in the audit), including the `.info` description that omits MMC3.
- Summaries for TUNE-07, TUNE-09, PERF-02, PERF-03 and TUNE-10 lack a `requirements-completed` key; the audit checked each by hand.

---

## v2 Most of the library plays (Shipped: 2026-10-11)

**Delivered:** Games on the six common board families (mappers 0, 1, 2, 3, 4 and 7, about 91% of the North American licensed library) play through the runner and RetroArch with battery saves, with frame and audio hashes identical on Linux, macOS and Windows on x64 and arm64, and one game proven to play from boot by an automated check.

**Phases completed:** 6 phases (05 to 10), 41 plans, 99 tasks
**Timeline:** 2026-10-10 → 2026-10-11 (2 days), releases v0.1.7 to v0.1.9, with v0.1.10 in release PR #35
**Code:** 22,419 lines of C (7,801 in the core library and public header), excluding the vendored `libretro.h`; 481 CTest tests in `ci`
**Requirements:** 19/19 v2 requirements satisfied ([audit](milestones/v2-MILESTONE-AUDIT.md), status tech_debt)

**Key accomplishments:**
- CI runs CTest in parallel with a nightly flake job, the v1 debt is closed (exact release-policy gate, trainer image through every host path, no self-skipping tests), and `retro_reset()` performs the console's soft reset.
- Every cartridge loads through one per-board mapper ops table, and the PPU runs the 2C02 fetch pipeline from `v` with A12 on the bus, so mid-frame scroll splits render as on the console.
- UxROM, CNROM, AxROM, MMC1 and MMC3 boards run, with bus conflicts by submapper and the MMC3 scanline IRQ clocked by M2-filtered A12 rises; Holy Mapperel reports `0000` on every board, and 10 of 12 blargg MMC3 ROMs pass nightly (the two NEC-revision ROMs are recorded unsupported).
- Battery saves persist as raw `.sav` bytes through a library span with a write generation, the runner's atomic `--save-dir` writes, and RetroArch's `.srm`.
- One 95-key hash list pins every board on six platforms, the libretro adapter matches the runner's frames and saves for every ROM in scope, and `runner.boot_to_play` proves nes-runner plays from boot against four negative controls and eight mutants.

**Closed after the audit:** G1 reset-retention tests (PR #38), 07 WR-01 AxROM reset-vector wording (PR #38), 08 WR-01 interval-flush retry at exit (PR #39), and the asan timeout, now 10 minutes against a measured 4m52s run (PR #39).

**Closeout:** override closeout. All six phase verifications passed, but GSD marked them stale because later phases and quick tasks #38 and #39 changed files they cover. CI passed on every job at `493b995`, and the milestone audit covers the phases.

Known verification overrides: 0 newly acknowledged, 9 carried forward from the v1 close (see STATE.md Deferred Items)

**Deferred tech debt:**
- G2: retroarch-e2e round-trips a `.srm` for MMC1 only; MMC3 saves are covered by the simulated host.
- G3: the MMC3 IRQ is checked against the blargg ROMs only in the nightly mmc3-oracle lane.
- G4: AccuracyCoin has no libretro parity test, by design (D-14).
- G6: `nesturbator_get_version` and `nesturbator_unload_cartridge` have no runner or libretro consumer and no README mention.
- Backlog 999.1: the nightly suite-flake timeout is a 30-minute ceiling, not twice a measured run.
- 08 IN-02: a failed save write can leave a stale `.tmp` file.
- Open review items in phases 05 to 10 (listed in the audit), most of them in test code.
- The PPU fetch pipeline cost about 19 percent frame rate; measured, not gated.

---

## v1 NROM games with sound (Shipped: 2026-10-09)

**Delivered:** NROM games render, take controller input and play with sound in RetroArch, with frame and audio hashes identical on Linux, macOS and Windows on x64 and arm64.

**Phases completed:** 6 phases (01, 02, 03, 04, 04.1, 04.2), 44 plans, 73 tasks
**Timeline:** 2026-10-02 → 2026-10-10 (8 days), releases v0.1.0 through v0.1.6
**Code:** 13,642 lines of C (5,964 in the core library), excluding the vendored `libretro.h`
**Requirements:** 19/19 v1 requirements satisfied ([audit](milestones/v1-MILESTONE-AUDIT.md))

**Key accomplishments:**
- A C17 library, headless runner and libretro core build, test and release on six platforms behind one required CI check; merging a behaviour change publishes 18 attested archives and `SHA256SUMS` with no manual step.
- The 6502 matches the public 65x02 vectors on all 256 opcodes on every bus cycle: a committed sample runs in CI, and the full 2,560,000-test set runs nightly at its pinned commit.
- A bounded mapper-0 iNES/NES 2.0 loader, a dot-timed PPU with sprites and OAM DMA, and two controller ports run three open-licence games whose frame hashes match byte for byte on all six platforms and in a pinned RetroArch screenshot check.
- Deterministic movie replay, AccuracyCoin results read from emulated RAM against a protected scoreboard, and loader fuzzing (corpus replay in CI, libFuzzer nightly).
- An integer APU and band-limited synthesiser produce mono PCM whose transition and sample hashes match on every platform; five reference tones stay below -80 dB of non-harmonic content, and libretro delivers equal stereo through both the batch and single-sample callbacks.

**Closeout:** override closeout. All six phase verifications passed, but GSD marked them stale because 03-15, 04.1, 04.2 and release version bumps later changed covered files. Those changes carry their own passing verifications, `main` CI passed at `5c082c1`, and the milestone audit covers them.

Known verification overrides: 9 newly acknowledged, 0 carried forward from a prior close (see STATE.md Deferred Items)

**Deferred tech debt:**
- `tests/cmake/release_policy.cmake` matches the publish gate by substring; an appended `|| always()` could evade it.
- No trainer-bearing fixture runs the full runner or libretro host path.
- The local `retroarch.testframe` and `retroarch.game` tests self-skip on the owner's Mac because RetroArch does not start there; only the hosted `retroarch-e2e` job gives live screenshot evidence.
- `retro_reset()` does nothing for a loaded game (outside v1 requirements).

---
