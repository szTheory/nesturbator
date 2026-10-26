---
phase: 19-jam-reporting-and-sprite-0-hit
plan: "03"
subsystem: libretro
tags: [libretro, jam, run-ahead, presentation-cache, timeline]
requires: ["19-02"]
provides:
  - "adapter presentation cache tagged with frame_number and timeline identity (state CRC)"
  - "libretro.host diverged-timeline JAM case"
  - "libretro.runahead JAM image across depths 1-3 with window-class counters and a black-oracle control"
  - "JAM-aware oracle frame record in tests/runahead_model.h"
affects: [19-06]
key-files:
  modified:
    - libretro/libretro.c
    - tests/libretro/libretro_host.c
    - tests/runahead_model.h
    - README.md
    - include/nesturbator.h
    - .planning/preparation/LIBRETRO-AND-RUNNER.md
key-decisions:
  - "D-16 as amended 2026-10-25: cache tag = frame_number + timeline generation; a load keeps the generation only for the state the cached frame produced or the jammed state reached from it"
  - "Black is a separate never-written buffer, so showing black does not destroy the cached picture a later load may return to"
  - "Scratch buffer for the CRC records is malloc'd at content load (adapter only); an allocation failure fails retro_load_game"
  - "Synthetic JAM image counts a counter + 0x10 backdrop and JAMs on the 13th NMI (sequence at executor discretion)"
requirements-completed: [JAM-02]
completed: 2026-10-25
status: complete
commits: 1
plan_head_before: 18df0486022af5549db04b4384dc6fb3a2330b01
plan_head_after: 13932e8
actuals:
  tokens: 60000
  tasks: 3
  commits: 1
---

# Phase 19 Plan 03: Run-ahead across a JAM Summary

Run-ahead across a guest JAM now shows the frozen last converted picture and silence, never a picture from another timeline: the adapter tags its cached frame with the frame number and a state-CRC timeline identity, and a successful load no longer drops it.

## What changed

