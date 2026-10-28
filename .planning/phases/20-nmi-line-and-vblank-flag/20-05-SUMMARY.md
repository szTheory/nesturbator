---
phase: 20-nmi-line-and-vblank-flag
plan: "05"
subsystem: testing
tags: [nmi, vblank, save-state, run-ahead, libretro, runner, ctest]
requires:
  - phase: 20-04
    provides: [serialized NMI capture/poll history and instruction-boundary replay oracles]
provides:
  - Public frame-boundary NMI fork and speculative replay evidence in the core.
  - Fresh-process runner save/load evidence with guest markers and same-suffix audio hashes.
  - Libretro rewind and run-ahead evidence at depths 1-3 with marker-based wrong-host controls.
affects: [20-06, KEEP-02, KEEP-03]
actuals:
  tokens: 12812.5
  tasks: 3
  commits: 3
plan_head_before: bb7a2e5c1f7eb49c81f3df1a7d3393acb3390a1e
plan_head_after: 28c6b21f5946140599474a69cc0f0558baa08971
commits: 3
tech-stack:
  added: []
  patterns:
    - Test-only NROMs are generated in scratch space and keep one-based guest NMI event sequences as independent oracles.
    - Host frame boundaries validate serialized frame/tick state while exact race cutpoints remain in internal tests.
    - Runner audio transitions and PCM compare the same resumed suffix against an uninterrupted reference.
key-files:
  created:
    - tests/runner/nmi_fixture.c
  modified:
    - tests/nmi_fixture.h
    - tests/core/test_state_fork.c
    - tests/core/test_state_runahead.c
    - tests/cmake/runner_state.cmake
    - tests/libretro/libretro_host.c
    - tests/runahead_model.h
    - tests/CMakeLists.txt
    - README.md
    - include/nesturbator.h
    - tests/accuracy/baseline-metadata.txt
    - tests/accuracy/measurements.md
key-decisions:
  - "Use a generated NROM that independently records one NMI event per frame, changes the visible backdrop, and produces non-silent pulse PCM."
  - "Keep public frame-boundary replay separate from Plan 20-04's exact instruction-boundary and modeled race evidence."
  - "Keep KEEP-02 and KEEP-03 pending: local host replay does not provide six-platform or later hosted evidence."
requirements-completed: []
requirements-addressed-but-pending: [KEEP-02, KEEP-03]
coverage:
  - id: D1
    description: "Core fork and run-ahead preserve independently expected NMI markers, native state, video, ticks, and non-silent PCM through depths 1-3."
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^(core\\.state_fork\\.nmi|core\\.state_runahead|core\\.nmi_state|accuracycoin\\.activation)$' --no-tests=error --output-on-failure"
        status: pass
    human_judgment: false
  - id: D2
    description: "Runner state files resume in fresh processes, remain idempotent, and expose stale or omitted restores through guest markers."
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^runner\\.state\\.nmi_(roundtrip|idempotent|wrong_load)$' --no-tests=error --output-on-failure"
        status: pass
    human_judgment: false
  - id: D3
    description: "Libretro serialization and rewind preserve NMI markers at all speculative depths and detect stale/omitted loads."
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^(libretro\\.host|libretro\\.serialize|libretro\\.runahead)$' --no-tests=error --output-on-failure"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci (683/683 tests)"
        status: pass
      - kind: integration
        ref: "ctest --preset asan -R '^libretro\\.(runahead|host)$' --no-tests=error --output-on-failure"
        status: pass
    human_judgment: false
  - id: D4
    description: "Later platform and hosted evidence remains outside this local replay plan."
    verification: []
    human_judgment: true
    rationale: "KEEP-02/KEEP-03 still require the later cross-platform and hosted evidence recorded for Plan 20-06."
duration: 32min
completed: 2026-10-26
status: complete
---

# Phase 20 Plan 05: NMI host replay evidence Summary

**Core, runner, and libretro host replays preserve per-frame NMI guest history and audiovisual state at saved boundaries.**

## Performance

- **Duration:** 31 minutes 54 seconds
- **Started:** 2026-10-27T00:39:01Z
- **Completed:** 2026-10-27T01:10:55Z
- **Tasks:** 3
- **Files modified:** 12

## Accomplishments

