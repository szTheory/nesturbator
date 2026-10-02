---
title: "Decisions"
summary: "One line per decision made during preparation, with its reason and the file and section that supports it."
read_when: "Before reopening a choice, or when a phase needs to know what is already settled."
updated: 2026-10-02
---

# Decisions

## Why this file exists
Each row is a choice already made, so a phase can build on it without re-deriving it. The supporting file gives the strongest alternative and the observation that would change the choice. IDs use a dot (`DEC.07`) so they never collide with GSD's own `D-NN` phase decisions.

## Product

| ID | Decision | Why | Supported by |
|---|---|---|---|
| DEC.01 | Three deliverables: C library, headless runner, libretro adapter. No GUI or host application. | The owner builds cores for several systems and one shared host later; RetroArch serves until then. | [ARCHITECTURE.md](ARCHITECTURE.md) 1; [LIBRETRO-AND-RUNNER.md](LIBRETRO-AND-RUNNER.md) 7 |
| DEC.02 | MIT licence; holder "nesturbator contributors". | Any frontend can embed it; matches the sibling core. | [NES-ECOSYSTEM.md](NES-ECOSYSTEM.md) 6 |
| DEC.03 | All emulation and test code is written here; `libretro.h` is the only vendored file. | The owner prefers owned code to dependencies; a small flat tree embeds anywhere. | [ENGINEERING.md](ENGINEERING.md) 3 |
| DEC.04 | Clean room: no GPL or LGPL emulator source is opened or kept in the workspace; reference emulators run as released binaries. | An AI-assisted MIT emulator was found to contain ported GPL code in 2026 and had to relicense. | [NES-ECOSYSTEM.md](NES-ECOSYSTEM.md) 3; [ENGINEERING.md](ENGINEERING.md) 7 |
| DEC.05 | NTSC NES first, modelled on the RP2A03G and RP2C02G. PAL, Dendy and Famicom differences come later. | AccuracyCoin targets that pair; one profile keeps the first milestones small. | [NES-HARDWARE-CPU-APU.md](NES-HARDWARE-CPU-APU.md) 6; [NES-HARDWARE-PPU-CARTRIDGE.md](NES-HARDWARE-PPU-CARTRIDGE.md) 8 |
| DEC.06 | No bundled header database; the host supplies header corrections. | No header database publishes a licence. | [NES-HARDWARE-PPU-CARTRIDGE.md](NES-HARDWARE-PPU-CARTRIDGE.md) 7 |

## Architecture

