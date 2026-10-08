---
phase: 03-a-real-game-in-retroarch
plan: 09
subsystem: emulator-core
tags: [c17, cartridge, mapper-0, ppu, cmake, ctest]

requires:
  - phase: 03-01
    provides: generated mapper-0 content-to-runner/libretro path and timing checks
provides:
  - dedicated cartridge ownership and mapper-0 access module
  - dedicated PPU register, memory, dot-clock, and native rendering module
affects: [03-02, 03-03, 03-10, 03-11, 03-12, 03-13]

actuals:
  tokens: 5182
  tasks: 2
  commits: 2
commits: 2
plan_head_before: c947372fde07faeac3879c8f257af5a146d53056
plan_head_after: 0268605eec4ccede1a916daa2a6b8484415a10cf

tech-stack:
  added: []
  patterns: [cartridge-owned content lifetime, PPU module behind CPU-bus scheduling]

key-files:
  created: [src/cartridge.c, src/ppu.c]
  modified: [CMakeLists.txt, src/instance.c, src/bus.c, src/internal.h, tests/CMakeLists.txt]

key-decisions:
  - "Keep CPU bus M2 scheduling in bus.c while PPU state transitions and register effects live in ppu.c."
  - "Preserve cartridge parsing, allocator ownership, and frame state reset behavior during extraction."

requirements-completed: [GAME-01, GAME-02]
coverage:
  - id: D1
    description: "The generated mapper-0 cartridge path remains byte-identical through the runner and libretro host after cartridge and PPU extraction."
    requirement: GAME-01
    verification:
      - kind: integration
        ref: "tests/libretro/libretro_host.c#libretro.host"
        status: pass
      - kind: other
        ref: "nesturbator-run --frames 1 --rom build/ci/tests/nrom-tracer.nes --hash-frame 1"
        status: pass
    human_judgment: false
  - id: D2
    description: "Frame timing, instruction overshoot, no-content output, and JAM stop behavior remain unchanged."
    requirement: GAME-02
    verification:
      - kind: integration
        ref: "tests/libretro/libretro_host.c#check_core_timing_and_jam"
        status: pass
      - kind: integration
        ref: "tests/libretro/libretro_host.c#check_ppu_frame_edges"
        status: pass
    human_judgment: false

duration: 7min
completed: 2026-10-08
status: complete
---

# Phase 03 Plan 09: Cartridge and PPU module extraction summary

**Cartridge lifetime and PPU register, clock, and rendering logic now live in dedicated C modules while the generated NROM host output and frame timing stay identical.**

## Performance

- **Duration:** 7 min
- **Started:** 2026-10-08T17:02:26Z
- **Completed:** 2026-10-08T17:09:15Z
- **Tasks:** 2
- **Files modified:** 7

## Accomplishments

- Moved cartridge load, unload, reset, and mapper-0 PRG reads from the instance and bus modules into `src/cartridge.c`.
- Moved PPU memory access, CPU-visible registers, dot clock, and native background rendering into `src/ppu.c`; bus scheduling still advances the same M2 edges.
- Registered both modules in the library and direct bus test build. The original and extracted generated ROM runs both reported frame 1 at 714744 ticks with SHA-256 `9c760217803a7ffc1c56d2a7777edca035d4194d6f486254f843c5e1e5a261ae`.

## Task Commits

1. **Task 1: Extract cartridge ownership and mapper-0 access without changing output** — `e17adc5` (`refactor`).
2. **Task 2: Extract PPU clock, registers and native output without changing time** — `0268605` (`refactor`).

## Files Created/Modified

- `src/cartridge.c` — owns cartridge allocation, load/unload reset, and PRG reads.
- `src/ppu.c` — owns PPU memory and register behavior, dot advancement, and native output.
- `src/instance.c` — retains instance creation/destruction and removes cartridge implementation.
- `src/bus.c` — retains CPU-cycle/M2 timing and delegates cartridge and PPU behavior.
- `src/internal.h`, `CMakeLists.txt`, `tests/CMakeLists.txt` — declare and register the extracted modules.

## Decisions Made

- Kept cycle timing and address decoding in `bus.c`; PPU register effects execute through internal PPU entry points after the same cycle advancement.
- Kept the existing mapper-0 geometry and allocator semantics unchanged.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Linked extracted modules into the direct bus test executable**
- **Found during:** Task 1 build
- **Issue:** `bus.unit` compiles `src/bus.c` directly, so the relocated cartridge reader had to be linked into that test target.
- **Fix:** Added `src/cartridge.c` to the direct bus test sources; Task 2 added `src/ppu.c` there as well.
- **Files modified:** `tests/CMakeLists.txt`
- **Verification:** `bus.unit` passed in focused checks and both full workflow runs.
- **Committed in:** `e17adc5` and `0268605`.

**Total deviations:** 1 auto-fixed (Rule 3: 1)
**Impact on plan:** Only build wiring needed adjustment to keep direct-source unit tests linked after extraction.

## Issues Encountered

- Both task-level `cmake --workflow --preset ci` runs passed 330/331 tests. The only failure was `retroarch.testframe`, which aborted with empty stdout/stderr; this is the same known local GUI failure recorded in `03-01-SUMMARY.md` and also reproduced before this plan's changes. `libretro.host`, `core.frame`, `bus.unit`, runner hash checks, and all other CI checks passed.
- The generated ROM frame reported 714744 ticks and SHA-256 `9c760217803a7ffc1c56d2a7777edca035d4194d6f486254f843c5e1e5a261ae` before and after the PPU extraction.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- The shared cartridge and PPU implementations now have the planned small-module boundaries. Existing host, frame, and PPU edge checks remain the regression baseline for subsequent Phase 3 work.

---
*Phase: 03-a-real-game-in-retroarch*
*Completed: 2026-10-08*

## Self-Check: PASSED

- Both task commits are ancestors of the current plan head.
- `src/cartridge.c` and `src/ppu.c` exist.
- `gsd_run check evaluation-scope --plan 03-09 --commits-only --raw` found both task commits on this branch.
- All non-GUI tests passed; the known local `retroarch.testframe` abort is recorded above.
