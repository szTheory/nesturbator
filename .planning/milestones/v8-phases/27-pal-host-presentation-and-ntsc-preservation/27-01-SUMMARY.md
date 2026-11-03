---
phase: 27-pal-host-presentation-and-ntsc-preservation
plan: "01"
subsystem: host
tags: [runner, libretro, PAL, AV, rational-rates]

# Dependency graph
requires:
  - phase: 26-pal-dma-and-regional-state-continuation
    provides: Effective-region queries and exact rate fractions on the active instance
provides:
  - Opt-in runner profile report for the loaded region and exact frame/sample rates
  - Runner and libretro lifecycle regressions for resolved profile reporting
affects: [runner, libretro, PAL host presentation]

# Actuals measured over the plan's three task commits
actuals:
  tokens: 3251
  tasks: 2
  commits: 3
plan_head_before: 760d48ebd1899f02f1a1f305077bbc210b4902f0
plan_head_after: 13cd9c698ddb9f2e4ff1da198863a77fd8bd7e46

# Tech tracking
tech-stack:
  added: []
  patterns:
    - Read region and exact rates from the same successfully loaded instance
    - Keep stable informational output opt-in and separate from existing stdout records

key-files:
  created: []
  modified:
    - runner/main.c
    - tests/libretro/libretro_host.c
    - tests/CMakeLists.txt
    - docs/runner.md
    - docs/api.md
    - README.md
    - include/nesturbator.h

key-decisions:
  - "Use the parseable `info region=... fps=... sample_rate=...` line with exact fractions."
  - "Keep existing runner output byte-for-byte unchanged unless `--info` is requested."
  - "Keep library region and rate queries separate and derive libretro AV from the loaded instance."

patterns-established:
  - "Host profile reports use the loaded instance as their single source of truth."

requirements-completed: [AV-01, NTSC-01]
coverage:
  - id: D1
    description: "The runner reports resolved region and exact rates only when `--info` is requested."
    requirement: AV-01
    verification:
      - kind: integration
        ref: "runner.info, runner.usage.info.*, runner.region.*"
        status: pass
    human_judgment: false
  - id: D2
    description: "Runner/library/libretro profile lifecycle agrees while preserving existing output and callbacks."
    requirement: NTSC-01
    verification:
      - kind: integration
        ref: "libretro.host"
        status: pass
    human_judgment: false

# Metrics
duration: 5min
completed: 2026-11-02
status: complete
---

# Phase 27 Plan 01: Runner resolved-profile reporting Summary

**The runner now reports its loaded region and exact rates on request, with real runner, library, and libretro lifecycle coverage.**

## Performance

- **Duration:** 5 minutes between the first and final task commits; active time was not recorded.
- **Started:** 2026-11-02T18:34:22Z (first task commit)
- **Completed:** 2026-11-02T18:39:23Z (final task commit)
- **Tasks:** 2
- **Files modified:** 7

## Accomplishments

- Added the opt-in `--info` line using `nesturbator_get_region()` and a size-initialized `nesturbator_info` from the same loaded instance. The report carries exact frame-rate and sample-rate fractions.
- Expanded real-process runner assertions and libretro module lifecycle checks for resolved profile, refusal/reload behavior, callback stability, and unchanged output when the option is absent.
- Updated the README, runner guide, API guide, and public-header comments without adding a public symbol or changing the public struct layout.

## Task Commits

1. **Task 1: Report one resolved PAL load through the real three-deliverable path** — `a3499ad` (failing runner assertion), `7fac2bc` (feature and initial docs).
2. **Task 2: Expand reporting and lifecycle protection to all region-policy edges** — `13cd9c6` (tests and API documentation).

## Files Created/Modified

- `runner/main.c` — opt-in report and mode/duplicate validation.
- `tests/libretro/libretro_host.c` — runner and loaded-adapter region/rate lifecycle assertions.
- `tests/CMakeLists.txt` — runner info and usage test registration.
- `docs/runner.md`, `docs/api.md`, `README.md`, `include/nesturbator.h` — CLI and host API contracts.

## Decisions Made

- Kept the line grammar stable and exact: `info region=pal fps=322445/6448 sample_rate=48000/1`.
- Preserved default runner stdout and the existing separate library region and info queries.
- Kept libretro region and AV reporting tied to the active loaded instance; no runtime AV reconfiguration was added.

## Verification

- `cmake --preset dev` and `cmake --build --preset dev --target libretro.host` passed.
- The focused CTest selection for runner info, usage, region, options/docs, and `libretro.host` passed all 13 tests.
- The earlier bounded full workflow on candidate `13cd9c698ddb` produced a complete intermediate AccuracyCoin report (126 PASS / 20 FAIL) with the established scoreboard hashes. Its activation checks rejected the stale source fingerprint, so that capture is not final phase acceptance. Plan 27-04 owns rebinding provenance after fingerprinted inputs are frozen and rerunning the full workflow.

## Deviations from Plan

None in implementation. The missing plan summary was reconstructed manually after the safe-resume check confirmed the three task commits and the project state already recorded Plan 27-01 as implemented; the focused verification was rerun before closing it out.

## Issues Encountered

The intermediate full-workflow source fingerprint no longer matched because later Phase 27 plans change fingerprinted inputs. The strict fingerprint and historical receipt remain untouched; final binding is deferred to Plan 27-04 as decided in D-07 through D-10.

## User Setup Required

None.

## Next Phase Readiness

Plan 27-02 can add the modeled PAL display table through the existing instance-aware palette API. Final source-bound acceptance remains assigned to Plan 27-04.

---
*Phase: 27-pal-host-presentation-and-ntsc-preservation*
*Completed: 2026-11-02*

## Self-Check: PASSED

- Summary file exists and all seven modified files are present.
- All three task commits are reachable from the phase branch.
- Evaluation-scope resolves exactly the three plan 27-01 task commits.
