---
phase: 13-rewind-and-run-ahead-in-retroarch
plan: 06
subsystem: retroarch-e2e
tags: [retroarch, frame-index, log-check, run-ahead, savestate-level]
requires:
  - phase: 13-05
    provides: green gcc 14 build of the phase's C code
provides:
  - frame_index tool and its two CTests
  - run_retroarch.cmake -DSTATE_ROM mode with lanes state-base and state-ahead, applying the .info level from a core copy in cores/
  - ra_state_outcome_checks (thumbnail and secondary-instance keys confirmed by outcome)
  - log_check.cmake, a measured level-scoped allowlist and nine fixture CTests
affects: [13-07]
key-files:
  created:
    - tests/retroarch/frame_index.c
    - tests/retroarch/state.cfg.in
    - tests/retroarch/log_check.cmake
    - tests/retroarch/log_allowlist.txt
    - tests/retroarch/fixtures/
  modified:
    - tests/retroarch/CMakeLists.txt
    - tests/retroarch/run_retroarch.cmake
    - README.md
key-decisions:
  - "The secondary-instance marker is a count, not a line: on 1.22.2 the key set true repeats the core setup lines, so more than 2 lines `[INFO] [Environ] SET_SUPPORT_NO_GAME` in a lane log fails; the core-copy file check never sees a copy on macOS."
  - "Allowlist entries hold only two measured lines; the run-ahead refusal warning is scoped serialized so a deterministic claim cannot be excused by it."
status: complete
requirements-completed: [LR-03]
commits: 2
plan_head_before: 6d6692e80ace7d973868fe9b7732561a9e5ff7f8
plan_head_after: 4b601c1d370d194452c4cb75bd54d8cb8aedd160
actuals:
  tokens: 14000
  tasks: 2
  commits: 2
completed: 2026-10-12
---

# Phase 13 Plan 06: frame_index, state lanes and the RetroArch log check Summary

RetroArch is shown to apply the `.info` savestate level from a core copy in its cores directory (frame 120 for serialized, 121 for deterministic), and every state lane now fails on an unexplained RetroArch `[ERROR]` line or rewind/run-ahead warning, with fixture CTests proving the check cannot pass vacuously.

## Commits

- 3f5e807: `frame_index`, `state.cfg.in`, STATE_ROM mode, `ra_state_outcome_checks`.
- 4b601c1: `log_check.cmake`, `log_allowlist.txt`, fixtures, CTests, wiring, README.

## Local STATE_ROM runs (RetroArch 1.22.2, foreground)

Serialized .info (committed):
```
-- [state-base] RetroArch's frame 120 equals the runner's frame 120
-- [state-base] frame_index: .../shot-state-base.bmp 120
-- [state-ahead] RetroArch's frame 120 equals the runner's frame 120
-- [state-ahead] frame_index: .../shot-state-ahead.bmp 120
-- RetroArch applied the .info level 'serialized': state-ahead showed frame 120
```
Scratch deterministic .info (in build/, not committed):
```
-- [state-base] RetroArch's frame 120 equals the runner's frame 120
-- [state-ahead] RetroArch's frame 121 equals the runner's frame 121
-- [state-ahead] frame_index: .../shot-state-ahead.bmp 121
-- RetroArch applied the .info level 'deterministic': state-ahead showed frame 121
```
The game-mode and first battery-save commands from ci.yml still pass locally. The lane also fails if the runner's frames 120 and 120+k_eff are identical, so the lane cannot be vacuous.

## Secondary-instance measurement

Scratch run: state-ahead, deterministic .info, `run_ahead_secondary_instance = "true"`. No new line text appears and no core library copy was found outside `RA_DIR/cores` (so the file-copy check alone did not fail). What differs is a count: `[INFO] [Environ] SET_SUPPORT_NO_GAME: yes.` appears 4 times, against 2 with the key false (every lane, both levels). `ra_state_outcome_checks` fails on more than 2; with the scratch config the run then failed naming the log. The file-copy check is kept as written; the comment in `run_retroarch.cmake` records the measurement.

Other local edits (not committed): `states/probe.png` made the run fail naming the file; deleting the comment above the first allowlist entry made log_check fail; appending `[ERROR] [Core] injected` to a lane log failed with `un-allowlisted line:`. The serialized-run state-ahead log fails at level deterministic (the run-ahead warning is not excused there).

## Allowlist entries (RetroArch 1.22.2)

| Level | Regex | Measured in | Why benign |
|---|---|---|---|
| any | `^\[ERROR\] failed_to_start_audio_driver` | state-base and state-ahead, both levels (4 per lane) | No audio device on a headless host; audio is checked by hashes (D-28). |
| serialized | `^\[WARN\] \[Run-Ahead\] Run-Ahead unavailable because this core lacks deterministic save state support\.$` | state-ahead with serialized .info (2 per lane) | RetroArch correctly refuses run-ahead for a serialized claim; shows the claim is applied. |

## Deviations from Plan

**1. [Rule 2 - Missing critical] Extra fixtures and CTests**
- **Issue:** the plan's behaviours (missing `[NetCMD]`, failed-load bypass of the allowlist, uncommented entry, pass at serialized) had no test in the listed four.
- **Fix:** added `log-no-netcmd.txt`, `log-failed-load.txt`, `allow-failed-load.txt`, `allow-uncommented.txt`, and tests `retroarch.log_check.serialized`, `.level_serialized`, `.no_netcmd`, `.failed_load`, `.uncommented`.
- **Commit:** 4b601c1

**2. [Rule 1 - Bug] frame_index test regex**
- `" 37( |$)"` did not match the tool's line end in CTest; changed to `" 37[ \n]"`. Commit 3f5e807.

**3. Marker is a count (see above)** rather than a distinct line; the plan allowed no marker, a count check was added as the stronger outcome.

Total deviations: 3 (1 Rule 2, 1 Rule 1, 1 measurement note). Impact: none on scope; Plan 07 calls `ra_state_outcome_checks` and `nesturbator_ra_log_check` after its lanes.

## CI status checked at start

PR #44 head 6d6692e: all jobs green except `asan`, still running when last seen (retroarch-e2e, Linux, nofp, Windows, macOS, hygiene green); the gcc 14 fix from plan 05 worked. Plan 06 commits pushed; new run pending.

## Verification

`cmake --workflow --preset ci` (546 tests) and `--preset hygiene` pass; frame_index.c passes gcc:14 -O3 with the project warning set and clang-format 18.

## Known Stubs

None.

## Self-Check: PASSED

Files exist (frame_index.c, state.cfg.in, log_check.cmake, log_allowlist.txt, fixtures); commits 3f5e807 and 4b601c1 are ancestors of HEAD.
