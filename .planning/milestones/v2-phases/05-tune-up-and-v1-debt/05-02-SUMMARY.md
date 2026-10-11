---
phase: 05-tune-up-and-v1-debt
plan: 02
subsystem: tests
tags: [ines, trainer, libretro, parity]
requires: []
provides:
  - "tests/ines.h synthetic iNES builder reusable by later phases"
  - "Trainer image frame parity between nesturbator-run and the libretro module inside libretro.host"
affects: [libretro.host]
tech-stack:
  added: []
  patterns: ["header-only static inline test builder", "negative control proving the input under test drives the output"]
key-files:
  created: [tests/ines.h]
  modified: [tests/libretro/libretro_host.c]
key-decisions:
  - "Trainer parity reuses compare_with_ppm and the existing libretro.host CTest case; no registration change"
requirements-completed: [TUNE-04]
status: complete
duration: 10 min
completed: 2026-10-10
commits: 2
plan_head_before: dc7869e4f4c81015e8ca80eebac04e0f681fda3b
plan_head_after: f7cefd81f09fbb1ee7990933f76d66b062cc352b
actuals:
  tokens: 6000
  tasks: 2
  commits: 2
coverage:
  - deliverable: "Trainer-bearing image gives equal frames through runner and libretro, pixel (0,0) 0xBA3100"
    verification:
      - kind: test
        ref: "libretro.host#check_trainer_frame_parity"
        status: pass
    human_judgment: false
  - deliverable: "Frame depends on the trainer (negative control) and ines_build header encoding is checked"
    verification:
      - kind: test
        ref: "libretro.host#check_ines_builder"
        status: pass
    human_judgment: false
---

# Phase 5 Plan 2: Trainer frame parity Summary

A synthetic trainer-bearing mapper-0 image built by the new `tests/ines.h` produces the same frame through `nesturbator-run` and the libretro module, and the frame is shown to depend on the trainer.

## Accomplishments

- Added `tests/ines.h` (`struct ines_spec`, `ines_size`, `ines_build`): static inline only, encodes PRG/CHR counts, flags 6/7, mapper across flags 6, 7 and NES 2.0 byte 8, submapper, optional trainer, PRG code and vectors.
- Added `check_trainer_frame_parity` to `libretro.host`: pixel (0,0) is 0xBA3100, runner PPM equals the libretro frame, and the same image without the trainer gives a different pixel.
- Added `check_ines_builder` (byte 6 = 0x21, byte 7 = 0x08, byte 8 = 0x11, size, too-small buffer and zero-PRG cases).

## Task Commits

1. Task 1 (tracer): 20eceb5 - trainer image through runner and libretro
2. Task 2: f7cefd8 - negative control, builder self-check, clang-format 18

## Deviations from Plan

None - plan executed exactly as written. (Per-plan commit ledger file was not written; the base `dc7869e` was recorded from HEAD at start.)

## Verification

Changing the trainer's `0x16` to `0x27` failed `libretro.host` on the 0xBA3100 check; reverted. `cmake --workflow --preset hygiene` and `cmake --workflow --preset ci` (357 of 357) pass.

## Known Stubs

None.

## Self-Check: PASSED

- tests/ines.h exists; commits 20eceb5 and f7cefd8 are on the branch.
