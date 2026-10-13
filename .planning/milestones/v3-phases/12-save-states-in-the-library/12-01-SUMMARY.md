---
phase: 12-save-states-in-the-library
plan: 01
subsystem: core-library
tags: [save-states, crc32, visitor, little-endian, c17]

requires:
  - phase: 11-tune-up-and-state-readiness
    provides: empty PCM ring and NULL frame-scoped pointers at every frame boundary (D-16), appended status numbering
provides:
  - nesturbator_state_size, nesturbator_save_state, nesturbator_load_state (D-01 signatures)
  - NESTURBATOR_ERR_STATE_FORMAT/CARTRIDGE/SIZE/CHECKSUM/INVALID = 8..12 and NESTURBATOR_STATE_FORMAT_VERSION 1
  - a 64-byte canonical little-endian header with CRC-32 over the state and a ROM CRC-32 identity
  - src/state.h visitor primitives (SIZE, WRITE, VALIDATE, APPLY) for the board ops of Plan 02
affects: [12-02 board state ops, 12-03 API contract and README, 12-04 fork matrix, 12-06 rejection sweeps, 12-08 runner]

actuals:
  tokens: 16900
  tasks: 2
  commits: 2

plan_head_before: cbc96b190a09f57d947ef0c731db1c762009d156
plan_head_after: 5c5f5b19a527ebd31b2fcaa08e67699c04374ab8

tech-stack:
  added: []
  patterns:
    - "One field list per struct drives SIZE, WRITE, VALIDATE and APPLY, so the four passes cannot drift"
    - "Static inline primitives in state.h, so board files link inside bus.unit without state.c"
    - "sizeof/offsetof _Static_assert tripwires on the pointer-free structs"

key-files:
  created: [src/crc32.c, src/state.h, src/state.c, tests/core/test_state.c]
  modified: [include/nesturbator.h, src/internal.h, src/cartridge.c, CMakeLists.txt, tests/CMakeLists.txt, tests/cmake/vector_api_policy.cmake]

key-decisions:
  - "Header holds frame_number (offset 32) and ticks (offset 40); 16 reserved zero bytes at 48"
  - "header_size is checked in the INVALID stage after the CRC, not as FORMAT (D-03 limits FORMAT to magic and version)"
  - "ppu_ticks rule is ppu_ticks <= ticks and ticks - ppu_ticks < NESTURBATOR_TICKS_PER_FRAME, so a state saved after nesturbator_reset loads"
  - "nesturbator_version does not gain a format field; NESTURBATOR_VERSION_SIZE_V1 stays 24"
  - "Signed primitives return 0 for an out-of-range value in VALIDATE, and only VALIDATE sums the synthesis history, so a poisoned instance cannot overflow in SIZE or WRITE"

patterns-established:
  - "Cross-field rules live in the same *_io function and use io->ticks from the header, never the instance"
  - "File-local st_* helpers in tests/core/test_state.c (instance, file read, frame digest, snapshot, le readers, st_recrc) for Plan 03 to move into tests/state_test.h"

requirements-completed: [STATE-01, STATE-02]

coverage:
  - id: D1
    description: "Save at frame 60 on dabg.nes (NROM), load into a fresh instance: the next 60 frames, audio, ticks, frame numbers and the final state bytes equal the uninterrupted run"
    requirement: STATE-01
    verification:
      - kind: unit
        ref: "ctest --preset ci -R ^core\\.state$ (tracer_round_trip)"
        status: pass
    human_judgment: false
  - id: D2
    description: "Header layout, CRC-32 rule and vector, D-05 save order, D-04 load order including header_size in the INVALID stage, no-cartridge edge and restored position"
    requirement: STATE-02
    verification:
      - kind: unit
        ref: "ctest --preset ci -R ^core\\.state$ (header_layout, crc_vector, save_order, load_order, empty_instance, position_restored)"
        status: pass
      - kind: unit
        ref: "ctest --preset ci -R ^vectors\\.api_policy"
        status: pass
    human_judgment: false

