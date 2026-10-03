---
phase: 02-the-cpu-matches-the-public-vectors
plan: 02
subsystem: cpu
tags: [c17, cmake, ctest, 6502, n65v, link-time-seam, object-library]

requires:
  - phase: 02-the-cpu-matches-the-public-vectors
    provides: "02-01: vecconv, the N65V reader and the a9-first3 fixture"
provides:
  - "src/cpu.c: nesturbator__cpu_step with LDA immediate, the D-13 JAM sequence as the default branch, and P-preserving fetch/set_flag/set_nz helpers"
  - "src/bus.c: the final Phase 2 bus: 24 ticks per call (9, catch-up, 15, catch-up), 2048 bytes of RAM mirrored over 0x0000-0x1FFF, an open-bus latch"
  - "struct nesturbator__cpu and struct nesturbator__bus as members cpu and bus of struct nesturbator"
  - "nesturbator_cpu OBJECT library folded into libnesturbator.a; nesturbator_core_flags helper (include, hidden, PIC, warnings, nofp)"
  - "cpu.vectors <file.n65v> <opcode-hex> <1|256> <tests-per-chunk> on a logging test bus (tests/cpu/vector_bus.{c,h})"
  - "tests/cmake/vectors_fixture.cmake (VECCONV VECTORS JSON OPCODE FIRST OUT RUN)"
  - "ctest bus.unit; vecconv.a9 now runs the converted tests through the CPU"
affects: [02-03, 02-04, 02-05, 02-06, 02-07, 02-08, 03]

actuals:
  tokens: 7070
  tasks: 2
  commits: 2
plan_head_before: 9837422ef35ca8625535d629c82662290261715d
plan_head_after: 444eb270efdd23f71efc2ea195f688a6857b302e

tech-stack:
  added: []
  patterns:
    - "Link-time bus seam: cpu.c calls only nesturbator__bus_read/write; the library gets them from src/bus.c, test programs from tests/cpu/vector_bus.c, never both in one link"
    - "A test program links $<TARGET_OBJECTS:nesturbator_cpu>, the object the library ships, and never nesturbator::nesturbator"
    - "A vector test passes on the fixture script's exit status; no PASS_REGULAR_EXPRESSION"
    - "Opcodes not yet written fall to the JAM default, so they stop the CPU in a way a test can see"

key-files:
  created:
    - src/cpu.c
    - src/bus.c
    - tests/cpu/vector_bus.h
    - tests/cpu/vector_bus.c
    - tests/cpu/test_vectors.c
    - tests/cpu/test_bus.c
    - tests/cmake/vectors_fixture.cmake
  modified:
    - src/internal.h
    - CMakeLists.txt
    - tests/CMakeLists.txt
    - tests/core/test_api.c
    - README.md

key-decisions:
  - "core.api's counting-allocator arena grows from 256 to 65536 bytes, because the instance now holds 2048 bytes of RAM (2112 bytes in all)"
  - "bus.c puts the 24-tick sequence in one static cycle() helper that both bus functions call"
  - "cpu.vectors accepts chunks 1 or 256 only, and tests-per-chunk 1 to 10000; any other value is a usage error (exit 2)"
  - "The README sentence for vecconv.a9 is updated in the Task 1 commit, which changes the test, rather than in Task 2 (CLAUDE.md rule 6)"

patterns-established:
  - "cpu.vectors stdout is exactly one line, '65x02/<xx>: <failed> of <run> vectors failed'; each mismatch goes to stderr as '<xx>.json[<index>]: <field> expected 0x.. got 0x..'"

requirements-completed: [CPU-01]

