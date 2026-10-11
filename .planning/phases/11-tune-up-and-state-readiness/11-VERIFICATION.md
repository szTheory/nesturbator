---
phase: 11-tune-up-and-state-readiness
verified: 2026-10-11T23:00:00Z
status: human_needed
score: 7/8 must-haves verified
covered_files:
  - ".github/workflows/ci.yml"
  - ".planning/phases/11-tune-up-and-state-readiness/11-01-PLAN.md"
  - ".planning/phases/11-tune-up-and-state-readiness/11-01-SUMMARY.md"
  - ".planning/phases/11-tune-up-and-state-readiness/11-02-PLAN.md"
  - ".planning/phases/11-tune-up-and-state-readiness/11-02-SUMMARY.md"
  - ".planning/phases/11-tune-up-and-state-readiness/11-03-PLAN.md"
  - ".planning/phases/11-tune-up-and-state-readiness/11-03-SUMMARY.md"
  - ".planning/phases/11-tune-up-and-state-readiness/11-04-PLAN.md"
  - ".planning/phases/11-tune-up-and-state-readiness/11-04-SUMMARY.md"
  - "README.md"
  - "libretro/libretro.c"
  - "runner/main.c"
  - "runner/save.c"
  - "runner/save.h"
  - "src/cartridge.c"
  - "src/instance.c"
  - "tests/core/test_boundary.c"
  - "tests/retroarch/run_retroarch.cmake"
  - "tests/runner/test_save_replace.c"
covered_digest: "v3:sha256:0e865bc834bafc196e70f83d4a624f84c0dc9ecd798f2272788a1918f231033a"
behavior_unverified: 1
behavior_unverified_items:
  - truth: "The retroarch-e2e job on the phase pull request passes the MMC1 and MMC3 save round trips inside RetroArch"
    test: "Open the phase PR and let the hosted retroarch-e2e job run (automated gate, no person needed)"
    expected: "Both save legs pass; saves/m4tkrom.srm is 8192 bytes with SAVEDATA at 0x100; artifact upload finds every file"
    why_human: "Runs only on hosted macOS runners on a PR; no local ctest case launches RetroArch"
human_verification:
  - test: "PR retroarch-e2e job (automated hosted gate, recorded in 11-UAT.md)"
    expected: "MMC3 round trip green alongside MMC1"
    why_human: "Not yet executed; the gate the plan designated. Not a manual test"
---

# Phase 11: Tune-up and state readiness Verification Report

**Phase Goal:** Battery saves can never be left torn or stale, MMC3 saves survive RetroArch sessions, and every public function has a caller and README text, so state work inherits no v2 debt.
**Verified:** 2026-10-11
**Status:** human_needed (one pending automated hosted gate; no gaps)
**Re-verification:** Yes, after review fixes WR-01..WR-04 and the clang-format pass (previous report was stale; no gaps were open)

## Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | SC1: failure injection (unwritable dir, dir at target, dir at .tmp) leaves old .sav intact and no .tmp | VERIFIED | runner/save.c `nesturbator_run_file_replace`: lstat of .tmp (directory refused with EISDIR, regular file or symlink removed), `fopen "wbx"`, fwrite, fflush, sync, checked fclose, rename or MoveFileExA, `remove(tmp)` on any failure. Tests runner.save_replace, runner.save_fsize, runner.save.dir_at_tmp pass in this run |
| 2 | Flush wrapper updates shadow on success only | VERIFIED | `nesturbator_run_save_flush` memcpy follows a zero return only |
| 3 | SC2: retroarch-e2e has an MMC3 `.srm` round trip, marker-based, no sleeps | VERIFIED (wiring) | ci.yml MMC3 step with SAVE_ROM m4tkrom.nes, SAVE_SIZE 8192, SAVE_FRAME 600; run_retroarch.cmake validates SAVE_SIZE; covered by the unchanged files since the last report |
| 4 | MMC3 round trip passes inside RetroArch | PRESENT_BEHAVIOR_UNVERIFIED | Only the hosted `retroarch-e2e` job on the PR runs it. RetroArch.app exists locally, but no ctest case launches it (tests/retroarch/CMakeLists.txt: 488 tests, only retroarch.compare and retroarch.compare.cli), and the script needs the pinned version and a GUI session; not run locally. Recorded as an automated hosted gate in 11-UAT.md |
| 5 | SC3: `nesturbator_unload_cartridge` has a caller, a ci test, README text | VERIFIED | libretro/libretro.c calls it; core.cartridge and libretro.host pass; README mentions it |
| 6 | SC3: `nesturbator_get_version` has a caller, a ci test, README text | VERIFIED | `build/ci/runner/nesturbator-run --version` prints `nesturbator-run 0.1.9`, equals version.txt; runner.version passes |
| 7 | Every public function has a README mention and a runner or libretro caller | VERIFIED | Loop over all 13 header functions: README count >= 1 and caller file count >= 1 for each |
| 8 | Frame-boundary white-box checks; exit 3 unused | VERIFIED | core.boundary passes |

