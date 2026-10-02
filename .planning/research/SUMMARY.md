# Project Research Summary

**Project:** nesturbator
**Domain:** NES emulator core in C (embeddable library, headless runner, libretro adapter)
**Researched:** 2026-10-02
**Confidence:** HIGH

## Executive Summary

nesturbator is an emulator core, not an application. Accuracy-class NES emulators are built around one idea: the CPU drives a shared timeline and the picture chip is stepped dot by dot, so every access a game makes lands at the right moment. The designs that score 136 or more of 140 on AccuracyCoin, today's public accuracy test, advance the picture chip inside each CPU cycle and keep its shift registers and latches as literal state. This project uses that model, in integer arithmetic only, so its output hashes are the same on every platform.

The approach is to own all the code, test against public suites with explicit licences, and release from the first phase, so each phase's result can be downloaded and run.

## Key Findings

### Recommended Stack

C17 without extensions, CMake workflow presets, Ninja and CTest, with no runtime dependencies. See `.planning/research/STACK.md`.

### Architecture Approach

One timeline counted in half-master-clock ticks. A CPU cycle is one bus call; inside it the picture chip catches up at both clock edges. DMA lives in the read path. Audio is synthesised in integers by an owned band-limited synthesiser. Mappers are small modules over 1 KiB page tables. The public API is an opaque instance with caller-owned buffers. Details: `.planning/preparation/ARCHITECTURE.md`.

**Major components:**
1. Core library — CPU, bus, PPU, APU, synthesiser, cartridge and mappers, state
2. Runner — replays input, prints hashes, runs test ROMs
3. libretro adapter — converts the core's native output for RetroArch

## Implications for Roadmap

Four phases. Merging each one publishes a release.

### Phase 1: A test frame in RetroArch
**Rationale:** The first release already runs in RetroArch, so every later phase ships by merging.
**Delivers:** The library, the runner and the libretro core, showing a built-in test frame; the build and test presets; the public GitHub repository with CI on six platforms; automatic releases; the one-line RetroArch install.
**Addresses:** FRAME-01 to FRAME-07
**UI hint**: no
**Canonical refs:** `.planning/preparation/ENGINEERING.md`, `.planning/preparation/LIBRETRO-AND-RUNNER.md`, `.planning/preparation/ARCHITECTURE.md`

### Phase 2: The CPU matches the public vectors
**Rationale:** Public vectors judge the 6502 on its own, before any other chip exists.
**Delivers:** An instruction-level 6502 with one bus call per cycle and all 256 opcodes, matching the committed vector sample on every pull request and the full set nightly.
**Addresses:** CPU-01, CPU-02
**UI hint**: no
**Canonical refs:** `.planning/preparation/NES-HARDWARE-CPU-APU.md`, `.planning/preparation/ARCHITECTURE.md`, `.planning/preparation/CONFORMANCE.md`, `.planning/preparation/ENGINEERING.md`

### Phase 3: A real game in RetroArch
**Rationale:** The picture chip, the ROM loader and controller input together make the first playable result, and AccuracyCoin starts measuring accuracy.
**Delivers:** NROM games that render, with sprites through OAM DMA, and take controller input; a loader that rejects hostile files; the committed AccuracyCoin scoreboard; frame hashes equal on every platform.
**Addresses:** GAME-01 to GAME-06
**Uses:** the per-dot PPU model and 1 KiB page tables from `ARCHITECTURE.md`
**UI hint**: no
**Canonical refs:** `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md`, `.planning/preparation/NES-HARDWARE-CPU-APU.md`, `.planning/preparation/ARCHITECTURE.md`, `.planning/preparation/CONFORMANCE.md`, `.planning/preparation/LIBRETRO-AND-RUNNER.md`

### Phase 4: Sound
**Rationale:** The APU and DMC DMA depend on exact bus timing from the earlier phases, and sound completes a playable game.
**Delivers:** NROM games with audio in RetroArch; integer band-limited synthesis; audio hashes equal on every platform; AccuracyCoin's length counter, frame counter and DMC tests passing.
**Addresses:** SND-01 to SND-04
**Uses:** the DMA-in-the-read-path model and the synthesiser design from `NES-HARDWARE-CPU-APU.md`
**UI hint**: no
**Canonical refs:** `.planning/preparation/NES-HARDWARE-CPU-APU.md`, `.planning/preparation/ARCHITECTURE.md`, `.planning/preparation/CONFORMANCE.md`

## Sources

The files under `.planning/preparation/`, each with its own Sources table of pinned commits and wiki revisions. `.planning/preparation/README.md` is the index.

---
*Research completed: 2026-10-02*
*Ready for roadmap: yes*
