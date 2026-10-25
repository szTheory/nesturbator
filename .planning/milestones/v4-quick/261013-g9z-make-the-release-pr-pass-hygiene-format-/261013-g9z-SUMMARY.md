---
phase: quick-261013-g9z
plan: "01"
subsystem: release
tags: [cmake, clang-format, release-please, hygiene]
requires: []
provides:
  - Stable numeric-only updates of release-managed public-header macros
  - Portable guard and marker regression with formatter-backed hygiene coverage
affects: [release, ci, hygiene]
tech-stack:
  added: []
  patterns: [narrow clang-format guards, CMake mutation tests]
key-files:
  created: [tests/cmake/release_version_format.cmake]
  modified: [include/nesturbator.h, tests/CMakeLists.txt, tests/hygiene/CMakeLists.txt, README.md]
key-decisions:
  - Keep formatting protection confined to the three release-managed version macros.
  - Run portable contract checks in CI and actual clang-format 18 mutations in hygiene.
requirements-completed: [QUICK-261013-g9z]
actuals:
  tokens: 2526
  tasks: 1
  commits: 1
commits: 1
plan_head_before: 3f39694fd4b8ea79b7cde7347a06b2e3840ecc30
plan_head_after: c79fca3ba3c1fb573d8ca3ab0d3530183b6d7272
coverage:
  - id: D1
    description: Release digit-width changes preserve header bytes under clang-format 18.
    requirement: QUICK-261013-g9z
    verification:
      - kind: integration
        ref: "cmake --workflow --preset ci: release.version_format"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset hygiene: hygiene.release_version_format"
        status: pass
    human_judgment: false
duration: 8min
completed: 2026-10-13
status: complete
---

# Quick Task 261013-g9z: Release Version Formatting Summary

**A three-macro formatting guard preserves release-please markers across numeric digit-width changes, with portable and formatter-backed mutation coverage.**

## Accomplishments

- Protected exactly MAJOR, MINOR and PATCH with the public header's existing block-comment formatting convention. Formatting resumes immediately after PATCH, before the ABI declaration.
- Added `release.version_format` to CI and `hygiene.release_version_format` to hygiene. Both check eight synthetic versions and nine malformed fixtures using the same validator. Hygiene also compares full-header clang-format 18 output and requires an unguarded PATCH-10 sensitivity control to change bytes.
- Updated the README Releases section and public-header comment together. Library version 0.1.9, ABI 1 and behaviour revision 6 remain unchanged.

## Verification

### RED

Before modifying the public header, the new portable script failed on the planned assertion: `release_version_format: missing format-off guard before version macros`. This was the intended contract failure, with no syntax, tool or fixture error.

Separately, actual clang-format 18 formatting of the original header after numeric-only PATCH 9-to-10 replacement inserted an extra space before the MAJOR and MINOR inline comments. The full-header comparison printed `byte_identical: False`; the diff showed only those two alignment changes.

### GREEN

After the narrow guard was added, standalone portable and formatter-backed modes each reported `8 numeric mutations and 9 negative controls passed`. Fixtures cover missing off/on guards, a region extended over ABI, missing markers and duplicate macros for all three components. Versions cover patch 9, 10, 99 and 100, plus major/minor digit-width transitions. The unguarded PATCH-10 sensitivity check also passed.

Final complete workflows ran from the final unchanged files:

- `cmake --workflow --preset ci`: exit 0; **634/634 tests passed**, 113.24 seconds of test time, including `release.version_format`; all three release archives generated.
- `cmake --workflow --preset hygiene`: exit 0; **13/13 tests passed**, 49.11 seconds, including `hygiene.format` and `hygiene.release_version_format`.
- `git diff --check`: passed.

The first CI run also passed but was repeated because a negative-fixture refinement occurred after it started. The final run above is the completion evidence. Final logs are ignored build artifacts at `build/quick-release-ci-final.log` and `build/quick-release-hygiene.log`.

## Task Commits

1. **Make tool-managed version updates format-stable and prove the contract** — `c79fca3` (`fix(release): preserve version macro formatting across digit widths`). Exactly the five implementation/test/documentation paths were staged. Configured GitHub noreply identity and the repository pre-commit hygiene hook were used.

## Decisions Made

Followed the plan's single atomic implementation commit. Test-first RED/GREEN evidence was recorded without separate failing-test commits; workflow TDD mode is disabled and the task explicitly requires the five files to land together.

## Deviations from Plan

None. A one-line refinement made duplicate-macro fixtures independent of missing-marker fixtures, and the complete workflows were verified after the final edit.

## Issues Encountered

None affecting delivery. No new dependencies, installs, network surfaces, core changes or release-number changes were introduced. Generated fixtures stay in distinct test-specific binary directories.

## Handoff

The unrelated three-line Codex routing edit in `.planning/config.json` is preserved and uncommitted. Planning artifacts and state updates are reserved for the orchestrator. No push, PR creation or merge was performed. Local verification ran on macOS arm64; hosted cross-platform CI remains outside this quick execution step.

## Self-Check: PASSED

The regression script and summary exist, implementation commit `c79fca3` is an ancestor of HEAD, no files were deleted by the commit, and `git diff --check` passed. The working tree contains only the preserved config change and the orchestrator's quick-task planning artifacts.
