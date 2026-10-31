---
phase: 23-reproduce-and-report-a-scene
plan: "03"
subsystem: testing
tags: [scene-reproduction, CMake, CTest, evidence-limits, documentation]

requires:
  - phase: 23-reproduce-and-report-a-scene
    provides: [Bounded cold replay, native checkpoint route and safety contracts from plans 23-01 and 23-02]
provides:
  - Layer-specific next-evidence guide with explicit native, adapter, host and correctness limits
  - Independent document mutations guarding against false evidence claims
  - Sanitized plan evidence with inherited and current-run results distinguished
affects: [phase-23-review, reproduction, test-documentation]

actuals:
  tokens: 6313
  tasks: 2
  commits: 2

tech-stack:
  added: []
  patterns:
    - Evidence decisions name supported inference, remaining unknown and the next bounded source
    - Scoped CMake document mutations guard claim boundaries while retaining prior controls

key-files:
  created:
    - .planning/phases/23-reproduce-and-report-a-scene/23-EXECUTION-EVIDENCE.md
  modified:
    - docs/reproduction.md
    - docs/runner.md
    - tests/cmake/reproduction_contract.cmake

key-decisions:
  - "Route each observation to the least costly evidence source that can answer its claim; matching hashes establish repeatability, not independent correctness."
  - "Keep commercial reports unclassified and retain judgment-only safety findings as flagged-unverified."

patterns-established:
  - "Evidence tables distinguish scene access, native output, adapter conversion, host presentation and independent oracles."
  - "Negative document mutations exercise false correctness, screenshot, host-pacing and commercial-status claims."

requirements-completed: [REPRO-01, REPRO-02, REPRO-03, REPRO-04]
coverage:
  - id: D1
    description: "The canonical guide selects a bounded next evidence source and states the limits of native, adapter, host and original-game comparisons."
    requirement: REPRO-04
    verification:
      - kind: unit
        ref: docs.reproduction
        status: pass
      - kind: unit
        ref: docs.reproduction.self_test (13 independent mutations)
        status: pass
      - kind: unit
        ref: docs.links and docs.links.self_test
        status: pass
    human_judgment: true
    rationale: "CTest guards key distinctions, while selecting an independent oracle and assessing commercial observations still requires source review."
  - id: D2
    description: "The execution evidence separates current local checks from inherited results and records unsupported, unavailable and untried routes without private artifacts."
    requirement: REPRO-01
    verification:
      - kind: integration
        ref: "Focused 10-test reproduction, preflight, NMI and document selection"
        status: pass
      - kind: integration
        ref: "Focused 18-test inherited document, runner, status, release and package selection"
        status: pass
      - kind: other
        ref: "Sanitized identity, evidence-window and platform-scope source review"
        status: pass
    human_judgment: true
    rationale: "The record distinguishes what was run from what was inherited, but a source review is needed to assess the judgment-only exclusions."

duration: 13min
completed: 2026-10-31
started: 2026-10-31T03:32:05Z
status: complete
commits: 2
plan_head_before: 7c71b25b2d9ed9e2b8e8198819a7abd336b7a5e9
plan_head_after: 36ac4302481be224683938aa7b555eb1ae885957
---

# Phase 23 Plan 03: Layer-specific scene triage Summary

**The reproduction guide now maps each observation to its next bounded evidence source and separates repeatability from adapter, host and hardware correctness claims.**

## Performance

- **Duration:** 13 minutes
- **Started:** 2026-10-31T03:32:05Z
- **Completed:** 2026-10-31T03:45:30Z
- **Tasks:** 2
- **Files modified:** 4 implementation/evidence files

## Accomplishments

- Added a decision table and evidence limits to [the reproduction guide](../../../../docs/reproduction.md). It separates title-level observations, repeated native windows, RGB conversion and test-host callbacks, frontend presentation, and independent correctness oracles. README, testing and progress links were already direct; the runner guide now links to the canonical guide.
- Extended the scoped reproduction contract with five new independent false-claim mutations, preserving the original eight controls. It also validates the four direct guide links and the generated 60-second CTest limits.
- Recorded permitted fixture/build digests, inherited cold frame/audio hashes, current checkpoint controls, exact focused commands, platform limits, helper-gap rationale and unresolved decisions in [the sanitized evidence record](23-EXECUTION-EVIDENCE.md). No emulator runtime source or public API changed.

## Verification

