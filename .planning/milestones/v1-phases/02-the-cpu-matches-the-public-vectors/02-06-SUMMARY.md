---
phase: 02-the-cpu-matches-the-public-vectors
plan: 06
subsystem: cpu
tags: [6502, unofficial-opcodes, jam, shx, ane, lxa, machine-profile, vectors, cycle-accuracy]

requires:
  - phase: 02-the-cpu-matches-the-public-vectors
    provides: "All 151 official opcodes, rmw/branch/stack helpers and nesturbator_vector_tests() (02-05)"
provides:
  - "All 256 opcodes as explicit switch cases in src/cpu.c, with no default:"
  - "Static helpers jam, store_sh, store_sh_abs, slo, rla, sre, rra, dcp, isc, lax, anc, arr, sbx"
  - "struct nesturbator__profile { ane_magic, lxa_magic } as member profile; NESTURBATOR_RP2A03G_ANE_MAGIC / _LXA_MAGIC = 0xEE set by nesturbator_create"
  - "cpu.vectors.00 to cpu.vectors.ff (256 tests, label vectors); cpu.unit; core.profile; vecconv.02 runs through the CPU"
affects: [02-07, 02-08, phase-03, cpu]

actuals:
  tokens: 5797
  tasks: 3
  commits: 4
plan_head_before: 509a5f687c7c96f33bb430841a3f1b7737c6ba5e
plan_head_after: 9476c2d08b4104a441d999c4c942301697803bc3

tech-stack:
  added: []
  patterns:
    - "Combined unofficial RMWs are rmw op helpers that call the official op, then the ALU op, and return the stored byte"
    - "Chip-dependent constants live in nes->profile, set by create; the vector harness sets them itself"
    - "Test names 00 to ff come from a nested foreach over hex digits, never a hand list"

key-files:
  created:
    - tests/cpu/test_cpu_unit.c
    - tests/core/test_profile.c
  modified:
    - src/cpu.c
    - src/internal.h
    - src/instance.c
    - tests/CMakeLists.txt
    - tests/cpu/test_vectors.c
    - include/nesturbator.h
    - README.md

key-decisions:
  - "The two opcode lists are removed: Task 2 replaced their calls with one nested foreach of all 256 opcodes, so the lists had no other use"
  - "store_sh_abs wraps store_sh for the four abs-indexed SHx/TAS forms; SHA (zp),Y calls store_sh directly with the pointer's high byte"
  - "Task 3's RED could not fail: the behaviours existed (JSR from 02-05, JAM and profile from Task 2). Each test was shown to catch a working-tree mutation of its behaviour, then the source was restored"

patterns-established:
  - "No exclusion list: a chip difference goes through a profile field (D-18)"

requirements-completed: [CPU-01]

coverage:
  - id: D1
    description: "The 85 stable unofficial opcodes match the sample per cycle, P bits 4 and 5 preserved (6b 0c 1c dc)"
    requirement: "CPU-01"
    verification:
      - kind: integration
        ref: "ctest --preset ci -L vectors (236 tests after Task 1)"
        status: pass
    human_judgment: false
  - id: D2
    description: "All 256 opcodes, including JAM, ANE, LXA, LAS, SHY, SHX, SHA and TAS, match the sample on registers, raw P, RAM and every bus cycle; vecconv.02 runs 3 JAM tests through the CPU"
    requirement: "CPU-01"
    verification:
      - kind: integration
        ref: "ctest --preset ci -L vectors (256 tests); ctest --preset ci -R ^vecconv\\.02$ -V"
        status: pass
    human_judgment: false
  - id: D3
    description: "JSR stack overlap (issue 18), jammed loop, JAM PC wrap and RP2A03G profile defaults are pinned by unit tests"
    requirement: "CPU-01"
    verification:
      - kind: unit
        ref: "tests/cpu/test_cpu_unit.c (cpu.unit); tests/core/test_profile.c (core.profile)"
        status: pass
    human_judgment: false
  - id: D4
    description: "Header (comments only) and README describe the CPU; all lanes pass"
    verification:
      - kind: other
        ref: "cmake --workflow --preset ci / asan / nofp / hygiene; header diff vs origin/main is comment-only"
        status: pass
    human_judgment: false

