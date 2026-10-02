# Roadmap: nesturbator

## Overview

Milestone 1 takes nesturbator from an empty repository to NROM games that render, take controller input and play with sound in RetroArch, with frame and audio hashes identical on six platforms. The first phase ships the library, the runner and the libretro core showing a built-in test frame, together with the build presets, CI and automatic releases, so every later phase ships by merging. The CPU is then proven alone against the public 65x02 vectors, the picture chip and ROM loader make the first playable game, and the APU completes it with sound. Merging each phase's pull request publishes a release.

## Phases

**Phase Numbering:**
- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

Decimal phases appear between their surrounding integers in numeric order.

- [ ] **Phase 1: A test frame in RetroArch** - The library, runner and libretro core build, test and release on six platforms and show a built-in test frame in RetroArch
- [ ] **Phase 2: The CPU matches the public vectors** - Every opcode matches the public 65x02 vectors on final state and on every bus cycle
- [ ] **Phase 3: A real game in RetroArch** - NROM games render and take controller input, with frame hashes equal on every platform
- [ ] **Phase 4: Sound** - NROM games play with sound, with audio hashes equal on every platform

## Phase Details

### Phase 1: A test frame in RetroArch
**Goal**: Anyone can build, test, download and install the library, the runner and the libretro core, and RetroArch shows the core's built-in test frame.
**Depends on**: Nothing (first phase)
**Requirements**: FRAME-01, FRAME-02, FRAME-03, FRAME-04, FRAME-05, FRAME-06, FRAME-07
**Success Criteria** (what must be TRUE):
  1. `cmake --workflow --preset ci` (`ci-msvc` on Windows) builds the library, the runner and the libretro core and passes every test on Linux, macOS and Windows, x64 and arm64, and CI runs the same commands; the run includes a C program that uses only the public header against the installed package to create an instance, run one frame and destroy it.
  2. With no cartridge, `nesturbator-run --frames 1 --dump-frame 1:FILE --hash-frame 1` writes the test frame as an image and prints a SHA-256 that is the same on all six platforms, and the libretro test program, calling the core the way RetroArch does, receives an equal frame.
  3. After the README's one-line install on an Apple Silicon Mac, `ctest -L retroarch` launches RetroArch unattended and its screenshot matches the runner's frame; where RetroArch is not installed the test reports itself skipped.
  4. `cmake --workflow --preset asan`, `--preset nofp` and `--preset hygiene` pass: the tests are clean under the address and undefined-behaviour sanitizers, the core has no floating point and no undefined symbols beyond the C memory functions, and the tree holds no personal data and no unlisted ROM.
  5. Every pull request builds the library, runner and libretro archives for each platform, and a behaviour-changing merge to `main` publishes them with a `SHA256SUMS` file as a GitHub release with no manual step.
**Plans**: TBD
**UI hint**: no
**Canonical refs:** `.planning/preparation/ENGINEERING.md`, `.planning/preparation/LIBRETRO-AND-RUNNER.md`, `.planning/preparation/ARCHITECTURE.md`

### Phase 2: The CPU matches the public vectors
**Goal**: The 6502 behaves as the public 65x02 vectors say on every opcode and every bus cycle, and the release that merging publishes carries it.
**Depends on**: Phase 1
**Requirements**: CPU-01, CPU-02
**Success Criteria** (what must be TRUE):
  1. `ctest -L vectors`, run as part of `cmake --workflow --preset ci`, passes the committed sample of the 65x02 vectors for all 256 opcodes, matching final state and every bus cycle.
  2. `ctest -L vectors-full` fetches the full 65x02 vector set at its pinned commit and every test matches; CI runs it nightly.
  3. The `ci`, `asan`, `nofp` and `hygiene` presets still pass on all six platforms with the CPU in the library, and merging the phase publishes a release.
**Plans**: TBD
**UI hint**: no
**Canonical refs:** `.planning/preparation/NES-HARDWARE-CPU-APU.md`, `.planning/preparation/ARCHITECTURE.md`, `.planning/preparation/CONFORMANCE.md`, `.planning/preparation/ENGINEERING.md`

### Phase 3: A real game in RetroArch
**Goal**: A player loads an NROM game in RetroArch, sees it render and controls it, and its frames are identical on every platform.
**Depends on**: Phase 2
**Requirements**: GAME-01, GAME-02, GAME-03, GAME-04, GAME-05, GAME-06
**Success Criteria** (what must be TRUE):
  1. The runner loads an iNES or NES 2.0 file for mapper 0, and exits non-zero with a message on a malformed, truncated or oversized file; a counting-allocator test shows a rejected file allocates nothing, `ctest -R fuzz.regress` replays the loader's corpus without a crash on every platform, and the libFuzzer target runs nightly in CI.
  2. The runner's frames from the committed open-licence NROM games match their recorded hashes on all six platforms, the libretro test program receives equal frames, and `ctest -L retroarch` loads one of the games in RetroArch and its screenshot matches the runner's frame.
  3. `nesturbator-run --movie FILE` replays recorded controller input and prints the same frame hashes on every run, and the libretro test program, fed the same input, receives equal frames.
  4. The runner runs AccuracyCoin and reads each result from RAM; the results equal the committed scoreboard file, a test fails if that file loses a pass that `main` has, and every test on pages 2 and 17 passes.
**Plans**: TBD
**UI hint**: no
**Canonical refs:** `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md`, `.planning/preparation/NES-HARDWARE-CPU-APU.md`, `.planning/preparation/ARCHITECTURE.md`, `.planning/preparation/CONFORMANCE.md`, `.planning/preparation/LIBRETRO-AND-RUNNER.md`

### Phase 4: Sound
**Goal**: NROM games play with sound in RetroArch, and the audio is identical on every platform.
**Depends on**: Phase 3
**Requirements**: SND-01, SND-02, SND-03, SND-04
**Success Criteria** (what must be TRUE):
  1. A test in the `ci` preset steps each APU channel (two pulse, triangle, noise, DMC) through its documented output sequence; the runner's audio for the committed open-licence games matches recorded hashes, and the libretro test program receives equal samples.
  2. `nesturbator-run --hash-audio` prints two hashes, one of the level-transition stream and one of the 16-bit samples, and both are identical on all six platforms.
  3. The committed scoreboard shows AccuracyCoin's Length Counter, Length Table, Frame Counter IRQ, Frame Counter 4-step, Frame Counter 5-step and Delta Modulation Channel tests passing.
  4. A test in the `ci` preset renders five reference tones and shows the synthesiser's largest non-harmonic peak below 16 kHz is under -80 dB.
**Plans**: TBD
**UI hint**: no
**Canonical refs:** `.planning/preparation/NES-HARDWARE-CPU-APU.md`, `.planning/preparation/ARCHITECTURE.md`, `.planning/preparation/CONFORMANCE.md`

## Progress

**Execution Order:**
Phases execute in numeric order: 1 → 2 → 3 → 4

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. A test frame in RetroArch | 0/TBD | Not started | - |
| 2. The CPU matches the public vectors | 0/TBD | Not started | - |
| 3. A real game in RetroArch | 0/TBD | Not started | - |
| 4. Sound | 0/TBD | Not started | - |