coverage:
  - id: D1
    description: "The first three a9 tests run from upstream JSON through the shipped CPU object and match on registers, RAM and every bus cycle"
    requirement: CPU-01
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^vecconv\\.a9$' -V (prints '65x02/a9: 0 of 3 vectors failed')"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset asan"
        status: pass
    human_judgment: false
  - id: D2
    description: "cpu.o and bus.o are inside libnesturbator.a, with no new undefined symbol, no writable data and no floating point"
    verification:
      - kind: integration
        ref: "cmake --workflow --preset nofp (abi.float_scan, abi.undefined_symbols, abi.global_symbols, 5 of 5)"
        status: pass
      - kind: other
        ref: "nm build/ci/CMakeFiles/nesturbator_cpu.dir/src/cpu.c.o shows only _nesturbator__bus_read undefined"
        status: pass
    human_judgment: false
  - id: D3
    description: "The library bus has 24 ticks per call, RAM mirrors and an open-bus latch"
    verification:
      - kind: unit
        ref: "ctest --preset ci -R '^bus\\.unit$'"
        status: pass
    human_judgment: false
  - id: D4
    description: "Runner frame hashes and every other lane are unchanged"
    verification:
      - kind: integration
        ref: "cmake --workflow --preset ci (37 of 37, runner.hash and runner.write_hashes.content included)"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset hygiene (6 of 6)"
        status: pass
    human_judgment: false

duration: 5min
completed: 2026-10-03
status: complete
---

# Phase 2 Plan 02: CPU object behind the link-time bus seam Summary

**The `$A9` slice now runs from upstream JSON through vecconv into `nesturbator__cpu_step`, the same `cpu.o` the library ships, linked against a test bus that logs every cycle. All three tests match on registers, RAM and each bus cycle. `bus.o`, in its final Phase 2 form, joined the library in the same commit.**

## Performance

- **Duration:** about 5 min
- **Started:** 2026-10-03T15:14:24Z
- **Completed:** 2026-10-03T15:19:23Z
- **Tasks:** 2
- **Files modified:** 12

## Accomplishments
- `src/cpu.c` is one `switch (opcode)`. `case 0xA9` is LDA immediate. The `default:` branch runs the D-13 JAM sequence: 11 reads, PC left at opcode plus 1, `jammed` set. A jammed step is one read of `0xFFFF`. `set_flag` masks its own bits, so P is never rebuilt.
- `src/bus.c` advances `ticks` by 9, catches up the PPU (an empty stub for now), advances 15, catches up again, then does the access. RAM is `ram[addr & 0x7FF]` below `0x2000`. Every read and write sets the latch, and an unmapped read returns it.
- `CMakeLists.txt`: `nesturbator_core_flags` gives both `nesturbator_cpu` (OBJECT) and `nesturbator` the include directory, hidden visibility, PIC, the warnings and the nofp flag. The CPU object goes into the library through `target_sources(... $<TARGET_OBJECTS:nesturbator_cpu>)`. All of this sits above the top-level `return()`.
- `cpu.vectors` reads the whole file through the N65V reader. It runs each test of the chosen opcode through one `cpu_step` and compares pc, s, a, x, y, raw p, the final RAM pairs, the cycle count and each cycle. It zeroes only the addresses the test named.
- `vectors_fixture.cmake` runs the conversion, then the CPU, removes `\r` and requires the exact summary line. `vecconv.a9` has no pass regex now, so its exit status decides the test.
- `bus.unit` compiles `src/bus.c` on its own and checks 24, 240 and 264 ticks, the three mirrors, the latch after a write and after a read, and that an unmapped write leaves RAM unchanged.

## Task Commits

1. **Task 1: End-to-end `a9` fixture through the shipped CPU object behind the link-time seam** - `c7eae82` (feat)
2. **Task 2: bus.unit, the library bus's ticks, mirror and open-bus latch** - `444eb27` (test)

