---
phase: 23-reproduce-and-report-a-scene
plan: "02"
subsystem: testing
tags: [native-state, CMake, CTest, SHA-256, scratch-safety]

requires:
  - phase: 23-reproduce-and-report-a-scene
    provides: [Sanitized report route and bounded cold replay from plan 23-01]
provides:
  - Same-invocation native checkpoint identity preflight and independent unsupported-identity controls
  - Neutral NMI checkpoint continuation evidence with same-suffix audio comparison
  - Explicit replay scratch, deadline, interruption, container and runner-exit policies
affects: [23-03, reproduction, progress-safety, test-documentation]

actuals:
  tokens: 11792
  tasks: 2
  commits: 2

tech-stack:
  added: []
  patterns:
    - Example-local CMake SHA-256 preflight keeps declared identity policy separate from native loader checks
    - Documentation contracts compare CTest commands, fixture relationships and generated timeout properties

key-files:
  created: []
  modified:
    - docs/reproduction.md
    - docs/progress.md
    - docs/testing.md
    - tests/CMakeLists.txt
    - tests/cmake/reproduction.cmake
    - tests/cmake/reproduction_contract.cmake

key-decisions:
  - "Qualify only runner-native checkpoints created in the same invocation with recorded runner, ROM and state digests."
  - "Keep exact-build/content support stricter than native loading; the loader validates its state but does not enforce build or behavior-revision pairing."
  - "Keep each replay in a fresh scratch build, bound all selected setup/replay paths, and treat stops or incomplete output as failure."

patterns-established:
  - "CMake preflight accepts an explicit origin label and recorded digests, reports unsupported before replay, and never changes runner flags or state format."
  - "The reproduction contract rejects independent mutations and checks actual CTest timeout and fixture metadata."

requirements-completed: [REPRO-02, REPRO-03]
coverage:
  - id: D1
    description: "Runner-native checkpoint identity is checked before the separate neutral NMI continuation route, while existing guest-marker and stale/omitted-load controls remain authoritative."
    requirement: REPRO-02
    verification:
      - kind: integration
        ref: runner.reproduction.preflight
        status: pass
      - kind: integration
        ref: runner.reproduction.preflight_self_test
        status: pass
      - kind: integration
        ref: runner.state.nmi_roundtrip
        status: pass
      - kind: integration
        ref: runner.state.nmi_idempotent
        status: pass
      - kind: integration
        ref: runner.state.nmi_wrong_load
        status: pass
    human_judgment: false
  - id: D2
    description: "Public replay instructions keep user progress separate, declare finite frame/time bounds, reject incomplete runs, and remain aligned with generated CTest and fixture metadata."
    requirement: REPRO-03
    verification:
      - kind: unit
        ref: docs.reproduction
        status: pass
      - kind: unit
        ref: docs.reproduction.self_test
        status: pass
      - kind: unit
        ref: docs.links
        status: pass
      - kind: unit
        ref: docs.links.self_test
        status: pass
      - kind: unit
        ref: cmake.script_policy
        status: pass
      - kind: integration
        ref: "Fresh scratch source build and the documented 18-test CTest selection"
        status: pass
    human_judgment: false

duration: 15 min
completed: 2026-10-31
started: 2026-10-31T03:15:52Z
status: complete
commits: 2
plan_head_before: 7f6145679b387a647cbeba4eb0856563c4d0a20d
plan_head_after: 6e42c5af472297c6f013ab586283c7fedd02a0ee
---

# Phase 23 Plan 02: Native checkpoint preflight and bounded scratch Summary

**Native checkpoint replay now requires matching runner, ROM and state identities and a declared runner-native origin, with neutral continuation kept separate from movie replay.**

## Performance

- **Duration:** 15 minutes
- **Started:** 2026-10-31T03:15:52Z
- **Completed:** 2026-10-31T03:31:13Z
- **Tasks:** 2
- **Files modified:** 6

## Accomplishments

