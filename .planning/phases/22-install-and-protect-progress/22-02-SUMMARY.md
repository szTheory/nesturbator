---
phase: 22-install-and-protect-progress
plan: "02"
subsystem: documentation
tags: [runner, cmake, ctest, documentation, tdd]
dependency-graph:
  requires:
    - phase: 22-01
      provides: canonical API guide and documentation-link/status-table checks
  provides:
    - canonical-runner-guide
    - bidirectional-runner-option-checks
    - canonical-exit-stop-relocation-controls
  affects: [release-guide, testing-guide, player-readme]
tech-stack:
  added: []
  patterns: [production-checker relocation fixtures, focused CMake self-tests]
key-files:
  created: [docs/runner.md]
  modified: [README.md, tests/CMakeLists.txt, tests/cmake/runner_options.cmake, tests/cmake/runner_exit_table.cmake]
key-decisions:
  - "docs/runner.md is the canonical home for runner options, examples, stop behavior and exit status."
  - "Runner save/state file behavior stays in the runner guide; native payload and library span contracts link to docs/api.md."
  - "Keep the checked MMC3 support clause in README while linking its detailed runner contract."
metrics:
  duration: 12 min
  completed: 2026-10-31
status: complete
actuals:
  tokens: 19136
  tasks: 2
  commits: 3
  plan_head_before: fac4464edf6ab29b4ee81a0a8321ebdcba7f0b10
  plan_head_after: 8a01df75e4e843826f981f88f00f7e4c453f5d6a
requirements-completed: []
---

# Phase 22 Plan 02: Runner Contracts Migration Summary

README routes runner users to a canonical guide that keeps executable option, stop and exit contracts checked against the real runner and exit header.

## Performance

- **Duration:** 12 min
- **Started:** 2026-10-31T01:07:37Z
- **Completed:** 2026-10-31T01:19:46Z
- **Tasks:** 2
- **Files modified:** 5

## Accomplishments

- Added [docs/runner.md](../../../docs/runner.md) with every CLI option and its ranges/interactions, movie and save/state examples, frame/audio/dump output, stop semantics, platform launcher qualifications, and the exact exit table.
- Changed both production documentation consumers to require `docs/runner.md`; added independent missing-guide and reason-specific contradiction controls that retain a stale README copy.
- Updated README links and moved its detailed movie/output/save-check presentation into the guide. The README's checked NTSC MMC3 support clause and marked Apple Silicon installation block remain intact.
- Kept state payload and battery-span semantics in [docs/api.md](../../../docs/api.md), with direct links from runner file behavior. Source links in the new guide are relative to `docs/`.

## Runner migration ledger

| Former README runner material | Maintained home |
|---|---|
| Usage syntax, option descriptions, `--bench`, AccuracyCoin modes/caps, validation, and range/interaction rules | `docs/runner.md` — The runner; AccuracyCoin cap values link to `tests/accuracy/budgets.txt` |
| Mapper/image diagnostics, frame/audio hashes, dumps, battery save files, intervals, and runner-specific state file errors/frames | `docs/runner.md` — option contracts and Output examples; native save and battery span contracts in `docs/api.md` |
| State capture/resume and two-port movie invocations and formats | `docs/runner.md` — Save and state examples; Input movies |
| JAM reporting, requested-output completion, signal handling, pipe flushing, forced termination, Windows console launch rules | `docs/runner.md` — Stopping the runner |
| Exit values, meanings, order, reserved values | `docs/runner.md` — Exit status, checked against `runner/exit_status.h` and real `--help` |
| Save writer failure evidence | `docs/runner.md` — Related runner checks, linked to the existing save/state/stop test scripts |
| Opening invocation/trainer guidance, supported-content summary, checked MMC3 clause | README; README links to the canonical guide for details |

## Verification

- **TDD tracer:** The RED self-test failed for the intended missing-guide condition while a complete stale README copy remained. `gsd_run check tdd-red-evidence` accepted the captured report and JSON as `RED_EVIDENCE_OK`. RED commit `07e5bf4`; GREEN commit `09c0df6`.
- **Tracer verification:** `cmake --preset ci` and `cmake --build --preset ci --target nesturbator-run` passed. The focused runner/link/script-policy selection passed 9/9, including `libretro.info_support`; it was rerun after the tracer commit with the same result.
- **Task 2:** `git diff --check` passed. The plan's runner-option/exit self-tests, state status-table checks, and documentation link checks passed 9/9. The option controls check both directions; exit controls compare exact ordered meanings and reserved values across header/help/guide; stop controls require each of five sentences and `--raise-after` placement. Relocation cases reject missing guides, missing/extra options, changed exit meaning, removed reserved text, and each removed stop sentence for its intended reason.
- Existing runtime save/state/stop and signal tests remain registered unchanged. The full `cmake --workflow --preset ci` run remains with the phase orchestrator; this docs migration did not rerun the signal campaign or perform host, private-content, player-progress, or release operations.

## Decisions Made

- Keep one canonical runner reference and link shared native API semantics to the API guide instead of maintaining two full contracts.
- Preserve the brief README invocation/trainer/support summaries and move the detailed output and movie examples to the guide.

## Issues Encountered

- The existing info-support check showed that the exact MMC3 support sentence is independently checked in README. It was retained there, and `libretro.info_support` passed.
- The runner stop self-test initially exposed CMake's semicolon list handling in one forced-termination sentence; the sentence is escaped as one check value, and the intended negative control passes.

## Deviations from Plan

None. No runtime code, public symbols, dependencies, release installation instructions, or test registrations for existing runtime contracts changed.

## User Setup Required

None.

## Next Plan Readiness

Plan 22-02 is complete. The next sequential plan is 22-03 (release and engineering/testing guide migration). READY-01 and READY-06 remain unmarked until Phase 22's complete scope is finished.

---
*Phase: 22-install-and-protect-progress*
*Completed: 2026-10-31*

## Self-Check: PASSED

- Summary file exists at the required phase path.
- Task commits `07e5bf4`, `09c0df6` and `8a01df7` are ancestors of HEAD.
