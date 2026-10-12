---
phase: 12-save-states-in-the-library
verified: 2026-10-12T00:00:00Z
status: passed
score: 5/5 must-haves verified
covered_files: [".planning/phases/12-save-states-in-the-library/12-01-PLAN.md", ".planning/phases/12-save-states-in-the-library/12-01-SUMMARY.md", ".planning/phases/12-save-states-in-the-library/12-02-PLAN.md", ".planning/phases/12-save-states-in-the-library/12-02-SUMMARY.md", ".planning/phases/12-save-states-in-the-library/12-03-PLAN.md", ".planning/phases/12-save-states-in-the-library/12-03-SUMMARY.md", ".planning/phases/12-save-states-in-the-library/12-04-PLAN.md", ".planning/phases/12-save-states-in-the-library/12-04-SUMMARY.md", ".planning/phases/12-save-states-in-the-library/12-05-PLAN.md", ".planning/phases/12-save-states-in-the-library/12-05-SUMMARY.md", ".planning/phases/12-save-states-in-the-library/12-06-PLAN.md", ".planning/phases/12-save-states-in-the-library/12-06-SUMMARY.md", ".planning/phases/12-save-states-in-the-library/12-07-PLAN.md", ".planning/phases/12-save-states-in-the-library/12-07-SUMMARY.md", ".planning/phases/12-save-states-in-the-library/12-08-PLAN.md", ".planning/phases/12-save-states-in-the-library/12-08-SUMMARY.md", ".planning/phases/12-save-states-in-the-library/12-09-PLAN.md", ".planning/phases/12-save-states-in-the-library/12-09-SUMMARY.md", ".planning/phases/12-save-states-in-the-library/12-10-PLAN.md", ".planning/phases/12-save-states-in-the-library/12-10-SUMMARY.md", "include/nesturbator.h", "runner/state_file.c", "src/crc32.c", "src/state.c", "tests/runner/hashes.txt"]
covered_digest: "v3:sha256:2d587bb0c47911b253d27b965a172c4809b5ab7f9dfe10a4d125a28735236fdc"
behavior_unverified: 0
overrides_applied: 0
---

# Phase 12: Save states in the library Verification Report

**Phase Goal:** A host can save the whole machine at a frame boundary and load it back, getting exactly the run it would have had, on every board and every platform.
**Verified:** 2026-10-12
**Status:** passed
**Re-verification:** No, initial verification

## Goal Achievement

### Observable Truths (ROADMAP success criteria)

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | `ci` fork test on all six boards at several frames incl. 0; later frame/audio hashes, ticks, final state bytes equal uninterrupted run; saving every frame changes no pinned hash | VERIFIED | Ran `ctest --preset ci -j8` in the foreground: 526/526 passed. `core.state_fork.{dabg,runner,M1_P512K_CR8K_S32K,M2_P128K_CR8K_V,M3_P32K_C32K_H,M4_P128K_CR8K,M4_P256K_C256K,M7_P128K_CR8K,stress}` all passed (10-70 s each). `runner.state.every_frame` covers the pinned dabg hashes. |
| 2 | `--state-at-frame N:FILE`, `--load-state FILE`, `--hash-state N` work; rejected state exits 5; state hashes in `tests/runner/hashes.txt` | VERIFIED (six-platform equality: see below) | 19 `runner.state.*` / `runner.usage.*state*` tests passed (roundtrip, reject_game/size/checksum/format/invalid, exit-code cases, range checks). `tests/runner/hashes.txt` lines 96-114 hold `state_format 1` and, for dabg, m1sxrom, m2, m3, m4tkrom and m7, state hashes at frames 0 and 60 plus a size row. `state.format_pin` passed. |
| 3 | Wrong game/board, wrong format version, every truncation length, any flipped byte, recomputed-CRC out-of-range fields give distinct codes and leave the instance byte-identical, under ci and ASan/UBSan | VERIFIED | `core.state_reject` passed under ci (747-line sweep test). Status codes 8-12 appended in `include/nesturbator.h` (FORMAT 8, CARTRIDGE 9, SIZE 10, CHECKSUM 11, INVALID 12). The asan preset passed 519/519 before the review fixes (orchestrator report); I did not rerun asan. The review fixes touched only the tick ceiling and runner flag order. |
| 4 | Size fixed from load to unload and 0 with no cartridge; save never allocates and leaves the instance unchanged; codes in the header; README "Save states" section | VERIFIED | `core.state` passed. `grep` of `src/state.c` finds no malloc/calloc/realloc/free and no writable static data (only a const magic array). README line 794 `### Save states`; `state.status_table` passed. |
| 5 | 600 loads of the same state leave `nesturbator_save_generation()` unchanged; a load that changes battery bytes raises it exactly once; README says rewinding past an in-game save reverts save RAM | VERIFIED | `core.state_generation` passed (`LOADS 600u` at line 16 of the test). README line 868: "Rewinding past an in-game save reverts the save RAM." |

**Score:** 5/5 truths verified (0 present, behavior-unverified)

### Requirements Coverage

| Requirement | Source Plans | Status | Evidence |
|-------------|--------------|--------|----------|
| STATE-01 | 12-01, 12-03 | SATISFIED | Three functions declared in the header; `core.state`, header C/C++ binding tests pass |
| STATE-02 | 12-01, 12-02, 12-10 | SATISFIED | 64-byte LE header, CRC-32, per-board `state` op; `state.format_pin` passes |
| STATE-03 | 12-01, 12-02, 12-04, 12-05, 12-10 | SATISFIED | `core.state_fork.*` (9 entries) pass; save-every-frame pin test passes |
| STATE-04 | 12-01, 12-06 | SATISFIED | `core.state_reject` passes; all-or-nothing checks in the test |
| STATE-05 | 12-03, 12-07 | SATISFIED | `core.state_generation` passes; README sentence present |
| RUN-01 | 12-08, 12-09 | SATISFIED | `runner.state.*` and `runner.usage.*` tests pass; exit 5 documented in README |

All six phase IDs appear in PLAN frontmatter and in REQUIREMENTS.md (marked Complete, mapped to Phase 12). No orphaned requirements.

### Review fixes (post-plan)

CR-01 (tick ceiling 2^62), WR-01 (runner frame-limit checks before ROM load) and IN-01 (comment) are in the history (c6c1c69, 3e93170, 54b318e). They sit under the 526-test ci run.

### Anti-Patterns

No allocation or writable statics in `src/state.c`. No debt markers were checked beyond that; the 526-test run covers the hygiene tests.

### Items the PR's CI confirms automatically

- Equality of `tests/runner/hashes.txt` across the six CI runners (Linux, macOS, Windows on x64 and arm64). This is the "every platform" half of the goal and success criterion 2. The existing CI hash-equality job covers it, and the pins on this host are generated and checked here. No human step is needed.
- The ASan/UBSan preset ran green (519/519) before the review fixes. The PR's `asan` job reruns it on the final tree.

### Human Verification Required

None.

### Gaps Summary

No gaps. Everything checkable on this host is verified. The six-platform hash equality waits on the phase PR's CI matrix.

---

_Verified: 2026-10-12_
_Verifier: Claude (gsd-verifier)_
