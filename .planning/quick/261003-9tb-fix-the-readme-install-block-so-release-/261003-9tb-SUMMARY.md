---
phase: quick-261003-9tb
plan: 01
subsystem: release
tags: [release-please, readme, install, ctest]
status: complete
requires: []
provides:
  - README install block with the version on its own line
  - release.one_version_per_line ctest and self-test
affects: [release PR #2]
tech-stack:
  added: []
  patterns:
    - "cmake -P scan reads release-please-config.json extra-files with string(JSON)"
key-files:
  created:
    - tests/cmake/release_markers.cmake
    - tests/release/two_versions.md
  modified:
    - README.md
    - tests/cmake/check_install_line.cmake
    - tests/CMakeLists.txt
decisions:
  - "README install block sets NESTURBATOR_VERSION=<version> on its own line; the quoted URL takes tag and file name from $NESTURBATOR_VERSION"
  - "release.one_version_per_line scans every release-please extra-file; marked lines (blocks and inline x-release-please-version) may hold one version only"
pr: https://github.com/szTheory/nesturbator/pull/3
metrics:
  duration: 2 min
  completed: 2026-10-03
actuals:
  tokens: 3100
  tasks: 3
  commits: 2
plan_head_before: 83d6a1b9dd6b40cef41511e36f985ee5dfb1d2a8
plan_head_after: 36a971f5f76c91fb57e5c9fa2eb9f9fd848a7412
---

# Phase quick-261003-9tb Plan 01: README install line version fix Summary

The README install block now puts `NESTURBATOR_VERSION=0.0.0` on a line of its own, and the quoted curl URL takes both the release tag and the zip name from `$NESTURBATOR_VERSION`. release-please's change of one version per line therefore updates both. A new ctest fails on any marked line that holds two versions. PR #3 is open against main.

## Tasks

| Task | Name | Commit | Files |
|------|------|--------|-------|
| 1 | Two-line README install block; check_install_line.cmake resolves NESTURBATOR_VERSION (tracer) | d003550 | README.md, tests/cmake/check_install_line.cmake |
| 2 | release.one_version_per_line ctest plus a self-test on a fixture | 36a971f | tests/cmake/release_markers.cmake, tests/release/two_versions.md, tests/CMakeLists.txt, README.md |
| 3 | Full ci workflow, push, open the PR | (no code commit) | — |

PR: https://github.com/szTheory/nesturbator/pull/3, titled "fix: bump every version in the README install line", base main, state OPEN. Not merged. PR #2 untouched.

## Verification

- `cmake --workflow --preset ci`: 34 of 34 tests pass, including `release.one_version_per_line` and `release.one_version_per_line.self_test`.
- `check_install_line.cmake` passes against `build/ci/packages` on macOS arm64. With `-DVERSION=1.2.3` it fails and names both values.
- Tracer gate: a simulated release-please bump of the block to 9.9.9 resolves to `.../download/v9.9.9/nesturbator-9.9.9-libretro-macos-arm64.zip`. zsh expands the variable the same way.
- Against the old README (HEAD~1 before the fix), the scan fails with `README.md:326: 2 versions on one line`.

## Deviations from Plan

1. **[Rule 1 - Bug] Masked backslashes in the scan.** Before splitting lines into a CMake list, `release_markers.cmake` masks backslashes as well as semicolons and brackets. Otherwise a line ending in `\` would escape the separator and merge two lines, which would make the line numbers wrong. The change is in commit 36a971f.
2. **Commits split per task.** Task 1 was committed as "fix: bump every version in the README install line" and Task 2 as "test: fail when a release-please marked line holds two versions". The plan's single commit became these two per-task commits. The PR title matches the plan exactly.

## Known Stubs

None.

## Self-Check: PASSED

- FOUND: tests/cmake/release_markers.cmake, tests/release/two_versions.md, tests/cmake/check_install_line.cmake
- FOUND: commits d003550, 36a971f
- FOUND: PR #3 open against main