**Score:** 7/8 truths verified (1 present and wired, behavior-unverified: the hosted RetroArch run)

## Contract Agreement After Review Fixes

| Point | runner/save.h | README.md (~942-949, 972-977) | runner/save.c / tests | Agree |
|-------|---------------|-------------------------------|-----------------------|-------|
| Symlink at .tmp (WR-01, WR-02) | stale regular file or symlink removed, link unlinked, target untouched | regular file or symlink, even dangling, cleared by next write | lstat then `remove()` (unlinks a link, never follows); tests (h) victim unchanged, (i) dangling link cleared and save succeeds | Yes |
| Directory at .tmp | refused and left alone | covered by `.tmp` directory cases | S_ISDIR returns EISDIR, nothing removed; runner.save.dir_at_tmp | Yes |
| Apple sync (WR-03) | F_FULLFSYNC on Apple, fsync elsewhere | "synced (with F_FULLFSYNC on macOS)" | `sync_stream`: F_FULLFSYNC with fsync fallback; `_DARWIN_C_SOURCE` set under `__APPLE__`; `_commit` on Windows | Yes |
| Never torn, no dir sync | old or new, never torn; power cut may show previous | same statement | no directory fsync | Yes |
| chmod case (WR-04) | n/a | n/a | test prints `note:` and checks the privileged write when chmod does not block | Yes |

Windows branch (`#define lstat _stat`) cannot be compiled on this host; it is exercised by the Windows CI runners.

## Local Verification Run

`cmake --workflow --preset ci` from the repo root: exit 0, configure, build, test (100% tests passed out of 488, 42.7 s), package. Spot checks passing individually: runner.save_replace, runner.save_fsize, runner.save.dir_at_tmp, core.boundary, core.cartridge, libretro.host, runner.version.

## Requirements Coverage

| Requirement | Source Plan | Status | Evidence |
|-------------|-------------|--------|----------|
| TUNE-07 | 11-01 | SATISFIED | Truths 1-2 |
| TUNE-08 | 11-04 | SATISFIED in code and wiring; hosted run pending | Truths 3-4 |
| TUNE-09 | 11-02, 11-03 | SATISFIED | Truths 5-8 |

All three IDs are claimed by plans and mapped to Phase 11 in REQUIREMENTS.md. No orphans.

## Anti-Patterns / Review

11-REVIEW-DISPOSITION.md: WR-01..WR-04 fixed. IN-01..IN-03 remain open (info only: power-on duplication in load_cartridge, zeroed av info on failed create, long comment line); none blocks the goal. No TBD/FIXME/XXX markers in the phase's modified source files.

## Gaps Summary

No gaps. The one remaining item is TUNE-08's real RetroArch MMC3 round trip, provable only by the hosted `retroarch-e2e` job when the phase pull request opens. It is an automated gate, not a human test.

_Verifier: Claude (gsd-verifier)_
