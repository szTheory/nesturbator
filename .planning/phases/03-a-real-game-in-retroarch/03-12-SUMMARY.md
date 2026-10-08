---
phase: 03-a-real-game-in-retroarch
plan: 12
subsystem: display-palette
tags: [NTSC, sRGB, deterministic]
requires:
  - phase: 03-06
    provides: Native game frame-hash baselines
provides:
  - Reproducible offline NTSC-to-sRGB display palette
  - Calibrated host display sample expectations
affects: [display-conversion, retroarch-frame-comparison]
tech-stack:
  added: []
  patterns:
    - Decode NTSC to 525-line BT.601, convert linear primaries to sRGB/D65 offline
key-files:
  created: []
  modified:
    - tools/palgen/palgen.c
    - src/palette_ntsc.c
    - tests/core/test_palette.c
    - tests/CMakeLists.txt
    - tests/libretro/libretro_host.c
    - README.md
    - include/nesturbator.h
decisions:
  - "Use the 525-line BT.601 2.4 transfer and primaries, then sRGB/D65 encoding with clipping and nearest 8-bit rounding."
  - "Keep native 16-bit frame pixels as the canonical hash input; display conversion remains outside runtime emulation state."
metrics:
  duration: 12min
  completed: 2026-10-08
  status: complete
  commits: 2
  plan_head_before: 85d4844f6ea70677cd88f3e79eade2c4f14c7e1d
  plan_head_after: 0bdf6a32a400bc873286094900da04f0f08f49be
actuals:
  tokens: 5614
  tasks: 1
  commits: 2
---

# Phase 03 Plan 12: Calibrated NTSC Display Palette

**The offline generator now converts its NTSC decode into a documented, reproducible sRGB table while preserving all native frame-hash baselines.**

## Accomplishments

- Added 525-line BT.601 transfer and primary conversion followed by the sRGB/D65 transfer curve, clipping and 8-bit rounding to the existing offline `palgen` tool.
- Regenerated all 512 XRGB8888 entries and documented the signal assumptions and regeneration command in the generated file, README and public header.
- Added fixed palette samples for grayscale, black, white, emphasis and table boundaries. Updated runner and libretro expected display colors to the calibrated values.
- Confirmed the three pinned games' native boot and scripted movie hashes still match the Plan 03-06 inventory byte for byte.

## TDD Gate Compliance

- **RED:** Added calibrated color assertions first. `core.palette` failed on the four intended values against the old table. The CTest JUnit report was checked by `gsd_run check tdd-red-evidence`, which returned `RED_EVIDENCE_OK`; semantic inspection confirmed the target test executed and failed at those assertions. The temporary report record was removed after classification because it contained the machine's absolute path, which repository hygiene forbids.
- **GREEN:** Implemented the conversion and refreshed affected display expectations. `core.palette`, `palette.regen`, runner hash inventory and libretro host checks passed. Commit: `0bdf6a3`.
- No separate refactor was needed.

## Verification

- `ctest --test-dir build/ci -R '^(runner.write_hashes|runner.write_hashes.content|core.palette|palette.regen)$' --output-on-failure` — 4/4 passed.
- `cmake --workflow --preset ci` — 338/339 passed. The only failure was `retroarch.testframe`, which aborted with empty stdout and stderr. This is the known local macOS GUI-session failure carried forward in STATE.md; the workflow is not fully green. `runner.dump` and dependent `libretro.host` passed after updating their expected calibrated colors.
- `scripts/hygiene.sh --tree` and `git diff --check` passed.

## Deviations from Plan

1. **[Rule 1 - Test baseline update]** The existing runner and libretro host tests pinned uncalibrated display colors. Updated those expected colors so both paths validate the new display table. Native frame hashes remain unchanged.
2. **[Rule 1 - Stale invariant]** The old palette test required green never to rise under emphasis. The BT.601-to-sRGB primary transform raises green for two blue-emphasis entries, so the stale display-space invariant was replaced with explicit calibrated emphasis samples.

## Task Commits

- `3ad2997` test(03-12): pin NTSC-to-sRGB palette samples
- `0bdf6a3` feat(03-12): calibrate NTSC palette for sRGB

## Self-Check: PASSED

- The palette generator, generated table, tests, README, public header and this summary exist.
- Both task commits (`3ad2997`, `0bdf6a3`) are ancestors of HEAD.
- `scripts/hygiene.sh --tree` and `git diff --check` passed.

*Phase: 03-a-real-game-in-retroarch · Completed: 2026-10-08*
