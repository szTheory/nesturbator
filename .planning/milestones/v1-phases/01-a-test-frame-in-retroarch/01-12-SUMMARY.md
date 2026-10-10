---
phase: 01-a-test-frame-in-retroarch
plan: 12
subsystem: ci
tags: [github, ci, ruleset, cross-platform, hashes, artifacts]
status: complete

requires:
  - phase: 01-11
    provides: Public repository szTheory/nesturbator, release App credential, install-line check
provides:
  - Repository settings (auto-merge, squash only with PR_TITLE/PR_BODY, delete branch on merge, wiki off), private vulnerability reporting and immutable releases, all read back
  - Ruleset `main` (id 24400039) active, matching .github/rulesets/main.json
  - Draft pull request #1 "feat: a test frame in RetroArch"
  - Green `CI required` on all six platforms (run 37084642541, confirmed again by 37084790893 with the new timeouts)
  - Cross-platform frame-hash identity and 18 per-platform archives as pull-request artifacts
affects: [01-verification, gsd-ship]

actuals:
  tokens: 1500
  tasks: 2
  commits: 3
plan_head_before: 6b230a5ef22870727b3e0bef85be3c353e865030
plan_head_after: 4070254d6ebb3b930359db6fcb347666c5403ddc

tech-stack:
  added: []
  patterns:
    - "CMake -P scripts that use post-3.0 syntax set cmake_minimum_required, because script mode starts with no policies"
    - "Workflow steps that run on Windows quote -D arguments containing a dot (PowerShell splits them)"

key-files:
  created: []
  modified:
    - tests/cmake/undefined_symbols.cmake
    - tests/cmake/write_hashes.cmake
    - .github/workflows/ci.yml

key-decisions:
  - "hashes.txt is written with file(CONFIGURE ... NEWLINE_STYLE LF) so every platform writes the same bytes"
  - "_GLOBAL_OFFSET_TABLE_ joins the undefined-symbol allowlist: a linker symbol referenced by x86-64 ELF PIC code, not a library function"
  - "Job timeouts are twice the durations in run 37084642541 (first run where every job completed), rounded up to whole minutes: build 2, asan 2, every other job 1"

patterns-established:
  - "CI fix loop: gh run view --log-failed, fix on the branch, push a new commit, watch again"

requirements-completed: [FRAME-01, FRAME-07]

coverage:
  - id: D1
    description: "Repository settings and ruleset applied and read back"
    requirement: FRAME-07
    verification:
      - kind: integration
        ref: "gh api repos/szTheory/nesturbator (true,false,false,\"PR_TITLE\"); gh api .../rulesets (main: active)"
        status: pass
    human_judgment: false
  - id: D2
    description: "CI required green on Linux, macOS and Windows on x64 and arm64, with asan, both nofp legs and hash-equality"
    requirement: FRAME-01
    verification:
      - kind: e2e
        ref: "gh pr checks phase/01-test-frame --required (run 37084790893)"
        status: pass
    human_judgment: false
  - id: D3
    description: "6 archive artifacts holding 18 zips that pass check_archives.cmake, and 6 byte-identical hashes.txt files"
    requirement: FRAME-07
    verification:
      - kind: e2e
        ref: "gh run download 37084642541; check_archives.cmake per archives-* directory; sort -u of hashes gives 2 lines"
        status: pass
    human_judgment: false
  - id: D4
    description: "After merge, release-please publishes 18 archives plus SHA256SUMS with an attestation"
    requirement: FRAME-07
    verification:
      - kind: backstop
        ref: "end-of-phase human-check in 01-12-PLAN.md; release.yml count step"
        status: pending
    human_judgment: true

duration: 10min
completed: 2026-10-03
---

# Phase 1 Plan 12: GitHub settings, draft pull request and green CI on six platforms Summary

**szTheory/nesturbator now has its settings and the `main` ruleset applied, plus draft PR #1. `CI required` is green on Linux, macOS and Windows on x64 and arm64, and all six platforms produce the same frame hashes. Three CI-only test-script bugs were fixed along the way.**

## Performance

- **Duration:** about 10 minutes (resumed after the owner-approved push)
- **Completed:** 2026-10-03
- **Tasks:** 2 of 2
- **Files modified:** 3

## Task 1: settings, ruleset, draft pull request

The orchestrator had already run the remote setup and the first pushes (see the continuation notes). The pre-push hygiene check passed.

Settings read back from `gh api repos/szTheory/nesturbator`:

| Setting | Value |
|---|---|
| visibility | public |
| has_wiki | false (the repository was created with the wiki on) |
| allow_auto_merge | true |
| allow_squash_merge / allow_merge_commit / allow_rebase_merge | true / false / false |
| squash_merge_commit_title / message | PR_TITLE / PR_BODY |
| delete_branch_on_merge | true |
| private-vulnerability-reporting | `{"enabled":true}` (PUT returned 204) |
| immutable-releases | `{"enabled":true,"enforced_by_owner":false}` (PUT returned 204) |
| ruleset `main` | id 24400039, enforcement active |

Ruleset read-back compared with `.github/rulesets/main.json`: every field in the file matches. GitHub added three fields with server defaults: `required_reviewers: []`, `do_not_enforce_on_create: false`, and `require_extra_approval_for_unattributed_changes: true` in the pull_request rule. The last one is recorded as an open item below.

Draft pull request: https://github.com/szTheory/nesturbator/pull/1, `true,"feat: a test frame in RetroArch"`. `git remote get-url origin` gives `https://github.com/szTheory/nesturbator.git`. `sh scripts/hygiene.sh --tree` exits 0.

