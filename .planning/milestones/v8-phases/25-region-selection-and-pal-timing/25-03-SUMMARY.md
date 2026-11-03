---
phase: 25-region-selection-and-pal-timing
plan: "03"
subsystem: timing
tags: [pal, ppu, apu, audio, timing]
dependency_graph:
  requires: [25-02]
  provides: [pal-ppu-raster, pal-apu-cadence, pal-continuous-pcm]
  affects: [25-04, 25-05, phase-26-dma-and-state, phase-27-presentation]
tech_stack:
  added: []
  patterns: [regional-integer-clock-denominators, source-derived-independent-oracles, continuous-pal-sample-phase]
key_files:
  created: []
  modified:
    - src/ppu.c
    - src/apu.c
    - src/synth.c
    - src/frame.c
    - include/nesturbator.h
    - README.md
    - tests/ppu/test_nmi_races.c
    - tests/core/test_apu.c
    - tests/core/test_api.c
    - tests/runner/test_spectral.c
    - tests/cmake/vector_api_policy.cmake
decisions:
  - Keep 256x240 raw output rows distinct from the 312-line PAL raster and analog picture/border claims.
  - Select PAL noise/DMC tables and frame events from the loaded effective region; preserve NTSC constants and behavior checks.
  - Use continuous PAL sample phase at 640 samples per 709379 half-master ticks and retain legitimate ring overshoot between nominal frame drains.
  - Raise behavior revision to 16 for the changed public signal behavior; keep ABI and native state format unchanged.
requirements_completed: [PAL-01]
metrics:
  duration: 16min
  completed: 2026-11-01
  tasks: 3
  files: 11
  commits: 6
  plan_head_before: a4b874a4e74fe27125ac1f4ffe17288358c5abb5
  plan_head_after: d408d1a8ff437389473535969e5d2493cbc39898
status: complete
actuals:
  tokens: 12746
  tasks: 3
  commits: 6
coverage:
  - id: D1
    description: PAL 312-line raster, vblank boundaries, native rows, and OAM refresh window.
    requirement: PAL-01
    verification:
      - kind: unit
        ref: tests/ppu/test_nmi_races.c#PAL raster, row, status, and OAM assertions
        status: pass
      - kind: unit
        ref: "ctest -R '^(ppu.(nmi_races|registers|render|sprites|fetch|mmc3))$'"
        status: pass
    human_judgment: false
  - id: D2
    description: PAL noise/DMC periods and four/five-step frame, IRQ, and reset timing.
    requirement: PAL-01
    verification:
      - kind: unit
        ref: tests/core/test_apu.c#independent PAL period and event fixtures
        status: pass
      - kind: unit
        ref: "ctest -R '^core.apu$'"
        status: pass
    human_judgment: false
  - id: D3
    description: Public PAL PCM pitch, sample count, phase continuity, and instance determinism.
    requirement: PAL-01
    verification:
      - kind: integration
        ref: tests/runner/test_spectral.c#public PAL tone and rational pitch oracle
        status: pass
      - kind: integration
        ref: "ctest -R '^(runner.spectral|core.apu|runner.hash_audio.silent)$'"
        status: pass
    human_judgment: false
---

# Phase 25 Plan 03: PAL Raster, APU Cadence and Synthesis Summary

**The PPU now follows the source-derived PAL raster, while regional APU timing and continuous 48 kHz PCM retain the established NTSC signal path.**

## Performance

- **Duration:** 16 minutes
- **Started:** 2026-11-01T23:16:00Z
- **Completed:** 2026-11-01
- **Tasks:** 3
- **Files modified:** 11

## Accomplishments

- Implemented PAL's 312 × 341 raster at 10 ticks per dot, source-derived vblank and OAM refresh windows, and the fixed 256 × 240 native output representation.
- Added region-selected PAL noise/DMC periods and frame-sequencer events while retaining NTSC tables and expectations.
- Routed PAL transitions and synthesis through the regional integer clock, keeping sample phase continuous across frame calls and preserving generated overshoot in the bounded PCM ring.
- Added independent PAL raster, event, pitch, sample-count, and instance-interleaving checks. Repeated-run hashes are used only as determinism evidence.

## Files Created or Modified

- `src/ppu.c` — PAL raster, vblank, row, OAM-refresh, and NTSC-only odd-frame behavior.
- `src/apu.c` — regional period tables, PAL frame events, CPU-cycle transition stamps, and continuous PAL sample accounting.
- `src/synth.c` — matching regional phase denominators with wide integer intermediates; mixer, kernel, and filters remain shared.
- `src/frame.c` — PAL absolute nominal frame targets carry bounded instruction/DMA overrun forward rather than accumulating it per frame.
- `include/nesturbator.h`, `README.md` — public behavior revision and bounded PAL support documentation.
- `tests/ppu/test_nmi_races.c`, `tests/core/test_apu.c`, `tests/runner/test_spectral.c` — independent PAL row/event/rate/pitch/count and phase coverage.
- `tests/core/test_api.c`, `tests/cmake/vector_api_policy.cmake` — behavior-revision and API policy expectation updates.

## Verification

The planned focused commands passed:

1. `cmake --build --preset dev --target ppu.nmi_races ppu.registers ppu.render ppu.sprites ppu.fetch ppu.mmc3 && ctest --test-dir build/dev -R '^(ppu\.(nmi_races|registers|render|sprites|fetch|mmc3))$' --output-on-failure --no-tests=error` — 6/6 selected tests passed.
2. `cmake --preset dev && cmake --build --preset dev --target core.apu && ctest --test-dir build/dev -R '^core\.apu$' --output-on-failure --no-tests=error` — the APU test passed.
3. `cmake --build --preset dev --target runner.spectral core.apu nesturbator-run && ctest --test-dir build/dev -R '^(runner\.spectral|core\.apu|runner\.hash_audio\.silent)$' --output-on-failure --no-tests=error` — all three selected tests passed; this command was repeated after the final PAL assertions and comments.

