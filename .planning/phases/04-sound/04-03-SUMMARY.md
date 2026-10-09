---
phase: 04-sound
plan: 03
subsystem: synth
tags: [fixed-point, pcm, filters, spectral, audio]
requires:
  - phase: 04-sound
    plan: 02
    provides: instance-owned APU channel state and CPU-cycle transition producer
provides:
  - Fixed-point band-limited transition-to-PCM synthesis with per-instance filters and bounded frame staging
  - Five-tone spectral gate with an integer-only FFT and the locked -80 dB threshold
  - Public audio documentation and behavior revision 3
affects: [04-sound, runner, libretro]
actuals:
  duration: 24min
  tasks: 1
  files: 15
  commits: 1
tech-stack:
  added: []
  patterns: [16-tap integrated-sinc kernel, Q30 filter state, bounded PCM staging, integer CORDIC FFT]
key-files:
  created: [src/synth.c, tests/core/test_synth.c, tests/runner/test_spectral.c]
  modified: [src/apu.c, src/cartridge.c, src/internal.h, CMakeLists.txt, tests/CMakeLists.txt, include/nesturbator.h, README.md]
key-decisions:
  - "Keep the checked-in mixer and 32-phase by 16-tap Q15 kernel together in src/synth.c; every row sums exactly to 32768."
  - "Retain Q30 guard precision in filter state so fixed-point filter steps do not discard sub-PCM precision on every sample."
  - "For spectral analysis, choose the nearest coherent bin to each timer tone, fold the triangle fundamental at Nyquist, and exclude ±1 bins around the first 512 folded harmonic orders."
  - "Raise the public behavior revision to 3 for the new PCM synthesis and filter output."
patterns-established:
  - "APU mixed-level transitions enter synthesis synchronously; the core stores no frame-sized transition log."
  - "A 1024-sample per-instance ring stages PCM while kernel, filter and sample-fraction state carry across frames."
  - "The spectral analyzer uses owned integer CORDIC coefficients and a radix-2 FFT; no test or shipped-core floating point or dependency is added."
requirements-completed: [SND-04]
commits: 1
plan_head_before: 90fa202
plan_head_after: d411737
coverage:
  - id: D1
    description: "Production APU level transitions produce deterministic mono PCM through the fixed-point kernel and NES output filters, while each instance retains bounded synthesis and filter history across frames."
    requirement: SND-04
    verification:
      - kind: unit
        ref: tests/core/test_synth.c
        status: pass
      - kind: integration
        ref: core.frame and core.apu
        status: pass
      - kind: other
        ref: "cmake --workflow --preset ci"
        status: pass
    human_judgment: false
  - id: D2
    description: "The five reference tones remain below the -80 dB non-harmonic peak limit below 16 kHz, and the integer mixed level returns after one coherent period window."
    requirement: SND-04
    verification:
      - kind: integration
        ref: runner.spectral
        status: pass
      - kind: other
        ref: "cmake --workflow --preset ci"
        status: pass
    human_judgment: false
metrics:
  duration: 24min
  completed: 2026-10-09
  status: complete
---

# Phase 04 Plan 03: Fixed-Point Synthesis Summary

**The production sound path now generates band-limited mono PCM, and all five reference tones pass the automated spectral threshold.**

## Accomplishments

- Moved the checked-in integer pulse and TND mixer tables into `src/synth.c` beside the 32-phase, 16-tap Q15 integrated-sinc kernel. Compile-time dimensions and runtime row-sum checks protect the tables.
- Connected APU mixed-level changes directly to the synthesizer. Per-instance impulse history, Q30 high-pass/low-pass state, sample timing and the bounded 1024-sample PCM ring persist across frame calls.
- Applied the NES 90 Hz and 440 Hz high-pass filters and 14 kHz low-pass filter using fixed-point coefficients and arithmetic.
- Added core tests for exact kernel sums and dimensions, impulse integration, filter decay, bounded chunk drains and adjacent-frame continuity.
- Added `runner.spectral`, an integer-only CORDIC twiddle/Hann generator and radix-2 FFT. It discards 4096 samples, analyzes 32768 periodic-Hann samples, uses coherent timer bins, folds the triangle tone at Nyquist, and masks ±1 bins around the first 512 folded harmonic orders.
- Updated the README and public header for the fixed-point mono PCM path; raised `NESTURBATOR_BEHAVIOUR_REVISION` to 3 and refreshed the temporary public API declaration guard.

## Task Commit

- **Task 1: Add fixed-point synthesis and the five-tone spectral gate** — `d411737`.

The plan began at `90fa202`; the implementation is committed through `d411737`.

## Spectral Results

| Reference tone | Largest non-harmonic peak below 16 kHz |
|---|---:|
| Pulse period 100 | -89.14 dB |
| Pulse period 40 | -86.05 dB |
| Pulse period 12 | -88.23 dB |
| Pulse period 8 | -85.46 dB |
| Triangle period 1 | -91.83 dB |

## Deviations from Plan

- The existing frame lifecycle already calls `nesturbator__apu_begin_frame` and `nesturbator__apu_end_frame`; staging drains through that seam in `src/apu.c`, so `src/frame.c` required no change.
- The filter state uses Q30 guard precision after the Q15 kernel integration. This keeps the output deterministic while avoiding repeated loss of fractional PCM precision at each filter step.
- The spectral test resolves “integer harmonics” as the first 512 coherent harmonic orders folded into the positive FFT spectrum. The finite order keeps the alias mask from covering the full finite DFT. The selected bin and harmonic rules are recorded in the plan, research, test comments, and README.

## Verification

- `cmake --workflow --preset ci` — passed; all 350 tests passed. `retroarch.testframe` and `retroarch.game` self-skipped because this host has no RetroArch environment. The three CI packages were generated.
- Focused tests — passed: `runner.spectral`, `core.synth`, `core.apu`, `core.frame`, `core.api`, `bus.unit`, and `accuracycoin.page14`.
- `scripts/hygiene.sh --tree` — passed.
- Spectral measurements — all five tones are below -80 dB; the worst measured result is pulse period 8 at -85.46 dB.

## User Setup Required

None. CTest and the standard CI workflow cover the synthesis and spectral acceptance checks without audio hardware or listening UAT.

## Next Plan

Proceed to **04-04**, canonical transition and PCM hashes plus libretro sample equality.

## Self-Check: PASSED

- The production library, runner test, documentation and per-plan summary are present.
- Commit `d411737` contains the complete implementation and is the recorded plan head.
- The full required workflow and hygiene check passed.

---
*Phase: 04-sound*
*Completed: 2026-10-09*
