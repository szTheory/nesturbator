---
phase: 03-a-real-game-in-retroarch
plan: 04
subsystem: ppu
tags: [ppu, sprites, oam, dma, timing, c17]
requires:
  - phase: 03-03
    provides: Per-dot background rendering, native caller-owned pixels, mapper-0 mirroring, and PPU registers
provides:
  - Secondary-OAM sprite evaluation and background/sprite pixel composition
  - Sprite priority, palette, flips, 8x16 selection, clipping, sprite-zero hit, and overflow boundaries
  - CPU-bus OAM DMA with page reads, OAMADDR wrap, parity timing, and continued PPU advancement
affects: [ppu, bus, frame-hashes, accuracycoin]
actuals:
  tokens: 4926
  tasks: 2
  commits: 10
tech-stack:
  added: []
  patterns:
    - Per-dot secondary OAM evaluation feeds next-scanline sprite pattern state
    - DMA source bytes use the regular CPU bus read path and destination writes use OAM register semantics
key-files:
  created: [tests/ppu/test_sprites.c]
  modified: [src/ppu.c, src/bus.c, src/internal.h, tests/CMakeLists.txt, README.md, include/nesturbator.h]
key-decisions:
  - "Keep sprite and DMA state instance-owned and integer-only."
  - "Continue PPU catch-up through every DMA CPU cycle so frame-edge timing remains on the shared tick cursor."
patterns-established:
  - "Sprite pixels are composed into the native frame buffer while visible PPU dots advance."
  - "DMA OAM writes reuse the PPU's $2004 increment/wrap behavior."
requirements-completed: [GAME-02, GAME-04]
coverage:
  - id: D1
    description: "Evaluated sprites compose with background pixels and set sprite status at tested boundaries."
    requirement: GAME-02
    verification:
      - kind: unit
        ref: tests/ppu/test_sprites.c#sprite pixel, priority, flip, clipping, zero-hit, and overflow cases
        status: pass
    human_judgment: false
  - id: D2
    description: "$4014 DMA copies the selected CPU page into wrapping OAM with parity-dependent cycle cost while PPU timing continues."
    requirement: GAME-04
    verification:
      - kind: unit
        ref: tests/ppu/test_sprites.c#OAM DMA byte, cycle, wraparound, and vblank cases
        status: pass
    human_judgment: false
metrics:
  duration: 19min
  completed: 2026-10-08
  status: complete
  commits: 10
  plan_head_before: 4dc047bead0a802134753eee412ed341f851e817
  plan_head_after: 8634abe2150448f03bb57889e72e1c6c428bfec3
---

# Phase 03 Plan 04: Sprite Composition and OAM DMA Summary

**Dot-evaluated sprites now compose over backgrounds, while `$4014` transfers 256 bytes through the CPU bus with measured 513/514-cycle parity and live PPU time.**

## Performance

- **Duration:** 19 min
- **Started:** 2026-10-08T17:33:00Z
- **Completed:** 2026-10-08T17:52:20Z
- **Tasks:** 2
- **Files modified:** 7

## Accomplishments

- Added secondary OAM evaluation, pattern fetching, and native-pixel sprite composition with priority, palette selection, horizontal/vertical flips, and 8x8/8x16 sprites.
- Covered eight versus nine in-range sprites, x=0/8/255, left-edge clipping, sprite-zero hit exclusion at x=255, and adjacent scanline output.
- Routed OAM DMA reads through the ordinary CPU bus and writes through `$2004`; checked both parity lengths, all byte positions, OAMADDR wrap, and vblank while stalled.

## Task Commits

1. **Task 1: Render evaluated sprites against backgrounds** — RED `902695f`; GREEN `e894671`.
2. **Task 2: Route CPU OAM DMA into the same rendering state** — RED `3990270`; GREEN `6156f21`.
3. **Task 2 supplemental edge coverage** — `f730000`, `669ca64`.
4. **Task 2 bounds safety correction** — `fda49bb`.
5. **Task 2 scheduling follow-up** — RED `345400b`; GREEN `8634abe`.

**Plan metadata:** `1c4e6d5` (initial summary/state close-out commit).

## TDD Gate Compliance

