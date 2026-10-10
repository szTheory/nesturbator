---
phase: 03-a-real-game-in-retroarch
plan: 14
subsystem: video
tags: [PPU, sprites, deterministic hashes, libretro]
requires:
  - phase: 03-04
    provides: Dot-timed PPU and the native framebuffer contract.
  - phase: 03-08
    provides: Hosted cross-platform hash equality and RetroArch screenshot gates.
provides:
  - Pre-render preparation and visible row-zero output for Y=$FF sprites.
  - Full-frame even/odd synthetic sprite boundary regressions.
  - Regenerated deterministic game and movie frame hashes.
affects: [GAME-02, GAME-04, GAME-06, PPU timing, video output]
actuals:
  tokens: 4745
  tasks: 2
  commits: 4
  plan_head_before: eca75256a0b0d87d693bcd3b13e5fc3f3dcebf64
  plan_head_after: 69b9f07c9f1641311f5590a2cf5f2446b00c25d8
tech-stack:
  added: []
  patterns:
    - Keep next-scanline sprite evaluation state separate from the sprite slots composing the current row.
    - Start instances on pre-render so the first delivered frame has prepared row-zero state.
key-files:
  created: []
  modified:
    - src/ppu.c
    - src/instance.c
    - src/internal.h
    - tests/ppu/test_sprites.c
    - tests/ppu/test_render.c
    - tests/core/test_api.c
    - tests/cmake/vector_api_policy.cmake
    - tests/libretro/libretro_host.c
    - tests/runner/hashes.txt
    - README.md
    - include/nesturbator.h
key-decisions:
  - "Prepare Y=$FF only for target scanline zero; ordinary visible scanlines keep the prior non-wrapping range check."
  - "Keep pre-render sprite-overflow status clear while staging row-zero sprite pixels; visible-line overflow remains checked by ppu.sprites and AccuracyCoin."
  - "Advance NESTURBATOR_BEHAVIOUR_REVISION to 4 because native frame pixels changed."
requirements-completed: [GAME-02, GAME-04, GAME-06]
coverage:
  - id: D1
    description: Y=$FF sprites render row zero after pre-render on even and odd frames, while Y=$00 begins on row one.
    requirement: GAME-02
    verification:
      - kind: unit
        ref: "ctest --preset ci -R '^ppu\\.sprites$' --output-on-failure"
        status: pass
    human_judgment: false
  - id: D2
    description: Generated native game and movie frame hashes match the committed sorted inventory.
    requirement: GAME-06
    verification:
      - kind: integration
        ref: "runner.write_hashes.content in cmake --workflow --preset ci"
        status: pass
    human_judgment: false
  - id: D3
    description: AccuracyCoin, libretro host parity, and the full local CI workflow pass after the PPU update.
    requirement: GAME-04
    verification:
      - kind: integration
        ref: "cmake --workflow --preset ci: 356/356 passed; two local RetroArch tests skipped after the GUI session aborted before startup"
        status: pass
    human_judgment: false
metrics:
  duration: 11min
  completed: 2026-10-09
  status: complete
  commits: 4
  plan_head_before: eca75256a0b0d87d693bcd3b13e5fc3f3dcebf64
  plan_head_after: 69b9f07c9f1641311f5590a2cf5f2446b00c25d8
---

# Phase 03 Plan 14: Scanline-Zero Sprite Summary

**The PPU now prepares the first visible row before scanline 0, draws wrapped Y=$FF sprites into that row, and keeps the generated native-frame inventory synchronized.**

## Performance

- **Duration:** 11 min
- **Started:** 2026-10-09T13:50:32Z
- **Completed:** 2026-10-09T14:01:40Z
- **Tasks:** 2
- **Files modified:** 11

## Accomplishments

