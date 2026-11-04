---
phase: 25-region-selection-and-pal-timing
plan: "01"
subsystem: emulation
tags: [region-selection, NES-2.0, PAL, C17, CMake]
requires: []
provides:
  - Shared per-load region selection and effective-region query
  - Atomic metadata/preference resolution with NTSC-compatible Auto behavior
  - Pinned source-derived PAL row and APU edge dispositions
affects:
  - Phase 25 Plans 02–05
tech-stack:
  added: []
  patterns:
    - Region selection is resolved into local cartridge layout and committed only after successful replacement.
    - PAL source-derived expectations retain their units, origins, assumptions, and evidence limits.
key-files:
  created:
    - .planning/phases/25-region-selection-and-pal-timing/25-PAL-EVIDENCE.md
  modified:
    - include/nesturbator.h
    - src/internal.h
    - src/cartridge.c
    - tests/core/test_cartridge.c
    - tests/core/test_api.c
    - tests/cmake/vector_api_policy.cmake
    - README.md
    - docs/api.md
key-decisions:
  - "Explicit single-region NES 2.0 metadata is authoritative; preferences choose only ambiguous NES 2.0 and legacy iNES content, and Dendy is refused."
  - "Keep the old load API as Auto and expose effective region through a separate query, preserving ABI 1 and the existing info-struct prefix."
  - "Keep behavior revision 15 with native state format 6 and no PAL state-continuation claim before Phase 26."
  - "Represent PAL output in physical rows 0–239 and use the documented APU GET/PUT-to-CPU cursor conversion as a source-derived model."
requirements-completed: [REG-01, PAL-01]
coverage:
  - id: D1
    description: Load-time region selection, metadata precedence, refusal, atomic replacement, and query lifecycle.
    requirement: REG-01
    verification:
      - kind: unit
        ref: tests/core/test_cartridge.c and tests/core/test_api.c; core.cartridge and core.api
        status: pass
    human_judgment: false
  - id: D2
    description: Public API compatibility, C/C++ header compilation, and declaration-policy guard.
    requirement: REG-01
    verification:
      - kind: unit
        ref: header.c, header.cxx, vectors.api_policy, vectors.api_policy.self_test
        status: pass
    human_judgment: false
  - id: D3
    description: PAL picture-row and APU event cursor dispositions are pinned and source-reviewed with explicit assumptions and limits.
    requirement: PAL-01
    verification:
      - kind: other
        ref: .planning/phases/25-region-selection-and-pal-timing/25-PAL-EVIDENCE.md; bounded source review
        status: pass
    human_judgment: false
duration: 12min
completed: 2026-11-01
status: complete
actuals:
  tokens: 8467
  tasks: 3
  commits: 5
commits: 5
plan_head_before: 4ef3a97951f44c2ccc0e13c4b37f660edc6608c4
plan_head_after: a98dfdca5f52e86662ca6796df2ccc97b5991729
---

# Phase 25 Plan 01: Region Selection and PAL Evidence Summary

**A transactional load-time region API preserves legacy Auto behavior and establishes reviewable, source-derived PAL raster and APU timing expectations.**

## Performance

- **Duration:** 11 minutes 26 seconds
- **Started:** 2026-11-01T22:45:50Z
- **Completed:** 2026-11-01T22:57:16Z
- **Tasks:** 3
- **Files modified:** 9 plan deliverables

## Accomplishments

- Added `nesturbator_load_cartridge_region()` and `nesturbator_get_region()`. Explicit NES 2.0 NTSC/PAL metadata takes precedence, preferences select ambiguous or legacy images, and Dendy is refused. A failed replacement leaves the active cartridge and its effective region unchanged. The old load function remains Auto.
- Added resolver, lifecycle, instance-isolation, allocator-failure and live-replacement tests, plus API-prefix, canary, C/C++ declaration, and declaration-policy checks. The NTSC tracer compares the new API with the old API across a complete loop-guest frame.
- Preserved ABI 1 and native format 6; behavior revision is now 15. README and API documentation state the load-time contract and PAL evidence limits.
- Created [25-PAL-EVIDENCE.md](25-PAL-EVIDENCE.md) with pinned source facts, explicit derivations, and independent row-marker/APU event fixture specifications.

## Build Identity and Verification

