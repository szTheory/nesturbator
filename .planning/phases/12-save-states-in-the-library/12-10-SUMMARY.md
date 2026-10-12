---
phase: 12-save-states-in-the-library
plan: 10
subsystem: tests
tags: [save-states, hash-pins, ci, cmake]
status: complete

requires:
  - phase: 12-save-states-in-the-library
    provides: runner --hash-state (Plan 09)
provides:
  - state_format, state/<board> 0 and 60 hash rows and state/<board>/size rows in tests/runner/hashes.txt
  - third hashes.txt row shape "<key> <integer>" accepted by write_hashes, hash_keys and hash_inventory
  - CTest state.format_pin (D-15) and runner.state.every_frame (STATE-03)
affects: [12-11]

requirements-completed: [STATE-02, STATE-03]

actuals:
  tokens: 14000
  tasks: 2
  commits: 2

plan_head_before: 290eae65b01985e9180776e7b46b5c91a7a7dc9e
plan_head_after: 023787b3e809a44122676666addf3225dda2f146

key-files:
  created:
    - tests/cmake/state_format_pin.cmake
  modified:
    - tests/cmake/write_hashes.cmake
    - tests/cmake/hash_keys.cmake
    - tests/cmake/hash_inventory.cmake
    - tests/cmake/runner_state.cmake
    - tests/runner/hashes.txt
    - tests/CMakeLists.txt
    - README.md

key-decisions:
  - "The six pinned boards are dabg plus the Holy Mapperel keys in NESTURBATOR_STATE_HM_BOARDS (hash_keys.cmake); write_hashes resolves their ROM paths from NESTURBATOR_HOLYMAPPEREL_ROMS, including the derived m4tkrom."
  - "hashes.txt grew from 95 to 114 keys (19 state keys: 1 format, 6 boards x 3)."

duration: about 40 min wall (write_hashes alone 570 s wall on the throttled host)
completed: 2026-10-12
---

# Phase 12 Plan 10: State hash pins Summary

State format, size and frame 0 and 60 state hashes are pinned for six boards in hashes.txt, covered by the six-leg hash-equality job, plus a format-vs-macro check and a save-every-frame pin check.

## Accomplishments

- `write_hashes.cmake` runs `--hash-state 0 --hash-state 60` per board and emits `state/<board> N ticks T sha256 H` and `state/<board>/size S`, plus `state_format <n>` read from the header macro. `hash_keys.cmake` and `hash_inventory.cmake` accept the `<key> <integer>` shape; the inventory self-test fabricates all three shapes.
- `tests/runner/hashes.txt` regenerated: 19 added lines, 0 deleted.
- `state.format_pin` fails unless the `state_format` row equals `NESTURBATOR_STATE_FORMAT_VERSION` (verified: `state_format 2` fails, restoring passes).
- `runner.state.every_frame` runs dabg with `--hash-state` for every N from 0 to 180 and checks frames 1, 30, 60, 120, 180 equal the `dabg/boot/frame` rows; also requires 181 state lines.
- README documents the state pins and the 114-key inventory. The README and call-site check (every public `nesturbator_` function named in README and called from runner or libretro) passes.

## Pinned state sizes (format 1)

| Board | Size (bytes) |
|---|---|
| dabg (NROM) | 13087 |
| m1sxrom (MMC1) | 45869 |
| m2 (UxROM) | 13088 |
| m3 (CNROM) | 4896 |
| m4tkrom (MMC3, derived battery) | 21303 |
| m7 (AxROM) | 13088 |

## Deviations from Plan

None - plan executed exactly as written. The `cmake --workflow --preset asan` run was not performed here (orchestrator: ci plus targeted runs; asan runs on the PR).

## Verification

- `cmake --workflow --preset ci`: 521 of 524 passed. The three failures are the host-throttling timeouts the orchestrator described: `core.state_fork.M2_P128K_CR8K_V`, `core.state_fork.M3_P32K_C32K_H` and `core.state_fork.stress` (300 s CTest timeout; the full run took 364 s wall). A rerun of those three alone: the two ROM cases passed; `core.state_fork.stress` timed out again at 300 s wall with about 79 s CPU across the three (25% CPU), so it is wall-clock starvation, not a regression. Not touched by this plan.
- Targeted: `runner.write_hashes` (570 s wall under throttling), `runner.write_hashes.content`, `hash.inventory.self_test`, `policy.rom_hash_coverage*`, `state.format_pin`, `runner.state.every_frame` all pass.

## Flagged assumption

Byte identity of states across the six platforms is proved only by the CI hash-equality job on the PR; a local run proves one platform. Check the PR's CI run.

## Known Stubs

None.

## Self-Check: PASSED

Files exist (state_format_pin.cmake, hashes.txt rows); commits 288681e and 023787b are on the branch.
