---
phase: 02-the-cpu-matches-the-public-vectors
plan: 08
subsystem: ci
tags: [github, ci, nightly, timeouts, msvc, cmake-policy, pull-request]

requires:
  - phase: 02-the-cpu-matches-the-public-vectors
    provides: "02-01..02-06: the CPU and 256 cpu.vectors tests; 02-07: vectors-full preset and nightly.yml"
provides:
  - "Draft pull request #5 'feat: the CPU matches the public 65x02 vectors', mergeable, CI required green"
  - "Green CI on six platforms with the CPU (runs 37136095554 and 37136269250)"
  - "Green Nightly vectors-full on the pull request: 258 of 258 (runs 37136095512 and 37136269309)"
  - "Measured timeouts: nightly vectors-full 3 min (88 s cold), ci build 3 min (69 s windows-11-arm)"
affects: [02-verification, gsd-ship, release]

actuals:
  tokens: 850
  tasks: 2
  commits: 5
plan_head_before: 057c5884bc4ae672d7494e35837182634d588797
plan_head_after: baba2ab0e29df9a35c5bdbbbb6d669e7a065365e

tech-stack:
  added: []
  patterns:
    - "Every cmake -P script that runs on CI sets cmake_minimum_required, because script mode starts with no policies and the runners have CMake 3.31"
    - "A conflicting pull request runs no pull_request workflows; merge main into the phase branch (never rebase and force-push)"

key-files:
  created: []
  modified:
    - src/cpu.c
    - tests/cmake/fetch_vectors.cmake
    - tests/cmake/vectors_full_run.cmake
    - tests/cmake/vectors_sample_match.cmake
    - .github/workflows/ci.yml
    - .github/workflows/nightly.yml

key-decisions:
  - "main was merged into the phase branch (db56d93) instead of rebasing, because force-push is forbidden; the branch side won in ROADMAP.md and STATE.md"
  - "pull_p narrows only the non-constant result: (uint8_t)((v & ~FLAG_B) | FLAG_U), since MSVC /W4 /WX rejects a cast that truncates a constant (C4310)"
  - "The three nightly-only CMake scripts set cmake_minimum_required(VERSION 3.25), as phase 1's scripts do"
  - "Nightly vectors-full timeout is 3 minutes (twice 88 s cold, run 37136095512); ci build is 3 minutes (twice 69 s, run 37136095554); other ci.yml jobs stayed within half their limits and are unchanged"
  - "The 88 s cold nightly is far under the 10-minute D-21 trigger, so the N65V cache stays deferred"

patterns-established:
  - "CI fix loop as in phase 1: gh run view --log-failed, fix on the branch in a new commit, push, watch again"

requirements-completed: [CPU-01, CPU-02]

coverage:
  - id: D1
    description: "CI required green on the pull request: hygiene, title, six build legs each running 256 cpu.vectors tests, asan, both nofp legs and hash-equality"
    requirement: CPU-01
    verification:
      - kind: e2e
        ref: "gh pr checks --required (run 37136269250, CI required pass); gh run view 37136095554 --log | grep -c 'cpu.vectors\\.ff' = 14"
        status: pass
    human_judgment: false
  - id: D2
    description: "Nightly vectors-full green on the pull request, 258 of 258 tests, failed keys none"
    requirement: CPU-02
    verification:
      - kind: e2e
        ref: "gh run list --workflow nightly.yml --limit 1 -> success (run 37136269309 on baba2ab; 37136095512 on 1d9c997: '100% tests passed, 0 tests failed out of 258')"
        status: pass
    human_judgment: false
  - id: D3
    description: "Timeouts set from measured runs, with the run IDs in the comments; no initial or provisional timeout comment remains"
    verification:
      - kind: other
        ref: "actionlint clean; grep 'initial; set to twice' nightly.yml = 0; grep 'provisional after timeout' = 0 in both files; nightly.yml matches 'timeout-minutes: 3 # twice the cold run 37136095512'"
        status: pass
      - kind: e2e
        ref: "run 37136269250 (CI) and 37136269309 (Nightly) green with the new timeouts"
        status: pass
    human_judgment: false
  - id: D4
    description: "hash-equality still passes: the CPU leaves the six platforms' frame hashes identical"
    verification:
      - kind: e2e
        ref: "hash-equality success in runs 37136095554 and 37136269250"
        status: pass
    human_judgment: false
  - id: D5
    description: "After the owner merges, release.yml publishes 0.1.1 carrying the CPU; the first scheduled nightly on main passes and its report job runs with issues: write"
    verification: []
    human_judgment: true
    rationale: "Backstop: both need the owner's merge, which this plan must not perform; they run on main afterwards"

