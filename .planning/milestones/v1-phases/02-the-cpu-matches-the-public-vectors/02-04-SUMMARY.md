---
phase: 02-the-cpu-matches-the-public-vectors
plan: 04
subsystem: cpu
tags: [6502, addressing-modes, vectors, ctest, cycle-accuracy]

requires:
  - phase: 02-the-cpu-matches-the-public-vectors
    provides: "cpu.vectors harness and test bus (02-02); committed 65x02 sample and reader (02-03)"
provides:
  - "Addressing-mode helpers ea_zp, ea_zpx, ea_zpy, ea_abs, ea_absi, ea_izx, ea_izy in src/cpu.c"
  - "Operation helpers adc (SBC through the complement), cmp_reg, bit_op, ora, and_op, eor"
  - "105 official opcodes: loads, stores, transfers, ALU, compares, flags, increments, NOP"
  - "nesturbator_vector_tests() and NESTURBATOR_OFFICIAL_OPCODES; tests cpu.vectors.<xx> under label vectors"
affects: [02-05, 02-06, cpu]

actuals:
  tokens: 5018
  tasks: 2
  commits: 2
plan_head_before: bc815061d2c8be21adfa1c8e0ddb425a8e8d8645
plan_head_after: 9b84a4fbd8058a4256073dcb1d73835b1f9533f8

tech-stack:
  added: []
  patterns:
    - "One switch case per opcode calling an address helper and an operation helper; the address helper is the call argument, so its cycles run first"
    - "index_base() shares the abs,X/Y and (zp),Y page-cross dummy read; always_dummy=1 for stores"
    - "Vector tests pass on expect_output.cmake's exit status and exact stdout, never a regex"

key-files:
  created: []
  modified:
    - src/cpu.c
    - tests/CMakeLists.txt
    - README.md

key-decisions:
  - "ea_zpx and ea_zpy share ea_zpi; ea_absi and ea_izy share index_base for the page-cross dummy read"
  - "Bus calls may sit in a call argument (an address helper) but never two in one expression with unspecified order; the cpu.c header comment states this rule"
  - "ADC computes in uint32_t; SBC is ADC of the operand's complement; D is never read (D-16)"

patterns-established:
  - "nesturbator_vector_tests(<xx>...) registers cpu.vectors.<xx> with label vectors; plan 05 extends NESTURBATOR_OFFICIAL_OPCODES"

requirements-completed: [CPU-01]

coverage:
  - id: D1
    description: "Every addressing-mode helper matches the sample on its read path (LDA family and LDX zp,Y)"
    requirement: "CPU-01"
    verification:
      - kind: integration
        ref: "ctest --preset ci -L vectors (cpu.vectors.a1 a5 a9 ad b1 b5 b6 b9 bd)"
        status: pass
    human_judgment: false
  - id: D2
    description: "105 non-control official opcodes match registers, raw P, RAM and every bus cycle on 100 sample tests each"
    requirement: "CPU-01"
    verification:
      - kind: integration
        ref: "ctest --preset ci -L vectors (105 tests)"
        status: pass
      - kind: other
        ref: "cmake --workflow --preset ci / asan / nofp / hygiene"
        status: pass
    human_judgment: false

duration: 3min
completed: 2026-10-03
status: complete
---

# Phase 2 Plan 4: Addressing modes and the non-control official opcodes Summary

**Seven addressing-mode helpers and 105 official 6502 opcodes (loads, stores, transfers, ALU, compares, flags, increments, NOP) that match the committed 65x02 sample on registers, raw P, RAM and every bus cycle, run as 105 `cpu.vectors.<xx>` tests under label `vectors`.**

## Performance

- **Duration:** 3 min
- **Started:** 2026-10-03T15:32:38Z
- **Completed:** 2026-10-03T15:35:53Z
- **Tasks:** 2
- **Files modified:** 3

## Accomplishments

- `ea_zp`, `ea_zpx`, `ea_zpy`, `ea_abs`, `ea_absi`, `ea_izx`, `ea_izy`: one bus call per cycle, dummy reads at the uncorrected address on a page cross for reads and always for stores, zero-page wrap for the pointer modes.
- 105 official opcodes as switch cases. ADC and SBC are binary whatever D holds, with the hardware comment citing the nes6502 README. Flag writes go through `set_flag`, which masks to its own bits, so nothing in this plan writes P bits 4 and 5.
- `nesturbator_vector_tests()` registers `cpu.vectors.<xx>` through `expect_output.cmake`: it checks exit 0 and the exact line `65x02/<xx>: 0 of 100 vectors failed`, with label `vectors` and no regex property.
- The README "Checks" section describes `cpu.vectors.<xx>` and the label.

## Task Commits

1. **Task 1 (tracer): addressing-mode helpers, LDA family, vector test function**: `14f0bf4` (feat)
2. **Task 2: the other 96 opcodes, the 105-opcode list, README**: `9b84a4f` (feat)

## Files Created/Modified

- `src/cpu.c`: flag constants, address helpers, operation helpers, 105 opcode cases
- `tests/CMakeLists.txt`: `nesturbator_vector_tests()` and `NESTURBATOR_OFFICIAL_OPCODES`
- `README.md`: `cpu.vectors.<xx>` in "Checks"

## Decisions Made

- `ea_zpx` and `ea_zpy` are thin wrappers over a shared `ea_zpi`. `ea_absi` and `ea_izy` share `index_base` for the page-cross dummy read, so that logic exists once.
- The `cpu.c` header comment said each bus call is "in a statement of its own". Address helpers now run as call arguments, so the comment states the rule as it holds: no expression contains two bus calls whose order C leaves unspecified.

## Deviations from Plan

None. The plan ran as written. The extra small static helpers (`ea_zpi`, `index_base`, `implied`, `assign`, `read_at`, `sbc`, `ora`, `and_op`, `eor`, `step_reg`, `flag_op`, `transfer`) each have callers and add no behaviour beyond the plan's.

## Issues Encountered

None. All 105 opcodes matched the sample the first time they were built. As a sanity check, the not-yet-written opcode `69` was run by hand before Task 2 and reported `100 of 100 vectors failed`.

## Verification

- `ctest --preset ci -N -L vectors | grep -c 'cpu.vectors\.'` gives 105.
- `ctest --preset ci -L vectors --show-only=json-v1 | grep -c PASS_REGULAR_EXPRESSION` gives 0.
- `grep -cE '^[[:space:]]*case 0x' src/cpu.c` gives 105.
- `grep -c 'cpu.vectors' README.md` gives 2.
- Workflows: `ci` exits 0 (162 tests), `asan` 0 (161), `nofp` 0 (5), `hygiene` 0 (6).
- Tracer gate (interactive, end-of-phase, automated verify only): the 9 tracer tests were re-run after the commit and passed, then the plan moved on to Task 2.

## User Setup Required

None. No external service configuration is required.

## Next Phase Readiness

- Ready for 02-05: read-modify-write, branches, jumps, subroutines and stack opcodes. It extends `NESTURBATOR_OFFICIAL_OPCODES` to 151 and can reuse the address helpers, calling `ea_absi`/`ea_izy` with always_dummy 1 for RMW.

---
*Phase: 02-the-cpu-matches-the-public-vectors*
*Completed: 2026-10-03*

## Self-Check: PASSED
