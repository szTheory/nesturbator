---
phase: 11-tune-up-and-state-readiness
plan: 03
subsystem: runner
tags: [version, boundary, docs]
requires: [11-01, 11-02]
provides: [nesturbator-run --version, runner.version, core.boundary]
affects: [runner/main.c, tests/cmake/runner_version.cmake, tests/core/test_boundary.c, tests/CMakeLists.txt, include/nesturbator.h, README.md]
key-files:
  created: [tests/cmake/runner_version.cmake, tests/core/test_boundary.c]
  modified: [runner/main.c, tests/CMakeLists.txt, include/nesturbator.h, README.md]
decisions:
  - "D-10 --version prints one line from nesturbator_get_version at run time; D-11 libretro version string unchanged"
requirements: [TUNE-09]
status: complete
commits: 3
plan_head_before: e55adece36536b134cd7724b4aaf2a446c4a79e7
plan_head_after: 96d7747be1c07dddcf683d10676f135ed4be0d39
actuals:
  tokens: 9000
  tasks: 3
  commits: 3
---

# Phase 11 Plan 03: --version, frame-boundary checks and docs Summary

`nesturbator-run --version` prints `nesturbator-run X.Y.Z` from `nesturbator_get_version`, pinned end to end by `runner.version`; `core.boundary` shows every frame boundary free of frame-scoped pointers and buffered PCM.

## Done
- Task 1 (0f04e83): `show_version` option handled when the loop reaches it; usage line; `runner.version` (exact stdout, empty stderr, exit 0, `--version --bogus` also 0) and `runner.usage.version_after_bad` (exit 2). `libretro/libretro.c` untouched.
- Task 2 (bf46c91): `core.boundary` over test-card frames, 60 APU frames (798/799 alternation seen), JAM, `ERR_BUFFER_TOO_SMALL` and unload/reload.
- Task 3 (96d7747): README synopsis, `--version` bullet, public function list under "Using the library" naming every declared function, test descriptions; header comment sentence; runner head comment.
- `cmake --workflow --preset ci`: 488/488. `asan`: 483/483. `scripts/hygiene.sh`: exit 0. The README/call-site shell loop exits 0.

## D-17 status confirmation
```
$ git grep -nE "return [3-9][0-9]*;|exit\( *[3-9]|\? *[3-9][0-9]* *:|: *[3-9][0-9]* *;" -- runner
runner/main.c:791:                    return ls == NESTURBATOR_RUN_SAVE_MISMATCH ? 4 : 1;
$ git grep -nE "EXIT (3|77)\b" -- tests/CMakeLists.txt
(no output)
```
The set of literals above 2 is exactly {4}. Library enum: OK 0, ERR_ARGUMENT 1, ERR_STRUCT_SIZE 2, ERR_ABI 3, ERR_NO_MEMORY 4, ERR_BUFFER_TOO_SMALL 5, STOP_JAM 6, ERR_CARTRIDGE 7, unchanged.

## Deviations from Plan
- [Rule 1 - Bug] The first `core.boundary` image cap (40000) was below the 40976-byte image; raised to 45000 in the new test before commit.
- The README function list says `nesturbator_get_palette` writes the palette as XRGB8888 (from the header).

## Known Stubs
None.

## Self-Check: PASSED