Passed on the local macOS arm64 CI build:

```text
cmake --preset ci
cmake --build --preset ci --target nesturbator-run
ctest --test-dir build/ci --output-on-failure --no-tests=error -R '^(docs.reproduction|docs.reproduction.self_test|docs.links|docs.links.self_test)$'
ctest --test-dir build/ci --output-on-failure --no-tests=error -R '^(runner.reproduction.cold|runner.reproduction.compare_self_test|runner.reproduction.preflight|runner.reproduction.preflight_self_test|runner.state.nmi_roundtrip|runner.state.nmi_idempotent|runner.state.nmi_wrong_load|docs.reproduction|docs.reproduction.self_test)$'
ctest --test-dir build/ci --output-on-failure --no-tests=error -R '^(state.status_table|state.status_table.self_test|runner.options.readme|runner.options.canonical.self_test|runner.exit.table|runner.exit.help|runner.exit.canonical.self_test|docs.links|docs.links.self_test|release.one_version_per_line|release.one_version_per_line.self_test|release.version_format|libretro.info_claims|libretro.info_claims.self_test|libretro.info_support|libretro.info_staged|libretro.info_archives.self_test|cmake.script_policy)$'
```

The focused guide/link selection passed 4/4; the replay, preflight and NMI selection passed 10/10 including fixture setup; the inherited-contract selection passed 18/18. `docs.reproduction.self_test` rejected all 13 mutations. The scratch source-build route and 18-test result are inherited from Plan 23-02, and the cold hashes are inherited from Plan 23-01; neither is presented as newly measured here. No full workflow or live host/device test ran.

## Task Commits

1. **Task 1: Route each scene report to the next appropriate evidence layer** — `c865b79` (docs)
2. **Task 2: Verify complete guide reachability and preserve sanitized evidence limits** — `36ac430` (test)

**Plan metadata:** committed after summary and state updates.

## Files Created/Modified

- [docs/reproduction.md](../../../../docs/reproduction.md) — layered next-evidence table and explicit evidence limits.
- [docs/runner.md](../../../../docs/runner.md) — direct link to the canonical scene reproduction guide.
- [tests/cmake/reproduction_contract.cmake](../../../../tests/cmake/reproduction_contract.cmake) — scoped evidence, link, timeout and false-claim controls.
- [23-EXECUTION-EVIDENCE.md](23-EXECUTION-EVIDENCE.md) — sanitized identity, commands, results, inherited evidence and limits.

## Decisions Made

- Matching native hashes are repeatability evidence only. Correctness claims need a separate, claim-specific oracle.
- Native conversion and test-host callbacks do not establish frontend input, physical-device behavior or host presentation.
- Historical commercial observations remain unclassified, and the five unresolved fallback edges and eight judgment-only prohibitions remain visible.

## Deviations from Plan

None — plan executed as written.

## Issues Encountered

- Early contract runs exposed three checker-authoring issues: one assertion crossed a source line break, one searched the wrong guide section, and one diagnostic exceeded the stable text expected by CMake. The assertions were narrowed or shortened; the final focused selections passed.
- The initial staged whitespace check found trailing whitespace on the evidence date line. It was removed before the atomic Task 2 commit; repository hooks passed.

## User Setup Required

None. No user action is needed for plan execution.

## Preserved Flags and Boundaries

The five unresolved fallback edges remain: REPRO-01 idempotency and concurrency, and unclassified REPRO-02, REPRO-03 and REPRO-04. Eight descriptor-less prohibitions remain flagged-unverified because their verification requires judgment. Linux and Windows were unavailable; no live host, device, commercial replay, private capture or campaign ran. Owner `AGENTS.md` and `.planning/config.json` edits remain unstaged and untouched.

## Next Phase Readiness

Plan 23-03 artifacts are ready for the orchestrator's full regression and phase review. This summary completes the plan, not Phase 23; no lifecycle transition or live operation was started. The next actor is the orchestrator, which should run `$gsd-verify-work 23` to review phase acceptance against the collected evidence. Context can be cleared before that command after reading the updated STATE.md.

---
*Phase: 23-reproduce-and-report-a-scene*
*Completed: 2026-10-31*

## Self-Check: PASSED

- Summary and execution evidence files exist.
- Task commits `c865b79` and `36ac430` are ancestors of the current phase branch HEAD.
- The measured plan commit count is 2, matching the two task commits.
