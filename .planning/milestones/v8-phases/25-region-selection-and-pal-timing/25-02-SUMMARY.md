---
phase: 25-region-selection-and-pal-timing
plan: "02"
subsystem: timing
tags: [C, NES, PAL, deterministic-audio, rational-sampling]

# Dependency graph
requires:
  - phase: 25-01
    provides: Load-time effective-region API and source-pinned PAL clock evidence
provides:
  - Effective-region CPU cycle, M2 edge, and PPU dot timing accessors
  - PAL frame duration, exact frame rate, and per-instance sample remainder accounting
affects: [phase-25-03 PAL raster and APU timing, phase-25-04 host region integration]

# Actuals: chars/4 over the changed implementation and regression diff
actuals:
  tokens: 5375
  tasks: 2
  commits: 4
commits: 4
plan_head_before: 86d2f07eef217536d2cab891a8d0c2cc4c109544
plan_head_after: 38dc2c7cb89ef086155a95868b05acfc8b4c6114

# Tech tracking
tech-stack:
  added: []
  patterns:
    - Regional integer timing is derived from the loaded effective region, with no mutable global profile.
    - PAL fractional audio samples carry in per-instance frame state.
key-files:
  created: []
  modified:
    - src/internal.h
    - src/bus.c
    - src/frame.c
    - src/instance.c
    - tests/cpu/test_bus.c
    - tests/core/test_frame.c
    - include/nesturbator.h
    - README.md
key-decisions:
  - "Use the latched effective region for shared CPU, M2, PPU-dot, frame, and audio-rate accessors."
  - "Preserve the NTSC clock and remainder units; PAL uses 640/709379 samples per tick with instance-local remainder."
patterns-established:
  - "Bus cycle duration and M2 opening offset come from the same effective-region timing accessors."
  - "Frame sample counts use integer rational arithmetic and retain their remainder per instance."
requirements-completed: [REG-01, PAL-01]
coverage:
  - id: D1
    description: PAL and NTSC CPU bus accesses preserve cycle ordering while using their regional integer clocks.
    requirement: PAL-01
    verification:
      - kind: unit
        ref: "ctest --test-dir build/dev -R '^bus\\.unit$' --output-on-failure --no-tests=error"
        status: pass
    human_judgment: false
  - id: D2
    description: PAL frame rate and sample totals are exact, with atomic buffer refusals and independent regional instances.
    requirement: PAL-01
    verification:
      - kind: unit
        ref: "ctest --test-dir build/dev -R '^(core\\.(frame|api)|runner\\.(hash|advance|hash_audio\\.silent))$' --output-on-failure --no-tests=error"
        status: pass
    human_judgment: false

# Metrics
duration: 12min
completed: 2026-11-01
status: complete
---

# Phase 25 Plan 02: Regional CPU Bus and PAL Frame Accounting Summary

**PAL bus cycles now use the 2A07 13/32 M2 timing, and public frame/audio accounting uses exact rational rates while retaining the established NTSC path.**

## Performance

- **Duration:** about 12 minutes; the exact executor start timestamp was not captured, so this is approximate.
- **Started:** approximately 2026-11-01T22:58:35Z, after Plan 25-01 bookkeeping.
- **Completed:** 2026-11-01T23:10:06Z.
- **Tasks:** 2.
- **Files modified:** 8.

## Build and Verification

- **Source:** `38dc2c7` on `gsd/phase-25-pal-region-support`; CMake 4.4.3, `dev` preset.
- Task 1 RED assertions were observed, then `cmake --preset dev && cmake --build --preset dev --target bus.unit && ctest --test-dir build/dev -R '^bus\.unit$' --output-on-failure --no-tests=error` passed (1 selected target).
- Task 2 RED assertions were observed, then `cmake --build --preset dev --target core.frame core.api nesturbator-run && ctest --test-dir build/dev -R '^(core\.(frame|api)|runner\.(hash|advance|hash_audio\.silent))$' --output-on-failure --no-tests=error` passed (5 selected targets).
- The raw local CTest RED report embeds the checkout path, so hygiene rejected staging it; it remained in ignored `build/dev/`. Only owned test sources were committed. The implementation and focused GREEN checks passed.
- Full CI and AccuracyCoin were not run. The source-backed expectations are not hardware proof. There were no changes to DMA behavior, PAL raster event ordering, or APU synthesis in this plan.

## Accomplishments

- Added shared integer accessors based on `cart.effective_region`: NTSC uses 24 ticks per CPU cycle, M2 at +9/+24 and 8 ticks per PPU dot; PAL uses 32 ticks per cycle, M2 at +13/+32 and 10 ticks per dot. Existing MMC1/MMC3 CPU-cycle semantics remain intact.
- Added actual-bus regressions for PAL reads, writes, status reads, CPU cycles, and interleaved instances, with NTSC controls.
- Added exact PAL frame/audio accounting: 1,063,920 ticks per frame, `322445/6448` fps, and `640/709379` samples per tick. Counts are 959 or 960 samples per frame; after 64,489 frames the carried remainder returns to zero and totals 61,900,800 samples.
- Tested exact and undersized audio buffers, NULL/zero and NULL/nonzero capacities, wide-pitch guards, reset/reload/failure remainder behavior, instruction-boundary overshoot, and interleaved PAL/NTSC instances. Public output remains 256×240.
- Kept NTSC frame duration at 714,732 ticks, frame rate at `39375000/655171`, sample accounting at `352/315000` samples per tick, and sample rate at 48,000 Hz.

## Task Commits

Each implementation task was committed atomically:

1. **Task 1: Run CPU bus accesses and mapper clocks on the latched region** — `897ec9d` (test), `c91cb30` (feat).
2. **Task 2: Deliver exact PAL frames and sample counts through the public API** — `a9d1773` (test), `38dc2c7` (feat).

The GSD tracking commit is created after this summary and is reported by the executor.

## Files Created/Modified

- `src/internal.h` — shared regional timing and frame/audio ratio accessors.
- `src/bus.c` — region-specific bus cycle/M2 timestamps.
- `src/frame.c`, `src/instance.c` — exact frame timing, sample carry, and public rate reporting.
- `tests/cpu/test_bus.c`, `tests/core/test_frame.c` — owned NTSC/PAL timing and accounting regressions.
- `include/nesturbator.h`, `README.md` — public timing and frame contract documentation.

## Decisions Made

- All regional timing is derived from the loaded effective region; no mutable global region profile was introduced.
- PAL sample fractions use a per-instance remainder. NTSC keeps its existing remainder units and values.
- The existing no-content NTSC test card/silence behavior, instruction-boundary overshoot contract, and 256×240 output contract remain in place.

## Deviations from Plan

None in implementation scope. The local RED report was not committed because it contains an absolute checkout path; it remained ignored build output, and RED behavior was verified before the passing focused GREEN runs.

## Issues Encountered

None affecting the implementation or focused verification.

## User Setup Required

None.

## Next Phase Readiness

The shared PAL dot accessor and frame clock are ready for Plan 25-03. That plan still owns PAL raster ties and boundaries, APU cadence, frequency tables, and deterministic PAL synthesis. The full-CI/AccuracyCoin restart stop remains active.

## Known Stubs

None introduced by this plan.

## Self-Check: PASSED

- Summary file exists at the planned path.
- Task commits `897ec9d`, `c91cb30`, `a9d1773`, and `38dc2c7` are ancestors of the current plan head.

---
*Phase: 25-region-selection-and-pal-timing*
*Completed: 2026-11-01*