- **Source:** branch `gsd/phase-25-pal-region-support`, task-complete commit `a98dfdc` (full source: `a98dfdca5f52e86662ca6796df2ccc97b5991729`).
- **Build:** CMake 4.4.3, Ninja, Debug, `dev` preset.
- **Focused checks:** `cmake --preset dev`; build targets `core.cartridge`, `core.api`, `header.c`, and `header.cxx`; focused CTest selection for `core.cartridge`, `core.api`, `header.c`, `header.cxx`, `docs.links`, `vectors.api_policy`, and `vectors.api_policy.self_test` — all seven passed.
- The plan's source review independently checked the pinned PAL video revision 23684, BreakingNESWiki PAL PPU notes at `5a55bac`, and APU Frame Counter revision 23448. No emulator output was used as an oracle.
- Full CI and AccuracyCoin were not run; the existing campaign stop remains in force.

## PAL Evidence Dispositions

- **Picture rows:** retain physical native rows 0–239 in the fixed 256×240 buffer. Source evidence supports active generated pixels on physical row 239; the 239-line visible picture is interpreted as border coverage of row 0, not a reason to shift or discard native rows. The owned fixture uses distinct markers on rows 0, 1, 238 and 239, horizontal guards, rendering-on/off cases, and output canaries.
- **APU events:** preserve the source count origin and GET/PUT phase. Four-step quarter clocks are at CPU cursors 8313/16627/24939/33253, half clocks at 16627/33253, enabled IRQ edges at 33252/33253/33254, with wrap at 33254. Five-step's final quarter/half clock is 41565 and wrap is 41566. Fixtures derive expectations from the pinned source table and test adjacent `$4015`/`$4017` behavior.
- **Arithmetic:** 32 ticks per PAL CPU cycle; M2 edges at +13/+32 ticks; 10 ticks per PPU dot; 312×341 dots or 1,063,920 ticks per frame; 53,203,425 ticks/second; 322445/6448 frames/second; 640/709379 samples/tick at 48 kHz; 959 or 960 samples per frame with a carried remainder.

These are source-derived model dispositions, not direct console or display measurements. In particular, the cited APU source explicitly assumes the PAL `$4017` 3/4-cycle delay matches its described behavior; no PAL subcycle race is claimed.

## Task Commits

1. **Task 1: NTSC region-selection tracer** — `427ceef` (RED test), `3523446` (implementation).
2. **Task 2: Public API compatibility** — `8a340f7` (RED test), `cdc6224` (implementation and documentation).
3. **Task 3: PAL source evidence review** — `a98dfdc` (evidence record).

## Files Created/Modified

- `include/nesturbator.h` — public region/preference enums, load/query functions and compatibility comments.
- `src/internal.h`, `src/cartridge.c` — per-instance effective region and shared transactional resolver.
- `tests/core/test_cartridge.c`, `tests/core/test_api.c` — resolver matrix, replacement atomicity, tracer and API compatibility coverage.
- `tests/cmake/vector_api_policy.cmake` — approved public declaration pin for behavior revision 15.
- `README.md`, `docs/api.md` — load behavior, compatibility and PAL limits.
- `25-PAL-EVIDENCE.md` — pinned PAL evidence and owned fixture specifications.

## Decisions Made

- Use an additive region-aware load API and a separate query; do not enlarge `nesturbator_info` or alter the old loader's Auto semantics.
- Store the effective profile in cartridge instance state and publish it only after successful replacement.
- Keep native format 6 and ABI 1. PAL state continuation remains unsupported until Phase 26.
- Keep physical row order and the converted APU cursor table as independently reviewable source-derived expectations.

## Deviations from Plan

None. The evidence markdown's trailing whitespace was removed after the staged whitespace check identified it.

## Issues Encountered

- The looping guest's first completed-frame boundary reports 714,744 ticks, including a 12-tick instruction-boundary carryover. The tracer pins that observed whole-instruction result while comparing old and new Auto paths; no production timing behavior was changed.

## Known Stubs

None found in plan-created or plan-modified files.

## Retained Flags and Limits

- PAL dispositions are source-backed, not hardware proof. PAL color calibration, direct 2A07 DMA behavior and physical timestamp-tie ordering remain unverified.
- PAL `$4017` 3/4-cycle equivalence is an explicit source assumption, not a measured subcycle result.
- PAL state continuation remains assigned to Phase 26. NTSC-focused AccuracyCoin and the interrupted full-CI campaign were not run or restarted.
- The plan's flagged unresolved requirement probe and broader PAL adjacency/ordering questions remain within the boundaries assigned to Plans 02–05; this summary does not treat the evidence review as universal PAL accuracy.

## Next Phase Readiness

Plan 25-02 can use the recorded row and APU cursor dispositions for regional clock/raster work. Continue to preserve the stated source-versus-hardware boundary and the existing full-CI/AccuracyCoin stop.

---
*Phase: 25-region-selection-and-pal-timing*
*Completed: 2026-11-01*

## Self-Check: PASSED

The summary and PAL evidence files exist, and all five plan task commits are ancestors of the current branch head.
