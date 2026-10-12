---
phase: 12-save-states-in-the-library
plan: 06
subsystem: core-library
tags: [save-states, rejection, validation, sanitizers, c17]

requires:
  - phase: 12-save-states-in-the-library
    provides: load pipeline and field lists (Plan 01), board state ops (Plan 02), shared state_test.h helpers (Plan 03), fork matrix (Plans 04 and 05)
provides:
  - core.state_reject, the exhaustive STATE-04 rejection test (ci full, asan --short)
  - nesturbator__state_bounds and the NESTURBATOR__SIO_RECORD visitor mode (internal test entry point)
  - st_refused and st_refused_with in tests/state_test.h
affects: [12-07 and later plans that extend the field list, 12-verification]

actuals:
  tokens: 30000
  tasks: 2
  commits: 2

plan_head_before: adb41269103b93c43f666015fdc7589a90ef968b
plan_head_after: 5506641930ae5c025d874f930fc22bdfe6007128

tech-stack:
  added: []
  patterns:
    - "A fifth visitor mode (RECORD) walks the same field list, so a sweep test keeps no second copy of the layout"
    - "Test locates a struct member in the state by saving, flipping the member's bytes in the instance and diffing the two saves"

key-files:
  created: [tests/core/test_state_reject.c]
  modified: [src/state.h, src/state.c, src/internal.h, tests/state_test.h, tests/CMakeLists.txt, include/nesturbator.h, README.md]

key-decisions:
  - "struct nesturbator__state_bound declares hi as int64_t (the plan said uint64_t): signed fields compare correctly under the warnings-as-errors build; an unbounded unsigned 64-bit field records INT64_MAX"
  - "ppu.v and ppu.t are validated to 0x7fff (the core masks every increment with 0x7fff); the bounds table row said so and the field list had 0xffff"
  - "RECORD emits a record for every field narrower than its type and for cross-field participants (flagged cross)"

patterns-established:
  - "Rows are named by C member path; the test asserts each row's hi equals the recorded bound and every recorded field has a row"

requirements-completed: [STATE-04]

coverage:
  - id: D1
    description: "Cross-ROM and no-cartridge loads give STATE_CARTRIDGE; format_version 0 and 2 give FORMAT; header_size 63 and 65 give CHECKSUM then INVALID; reserved bytes give INVALID"
    requirement: STATE-04
    verification:
      - kind: unit
        ref: "ctest --preset ci -R ^core\\.state_reject$ (case_cross_rom, case_header)"
        status: pass
    human_judgment: false
  - id: D2
    description: "Every truncation length below S, S + 1, NULL arguments, a flip of every byte with the D-04 class per offset, and the two-fault ordering"
    requirement: STATE-04
    verification:
      - kind: unit
        ref: "ctest --preset ci -R ^core\\.state_reject$ (case_truncation, case_flip, case_two_faults)"
        status: pass
    human_judgment: false
  - id: D3
    description: "CRC-recomputed sweep of every bounded field past each bound and at the type limits gives INVALID; at-bound table, synthesis sum invariant, eval pairing and the ppu_ticks gap boundary behave as the plan states; every refusal byte-identical"
    requirement: STATE-04
    verification:
      - kind: unit
        ref: "ctest --preset ci -R ^core\\.state_reject$ (case_field_sweep, case_coverage, case_at_bound, case_synth, case_eval, case_ppu_gap)"
        status: pass
      - kind: unit
        ref: "ctest --preset asan -R ^core\\.state_reject$ (--short, no sanitizer report)"
        status: pass
    human_judgment: false

duration: 90min
completed: 2026-10-12
status: complete
---

# Phase 12 Plan 06: Rejection sweeps Summary

**Exhaustive all-or-nothing load tests: a RECORD visitor mode feeds a CRC-recomputed sweep of every bounded field, and every header byte, length and cross-ROM pair is refused with its own status and a byte-identical instance, clean under ASan and UBSan.**

## Performance

