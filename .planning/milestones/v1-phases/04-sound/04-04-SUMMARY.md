---
phase: 04-sound
plan: 04
subsystem: audio
tags: [apu, pcm, sha256, libretro, deterministic]
requires:
  - phase: 04-sound
    plan: 03
    provides: Fixed-point transition-to-PCM synthesis and per-instance audio history
provides:
  - Private per-instance APU transition observer and canonical transition/PCM SHA-256 streams
  - Runner --hash-audio option with serialization known answers
  - Three licensed-game audio baselines and exact libretro stereo duplication checks
affects: [runner, libretro, ci, phase-04]
actuals:
  tokens: 13203
  tasks: 2
  commits: 2
tech-stack:
  added: []
  patterns: [synchronous transition observation, explicit little-endian hash streams, device-independent sample parity]
key-files:
  created: [runner/audio_hash.c, runner/audio_hash.h, tests/runner/test_audio_hash.c]
  modified: [src/apu.c, src/internal.h, runner/main.c, runner/CMakeLists.txt, tests/CMakeLists.txt, tests/core/test_synth.c, tests/cmake/write_hashes.cmake, tests/runner/hashes.txt, tests/libretro/libretro_host.c, include/nesturbator.h, README.md]
key-decisions:
  - "Observe only changed mixed levels, synchronously before synthesis, using the per-instance tick cursor divided by 24 as the CPU-cycle timestamp."
  - "Serialize each transition as uint64 cycle LE plus int32 level LE, and each signed PCM sample as int16 LE, feeding the existing streaming SHA-256 without retaining an event log."
  - "Keep acceptance device-independent: pinned licensed ROM hashes and sample-by-sample libretro/core parity run in CI without owner UAT."
patterns-established:
  - "A private callback seam can expose production APU transitions to runner diagnostics without changing the public API or ABI."
  - "Frontend output parity is checked against a separately executed core instance using the same synthetic NROM and frame inputs."
requirements-completed: [SND-01, SND-02]
coverage:
  - id: D1
    description: "The runner prints independent, canonical SHA-256 hashes for APU transition records and signed 16-bit PCM."
    requirement: SND-02
    verification:
      - kind: unit
        ref: tests/runner/test_audio_hash.c (empty, single-event, equal-cycle order, PCM byte order, and silence known answers)
        status: pass
      - kind: integration
        ref: runner.hash_audio.silent (one-frame no-cartridge transition and PCM digests)
        status: pass
      - kind: other
        ref: "cmake --workflow --preset ci"
        status: pass
    human_judgment: false
  - id: D2
    description: "APU transition callbacks observe each changed mixed level before synthesis, preserve same-cycle order, and omit unchanged levels."
    requirement: SND-02
    verification:
      - kind: unit
        ref: tests/core/test_synth.c#check_transition_observer_order
        status: pass
      - kind: other
        ref: "cmake --workflow --preset ci"
        status: pass
    human_judgment: false
  - id: D3
    description: "Nesteroids, DABG, and RHDE have stable transition and PCM baselines, and libretro duplicates every tested mono sample to both stereo channels."
    requirement: SND-01
    verification:
      - kind: integration
        ref: tests/runner/hashes.txt and runner.write_hashes.content (three licensed-game boot baselines)
        status: pass
      - kind: integration
        ref: tests/libretro/libretro_host.c#check_sound_frame_parity (two audible frames compared sample-by-sample)
        status: pass
      - kind: other
        ref: "cmake --workflow --preset ci"
        status: pass
    human_judgment: false
metrics:
  duration: 10min
  completed: 2026-10-09
  status: complete
---

# Phase 04 Plan 04: Canonical Audio Hashes and Frontend Parity Summary

**The runner now hashes canonical APU transitions and PCM, while CI pins three games and proves libretro stereo samples match the core.**

## Performance

- **Duration:** 10 min (estimated from the prior execution checkpoint)
- **Started:** 2026-10-09T04:58:00Z (estimated from STATE.md)
- **Completed:** 2026-10-09T05:07:54Z
- **Tasks:** 2
- **Files modified:** 14

## Accomplishments

- Added a private per-instance transition sink in the APU path. It observes changed mixed levels before synthesis and timestamps them with the CPU-cycle cursor.
- Added `nesturbator-run --hash-audio`, which streams 12-byte transition records and signed 16-bit PCM bytes into separate SHA-256 digests.
- Added canonical known answers, a no-cartridge silence digest, three licensed-game dual-hash baselines, and sample-by-sample libretro stereo parity checks.
- Updated README and public-header audio comments; no public API, ABI, external dependency, ROM, or audio device was added.

## Task Commits

1. **Task 1: Add the private transition observer and canonical dual hashes** — `35d83b0` (`feat`).
2. **Task 2: Lock licensed-game audio hashes and libretro sample equality** — `07708de` (`test`).

## Files Created/Modified

- `runner/audio_hash.c`, `runner/audio_hash.h` — streaming canonical transition and PCM serializers.
- `tests/runner/test_audio_hash.c` — empty, event, tie-order, signed-sample, and silence known answers.
- `src/apu.c`, `src/internal.h`, `runner/main.c` — private observer seam and runner integration.
- `tests/runner/hashes.txt`, `tests/cmake/write_hashes.cmake` — six additional audio baselines for the three licensed games.
- `tests/libretro/libretro_host.c` — compares both channels for two audible frames against a direct core instance.
- `tests/core/test_synth.c`, `tests/CMakeLists.txt`, `runner/CMakeLists.txt`, `include/nesturbator.h`, `README.md` — tests, build wiring, and documentation.

## Decisions Made

- Transition records use the instance tick cursor divided by 24 for the CPU-cycle field; the callback runs before synthesis and the stream retains generation order for equal-cycle events.
- The runner reuses the owned SHA-256 implementation and stores no transition list.
- CI covers the acceptance path without audio hardware or manual UAT.

## Deviations from Plan

- Added the observer timing/order assertion to the existing `tests/core/test_synth.c` and extended the existing `tests/cmake/write_hashes.cmake` inventory generator. These are the established core and baseline test seams; no new test framework or runtime fixture fetch was introduced.

## Issues Encountered

- The local `retroarch.testframe` and `retroarch.game` tests self-skipped because RetroArch is not available on this host. The device-independent `libretro.host` test passed with audible samples on both checked frames.

## User Setup Required

None. The phase's sound behavior is covered by CI; no audio device or listening check is required.

## Next Phase Readiness

Plan 04-04 is complete. Phase-goal verification and the enabled code-review gate remain in the execute-phase closeout; the plan's acceptance checks are automated.

---
*Phase: 04-sound*
*Completed: 2026-10-09*
