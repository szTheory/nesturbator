# Requirements: nesturbator

**Defined:** 2026-10-02
**Core Value:** Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.

## v1 Requirements

Milestone 1. Each category's pull request builds the release archives, and merging it publishes them (FRAME-07). A platform is one of six: Linux, macOS and Windows, each on x64 and arm64. Output from the libretro core equals the runner's when it matches after the adapter's fixed conversion: the palette table for pixels, both channels for audio.

### A test frame in RetroArch

- [x] **FRAME-01**: `cmake --workflow --preset ci` (`ci-msvc` on Windows) builds the library, the runner and the libretro core and runs every test on all six platforms; CI runs the same commands.
- [x] **FRAME-02**: A C program that includes only the public header creates an instance, runs one frame and destroys it; the `ci` preset builds and runs it against the installed package.
- [x] **FRAME-03**: With no cartridge loaded the core outputs a fixed test frame. `nesturbator-run --frames 1 --dump-frame 1:FILE --hash-frame 1` writes it as an image and prints its SHA-256, and the hash is the same on every platform.
- [x] **FRAME-04**: The libretro core loads in a test program that calls it the way RetroArch does, and the frame it delivers equals the runner's.
- [x] **FRAME-05**: The README's one-line install lets RetroArch on an Apple Silicon Mac load the core and show the test frame. `ctest -L retroarch` runs that launch unattended with RetroArch's frame-limit and screenshot options and compares the screenshot with the runner's frame; it reports itself skipped where RetroArch is not installed.
- [x] **FRAME-06**: The `asan`, `nofp` and `hygiene` presets pass: the tests run clean under the address and undefined-behaviour sanitizers, the core has no floating point and no undefined symbols beyond the C memory functions, and the tree holds no personal data and no unlisted ROM.
- [x] **FRAME-07**: Every pull request builds the library, runner and libretro archives for each platform; a behaviour-changing merge to `main` publishes them, with a `SHA256SUMS` file, as a GitHub release with no manual step.

### The CPU matches the public vectors

- [x] **CPU-01**: `ctest -L vectors` runs the committed sample of the 65x02 vectors through the CPU, and every opcode matches on final state and on every bus cycle.
- [x] **CPU-02**: `ctest -L vectors-full` runs the full 65x02 vector set, fetched at its pinned commit, with every test matching; CI runs it nightly.

### A real game in RetroArch

- [ ] **GAME-01**: The runner loads an iNES or NES 2.0 file for mapper 0, and rejects a malformed, truncated or oversized file with a message and a non-zero exit status; a test with a counting allocator shows that a rejected file allocates nothing.
- [ ] **GAME-02**: The runner's frames from the committed open-licence NROM games match their recorded hashes, and the libretro test program receives equal frames; `ctest -L retroarch` loads one of the games in RetroArch and compares its screenshot with the runner's frame.
- [ ] **GAME-03**: `nesturbator-run --movie FILE` replays recorded controller input and prints the same frame hashes on every run; the libretro test program, fed the same input, receives equal frames.
- [ ] **GAME-04**: The runner runs AccuracyCoin and reads each test's result from RAM; the results equal the committed scoreboard file, a test fails if that file loses a pass that `main` has, and every test on pages 2 and 17 (addressing-mode wraparound, PPU behaviour) passes.
- [ ] **GAME-05**: `ctest -R fuzz.regress` replays the ROM loader's corpus without a crash on every platform, and the loader's libFuzzer target runs nightly in CI.
- [ ] **GAME-06**: Frame hashes for every committed ROM are identical on all six platforms.

### Sound

- [ ] **SND-01**: A test steps each APU channel (two pulse, triangle, noise, DMC) through its documented output sequence; the runner's audio for the committed open-licence games matches recorded hashes, and the libretro test program receives equal samples.
- [ ] **SND-02**: `nesturbator-run --hash-audio` prints two hashes, one of the level-transition stream and one of the 16-bit samples, and both are identical on every platform.
- [ ] **SND-03**: The scoreboard shows AccuracyCoin's length counter, frame counter and DMC tests passing: Length Counter, Length Table, Frame Counter IRQ, Frame Counter 4-step, Frame Counter 5-step and Delta Modulation Channel.
- [ ] **SND-04**: A test in the `ci` preset renders five reference tones and shows that the synthesiser's largest non-harmonic peak below 16 kHz is under -80 dB.

## v2 Requirements

Later milestones. Each has a seed in `.planning/seeds/` that says when it comes back.

### Library coverage

- **LIB-01**: A player loads games on MMC1, MMC3, UxROM, CNROM and AxROM boards, each shown by Holy Mapperel and frame hashes.
- **LIB-02**: Battery saves persist across runs as raw bytes compatible with existing `.sav` files.

### Save states and replay

- **STATE-01**: A player uses save states, rewind and run-ahead in RetroArch; a property test shows save, load and continue equals an uninterrupted run.
- **STATE-02**: The runner honours the process contract Playstead needs: save directory, flush on signal, distinct exit statuses.

### Accuracy

- **ACC-01**: Every AccuracyCoin test passes at the pinned commit.
- **ACC-02**: PAL and Dendy games run at their own timing.

### Robustness and speed

- **PERF-01**: Property tests and long fuzz runs cover the loader, the state loader and bounded runs of arbitrary ROM bytes.
- **PERF-02**: CI compares instructions per frame for the committed ROMs between a pull request and `main`, and fails when the count rises past the committed baseline.
- **PERF-03**: `nesturbator-run --bench ROM` prints frames per second and frame-time percentiles up to the slowest frame.

### More hardware

- **HW-01**: Expansion audio, the Famicom Disk System and peripherals beyond the standard controller.

## Out of Scope

See "Out of Scope" in `.planning/PROJECT.md`.

## Traceability

Which phases cover which requirements. Updated during roadmap creation.

| Requirement | Phase | Status |
|-------------|-------|--------|
| FRAME-01 | Phase 1 | Complete |
| FRAME-02 | Phase 1 | Complete |
| FRAME-03 | Phase 1 | Complete |
| FRAME-04 | Phase 1 | Complete |
| FRAME-05 | Phase 1 | Complete |
| FRAME-06 | Phase 1 | Complete |
| FRAME-07 | Phase 1 | Complete |
| CPU-01 | Phase 2 | Complete |
| CPU-02 | Phase 2 | Complete |
| GAME-01 | Phase 3 | Pending |
| GAME-02 | Phase 3 | Pending |
| GAME-03 | Phase 3 | Pending |
| GAME-04 | Phase 3 | Pending |
| GAME-05 | Phase 3 | Pending |
| GAME-06 | Phase 3 | Pending |
| SND-01 | Phase 4 | Pending |
| SND-02 | Phase 4 | Pending |
| SND-03 | Phase 4 | Pending |
| SND-04 | Phase 4 | Pending |

**Coverage:**
- v1 requirements: 19 total
- Mapped to phases: 19
- Unmapped: 0

---
*Requirements defined: 2026-10-02*
*Last updated: 2026-10-02 after roadmap creation*