duration: 15min
completed: 2026-10-03
status: complete
---

# Phase 2 Plan 08: Draft pull request, green CI and nightly, measured timeouts Summary

**Draft PR #5 "feat: the CPU matches the public 65x02 vectors" is open and mergeable. `CI required` is green on all six platforms with 256 `cpu.vectors` tests per build leg, and the Nightly `vectors-full` job passes 258 of 258 on the pull request. Getting there took three fixes: a merge of `main`, an MSVC C4310 cast fix in `pull_p`, and CMake policy pins in the nightly-only scripts. The timeouts now come from measured runs: nightly 3 min (88 s cold) and ci `build` 3 min (69 s).**

## Performance

- **Duration:** about 15 min
- **Started:** 2026-10-03T16:04:10Z
- **Completed:** 2026-10-03T16:18:49Z
- **Tasks:** 2 of 2
- **Files modified:** 6

## Accomplishments

- Locally, before pushing, every lane passed: `ci` (317 tests), `asan` (316), `nofp` (5) and `hygiene` (6), plus `scripts/hygiene.sh --history origin/main..HEAD`.
- Pushed `phase/02-cpu-vectors` and opened draft PR https://github.com/szTheory/nesturbator/pull/5. The body covers CPU-01, CPU-02, the sample, the nightly and the 0.1.1 release a merge will publish, and ends with the attribution line.
- Got CI and the nightly to green through the fix loop (runs below).
- Set the timeouts from measured runs and confirmed both workflows green again on the final commit.

## CI runs

| Run | Workflow | Head | Result |
|---|---|---|---|
| (none) | both | 057c588 | not started: PR conflicting with `main` |
| 37135715969 | CI | db56d93 | failed: both Windows build legs, C4310 at `src/cpu.c(426)` |
| 37135715964 | Nightly | db56d93 | failed: `cpu.vectors-full.fetch`, `IN_LIST` unknown in script mode on CMake 3.31 |
| 37135959474 | CI | 5b7cd32 | **green** |
| 37135959473 | Nightly | 5b7cd32 | failed (same fetch cause, fix not yet pushed) |
| 37136095554 | CI | 1d9c997 | **green**; ci.yml timeouts measured here |
| 37136095512 | Nightly | 1d9c997 | **green**, 258 of 258, cold; nightly timeout measured here |
| 37136269250 | CI | baba2ab | **green** with the new timeouts |
| 37136269309 | Nightly | baba2ab | **green** with the new timeout (81 s) |

## Timeouts (Task 2)

Durations come from `gh run view <id> --json jobs`. A timeout changes only when a job's duration exceeded half its limit.

| Job | Measured | Old limit | New limit |
|---|---|---|---|
| nightly vectors-full | 88 s (37136095512) | 30 (initial) | **3** |
| build (max: windows-11-arm) | 69 s (37136095554) | 2 | **3** |
| build, other legs | 16 to 53 s | | |
| hygiene | 20 s | 1 | 1 (unchanged) |
| title | 5 s | 1 | 1 |
| asan | 32 s | 2 | 2 |
| nofp (max) | 12 s | 1 | 1 |
| hash-equality | 5 s | 1 | 1 |
| CI required | 3 s | 1 | 1 |

The provisional-timeout route (Task 1 step 6) was not needed: no job was cancelled. The nightly cold run took 88 s, far below the 10-minute D-21 trigger, so the N65V cache stays deferred.

## Task Commits

1. **Task 1 (tracer): push, draft PR, CI and nightly green**
   - `db56d93` chore(02-08): merge main into the phase branch
   - `5b7cd32` fix(02-08): drop the constant narrowing cast in pull_p for MSVC
   - `1d9c997` fix(02-08): set policies in the vectors-full scripts for CMake 3.31
2. **Task 2: measured timeouts**: `baba2ab` ci: set timeouts from measured phase 2 runs

The measured `commits: 5` counts `git rev-list 057c588..HEAD`. That range includes `main`'s `f3be8fe` (PR #4, planning files only), which the merge brought in. Four commits were made on the branch.

## Files Created/Modified