duration: 6min
completed: 2026-10-03
status: complete
---

# Phase 2 Plan 6: Unofficial opcodes, JAM, unstable stores and the machine profile Summary

**All 256 6502 opcodes now match the committed 65x02 sample on every bus cycle. That covers the 85 stable unofficial opcodes, the 12 JAMs and ANE/LXA/LAS/SHY/SHX/SHA/TAS. ANE and LXA read a per-instance RP2A03G profile (0xEE), and unit tests cover the JSR stack overlap, the jammed loop and the profile defaults.**

## Performance

- **Duration:** 6 min
- **Started:** 2026-10-03T15:42:32Z
- **Completed:** 2026-10-03T15:48:51Z
- **Tasks:** 3
- **Files modified:** 9

## Accomplishments

- Stable unofficial opcodes: 27 NOP variants (abs,X reads with a page-cross dummy only), LAX, SAX, ANC, ALR, ARR (touches only N Z C V), SBX and SBC `eb`. SLO/RLA/SRE/RRA/DCP/ISC go through `rmw` with combined helpers, and abs,Y and (zp),Y always take the dummy read.
- JAM has its own `jam` helper and 12 explicit cases. ANE and LXA read `nes->profile`. LAS sets S = A = X = mem & S. `store_sh` is the single place that applies `reg & (H + 1)` and the page-cross high-byte replacement, and TAS sets S = A & X first. A comment marks where the RDY case joins later.
- `src/cpu.c` has exactly 256 `case 0x` lines and no `default:`. Tests `cpu.vectors.00` to `.ff` come from a nested foreach with no skip list (D-18).
- `cpu.unit` runs the issue 18 JSR case (6 exact cycles, pc 0x0155, s 0x7B), the jammed loop (each step is one read of 0xFFFF and nothing else changes) and a JAM at 0xFFFF that wraps PC to 0x0000. `core.profile` checks that create sets 0xEE twice.
- The `nesturbator_create` comment says the instance holds a 2A03 CPU that matches the vectors and does not yet run during frames. The ABI and the behaviour revision are unchanged. The README status line reads "Phase 2, the CPU", and Checks describes the 256 tests, `vecconv.02`, `cpu.unit` and `core.profile`.

## Task Commits

1. **Task 1 (tracer): stable unofficial opcodes**: `f12b940` (feat)
2. **Task 2: JAM, unstable opcodes, machine profile, all 256 tests**: `b951f2b` (feat)
3. **Task 3: unit tests**: `1c3da66` (test); **header and README**: `9476c2d` (docs)

## Files Created/Modified

- `src/cpu.c`: the unofficial helpers, `jam`, `store_sh`, `store_sh_abs` and 105 new cases (256 in all), with `default:` removed
- `src/internal.h`: `struct nesturbator__profile`, member `profile`, and the RP2A03G macros
- `src/instance.c`: create sets both profile fields
- `tests/cpu/test_vectors.c`: the harness sets both profile fields to 0xEE before each test
- `tests/CMakeLists.txt`: the 256-opcode foreach, `vecconv.02` run through the CPU, `cpu.unit` and `core.profile`
- `tests/cpu/test_cpu_unit.c`, `tests/core/test_profile.c`: new unit tests
- `include/nesturbator.h`: the create comment (comments only)
- `README.md`: status line and Checks

## Decisions Made

