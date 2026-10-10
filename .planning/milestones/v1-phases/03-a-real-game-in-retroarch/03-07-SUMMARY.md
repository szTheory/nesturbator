---
phase: 03-a-real-game-in-retroarch
plan: 07
subsystem: conformance-testing
tags: [AccuracyCoin, PPU, RAM-inspection, C17]
requires:
  - phase: 03-06
    provides: Pinned game fixture conventions and licensed ROM manifest
  - phase: 03-12
    provides: Current PPU/display integration used by game execution
provides:
  - Pinned MIT AccuracyCoin fixture with verified SHA-256
  - Bounded runner mode that drives pages 2 and 17 and reads test results from instance RAM
  - Sorted scoreboard of all 11 named page results
affects: [GAME-04, PPU-register-behavior, conformance-runner]
actuals:
  tokens: 18266
  tasks: 1
  commits: 1
  plan_head_before: 1a03a8553e3134db0294229c7da7cd3ef2f464fe
  plan_head_after: 14d006c511098cf72d5d5ea1886ebd0fb4674f56
tech-stack:
  added: []
  patterns:
    - Parse ROM-resident test names and result pointers, then read each result through a side-effect-free public RAM peek.
    - Keep conformance rows sorted by exact ROM test names and compare result bytes against a committed scoreboard.
key-files:
  created:
    - tests/roms/accuracycoin.nes
    - tests/accuracy/scoreboard.txt
    - tests/accuracy/test_scoreboard.c
    - .planning/phases/03-a-real-game-in-retroarch/03-07-red-evidence.json
  modified:
    - runner/main.c
    - src/ppu.c
    - src/instance.c
    - src/internal.h
    - include/nesturbator.h
    - tests/CMakeLists.txt
    - tests/ppu/test_registers.c
    - tests/core/test_api.c
    - tests/roms/manifest.txt
    - README.md
decisions:
  - "Expose a read-only CPU RAM peek for conformance tooling; accept RAM mirrors through $1FFF and reject other addresses without side effects."
  - "Use the exact ROM-resident AccuracyCoin directory as the source of names and result locations."
  - "Model PPU I/O bus decay at the conservative end of the documented 3–30 ms range using integer dot counts."
metrics:
  duration: 57min
  completed: 2026-10-08
  status: complete
requirements-completed: [GAME-04]
coverage:
  - id: D1
    description: AccuracyCoin pages 2 and 17 execute to completion and every declared test passes from emulated RAM.
    requirement: GAME-04
    verification:
      - kind: integration
        ref: "ctest --test-dir build/ci -R '^(accuracycoin.page2|accuracycoin.page17)$' --output-on-failure"
        status: pass
    human_judgment: false
  - id: D2
    description: The named result bytes are sorted and match the checked-in scoreboard.
    requirement: GAME-04
    verification:
      - kind: unit
        ref: tests/accuracy/test_scoreboard.c
        status: pass
    human_judgment: false
---

# Phase 03 Plan 07: AccuracyCoin RAM Scoreboard Summary

**AccuracyCoin’s six page 2 tests and five page 17 tests now run under a bounded script and match named result bytes read from CPU RAM.**

## Accomplishments

- Added the exact 40,976-byte MIT AccuracyCoin ROM from commit `673ef550db296136d52229961e7d39366116882a`; manifest SHA-256: `4fe8c2bc9abc6f4d418da47b73f62cba89fcacd950fae763097a1681d650e839`.
- Added `--accuracycoin-page` runner mode. It reads the ROM’s directory, selects page 2 or 17, runs the tests within a fixed frame bound, reads result bytes through `nesturbator_peek_cpu_ram`, prints stable name/status/code rows, and checks the scoreboard.
- Added the public read-only RAM peek API with mirror and invalid-argument coverage, plus README and header documentation.
- Fixed PPU I/O open-bus retention/decay, `$2007` buffered memory reads, palette reads, and greyscale handling exposed by page 17. Added focused PPU register regressions.
- Updated the Phase 3 public API declaration guard for the intentional API addition.

## AccuracyCoin Results

| Page | Tests | Result |
|---|---:|---|
| 2 | 6 | 6 PASS (`0x01`) |
| 17 | 5 | 5 PASS (`0x01`, except PPU Read Buffer `0x41`) |

The `0x41` result is the suite’s accepted revision-G-or-later result for PPU Read Buffer. All 11 names and bytes are present in sorted order in `tests/accuracy/scoreboard.txt`.

## TDD Gate Compliance

- **RED:** The committed `accuracycoin.page2` test failed because the runner rejected the not-yet-implemented option. The sanitized JUnit evidence record passes `gsd-tools check tdd-red-evidence` with `RED_EVIDENCE_OK`.
- **GREEN:** Commit `14d006c` implements the runner, PPU fixes, fixture, API and scoreboard. Both selected AccuracyCoin pages pass from emulated RAM.
- **REFACTOR:** No separate refactor was needed.

## Verification

- Focused checks: AccuracyCoin pages 2 and 17, scoreboard, public API, and PPU register tests passed (5/5).
- `cmake --workflow --preset ci`: configure and build passed; 341/342 tests passed. The only failure was the existing local `retroarch.testframe` GUI-session abort (`Subprocess aborted`, empty stdout and stderr). `deferred-items.md` records it for separate investigation.
- `scripts/hygiene.sh --tree` and `git diff --check` passed.
- Workflow command was run locally on 2026-10-08; there is no hosted Actions run ID for this execution.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Restored `$2007` buffered reads from PPU memory**
- **Found during:** Task 1 page 17 execution
- **Issue:** The in-progress implementation filled the read buffer from the prior PPU I/O bus value instead of the addressed PPU memory.
- **Fix:** Read the PPU memory address before returning the previous buffer value; add delayed-read and palette-refill regressions.
- **Files modified:** `src/ppu.c`, `tests/ppu/test_registers.c`
- **Commit:** `14d006c`

**2. [Rule 2 - Missing critical behavior] Completed PPU I/O open-bus and palette behavior**
- **Found during:** Task 1 page 17 execution
- **Issue:** AccuracyCoin also exposed missing PPU I/O latch behavior/decay and palette read bus-bit and greyscale handling.
- **Fix:** Retain the PPU I/O bus separately from CPU open bus, apply deterministic dot-count decay, and return palette data with the appropriate bus bits and greyscale mask.
- **Files modified:** `src/internal.h`, `src/ppu.c`, `tests/ppu/test_registers.c`
- **Commit:** `14d006c`

**3. [Rule 1 - API guard baseline] Refreshed the temporary Phase 3 declaration hash**
- **Found during:** Full CI workflow
- **Issue:** The API guard correctly rejected the newly required public RAM peek declaration.
- **Fix:** Updated the Phase 3 baseline to the normalized declaration stream for the documented API; its mutation self-test remains enabled.
- **Files modified:** `tests/cmake/vector_api_policy.cmake`
- **Commit:** `14d006c`

## Task Commits

- `1a03a85` `test(03-07): add AccuracyCoin page scoreboard checks` (RED)
- `14d006c` `feat(03-07): run AccuracyCoin page scoreboards from RAM` (GREEN)

## Self-Check: PASSED

- The pinned ROM, manifest entry, scoreboard, runner mode, public API, tests and documentation are present.
- Both task commits are ancestors of the task implementation commit recorded in `plan_head_after`.
- The ROM SHA-256 matches the manifest. Hygiene passed with no personal paths or unlisted ROM bytes.

*Phase: 03-a-real-game-in-retroarch · Completed: 2026-10-08*