- `src/cpu.c`: `pull_p` no longer casts the constant `~FLAG_B` to `uint8_t`
- `tests/cmake/fetch_vectors.cmake`, `vectors_full_run.cmake`, `vectors_sample_match.cmake`: `cmake_minimum_required(VERSION 3.25)`
- `.github/workflows/nightly.yml`: `vectors-full` timeout 3, with the run ID
- `.github/workflows/ci.yml`: `build` timeout 3, with the run ID

## Decisions Made

See key-decisions in the frontmatter.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] The pull request conflicted with `main`, so no workflow ran**
- **Found during:** Task 1, step 4. `gh run list` was empty and `mergeable` was `CONFLICTING`.
- **Issue:** `main` gained `f3be8fe` (phase 1 verification) after the branch was cut. ROADMAP.md and STATE.md conflicted, and GitHub runs no `pull_request` workflows on a conflicting PR.
- **Fix:** Merged `origin/main` into the branch. The branch side won both conflicts: phase 2 is executing, and WR-08 was resolved in 02-01. The merged tree equals the branch tree, so main's other edits were already on the branch. No rebase and no force-push.
- **Commit:** db56d93

**2. [Rule 1 - Bug] MSVC C4310 in `pull_p` failed both Windows legs**
- **Found during:** Task 1, run 37135715969.
- **Issue:** `(uint8_t)~FLAG_B` truncates a constant. That is an error under `/W4 /WX`, while clang and GCC don't warn.
- **Fix:** `(uint8_t)((v & ~FLAG_B) | FLAG_U)`. P gets the same value. `cpu.vectors.28` (PLP) and `.40` (RTI) pass, as do all of `ci`, `asan` and `nofp`. Lines 37 and 173 complement variables, so C4310 does not apply to them.
- **Commit:** 5b7cd32. src/cpu.c is not in the plan's files_modified. Behaviour is unchanged, so the README and header need no edit.

**3. [Rule 1 - Bug] The nightly's fetch failed on CMake 3.31 (same class as phase 1 deviation 1)**
- **Found during:** Task 1, run 37135715964.
- **Issue:** `fetch_vectors.cmake` uses `IN_LIST`. Script mode has no policies, so CMake 3.31 rejects it. The local CMake 4.4 cannot reproduce this.
- **Fix:** `cmake_minimum_required(VERSION 3.25)` in the three scripts that run only in the nightly. The two run-only scripts got it too, so the next run would not stop one script further on.
- **Verification:** `cmake --workflow --preset vectors-full` passes 258 of 258 locally. Nightly run 37136095512 passes 258 of 258.
- **Commit:** 1d9c997. These files are not in the plan's files_modified.

---

**Total deviations:** 3 auto-fixed (1 rule 3, 2 rule 1). **Impact:** no test was weakened and no timeout was raised to get to green. Every fix is in build or test plumbing, or is a cast with identical results.

## Issues Encountered

- Phase 2 shipped a Windows-only compile error and a CMake-3.31-only script error. Local development uses clang and CMake 4.4, so neither showed until CI ran. Both are now covered on every pull request (MSVC) and every night (CMake 3.31 on the nightly runner).
- Two things only run on the hosted runners: the MSVC build and script mode under CMake 3.31. Nothing local stands in for them yet.
- Nightly `vectors-full` now has a 3-minute limit, and its cold time is dominated by the git fetch (62 s in the failed run). A slow GitHub fetch could cancel a nightly. If one does, measure again under the same rule rather than adding margin.

## User Setup Required

None.

## Next Phase Readiness

- The pull request is ready for phase verification and `/gsd-ship`, which reuses draft PR #5. The owner merges it.
- Backstops after the merge: release.yml publishes 0.1.1 with the CPU, and the first scheduled nightly on `main` passes and runs `report` with `issues: write`.

---
*Phase: 02-the-cpu-matches-the-public-vectors*
*Completed: 2026-10-03*

## Self-Check: PASSED

- FOUND commits: db56d93, 5b7cd32, 1d9c997, baba2ab (git log 057c588..HEAD)
- FOUND: PR #5 draft, title as planned, mergeable; `gh pr checks --required` pass (run 37136269250)
- FOUND: latest nightly on the branch `success` (run 37136269309, head baba2ab)
- Acceptance greps: 'initial; set to twice' 0; 'provisional after timeout' 0 and 0; nightly timeout line matches; actionlint clean
