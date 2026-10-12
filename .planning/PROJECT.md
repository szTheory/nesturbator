# nesturbator

## What This Is

nesturbator is a NES emulator core written in C. It ships as a library other programs embed, a headless command-line runner for automation, and a libretro adapter so that hosts such as RetroArch can play games with it. It is for people who build emulator hosts and want an accurate, permissively licensed NES core, and for the players who use those hosts.

## Core Value

Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.

## Current State

v2 "Most of the library plays" shipped on 2026-10-11 (releases v0.1.7 to v0.1.9, with v0.1.10 in release PR #35). Games on mappers 0, 1, 2, 3, 4 and 7, about 91% of the North American licensed library, play through the runner and RetroArch with battery saves. Frame and audio hashes for every board match on Linux, macOS and Windows on x64 and arm64, and `runner.boot_to_play` proves a game plays from boot. The code is 22,419 lines of C, 7,801 of them in the core library and public header, with 481 CTest tests in `ci`.

v1 "NROM games with sound" shipped on 2026-10-09 (v0.1.6). The milestone records are in `.planning/MILESTONES.md`, and each milestone's roadmap, requirements, audit and phase history are in `.planning/milestones/`.

## Current Milestone: v3 Save states and rewind

**Goal:** Players save and load states, rewind and use run-ahead in RetroArch, and Playstead can launch nes-runner and rely on its saves and exit statuses, with no frame or audio hash changing and speed measured on every pull request.

**Target features:**
- Tune-up (SEED-006) and v2 debt: CI time, flaky tests, pins and README drift; an MMC3 `.srm` round trip in retroarch-e2e (G2); a consumer or README text for `nesturbator_get_version` and `nesturbator_unload_cartridge` (G6); no stale `.tmp` after a failed save (08 IN-02); a measured nightly suite-flake timeout (Backlog 999.1).
- Save states (SEED-002): a fixed-size state with its own format version, checked in full before it is applied, through the library API; RetroArch save and load state, rewind and run-ahead, declared deterministic so netplay can use it.
- Runner contract (SEED-002): save states in nes-runner, saves flushed on a termination signal, and documented exit statuses for Playstead.
- Speed (SEED-004, speed half): a runner mode that prints frames per second and frame-time percentiles, and an instruction-count comparison on every pull request.

Left for later milestones: fuzzing the state loader and arbitrary ROM bytes (SEED-004), accuracy and PAL/Dendy (SEED-003), Famicom hardware (SEED-005).

## Requirements

### Validated

- ✓ One command installs the core into RetroArch on an Apple Silicon Mac, and a test shows RetroArch displays the runner's frame. — v1, Phase 1 (v0.1.0)
- ✓ `cmake --workflow --preset ci` (`ci-msvc` on Windows) builds and tests everything on Linux, macOS and Windows, x64 and arm64, and CI runs the same commands. — v1, Phase 1
- ✓ Every behaviour-changing merge to `main` produces a release with the library, the runner and the libretro core. — v1, v0.1.0 through v0.1.6, published automatically
- ✓ The CPU matches the public 65x02 vectors on all 256 opcodes and every bus cycle. — v1, Phase 2 (62/62 verification truths passed)
- ✓ The pinned full 65x02 vector set passes, with CI scheduled to run it nightly. — v1, Phase 2 (scheduled runs passed 2026-10-07 to 2026-10-09)
- ✓ NROM games render, take controller input and play with sound in RetroArch. — v1, Phases 3–4.2 (frame, controller, APU, and libretro callback behavior have automated coverage)
- ✓ Frame and audio hashes are identical on every supported platform. — v1, Phases 3–4 (pinned outputs run in the six-platform CI matrix)
- ✓ CI runs CTest in parallel with a nightly flake job, and the v1 debt is closed: exact release-policy gate check, trainer image through the full host path, a soft-resetting `retro_reset()`, and no self-skipping tests. — v2, Phase 5 (PR #25 CI and nightly green; asan leg 171 s to 56 s)
- ✓ Every cartridge loads through one per-board mapper interface with cycle-stamped CPU writes and one IRQ OR, and the PPU runs the 2C02 fetch pipeline with A12 reported to the board, so mid-frame scroll splits render as on the console. — v2, Phase 6 (MAP-01, MAP-02; CI runs 38070510500 and 38072045594 green on all jobs)
- ✓ UxROM, CNROM and AxROM games run, with bus conflicts where the board has them; Holy Mapperel M2, M3 and M7 report `0000` and pin one frame hash each on six platforms. — v2, Phase 7 (BOARD-01; CI run 38082253437 green on all jobs)
- ✓ MMC1 games run, and battery saves persist across runs as raw `.sav` bytes through the library span, the runner's `--save-dir` and RetroArch's `.srm`; Holy Mapperel's five mapper 1 ROMs report `0000` and the SXROM ROM reads its own save back. — v2, Phase 8 (BOARD-02, SAVE-01 to SAVE-05; CI run 38096603810 and Nightly run 38096603867 green on all jobs)
- ✓ MMC3 games run with the scanline IRQ clocked by M2-filtered PPU A12 rises (Sharp, NEC by submapper 4), and the loader accepts exactly mappers 0, 1, 2, 3, 4 and 7; Holy Mapperel's two mapper 4 ROMs and two derived PRG RAM copies report `0000`, and 10 of 12 blargg MMC3 ROMs pass nightly with the two NEC-revision ROMs recorded unsupported. — v2, Phase 9 (BOARD-03, MAP-03; CI run 38108996110 green on all jobs)

- ✓ Every board's frames and audio are pinned by one 95-key hash list on six platforms, and the libretro adapter receives frames and save spans equal to the runner's for every ROM in scope (19 parity tests); a drift guard rejects any committed ROM that is neither hashed nor exempt. — v2, Phase 10 (BOARD-04; six-platform equality shown by the PR's CI matrix)
- ✓ One supported game is proven from boot through interactive play by an automated check: `runner.boot_to_play` replays a nes-runner movie and requires a RAM, scene and audio change that four negative controls and eight synthetic mutants fail. — v2, Phase 10 (PLAY-01)
- ✓ Battery saves are replaced atomically (temp file, full flush, checked close, rename; a stale `.tmp` file or link is cleared and a directory refused), the `retroarch-e2e` job round-trips an MMC3 `.srm` beside MMC1, and `nesturbator_get_version` and `nesturbator_unload_cartridge` have callers and README text. — v3, Phase 11 (TUNE-07, TUNE-08, TUNE-09; PR #42 CI run 38179044953 green on all jobs)
- ✓ A host saves the whole machine at a frame boundary into a fixed-size, versioned, CRC-checked state and loads it back to the same run on all six boards; every field is validated before anything is applied, a refused load leaves the instance unchanged, and nes-runner has `--save-state` and `--load-state`. — v3, Phase 12 (STATE-01 to STATE-05, RUN-01; six-platform hash equality shown by the PR's CI matrix)

### Active

- [ ] v2 debt closed and the pipeline tuned up (SEED-006)
- [ ] Save states in RetroArch with rewind and run-ahead (the library API shipped in Phase 12)
- [ ] nes-runner signal-safe save flush and documented exit statuses (runner save states shipped in Phase 12)
- [ ] Speed measured by the runner and compared on every pull request

The numbered requirements are in `.planning/REQUIREMENTS.md`.

### Out of Scope

- A GUI, or any window, audio-device or input-device code — a shared host application will provide these for every core; RetroArch does until then.
- Shaders, menus, input mapping and netplay lobbies — these belong to the host.
- Bundled ROMs, BIOS files or a ROM database — players supply their own games, and no such database publishes a licence.
- Code from GPL or LGPL emulators — the project is MIT and written from hardware documentation.
- A separate fast or low-accuracy mode — one accurate core keeps one set of bugs and one set of hashes.
- Signed or notarized macOS artifacts — the script install path does not need them.
- A test framework dependency — plain CTest executables with an in-repo `check.h` cover the need.
- PAL and Dendy timing, mappers beyond the six common families, expansion audio, FDS, extra peripherals — planned for later milestones; see `.planning/seeds/`.
- Submitting the core to the libretro buildbot — an outward-facing step the owner decides on separately.

## Context

- **Ecosystem.** Every mainstream libretro NES core is GPL or LGPL. The most accurate emulators are GPL applications or permissive C# and Java programs. A permissive, accuracy-class, embeddable C core does not exist yet. AccuracyCoin, an MIT test ROM with 144 tests, is today's public measure of accuracy.
- **Sibling projects.** A Neo Geo core with the same three deliverables and the same API conventions. Playstead, a host that launches emulator processes and needs deterministic input replay and safe battery saves from them.
- **How releases are tried.** In RetroArch on an Apple Silicon Mac, installed by script. The local RetroArch tests were removed in Phase 5; the hosted `retroarch-e2e` job carries the screenshot evidence, and `policy.no-skip` fails any CI job whose expected test reports itself skipped.
- **Known debt after v1.** Closed in Phase 5. The audit that listed it is `.planning/milestones/v1-MILESTONE-AUDIT.md`.
- **Known debt after v2.** Listed in `.planning/milestones/v2-MILESTONE-AUDIT.md` and the v2 entry of `.planning/MILESTONES.md`. None of it blocks play; G1, 07 WR-01, 08 WR-01 and the asan timeout were closed after the audit.
- **CI cost.** The asan job skips or shortens the long hash tests (`NESTURBATOR_SHORT_RUNS`), whose full-length hashes the build jobs and hash-equality still check; it runs in about 5 minutes against a 10-minute timeout.
- **Reference material.** Each file below states facts with their sources. Open the one that matches the work before designing or building.
  - `.planning/preparation/README.md` — index of the files below
  - `.planning/preparation/ARCHITECTURE.md` — time model, CPU, PPU, audio, mappers, public API
  - `.planning/preparation/DECISIONS.md` — choices already made, one line each
  - `.planning/preparation/NES-HARDWARE-CPU-APU.md` — CPU, bus, DMA, interrupts, APU, audio synthesis
  - `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md` — PPU, video output, mappers, ROM formats
  - `.planning/preparation/CONFORMANCE.md` — test suites, how each reports, licences, the scoreboard
  - `.planning/preparation/LIBRETRO-AND-RUNNER.md` — libretro requirements, RetroArch on macOS, the runner's command line
  - `.planning/preparation/ENGINEERING.md` — C rules, CMake presets, tests, CI, release
  - `.planning/preparation/NES-ECOSYSTEM.md` — existing emulators, their lessons, what users report

## Constraints

- **Deliverables**: a C library, a headless runner and a libretro adapter only — the host owns windows, audio devices and input devices.
- **Language**: C17 without extensions; the core links only the C memory functions — it has to embed anywhere.
- **Ownership**: all emulation and test code is written here; `libretro.h` is the only vendored file — owned code over dependencies.
- **Determinism**: integer-only core with all state in the instance — the same inputs give identical frame and audio hashes on every platform.
- **Legal**: no ROM or BIOS bytes beyond licensed test ROMs; nothing taken from GPL or LGPL emulators — the repository is public and MIT.
- **Privacy**: no personal paths, addresses or names in tracked files or release artifacts — the repository is public.
- **Verification**: every machine-verifiable behaviour is shown by a command: `cmake --workflow --preset ci` for the main suite, with other presets and hosted workflows covering the rest. Before surfacing a pending UAT item, verify-work audits it for existing command evidence or a recurring regression test; automated items are run and recorded without an owner prompt. Owner input is reserved for consent or irreducible judgment, such as source provenance.
- **Hand-offs**: integration, end-to-end and smoke checks run as commands locally or on CI (on CI when they keep paying off there); identity, consent and irreducible judgments are the only owner hand-offs, so product behavior closes without manual UAT.
- **Delivery**: the owner creates each phase's branch; it merges by pull request into a green `main` and is released automatically — each phase ends in something to download and run.

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Time in half-master-clock ticks; the CPU drives the bus; the PPU catches up at both M2 edges | The coarsest unit where every clock edge is an integer; the shape of designs that score 136 or more of 140 | ✓ Good — Phases 3–4 exercise shared CPU/PPU/APU timing with frame, channel and DMA edge tests |
| Per-dot PPU with literal shift registers, address latch and OAM counters | The hardest public tests probe that state | ✓ Good — Phase 3 render and AccuracyCoin checks pass |
| Integer-only core; `cmake --workflow --preset nofp` shows it | Bit-identical hashes on every platform | ✓ Good — fixed-point synthesis and integer spectral analysis pass the Phase 4 CI gate |
| Video out as palette index plus emphasis; audio out as mono 16-bit from an owned synthesiser | Hashes stay independent of palette; existing audio libraries are LGPL | ✓ Good — Phases 3–4 pin frame/audio output and libretro parity without a device dependency |
| Opaque-instance C API shared with the sibling core | One future host can load every core | ✓ Good — Phase 1's public-header consumer and libretro host exercise the API |
| Integration, end-to-end and smoke checks are automated; owner hand-offs are limited to consent and irreducible judgment | CI evidence closes product behavior; source provenance cannot be machine-proven | ✓ Good — Phase 2 behavior used automated evidence; Phase 01's release credential and non-releasing commit policies now have CTest gates; owner input is limited to clean-room provenance and authorizing the manual schedule substitute, since replaced by passing scheduled runs |
| Tests use committed, licensed samples; full pinned suites are fetched cold and their run-bound inventory and JUnit results are checked separately | Reproducibility, clear rights and evidence that distinguishes passing tests from merely registered tests | ✓ Good — Phase 2 sample, main-push and manual full-vector runs, with run-bound artifacts; scheduled runs passed from 2026-10-07 |
| The public API declaration baseline is a temporary Phase 2 guard; revise or retire it in Phase 3 if the API changes | Protect the shipped boundary during CPU work while preserving planned API evolution | ✓ Good — Phase 3 refreshed the declaration guard for the expanded API |
| One scoreboard file in which a passing test stays passing | A regression is a one-line diff | ✓ Good — Phase 4 requires the six named APU results and checks the baseline for lost passes |
| One entrypoint: CMake workflow presets | No script to keep in step with CI | ✓ Good — `cmake --workflow --preset ci` builds, tests and packages all three deliverables |
| release-please, whose release pull request merges itself when CI passes; unsigned macOS artifacts installed by script | Automatic releases; no legal name in artifacts | ✓ Good — v0.1.0 through v0.1.6 released automatically; `release.one_version_per_line` remains guarded |
| With no cartridge loaded the core outputs a built-in test frame | A build or a release can be checked without any ROM | ✓ Good — Phase 1 checked v0.1.0 in RetroArch with no ROM |
| NROM first, then MMC1, MMC3, UxROM, CNROM, AxROM | Six mappers cover 96% of the North American licensed library | ✓ Good — NROM shipped in Phase 3; remaining mapper families stay in later scope |
| Keep APU channel state and the sample scheduler per instance, clocked from CPU bus cycles | Hardware phase order and isolated emulator state make runs reproducible | ✓ Good — Phase 4 APU, DMC and AccuracyCoin checks pass |
| Use fixed-point transition-to-PCM synthesis with bounded per-instance history | Stable platform output without floating point or a new audio dependency | ✓ Good — Phase 4 spectral, continuity and game-hash checks pass |
| Serialize audio transitions and signed PCM explicitly as little-endian before hashing | Canonical bytes make audio baselines portable across hosts | ✓ Good — Phase 4 known-answer tests and three game baselines pass |
| Allocate instance-owned PRG RAM for accepted trainer-bearing mapper-0 images and copy the trainer before reset-vector setup | The CPU must observe the accepted image's initial state through the normal bus, with no cross-instance state | ✓ Good — Phase 4.1 byte-exact bus, ownership, reload, and rejection regressions pass in CI |
| Copy each selected sprite's four OAM bytes over odd-read/even-write pairs; after eight selections, use the 2C02 diagonal n/m overflow scan | `$2002` overflow timing and false-positive outcomes depend on the documented per-dot OAM sequence | ✓ Good — dot-129/130 boundary, non-Y false positives, skipped-Y cases, pre-render behavior, and cross-platform CI are covered |
| Preserve batch audio as the primary libretro path; use the single-sample callback only when batch audio is unavailable | Batch delivery remains efficient for capable hosts, while the mutually exclusive fallback supports sample-only frontends without dropping PCM or duplicating output | ✓ Good — Phase 4.2 host tests compare sample-only and batch output with direct core PCM |
| Soft reset keeps CPU RAM, battery RAM and mapper registers; `retro_reset()` only calls `nesturbator_reset()` | The console's reset line does not clear memory, and one reset path keeps libretro and direct-API frames equal | ✓ Good — Phase 5 `core.reset` and `libretro.host` frame parity pass |
| No skip allowlist: a test that cannot run is not registered, or a required CI job carries it | A self-skipping test reads as green while proving nothing | ✓ Good — Phase 5 `policy.no-skip` passes on every leg |
| Parallel CTest (jobs 4, COST on long tests), no ccache | Compile share measured at 4 to 22 percent, under the 40 percent rule | ✓ Good — Phase 5 slowest-leg CTest 152 s to 140 s; wall time within runner noise |
| One mapper ops table held by value per instance; bus and PPU read 1 KiB page tables and a four-entry nametable map; CPU writes reach the board stamped with the CPU cycle | No board switch in the bus or PPU, no file-scope data, and every board serialises as registers plus derived pages | ✓ Good — Phase 6 seam landed with every v1 hash byte-identical on six platforms |
| Background and sprite patterns come from the `v`-driven 2C02 fetch pipeline, with A12 as literal PPU-bus state | Mid-frame scroll and MMC3's scanline counter depend on the real fetch dots | ✓ Good — Phase 6 `ppu.fetch` and `ppu.split_scroll` pass; revision 5 re-pinned eight frame hashes; costs about 20 percent frame time |
| Board chosen by one switch that the loader probes before allocating; per-board CHR/PRG geometry profile; bus-conflict AND by NES 2 submapper with submapper 0 defaulting to AND on UxROM and CNROM | An unknown mapper id is refused instead of loading empty, and every board decides conflicts from the header alone | ✓ Good — Phase 7 boards pass 19 accept / 35 reject loader rows and Holy Mapperel M2, M3, M7 |
| PRG RAM is one `[work][NVRAM]` allocation; the battery half is exposed as one span with a write-generation counter; hosts write it only when the generation changed, and the runner writes through temp file, fsync and rename | Raw bytes stay compatible with existing `.sav` files, a host never polls memory, and a crash never leaves a torn save | ✓ Good — Phase 8 `core.save`, `runner.save.*` and the two-session RetroArch round trip pass |
| A `$2000` write raises NMI only on a bit-7 0-to-1 edge while vblank is set; behaviour revision 6 | Re-raising on every write with bit 7 set fired 1086 NMIs in 10 frames | ✓ Good — Phase 10 `core.nmi_edge` and `ppu.registers` pass; exactly 24 hash rows moved |
| Boot-to-play is judged by a pure `judge()` over a recorded trace: one allowlisted RAM byte, scene change and audio variation, with negative controls and synthetic mutants pinning every threshold | A play check that a broken core can pass proves nothing | ✓ Good — Phase 10 positive run passes; controls and mutants return their exact masks |
| The MMC3 A12 filter counts CPU cycles from the A12 fall (a gap of 3 or more clocks), with no extra IRQ delay; the 2C02 shows `v`, not the pattern address, at pre-render dot 0 | blargg scanline timing passes without the conditional one-cycle delay once the pre-render dot-0 address is right | ✓ Good — Phase 9 `ppu.mmc3`, blargg `4-scanline_timing` and `4.Scanline_timing` pass; no frame hash moved |
| Under asan, long hash tests are dropped or run with a short frame budget (`NESTURBATOR_SHORT_RUNS`); full-length hashes stay on the build jobs and hash-equality | Sanitizers find memory errors in the first frames, not the thousandth; repeating full runs under asan took about 15 minutes | ✓ Good — hosted asan 15 min to 4m52s, timeout 10 min, `ci` test list unchanged (PR #39) |
| Unloading a cartridge always runs the power-on helper; loading never does; the libretro adapter keeps one instance for the whole session | One instance means no allocation churn across games, and unload leaves a clean machine without moving any pinned hash | ✓ Good — Phase 11 `core.cartridge` unload tests and libretro parity pass; no hash row moved |
| `nesturbator --version` prints the library's run-time version; the libretro version string stays compile-time | The runner reports the library actually linked, while libretro hosts read a static string | ✓ Good — Phase 11 `runner.version` passes |
| A save state is a 64-byte header (format version, CRC-32 of the state and of the ROM, frame number, ticks) plus every field written by one visitor in SIZE, WRITE, VALIDATE, APPLY and RECORD modes, with a mandatory `state` op per board; VALIDATE checks the whole state before APPLY changes anything | One field list cannot drift between save, load and size; a refused load cannot leave a half-applied machine | ✓ Good — Phase 12 `core.state_fork.*` forks every board and the rejection sweep refuses each out-of-range field; no frame or audio hash moved |
| A loaded state's ticks are capped at 2^62 and its PPU clock must trail the CPU clock by less than one frame | A CRC-valid state with ticks near 2^64 made the next frame loop about 2^61 times (Phase 12 CR-01) | ✓ Good — `core.state_reject` refuses `UINT64_MAX` and 2^62+1 and runs a frame from 2^62 |

The full list with sources is `.planning/preparation/DECISIONS.md`.

## Evolution

This document evolves at phase transitions and milestone boundaries.

**After each phase transition** (via `/gsd-transition`):
1. Requirements invalidated? → Move to Out of Scope with reason
2. Requirements validated? → Move to Validated with phase reference
3. New requirements emerged? → Add to Active
4. Decisions to log? → Add to Key Decisions
5. "What This Is" still accurate? → Update if drifted

**After each milestone** (via `/gsd-complete-milestone`):
1. Full review of all sections
2. Core Value check — still the right priority?
3. Review Out of Scope — reasons still valid?
4. Update Context with current state

---
*Last updated: 2026-10-12 after Phase 12*
