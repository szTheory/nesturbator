---
phase: 13-rewind-and-run-ahead-in-retroarch
plan: 01
subsystem: libretro
tags: [libretro, savestate, serialize, cmake-guard, info-claims]
requires:
  - phase: 12
    provides: nesturbator_state_size, nesturbator_save_state, nesturbator_load_state
provides:
  - retro_serialize_size, retro_serialize, retro_unserialize delegating to the library state API
  - libretro.host --serialize mode and CTest libretro.serialize
  - configure-time claim guard, .info check, no-quirks check
affects: [13-02, 13-03]
tech-stack:
  added: []
  patterns: ["claim-to-tests table once in tests/cmake/info_claims.cmake, guard via cmake_language(DEFER)"]
key-files:
  created:
    - tests/cmake/info_claims.cmake
    - tests/cmake/claim_guard/CMakeLists.txt
    - tests/cmake/libretro_no_quirks.cmake
  modified:
    - libretro/libretro.c
    - tests/libretro/libretro_host.c
    - tests/CMakeLists.txt
    - README.md
key-decisions:
  - "Adapter keeps no state for serialization; size/save/load are direct delegations."
  - "D-09 battery check follows the documented save-generation rule: a load counts once when it changes the battery bytes, never when loading the same state again."
  - "libretro.serialize runs five ROMs, trimmed from seven to keep the asan run under 60 s."
requirements-completed: [LR-01, LR-02]
status: complete
commits: 3
plan_head_before: 2cac8fb2677a1ea259b33e92f186166024225e00
plan_head_after: 7aa3ac41cf1bb412e4aab2c834e551df276db8bd
actuals:
  tokens: 11800
  tasks: 3
  commits: 3
duration: 45 min
completed: 2026-10-12
---

# Phase 13 Plan 01: Serialize entry points and the claim guard Summary

The libretro module now saves and loads through the library state API, `libretro.serialize` proves every D-01..D-09 property through the real entry points on five ROMs, and a configure-time guard ties any `.info` save-state claim to registered proving tests (the `.info` still says `savestate = "false"`).

## Accomplishments

- `retro_serialize_size`, `retro_serialize`, `retro_unserialize` are three one-line delegations with no adapter state, no quirks, and no context or AV-flag queries. Commit de37d5e.
- `libretro.host --serialize` checks size (library equality, constant over 120 frames, reset, JAM, controller-port change, 0 after unload, right after reload of another board), exact write length with sentinel bytes, 13 refusal cases per ROM against a lockstep control (no callbacks, same next frame and state), unserialize straight after load and after reset, six context/AV passes with identical bytes, battery span stability, and test-card / failed-load / unload as no content. Commit d010456.
- `tests/cmake/info_claims.cmake` holds the claim-to-tests table, `.info` parser, guard, `-P` check and self-test; `libretro.info_claims`, `libretro.info_claims.self_test`, `libretro.no_quirks` registered; README updated. Commit 7aa3ac4.

Printed count line (ci): `roms=5 min_refusals=13 frames_compared>=120 per ROM battery_roms=1 context_queries=0 av_queries=0`.

## Measured budget and the D-09 correction

- libretro.serialize wall time, 7 ROMs (first design): ci about 10 s, asan 89 s (108 s once under load), over the 60 s rule.
- After faster hashing, shorter unhashed stretches, loading the reference state instead of re-running 20 frames per refusal, 30-frame round trip: 6 ROMs, ci 4.5 s, asan 66 s. Still over.
- Final: 5 ROMs (removed from the end of the list: M7_P128K_CR8K, M4_P128K_CR8K), ci 3.7 s, asan 56.5 s and 57.1 s. `SER_MIN_ROMS` lowered to 5. Margin under 60 s is thin; TIMEOUT is 120.
- D-09 ROM correction: `M1_P128K_C128K_S8K` (battery, 8192-byte span) is used because `M1_P128K_C32K_W8K` has no battery bit; the span is asserted non-NULL and 8192 bytes before the loop.
- Mutation check: making `retro_unserialize` return true without loading fails libretro.serialize with `CHECK(!p_unserialize(r->state, r->state_size)) failed` (no-content refusal) and the NULL-buffer refusal.
- Guard check: setting `savestate = "true"` with `deterministic` makes `cmake --preset ci` fail naming `libretro.runahead` as not registered.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] D-09 wording contradicted the documented save-generation rule**
- **Found during:** Task 2
- **Issue:** the plan said a load per frame never raises `nesturbator_save_generation`. The library (documented in `nesturbator.h`, Phase 12) counts a load once when it changes the battery bytes; frames run since the save change them, so a load per frame moved it by exactly 1.
- **Fix:** the direct-instance check asserts the generation rises by 0 or 1 on a load after frames, and never moves when the same state is loaded again (what rewind does). The module-side check (span pointer and size unchanged, non-NULL, 8192) is as planned.
- **Files modified:** tests/libretro/libretro_host.c
- **Commit:** d010456

**2. [Rule 3 - Blocking] ROM list trimmed and test cost reduced to meet the asan budget**
- **Found during:** Task 2
- **Fix:** per the plan's trim rule, M7 and M4 removed; ROM minimum lowered to 5. The 120-frame size loop counts toward the 120-frames-compared floor (size compared every frame); hashed comparisons per ROM are about 100.
- **Commit:** d010456

**Total deviations:** 2 auto-fixed. **Impact:** board coverage in libretro.serialize is NROM (two ROMs), MMC1, UxROM-class M2, M3; MMC3 (M4) and AxROM (M7) are not in this test.

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED

Files exist (info_claims.cmake, claim_guard/CMakeLists.txt, libretro_no_quirks.cmake); commits de37d5e, d010456, 7aa3ac4 are on the branch; `cmake --workflow --preset ci` and `hygiene` pass; GCC 14 `-Werror` compile and clang-format 18 (local and ubuntu:24.04) pass.