No full CI, AccuracyCoin run, stopped campaign, hosted workflow, or direct-hardware test was run.

## Signal Oracles and Limits

- PAL raster/event expectations and PAL noise/DMC periods are transcribed from the pinned references and dispositions in `25-PAL-EVIDENCE.md`; tests use independent fixture values rather than production tables. The source-derived mapping keeps 240 raw rows separate from the internal 312-line raster and does not claim analog PAL color calibration.
- At 48 kHz, the pitch oracle uses pulse `53203425 / (32 × 16 × (timer + 1))` and triangle `53203425 / (32 × 32 × (timer + 1))`. The public runner checks peaks against these rational expectations within one FFT bin and checks generated sample totals and phase across frames. Hashes establish repeatability only.
- PAL accounts for exactly 640 samples per 709379 half-master ticks (the documented 959/960 relation), or 20480 phase units per CPU cycle over denominator 709379. NTSC retains 352/315000 and its existing staging and hashes.
- The conservative PAL ring bound is 600 CPU cycles of possible instruction/DMA overrun: 514 OAM-DMA cycles, 8 instruction cycles, up to 11 × 4 DMC allowance, and 34 cycles of margin. At 20480/709379 samples per CPU cycle this gives at most `ceil(600 × 20480 / 709379) + 1 = 19` retained samples, below the existing 1024-sample ring capacity. This bounds internal buffering only; DMA timing is not qualified here.
- PAL state continuation remains unsupported pending Phase 26. Palette presentation/color calibration remains in Phase 27. All PAL behavioral evidence in this plan is source-derived software evidence, not physical-hardware qualification.

## Decisions Made

- Keep native palette-index/emphasis data and the 256 × 240 output API; do not infer analog picture borders or palette conversion from raster row mapping.
- Use the loaded effective region for PAL APU periods, events, and sample timing. Preserve NTSC constants and observer timestamps.
- Keep PAL sample phase continuous and carry legitimate overshoot in the existing ring while draining each nominal public-frame count.
- Advance behavior revision from 15 to 16 for changed public signal behavior; ABI 1 and native state format 6 remain unchanged.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing compatibility metadata] Advanced the behavior revision to 16.**
- **Found during:** Task 2
- **Issue:** PAL changes alter public timing and signal behavior, so leaving revision 15 would misidentify the behavior contract.
- **Fix:** Updated the revision and the API/vector-policy expectations; ABI and serialized state format remain unchanged.
- **Files modified:** `include/nesturbator.h`, `tests/core/test_api.c`, `tests/cmake/vector_api_policy.cmake`
- **Commit:** `fb520b4`

**2. [Rule 1 - Bug] Gated the five-step half-frame clock away from the four-step-only quarter event.**
- **Found during:** Task 2
- **Issue:** The shared quarter-event path also clocked length counters at the four-step q4 point while five-step mode was active.
- **Fix:** Made that half-frame clock mode-specific and added a five-step fixture checking the source-derived absent event.
- **Files modified:** `src/apu.c`, `tests/core/test_apu.c`
- **Commit:** `fb520b4`

**3. [Rule 3 - Blocking issue] Carried PAL frame-target overrun forward across public frames.**
- **Found during:** Task 3
- **Issue:** Recomputing a PAL endpoint from each actual frame start would accumulate instruction/DMA overrun and undermine exact nominal sample accounting.
- **Fix:** Derived PAL endpoints from the absolute frame number and retained bounded overshoot for the next frame in the PCM ring.
- **Files modified:** `src/frame.c`, `src/apu.c`, `src/synth.c`, `tests/runner/test_spectral.c`
- **Commit:** `d408d1a`

## TDD Gate Compliance

- **Task 1 RED:** `build/dev/25-03-task1-red.json` and `build/dev/build/dev/gsd-red-25-03-task1.xml` captured the expected PAL raster/row/OAM assertion failures before implementation. The focused run failed only on the new PAL PPU case; retained PPU controls passed.
- **Task 1 GREEN:** The six-test PPU command above passed after implementation.
- **Task 2 RED:** `build/dev/25-03-task2-red.json` and `build/dev/build/dev/25-03-task2-red.xml` recorded expected PAL table and event failures before implementation.
- **Task 2 GREEN:** The focused APU build and test passed after implementation. During development, one LFSR test initially assumed a zero output for mode-bit rates; the assertion was corrected to check the rate interval through shift behavior independent of tap value, then the focused test passed.
- **Task 3 RED:** `build/dev/25-03-task3-red.json` and `build/dev/build/dev/25-03-task3-red.xml` recorded the expected CPU-cycle stamp and rational-pitch failures before implementation; unchanged APU/silent-audio controls passed.
- **Task 3 GREEN:** The runner spectral, APU, and silent audio-hash tests passed after implementation, including the final repeated run.
- The independent frequency/event/sample formulas above are acceptance oracles; digest equality is used only to establish determinism.

## Known Stubs

None found in files created or modified by this plan.

## Next Phase Readiness

Plans 25-04 and 25-05 remain in Phase 25. The next command is `$gsd-execute-phase 25`; it continues the remaining reviewed Phase 25 plans and their bounded checks. Context can be cleared before running it if `.planning/STATE.md`, the Phase 25 context/research/evidence, and plans 25-04/25-05 are read first. No user action is needed now.

## Self-Check: PASSED

- Summary file exists at the required phase path.
- All six task commits are ancestors of the recorded plan head.
- Measured plan commit count is six, matching `actuals.commits`.
