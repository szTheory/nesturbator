---
phase: 10-close-out-and-boot-to-play
plan: 03
subsystem: tests
tags: [nes-runner, boot-to-play, movie, judge, ram-observation]
requires: ["10-01", "10-02"]
provides:
  - "four 600-frame nes-runner NMOVIE1 scenarios from runner.game_movie"
  - "runner.boot_to_play: ROM pin, layout canary, judge() with P1/P2/P3, four controls, save checks"
  - "runner.nesrunner_movie.* fixtures and the runner.boot_to_play CTest (TIMEOUT 300, COST 45)"
affects: [10-04, 10-05, 10-06, 10-07]
tech-stack:
  added: []
  patterns: ["one pure judge over two traces with exact expected masks", "closed RAM-read table enforced at run time"]
key-files:
  created: [tests/runner/test_boot_to_play.c]
  modified: [tests/runner/game_movie.c, tests/CMakeLists.txt]
key-decisions:
  - "Pinned the mid-window jump list {251,21} {303,21} {351,21} {408,23} {463,21} {558,25}; the latest-feasible list stays in the header as reference only"
  - "LATCH_SLACK = 1: measured first input frame 61, first RAM divergence frame 61, offset 0, plus 1"
  - "Window PCM hashes reuse nesturbator_run_audio_hash_pcm rather than a second serialiser"
  - "Control failure lines are suppressed; only the positive run prints judge failures, the controls print their masks"
requirements-completed: [PLAY-01]
status: complete
commits: 3
plan_head_before: b3760de2b33a381c809f9da974bbae3bec346d17
plan_head_after: dd0eada81e2fe6dd4935c0e27535ef49884ded85
actuals:
  tokens: 21000
  tasks: 2
  commits: 3
duration: 25 min
completed: 2026-10-11
---

# Phase 10 Plan 03: nes-runner boot-to-play proof Summary

A generated movie takes nes-runner from boot into scoring play through the library, and one pure `judge` separates that run from four controls by exact failure masks.

REQUIREMENTS.md still shows PLAY-01 open on purpose: Plan 04 (mutants, boundary traces, README text) and later plans still declare it.

## Tasks

| Task | Commits | Result |
|------|---------|--------|
| 1 Tracer: movie writer, fixtures, ROM pin, canary, positive run | 46922e4 | `runner.nesrunner_movie.*` and `runner.boot_to_play` pass; the DABG rows in `runner.write_hashes.content` did not move |
| 2 TDD: `judge` and the four controls | c13d9f3 (RED), dd0eada (GREEN) | Positive mask 0; none 0x7, attract 0x3, failed load 0x7, start-only 0x3 |

## Reproduced measurements (fixed core, `ci` Release)

| Measure | Value | Research value |
|---------|-------|----------------|
| Positive final score at frame 600 | 52 | 52 |
| Start-only final score | 16 | 16 |
| Positive save generation at frame 600 | 2 | 2 |
| Start-only save generation at frame 600 | 120 | 120 |
| SRAM bytes 0-1 | 5A 6F | 5A 6F |
| First input frame, first RAM divergence frame | 61, 61 | 61 |
| `LATCH_SLACK` (offset 0, plus 1) | 1 | not pinned in research |
| Distinct frame hashes (log only) | pos 551, none 15, attract 275, start-only 228, failed load 1 | n/a |
| Layout canary (frame 4), level canary (frame 135) | held on every run that loaded the ROM | as stated |

No pin needed re-measuring: the research values reproduced on the fixed core, so the six-jump list, K = 36, F = 4096, the 2048 audio floor, 50% and 75% are used as planned. Start-only dies near frame 265 (`$3E` = 3 at the frame 270 checkpoint, score frozen at 16).

## TDD record (Task 2)

- RED: with `judge` a stub returning 0, `runner.boot_to_play` failed on the four control-mask assertions (`judge(none) == 7 failed: 0 != 7`, and likewise attract, failed load, start-only). The positive `judge == 0` assertion and every canary passed, so the failure was the planned assertion. Semantic assessment: the target ran and failed for the intended reason, not a load or setup fault.
- GREEN: `judge` implemented; all masks exact.
- REFACTOR: none.

## Guard check (PROH-10-03-01), one-off local mutations, not committed

- Adding `0x4E` to `ram_reads`: `runner.boot_to_play` fails at `check_read_table` (`CHECK(!in_table(never_read, ...))`).
- Calling `ram_read(inst, 78)` with the table unchanged: fails at `ram_read` (`CHECK(allowed)`).
- Both were reverted before commit.

## Deviations from Plan

None - plan executed as written. Two small choices inside the plan's discretion: window hashes go through the existing `nesturbator_run_audio_hash_pcm` (same bytes as the pinned audio rows) instead of a copy of its loop, and control failure messages are suppressed so the CTest log shows one mask line per control.

## Acceptance results

All greps from both tasks pass (`nesturbator_peek_cpu_ram(` appears twice, in `ram_read` and `ram_digest`; the two tables and `check_read_table()` are present; `TIMEOUT 300`; five `CHECK_EQ_U64(judge(` calls; `16u + 16384u`; `11209`/`735`; `LATCH_SLACK`). `rng_seed`'s address was not re-derived; the closed `ram_reads` table keeps it out.

## Verification

`cmake --workflow --preset ci` passed (452 of 452) and `cmake --workflow --preset hygiene` passed (8 of 8). The ASan run is Plan 04.

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED

tests/runner/test_boot_to_play.c, game_movie.c and CMakeLists.txt changes present; commits 46922e4, c13d9f3 and dd0eada are ancestors of HEAD.
