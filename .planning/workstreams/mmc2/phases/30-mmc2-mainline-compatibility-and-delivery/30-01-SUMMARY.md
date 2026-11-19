---
phase: 30-mmc2-mainline-compatibility-and-delivery
plan: "01"
subsystem: core
tags: [MMC2, mapper-9, NES-2, save-state]

requires: []
provides:
  - Bounded NTSC NES 2.0 mapper 9/submapper 0 core path with watched PPU reads and eight serialized register bytes.
  - Independent marker, transactional refusal, state restore, renderer and buffered-read coverage.
affects: [30-02, runner, libretro, provider]

actuals:
  tokens: 20297
  tasks: 2
  commits: 2

tech-stack:
  added: []
  patterns:
    - Mapper state stores fixed-width registers; state load rebuilds derived bank maps.
    - PPU pattern reads call a board hook only when the mapper watches those reads.

key-files:
  created: [src/mapper_mmc2.c, tests/core/test_mapper_mmc2.c, tests/mmc2_fixture.h]
  modified: [src/mapper.h, src/cartridge.c, src/ppu.c, CMakeLists.txt, tests/CMakeLists.txt, include/nesturbator.h, README.md, tests/core/test_api.c, tests/core/test_cartridge.c, tests/core/test_state_reject.c]

key-decisions:
  - "Keep ABI 1, advance behavior revision 9 to 10, and advance state format 3 to 4 for the eight MMC2 bytes."
  - "Admit only NTSC NES 2.0 mapper 9/submapper 0 with 128 KiB PRG and CHR ROM and no trainer or RAM."
  - "Keep the adopted latch policy explicitly at secondary-reference evidence; it does not establish silicon or title compatibility."

patterns-established:
  - "Initial FE latches and header-derived mirroring are deterministic load policy; mutable selectors and latches remain per instance."
  - "Old state formats refuse before applying state; page tables are rebuilt from the saved mapper registers."

requirements-completed: [MMC2-07, MMC2-08, MMC2-10]
coverage:
  - id: D1
    description: "MMC2 public cartridge load, PRG/CHR markers, mirroring, watched PPU reads and old-bank return behavior."
    requirement: MMC2-07
    verification:
      - kind: unit
        ref: "ctest core.mapper_mmc2; build/ci/phase30-core-focused.log"
        status: pass
    human_judgment: false
  - id: D2
    description: "Bounded profile refusal, deterministic restore, transactional state validation and continued execution."
    requirement: MMC2-08
    verification:
      - kind: unit
        ref: "ctest core.mapper_mmc2 and core.state_reject; build/ci/phase30-core-regression-focused.log"
        status: pass
    human_judgment: false
  - id: D3
    description: "Public ABI/revision/state compatibility and bounded-scope documentation."
    requirement: MMC2-10
    verification:
      - kind: unit
        ref: "ctest core.api and core.cartridge; build/ci/phase30-core-regression-focused.log"
        status: pass
    human_judgment: false

duration: 17min
completed: 2026-11-19
status: complete
---

# Phase 30 Plan 01 Summary

**The exact-main core now loads the bounded MMC2 profile, switches CHR banks on watched pattern reads and restores all eight mapper bytes deterministically.**

## Performance

- **Duration:** about 17 minutes
- **Started:** 2026-11-19T20:20:50Z
- **Completed:** 2026-11-19T20:37:07Z
- **Tasks:** 2
- **Files modified:** 13

## Accomplishments

- Added mapper 9 only for NTSC NES 2.0 submapper 0 with exactly 128 KiB of PRG ROM and CHR ROM, rejecting trainers, RAM, battery, four-screen and other profiles before allocation.
- Added the PPU-read hook, PRG/CHR/mirroring registers, two FE/FD latches, bounded eight-byte state serialization and derived-map rebuild after restore.
- Updated ABI1/rev10/state4 comments and README support claims; added independent literal markers, full trigger-row coverage, buffered PPUDATA and rendering checks, reset/isolation checks and transactional state refusal.
- Preserved existing frame/audio/tick expectations and protected AccuracyCoin evidence. The exact source base and live `main` both remained `8923b66cca16739c5b402643798515cf5537ae22`.

## Verification

- The initial public-load MMC2 test failed before implementation because mainline refused the image (`NESTURBATOR_ERR_CARTRIDGE`), as expected; log: `build/ci/phase30-initial-red.log`.
- `cmake --preset ci -DNESTURBATOR_ACCURACYCOIN_FULL=ON` passed; `build/ci/phase30-core-configure.log` records the final focused configuration.
- `cmake --build --preset ci --target core.mapper_mmc2 bus.unit` passed; `build/ci/phase30-core-build.log`.
- The focused mapper and bus test inventory contained only `core.mapper_mmc2` and `bus.unit`; both passed; `build/ci/phase30-core-inventory.log` and `build/ci/phase30-core-focused.log`.
- The expanded regression build passed for API, cartridge, existing mapper families, MMC2, state and state rejection. The CTest inventory excluded campaign fixtures. All nine selected tests passed in 6.5 seconds; `build/ci/phase30-core-regression-build.log`, `build/ci/phase30-core-regression-inventory.log` and `build/ci/phase30-core-regression-focused.log`.
- `scripts/hygiene.sh --staged` and `git diff --check` passed before both commits.

## Task Commits

1. **Task 1: Public load/read/restore path and compatibility docs** — `67b540b` (`feat(mmc2): port bounded mapper 9 core path`)
2. **Task 2: Independent profile and state coverage** — `f2161b0` (`test(mmc2): cover mainline profile and state`)

## Decisions Made

- Kept ABI 1 because the public C interface did not change; advanced behavior revision to 10 and state format to 4 for the appended MMC2 state bytes.
- Kept the adopted full-row FD/FE trigger model and marked it as secondary-reference evidence, not hardware validation.
- Refused PAL and dual-region MMC2 headers transactionally; no region/runtime API was added.

## Deviations from Plan

None. The existing owned MMC2 fixture and tests were adapted to the target NTSC contract; no v10 branch merge, v10 version pin, dependency or full workflow was used.

## Issues Encountered

The project-pinned clang-format 18.1.8 executable was available. The changed C files passed its formatting check; `scripts/hygiene.sh --staged` passed before the code commits.

## User Setup Required

None.

## Next Plan Readiness

Plan 30-02 can proceed. The core tests and mainline compatibility contract are in place; its next work is actual runner/libretro cold and resumed parity, then provider metadata and state-only golden reconciliation. The one full CI workflow remains reserved for plan 30-03.

---
*Phase: 30-mmc2-mainline-compatibility-and-delivery*
*Plan: 01*
*Completed: 2026-11-19*
