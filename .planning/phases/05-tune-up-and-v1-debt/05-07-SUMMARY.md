---
phase: 05-tune-up-and-v1-debt
plan: 07
subsystem: libretro
tags: [libretro, soft-reset, readme, tests]
requires:
  - phase: 05-tune-up-and-v1-debt
    provides: nesturbator_reset CPU, PPU and APU halves (05-05, 05-06); ines.h builder (05-02)
provides:
  - retro_reset runs nesturbator_reset
  - libretro.host reset frame parity and test-card reset checks
  - README status and RetroArch text for v1 and Phase 5
affects: [phase 5 verification]
actuals:
  tokens: 4500
  tasks: 2
  commits: 2
plan_head_before: e17316c4ec3b21159bd1ed3ec4a0f582e27bcce6
plan_head_after: 0ff0fbc97cfb6b3cb3db519e72cab823883f9cf5
commits: 2
tech-stack:
  added: []
  patterns: ["reset handler counting resets in surviving RAM to prove a reset ran"]
key-files:
  created: []
  modified: [libretro/libretro.c, tests/libretro/libretro_host.c, README.md]
key-decisions:
  - "retro_reset only calls nesturbator_reset; no unload or reload path (PROH-05-07-01)"
requirements-completed: [TUNE-06, TUNE-02]
coverage:
  - id: D1
    description: "RetroArch Reset runs the core's soft reset; frames after it equal a direct-API instance's"
    requirement: TUNE-06
    verification:
      - kind: integration
        ref: "ctest --preset ci -R ^libretro\\.host$ (check_reset_frame_parity, check_test_card_reset)"
        status: pass
    human_judgment: false
  - id: D2
    description: "README status line and RetroArch text describe the released code"
    requirement: TUNE-02
    verification:
      - kind: other
        ref: "cmake --workflow --preset hygiene; ctest -R release.one_version_per_line"
        status: pass
    human_judgment: true
    rationale: "Prose accuracy is not asserted by a test"
duration: 15min
completed: 2026-10-10
status: complete
---

# Phase 5 Plan 07: libretro soft reset Summary

**retro_reset now calls nesturbator_reset, and libretro.host proves post-reset frames match a direct-API instance using a reset handler that counts resets in surviving RAM.**

## Accomplishments
- `retro_reset` calls `nesturbator_reset(inst)` when an instance exists; no unload or recreate path.
- `check_reset_frame_parity`: trainer NROM image, reset handler follows the PPU power-up init (two vblank waits past the write-ignore window); three post-reset frames equal between module and direct instance, pixel (0,0) differs from pre-reset, RAM `$10` is one higher after the reset.
- `check_test_card_reset`: with no game, reset leaves the next frame unchanged.
- README status line now names v1 shipped and Phase 5; the libretro section describes RetroArch's Reset.

## Task Commits
1. **Task 1: retro_reset and parity checks** - `97d73ae` (feat)
2. **Task 2: README** - `0ff0fbc` (docs)

## Files Created/Modified
- `libretro/libretro.c` - retro_reset body and comment
- `tests/libretro/libretro_host.c` - two new checks, called from main
- `README.md` - status paragraph, libretro core reset sentence

## Decisions Made
None beyond the plan. Emptying retro_reset's body made `libretro.host` fail (mutation check), then reverted.

## Deviations from Plan

None - plan executed exactly as written. The README's "Install in RetroArch" section lists no frontend actions, so only "The libretro core" got the reset sentence.

## Issues Encountered
None. `nofp` preset not run: it fails `abi.undefined_symbols` on the base commit (known, out of scope). ci, hygiene and asan pass.

## Next Phase Readiness
Last plan of Phase 5; ready for phase verification. On the PR, the six CI legs and `retroarch-e2e` remain to be seen.

## Self-Check: PASSED
