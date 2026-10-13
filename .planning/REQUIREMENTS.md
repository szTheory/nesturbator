# Requirements: nesturbator

**Defined:** 2026-10-11
**Core Value:** Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.

## v3 Requirements

Milestone v3: save states and rewind. These replace the v2 future requirements STATE-01, STATE-02, PERF-02 and PERF-03.

A platform is one of six: Linux, macOS and Windows, each on x64 and arm64. A "state" is a save state: the whole machine at a frame boundary. A battery save (`.sav`, `.srm`) is not a state. Research and the decisions behind these requirements are in `.planning/research/SUMMARY.md`.

### Tune-up and v2 debt

- [x] **TUNE-07**: Battery saves and state files are written by one helper that uses a temporary file, an fsync, a checked close and a rename. A failure-injection test, with an unwritable directory or a directory at the target path, finds the old file intact and no `.tmp` left behind. This closes v2 debt 08 IN-02.
- [x] **TUNE-08**: The `retroarch-e2e` job round-trips an MMC3 battery game's `.srm` across two RetroArch sessions, as it already does for MMC1. This closes v2 debt G2.
- [x] **TUNE-09**: `nesturbator_get_version` and `nesturbator_unload_cartridge` each have a caller in the runner or the libretro adapter and are described in the README. This closes v2 debt G6.
- [x] **TUNE-10**: The nightly suite-flake timeout is about twice a measured cold run, taken after the v3 tests exist. Action pins, runner labels and the AccuracyCoin pin are reviewed, and the README and the public header describe the released code. This closes Backlog 999.1 and SEED-006.

### Save states in the library

- [x] **STATE-01**: A host can call `nesturbator_state_size`, `nesturbator_save_state` and `nesturbator_load_state`. The size is fixed from cartridge load to unload, and is 0 with no cartridge. A save writes into the caller's buffer, never allocates, and does not change the instance. Status codes are appended to the public header, and a "Save states" section is added to the README.
- [x] **STATE-02**: A state is canonical little-endian bytes. It starts with a header that carries:
  - a magic and the state's own format version
  - the total size and the behaviour revision (recorded, not enforced)
  - the mapper, submapper and region
  - a CRC-32 of the ROM and a CRC-32 of the state
  
  The same game and inputs give byte-identical states on all six platforms, and the state hashes are pinned in `tests/runner/hashes.txt`.
- [x] **STATE-03**: Saving, loading and continuing gives the same result as an uninterrupted run, for a committed ROM on each of the six boards. The check runs at several frames, including frame 0, and loads into a fresh instance pre-filled with another game. It compares every later frame hash, every audio hash, the tick count and the final state bytes. Saving every frame changes no pinned hash.
- [x] **STATE-04**: A load either applies the whole state or changes nothing. It rejects the following with distinct status codes and leaves the instance byte-identical:
  - a state from another game or board
  - a state from another format version
  - a state cut short at any length
  - a state with any flipped byte
  - a state whose CRC was recomputed but whose fields are out of range
  
  The ASan and UBSan presets run these checks.
- [x] **STATE-05**: The state includes battery RAM. A load raises `nesturbator_save_generation()` only when the battery bytes actually change, so repeated loads of the same state never trigger a `.sav` write. The README says that rewinding past an in-game save reverts the save RAM.

### libretro: rewind and run-ahead

- [x] **LR-01**: The libretro core implements `retro_serialize_size`, `retro_serialize` and `retro_unserialize` over the library state, accepting only the exact size and setting no serialization quirks. With no content loaded the size is 0. A rejected buffer returns false, and the next frame equals one with no load.
- [x] **LR-02**: The `.info` file claims `serialized` (rewind) once the serialize round trip passes on all six platforms. It claims `deterministic` (run-ahead and netplay) only once a simulated host passes. That host runs ahead by one to three frames with a save and load per frame, and its visible frame and audio hashes must equal an uninterrupted run. A test checks that the `.info` keys match these claims.
- [x] **LR-03**: The `retroarch-e2e` job saves and loads a state, rewinds, and runs with run-ahead enabled in RetroArch. It drives these through RetroArch's network command interface, and the run must finish with no core error and the expected screenshot.
- [x] **LR-04**: A nightly libFuzzer target feeds the state loader, and its committed corpus is replayed in `ci`.

### Runner process contract

- [x] **RUN-01**: The runner has these state options, and a rejected state exits with status 5:
  - `--state-at-frame N:FILE` writes a state after frame N.
  - `--load-state FILE` resumes from a state after the battery save loads.
  - `--hash-state N` prints the state hash.
  
  State files are written only to the path given.