duration: 70min
completed: 2026-10-12
status: complete
---

# Phase 12 Plan 01: Save states on NROM Summary

**Whole-machine save and load in a 64-byte-header, CRC-32 protected, canonical little-endian format with an all-or-nothing load pipeline in the D-04 order, proven by a dabg round trip at frame 60.**

## Performance

- **Duration:** about 70 min
- **Tasks:** 2 of 2
- **Files:** 4 created, 6 modified

## Accomplishments

- `src/state.h` and `src/state.c`: one field list per struct (CORE, CPU, BUS, APU with pulse/triangle/noise/DMC, SYNTH, PPU, MAPPER `irq`) visited in SIZE, WRITE, VALIDATE and APPLY modes. Every S and L bound from RESEARCH section 3 is enforced, plus the synthesis sum invariant, the `eval_copy_remaining` pairing rule and the `ppu_ticks` gap rule. APPLY zeroes the PCM ring, copies PRG-RAM and CHR-RAM into the existing allocations, calls `nesturbator__mapper_load` and rebuilds `map.a12` from `ppu.bus_addr`.
- `src/crc32.c`: IEEE CRC-32 update function with a const 256-entry table; `cart.rom_crc32` is computed once at the end of `nesturbator_load_cartridge` over the 16 header bytes, PRG ROM and CHR ROM (not CHR RAM, not the trainer).
- Public header: five statuses 8 to 12, `NESTURBATOR_STATE_FORMAT_VERSION 1`, the three D-01 functions with "Checks in order:" comments for save and load, the header layout, the JAM rule and the D-08 note; `frame_number` and `ticks` comments now say "since power-on, restored by a state load".
- Measured state sizes (current field list, before Plan 02 adds board registers): **dabg.nes (NROM, CHR RAM) 13087 bytes**; NROM with CHR ROM 4895; MMC1 with 8 KiB RAM and CHR RAM 13087 (21279 with 8 KiB battery and a 512 KiB PRG image, 45855 with 32 KiB RAM). Plan 02 appends board bytes after `mapper.irq`.
- Ad-hoc check outside the repo: save then load every frame for 400 frames over all 15 committed ROMs, zero rejections, so no bound rejects a state the core produces.

## Task Commits

1. **Task 1 (tracer): CRC-32, ROM identity, header, visitor, public API, dabg round trip** - `3834bc3` (feat)
2. **Task 2: declarations hash refresh, header layout, CRC vector, D-05 and D-04 orders, D-08** - `5c5f5b1` (test)

## Deviations from Plan

None to code behaviour. Notes:

- Task 2 is marked `tdd="true"`, but its behaviours were already implemented by the Task 1 tracer, so every new case passed on its first run. There was no failing-test (RED) commit, because no unimplemented behaviour existed to fail; `workflow.tdd_mode` is false. The cases are written to fail on a wrong order or a wrong header field (for example `header_size` 63 and 65 assert INVALID, never FORMAT).
- The tripwire constants differ from my first guess and were taken from a compile-time probe: `sizeof` cpu 14, bus 2061, pulse 16, triangle 14, noise 12, dmc 22; `offsetof` apu sample_phase 92 and sample_output 96. PPU and synth use relative `offsetof` checks.
- The README "Save states" section and status table are delivered by Plan 03, as the plan states; this plan updated the public header comments.

## Known Stubs

None.

## Threat Flags

None beyond the plan's register. T-12-01 and T-12-02 are mitigated here (VALIDATE before APPLY, S/L bounds, `ppu_ticks` gap rule); Plan 06 sweeps them. T-12-03: the visitor writes field by field, no struct copy.

## Verification

- `cmake --workflow --preset ci`: 489 of 489 tests passed, package built.
- `cmake --workflow --preset asan`: 484 of 484 tests passed; `core.state` ran in 4.3 s under ASan and UBSan.

## Self-Check: PASSED

- FOUND: src/crc32.c, src/state.h, src/state.c, tests/core/test_state.c
- FOUND commits 3834bc3 and 5c5f5b1 in HEAD ancestry
