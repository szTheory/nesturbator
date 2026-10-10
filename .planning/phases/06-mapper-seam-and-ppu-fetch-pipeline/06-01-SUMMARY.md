---
phase: 06-mapper-seam-and-ppu-fetch-pipeline
plan: 01
subsystem: cartridge-mapper-seam
tags: [mapper, nrom, bus, ppu, irq, map-01]
requires: []
provides:
  - "src/mapper.h: nesturbator__mapper_ops, nesturbator__mapper, nesturbator__map, watch bits"
  - "src/mapper_nrom.c: NROM as the first board module"
  - "nesturbator__mapper_load (src/cartridge.c): the only board choice"
  - "nesturbator__irq_update (src/apu.c): the one writer of cpu.irq_line"
  - "nes->cpu_cycle stamp and tests/mapper_test.h test board"
affects: [06-02, phase-07, phase-08, phase-09, phase-10]
tech-stack:
  added: []
  patterns:
    - "ops struct held by value in nes->map.ops and filled by code (no file-scope pointer table)"
    - "1 KiB page tables and a four-entry nametable map; NULL page reads open bus or 0, writes drop"
    - "hooks guarded by map.watch bits, never by pointer tests"
key-files:
  created:
    - src/mapper.h
    - src/mapper_nrom.c
    - tests/mapper_test.h
    - tests/core/test_mapper.c
  modified:
    - src/internal.h
    - src/cartridge.c
    - src/bus.c
    - src/apu.c
    - src/ppu.c
    - CMakeLists.txt
    - tests/CMakeLists.txt
    - tests/ppu/test_render.c
    - tests/ppu/test_sprites.c
    - tests/ppu/test_registers.c
key-decisions:
  - "Ops struct lives by value in nes->map.ops so abi.global_symbols passes on ELF (D-01 implementation constraint)"
  - "Resolver and page helpers live in cartridge.c and internal.h; no src/mapper.c (file budget)"
  - "cpu_cycle is incremented last in cycle(); the hook receives the post-increment value"
requirements-completed: [MAP-01]
duration: about 1 h
completed: 2026-10-10
status: complete
commits: 3
plan_head_before: 371724e9eb4d961f4743614504cb49b8d682046e
plan_head_after: f5e9d860e2f5047a6f07f912be43a4183e339293
actuals:
  tokens: 40000
  tasks: 3
  commits: 3
---

# Phase 6 Plan 01: Mapper seam Summary

NROM now loads through a per-board ops interface with 1 KiB CPU and CHR page tables, a four-entry nametable map, a CPU-cycle-stamped write hook and a shared IRQ OR, with every v1 frame and audio hash byte-identical.

## Accomplishments

- `src/mapper.h` and `src/mapper_nrom.c`: ops (`init`, `rebuild`, `cpu_write`, `ppu_a12`, reserved `ppu_read`), serialisable `nesturbator__mapper`, derived `nesturbator__map`; NROM rebuilds pages and `nt[]` from the header.
- `bus.c` and `ppu.c` read and write the cartridge only through `map.cpu_r/cpu_w`, `map.chr_r/chr_w` and `map.nt`; `nesturbator__cart_read` is retired; no board switch in either file.
- `cpu_cycle` stamp in `cycle()`, write hook after the page store (fires for NULL ROM pages too), bus-conflict AND, `nesturbator__irq_update` ORing frame, DMC and mapper IRQs.
- `core.mapper` CTest (12 cases) and the PPU unit tests migrated to the board map.

## Seam gate (D-16)

- Seam commit (branch tip, Task 2 plus a comment fix): `f5e9d860e2f5047a6f07f912be43a4183e339293`. Task 2 test commit: `65d7de8`.
- Draft PR: https://github.com/szTheory/nesturbator/pull/30
- Green CI run: ID `38070510500`, https://github.com/szTheory/nesturbator/actions/runs/38070510500 (head SHA equals the seam commit). Every job `success`: both `nofp` legs, all six `build` legs, `asan`, `hygiene`, `hash-equality`, `retroarch-e2e`, `title`, `CI required`.
- `git diff --exit-code main -- tests/runner/hashes.txt tests/accuracy/scoreboard.txt` exits 0; no `sha256 <64 hex>` line added or removed in `tests/CMakeLists.txt`; revision still 4.
- Local presets: `ci` pass, `asan` pass, `hygiene` pass, `nofp` see Deviations.

## Performance baseline for Plan 02

`build/ci/runner/nesturbator-run --rom tests/roms/nesteroids.nes --frames 1800`, three runs on the owner's Mac (macOS arm64): 1.60 s, 1.69 s, 1.67 s. Median 1.67 s, about 1078 frames per second. Measured, not gated.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Decimal literal in core comments tripped the nofp float scan**
- **Found during:** Task 3 (local `nofp` preset)
- **Issue:** `abi.float_scan` read "NES 2.0" in two new comments as a float literal.
- **Fix:** reworded to "NES2".
- **Files modified:** src/cartridge.c, src/internal.h
- **Commit:** f5e9d86

### Known local-only failure (out of scope)

`abi.undefined_symbols` fails in the local macOS `nofp` run on `___stack_chk_fail` / `___stack_chk_guard`, referenced only from `src/synth.c.o`, which this plan does not touch. The Linux `nofp` CI legs (the real gate) pass. Not fixed here.

**Total deviations:** 1 auto-fixed (Rule 3). **Impact:** none on behaviour.

Commit shape: Task 1's commit left `ppu.render`, `ppu.sprites` and `ppu.registers` unmigrated as the plan allows; in practice they still compiled and passed because the retired cartridge fields still exist, so no commit on the branch was red.

## Known Stubs

None.

## Threat Flags

None. T-06-01 (bank modulo), T-06-02 (NULL pages) and T-06-03 (loader unchanged, mapper 0 only) are covered by `core.mapper`, the asan workflow and `fuzz.regress`.

## Self-Check: PASSED

- Created files exist: src/mapper.h, src/mapper_nrom.c, tests/mapper_test.h, tests/core/test_mapper.c.
- Commits b5b5655, 65d7de8, f5e9d86 are ancestors of HEAD; CI run 38070510500 concluded success for f5e9d86.