- **core.state_reject:** 5.9 s in ci (full), 10.4 s in asan with `--short` (registered TIMEOUT 300, COST 60).
- **core.state_fork:** 139.8 s of CPU, unchanged by this plan. In the full `cmake --workflow --preset ci` run and in a later targeted `ctest` run it timed out at 300 s of wall clock while getting only about 6 to 8 percent of a CPU (host load; 1622 s and 2169 s wall for the same CPU time). Run directly with no timeout it passed (exit 0, 139.77 s user). Every other test (491 of 492) passed in the workflow.

## Accomplishments

- `NESTURBATOR__SIO_RECORD` and `nesturbator__state_bounds(inst, out, cap)`: one record (offset, width, signedness, lo, hi, cross) per field narrower than its type or taking part in a cross-field rule (153 to 159 records per board). SIZE, WRITE, VALIDATE and APPLY are untouched; core.state and core.state_fork stay green.
- `core.state_reject`: cross-ROM matrix over nine ROMs plus an empty instance; version, header_size and reserved byte cases; every truncation length; a flip of every byte with the D-04 status per offset class (RAM bytes stride 97 under `--short`); two-fault ordering; sweep of every recorded bound at hi + 1, the type maximum, lo - 1 and the type minimum; status bits 0 to 4; the at-bound table (one printed line per row); synthesis sum, `eval_copy_remaining` pairing; the `ppu_ticks` gap at TPF - 1 (loads, next frame OK), TPF, TPF + 1, `ppu_ticks = ticks + 1` and `ppu_ticks = 0` with ticks 2^50.
- A coverage check asserts every recorded field has a named row or a dedicated case, so a field added to the list without a test row fails the build of the suite.
- README and the public header comment describe the PPU catch-up rule and the new test.

## Task Commits

1. **Task 2 core (bounds recorder and the v/t fix)** - `46cf046` (feat)
2. **Task 1 and Task 2 tests (core.state_reject, helpers, registration, docs)** - `5506641` (test)

The two tasks were written together because the sweep needs the recorder; the commits split library from test.

## Deviations from Plan

**1. [Rule 1 - Bug] ppu.v and ppu.t allowed 16 bits**
- **Found during:** Task 2 (at-bound table row `v`/`t` 0x7fff versus the field list's 0xffff)
- **Issue:** a state with bit 15 set in v or t loaded; the core only ever holds 15 bits.
- **Fix:** both validated to 0x7fff in `src/state.c`.
- **Commit:** 46cf046

**2. [Planning advisory] `hi` is int64_t**
- The plan declared `uint64_t hi`; signed fields need a signed upper bound. Unsigned 64-bit fields have no bound and record INT64_MAX.

**3. [Table row correction] noise.counter bound is 0xfff**
- The table's "timers and counters 0x7ff" row does not cover `noise.counter`, whose use-site bound is 0xfff (largest noise period 4068). The row in the test uses 0xfff; the field list was right.

**4. Rows came out right otherwise.** Every other at-bound row (cpu.jammed giving STOP_JAM, synthesis sum rows, `eval_copy_remaining`, the TPF - 1 gap finishing a frame) matched the plan with no code change. The test's own first run exposed a bug in my `locate` helper (stale baseline after earlier rows ran a frame), fixed in the test.

**Total deviations:** 1 auto-fixed bug, 2 plan-text corrections.

## Known Stubs

None.

## Threat Flags

None. T-12-04, T-12-05 and T-12-06 are mitigated and covered by the sweeps above.

## Verification

- `ctest --preset ci -R ^core\.state_reject$`: passed (5.9 s).
- `ctest --preset asan -R ^core\.state_reject$`: passed (10.4 s), no sanitizer report.
- `cmake --workflow --preset ci`: 491 of 492 passed; the one failure is the `core.state_fork` wall-clock timeout under host contention described above (it passes when run directly).
- `cmake --workflow --preset asan` was not run in full; the asan build and `core.state_reject` were run on their own.

## Self-Check: PASSED

Files exist: tests/core/test_state_reject.c; commits 46cf046 and 5506641 are on the branch.
