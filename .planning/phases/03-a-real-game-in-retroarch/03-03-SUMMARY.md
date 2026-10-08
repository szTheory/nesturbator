---
phase: 03-a-real-game-in-retroarch
plan: 03
subsystem: ppu
tags: [c17, ppu, ntsc, rendering, tdd]
requires:
  - phase: 03-09
    provides: dedicated PPU clock, register, memory and render module
  - phase: 03-02
    provides: mapper-0 cartridge header geometry and mirroring metadata
provides:
  - pre-vblank PPUSTATUS suppression and mapper-0 CIRAM mirroring
  - per-visible-dot native background pixels with scrolling, attributes, grayscale and emphasis
  - focused PPU register and synthetic pixel regression tests
affects: [03-04, 03-05, 03-06, 03-07, 03-08, 03-11, 03-12, 03-13]
actuals:
  tokens: 4785
  tasks: 2
  commits: 4
commits: 4
plan_head_before: 8781562beecf79bcde23b594f1d633bcdb9fba02
plan_head_after: 2e7c0206f5fda514f9ca28d3d1145e12cc3cede4
tech-stack:
  added: []
  patterns: [caller-owned framebuffer written at visible PPU dots, synthetic CHR and nametable test fixtures]
key-files:
  created: [tests/ppu/test_registers.c, tests/ppu/test_render.c]
  modified: [src/ppu.c, src/frame.c, src/internal.h, tests/CMakeLists.txt, include/nesturbator.h, README.md]
key-decisions:
  - "Keep frame pixels in the caller-owned buffer and write them as visible PPU dots advance, preserving the 64 KiB instance allocator contract."
  - "Select mapper-0 CIRAM mirroring from iNES header flags 6 bit 0."
requirements-completed: [GAME-02, GAME-04]
coverage:
  - id: D1
    description: "Register latch, palette aliases, horizontal/vertical nametable mirroring, and the pre-vblank status-read suppression window are covered by focused tests."
    requirement: GAME-04
    verification:
      - kind: unit
        ref: tests/ppu/test_registers.c#ppu.registers
        status: pass
    human_judgment: false
  - id: D2
    description: "Synthetic background output selects fine-scroll pattern bits and attribute palettes and preserves grayscale/emphasis in native pixels."
    requirement: GAME-04
    verification:
      - kind: unit
        ref: tests/ppu/test_render.c#ppu.render
        status: pass
    human_judgment: false
  - id: D3
    description: "The NROM runner and libretro host remain integrated with the dot-timed framebuffer path."
    requirement: GAME-02
    verification:
      - kind: integration
        ref: "libretro.host; runner NROM frame 1 (714744 ticks, hash d9c1d1af5a613b96af0db59ee7ffb4b1543f6ab3fa6a4d00ad6ba9bc5e12a1b3)"
        status: pass
    human_judgment: false
patterns-established:
  - "Visible PPU dots write native pixels directly to the caller's pitched frame buffer."
  - "Synthetic PPU tests use owned CHR, nametable, and cartridge-header bytes."
duration: 10min
completed: 2026-10-08
status: complete
---

# Phase 03 Plan 03: PPU register timing and scrolling background summary

**CPU-visible PPU timing now suppresses the pre-vblank NMI race, mapper-0 mirroring selects the correct CIRAM page, and visible dots emit native scrolling background pixels.**

## Performance

- **Duration:** 10 min
- **Started:** 2026-10-08T17:24:00Z
- **Completed:** 2026-10-08T17:34:51Z
- **Tasks:** 2
- **Files modified:** 8

## Accomplishments

- Added register tests for scroll/address latch behavior, palette aliases, horizontal and vertical nametable mirroring, and `$2002` reads immediately before and on vblank dot 1.
- Added the vblank suppression window and selected nametable CIRAM pages from mapper-0 header mirroring flags.
- Generated each visible background pixel as its PPU dot advances, using pattern bits, attribute quadrants, coarse/fine scroll, backdrop aliases, grayscale, and emphasis. The instance retains no full-frame allocation: PPU output is borrowed from the caller for the duration of `nesturbator_run_frame`.
- Updated README and public frame documentation. The generated NROM tracer now reports frame 1 at 714744 ticks with SHA-256 `d9c1d1af5a613b96af0db59ee7ffb4b1543f6ab3fa6a4d00ad6ba9bc5e12a1b3`; the prior hash changed because background pixel selection is now implemented.

