# nesturbator

## What This Is

nesturbator is a NES emulator core written in C. It ships as a library other programs embed, a headless command-line runner for automation, and a libretro adapter so that hosts such as RetroArch can play games with it. It is for people who build emulator hosts and want an accurate, permissively licensed NES core, and for the players who use those hosts.

## Core Value

Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.

## Requirements

### Validated

- ✓ One command installs the core into RetroArch on an Apple Silicon Mac, and a test shows RetroArch displays the runner's frame. — Phase 1 (v0.1.0)
- ✓ `cmake --workflow --preset ci` (`ci-msvc` on Windows) builds and tests everything on Linux, macOS and Windows, x64 and arm64, and CI runs the same commands. — Phase 1
- ✓ Every behaviour-changing merge to `main` produces a release with the library, the runner and the libretro core. — Phase 1 (v0.1.0) and Phase 2 (v0.1.1), published automatically
- ✓ The CPU matches the public 65x02 vectors on all 256 opcodes and every bus cycle. — Phase 2 (62/62 verification truths passed)
- ✓ The pinned full 65x02 vector set passes, with CI scheduled to run it nightly. — Phase 2 (main-push and owner-authorized manual runs passed; the first cron event was not observed)
- ✓ NROM games render, take controller input and play with sound in RetroArch. — Phases 3–4 (frame, controller, APU, and libretro callback behavior have automated coverage)
- ✓ Frame and audio hashes are identical on every supported platform. — Phases 3–4 (pinned outputs run in the six-platform CI matrix)

### Active
None for v1; later work remains tracked in `.planning/seeds/`.

### Out of Scope

- A GUI, or any window, audio-device or input-device code — a shared host application will provide these for every core; RetroArch does until then.
- Shaders, menus, input mapping and netplay lobbies — these belong to the host.
- Bundled ROMs, BIOS files or a ROM database — players supply their own games, and no such database publishes a licence.
- Code from GPL or LGPL emulators — the project is MIT and written from hardware documentation.
- A separate fast or low-accuracy mode — one accurate core keeps one set of bugs and one set of hashes.
- Signed or notarized macOS artifacts — the script install path does not need them.
- A test framework dependency — plain CTest executables with an in-repo `check.h` cover the need.
- PAL and Dendy timing, mappers beyond NROM, save states, expansion audio, FDS, extra peripherals — planned for later milestones; see `.planning/seeds/`.

## Context

- **Ecosystem.** Every mainstream libretro NES core is GPL or LGPL. The most accurate emulators are GPL applications or permissive C# and Java programs. A permissive, accuracy-class, embeddable C core does not exist yet. AccuracyCoin, an MIT test ROM with 144 tests, is today's public measure of accuracy.
- **Sibling projects.** A Neo Geo core with the same three deliverables and the same API conventions. Playstead, a host that launches emulator processes and needs deterministic input replay and safe battery saves from them.
- **How releases are tried.** In RetroArch on an Apple Silicon Mac, installed by script.
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
- **Verification**: every machine-verifiable behaviour is shown by a command: `cmake --workflow --preset ci` for the main suite, with other presets and hosted workflows covering the rest. Owner input is reserved for consent or irreducible judgment, such as source provenance.
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
| Integration, end-to-end and smoke checks are automated; owner hand-offs are limited to consent and irreducible judgment | CI evidence closes product behavior; source provenance cannot be machine-proven | ✓ Good — Phase 2 behavior used automated evidence; owner input was limited to clean-room provenance and authorizing the manual schedule substitute |
| Tests use committed, licensed samples; full pinned suites are fetched cold and their run-bound inventory and JUnit results are checked separately | Reproducibility, clear rights and evidence that distinguishes passing tests from merely registered tests | ✓ Good — Phase 2 sample, main-push and manual full-vector runs, with run-bound artifacts; the cron trigger remains policy-checked |
| The public API declaration baseline is a temporary Phase 2 guard; revise or retire it in Phase 3 if the API changes | Protect the shipped boundary during CPU work while preserving planned API evolution | ✓ Good — Phase 3 refreshed the declaration guard for the expanded API |
| One scoreboard file in which a passing test stays passing | A regression is a one-line diff | ✓ Good — Phase 4 requires the six named APU results and checks the baseline for lost passes |
| One entrypoint: CMake workflow presets | No script to keep in step with CI | ✓ Good — `cmake --workflow --preset ci` builds, tests and packages all three deliverables |
| release-please, whose release pull request merges itself when CI passes; unsigned macOS artifacts installed by script | Automatic releases; no legal name in artifacts | ✓ Good — v0.1.0 and v0.1.1 released automatically; `release.one_version_per_line` remains guarded |
| With no cartridge loaded the core outputs a built-in test frame | A build or a release can be checked without any ROM | ✓ Good — Phase 1 checked v0.1.0 in RetroArch with no ROM |
| NROM first, then MMC1, MMC3, UxROM, CNROM, AxROM | Six mappers cover 96% of the North American licensed library | ✓ Good — NROM shipped in Phase 3; remaining mapper families stay in later scope |
| Keep APU channel state and the sample scheduler per instance, clocked from CPU bus cycles | Hardware phase order and isolated emulator state make runs reproducible | ✓ Good — Phase 4 APU, DMC and AccuracyCoin checks pass |
| Use fixed-point transition-to-PCM synthesis with bounded per-instance history | Stable platform output without floating point or a new audio dependency | ✓ Good — Phase 4 spectral, continuity and game-hash checks pass |
| Serialize audio transitions and signed PCM explicitly as little-endian before hashing | Canonical bytes make audio baselines portable across hosts | ✓ Good — Phase 4 known-answer tests and three game baselines pass |

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
*Last updated: 2026-10-09 after Phase 4*
