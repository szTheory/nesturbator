---
phase: 06-mapper-seam-and-ppu-fetch-pipeline
verified: 2026-10-10T18:00:00Z
status: passed
score: 5/5 must-haves verified
covered_files:
  - ".planning/phases/06-mapper-seam-and-ppu-fetch-pipeline/06-01-PLAN.md"
  - ".planning/phases/06-mapper-seam-and-ppu-fetch-pipeline/06-01-SUMMARY.md"
  - ".planning/phases/06-mapper-seam-and-ppu-fetch-pipeline/06-02-PLAN.md"
  - ".planning/phases/06-mapper-seam-and-ppu-fetch-pipeline/06-02-SUMMARY.md"
  - "include/nesturbator.h"
  - "src/apu.c"
  - "src/bus.c"
  - "src/cartridge.c"
  - "src/internal.h"
  - "src/mapper.h"
  - "src/mapper_nrom.c"
  - "src/ppu.c"
  - "tests/core/test_mapper.c"
  - "tests/mapper_test.h"
  - "tests/ppu/ppu_fixture.h"
  - "tests/ppu/test_fetch.c"
  - "tests/ppu/test_split_scroll.c"
  - "tests/runner/hashes.txt"
covered_digest: "v3:sha256:d44f1129446ce8fa370fc42cb5aabdf6ba50efdd89b7aac4177c2b96cfd3a051"
behavior_unverified: 0
---

# Phase 6: Mapper seam and PPU fetch pipeline Verification Report

**Phase Goal:** Every cartridge loads through one per-board mapper interface, mid-frame scroll changes render as on the console, and A12 rises on the hardware's dots.
**Status:** passed. **Re-verification:** No.

## Observable Truths (ROADMAP success criteria)

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | NROM loads through the ops table; v1 hashes byte-identical at seam commit | VERIFIED | src/mapper.h, mapper_nrom.c and cartridge.c exist. No switch on the mapper id in bus.c or ppu.c (grep). `git diff f1b3309 f5e9d86 -- tests/runner/hashes.txt tests/accuracy/scoreboard.txt include/nesturbator.h` is empty. CI run 38070510500 on f5e9d86 concluded success (confirmed with gh). |
| 2 | Test board sees each write with its CPU cycle; mapper IRQ ORed with APU sources | VERIFIED | `core.mapper` (tests/core/test_mapper.c, tests/mapper_test.h) passes locally and in CI. It covers write stamps, the eight-combination IRQ OR, and the empty-struct edge. See WR-01 below. |
| 3 | Split-scroll test; v increments and copies; fetch dots with A12 levels, empty slots included | VERIFIED | `ppu.fetch` and `ppu.split_scroll` pass locally. Both files are non-stub with real assertions. |
| 4 | Revision bumped once; re-pinned hashes listed with reasons; six-platform agreement; scoreboard loses no row | VERIFIED | `NESTURBATOR_BEHAVIOUR_REVISION 5`. Restricted to source paths it is introduced by exactly one commit, 9389198. Eight rows re-pinned (nesteroids frames 30/60/120/180 and rhde frames 30/60/120/180), each with a reason in 06-02-SUMMARY. Ticks and audio are unchanged. The scoreboard diff is empty. CI run 38072045594 on 62fb2bb concluded success (confirmed with gh), including hash-equality, all six build legs and the scoreboard job. No `sha256 <64 hex>` line is added or removed in the tests/CMakeLists.txt diff. |
| 5 | asan, nofp and hygiene pass; core links only C memory functions | VERIFIED | CI jobs asan, nofp (Linux, which runs the abi checks) and hygiene are green. Orchestrator ran the local ci preset at 361/361. The local macOS nofp failure on `___stack_chk_*` is a pre-existing platform artifact; the Linux nofp gate is authoritative. |

**Score:** 5/5. No truths are behavior-unverified.

## Requirements Coverage

| Requirement | Source | Status | Evidence |
|-------------|--------|--------|----------|
| MAP-01 | 06-01 | SATISFIED | Truth 1 and truth 2 |
| MAP-02 | 06-02 | SATISFIED | Truth 3 and truth 4 |

No orphaned requirements. REQUIREMENTS.md maps only MAP-01 and MAP-02 to Phase 6. MAP-03 belongs to Phase 9.

## Behavioral Spot-Checks

`ctest -R "core.mapper|ppu.fetch|ppu.split_scroll|ppu.render|runner"` in build/ci: 24/24 passed.

## Anti-Patterns

No unreferenced TBD, FIXME or XXX markers were found as blockers. The code review has 0 critical findings.

## Advisory (code review, all open, not blocking)

- WR-01: the mapper write stamp equals cycles completed including the write, which is one above the zero-based index that the comment documents. Relative differences are correct. Phase 8 (MMC1) and Phase 9 (MMC3) should settle the contract wording or the value before relying on absolute stamps.
- WR-02: `$2007` treats v >= $4000 as non-palette.
- WR-03: `nesturbator__map_cpu_read` indexes `cpu_r` with no range guard.
- IN-01: an unknown mapper id loads as an empty cartridge. This is superseded by MAP-03 in Phase 9.
- IN-02: contradictory comments on the NROM stamp.

## Notes for later phases

Performance: frames per second fell about 19 percent against the seam baseline. It is measured, not gated. The odd-frame A12 difference at line 0 is recorded as a Phase 9 input.

## Gaps Summary

None. The phase goal is achieved.

_Verified: 2026-10-10_
_Verifier: Claude (gsd-verifier)_