## Task Commits

1. **Task 1 RED: PPU register boundary tests** — `ecd6999` (`test`).
2. **Task 1 GREEN: PPU timing and mirroring** — `5353fb5` (`feat`).
3. **Task 2 RED: synthetic PPU render tests** — `930c06f` (`test`).
4. **Task 2 GREEN: scrolling background renderer** — `2e7c020` (`feat`).

## Files Created/Modified

- `src/ppu.c` — register boundaries, nametable mirroring, visible-dot background sampling and native pixel encoding.
- `src/frame.c`, `src/internal.h` — route the caller's pitched frame buffer through PPU dot advancement without enlarging the instance with a framebuffer.
- `tests/ppu/test_registers.c`, `tests/ppu/test_render.c` — owned synthetic register, timing and pixel fixtures.
- `tests/CMakeLists.txt` — register both focused PPU targets.
- `README.md`, `include/nesturbator.h` — document timing and native background pixel behavior.

## Decisions Made

- Kept the framebuffer in caller-owned memory and borrowed it only while advancing a frame. An initial embedded 256×240 buffer exceeded the repository's 64 KiB allocator fixture; `core.api` exposed the regression, and direct dot output avoids the extra allocation while keeping instance memory bounded.
- Used iNES flags 6 bit 0 to select the two supported mapper-0 CIRAM layouts.

## TDD Gate Compliance

- **Task 1 RED:** `ppu.registers` failed on the intended missing pre-vblank suppression assertions. `gsd_run check tdd-red-evidence` returned `RED_EVIDENCE_OK`; report and record are in ignored `build/ci/ppu-registers-red.xml` and `.json`.
- **Task 1 GREEN:** The register and timing target passed, along with the full workflow except the known local RetroArch GUI abort.
- **Task 2 RED:** `ppu.render` failed on the intended fine-scroll, attribute, grayscale, and emphasis pixel assertions. The classifier returned `RED_EVIDENCE_OK`; report and record are in ignored `build/ci/ppu-render-red.xml` and `.json`.
- **Task 2 GREEN:** Both focused PPU targets and `core.api` passed; the final workflow passed 335/336 tests.
- **REFACTOR:** None required.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Kept rendered pixels out of the instance allocation**
- **Found during:** Task 2 implementation
- **Issue:** Embedding a 256×240 native framebuffer made the instance larger than the existing fixed 64 KiB allocator fixture, causing `core.api` to fail.
- **Fix:** Write visible pixels to the caller-owned output buffer during PPU dot advancement and clear the temporary pointer before returning from the frame call.
- **Files modified:** `src/frame.c`, `src/internal.h`, `src/ppu.c`
- **Verification:** `core.api`, both PPU tests, and final CI passed.
- **Commit:** `2e7c020`.

**2. [Rule 3 - Blocking] Corrected the native pixel test fixture timing**
- **Found during:** Task 2 GREEN
- **Issue:** The synthetic test initially advanced dots on pre-render scanline 0, so it did not exercise visible output.
- **Fix:** Start the fixture on visible scanline 1 and advance exactly two dots.
- **Files modified:** `tests/ppu/test_render.c`
- **Verification:** `ppu.render` passed.
- **Commit:** `2e7c020`.

**Total deviations:** 2 auto-fixed (one instance-size regression and one test fixture setup correction).
**Impact on plan:** The framebuffer stays bounded and is emitted during dot scheduling; no dependency or public API layout change was needed.

## Issues Encountered

- `cmake --workflow --preset ci` after Task 1: 334/335 passed. After Task 2's initial embedded framebuffer attempt: 335/336 passed with a new `core.api` failure plus the known GUI abort. After the caller-buffer correction: 335/336 passed; only `retroarch.testframe` aborted with empty stdout/stderr (`Subprocess aborted`), matching the known local macOS GUI-session issue.
- The synthetic NROM runner hash changed as expected when PPU background rendering began using scroll and attribute state. Frame timing remained 714744 ticks.

---
*Phase: 03-a-real-game-in-retroarch*
*Completed: 2026-10-08*

## Self-Check: PASSED

- Task commits `ecd6999`, `5353fb5`, `930c06f`, and `2e7c020` are present on the current plan branch.
- Both focused PPU test executables are registered, and final full-workflow evidence is 335/336 passing with only the known `retroarch.testframe` local abort.