- Added the example-local `MODE=preflight` contract for 64-hex recorded runner, ROM and state identities plus the `runner-native-same-invocation` origin. Its positive fixture creates a fresh NMI ROM and a state at absolute frame 1, records digests at creation, and runs the same preflight function. Seven independent negative cases reject wrong digests, a missing identity, unknown/host origin and changed state bytes before replay.
- Documented the existing neutral-input NMI continuation at checkpoint K=1 through end frame N=4, comparing transition and PCM output for frames 2–4. Existing NMI event, idempotence, stale-load and omitted-load checks remain unchanged and passing.
- Expanded the reproduction guide with fresh-root storage, daily-battery separation, exact frame/process/CTest bounds, incomplete-run handling and a decision table that separates procedure-level unsupported outcomes from runner exits. The contract test now checks these claims and actual generated fixture/timeout metadata with independent scratch, container and loader-policy mutations.
- Added the reproduction-guide links to progress and testing documentation and bounded the existing save/movie fixture setup tests.

## Verification

Passed on the local macOS arm64 CI build:

```text
cmake --preset ci
cmake --build --preset ci --target nesturbator-run runner.game_movie runner.nmi_fixture runner.save_rom runner.state_mutate
ctest --test-dir build/ci --output-on-failure --no-tests=error -R '^(runner.reproduction.preflight|runner.reproduction.preflight_self_test|runner.state.nmi_roundtrip|runner.state.nmi_idempotent|runner.state.nmi_wrong_load|docs.reproduction|docs.reproduction.self_test|docs.links|docs.links.self_test|cmake.script_policy)$'
```

The focused selection passed all 11 selected tests, including the automatic save-fixture setup. The documented POSIX scratch route was also run from a new build directory with the complete target list; its cold replay, preflight, NMI, played-input, link and document checks passed all 18 tests. The public header confirms state format 3 and behavior revision 9. No full workflow, host/device, commercial-content, private-artifact or daily-progress operation ran.

## Task Commits

1. **Task 1: Stop unsupported checkpoint identities before replay and prove supported neutral continuation** — `b73c6b5` (`feat`)
2. **Task 2: Keep every public replay isolated, finite and honest about failure** — `6e42c5a` (`test`)

**Plan metadata:** recorded after summary and state updates.

## Files Created/Modified

- [docs/reproduction.md](../../../../docs/reproduction.md) — native checkpoint preflight, continuation route, scratch bounds and outcome table.
- [docs/progress.md](../../../../docs/progress.md) — link to isolated scene reproduction and the independent battery-backup policy.
- [docs/testing.md](../../../../docs/testing.md) — link to the neutral fixture reproduction route.
- [tests/CMakeLists.txt](../../../../tests/CMakeLists.txt) — preflight registrations and finite fixture setup deadlines.
- [tests/cmake/reproduction.cmake](../../../../tests/cmake/reproduction.cmake) — digest preflight, supported fixture creation, negative controls and stop-line rejection.
- [tests/cmake/reproduction_contract.cmake](../../../../tests/cmake/reproduction_contract.cmake) — guide, scratch, container, CTest metadata and mutation checks.

## Decisions Made

- A digest and origin label qualify only the recorded runner-native route; they do not authenticate hostile files or convert host containers.
- Native loader format/cartridge validation remains separate from exact-build and behavior-revision support. The loader does not enforce that pairing.
- The NMI checkpoint route is neutral and independent from the movie route. Absolute end-frame numbering and audio suffix boundaries stay explicit.
- Every example uses its own disposable build root; incomplete or interrupted output is never counted as a matching replay.

## Deviations from Plan

None — plan executed as written.

## Issues Encountered

- The workspace sandbox rejected Git index and `.git` ledger writes. The authorized narrow Git-metadata escalation succeeded; both task commits ran the repository hooks. The owner’s `AGENTS.md` and `.planning/config.json` edits were not staged or changed.
- Early document-contract runs caught source-line-wrapping assumptions in assertions. The checks were adjusted to stable phrases and then passed, including all independent negative mutations.

## Preserved Flags and Boundaries

This plan adds scoped executable evidence only. The five unresolved heuristic fallback edges and descriptor-less prohibitions remain unverified; exact commercial-scene access, host presentation and hardware correctness are not established. No user progress, live host/device operation, campaign budget, collector debt or private artifact boundary changed.

## User Setup Required

None.

## Next Phase Readiness

Plan 23-03 is ready to execute. No user action is needed; the next step is `$gsd-execute-phase 23`, which continues with plan 23-03. Context can be cleared after reading the updated STATE.md.

## Self-Check: PASSED

- Summary and all six modified plan files exist.
- Task commits `b73c6b5` and `6e42c5a` are ancestors of the current phase branch HEAD.
- The measured plan commit count is 2, matching the task commits.