- Added an owned, generated NROM fixture with one NMI event per frame, a one-based CPU RAM event sequence, visible backdrop changes, and active pulse audio. Core fork and run-ahead tests cover frame 0, the first NMI frame, successive boundaries, independent instances, and speculative depths 1–3 with changed future inputs.
- Added fresh-process runner roundtrip, idempotence, and wrong-load cases. The helper verifies serialized guest markers through the public state API. Runner and direct-core audio transition/PCM hashes cover the same frames 2–4 suffix.
- Extended libretro run-ahead and rewind coverage at depths 1–3 in real-future-input and repeated-current-input modes. Real and speculative states check the expected guest marker sequence, native state, frame number, ticks, visible frame, and PCM; omitted-load and stale-load controls must trip the marker.
- Updated README, the public frame API comment, and AccuracyCoin measurement notes to distinguish host frame-boundary evidence from exact internal race tests. Refreshed only the measured AccuracyCoin build-input fingerprint; campaign pins, report, scoreboard, candidate provenance, and protected-main authority were preserved.

## Task Commits

1. **Task 1: Fork and speculatively replay one NMI-sensitive core frame** — `6fb55ca` (`test(20-05): add NMI core replay evidence`)
2. **Task 2: Round-trip the owned NMI program through runner processes** — `4a51502` (`test(20-05): verify NMI runner state continuation`)
3. **Task 3: Verify libretro rewind/run-ahead with NMI-sensitive markers** — `28c6b21` (`test(20-05): verify libretro NMI rewind paths`)

## Decisions Made

- Public host calls are frame-output intervals; NMI edge capture and instruction polling continue at hardware timing and are not defined by the host boundary.
- Runner audio comparison uses the identical continuation suffix, avoiding a whole-run versus resumed-tail comparison.
- KEEP-02 and KEEP-03 remain open for later cross-platform and hosted evidence; this plan records local host behavior only.

## Deviations from Plan

None. The AccuracyCoin input fingerprint update was required for the added CTest registration and did not change the measured campaign or scoreboard.

## Issues Encountered

- The first compile of the runner helper caught a misspelled public status constant. It was corrected before the runner tests passed; no production code change was needed.
- The workspace sandbox denied writes to `.git`; the plan's authorized scoped Git escalation enabled the ledger, staged task files, and commits. The preexisting `.planning/config.json` edit stayed unstaged.

## Verification

- `cmake --preset ci && cmake --build --preset ci && ctest --preset ci -R '^(core\\.state_fork\\.nmi|core\\.state_runahead|core\\.nmi_state|accuracycoin\\.activation)$' --no-tests=error --output-on-failure` — 5/5 passed, including the AccuracyCoin campaign fixture and activation check.
- `ctest --preset ci -R '^runner\\.state\\.nmi_(roundtrip|idempotent|wrong_load)$' --no-tests=error --output-on-failure` — all 3 runner NMI cases passed; the runner-save fixture also passed.
- `ctest --preset ci -R '^(libretro\\.host|libretro\\.serialize|libretro\\.runahead)$' --no-tests=error --output-on-failure` — all 3 named libretro cases passed; the runner-dump fixture also passed.
- `cmake --preset asan && cmake --build --preset asan && ctest --preset asan -R '^libretro\\.(runahead|host)$' --no-tests=error --output-on-failure` — both named ASan/UBSan cases passed; the runner-dump fixture also passed.
- `cmake --workflow --preset ci` — all 683/683 tests passed and the library, runner, and libretro archives were packaged.

## TDD Notes

The plan adds evidence for production NMI behavior already implemented and validated in Plans 20-03/20-04. No artificial production failure was introduced to manufacture RED evidence. The new regression assertions, existing core state matrix, runner processes, libretro module, and full CI/ASan suites all ran against the unchanged production implementation.

## User Setup Required

None — no external service configuration required.

## Next Phase Readiness

Plan 20-05's local core, runner, and libretro replay evidence is complete. Plan 20-06 can continue with its assigned final evidence scope. KEEP-02 and KEEP-03 remain pending; no hosted or six-platform completion is claimed.

---
*Phase: 20-nmi-line-and-vblank-flag*
*Completed: 2026-10-26*

## Self-Check: PASSED

- The summary file was created at the plan output path.
- Task commits `6fb55ca`, `4a51502`, and `28c6b21` are ancestors of the plan head recorded above.
- Stub scan of plan changes found no TODO, FIXME, placeholder, or unwired fixture entries.
- No new runtime endpoint, auth path, or trust-boundary access was introduced.