- Added a full clock-path sprite regression that crosses pre-render on even and odd frames, checks Y=$FF on row 0, confirms Y=$00 begins on row 1, and checks transparent pixels and the following row.
- Split next-line evaluation state from current-row sprite render slots so evaluation on scanline 0 cannot erase pixels that are still being composed on that row.
- Initialized instances on pre-render, rendered physical scanlines 0 through 239 into matching framebuffer rows, documented the native-video behavior, and advanced the behavior revision to 4.
- Regenerated the existing 36-row frame/audio inventory from `runner.write_hashes`; the keys and audio records are unchanged, and every changed frame digest came from the generator.

## TDD Gate Compliance

- **RED:** Commit `56a37a2` added the synthetic test before production changes. The rebuilt `ppu.sprites` target failed on the planned Y=$FF row-zero pixel assertion (`0` instead of `42`) on both parity paths. The JUnit evidence record was accepted by `gsd_run check tdd-red-evidence` as `RED_EVIDENCE_OK`; semantic assessment confirmed the intended assertion failed.
- **GREEN:** Commit `58fd898` implemented the first-row path and passed the focused sprite test. Follow-up commit `41d0807` separated current and next sprite slots, constrained the Y=$FF wrap to target row zero, and restored the AccuracyCoin page-17 register-open-bus check.
- **REFACTOR:** No behavior-neutral refactor was needed.

## Task Commits

1. **Task 1: Render the first visible sprite row from pre-render state** — `56a37a2` (RED test), `58fd898` (GREEN implementation), `41d0807` (pipeline and regression corrections).
2. **Task 2: Reconcile deterministic frame hashes and run the full automated gate** — `69b9f07` (generated hash inventory and libretro steady-state check).

The summary, state, roadmap, and requirements update are recorded in a separate plan-metadata commit after this file is written.

## Files Created/Modified

- `src/ppu.c`, `src/internal.h`, and `src/instance.c` — pre-render initialization, wrapped row-zero evaluation, separate next-row evaluation state, and physical framebuffer row numbering.
- `tests/ppu/test_sprites.c` and `tests/ppu/test_render.c` — pre-render parity, Y-coordinate, overflow, and physical row regressions.
- `tests/core/test_api.c` and `tests/cmake/vector_api_policy.cmake` — behavior revision and public declaration baseline.
- `tests/libretro/libretro_host.c` — compare stable frames after the synthetic cartridge’s first-frame nametable write.
- `tests/runner/hashes.txt` — generated native pixel hash inventory.
- `README.md` and `include/nesturbator.h` — row-zero sprite and caller-owned framebuffer contract.

## Decisions Made

- Only Y=$FF targeting physical scanline zero wraps to row zero. Later visible scanlines retain the existing bounded range calculation, so hidden Y=$FF OAM entries do not become visible or spuriously set overflow.
- Pre-render work prepares row-zero sprite pixels without setting sprite-overflow status; normal visible-line overflow behavior remains covered by synthetic and AccuracyCoin checks.
- New instances begin on pre-render so their first completed output frame consumes prepared scanline-zero state.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Kept row-zero sprites available through the final visible dots**
- **Found during:** Task 1, focused sprite regression.
- **Issue:** Reusing one sprite count for both evaluation and composition cleared the pre-render sprites at dot 65 of visible scanline 0; the regression caught the right edge disappearing at x=255.
- **Fix:** Added separate next-line count and sprite-zero markers, and transfer the evaluated slots at dot 257 after the current row is composed.
- **Files modified:** `src/ppu.c`, `src/internal.h`, `tests/ppu/test_sprites.c`.
- **Verification:** `ppu.sprites` passed through the full pre-render-to-row-1 transition.
- **Commit:** `41d0807`.

**2. [Rule 1 - Regression] Bounded Y=$FF wrap to the first visible row**
- **Found during:** Task 2, `accuracycoin.page17`.
- **Issue:** Applying 8-bit subtraction to every target row made Y=$FF filler entries appear in rows 1–7 and changed the PPU status seen by the register-open-bus test.
- **Fix:** Special-cased Y=$FF for target row 0 and kept overflow clear during pre-render row preparation; visible-line evaluation keeps the prior range behavior.
- **Files modified:** `src/ppu.c`, `tests/ppu/test_sprites.c`.
- **Verification:** `accuracycoin.page17` and `ppu.sprites` passed; no scoreboard rows were changed.
- **Commit:** `41d0807`.

