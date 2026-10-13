---
phase: 14-runner-process-contract
verified: 2026-10-12T00:00:00Z
status: passed
score: 4/4 must-haves verified
covered_files:
  - .planning/phases/14-runner-process-contract/14-01-PLAN.md
  - .planning/phases/14-runner-process-contract/14-01-SUMMARY.md
  - .planning/phases/14-runner-process-contract/14-02-PLAN.md
  - .planning/phases/14-runner-process-contract/14-02-SUMMARY.md
  - .planning/phases/14-runner-process-contract/14-03-PLAN.md
  - .planning/phases/14-runner-process-contract/14-03-SUMMARY.md
  - .planning/phases/14-runner-process-contract/14-04-PLAN.md
  - .planning/phases/14-runner-process-contract/14-04-SUMMARY.md
  - .planning/phases/14-runner-process-contract/14-05-PLAN.md
  - .planning/phases/14-runner-process-contract/14-05-SUMMARY.md
  - .planning/phases/14-runner-process-contract/14-06-PLAN.md
  - .planning/phases/14-runner-process-contract/14-06-SUMMARY.md
  - .planning/phases/14-runner-process-contract/14-07-PLAN.md
  - .planning/phases/14-runner-process-contract/14-07-SUMMARY.md
  - .planning/phases/14-runner-process-contract/14-08-PLAN.md
  - .planning/phases/14-runner-process-contract/14-08-SUMMARY.md
  - README.md
  - runner/exit_status.h
  - runner/main.c
  - runner/stop.c
  - runner/stop.h
  - tests/CMakeLists.txt
covered_digest: "v3:sha256:a6e9e8b354acfef97ce4eb0401663607a1530b60b1449f8aa702c562d9a3308e"
behavior_unverified: 0
overrides_applied: 0
---

# Phase 14: Runner process contract Verification Report

**Phase Goal:** A launcher such as Playstead can stop nes-runner at any time without losing a save, and can read every outcome from one documented exit status.
**Status:** passed
**Re-verification:** No, initial verification

## Observable Truths (ROADMAP success criteria)

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | Frame-indexed signal test on all six platforms: finish frame, flush save, exit 0 (or 1 on flush failure) | VERIFIED | `--raise-after N[:SIG[:K]]` in `runner/main.c`; stop polled before each frame, exit flush then `nesturbator_run_stop_done`. `runner.stop.frame_exact.{int,term,break}`, `flush_missing_dir`, `flush_dir_at_tmp` registered in `tests/CMakeLists.txt` and pass locally. Windows handler is a separate `SetConsoleCtrlHandler` path (`runner.stop.win_handler`, `win_console`). CI run 38324782351 is green on all 14 jobs (all six build platforms included) at head 31b5247. |
| 2 | Linux/macOS external SIGTERM: exit 0 within 2 s, whole `.sav`, no `.tmp`; second signal ends at once | VERIFIED | `runner.signal.{term,int,term_exitflush,second,load_error,second_in_flush}` pass locally on macOS (term 0.03 s). Handler resets to `SIG_DFL` then sets a `sig_atomic_t` flag (`runner/stop.c`), so a second signal kills at once. Timeout 2 s is set in `tests/CMakeLists.txt`; CI green on Linux and macOS. |
| 3 | One exit table (0,1,2,4,5; 3 and 77 reserved) in one header, `--help` and README; a test checks it; each status has a test | VERIFIED | `runner/exit_status.h` X-macro; `--help` output iterates the same macro (confirmed by running the binary); README "Exit status" table matches. `runner.exit.table` and `runner.exit.help` pass. Per-status tests: 0 `runner.exit.done`, 1 `runner.exit.failure`, 2 `runner.exit.usage`, 4 `runner.save.mismatch`, 5 `runner.state.reject_game`. No TBD/FIXME/XXX in the touched runner files. |
| 4 | README says Ctrl+Break and console close are handled on Windows and forced termination cannot be | VERIFIED | README "Stopping the runner": "On Windows the runner handles Ctrl+C, Ctrl+Break and console close the same way ... A forced termination (TerminateProcess, taskkill /F, power loss) cannot be handled". |

**Score:** 4/4 truths verified.

## Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|----------|---------|--------|--------|
| Runner exit and stop tests | `ctest -R "runner\.(exit\|stop\|signal\|usage)"` in `build/ci` | 62/62 passed | PASS |
| `--help` shows the table | `nesturbator-run --help` | 0,1,2,4,5 and "3 and 77 are reserved" printed | PASS |
| Full suite | orchestrator's `cmake --workflow --preset ci` | 585/585 | PASS (reported, not rerun) |
| CI | `gh run view 38324782351` | 14/14 jobs success, head 31b5247 | PASS |

The two later commits (d686252, 710b37c) change only review documents under `.planning/`.

## Requirements Coverage

| Requirement | Plans | Status | Evidence |
|-------------|-------|--------|----------|
| RUN-02 | 14-01..07 | SATISFIED | Truths 1, 2, 4; includes unmet-output exit 1 (`runner.stop.unmet_*`) and Windows Ctrl+Break/close handling. |
| RUN-03 | 14-01..07 | SATISFIED | Truth 3, plus the signal tests on all platforms and the external test on Linux and macOS. |

Plan 14-08 lists RUN-02 only, and RUN-02 and RUN-03 each appear in plan frontmatter; no orphans. REQUIREMENTS.md shows both unchecked, as the orchestrator intends.

## Anti-Patterns

None blocking. Review warnings (14-REVIEW.md), judged against the must-haves:

- WR-01 (`signal()` over an inherited `SIG_IGN`, restart semantics via `_DEFAULT_SOURCE`): robustness, not a must-have failure. The tests prove the restart behavior on the hosted platforms.
- WR-02 (tight CTest timeouts of 1 s and 2 s): flake risk, not a gap. The timeouts follow measured hosted runs and the final head was green on all jobs.
- WR-03 (POSIX README lacks the drain-stdout advice): documentation completeness only.

These are advisory follow-ups. 14-REVIEW-DISPOSITION.md records the disposition.

## Human Verification Required

None. Every check is automated.

## Gaps Summary

No gaps. Phase goal achieved.

---

_Verified: 2026-10-12_
_Verifier: Claude (gsd-verifier)_