## Task 2: CI to green

| Run | Head | Result |
|---|---|---|
| 37084293594 | 6b230a5 | failed: nofp x2 (CMake IN_LIST policy), Windows build x2 (runner.write_hashes.content) |
| 37084485603 | 53f8353 | failed: nofp ubuntu-24.04 (`_GLOBAL_OFFSET_TABLE_`), Windows build x2 (hashes.txt upload found no file) |
| 37084642541 | 4ebe7ab | **green**, every job success; timeouts measured here |
| 37084790893 | 4070254 | **green** with the new timeouts; `gh pr checks --required` passes |

Durations of the jobs in run 37084642541 and the timeouts set from them (twice the duration, rounded up to whole minutes):

| Job | Duration | timeout-minutes |
|---|---|---|
| hygiene | 21 s | 1 |
| title | 2 s | 1 |
| build linux x64 / linux arm64 | 17 s / 16 s | 2 (job-wide) |
| build macos arm64 / macos x64 | 14 s / 40 s | 2 |
| build windows x64 / windows arm64 | 38 s / 53 s | 2 |
| asan | 31 s | 2 |
| nofp ubuntu-24.04 / ubuntu-24.04-arm | 14 s / 15 s | 1 |
| hash-equality | 5 s | 1 |
| CI required | 4 s | 1 |

Artifacts of run 37084642541 (`gh run download --dir build/ci-artifacts`):
- 12 directories: `archives-{linux,macos,windows}-{x64,arm64}` and `hashes-{...}`.
- 18 zips. `check_archives.cmake -DPACKAGES=...` passes for all six directories, three zips each.
- `nesturbator-0.0.0-libretro-macos-arm64.zip` lists `cores/nesturbator_libretro.dylib` and `info/nesturbator_libretro.info`.
- All six `hashes.txt` files have SHA-256 `f3eb7f18…9e34`. `sort -u` over them gives 2 lines (frames 1 and 3).
- `hash-equality` and `CI required` both succeeded in the same run.

## Task Commits

1. **Task 1** made no change to the tree. Its work is the remote settings, the ruleset and PR #1.
2. **Task 2:**
   - `53f8353` fix(01-12): make the CMake test scripts pass on the CI runners
   - `4ebe7ab` fix(01-12): allow the GOT symbol and quote the hashes step for PowerShell
   - `4070254` ci(01-12): set job timeouts to twice the measured cold run

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] undefined_symbols.cmake failed under CMake 3.31**
- **Found during:** Task 2, run 37084293594, both nofp legs.
- **Issue:** Script mode starts with no policies set. CMake 3.x then treats `IN_LIST` as unknown arguments. The local CMake 4.4 has no such old behaviour, so the local build never failed.
- **Fix:** `cmake_minimum_required(VERSION 3.25)` at the top of the script, and in write_hashes.cmake as well.
- **Commit:** 53f8353

**2. [Rule 1 - Bug] hashes.txt differed from the LF reference on Windows**
- **Found during:** Task 2, run 37084293594, both Windows legs (runner.write_hashes.content).
- **Fix:** write_hashes.cmake writes with `file(CONFIGURE ... NEWLINE_STYLE LF)`. It removes the runner's last newline first, because file(CONFIGURE) adds one (seen locally as a doubled `\n`, then fixed). runner.write_hashes.content passed on both Windows legs in the next run. tests/cmake/write_hashes.cmake was not in the plan's files_modified.
- **Commit:** 53f8353

**3. [Rule 1 - Bug] `_GLOBAL_OFFSET_TABLE_` missing from the allowlist (expected first-run cause, ELF helper)**
- **Found during:** Task 2, run 37084485603, nofp ubuntu-24.04 (x86-64, GCC 14).
- **Fix:** Added to the allowlist with a comment citing the x86-64 psABI and the run.
- **Commit:** 4ebe7ab

**4. [Rule 1 - Bug] PowerShell split `-DOUT=hashes.txt` at the dot**
- **Found during:** Task 2, run 37084485603. The script logged "wrote hashes", and the upload found no `hashes.txt`.
- **Fix:** The step's `-D` arguments are quoted in ci.yml, with a comment. This was also the reason the first run's Windows legs had no hashes artifact.
- **Commit:** 4ebe7ab

**Total deviations:** 4 auto-fixed, all Rule 1 and all in test or CI scripts. The core and the frame hash are unchanged. **Impact:** none on behaviour. The README and the public header needed no change.

## Open Items

- **`require_extra_approval_for_unattributed_changes: true`**: GitHub added this server default to the ruleset's pull_request rule. It is not in `.github/rulesets/main.json`. If it stops the release-please pull request from auto-merging after the phase merge, set it to false in the file and in the ruleset.
- **Tight timeouts:** in the confirming run, build windows x64 took 65 s against a 2-minute limit. If timeouts start cancelling jobs, measure again and raise them under the same rule.
- **release.yml timeouts** stay at the initial 30 minutes. The release jobs only run after a merge, so there is no measured run for them yet.

## Issues Encountered

None beyond the deviations above. No authentication gates.

## Next Phase Readiness

The phase is ready for verification and `/gsd-ship`, which reuses draft PR #1. The owner merges it. Plan 12's human-check then covers the post-merge half: the release-please PR, v0.1.0 with 19 assets, the attestation, and the README install line in RetroArch.

## Self-Check: PASSED

- FOUND: .planning/phases/01-a-test-frame-in-retroarch/01-12-SUMMARY.md
- FOUND commits: 53f8353, 4ebe7ab, 4070254
- FOUND: PR #1 draft, `CI required` pass (run 37084790893)
