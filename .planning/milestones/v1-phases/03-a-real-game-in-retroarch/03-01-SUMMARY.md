---
phase: 03-a-real-game-in-retroarch
plan: 01
subsystem: emulator-core
tags: [c17, nrom, mapper-0, libretro, ppu, ctest]

requires:
  - phase: 02-the-cpu-matches-the-public-vectors
    provides: instruction-level 6502 core and 24-tick CPU bus cycles
provides:
  - mapper-0 iNES load/unload API for one 16 KiB PRG and 8 KiB CHR image
  - shared CPU, cartridge, PPU, and frame-clock path for runner and libretro
  - generated ROM integration test with deterministic native frame comparison
  - JAM stop status and vblank, NMI, odd-frame timing behavior
affects: [03-02, 03-03, 03-04, 03-05, 03-06, 03-07, 03-08, 03-09, 03-10, 03-11, 03-12, 03-13]

actuals:
  tokens: 10072.5
  tasks: 2
  commits: 4
commits: 4
plan_head_before: 296ea3d8833383a4986995a2c2616e2be1ab35c0
plan_head_after: 7bc778834b41f439468f5c4eb1d77b69fe8bd8db

tech-stack:
  added: []
  patterns: [validated in-memory cartridge ownership, instruction-boundary frame stepping, caller-owned video rendering]

key-files:
  created: []
  modified:
    - include/nesturbator.h
    - src/internal.h
    - src/bus.c
    - src/frame.c
    - src/instance.c
    - src/cpu.c
    - runner/main.c
    - libretro/libretro.c
    - tests/CMakeLists.txt
    - tests/cmake/vector_api_policy.cmake
    - tests/libretro/libretro_host.c
    - README.md

key-decisions:
  - "Support one 16 KiB PRG and one 8 KiB CHR mapper-0 iNES geometry in this tracer."
  - "Carry frame overshoot from the current instance tick cursor; JAM reports a stable nonzero stop status."
  - "Refresh the temporary public declaration guard for the appended cartridge API."

requirements-completed: [GAME-01, GAME-02]
coverage:
  - id: D1
    description: "Generated mapper-0 content loads through the shared core, runner, and libretro host and produces matching deterministic pixels."
    requirement: GAME-01
    verification:
      - kind: integration
        ref: "tests/libretro/libretro_host.c#libretro.host"
        status: pass
    human_judgment: false
  - id: D2
    description: "Frame calls retain instruction overshoot, JAM remains latched, and vblank/NMI and odd-frame edges are checked."
    requirement: GAME-02
    verification:
      - kind: integration
        ref: "tests/libretro/libretro_host.c#check_core_timing_and_jam"
        status: pass
      - kind: integration
        ref: "tests/libretro/libretro_host.c#check_ppu_frame_edges"
        status: pass
    human_judgment: false

duration: 13min
completed: 2026-10-08
status: complete
---

# Phase 03 Plan 01: Mapper-0 tracer summary

**Generated NROM content now boots through the shared core, runner, and libretro adapter with deterministic background output and retained frame timing.**

## Performance

- **Duration:** 13 min
- **Started:** 2026-10-08T16:47:17Z
- **Completed:** 2026-10-08T17:00:57Z
- **Tasks:** 2
- **Files modified:** 12

## Accomplishments

- Added validated cartridge load/unload APIs and copied mapper-0 PRG/CHR bytes into allocator-owned instance state.
- Wired CPU bus accesses to NROM and basic PPU registers, nametable, palette, background rendering, and shared tick stepping.
- Added `--rom FILE` to the runner and enabled in-memory content loading through `retro_load_game`; the generated test image reaches both hosts and converted output matches.
- Kept the no-content test card, documented silent audio, reported JAM through `NESTURBATOR_STOP_JAM`, and tested NMI/vblank and odd-frame dot behavior.
- Preserved the custom allocator size expectations by rendering directly into the caller-owned video buffer.

## Task Commits

1. **Task 1 RED:** `8660f5a` — add generated NROM load assertion.
2. **Task 1 GREEN:** `019df58` — boot mapper-0 content through both hosts.
3. **Task 2 RED:** `56487c3` — cover vblank and odd-frame PPU edges.
4. **Task 2 GREEN:** `7bc7788` — preserve PPU vblank and odd-frame edges.

## Files Created/Modified

- `include/nesturbator.h`, `src/internal.h`, `src/instance.c` — public cartridge lifecycle, status, and instance state.
- `src/bus.c`, `src/frame.c`, `src/cpu.c` — NROM bus, background PPU path, timing, vblank/NMI, odd-frame skip, JAM stop.
- `runner/main.c`, `libretro/libretro.c` — content loading at both host boundaries.
- `tests/libretro/libretro_host.c`, `tests/CMakeLists.txt` — generated fixture, host pixel comparison, timing and edge checks.
- `tests/cmake/vector_api_policy.cmake` — refreshed Phase 3 temporary declaration baseline.
- `README.md` — current support limits and timing/API behavior.

## Decisions Made

- Kept the first cartridge geometry narrow: one PRG bank and one CHR-ROM bank.
- Used actual instance ticks as each frame’s next target so instruction overshoot remains in the shared timeline.
- Retired the Phase 2 declaration hash in favor of the Phase 3 API baseline, as required when Phase 3 appends public declarations.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing critical] Updated the temporary public API declaration guard**
- **Found during:** Task 1
- **Issue:** The Phase 2 guard rejected the planned public cartridge API addition.
- **Fix:** Rebased its normalized declaration hash to the Phase 3 header while preserving the mutation self-test.
- **Files modified:** `tests/cmake/vector_api_policy.cmake`
- **Verification:** Both `vectors.api_policy` and `vectors.api_policy.self_test` passed in the full workflow.
- **Committed in:** `019df58`

**2. [Rule 1 - Bug] Reduced instance footprint after allocator regression**
- **Found during:** Task 1
- **Issue:** An embedded frame buffer made an existing fixed-capacity allocator test fail.
- **Fix:** Rendered directly to the caller’s pitched video buffer.
- **Files modified:** `src/internal.h`, `src/bus.c`, `src/frame.c`
- **Verification:** `core.api` and the focused libretro integration test passed.
- **Committed in:** `019df58`

**Total deviations:** 2 auto-fixed (Rule 1: 1, Rule 2: 1)
**Impact on plan:** Both changes were required for the public API and existing allocator contract.

## Issues Encountered

- `cmake --workflow --preset ci` passed 330/331 tests on the final run. The only failure was the already-recorded local `retroarch.testframe` abort: RetroArch exited with `Subprocess aborted` and empty stdout/stderr. All other tests passed, including the new generated-content seam and Phase 2 API/vector checks. The focused `libretro.host` test passed.

## Known Limitations

- This tracer supports only the generated one-bank mapper-0 geometry and background tile output. Sprite rendering, CHR RAM, broad iNES/NES 2.0 validation, and audio remain for later Phase 3/4 work.

## Next Phase Readiness

- The shared load-to-frame path is in place for the next plan to extract cartridge and PPU helpers into dedicated modules without changing the public API or instance state model.

---
*Phase: 03-a-real-game-in-retroarch*
*Completed: 2026-10-08*

## Self-Check: PASSED

- All 12 key files exist.
- All four task commits are ancestors of the current plan head.
- `gsd_run check evaluation-scope --plan 03-01 --commits-only --raw` found four commits on this branch.
- Focused `libretro.host` test passed; full CI had only the known local `retroarch.testframe` GUI abort documented above.
