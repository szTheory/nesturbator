---
phase: 10-close-out-and-boot-to-play
verified: 2026-10-11T00:00:00Z
status: passed
score: 4/4 must-haves verified
covered_files:
  - ".planning/phases/10-close-out-and-boot-to-play/10-01-PLAN.md"
  - ".planning/phases/10-close-out-and-boot-to-play/10-01-SUMMARY.md"
  - ".planning/phases/10-close-out-and-boot-to-play/10-02-PLAN.md"
  - ".planning/phases/10-close-out-and-boot-to-play/10-02-SUMMARY.md"
  - ".planning/phases/10-close-out-and-boot-to-play/10-03-PLAN.md"
  - ".planning/phases/10-close-out-and-boot-to-play/10-03-SUMMARY.md"
  - ".planning/phases/10-close-out-and-boot-to-play/10-04-PLAN.md"
  - ".planning/phases/10-close-out-and-boot-to-play/10-04-SUMMARY.md"
  - ".planning/phases/10-close-out-and-boot-to-play/10-05-PLAN.md"
  - ".planning/phases/10-close-out-and-boot-to-play/10-05-SUMMARY.md"
  - ".planning/phases/10-close-out-and-boot-to-play/10-06-PLAN.md"
  - ".planning/phases/10-close-out-and-boot-to-play/10-06-SUMMARY.md"
  - ".planning/phases/10-close-out-and-boot-to-play/10-07-PLAN.md"
  - ".planning/phases/10-close-out-and-boot-to-play/10-07-SUMMARY.md"
  - "include/nesturbator.h"
  - "src/ppu.c"
  - "tests/cmake/hash_keys.cmake"
  - "tests/core/test_nmi_edge.c"
  - "tests/libretro/libretro_host.c"
  - "tests/runner/hashes.txt"
  - "tests/runner/test_boot_to_play.c"
covered_digest: "v3:sha256:630c1ecdf749bda3f06b1f4407455a29a344086d071efa08487536d8bb9d7c3b"
behavior_unverified: 0
---

# Phase 10: Close-out and boot-to-play Verification Report

**Phase Goal:** Every board's output is the same on all six platforms, and a game is proven by an automated check to play from boot through interactive play.
**Status:** passed (initial verification)

## Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | nes-runner ROM committed unmodified | VERIFIED | `tests/roms/runner.nes` is 81,936 bytes, sha256 1a67bfab...4384, matching the plan. |
| 2 | NMI edge fix, revision 6, documented | VERIFIED | `src/ppu.c` and `include/nesturbator.h` changed (only library files). The revision is 6 in the header, `test_api.c` and the README. `core.nmi_edge`, `core.reset` and `ppu.registers` pass. |
| 3 | Boot-to-play proven by automated check with controls and mutants | VERIFIED | `runner.boot_to_play` passes in 4.65 s. It holds the judge predicates, the four controls, the mutants and the boundary pairs. |
| 4 | Parity of frames through libretro for every board (BOARD-04) | VERIFIED (local) | There are exactly 19 `libretro.parity.*` tests, and those I ran pass. The hash inventory has 95 rows. `policy.rom_hash_coverage` and its self-test pass. |

## Requirements Coverage

| ID | Plans | Status |
|----|-------|--------|
| BOARD-04 | 10-01, 10-02, 10-05, 10-06, 10-07 | SATISFIED. Equality across the six platforms can only be shown by the hosted CI matrix at PR time. Every locally checkable part is in place: expanded hash rows, parity tests, drift guards. |
| PLAY-01 | 10-01 to 10-07 | SATISFIED by `runner.boot_to_play`, which runs on ci, ci-msvc and asan. |

No orphaned requirements: REQUIREMENTS.md maps only these two IDs to Phase 10, and both appear in plan frontmatter.

## Behavioral Spot-Checks

I ran 74 selected ctest tests (boot_to_play, nmi_edge, core.reset, hash, policy, parity, game_movie) under the ci preset, and all 74 passed. The full ci suite lists 480 tests.

## Anti-Patterns and Review

10-REVIEW.md has 3 warnings and 3 info items, all in test code. They are recorded as open in 10-REVIEW-DISPOSITION.md. I treat them as non-blocking.

## Notes (not gaps)

- The hosted six-platform matrix has not run on this branch (not pushed). This is automated verification that happens at PR time, not human verification.
- `nofp` fails locally on `abi.undefined_symbols` (`___stack_chk_*`). This is the known macOS toolchain issue from phases 05 and 08. Phase 10 changed only `src/ppu.c` in the library. I did not re-run nofp or asan myself and rely on the executors' asan 480/480 report.
- The PR title is recorded in 10-07-SUMMARY.md: `feat: NMI edge fix, board parity close-out and nes-runner boot-to-play`.

_Verifier: Claude (gsd-verifier)_
