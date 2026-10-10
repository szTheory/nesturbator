---
phase: 02-the-cpu-matches-the-public-vectors
plan: 05
subsystem: cpu
tags: [6502, read-modify-write, branches, stack, jsr, brk, rti, vectors, cycle-accuracy]

requires:
  - phase: 02-the-cpu-matches-the-public-vectors
    provides: "Addressing-mode helpers, 105 non-control opcodes and nesturbator_vector_tests() (02-04)"
provides:
  - "Operation helpers rmw, push, pull, branch, asl, lsr, rol, ror (plus inc, dec, accumulator, stack_dummy, push_p, pull_p, read_addr) in src/cpu.c"
  - "All 151 official opcodes as switch cases"
  - "NESTURBATOR_OFFICIAL_OPCODES holding all 151 official opcodes; 151 cpu.vectors.<xx> tests under label vectors"
affects: [02-06, cpu]

actuals:
  tokens: 3567
  tasks: 2
  commits: 2
plan_head_before: c4165e96a52f52af9e8536fb332e37e2c22a709d
plan_head_after: 00f3751b6f7cbcd035e37b73c896dc5e479f6ab6

tech-stack:
  added: []
  patterns:
    - "Read-modify-write and accumulator forms take a static uint8_t op(nes, v) helper passed by pointer; no opcode table in data"
    - "P bits 4 and 5 are written only by pull_p (PLP, RTI); push_p (PHP, BRK) ORs 0x30 into the pushed byte without touching P"

key-files:
  created: []
  modified:
    - src/cpu.c
    - tests/CMakeLists.txt
    - README.md

key-decisions:
  - "rmw(nes, ea, op) takes the shift/rotate/inc/dec helper as a function pointer argument, so the four shifts and INC/DEC share one read, write-old, write-new sequence"
  - "JSR reads ADH with fetch after both pushes, so a push over the operand is seen (D-17)"
  - "FLAG_B and FLAG_U are named only for the push and pull paths; set_flag never touches them"

patterns-established:
  - "stack_dummy() is the 0x0100+S dummy read shared by PLA, PLP, RTS, RTI and JSR"

requirements-completed: [CPU-01]

coverage:
  - id: D1
    description: "rmw, asl, push, pull, branch and the JSR sequence each match the sample (ASL zp, PHA, PLA, BNE, JSR)"
    requirement: "CPU-01"
    verification:
      - kind: integration
        ref: "ctest --preset ci -L vectors (cpu.vectors.06 20 48 68 d0)"
        status: pass
    human_judgment: false
  - id: D2
    description: "All 151 official opcodes match registers, raw P, RAM and every bus cycle on 100 sample tests each"
    requirement: "CPU-01"
    verification:
      - kind: integration
        ref: "ctest --preset ci -L vectors (151 tests)"
        status: pass
      - kind: other
        ref: "cmake --workflow --preset ci / asan / nofp / hygiene"
        status: pass
    human_judgment: false

duration: 3min
completed: 2026-10-03
status: complete
---

# Phase 2 Plan 5: Read-modify-write, branches, jumps, stack and interrupt-return opcodes Summary

**The last 46 official 6502 opcodes (shifts and rotates, INC/DEC in memory, eight branches, JMP, JSR, RTS, RTI, BRK, PHA/PHP/PLA/PLP) match the committed 65x02 sample on every bus cycle, so all 151 official opcodes now run as `cpu.vectors.<xx>` tests under label `vectors`.**

## Performance

- **Duration:** 3 min
- **Started:** 2026-10-03T15:37:41Z
- **Completed:** 2026-10-03T15:40:25Z
- **Tasks:** 2
- **Files modified:** 3

## Accomplishments

- `rmw` reads v, writes v back, then writes the result, each as a real bus call. abs,X read-modify-write always takes the dummy read, through `ea_absi(..., 1)`.
- `branch` makes the dummy read at PC when the branch is taken, and a second one at `(oldPCH << 8) | newPCL` only when the page changes.
- JMP (ind) reads the high byte from `hi:((lo+1) & 0xFF)`.
- JSR order is read ADL, dummy stack read, push PCH, push PCL, read ADH (D-17).
- PHP and BRK push `P | 0x30`. PLP and RTI load `(v & ~0x10) | 0x20`. BRK sets I and leaves D alone. No other path writes bits 4 and 5 (D-16, Pitfall 1).
- README "Checks" now says all 151 official opcodes, and says that PHP, PLP and RTI keep D.

## Task Commits

1. **Task 1 (tracer): rmw, push, pull, branch, asl helpers; ASL zp, PHA, PLA, BNE, JSR**: `d052bfe` (feat)
2. **Task 2: the other 41 opcodes, the 151-opcode list, README**: `00f3751` (feat)

## Files Created/Modified

- `src/cpu.c`: FLAG_B/FLAG_U, shift/rotate/inc/dec helpers, rmw, stack helpers, branch, 46 new cases (151 in all)
- `tests/CMakeLists.txt`: `NESTURBATOR_OFFICIAL_OPCODES` holds the plan's exact 151-opcode list
- `README.md`: the vectors paragraph covers all official opcodes

## Decisions Made

- `rmw` and `accumulator` take the operation as a `static` function pointer argument. No pointer table sits in data, so the `abi.global_symbols` anti-pattern does not apply. `ci` and `asan`, which run the abi tests, both pass.
- `read_addr`, `push_p`, `pull_p` and `stack_dummy` are small shared helpers, so each sequence is written once.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Docs with behaviour] README updated alongside the code**
- **Found during:** Task 2
- **Issue:** The README said "the 105 official ... opcodes". CLAUDE.md rule 6 requires the docs to change in the same change as the behaviour.
- **Fix:** The paragraph now says 151 official opcodes and that PHP, PLP and RTI keep D.
- **Files modified:** README.md
- **Committed in:** 00f3751

The extra static helpers (`inc`, `dec`, `accumulator`, `stack_dummy`, `push_p`, `pull_p`, `read_addr`) all have callers. They add no behaviour the plan does not ask for.

**Total deviations:** 1 auto-fixed (docs). **Impact:** none on scope.

## Issues Encountered

None. All 46 opcodes matched the sample the first time they were built.

## Verification

- `ctest --preset ci -N -R '^cpu\.vectors\.(06|20|48|68|d0)$'`: 5 tests.
- `ctest --preset ci -L vectors`: 100% of 151 passed. `ctest --preset ci -N -L vectors | grep -c 'cpu.vectors\.'`: 151.
- `grep -cE '^[[:space:]]*case 0x' src/cpu.c`: 151.
- Workflows: `ci` exits 0 (208 tests), `asan` 0 (207), `nofp` 0 (5), `hygiene` 0 (6).
- Tracer gate (interactive, end-of-phase, automated verify only): after the commit the 110 vector tests were run again and passed, then Task 2 started.

## User Setup Required

None.

## Next Phase Readiness

- Ready for 02-06, the unofficial opcodes. Unofficial RMW (SLO, RLA, SRE, RRA, DCP, ISC) can reuse `rmw` with the existing shift, rotate, inc and dec helpers. The `default:` JAM branch still handles every unofficial opcode.

---
*Phase: 02-the-cpu-matches-the-public-vectors*
*Completed: 2026-10-03*

## Self-Check: PASSED
