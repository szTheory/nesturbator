---
phase: 10-close-out-and-boot-to-play
plan: 01
subsystem: ppu
tags: [ppu, nmi, ppuctrl, behaviour-revision, hashes]
requires: []
provides:
  - "PPUCTRL NMI edge rule in src/ppu.c"
  - "core.nmi_edge regression test"
  - "behaviour revision 6 and re-pinned runner hashes"
affects: [10-02, 10-03, 10-04, 10-05, 10-06, 10-07]
tech-stack:
  added: []
  patterns: ["synthetic NROM ROM via tests/ines.h for end-to-end core tests"]
key-files:
  created: [tests/core/test_nmi_edge.c]
  modified: [src/ppu.c, tests/ppu/test_registers.c, tests/core/test_reset.c, tests/CMakeLists.txt, include/nesturbator.h, tests/core/test_api.c, tests/cmake/vector_api_policy.cmake, tests/runner/hashes.txt, README.md, .planning/phases/05-tune-up-and-v1-debt/05-REVIEW-DISPOSITION.md]
key-decisions:
  - "$2000 raises NMI only on bit 7 going 0 to 1 with vblank set; clears pending only when bit 7 is 0 (D-01)"
  - "CR-01 no longer reproduces: closed with a library-level sentinel test, no core change (D-19)"
  - "Behaviour revision raised once, to 6, with 24 hash rows re-pinned (D-03)"
requirements-completed: [BOARD-04, PLAY-01]
status: complete
commits: 3
plan_head_before: 3de6ec09431b6ffb6ba097b215fda9cbb30ec954
plan_head_after: 3fd54b757da725187d9d4b15cd0739c1d985e88e
actuals:
  tokens: 14000
  tasks: 3
  commits: 3
duration: 25 min
completed: 2026-10-11
---

# Phase 10 Plan 01: PPUCTRL NMI edge fix, CR-01 closure, revision 6 Summary

A `$2000` write now raises an NMI only on a bit 7 0-to-1 change while vblank is set, ending the NMI storm that kept nes-runner on its title, with revision 6 and 24 re-pinned hash rows.

## Tasks

| Task | Commit | Result |
|------|--------|--------|
| 1 Edge fix (tracer) | 3de0c41 | Red first: pre-fix count was 1086 at frame 10. Fixed; `core.nmi_edge` counts exactly n NMIs for n = 1..10; five register cases in `ppu.registers` |
| 2 CR-01 sentinel | 94bee54 | `check_reset_sentinel_frame` passed on the fixed core |
| 3 Revision 6 + re-pin | 3fd54b7 | Four revision sites, README and header text, hashes re-pinned |

Tracer verified end-to-end before expansion (core.nmi_edge, ppu.* and core.* subsets passed).

## CR-01 branch taken

No-fix branch. The sentinel test (0xFFFF pre-fill, pre-reset frames 0..120, first two frames; plus 0x0000 vs 0xFFFF equality for pre-reset 0, 1, 60 over three frames) passed on the Task 1 core. `src/instance.c`, `src/cpu.c` and `src/frame.c` are unchanged. The CR-01 row in `05-REVIEW-DISPOSITION.md` is now `fixed` with the evidence. Only host-buffer independence is claimed (assumption A5).

## Moved hash rows (24, exact set verified against 10-RESEARCH.md)

Reason for all: games that rewrite `$2000` inside their NMI handler no longer re-enter NMI, so their frames and tick counts change under revision 6.

- nesteroids/boot/frame 30, 60, 120, 180 (hash and ticks)
- dabg/p0, dabg/p1, dabg/both, frame 60, 120, 180 (9 rows)
- rhde/boot/frame 30, 60, 120, 180
- holymapperel ticks-only rows: m1surom/frame 200; m1sxrom/frame 400; m1sxrom.saved/frame 400; m4tnrom/frame 600; m4tkrom/frame 600; m4tkrom.saved/frame 600; m4w8k/frame 600

No audio row and no frame 1 row moved; hashes.txt stays at 50 rows; the AccuracyCoin scoreboard is unchanged. `cmake --workflow --preset ci` passes (447 tests) and `cmake --workflow --preset hygiene` passes.

## Deviations from Plan

- **[Rule 3 - Blocking] Helper name clash.** `tests/core/test_reset.c` already had `run_into`; my new helper is named `run_one_into`. No behaviour change.
- The edge variable in `src/ppu.c` is named `old` to match the plan's acceptance grep.

**Total deviations:** 2 trivial. **Impact:** none.

## Known Stubs

None.

## Threat Flags

None.

## Self-Check: PASSED

Files present (test_nmi_edge.c and the modified files); commits 3de0c41, 94bee54, 3fd54b7 are ancestors of HEAD; acceptance greps and the moved-row set check passed.
