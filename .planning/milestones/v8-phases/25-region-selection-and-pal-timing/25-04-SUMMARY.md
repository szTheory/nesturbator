---
phase: 25-region-selection-and-pal-timing
plan: "04"
subsystem: region-selection
tags: [NES, PAL, libretro, runner, C, CTest]
requires:
  - phase: 25-01
    provides: Shared region-aware load API and metadata precedence policy
  - phase: 25-03
    provides: Effective-region timing and AV information from the core
provides:
  - Runner Auto/NTSC/PAL preference parsing and shared-loader forwarding
  - Libretro load-latched region option with effective-region reporting
  - Owned runner/library/module parity and lifecycle coverage
affects: [Phase 25 Plan 05, libretro integration, runner]
actuals:
  tokens: 8865
  tasks: 2
  commits: 4
plan_head_before: f61ae63b61ba80fd0765385235ab9361d621883a
plan_head_after: a61f49e769dc9ae88f50c6aa32da49e7e6235e96
commits: 4
tech-stack:
  added: []
  patterns:
    - Frontends forward per-load preference; shared library remains the sole precedence policy
    - Libretro option changes are sampled only on content load
key-files:
  created: []
  modified:
    - runner/main.c
    - libretro/libretro.c
    - tests/CMakeLists.txt
    - tests/libretro/libretro_host.c
    - docs/runner.md
    - include/nesturbator.h
    - README.md
key-decisions:
  - "Keep all region precedence in the shared loader; runner and libretro forward preferences only."
  - "Read the libretro preference on non-NULL content load and keep the active profile fixed until reload."
  - "Report the loaded effective region through existing AV fraction conversion while retaining 256×240 geometry."
requirements-completed: [REG-01, PAL-01]
coverage:
  - id: D1
    description: Runner accepts Auto/NTSC/PAL, rejects malformed preferences before content loading, and preserves the no-content NTSC contract.
    requirement: REG-01
    verification:
      - kind: integration
        ref: "ctest runner region/usage/hash/advance/options and docs.links selection"
        status: pass
    human_judgment: false
  - id: D2
    description: Real libretro module, direct library and runner loads agree across explicit, ambiguous, legacy and refused-region cases; option changes apply on reload.
    requirement: REG-01
    verification:
      - kind: integration
        ref: "ctest --test-dir build/dev -R '^libretro\\.host$'"
        status: pass
    human_judgment: false
  - id: D3
    description: Active region reporting and existing AV rates follow the loaded profile while callback geometry remains 256×240.
    requirement: REG-01
    verification:
      - kind: integration
        ref: "libretro.host region, AV, geometry and sample-capacity assertions"
        status: pass
    human_judgment: false
duration: 9min
completed: 2026-11-01
status: complete
---

# Phase 25 Plan 04: Runner and Libretro Region Selection Summary

**Runner and libretro now forward load preferences through the shared region policy, with real module, library and CLI parity checks.**

## Performance

- **Duration:** 9 min
- **Started:** 2026-11-01T23:37:25Z
- **Completed:** 2026-11-01T23:46:15Z
- **Tasks:** 2
- **Files modified:** 7

## Accomplishments

- Added `--region auto|ntsc|pal` to the runner. Invalid, empty and duplicate values retain usage-error behavior; no-content output remains NTSC.
- Added a libretro `nesturbator_region` option with V2 registration and legacy fallback. The adapter samples it once per non-NULL load, defaults missing or unknown values to Auto, and reports the effective loaded region.
- Extended the owned host harness to compare the real module, direct library and runner across explicit NTSC/PAL, multi-region NES 2.0, legacy iNES and Dendy cases. It covers reload latching, refused replacement preservation, option API fallbacks, unload/reinit, 256×240 geometry and PAL callback capacity.

## Task Commits

1. **Task 1 RED:** `26ca6c4` — `test(25-04): add runner region preference cases`
2. **Task 1 GREEN:** `89710bb` — `feat(25-04): forward region preference from runner`
3. **Task 2 RED:** `7749548` — `test(25-04): add libretro region lifecycle matrix`
4. **Task 2 GREEN:** `a61f49e` — `feat(25-04): latch libretro region preference on load`

The four task commits are measured from the persisted plan ledger. The final GSD metadata commit is separate.

## Files Created/Modified

- `runner/main.c` — Parses one runner preference and forwards it to the shared loader.
- `libretro/libretro.c` — Registers and samples the core option and reports the active region.
- `tests/CMakeLists.txt`, `tests/libretro/libretro_host.c` — Adds runner boundary cases and real adapter lifecycle/parity coverage.
- `docs/runner.md`, `README.md`, `include/nesturbator.h` — Documents option behavior, load latching and evidence boundaries.

## Decisions Made

- Keep region precedence exclusively in the shared library loader.
- Sample the libretro option only while loading non-NULL content; changing the host preference or resetting does not mutate the running profile.
- Preserve the fixed 256×240 callback geometry and use the existing AV fraction conversion for the loaded profile.

## Verification

- RED evidence for both TDD tasks was accepted by `gsd-tools check tdd-red-evidence`.
- Task 1 focused runner and documentation selection passed all 11 selected tests.
- Task 2 `cmake --build --preset dev --target libretro.host` and `ctest --test-dir build/dev -R '^libretro\.host$' --output-on-failure --no-tests=error` passed. Its fixture closure ran `runner.dump` and `libretro.host` only.
- No full CI, AccuracyCoin run or stopped campaign was run.

The host PCM comparison establishes adapter parity only. Independent PAL timing and pitch evidence remains source-derived and belongs to the core verification records; PAL save/restore continuation remains Phase 26, and final palette/presentation and host AV acceptance remain Phase 27.

## Deviations from Plan

None — plan executed as written.

## Issues Encountered

Two initial host-test assertions exposed harness setup assumptions: the test needed to re-register callbacks after an earlier parity check cleared them, and the legacy definition should be asserted as registered even when the simulated frontend rejects that API. Both were corrected and the focused host test passed.

## User Setup Required

None — no external configuration is required.

## Next Phase Readiness

Plan 25-04 provides the adapter evidence Plan 25-05 needs before it advertises the capability metadata. The phase still has Plan 25-05 remaining; the interrupted campaign stop and publication boundaries remain in force.

## Self-Check: PASSED

The summary and all seven key files exist; all four task commits are reachable from the current plan head.

---
*Phase: 25-region-selection-and-pal-timing*
*Completed: 2026-11-01*