- `libretro/libretro.c`: `cached_frame_number`, `current_frame_number` (from `io.frame_number` after each run and from state bytes 32-39 after each load), `timeline` / `cached_timeline`, `cached_state_crc`, `jam_state_crc` (+ valid flag) and a scratch buffer of `nesturbator_state_size` bytes. One `nesturbator_save_state` per converted frame records the CRC (header bytes 28-31); after a STOP_JAM run on a current cache the jammed state's CRC is recorded. A load keeps the generation when its CRC equals one of the two records, otherwise moves to `cached_timeline + 1`. STOP_JAM shows the cache only when the tags and generation match, else a never-written all-zero buffer. reset, load_game, unload, init and deinit invalidate everything; a refused load preserves it; nothing enters the native state; no public API or ABI change.
- `tests/runahead_model.h`: `struct ra_frame {hash, frame_number, valid}`, `ra_lib_frame_status`, `ra_run_uninterrupted_ex` / `ra_run_two_instance_ex` with a `jam_aware` flag; the existing entry points keep the strict default (core.state_runahead untouched).
- `tests/libretro/libretro_host.c`: `check_jam_timeline` (diverged timeline), `rah_jam_image` (in-memory NROM), JAM matrix (mask 0xf, script 1, two-instance repeat, depths 1-3), window-class counters, `jam_images`, black-oracle control, one `check_jam_presentation` assertion changed (load of the cached frame's state now shows the cached frame).
- `README.md`, `include/nesturbator.h` (comment only), `.planning/preparation/LIBRETRO-AND-RUNNER.md`: new rule and residual stated.

## Results

- J read from the oracle: **12** at T=180 (full run). Under `--short` (T=30, used by asan) the same synthetic image gives the same J (it does not depend on T); bound `RA_MAX_DEPTH+2 <= J <= T-RA_MAX_DEPTH-2` asserted for the active T (full run exercised T=180; asan run exercised T=30 and passed).
- Window classes (ticks, T=180 full run, real-input plus repeat mode):

| Depth | no JAM in sight | JAM in speculative frames | boundary t=J-1 | jammed state loaded |
|-------|-----------------|---------------------------|----------------|---------------------|
| 1 | 22 | 2 | 2 | 336 |
| 2 | 20 | 4 | 2 | 336 |
| 3 | 18 | 6 | 2 | 336 |

- Per-run assertions hold at every depth: T-J real ticks silent, T-J load-has-work skips, every tick at or after J-N shows the oracle's video[J-1], real/visible video, real/speculative audio and saved state match.
- Controls on the JAM image: skip-load 218, stale-load 416, late-input 181, audio-leak 180, black-oracle 338 mismatches; all five tripped (`jam_controls_tripped=5`). Nesteroids controls still 4/4. `jam_images=1`, `RAH_MIN_ROMS` still `7u`.
- Sensitivity: re-adding `invalidate_presentation()` after a successful load (the pre-change behaviour) makes `libretro.runahead` fail with "real video differs at tick 12" (= J); restored afterwards.
- Diverged-timeline case (`libretro.host`): load of the frame-N state from the A-held timeline then JAM gives an all-black frame; load of the state the cached frame produced gives P_A; a state with another frame number gives black; back to the cache's timeline gives P_A again. Control: dropping the `cached_timeline == timeline` comparison made the S_B leg fail (frame showed P_A); restored, not committed.
- ci: `ctest --preset ci` 669/670 pass; the single failure is the expected `accuracycoin.activation` (build-input fingerprint, Plan 19-06). asan: `libretro.host` 0.73 s, `libretro.runahead` 15.23 s (about 16 s total), no sanitizer report.

## Per-frame cost of the timeline identity (measured, not gated)

Scratch program outside the repo, ci Release build of the core library, 600 calls each after 60 warm-up frames:

| ROM | run_frame mean | save_state mean | ratio | state bytes |
|-----|----------------|-----------------|-------|-------------|
| dabg (first of two runs) | 1380.58 us | 26.70 us | 0.0193 | 13087 |
| dabg (second run) | 1220.02 us | 26.24 us | 0.0215 | 13087 |
| nesteroids (first run) | 2275.92 us | 12.00 us | 0.0053 | 4895 |
| nesteroids (second run) | 3662.54 us | 12.04 us | 0.0033 | 4895 |

Machine: Apple silicon arm64, macOS 26.6. About 0.3 to 2 percent of a frame run; no CI performance gate added.

## Accepted residual (D-16 amendment, 2026-10-25, owner)

When a mispredicted run-ahead input (real input differs from the speculative input) leads the real timeline into a JAM on the first real frame after a load, the frozen real frame was never retained and STOP_JAM shows black. Black is the safe direction. It is stated in the adapter comment and the LIBRETRO-AND-RUNNER.md planning row (both citing the D-16 amendment), and in the README and public header (no decision IDs). The D-17 oracle sequence, where real and speculative input agree, always shows video[J-1].

## Deviations from Plan

**1. [Rule 1 - Bug] Showing black clobbered the cached picture**
- **Found during:** Task 2 (diverged-timeline leg)
- **Issue:** the STOP_JAM black path did `memset(xrgb, 0, ...)`, destroying the cached frame, so a later load back to the cache's timeline showed black instead of the picture.
- **Fix:** send a separate never-written zero buffer (`blank`) for black; the cache is untouched.
- **Files modified:** libretro/libretro.c
- **Commit:** 13932e8

**2. [Plan detail] Backdrop sequence of the JAM image**
- The image writes counter + `$10` (values `$11`-`$1C`) and JAMs on the 13th NMI instead of `counter AND $3F` skipping `$0D`; with 12 values before the JAM the skip is never reached. Sequence is at executor discretion per the plan.

**Total deviations:** 1 auto-fixed (bug), 1 discretionary detail. **Impact:** none on scope.

Tasks 1-3 are one commit (D-20 commit 2) as the plan specifies, so there are no separate per-task hashes.

## Known Stubs

None.

## Threat Flags

None. The adapter gained one heap allocation (the scratch state buffer) freed on unload, load_game and deinit.

## Commit

`13932e8 fix(libretro): keep the JAM picture under run-ahead`

```
 .planning/preparation/LIBRETRO-AND-RUNNER.md |   4 +-
 README.md                                    |  30 ++-
 include/nesturbator.h                        |   9 +-
 libretro/libretro.c                          | 135 +++++++++++-
 tests/libretro/libretro_host.c               | 313 +++++++++++++++++++++++++--
 tests/runahead_model.h                       | 128 +++++++++--
 6 files changed, 557 insertions(+), 62 deletions(-)
```

Ready for Plan 19-04.

## Self-Check: PASSED

- libretro/libretro.c, tests/libretro/libretro_host.c, tests/runahead_model.h: present and committed in 13932e8.
- `grep -c cached_frame_number` = 5, `cached_timeline` = 7, `retro_unserialize` has no `invalidate_presentation`, `D-16` present, `mispredict` in README, `D-16` in LIBRETRO-AND-RUNNER.md, `define RAH_MIN_ROMS 7u` unchanged, `jam_images` count 5.
