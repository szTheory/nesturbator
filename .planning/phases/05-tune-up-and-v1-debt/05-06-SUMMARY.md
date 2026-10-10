---
phase: 05-tune-up-and-v1-debt
plan: 06
subsystem: core
tags: [reset, ppu, apu, public-api, tests]
requires: [05-05]
provides:
  - "nesturbator__ppu_reset and ppu.reset_flag: top of the picture, $2000/$2001/$2005/$2006 ignored until 261/1"
  - "nesturbator__apu_reset: $4015 = 0 path, IRQ and DMC DMA latch clears, $4017 re-applied through the delay path"
  - "core.reset cases for the window boundary, vblank, APU and two instances"
affects: [libretro, core]
tech-stack:
  added: []
  patterns: ["state-level reset checks call the internal reset directly when later cycles would mask a missed clear"]
key-files:
  created: []
  modified: [src/internal.h, src/ppu.c, src/apu.c, src/instance.c, tests/core/test_reset.c, include/nesturbator.h, README.md]
key-decisions:
  - "vblank_suppress is cleared with the position; rendering pipeline state is left alone"
  - "$4017 is re-applied through frame_reset_delay = phase ? 2 : 1, not a direct frame_cycle write"
requirements-completed: [TUNE-06, TUNE-02]
status: complete
duration: 25 min
completed: 2026-10-10
commits: 2
plan_head_before: e0d3ad4
plan_head_after: a9c83215491fd4efc14a812e2b23a670e67b63c1
actuals:
  tokens: 16000
  tasks: 3
  commits: 2
coverage:
  - deliverable: "PPU restarts at the top of the picture and drops $2000/$2001/$2005/$2006 for 29,667 CPU cycles; boundary pinned at cycle 29,666 (dropped) and 29,667 (lands)"
    verification:
      - kind: test
        ref: "tests/core/test_reset.c#core.reset"
        status: pass
    human_judgment: false
  - deliverable: "APU silenced, IRQs and DMC DMA latches cleared, $4017 re-applied (frame_cycle 5 or 6 at the first instruction), $4015 reads 0"
    verification:
      - kind: test
        ref: "tests/core/test_reset.c#core.reset"
        status: pass
    human_judgment: false
  - deliverable: "Header and README describe the released reset; v1 hashes, scoreboard and behaviour revision 4 unchanged"
    verification:
      - kind: command
        ref: "cmake --workflow --preset ci; git diff --exit-code main -- tests/runner/hashes.txt tests/accuracy/scoreboard.txt"
        status: pass
    human_judgment: false
---

# Phase 5 Plan 06: Soft reset, PPU and APU half Summary

`nesturbator_reset()` now restarts the PPU at scanline 0 dot 0 with a 29,667-cycle write-ignore window and silences the APU while re-applying the last `$4017`, proven by `core.reset`.

## Accomplishments

- `nesturbator__ppu_reset` clears control, mask, w latch, t, fine X, read buffer, odd_frame, vblank_suppress and the CPU NMI latches; keeps v, status, oam_addr and all memories; sets `reset_flag`, which `nesturbator__ppu_register_write` uses to drop `$2000/$2001/$2005/$2006` (w latch untouched) until the 261/1 block clears it. Cites NESdev Wiki PPU power up state.
- `nesturbator__apu_reset`: triangle phase 0, `dmc.output &= 1`, `$4015 = 0` through the existing write path, frame IRQ and DMC DMA latches cleared, `frame_reset_delay = phase ? 2 : 1`. The frame-IRQ clear is documented as resting on the blargg apu_reset readme only.
- `core.reset` gains: all four gated registers at the 29,666/29,667 boundary (scanline 261 dot 0 dropped, dot 3 lands), registers that work inside the window, idempotent double reset, reset placed in vblank (scanline 245, bit 7 kept, clears at 261/1 with the flag), APU clear group, `$4017` timing for both phases (frame_cycle 5 and 6), no-cartridge case, two-instance equality and isolation.
- Mutation check: all 11 planned deletions and additions make `core.reset` fail.
- Header comment and README Soft reset paragraph describe the released behaviour (TUNE-02).

## Deviations from Plan

**1. [Rule 1 - Plan precision] Dot after reset is 21, not 0**
- **Found during:** Task 1
- **Issue:** The plan says scanline 0 dot 0 after reset, but the 7 reset cycles run the PPU 21 dots from the new position.
- **Fix:** The test asserts scanline 0, dot 21 after `nesturbator_reset`, which is dot 0 at the start of the reset, the same instant the 29,667-cycle window counts from. Mutation of `scanline = 0` still fails.

**2. [Rule 3 - Test design] APU latch clears checked by calling `nesturbator__apu_reset` directly**
- **Issue:** Through the public call, the first reset cycles clear `frame_irq`/`frame_irq_clear_pending` themselves, so deleting those clears would survive.
- **Fix:** `check_apu_cleared` inspects state straight after the internal call; the public-path test covers `$4015`, `$4017` timing and the rest.

**3. Task commits combined**
- Tasks 1 and 2 share `src/internal.h`, `src/instance.c` and `tests/core/test_reset.c`, so they landed in one commit (817bd3a); Task 3 is a9c8321.

**Total deviations:** 3 (one precision fix, one test design, one commit grouping). **Impact:** none on scope.

## Issues Encountered

- `cmake --workflow --preset nofp` fails `abi.undefined_symbols` locally on macOS arm64: `src/synth.c` references `___stack_chk_fail`/`___stack_chk_guard`. Not touched by this plan and not caused by it (only synth.c.o has the symbols); out of scope, left as is. ci, asan and hygiene pass.

## Commits

- 817bd3a feat(05-06): reset restarts the PPU and silences the APU
- a9c8321 docs(05-06): describe the PPU and APU reset in the header and README

## Verification

`ci` and `asan`: 100% tests passed (358). `hygiene`: 100% (8). `tests/runner/hashes.txt` and `tests/accuracy/scoreboard.txt` identical to main; `NESTURBATOR_BEHAVIOUR_REVISION` is 4.

## Known Stubs

None.

## Next

Ready for 05-07.

## Self-Check: PASSED
