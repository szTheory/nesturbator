---
phase: 04-sound
plan: 02
subsystem: audio
tags: [apu, frame-counter, dmc, dma, irq, accuracycoin]
requires:
  - phase: 04-sound
    plan: 01
    provides: instance-owned APU channels and CPU-cycle clocking
provides:
  - Parity-timed frame counter, length counter, and independent frame/DMC IRQ behavior
  - DMC DMA read-cycle halt, fetch, parked-read, and reload timing
  - Six individually required AccuracyCoin APU scoreboard rows
affects: [04-sound, runner, libretro]
actuals:
  duration: 122min
  tasks: 2
  files: 15
  commits: 5
tech-stack:
  added: []
  patterns: [phase-sensitive frame counter, read-cycle DMC DMA seam, exact named-test scoreboarding]
key-files:
  created: []
  modified: [src/apu.c, src/bus.c, src/cpu.c, src/cartridge.c, src/instance.c, src/internal.h, tests/core/test_apu.c, runner/main.c, tests/CMakeLists.txt, tests/accuracy/scoreboard.txt, tests/accuracy/scoreboard-main.txt, include/nesturbator.h, tests/core/test_api.c, tests/cmake/vector_api_policy.cmake, README.md]
key-decisions:
  - "A DMC sample fetch halts only a CPU read and completes the parked read after the transfer; a delayed reload after re-enabling with a full sample buffer uses the observed GET phase."
  - "Raise the public behavior revision to 2 for the changed CPU/APU output behavior and refresh the temporary declaration-stream guard."
  - "Require the six named AccuracyCoin page-14 rows in local and hosted CI; owner UAT is unnecessary for these machine-checked behaviors."
patterns-established:
  - "Frame-counter and DMC IRQ flags stay independent while the CPU sees their aggregated line."
  - "DMC tests assert both cycle counts and AccuracyCoin edge probes instead of inferring DMA timing from final audio alone."
requirements-completed: [SND-03]
commits: 5
plan_head_before: 4f8be70b351b706af42906309c9fdb67eeef8070
plan_head_after: 90fa202
coverage:
  - id: D1
    description: "Frame-counter reset and status edges, length clocks, and the independent frame IRQ source match the pinned behavior."
    requirement: SND-01
    verification:
      - kind: unit
        ref: tests/core/test_apu.c#test_frame_counter_modes_and_irq_sources and tests/core/test_apu.c#test_frame_irq_cpu_entry
        status: pass
      - kind: other
        ref: "cmake --workflow --preset ci"
        status: pass
    human_judgment: false
  - id: D2
    description: "DMC read stalls, fetch/reload cycles, half-rate timer phase, re-enable delay, and independent IRQ aggregation are covered by focused tests and six required AccuracyCoin results."
    requirement: SND-03
    verification:
      - kind: unit
        ref: tests/core/test_apu.c#test_dmc_load_halts_on_get_cycle and tests/core/test_apu.c#test_dmc_restart_preserves_full_sample_buffer
        status: pass
      - kind: integration
        ref: accuracycoin.page14 (Length Counter, Length Table, Frame Counter IRQ, Frame Counter 4-step, Frame Counter 5-step, Delta Modulation Channel)
        status: pass
      - kind: other
        ref: "cmake --workflow --preset ci"
        status: pass
    human_judgment: false
metrics:
  duration: 122min
  completed: 2026-10-09
  status: complete
---

# Phase 04 Plan 02: Frame Counter and DMC Timing Summary

**Frame sequencing, IRQ aggregation, DMC DMA edges, and six named AccuracyCoin APU checks now pass in the standard CI workflow.**

## Accomplishments

- Implemented 4-step and 5-step frame-counter timing, delayed `$4017` writes, length-counter clocks, frame IRQ status/clear behavior, and CPU IRQ entry while preserving independent frame and DMC IRQ sources.
- Connected DMC DMA to the CPU read-cycle seam. Fetches halt reads, transfer the sample byte, then complete the parked CPU read; controller, OAM DMA, and open-bus regressions remain covered.
- Corrected DMC timer half-rate phase and the enable-delay edge. A full sample buffer retained during `$4015` re-enable now schedules the deferred fetch on its observed GET phase.
- Added the AccuracyCoin page-14 runner path and requires named passes for Length Counter, Length Table, Frame Counter IRQ, Frame Counter 4-step, Frame Counter 5-step, and Delta Modulation Channel.
- Raised `NESTURBATOR_BEHAVIOUR_REVISION` to 2 and refreshed the temporary public declaration-stream policy for the changed behavior contract.

## Task Commits

1. **Task 1: Complete frame counter, length counter and DMC bus timing** — `2e82760`, `2c79c0d`, `2068993`, `a3ce4c9`, `90fa202`.
2. **Task 2: Require the six named AccuracyCoin APU scoreboard passes** — `90fa202`.

The plan began at `4f8be70b351b706af42906309c9fdb67eeef8070` and its code is committed through `90fa202`.

## Deviations from Plan

- CPU IRQ entry and reset integration also required changes in `src/cpu.c`, `src/cartridge.c`, and `src/instance.c`; those modules were absent from the plan's file list, but they are necessary for the required interrupt behavior.
- The public behavior revision and its comment were updated in the header, following the repository rule to document behavior changes with the public header. The temporary declaration guard and version test were updated to preserve an explicit API contract check.
- Diagnostic traces showed the DMA needed one resumed CPU read cycle after sample transfer and a GET-phase deferred load for the `$4015` full-buffer restart edge. These changes were pinned by unit tests and AccuracyCoin before completion.

## Verification

- `cmake --workflow --preset ci` — passed; all 348 registered tests passed. `retroarch.testframe` and `retroarch.game` self-skipped because the host environment did not provide RetroArch.
- AccuracyCoin page 14 — passed; all six required named rows are present and pass. The page also reports two non-required probes as unsupported; they are not part of the Phase 04-02 acceptance set.
- `scripts/hygiene.sh --tree` — passed.

## User Setup Required

None. Automated tests cover the planned timing and scoreboard acceptance checks.

## Next Plan

Proceed to **04-03**, fixed-point synthesis and the five-tone spectral gate.

## Self-Check: PASSED

- Summary file exists at the required phase path.
- All five listed plan commits are ancestors of the recorded plan head.
- The full CI workflow, including API policy and behavior revision tests, passed.

---
*Phase: 04-sound*
*Completed: 2026-10-09*