**3. [Rule 2 - Missing first-frame preparation] Started new instances before visible scanline 0**
- **Found during:** Task 2, `libretro.host`.
- **Issue:** Instances began at scanline 0, so the first delivered frame had no pre-render-produced sprite state for row 0.
- **Fix:** Initialize the PPU scanline to 261; the host test now checks runner parity on the first output, then checks repeatability across two stable frames after the generated cartridge finishes its initial nametable write.
- **Files modified:** `src/instance.c`, `tests/libretro/libretro_host.c`.
- **Verification:** `libretro.host` passed.
- **Commit:** `41d0807` for instance initialization; `69b9f07` for the host regression.

**4. [Rule 3 - Blocking] Refreshed checks pinned to the public behavior revision**
- **Found during:** Task 2, full CI workflow.
- **Issue:** The API test and temporary declaration guard still expected behavior revision 3.
- **Fix:** Updated the expected version and declaration digest for revision 4.
- **Files modified:** `tests/core/test_api.c`, `tests/cmake/vector_api_policy.cmake`.
- **Verification:** Both `vectors.api_policy` tests and the full workflow passed.
- **Commit:** `58fd898`, `41d0807`.

---

**Total deviations:** 4 auto-fixed (2 Rule 1, 1 Rule 2, 1 Rule 3).
**Impact on plan:** These changes preserve first-frame correctness, prevent collateral row/overflow changes, and keep the behavior-revision guard aligned with the public header.

## Verification

- RED evidence: accepted by `gsd_run check tdd-red-evidence` with `RED_EVIDENCE_OK`; target `ppu.sprites` failed on the expected Y=$FF framebuffer row-zero assertion.
- `ctest --preset ci -R '^ppu\.sprites$' --output-on-failure`: passed.
- `ctest --preset ci -R '^(runner\.write_hashes|runner\.write_hashes\.content|ppu\.sprites|accuracycoin\.page2|accuracycoin\.page14|accuracycoin\.page17|libretro\.host)$' --output-on-failure`: passed 8/8.
- `cmake --workflow --preset ci`: configure, build, tests, and package passed; 356/356 tests passed. `retroarch.testframe` and `retroarch.game` skipped because RetroArch aborted before startup in the local GUI session.
- The generated frame inventory exactly matched the updated `tests/runner/hashes.txt`; audio rows and inventory keys were preserved.
- Hosted six-platform hash equality and required RetroArch screenshot evidence remain on the existing CI workflow; no hosted run for this unpushed exact commit was observed.

## Issues Encountered

- The first full workflow caught the test target assumptions, API revision guard, transient first-frame cartridge setup, and Y=$FF filler range behavior. The fixes are recorded above; the final workflow passed.
- Local RetroArch GUI startup aborted before the tests could launch the released binary. The existing hosted `retroarch-e2e` gate remains responsible for the screenshot comparison.

## User Setup Required

None.

## Next Phase Readiness

- The structured Phase 03 sprite-row gap is closed for the local automated suite. The required hosted six-platform and RetroArch screenshot jobs should report against the exact PR commit when it is submitted to hosted CI.

## Self-Check: PASSED

- Summary file exists at the required phase path.
- Task commits `56a37a2`, `58fd898`, `41d0807`, and `69b9f07` are ancestors of the current branch head.
- The measured plan ledger records 4 task commits from `eca75256a0b0d87d693bcd3b13e5fc3f3dcebf64` through `69b9f07c9f1641311f5590a2cf5f2446b00c25d8`.

---
*Phase: 03-a-real-game-in-retroarch*
*Completed: 2026-10-09*