## Files Created/Modified
- `src/internal.h`: CPU and bus state structs, the `cpu` and `bus` members, and the three internal prototypes
- `src/cpu.c`: `nesturbator__cpu_step` and its helpers
- `src/bus.c`: the library's bus functions
- `CMakeLists.txt`: `nesturbator_core_flags`, `nesturbator_cpu`, `bus.c` in the library, NOFP option moved above the core targets
- `tests/cpu/vector_bus.{h,c}`: `struct vector_machine`, the test bus, `vector_machine_reset_log`
- `tests/cpu/test_vectors.c`: the `cpu.vectors` program
- `tests/cpu/test_bus.c`: `bus.unit`
- `tests/cmake/vectors_fixture.cmake`: converts a fixture, then optionally runs it
- `tests/CMakeLists.txt`: `cpu.vectors`, `bus.unit`, `vecconv.a9` through the script
- `tests/core/test_api.c`: counting-allocator arena enlarged
- `README.md`: `vecconv.a9` now runs through the CPU; `bus.unit` described

## Decisions Made
- `core.api`'s test arena is now 65536 bytes. Phase 3 will add more instance state, and this leaves room for it without editing the test again.
- `cpu.vectors` checks its arguments strictly, so a typo in a later plan's CMake call fails loudly rather than running zero tests.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] core.api's counting allocator was too small for the instance**
- **Found during:** Task 1
- **Issue:** The test's allocator handed out at most 256 bytes. Adding `bus.ram[2048]` makes `struct nesturbator` 2112 bytes, so `nesturbator_create` returned `NESTURBATOR_ERR_NO_MEMORY` and `core.api` failed.
- **Fix:** Enlarged the arena union to 65536 bytes, with a comment.
- **Files modified:** tests/core/test_api.c
- **Verification:** `core.api` passes in the ci and asan lanes.
- **Committed in:** c7eae82

**2. [Rule 2 - Docs together] README `vecconv.a9` sentence moved into Task 1**
- **Found during:** Task 1
- **Issue:** The plan updates that sentence in Task 2. Task 1 changes what the test does, and CLAUDE.md rule 6 requires the README to change in the same commit.
- **Fix:** Rewrote the sentence in c7eae82; Task 2 added only the `bus.unit` sentence.
- **Committed in:** c7eae82, 444eb27

---

**Total deviations:** 2 auto-fixed (1 blocking, 1 documentation timing)
**Impact on plan:** None on scope.

## TDD Gate Compliance

Task 2 is marked `tdd="true"`, but Task 1 had to ship `src/bus.c` in its final form first (D-12: `bus.c` and `cpu.o` enter the library in one commit). So `bus.unit` passed as soon as it was written, and no real RED state existed. To show the test can fail, it was run against mutated copies of `bus.c`, each restored before the commit:
- 15 ticks changed to 14: three tick checks failed (23, 230 and 253 ticks).
- No latch update on a RAM read: the open-bus read of `0x8000` returned `0x11` instead of `0x5a`.
- RAM mask `0x3FF` instead of `0x7FF`: the mirror read at `0x07FF` returned `0x00`.

The task has a `test(02-02)` commit and no separate `feat` commit, because the implementation is in Task 1's `feat(02-02)` commit. `workflow.tdd_mode` is off, so this is recorded here and was not enforced.

## Issues Encountered
- `nm` on `cpu.c.o` lists only `nesturbator__bus_read` as undefined. No opcode writes yet, so `nesturbator__bus_write` is not referenced. The acceptance criterion allows these two names and no others, so this meets it. The first store opcode will add the second.
- A mutation check on Task 1: removing `set_nz` from `case 0xA9` made `cpu.vectors` report `a9.json[0]: p expected 0xa0 got 0xa2` for all 3 tests and exit 1. Passing a wrong opcode argument gave the reader's `offset 5:` error and exit 1.

## User Setup Required
None.

## Next Phase Readiness
- Plans 03 onward add opcodes as `case` lines and fixtures through `vectors_fixture.cmake`. `RUN=OFF` is available for the compact-layout fixture until plan 06.
- `cpu.vectors` already handles a 256-chunk sample file (`<file> <xx> 256 <n>`).
- The public header is unchanged (`git diff 9837422 -- include` is empty).

---
*Phase: 02-the-cpu-matches-the-public-vectors*
*Completed: 2026-10-03*

## Self-Check: PASSED