| ID | Decision | Why | Supported by |
|---|---|---|---|
| DEC.07 | One accurate core; no separate fast mode. | Two cores mean two sets of bugs and hashes. | [ARCHITECTURE.md](ARCHITECTURE.md) 1 |
| DEC.08 | Time is counted in half-master-clock ticks; the CPU drives the bus; the PPU catches up at M2 rise and M2 fall of every CPU cycle. | The coarsest unit in which every documented clock edge is an integer; the shape of the designs that score 136 or more. | [ARCHITECTURE.md](ARCHITECTURE.md) 2; [NES-HARDWARE-CPU-APU.md](NES-HARDWARE-CPU-APU.md) 2.1 |
| DEC.09 | Instruction-level CPU with one bus call per cycle; DMA lives inside the read call. | Matches hardware, where the halted CPU repeats its read; no source needs a mid-instruction stop. | [NES-HARDWARE-CPU-APU.md](NES-HARDWARE-CPU-APU.md) 2.2, 2.4 |
| DEC.10 | Per-dot PPU with two half-dot phases; fetches are real bus accesses; shifters, address latch and OAM counters are literal state. | The hardest public tests probe that state; a batch renderer would need rewriting. | [NES-HARDWARE-PPU-CARTRIDGE.md](NES-HARDWARE-PPU-CARTRIDGE.md) 2 |
| DEC.11 | Eager PPU catch-up first; a lazy schedule only if frame hashes stay identical. | Correct by construction first; speed work is then checked by hash equality. | [ARCHITECTURE.md](ARCHITECTURE.md) 2 |
| DEC.12 | Integer-only core, checked three ways: a source scan, a no-floating-point build preset, and an undefined-symbol allowlist. | Bit-identical hashes across platforms; the compiler flag alone lets soft-float calls through. | [ENGINEERING.md](ENGINEERING.md) 1 |
| DEC.13 | Video out is 16 bits per pixel (palette entry plus emphasis) with frame metadata; colour conversion happens outside the core. | No palette choice enters a hash; no single palette suits every game. | [NES-HARDWARE-PPU-CARTRIDGE.md](NES-HARDWARE-PPU-CARTRIDGE.md) 4 |
| DEC.14 | Audio out is mono signed 16-bit from an owned band-limited synthesiser; the pre-synthesis transition stream is hashed as well as the samples. | Existing libraries are LGPL; the first hash changes only when APU logic changes. | [NES-HARDWARE-CPU-APU.md](NES-HARDWARE-CPU-APU.md) 5; [ARCHITECTURE.md](ARCHITECTURE.md) 5 |
| DEC.15 | 1 KiB page tables on both buses; one module per mapper family; a mapper declares the events it watches. | Covers the smallest bank sizes in use; boards that watch nothing cost nothing. | [NES-HARDWARE-PPU-CARTRIDGE.md](NES-HARDWARE-PPU-CARTRIDGE.md) 2 |
| DEC.16 | Opaque-instance C API; allocation only at create and load; ABI, state format, behaviour revision and battery-RAM layout versioned separately. | Many instances, any host, same conventions as the sibling core. | [ARCHITECTURE.md](ARCHITECTURE.md) 7 |
| DEC.17 | Save state is fixed-size per cartridge, canonical, independent of options, and validated before it is applied. | libretro forbids a growing state; option-dependent or version-fragile states are the most heated user complaint. | [LIBRETRO-AND-RUNNER.md](LIBRETRO-AND-RUNNER.md) 2; [NES-ECOSYSTEM.md](NES-ECOSYSTEM.md) 4, 5 |
| DEC.18 | A machine profile holds power-on alignment, APU phase, RAM fill, chip revision and the constant used by the two unstable opcodes. | Hardware varies in these; the public vectors fit one constant, and an open report says test ROMs need another. | [NES-HARDWARE-CPU-APU.md](NES-HARDWARE-CPU-APU.md) 7, 8 |
| DEC.19 | Mapper order: NROM, then MMC1, MMC3, UxROM, CNROM, AxROM. | Six mappers cover 96.2% of North American licensed dumps; AccuracyCoin runs on NROM. | [NES-HARDWARE-PPU-CARTRIDGE.md](NES-HARDWARE-PPU-CARTRIDGE.md) 5 |
| DEC.38 | With no cartridge loaded the core outputs a built-in test frame. | A build or a release can be checked without any ROM, in CI and by a host. | [ARCHITECTURE.md](ARCHITECTURE.md) 7 |

## Tests

| ID | Decision | Why | Supported by |
|---|---|---|---|
| DEC.20 | Pull-request tests use committed, licensed files only: AccuracyCoin, a 65x02 vector sample (first 100 tests per opcode, binary), Holy Mapperel, open-licence games. | Reproducible without a network; every file has an explicit licence. | [CONFORMANCE.md](CONFORMANCE.md) |
| DEC.21 | Classic suites without a licence are fetched at a pinned commit against a checksum list in the nightly run, never committed. | Their authors state no licence. | [CONFORMANCE.md](CONFORMANCE.md) |
| DEC.22 | One committed, sorted scoreboard file keyed by test name; a passing test stays passing; a pin bump is its own change. | Regressions show as one-line diffs; upstream tests are added and renamed often. | [CONFORMANCE.md](CONFORMANCE.md) |
| DEC.39 | Milestone 1 asks AccuracyCoin for pages 2 and 17 and six named APU tests; every other test is held by the scoreboard until the accuracy milestone. | Those tests cover basic CPU addressing, PPU registers and what a player hears; a pass-all bar this early could stall a phase. | [CONFORMANCE.md](CONFORMANCE.md), AccuracyCoin in detail |
| DEC.23 | MesenCE 2.2.1 is the routine reference emulator, run as a pinned binary. A difference locates a divergence; it does not say which side is right. | Its predecessor documents a headless test runner; TriCNES has no command line. | [CONFORMANCE.md](CONFORMANCE.md) |
| DEC.24 | Tests are plain C executables under CTest with an in-repo `check.h`. | CTest already gives selection, parallelism, timeouts and reports. | [ENGINEERING.md](ENGINEERING.md) 3 |
| DEC.25 | Fuzz targets use libFuzzer on Linux in the nightly run; every build replays the committed corpus. | Apple's clang ships no libFuzzer runtime. | [ENGINEERING.md](ENGINEERING.md) 4 |
| DEC.26 | Speed is compared by instruction count under Cachegrind on Linux, head against merge-base. | Wall-clock time on hosted runners is too noisy; Valgrind does not run on the owner's Mac. | [ENGINEERING.md](ENGINEERING.md) 5 |

