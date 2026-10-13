---
phase: 13-rewind-and-run-ahead-in-retroarch
plan: 08
subsystem: libretro
tags: [libretro, savestate, run-ahead, ci]
requires: [13-07]
provides:
  - "savestate_features = deterministic claim, last code commit of the phase"
  - "re-measured build and asan job timeouts"
affects: [libretro/nesturbator_libretro.info, libretro/libretro.c, README.md, .github/workflows/ci.yml]
key-files:
  modified:
    - libretro/nesturbator_libretro.info
    - libretro/libretro.c
    - README.md
    - .github/workflows/ci.yml
decisions:
  - "Build job timeout 22 min and asan 14 min, each twice the slowest measured hosted run on the phase workload (ENGINEERING section 5)"
status: complete
commits: 4
plan_head_before: 22abb81fce1032a24034a780daa30e829f763621
plan_head_after: 137d855cfbf9a8bf9875aaf911ccb38a4afe4d4e
actuals:
  tokens: 4000
  tasks: 2
  commits: 4
---

# Phase 13 Plan 08: Deterministic claim Summary

The `.info` now claims `savestate_features = "deterministic"` in the last code commit of the phase, with the README and the serialize comment in `libretro/libretro.c`; the final pushed head is green on every check, including `CI required`.

## Blocking issue first: CI timeouts

PR #44 was red only because `build (macos-15-intel)` hit `timeout-minutes: 7`. The tests were kept and the timeout re-measured.

| Run | Head | Slowest build job | asan |
|---|---|---|---|
| 38293977124 (timeout raised to 25) | 45cbb80 | macOS Intel 9m18s (558 s); Windows x64 6m06s | cancelled by a push |
| 38295163207 | 958a03c | macOS Intel 10m40s (640 s) | 6m45s (405 s) |
| 38297707729 (final) | 137d855 | macOS Intel 9m20s | 7m11s |

Timeouts set from the slowest observed (640 s, 405 s): build 22 min, asan 14 min (asan's old 10 min was under twice its measured time, so it was updated on the same basis). Comments cite the run and ENGINEERING section 5. Commits: 45cbb80 (temporary 25), 958a03c (19), then the final value in commit "ci(13-08): measure build and asan timeouts on the phase workload".

## Task 1: claim commit 137d855

- Only README.md, libretro/libretro.c and libretro/nesturbator_libretro.info are touched. The libretro.c diff is comment lines only (the serialize comment extended: rewind, single-instance run-ahead, ignored savestate context, little-endian format vs libretro.h L5695-5707, no netplay end-to-end test).
- Local: `cmake --workflow --preset ci`, `asan` and `hygiene` pass; ctest for libretro.serialize, libretro.runahead, libretro.info_claims(.self_test), libretro.no_quirks, core.state_runahead passes (runahead 30.6 s, state_runahead 29.5 s). gcc:14 -O3 -Werror compile and clang-format-18 pass on libretro.c.
- Local RetroArch 1.22.2 STATE_ROM run with the committed `.info` (level deterministic):
  - state-ahead: RetroArch's frame 121 equals the runner's frame 121
  - state-load-ahead: RetroArch's frame 180 equals the runner's frame 180
  - state-net: `pause-a={487} ... rew-3={815;816} ahead-1={815;816}`; "pause, advance, load, resume, rewind and run-ahead (lead 1) hold"; log check passed at level deterministic for every lane.

## Task 2: push and CI

Pushed without force; main had not moved. Final head 137d855, run https://github.com/szTheory/nesturbator/actions/runs/38297707729 succeeded: build x6 (macos-15-intel 9m20s, windows-2025 6m22s, ubuntu-24.04 3m57s, windows-11-arm 3m37s, macos-15 3m29s, ubuntu-24.04-arm 2m23s), asan 7m11s, retroarch-e2e 4m45s (state/rewind/run-ahead step 54 s), nofp x2, hygiene, title, hash-equality, CI required; the companion run 38297707605 (rom-loader-fuzz, state-fuzz, suite-flake, mmc3-oracle, vectors-full) also passed. PR body updated with the timings. PR: https://github.com/szTheory/nesturbator/pull/44, left as draft per the orchestrator (ready/merge happens after verification).

## Deviations

- [Rule 3 - Blocking] CI timeout re-measurement (above); not in the plan.
- The plan asks for `gh pr ready`; the orchestrator instructed to leave the PR draft, so it was not run.
- This SUMMARY and the STATE/ROADMAP docs commits follow the claim commit, so the claim is the last code commit, not the last commit on the branch; they are not pushed.

## Known Stubs

None.

## Self-Check: PASSED