- After Task 2's foreach replaced their calls, `NESTURBATOR_OFFICIAL_OPCODES` and `NESTURBATOR_STABLE_UNOFFICIAL_OPCODES` were deleted, because nothing else read them. The stable list did exist in Task 1's commit `f12b940`.
- DCP and ISC compute the decrement and increment inline and do not call `dec` and `inc`, so N and Z are set only once, by CMP or SBC. The result is the same.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Docs with behaviour] README updated in Task 2**
- **Found during:** Task 2
- **Issue:** The plan puts all README edits in Task 3. Task 2 changed what the README described ("today the 151 official opcodes"; `vecconv.02` "converts only"). CLAUDE.md rule 6 requires the docs to change with the behaviour.
- **Fix:** The vectors paragraph now describes all 256 opcodes, JAM and the 0xEE profile, and `vecconv.02` now says it runs through the CPU.
- **Files modified:** README.md
- **Committed in:** b951f2b

**2. [Rule 3 - Blocking] One helper added: `store_sh_abs`**
- **Found during:** Task 2
- **Issue:** The four abs-indexed SHx/TAS forms share the same two address fetches before `store_sh`.
- **Fix:** `store_sh_abs` fetches lo and hi, then calls `store_sh`, so the D-15 mask stays in one helper.
- **Committed in:** b951f2b

**Total deviations:** 2 auto-fixed (1 docs, 1 helper). **Impact:** no change in scope.

## TDD Gate Compliance

Task 3 is `tdd="true"`, and `workflow.tdd_mode` is off. RED could not fail as written, because every behaviour it pins already existed: JSR ordering from 02-05, and JAM and the profile from Task 2 of this plan. This is fail-fast rule 1 (unexpected GREEN). The investigation found the behaviour was already implemented, as the plan's ordering intends. To show the tests are real, three mutations were applied in the working tree and each test failed on its target assertion:
- JSR reading ADH before the pushes gave pc 0x1355 and the wrong cycles 3 to 5.
- The jammed loop reading 0xFFFE was caught at cycle 0 of each later step.
- `lxa_magic` set to 0xFF gave `0xff != 0xee` in `core.profile`.

The sources were then restored from HEAD (byte-compared with a backup) and the tests passed. Commits: `test(02-06)` 1c3da66, then `docs(02-06)` 9476c2d. No `feat` commit, because Task 3 adds no production behaviour.

## Issues Encountered

None. Every new opcode matched the sample the first time it was built.

## Verification

- `ctest --preset ci -L vectors`: 100% of 256 passed. `ctest --preset ci -N -L vectors | grep -c 'cpu.vectors\.'`: 256 (236 after Task 1).
- `cpu.vectors.6b 0c 1c dc` pass (Pitfall 1).
- `grep -cE '^[[:space:]]*case 0x' src/cpu.c`: 256. No `default:` outside comments: 0.
- `ctest --preset ci -R '^vecconv\.02$' -V`: `65x02/02: 0 of 3 vectors failed`. No `vecconv.*` test has a PASS_REGULAR_EXPRESSION.
- `cpu.unit` and `core.profile`: 2 of 2 pass. `header.c` and `header.cxx` pass.
- The diff of `include/nesturbator.h` against `origin/main` changes only comment lines. `grep -c 'Phase 2' README.md`: 1.
- Workflows: `ci` exits 0 (315 tests), `asan` 0 (314, no runtime error or ASan report), `nofp` 0 (5), `hygiene` 0 (6).
- Tracer gate (interactive, end-of-phase, automated verify only): after commit f12b940 the 236 vector tests ran again and passed before Task 2 started.

## User Setup Required

None.

## Next Phase Readiness

- CPU-01 holds. Plans 02-07 and 02-08 (the full vector set and the nightly) can run the same `cpu.vectors` program over all 256 files.
- Deferred, as planned: settle LXA's constant against a console test ROM (Phase 3), the SHx RDY branch (with DMC DMA), and reporting the jam to hosts (Phase 3 run API).

---
*Phase: 02-the-cpu-matches-the-public-vectors*
*Completed: 2026-10-03*

## Self-Check: PASSED