## Build, CI and release

| ID | Decision | Why | Supported by |
|---|---|---|---|
| DEC.27 | C17 with extensions off; CMake 3.25 or newer, Ninja, CTest. | Works on GCC, Clang and MSVC; 3.25 is the first release with workflow presets. | [ENGINEERING.md](ENGINEERING.md) 1, 2 |
| DEC.28 | One entrypoint, locally and in CI: `cmake --workflow --preset <lane>`, with lanes `dev`, `ci`, `ci-msvc`, `asan`, `nofp`, `hygiene`. | No script to keep in step with presets and the CI matrix. | [ENGINEERING.md](ENGINEERING.md) 2 |
| DEC.29 | Six runners per pull request, one `CI required` roll-up check, no trigger-level path filter, every action pinned by full SHA. | Cross-platform identity is a product claim; a skipped or filtered check can otherwise pass or hang. | [ENGINEERING.md](ENGINEERING.md) 5 |
| DEC.30 | release-please with the `simple` type; a draft release receives the archives, checksums and attestation, then is published. | Releases are immutable once published. | [ENGINEERING.md](ENGINEERING.md) 6 |
| DEC.31 | The release pull request is opened with a GitHub App token; a fine-grained personal access token is the proven alternative. | The default token's pull requests wait for approval instead of running CI. | [ENGINEERING.md](ENGINEERING.md) 6 |
| DEC.32 | macOS artifacts carry only the linker's ad-hoc signature and are installed by script. | Files fetched with `curl` or `gh` carry no quarantine flag; a Developer ID would put a legal name in every artifact. | [LIBRETRO-AND-RUNNER.md](LIBRETRO-AND-RUNNER.md) 3, 6 |
| DEC.33 | Names: target and package `nesturbator`, runner `nesturbator-run`, core `nesturbator_libretro` with its `.info` file; release zips hold `cores/`, `info/` and `LICENSE`. | libretro and sibling-core conventions. | [LIBRETRO-AND-RUNNER.md](LIBRETRO-AND-RUNNER.md) 6; [ENGINEERING.md](ENGINEERING.md) 2 |

## Working method

| ID | Decision | Why | Supported by |
|---|---|---|---|
| DEC.34 | opengsd runs in interactive mode, one step per command, with chaining off. | The owner inspects each step and picks the model for the next. | [GSD-HANDOFF.md](GSD-HANDOFF.md) |
| DEC.35 | Each phase is one branch, squash-merged by pull request with a Conventional Commit title, automatically when CI passes. | release-please reads the squash title; `main` stays green. | [ENGINEERING.md](ENGINEERING.md) 6; [GSD-HANDOFF.md](GSD-HANDOFF.md) |
| DEC.36 | `scripts/hygiene.sh` runs in git hooks and in CI. | A CI check alone runs after the push, which is too late for a public repository. | [ENGINEERING.md](ENGINEERING.md) 7 |
| DEC.37 | Milestone 1 is four phases: a test frame in RetroArch, the CPU against the public vectors, a real game, sound. Later work waits in seeds. | Phases are built from requirement categories; delivery is proven before emulation, and each phase ends in a release. | [GSD-HANDOFF.md](GSD-HANDOFF.md) |

## Open questions
- Release token: GitHub App or personal access token. The owner chooses in the first phase.
- Whether GPL-licensed test ROMs are fetched in the nightly run or left out. They are never committed.
- Whether to submit results to the public AccuracyCoin leaderboard, and when.

## Sources
| ID | Tier | Source | Pin or access date | Supports |
|---|---|---|---|---|
| DEC.S1 | T2 | The files linked in each row, with their own Sources tables | 2026-10-02 | Every row |
| DEC.S2 | T1 | Owner direction given during preparation | 2026-10-02 | DEC.01, DEC.02, DEC.03, DEC.34 |