- [x] **RUN-02**: On SIGTERM or SIGINT the runner finishes the current frame, flushes the battery save, and exits 0, or 1 if the flush failed or a requested output (a --hash-frame, --hash-state, --state-at-frame or --dump-frame frame, --hash-audio, or the end of a --movie) was not reached. The signal handler only sets a flag. A second signal kills it immediately. On Windows, Ctrl+Break and console close are handled the same way, and the README says that a forced termination cannot be handled.
- [x] **RUN-03**: Exit statuses are one table, in one header, `--help` and the README:
  - 0 done
  - 1 failure
  - 2 usage
  - 4 battery file wrong length
  - 5 state rejected
  
  3 and 77 are reserved. Each status has a test. A frame-indexed signal test runs on all six platforms, and an external-signal test on Linux and macOS checks exit 0 within 2 s with the whole `.sav` and no `.tmp`.

### Speed

- [ ] **PERF-02**: On every pull request, a CI job uses Cachegrind to compare instructions per frame between the PR head and its merge base on three committed ROMs (NROM, MMC1 and MMC3), and writes the table to the job summary. It starts as a report. It becomes a gate at +1% per ROM only after identical builds show zero spread, and a deliberate slowdown passes with a reviewed line in an allow file.
- [ ] **PERF-03**: `nesturbator-run --bench N` prints frames per second and the p50, p90, p99, p99.9 and max frame times. `--bench-state` adds the cost of a save and load per frame. A test shows that benchmarking changes no hash, and wall time is never gated.

## Future Requirements

- **PERF-01**: Property tests and long fuzz runs over bounded runs of arbitrary ROM bytes (the state-loader fuzz target is LR-04).
- **STATE-06**: A movie that starts from a state.
- **RUN-04**: `--expect` (exit 3) and `--capabilities` (exit 77) for launcher preflight.
- **ACC-01**: Every AccuracyCoin test passes at the pinned commit.
- **ACC-02**: PAL and Dendy games run at their own timing.
- **LIB-03**: A committed MMC3 real-game hash from Stallar, once the provenance of its art is verified.
- **LIB-04**: Mapper 66 (GxROM), mappers 206, 5, 19, 16, 18, 69 and 210, four-screen cartridge VRAM, and MMC6.
- **HW-01**: Expansion audio, the Famicom Disk System and peripherals beyond the standard controller.

## Out of Scope

| Feature | Reason |
|---------|--------|
| Rewind, run-ahead or netplay loops inside the core | The host owns them; the core provides fast, deterministic states |
| Compression or deltas inside the core | The host's job; the core stays a pure byte format |
| Converting states from an older format version | No second format exists yet; a version mismatch is refused cleanly |
| Serialization quirk flags | Each one turns off run-ahead or netplay; a gap is fixed, not excused |
| Saving mid-instruction or mid-frame through the API | libretro requests states between frames; the format still carries position fields |
| A committed binary state file | Hash pins catch layout drift without committing RAM bytes |
| Wall-clock thresholds in CI | Hosted runners vary by several percent; instruction counts are deterministic |
| Submitting the core to the libretro buildbot | An outward-facing step the owner decides on separately |
| Code from GPL or LGPL emulators | The project is MIT and clean-room |

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| TUNE-07 | Phase 11 | Complete |
| TUNE-08 | Phase 11 | Complete |
| TUNE-09 | Phase 11 | Complete |
| STATE-01 | Phase 12 | Complete |
| STATE-02 | Phase 12 | Complete |
| STATE-03 | Phase 12 | Complete |
| STATE-04 | Phase 12 | Complete |
| STATE-05 | Phase 12 | Complete |
| RUN-01 | Phase 12 | Complete |
| LR-01 | Phase 13 | Complete |
| LR-02 | Phase 13 | Complete |
| LR-03 | Phase 13 | Complete |
| LR-04 | Phase 13 | Complete |
| RUN-02 | Phase 14 | Complete |
| RUN-03 | Phase 14 | Complete |
| PERF-02 | Phase 15 | Pending |
| PERF-03 | Phase 15 | Pending |
| TUNE-10 | Phase 15 | Complete |

**Coverage:**
- v3 requirements: 18 total
- Mapped to phases: 18
- Unmapped: 0

---
*Requirements defined: 2026-10-11*
*Last updated: 2026-10-11 after v3 roadmap creation*
