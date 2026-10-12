---
phase: 13-rewind-and-run-ahead-in-retroarch
plan: 05
subsystem: ci-fuzz
tags: [nightly, libfuzzer, state-loader, ci]
requires:
  - phase: 13-04
    provides: fuzz.state.libfuzzer target and tests/fuzz/corpus_state
provides:
  - nightly job state-fuzz (60 s libFuzzer over the state corpus, crash hex dump and artifact upload)
  - report aggregation of the state-fuzz outcome in the rolling issue
affects: []
key-files:
  modified:
    - .github/workflows/nightly.yml
    - README.md
    - tests/fuzz/state_loader.c
key-decisions:
  - "state-fuzz is a copy of rom-loader-fuzz (duplication over abstraction) with -timeout=5 and -artifact_prefix; timeout-minutes stays 3, not yet measured (same basis as the sibling)."
status: complete
requirements-completed: [LR-04]
commits: 2
plan_head_before: d52d9faf3faa2e5c937604b52f2b88a4818845b4
plan_head_after: 792657a
actuals:
  tokens: 6000
  tasks: 2
  commits: 2
completed: 2026-10-12
---

# Phase 13 Plan 05: Nightly state-loader fuzz job Summary

A nightly `state-fuzz` job runs `fuzz.state.libfuzzer` for 60 s over `tests/fuzz/corpus_state`, dumps and uploads any crash input, and feeds the rolling nightly issue; the libFuzzer build was first proven in a Linux container.

## Commits

- e41f266: `state-fuzz` job, report `needs`/RESULT/`STATE_FUZZ`/issue-body line, README sentence.
- 792657a: fix of a Plan 04 harness warning (see Deviations).

## Task 1: Docker libFuzzer smoke run (assumption A6)

Container: ubuntu:24.04, clang, ASan+UBSan, `-max_total_time=30 -timeout=5 -max_len=65536` over a copy of the corpus.

`Done 361 runs in 31 second(s)`

No sanitizer or libFuzzer error; the repository corpus was untouched (`git status` clean). The largest state (45,869 bytes, 13-04) is below 65536. Throughput is about 12 exec/s because each input builds a cart and runs 2 frames.

## Task 2: nightly job

- `state-fuzz`: same runner, `contents: read`, pinned checkout, `persist-credentials: false`, libFuzzer configure and build, run step `fuzz` with `-max_total_time=60 -timeout=5 -max_len=65536 -artifact_prefix=build/fuzz/state-artifacts/`.
- `if: failure()` steps: hex dump (first 512 bytes of each `crash-*`, `[ -e "$f" ] || continue` guard) into the step summary; upload with the ci.yml-pinned `actions/upload-artifact` SHA, `if-no-files-found: ignore`.
- `timeout-minutes: 3`, not yet measured (the sibling's rule: state 3 with a comment until a cold run is measured).
- report: `state-fuzz` in `needs`, `needs.state-fuzz.result == 'success'` in RESULT, `STATE_FUZZ` env, "State loader fuzz outcome" line.
- `cmake --workflow --preset hygiene` passes (8 tests incl. `hygiene.nightly_workflow_policy` and self-test); `grep -c "issues: write"` is 1; `cmake --workflow --preset ci` passes (534 tests).

## Deviations from Plan

**1. [Rule 1 - Bug] gcc 14 -Werror format-truncation in tests/fuzz/state_loader.c**
- **Found during:** PR #44 check review. The Linux CI, nofp, mmc3-oracle, suite-flake and vectors-full jobs failed to build `fuzz.state_loader` at -O3 (`path[512]` with a seed name field up to 815 bytes). It was missed in Plan 04 because local builds were clang/macOS.
- **Fix:** `path` buffers widened to 1024 in `nt_write_seeds` and `nt_check_seeds`; verified clean with gcc:14 in Docker using the project warning set at -O3 (full compile, not syntax-only).
- **Files modified:** tests/fuzz/state_loader.c
- **Commit:** 792657a

Total deviations: 1 auto-fixed (Rule 1). Impact: restores green Linux CI on PR #44; no behaviour change.

## PR #44 checks seen at the start

Red: Linux build/nofp (x64, arm64), mmc3-oracle, suite-flake, vectors-full, all from the gcc 14 error above. Passing: macOS arm64, Windows x64/arm64, hygiene, title, rom-loader-fuzz. Pending: macOS x64, asan, retroarch-e2e. The new head should be re-checked.

## Known Stubs

None.

## Self-Check: PASSED

nightly.yml contains `state-fuzz:`, `-timeout=5 -max_len=65536`, `-artifact_prefix=`, `State loader fuzz outcome`; README contains `state-fuzz`; commits e41f266 and 792657a are ancestors of HEAD.
