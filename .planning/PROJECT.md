# nesturbator

## What This Is

nesturbator is a NES emulator core written in C. It ships as a library other programs embed, a headless command-line runner for automation, and a libretro adapter so that hosts such as RetroArch can play games with it. It is for people who build emulator hosts and want an accurate, permissively licensed NES core, and for the players who use those hosts.

## Core Value

Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.

## Current State

v1 "NROM games with sound" shipped on 2026-10-09 (final release v0.1.6). NROM games render, take input from two controller ports and play with sound in RetroArch. Frame and audio hashes match on Linux, macOS and Windows on x64 and arm64. The code is 13,642 lines of C, 5,964 of them in the core library. The milestone record is in `.planning/MILESTONES.md`, and the roadmap, requirements, audit and phase history are in `.planning/milestones/`.

## Current Milestone: v2 Most of the library plays

**Goal:** Games on the six common board families play in RetroArch with battery saves, with frame and audio hashes identical on every supported platform.

**Target features:**
- A tune-up phase (SEED-006): faster CI, no flaky test, current pins, docs that match the release, and the v1 debt closed — an exact release-policy gate check, a trainer-bearing image through the full host path, a working `retro_reset()`, and local RetroArch tests that run or are retired instead of self-skipping.
- MMC1 games run, and battery saves persist across runs as raw `.sav` bytes through the runner and libretro (SEED-001).
- MMC3 games run with the scanline IRQ counter (SEED-001).
- UxROM, CNROM and AxROM games run, with bus conflicts where the board has them (SEED-001).
- Holy Mapperel and per-board frame hashes are pinned in CI.
- One supported game is proven from boot through interactive play: a defined game-state change, a visible response to input, and non-silent audio (SEED-261009-zs2).

Later milestones remain seeds in `.planning/seeds/`: save states and the runner contract (SEED-002), accuracy and PAL/Dendy (SEED-003), robustness and speed (SEED-004), Famicom hardware (SEED-005).

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

### Active

- [ ] MMC1 and MMC3 games run, shown by Holy Mapperel, mapper test ROMs and frame hashes.
- [ ] Battery saves persist across runs as raw bytes compatible with existing `.sav` files.
- [ ] One supported game is proven from boot through interactive play by an automated check.

### Out of Scope

- A GUI, or any window, audio-device or input-device code — a shared host application will provide these for every core; RetroArch does until then.
- Shaders, menus, input mapping and netplay lobbies — these belong to the host.
- Bundled ROMs, BIOS files or a ROM database — players supply their own games, and no such database publishes a licence.
- Code from GPL or LGPL emulators — the project is MIT and written from hardware documentation.
- A separate fast or low-accuracy mode — one accurate core keeps one set of bugs and one set of hashes.
- Signed or notarized macOS artifacts — the script install path does not need them.
- A test framework dependency — plain CTest executables with an in-repo `check.h` cover the need.
- PAL and Dendy timing, mappers beyond the six common families, save states, expansion audio, FDS, extra peripherals — planned for later milestones; see `.planning/seeds/`.
- Submitting the core to the libretro buildbot — an outward-facing step the owner decides on separately.

## Context

- **Ecosystem.** Every mainstream libretro NES core is GPL or LGPL. The most accurate emulators are GPL applications or permissive C# and Java programs. A permissive, accuracy-class, embeddable C core does not exist yet. AccuracyCoin, an MIT test ROM with 144 tests, is today's public measure of accuracy.
- **Sibling projects.** A Neo Geo core with the same three deliverables and the same API conventions. Playstead, a host that launches emulator processes and needs deterministic input replay and safe battery saves from them.
- **How releases are tried.** In RetroArch on an Apple Silicon Mac, installed by script. The local RetroArch tests were removed in Phase 5; the hosted `retroarch-e2e` job carries the screenshot evidence, and `policy.no-skip` fails any CI job whose expected test reports itself skipped.
- **Known debt after v1.** Closed in Phase 5. The audit that listed it is `.planning/milestones/v1-MILESTONE-AUDIT.md`.
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
*Last updated: 2026-10-10 after Phase 7*
