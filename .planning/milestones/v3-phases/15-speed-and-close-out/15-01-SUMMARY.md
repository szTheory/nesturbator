---
phase: 15-speed-and-close-out
plan: 01
subsystem: runner
tags: [runner, signals, readme, ctest]
requires: []
provides:
  - runner.options.readme (two-way --help vs README runner-option check)
  - runner.stop.inherited_ignore (POSIX)
  - Phase 14 review disposition closed
affects: [15-02, 15-03]
key-files:
  created: [tests/cmake/runner_options.cmake]
  modified: [runner/main.c, runner/stop.c, README.md, tests/CMakeLists.txt, tests/cmake/runner_stop.cmake, .planning/phases/14-runner-process-contract/14-REVIEW-DISPOSITION.md]
decisions:
  - "Signal-test TIMEOUTs are 30 s hang detectors, not 10 s, so they cannot fire inside the helpers' own deadlines."
metrics:
  duration: 25m
  completed: 2026-10-12
status: complete
commits: 5
plan_head_before: aa816236eafa16c8c85d649d61e1b6496050339a
plan_head_after: f6e20a0083ddf12c91c02e20ae87dbada435c789
actuals:
  tokens: 9000
  tasks: 3
  commits: 5
---

# Phase 15 Plan 01: Runner debt and the docs-match-code check Summary

A ci test now fails when `nesturbator-run --help` and the README runner bullets disagree, and the POSIX runner honours an inherited SIG_IGN.

## What was built

- **runner.options.readme** (`tests/cmake/runner_options.cmake`): compares options printed by `--help` with `` - `--opt` `` bullets between `## The runner` and `## The libretro core`, both ways. Before the doc fixes it found four options without a bullet-or-help line (`--movie`, `--help` missing in the README; `--accuracycoin-page`, `--scoreboard` missing in both). After: all runner options agree, test passes. Mutation check (removing the `--hash-audio` bullet) failed with `option --hash-audio is in --help but has no README bullet`, and the README was restored byte for byte.
- `--help` gained `--accuracycoin-page N` and `--scoreboard FILE` and a synopsis line; README gained bullets for `--movie`, `--help`, `--accuracycoin-page`, `--scoreboard`, and a status paragraph saying v2 shipped and v3 is closing.
- **WR-01** (`runner/stop.c`): `signal()` kept; `install_one()` puts back an inherited SIG_IGN; the handler saves and restores errno. `runner.stop.inherited_ignore` RED: exit 1 with `stopped by signal after frame 2`. GREEN: exit 0, `frame 5 ticks` printed; passes in ci and asan.
- **WR-02**: `NESTURBATOR_SIGNAL_TIMEOUT` and `runner.stop.win_console` TIMEOUT are 30 s hang detectors with a comment saying so.
- **WR-03**: README tells POSIX launchers to keep reading or redirect standard output.
- 14-REVIEW-DISPOSITION.md: WR-01 to WR-03 fixed with SHAs, IN-01 to IN-04 deferred, `open: 0`.

## Commits

- 1b5c273 feat(15-01): options check, help and README fixes
- 12daaca test(15-01): inherited_ignore (RED)
- 59902dd fix(15-01): stop.c SIG_IGN and errno (GREEN)
- b0d18c6 fix(15-01): 30 s timeouts, README stdout sentence
- f6e20a0 docs(15-01): Phase 14 disposition

## Deviations from Plan

None in behaviour. Note: `clang-format` is not installed on this machine; `runner/stop.c` and `runner/main.c` were hand-formatted to the surrounding style, and `cmake --workflow --preset hygiene` passed.

## Verification

`cmake --workflow --preset ci` and `cmake --workflow --preset hygiene` pass locally.

## Known Stubs

None.

## Self-Check: PASSED
