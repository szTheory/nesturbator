---
phase: 11-tune-up-and-state-readiness
verified: 2026-10-11T22:00:00Z
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
  - "libretro/libretro.c"
  - "runner/main.c"
  - "runner/save.c"
  - "runner/save.h"
  - "src/cartridge.c"
  - "src/instance.c"
  - "tests/retroarch/run_retroarch.cmake"
covered_digest: "v3:sha256:d9c19853b73c6dbaef644f9709891572ab2a788106d4dc6ce000fa2bd35d2632"
behavior_unverified: 1
behavior_unverified_items:
  - truth: "The retroarch-e2e job on the phase pull request passes the MMC1 and MMC3 save round trips inside RetroArch"
    test: "Open the phase PR and let the hosted retroarch-e2e job run (automated gate, no person needed)"
    expected: "Both save legs pass; saves/m4tkrom.srm is 8192 bytes with SAVEDATA at 0x100; artifact upload finds every file"
    why_human: "Runs only on hosted macOS runners on a PR; cannot run locally"
human_verification:
  - test: "PR retroarch-e2e job (automated hosted gate)"
    expected: "MMC3 round trip green alongside MMC1"
    why_human: "Not yet executed; the gate the plan designated"
---

# Phase 11: Tune-up and state readiness Verification Report

**Phase Goal:** Battery saves can never be left torn or stale, MMC3 saves survive RetroArch sessions, and every public function has a caller and README text.
**Status:** human_needed (one pending automated hosted gate; no gaps)
**Re-verification:** No

## Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | SC1: failure-injection test in ci: unwritable dir and dir at target leave old .sav intact, no .tmp | VERIFIED | `runner/save.c` `nesturbator_run_file_replace`: stat .tmp (refuses directory), `fopen "wbx"`, fwrite, fflush, fsync, checked fclose, rename/MoveFileExA, remove .tmp on failure. `ctest --preset ci`: runner.save_replace, runner.save_fsize, runner.save.dir_at_tmp, runner.save.interval_retry, runner.save.unchanged pass. Full `cmake --workflow --preset ci` ran here: 488 tests, all passed |
| 2 | Flush wrapper updates shadow on success only; single call site | VERIFIED | `nesturbator_run_save_flush` memcpy after success only; `battery_save` struct rename present in runner/main.c |
| 3 | SC2: retroarch-e2e has MMC3 `.srm` round trip, marker-based, no sleeps | VERIFIED (wiring) | ci.yml step "Compare the RetroArch MMC3 battery save round trip" with SAVE_ROM=m4tkrom.nes, SAVE_FRAME=600, SAVE_SIZE=8192, own RA_DIR; artifacts added with `if-no-files-found: error`; `SAVE_SIZE` validated in run_retroarch.cmake; no `sleep` in script or workflow. m4tkrom is built by the ci preset (holymapperel derive) |
| 4 | MMC3 round trip actually passes inside RetroArch | PRESENT_BEHAVIOR_UNVERIFIED | Only runs in hosted `retroarch-e2e` on the PR (not yet opened). Local evidence: script validation and workflow wiring only. The designated automated gate is pending |
| 5 | SC3: `nesturbator_unload_cartridge` has a caller, a ci test, README text | VERIFIED | libretro/libretro.c lines 292 and 322 call it; adapter keeps one instance init..deinit; src/cartridge.c unload runs the shared power-on helper in src/instance.c; core.cartridge and libretro.host pass; README lines ~708 describe it |
| 6 | SC3: `nesturbator_get_version` has a caller, a ci test, README text | VERIFIED | runner/main.c `--version` calls it; `build/ci/runner/nesturbator-run --version` prints `nesturbator-run 0.1.9`, exit 0, equals version.txt; runner.version and runner.usage.version_after_bad pass; README line ~702 |
| 7 | Every public function has a README mention and a runner/libretro call site | VERIFIED | Loop over all 13 declarations in include/nesturbator.h: each has readme >= 1 and >= 1 caller file |
| 8 | Frame-boundary white-box checks; exit 3 unused | VERIFIED | core.boundary passes; no "exit 3" in runner or runner tests |

**Score:** 7/8 truths verified (1 present, behavior-unverified: the hosted RetroArch run)

## Requirements Coverage

| Requirement | Source Plan | Status | Evidence |
|-------------|-------------|--------|----------|
| TUNE-07 | 11-01 | SATISFIED | Truths 1-2 |
| TUNE-08 | 11-04 | SATISFIED in code and wiring; hosted run pending | Truths 3-4 |
| TUNE-09 | 11-02, 11-03 | SATISFIED | Truths 5-8 |

All three IDs are claimed by plans and mapped to Phase 11 in REQUIREMENTS.md. No orphans.

## Anti-Patterns / Review

11-REVIEW-DISPOSITION.md lists 7 open advisory findings, none a goal blocker:
- WR-01: `save.h` says a symlink at `.tmp` is "never removed" but the helper removes a regular-file or symlink-to-file there (documentation/contract mismatch; the symlink target is never written, so the prohibition on touching victims holds, as the test shows).
- WR-02: a dangling symlink at `.tmp` makes saves fail permanently (fails closed, old save kept).
- WR-03: `fsync` on macOS is not a full durable flush (F_FULLFSYNC); the torn-file guarantee still holds.
- WR-04: chmod and symlink sub-cases can pass without testing (e.g. as root).
- IN-01..03: duplication, zeroed av info on failed create, long comment line.

No TBD/FIXME/XXX markers were checked beyond the review; none were reported.

## Gaps Summary

No gaps. The one remaining item is TUNE-08's real RetroArch MMC3 round trip, which only the hosted `retroarch-e2e` job on the phase pull request can prove. It is an automated gate, so the phase moves on once that job is green. The MMC3 truth is not counted as verified until then. Resolve the open WR-01 to WR-04 findings or record dispositions before merge.

_Verifier: Claude (gsd-verifier)_
