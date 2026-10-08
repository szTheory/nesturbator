---
phase: 03-a-real-game-in-retroarch
plan: 11
subsystem: runner
tags: [C17, movie-replay, deterministic-input, libretro]
requires:
  - phase: 03-05
    provides: Size-checked per-frame two-port input masks and libretro polling
provides:
  - Versioned, bounded, validated two-port movie format and runner replay
  - Stable ordered native frame hashes for every movie frame
  - Cross-host pixel equality for shared scripted input
affects: [runner, libretro-input, deterministic-frame-hashes]
actuals:
  tokens: 6428
  tasks: 1
  commits: 2
tech-stack:
  added: []
  patterns:
    - Parse and validate the complete bounded movie before creating an emulator instance
    - Replay each port mask through the public frame-entry input API
key-files:
  created:
    - runner/movie.c
    - runner/movie.h
    - tests/runner/movie_fixture.h
    - tests/runner/test_movie.c
  modified:
    - runner/main.c
    - runner/CMakeLists.txt
    - tests/libretro/libretro_host.c
    - tests/CMakeLists.txt
    - README.md
    - include/nesturbator.h
key-decisions:
  - "Use a 16-byte NMOVIE1 header with little-endian version and frame count, followed by two little-endian uint16 button masks per frame; masks must fit the standard eight buttons."
  - "Bound movies to 1,000,000 frames and require exact file length before replay."
  - "Movie runs hash every completed frame in ascending order; JAM terminates nonzero with the one-based frame number."
requirements-completed: [GAME-03]
coverage:
  - id: D1
    description: "Runner replays empty and multi-frame owned movies deterministically and rejects malformed bytes before emitting a frame."
    requirement: GAME-03
    verification:
      - kind: unit
        ref: tests/runner/test_movie.c#runner.movie
        status: pass
    human_judgment: false
  - id: D2
    description: "The same two-port scripted masks produce pixel-equal runner and libretro host frames."
    requirement: GAME-03
    verification:
      - kind: integration
        ref: tests/libretro/libretro_host.c#check_input_frame_parity
        status: pass
    human_judgment: false
metrics:
  duration: 8min
  completed: 2026-10-08
  status: complete
  commits: 2
  plan_head_before: 0686bdd74207ce3a731ba5fe8f0a997894bdfae5
  plan_head_after: 5c2a7b65f4b714600272e5973cd9406983462fcc
---

# Phase 03 Plan 11: Deterministic Movie Replay Summary

**A validated, versioned two-port movie format now drives repeatable native frame hashes and matches the scripted libretro host frame.**

## Performance

- **Duration:** 8 min
- **Started:** 2026-10-08T18:04:00Z (approximate)
- **Completed:** 2026-10-08T18:12:14Z
- **Tasks:** 1
- **Files modified:** 10

## Accomplishments

- Added the owned `NMOVIE1` format: a fixed magic, little-endian version and frame count, followed by two little-endian 16-bit masks per frame in port order.
- Validated the whole file, exact byte length, version, 1,000,000-frame bound, and mask width before emulator creation or frame execution.
- Added `--movie FILE`; each replayed frame applies both masks through `nesturbator_set_input`, emits its canonical native-pixel hash, and terminates predictably on JAM with the frame number.
- Reused a shared A-on-port-0 and B+Left-on-port-1 fixture in runner and libretro host checks, then compared the produced frames pixel-for-pixel.
- Documented the movie format and relationship to the public frame input API.

## Task Commits

1. **Task 1 RED: add empty movie replay regression** — `dd62561` (`test(03-11): add empty movie replay regression`).
2. **Task 1 GREEN: replay deterministic two-port movies** — `5c2a7b6` (`feat(03-11): replay deterministic two-port movies`).

**Plan metadata:** recorded with the GSD summary/state commit.

## Files Created/Modified

- `runner/movie.c`, `runner/movie.h` — complete bounds-checked parser and movie storage lifecycle.
- `runner/main.c`, `runner/CMakeLists.txt` — CLI replay, per-frame input application, ordered hashes, and JAM reporting.
- `tests/runner/test_movie.c`, `tests/runner/movie_fixture.h`, `tests/CMakeLists.txt` — empty/multi-frame replay, repeat equality, malformed-file rejection, and JAM checks.
- `tests/libretro/libretro_host.c` — shared mask replay compared pixel-for-pixel across hosts.
- `README.md`, `include/nesturbator.h` — user-facing format and public input contract documentation.

## Decisions Made

- Records store each mask as a little-endian `uint16_t` so the parser can reject values using bits outside the eight standard NES buttons.
- The movie frame count is authoritative. If `--frames` is also supplied, it must match exactly.
- Movie playback emits hashes for every frame in ascending order, regardless of `--hash-frame` filters.

## TDD Gate Compliance

- **RED:** `ctest --test-dir build/ci -R '^runner\.movie\.empty$' --output-on-failure` failed because `--movie` was an unknown option; the intended empty-movie replay assertion reached the runner and failed for the missing behavior. Evidence was reviewed semantically before implementation.
- **GREEN:** `runner.movie` passed, including empty and three-frame replay, repeat byte equality, malformed/truncated/version/count/mask/trailing/missing-file rejection before any frame line, and JAM frame reporting. `libretro.host` passed the shared two-port cross-host pixel comparison.
- **REFACTOR:** No separate refactor commit was needed.

## Deviations from Plan

None - plan executed as specified. The internal `runner/movie.h` and shared test fixture header were added to keep the parser interface private and avoid duplicating scripted input values.

## Issues Encountered

- `cmake --workflow --preset ci` configured and built successfully, then ran 339 tests: 338 passed. The sole failure was the known local GUI-only `retroarch.testframe` abort (`Subprocess aborted`, empty stdout/stderr), also recorded in the Phase 03-05 summary. This run is not a fully green workflow. The new movie and libretro host tests passed in the same run.

## Threat Flags

| Flag | File | Description |
|------|------|-------------|
| threat_flag: file-input | runner/movie.c | Parses an untrusted local movie file; exact size, version, frame count, and mask bounds are checked before replay. |

## User Setup Required

None - movie and host checks use owned synthetic inputs and cartridge bytes.

## Next Phase Readiness

GAME-03 now has automated deterministic movie replay and cross-host input/frame coverage. The existing two-port input seam is ready for the planned game fixture extension.

---
*Phase: 03-a-real-game-in-retroarch*
*Completed: 2026-10-08*

## Self-Check: PASSED

- SUMMARY.md exists at the planned phase path.
- RED commit `dd62561` and GREEN commit `5c2a7b6` are ancestors of HEAD.
- The two task commits measured from plan base `0686bdd74207ce3a731ba5fe8f0a997894bdfae5` are present.
