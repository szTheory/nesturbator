# nesturbator

## What This Is

nesturbator is a NES emulator core written in C. It ships as a library other programs embed, a headless command-line runner for automation, and a libretro adapter so that hosts such as RetroArch can play games with it. It is for people who build emulator hosts and want an accurate, permissively licensed NES core, and for the players who use those hosts.

## Core Value

Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.

## Requirements

### Validated

- ✓ One command installs the core into RetroArch on an Apple Silicon Mac, and a test shows RetroArch displays the runner's frame. — Phase 1 (v0.1.0)
- ✓ `cmake --workflow --preset ci` (`ci-msvc` on Windows) builds and tests everything on Linux, macOS and Windows, x64 and arm64, and CI runs the same commands. — Phase 1
- ✓ Every behaviour-changing merge to `main` produces a release with the library, the runner and the libretro core. — Phase 1 (v0.1.0 published by the release App, no person acting)

### Active

- [ ] The CPU matches the public 65x02 vectors on every opcode and every bus cycle.
- [ ] NROM games render, take controller input and play with sound in RetroArch.
- [ ] Frame and audio hashes are identical on every supported platform.

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
- **Verification**: every behaviour is shown by a command: `cmake --workflow --preset ci` for the main suite, the other presets and the nightly jobs for the rest — nothing waits on a person.
- **Hand-offs**: integration, end-to-end and smoke checks run as commands locally or on CI (on CI when they keep paying off there); the owner is asked only for what needs their identity or consent, such as a browser click on GitHub or publishing — and everything around that step is scripted — so phases close with no manual testing.
- **Delivery**: the owner creates each phase's branch; it merges by pull request into a green `main` and is released automatically — each phase ends in something to download and run.

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Time in half-master-clock ticks; the CPU drives the bus; the PPU catches up at both M2 edges | The coarsest unit where every clock edge is an integer; the shape of designs that score 136 or more of 140 | — Pending |
| Per-dot PPU with literal shift registers, address latch and OAM counters | The hardest public tests probe that state | — Pending |
| Integer-only core; `cmake --workflow --preset nofp` shows it | Bit-identical hashes on every platform | — Pending |
| Video out as palette index plus emphasis; audio out as mono 16-bit from an owned synthesiser | Hashes stay independent of palette; existing audio libraries are LGPL | — Pending |
| Opaque-instance C API shared with the sibling core | One future host can load every core | — Pending |
| Integration, end-to-end and smoke checks are commands; owner hand-offs shrink to consent clicks | Phases close with no manual testing (Hand-offs constraint) | ✓ Good — Phase 1 UAT ran entirely by command after the merge |
| Tests run on committed, licensed files: a 65x02 sample, AccuracyCoin, Holy Mapperel, open-licence games | Reproducible without a network; clear rights | — Pending |
| One scoreboard file in which a passing test stays passing | A regression is a one-line diff | — Pending |
| One entrypoint: CMake workflow presets | No script to keep in step with CI | — Pending |
| release-please, whose release pull request merges itself when CI passes; unsigned macOS artifacts installed by script | Automatic releases; no legal name in artifacts | ✓ Good — v0.1.0 released itself; a version must appear once per README line (`release.one_version_per_line`) |
| With no cartridge loaded the core outputs a built-in test frame | A build or a release can be checked without any ROM | ✓ Good — Phase 1 checked v0.1.0 in RetroArch with no ROM |
| NROM first, then MMC1, MMC3, UxROM, CNROM, AxROM | Six mappers cover 96% of the North American licensed library | — Pending |

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
*Last updated: 2026-10-03 after Phase 1*
