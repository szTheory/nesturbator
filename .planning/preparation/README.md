---
title: "Preparation index"
summary: "Which reference file answers which question, and the conventions the files share."
read_when: "First, before opening any other file in this directory."
updated: 2026-10-02
---

# Preparation index

Reference material gathered before the first line of code. Each file states facts with their sources and gives one recommendation per choice. Open the file a question points to; none needs reading end to end.

## Files

| File | Holds | Read when |
|---|---|---|
| [ARCHITECTURE.md](ARCHITECTURE.md) | The core's shape: time in half-master-clock ticks, the CPU driving the bus, the per-dot PPU, integer audio, page-table mappers, the public C API | Before designing a core subsystem, the public header, the libretro adapter or the runner |
| [DECISIONS.md](DECISIONS.md) | One line per choice already made, with its reason and the section that supports it | Before reopening a choice, or when a phase needs to know what is settled |
| [NES-HARDWARE-CPU-APU.md](NES-HARDWARE-CPU-APU.md) | Where CPU, bus, DMA, interrupt and APU behaviour is documented, the hard cases, and the public test for each | Designing or building the CPU, bus timing, DMA, interrupts, the APU or the audio synthesiser |
| [NES-HARDWARE-PPU-CARTRIDGE.md](NES-HARDWARE-PPU-CARTRIDGE.md) | The same for the PPU, video output, mappers and ROM file formats | Designing or building the PPU, video output, the cartridge and mapper layer, or the ROM loader |
| [CONFORMANCE.md](CONFORMANCE.md) | Public test suites and reference emulators: what each shows, how it reports without a display, its licence and pin; the scoreboard file | Choosing test ROMs; writing the runner's test modes, the scoreboard or the CI tiers |
| [LIBRETRO-AND-RUNNER.md](LIBRETRO-AND-RUNNER.md) | What libretro requires of a core, how a core reaches RetroArch on an Apple Silicon Mac, and the runner's command line | Building the libretro adapter, packaging a release, or designing the runner |
| [ENGINEERING.md](ENGINEERING.md) | C rules, CMake presets, tests, fuzzing, CI jobs, release configuration, repository hygiene | Creating or changing the build, tests, CI, repository settings or the release pipeline |
| [NES-ECOSYSTEM.md](NES-ECOSYSTEM.md) | Existing emulators: licence, score, timing model and speed; what users report; the slot this project fills | Choosing a timing design, ranking features, or checking what another emulator already does |
| [GSD-HANDOFF.md](GSD-HANDOFF.md) | The kickoff text, the settings that make GSD stop after each step, the per-phase steps, and the owner's tasks | Before starting the GSD project, before each phase, and when a GSD step behaves unexpectedly |

## Where to look

| Question | File and section |
|---|---|
| How is time counted, and where does the PPU catch up inside a CPU cycle? | ARCHITECTURE 2; NES-HARDWARE-CPU-APU 2.1 |
| How do OAM DMA and DMC DMA behave? | NES-HARDWARE-CPU-APU 2.2 |
| When are NMI and IRQ polled? | NES-HARDWARE-CPU-APU 2.3 |
| Which public test shows that a behaviour is right? | Section 3 of each hardware file |
| How does the APU work, and how are samples synthesised? | NES-HARDWARE-CPU-APU 4 and 5 |
| What state does the PPU model need? | NES-HARDWARE-PPU-CARTRIDGE 2 |
| What does the core hand to a host as video and audio? | NES-HARDWARE-PPU-CARTRIDGE 4; ARCHITECTURE 4 and 5 |
| Which mappers come next, and what share of the library does each add? | NES-HARDWARE-PPU-CARTRIDGE 5 |
| How is a ROM file recognised, and rejected safely? | NES-HARDWARE-PPU-CARTRIDGE 6 |
| Can a header database be bundled? | NES-HARDWARE-PPU-CARTRIDGE 7 |
| What are the public API's principles? | ARCHITECTURE 7 |
| What must a libretro core export, and what goes in its `.info` file? | LIBRETRO-AND-RUNNER 1 and 2 |
| How does a release reach RetroArch on a Mac? | LIBRETRO-AND-RUNNER 3 and 6 |
| What are the runner's options and exit statuses? | LIBRETRO-AND-RUNNER 5 |
| How does AccuracyCoin report its results? | CONFORMANCE, "AccuracyCoin in detail" |
| How are the 65x02 vectors sampled and stored? | CONFORMANCE, "The 65x02 vectors" |
| Which test ROMs may be committed? | CONFORMANCE, "ROMs that can be committed or released" |
| How does the scoreboard file work? | CONFORMANCE, "Scoreboard design" |
| Which compiler flags, presets and build checks are used? | ENGINEERING 1 and 2 |
| What runs in CI, and on which runners? | ENGINEERING 5 |
| How is a release made and published? | ENGINEERING 6 |
| How do other emulators model timing, and how fast are they? | NES-ECOSYSTEM 1 and 2 |
| What do users report against existing cores? | NES-ECOSYSTEM 5 |
| What is already decided? | DECISIONS |
| How is GSD run for this project? | GSD-HANDOFF |

## Conventions

- **Frontmatter.** Every file starts with `title`, `summary`, `read_when` and `updated`.
- **Citations.** `[HWC.07]` resolves in the Sources table at the end of the same file. Prefixes: ARCH, DEC, HWC (CPU and APU), HWP (PPU and cartridge), CF (conformance), LR (libretro and runner), ENG, ECO, GH (GSD hand-off).
- **Source tiers.** T1: primary material, such as a project's own repository or official documentation, a circuit-level analysis, a paper, or a measurement made here. T2: a curated community reference, such as the NESdev Wiki. T3: one implementation's behaviour. T4: forum posts and issue trackers. A tier describes the source for the claim it supports, so an emulator's repository is T1 for what that emulator does and T3 for what the hardware does.
- **Pins.** A source row carries a commit, a release tag or a wiki revision; where none exists it carries the access date. Sibling repositories are cited by project name and commit; two are not public and are described instead of named.
- **Markers.** "(tested)" was run on the project's Mac; "(untested)" was not run; "(derived)" is inferred from cited facts; "(unverified)" comes from recall and was not fetched.
- **Choices.** Each gives one recommendation, the strongest alternative, and the observation that would change it.
- **IDs** use a dot (`DEC.07`), so they never collide with GSD's own `D-NN` phase decisions.

## Keeping it current

When a phase finds one of these files wrong or out of date, the same pull request corrects the file and its `updated` date. Research for a single phase lives with that phase under `.planning/phases/`.
