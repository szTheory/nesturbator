---
phase: 05-tune-up-and-v1-debt
plan: 05
subsystem: core
tags: [reset, cpu, public-api, tests]
requires: [05-02, 05-04]
provides:
  - "nesturbator_reset(): public soft reset, CPU half (I set, 7 bus cycles, S - 3, reset vector)"
  - "nesturbator__cpu_reset in src/cpu.c"
  - "core.reset CTest on a synthetic trainer image, including kept PRG-RAM, CHR-RAM and cartridge struct"
  - "PHASE5_DECLARATIONS_SHA256 declaration baseline"
affects: [libretro, core]
tech-stack:
  added: []
  patterns: ["cpu.c calls only bus functions, so the reset sequence links into the cpu.unit/cpu.vectors targets"]
key-files:
  created: [tests/core/test_reset.c]
  modified: [include/nesturbator.h, src/internal.h, src/instance.c, src/cpu.c, tests/CMakeLists.txt, tests/cmake/vector_api_policy.cmake, README.md]
key-decisions:
  - "The poll latch is cleared after the last reset cycle, because every bus cycle resamples the live IRQ line into it"
requirements-completed: [TUNE-06, TUNE-02]
status: complete
duration: 20 min
completed: 2026-10-10
commits: 2
plan_head_before: 8e7f8efd67c52e3f102127bc431776c9c6c85703
plan_head_after: d17af0cc2c66416865792115b0307d4d704bea5a
actuals:
  tokens: 14000
  tasks: 2
  commits: 2
coverage:
  - deliverable: "nesturbator_reset keeps RAM, PRG-RAM, CHR-RAM and cartridge state; S - 3, I set, vector, ticks + 168; NULL and no-cartridge cases"
    verification:
      - kind: test
        ref: "tests/core/test_reset.c#core.reset"
        status: pass
    human_judgment: false
  - deliverable: "Declaration baseline, header, internal and README text describe the released CPU-side reset"
    verification:
      - kind: command
        ref: "cmake --workflow --preset ci (vectors.api_policy); cmake --workflow --preset hygiene"
        status: pass
    human_judgment: false
---

# Phase 5 Plan 05: Soft reset, CPU half Summary

`nesturbator_reset()` keeps CPU RAM and all cartridge RAM and enters the reset vector with I set and S lowered by 3, proven by `core.reset` on a synthetic NROM trainer image.

## Accomplishments

- Public `nesturbator_reset` (ABI unchanged): NULL gives `ERR_ARGUMENT`, no cartridge is a no-op returning OK, no allocation. It catches the PPU up, clears controller strobe, shift and pending OAM DMA, keeps host input state, then calls `nesturbator__cpu_reset`.
- `nesturbator__cpu_reset`: sets I, clears `jammed`, five stack-page reads taking S down by 3, vector read from `$FFFC/$FFFD`; 168 ticks, `frame_number` untouched. Cites NESdev Wiki CPU power up state and CPU interrupts.
- `core.reset` covers NULL, no cartridge, every cleared and kept field (set non-default beforehand), idempotency, whole-struct cartridge compare plus 8,192 PRG-RAM bytes, and 8,192 CHR-RAM bytes on a CHR-RAM image. All ten acceptance mutations make it fail.
- TUNE-02: stale "CPU does not yet run during frames" and "nothing clears it yet" comments replaced; `ticks` comment mentions reset cycles; README Soft reset paragraph; declaration baseline renamed `PHASE5_DECLARATIONS_SHA256`.

## Deviations from Plan

**1. [Rule 1 - Bug] Poll latch cleared at the end of the sequence**
- **Found during:** Task 1
- **Issue:** Clearing `poll_latch` before the reset bus cycles was overwritten by `cycle()` in bus.c, which resamples `irq_line` each cycle, so `core.reset` saw 1.
- **Fix:** Clear it after the vector reads (I is set, so the first instruction polls afresh).
- **Files modified:** src/cpu.c
- **Commit:** f8802ac

**Total deviations:** 1 auto-fixed (1 bug). **Impact:** none on scope.

Note: the internal.h and header stale-comment edits (Task 2) landed in the Task 1 commit f8802ac because they touch the same files; Task 2 commit d17af0c holds the baseline and README.

## Commits

- f8802ac feat(05-05): add nesturbator_reset with the CPU reset sequence
- d17af0c docs(05-05): refresh declaration baseline and describe soft reset

## Verification

`cmake --workflow --preset ci`, `hygiene` and `asan`: 100% tests passed (358).

## Known Stubs

None. Plan 06 adds PPU and APU reset at the marked comment in `nesturbator_reset`.

## Next

Ready for 05-06.

## Self-Check: PASSED
