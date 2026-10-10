---
phase: 05-tune-up-and-v1-debt
plan: 04
subsystem: ci
tags: [ctest, ci, nightly, action-pins]
requires: [05-01, 05-03]
provides:
  - "CTest runs four-wide from the dev, ci, ci-msvc and asan test presets, with COST on the three long tests"
  - "Nightly suite-flake job (ctest --repeat until-fail:3 --schedule-random) wired into the rolling issue"
  - "upload-artifact v7.0.2 and download-artifact v8.0.2 pinned by full SHA"
  - "05-CI-RECORD.md with before timing, ccache decision, pin, label and AccuracyCoin review"
affects: [ci, nightly, release]
tech-stack:
  added: []
  patterns: ["COST property so a cold CI build starts long tests first"]
key-files:
  created: [.planning/phases/05-tune-up-and-v1-debt/05-CI-RECORD.md]
  modified: [CMakePresets.json, tests/CMakeLists.txt, .github/workflows/nightly.yml, .github/workflows/ci.yml, .github/workflows/release.yml, tests/cmake/nightly_workflow_policy.cmake, tests/cmake/release_policy.cmake, README.md, .planning/preparation/ENGINEERING.md]
key-decisions:
  - "execution.jobs 4 with COST, no RUN_SERIAL or RESOURCE_LOCK: no collision found locally"
  - "No ccache and no actions/cache: compile share 4 to 22 percent, under the 40 percent rule"
  - "AccuracyCoin pin 673ef550 reviewed, not moved (7 commits ahead upstream)"
requirements-completed: [TUNE-01, TUNE-02]
status: complete
duration: 12 min
completed: 2026-10-10
commits: 3
plan_head_before: 1e1f60910111819bdd75b35096a04b5fe212e3be
plan_head_after: 0815acad017270341a2222d6e765d6352edd3638
actuals:
  tokens: 9000
  tasks: 3
  commits: 3
coverage:
  - deliverable: "Parallel CTest with COST on three tests; ci and asan pass"
    verification:
      - kind: command
        ref: "cmake --workflow --preset ci; cmake --workflow --preset asan"
        status: pass
    human_judgment: false
  - deliverable: "Nightly suite-flake job and current artifact pins; policy checks pass"
    verification:
      - kind: command
        ref: "cmake --workflow --preset hygiene; release_policy.cmake SELFTEST"
        status: pass
    human_judgment: false
  - deliverable: "After timing in 05-CI-RECORD.md"
    human_judgment: true
    rationale: "Needs the phase PR's CI run; post-PR gate in /gsd-verify-work 5"
---

# Phase 5 Plan 04: Parallel CTest, nightly flake hunt and pin refresh Summary

CTest now runs four tests at a time from the test presets with COST on the three long tests, a nightly `suite-flake` job runs the suite three times in random order, and the two artifact actions moved to their current patch releases.

## Tasks

1. Parallel CTest (615db60): `"jobs": 4` on dev, ci, ci-msvc and asan; COST 1000, 200 and 50 on `vectors.registration_policy`, its self test and `runner.write_hashes`. Local `ctest --preset ci`: 31.53 s real for 357 tests, 100 percent passed. ci and asan workflows exit 0.
2. Nightly flake job and pins (7d7bccc): `suite-flake` job (ubuntu-24.04, 10 minute initial timeout) added to `report` needs, RESULT and issue body. Pins resolved with `gh api` as type `commit` and equal to the planned SHAs. Hygiene (8 tests) and `release_policy.cmake` self test pass; no old pin remains.
3. Docs and record (0815aca): README, ENGINEERING section 5, and `05-CI-RECORD.md`.

## Deviations from Plan

**1. [Rule 3 - Blocking] release_policy.cmake fixtures carried the old download-artifact SHA**
- **Found during:** Task 2
- **Issue:** `tests/cmake/release_policy.cmake` lines 260-261 hard-code the v8.0.1 `download-artifact` line as a mutation target; the plan only named `nightly_workflow_policy.cmake`.
- **Fix:** Updated the SHA there too so the self test mutates the real line.
- **Files modified:** tests/cmake/release_policy.cmake
- **Commit:** 7d7bccc

**Total deviations:** 1 auto-fixed (1 blocking). **Impact:** none beyond one extra file.

## Left for the hosted run

- The `## After` section of `05-CI-RECORD.md` reads "After: pending the phase PR's CI run" with both gh commands. `/gsd-verify-work 5` fills it after `/gsd-ship 5` and a completed CI run (post-PR gate). I did not fabricate any after number.
- RESEARCH A5 and A7 (no test collision under `-j4` on Windows and slower runners; COST ordering on runner CMake versions) are checked only by the PR's six-platform run.
- The `suite-flake` job has run nowhere yet; the PR's nightly run (triggered by the changed paths) is its first check.
- The 10 minute timeout on `suite-flake` is an initial guess, to be set to twice the first measured cold run.

## Known Stubs

None.

## Self-Check: PASSED

Files exist (05-CI-RECORD.md, workflows, presets); commits 615db60, 7d7bccc and 0815aca are ancestors of HEAD; `cmake --workflow --preset ci` (357 tests), asan and hygiene pass.