- **Task 1 RED:** `ctest --test-dir build/ci -R '^ppu\.sprites$' --output-on-failure --output-junit ...` exited 8. The target executed and failed its planned sprite-pixel, sprite-zero, and overflow assertions; observed pixel was 15 instead of 42 and both status bits were clear. `gsd_run check tdd-red-evidence` returned `RED_EVIDENCE_OK` (`build/ci/ppu-sprites-red.json`). GREEN passed the focused test and the full CI workflow apart from the known GUI abort.
- **Task 2 RED:** The same target exited 8 on the planned DMA content and cycle assertions; only the `$4014` write cycle elapsed and OAM remained unchanged. The classifier returned `RED_EVIDENCE_OK` (`build/ci/ppu-dma-red.json`). GREEN passed both parity cases and the focused sprite/DMA suite.
- **Scheduling follow-up RED:** The target exited 8 because the current implementation performed the transfer inside the `$4014` write, before the next CPU read. The classifier returned `RED_EVIDENCE_OK` (`build/ci/ppu-scheduling-red.json`). The GREEN correction queues the page and starts halt/alignment on the next bus read; the focused suite passes.
- No REFACTOR phase was needed.

## Files Created/Modified

- `src/ppu.c` — sprite evaluation, pattern selection, composition, hit/overflow flags, and primary-OAM bounds guard.
- `src/bus.c` — halt/alignment and 256 CPU-read/OAM-write DMA cycles.
- `src/internal.h` — instance-owned secondary OAM and per-sprite fetch state.
- `tests/ppu/test_sprites.c`, `tests/CMakeLists.txt` — synthetic sprite and DMA coverage.
- `README.md`, `include/nesturbator.h` — sprite output and DMA behavior documentation.

## Decisions Made

- Kept all new mutable state inside the PPU instance.
- Used one CPU cycle for every DMA read/write and retained the existing bus-to-PPU catch-up path across halt, alignment, transfer, and frame edges.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Added the standard declaration for `memset`**
- **Found during:** Task 1 GREEN
- **Issue:** The new secondary-OAM initialization failed the warnings-as-errors build because `string.h` was not included.
- **Fix:** Included `<string.h>` in `src/ppu.c`.
- **Verification:** Sprite target and full workflow built successfully.
- **Committed in:** `e894671`.

**2. [Rule 1 - Bug] Bounded primary-OAM evaluation**
- **Found during:** Task 2 supplemental review
- **Issue:** Dot evaluation could advance beyond sprite 63 and read beyond the 256-byte OAM array.
- **Fix:** Stop candidate reads once all 64 primary OAM entries have been evaluated.
- **Verification:** Focused suite and full workflow passed all non-GUI tests.
- **Committed in:** `fda49bb`.

**3. [Rule 1 - Bug] Start OAM DMA at the following CPU read**
- **Found during:** Task 2 integration review
- **Issue:** Starting DMA inside every `$4014` write halts before subsequent writes in read-modify-write instructions have completed.
- **Fix:** Queue the selected page and perform halt/alignment only when the next CPU read arrives, repeating that read through normal bus decoding.
- **Verification:** Test-first evidence confirmed the early transfer; focused tests and the full workflow passed after the fix.
- **Committed in:** `8634abe` (preceded by RED commit `345400b`).

**Total deviations:** 3 auto-fixed (one blocking build issue, two correctness/safety bugs).
**Impact on plan:** Both corrections were required for safe execution; no dependencies or architectural changes were introduced.

## Issues Encountered

- `cmake --workflow --preset ci` was run after both behavior tasks and after the scheduling correction. Each final run built successfully and passed 336/337 tests. The only failure was the previously observed local `retroarch.testframe`: RetroArch exited with `Subprocess aborted`, with empty stdout and stderr. No additional failing tests appeared.
- Focused `ppu.sprites` passed after each task's final changes, including both DMA cycle parities and the 8/9-sprite boundary.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

The PPU now produces background/sprite native pixels and OAM DMA advances shared CPU/PPU time. DMC DMA remains out of scope for Phase 4 as planned.

## Self-Check: PASSED

- `tests/ppu/test_sprites.c` exists and is registered as `ppu.sprites`.
- All nine task commits are present from `plan_head_before` through `plan_head_after`.
- Modified files contain no UI-fed placeholder or unfinished TODO/FIXME stubs.

---
*Phase: 03-a-real-game-in-retroarch*
*Completed: 2026-10-08*
