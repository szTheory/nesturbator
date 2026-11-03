---
phase: 23-reproduce-and-report-a-scene
plan: "01"
subsystem: testing
tags: [scene-report, CMake, CTest, NMOVIE1]

requires:
  - phase: 22-install-and-protect-progress
    provides: Core/build identity guidance and separate battery-save and checkpoint policy
provides:
  - Sanitized scene-report form linked from README troubleshooting
  - Bounded two-run cold movie replay comparing native frames and both audio hashes
  - Positive and negative output/report contract tests
affects: [23-02, 23-03, scene-reporting, reproduction]

actuals:
  tokens: 9579
  tasks: 2
  commits: 2

tech-stack:
  added: []
  patterns:
    - Reuse one strict CMake parser/comparator for real CLI output and synthetic negative controls
    - Validate documented build targets and test names against generated CTest command metadata

key-files:
  created:
    - docs/reproduction.md
    - tests/cmake/reproduction.cmake
    - tests/cmake/reproduction_contract.cmake
  modified:
    - README.md
    - tests/CMakeLists.txt

key-decisions:
  - Keep one sanitized scene-report guide and leave raw private artifacts local.
  - Treat frame, transition and PCM hash equality as repeatability evidence only; retain gameplay controls independently.

patterns-established:
  - CTest replay orchestration creates a fresh build-tree scratch directory and bounds every child process.
  - Synthetic comparator controls validate rejection behavior without claiming emulator results.

requirements-completed: [REPRO-01, REPRO-02, REPRO-03]
coverage:
  - id: D1
    description: README troubleshooting reaches a copyable report with explicit uncertainty, provenance and privacy limits.
    requirement: REPRO-01
    verification:
      - kind: unit
        ref: docs.reproduction
        status: pass
      - kind: unit
        ref: docs.links
        status: pass
    human_judgment: false
  - id: D2
    description: Public source-build facilities repeat a bounded 600-frame cold movie and compare native frame, transition and PCM output.
    requirement: REPRO-02
    verification:
      - kind: integration
        ref: runner.reproduction.cold
        status: pass
      - kind: integration
        ref: runner.boot_to_play
        status: pass
    human_judgment: false
  - id: D3
    description: Synthetic and document mutations reject changed or incomplete output and stale report commands.
    requirement: REPRO-03
    verification:
      - kind: unit
        ref: runner.reproduction.compare_self_test
        status: pass
      - kind: unit
        ref: docs.reproduction.self_test
        status: pass
    human_judgment: false

duration: 12 min
started: 2026-10-31T03:01:14Z
completed: 2026-10-31
status: complete
commits: 2
plan_head_before: d574549572897e3d1647c56d99cd0eaf60ac9745
plan_head_after: d3e2dba87ff2b4ac56d553447bda342d16eafdb8
---

# Phase 23 Plan 01: Scene report and cold replay Summary

**Sanitized scene reports now lead from README troubleshooting to a bounded two-run NES movie replay with complete frame and audio comparisons.**

## Performance

- **Duration:** 12 minutes
- **Started:** 2026-10-31T03:01:14Z
- **Completed:** 2026-10-31T03:12:23Z
- **Tasks:** 2
- **Files modified:** 5

## Accomplishments

- Added a player report form, permitted-fixture example, privacy limits and source-build instructions in [docs/reproduction.md](../../../../docs/reproduction.md); README troubleshooting links directly to it.
- Added `runner.reproduction.cold`, which creates a fresh scratch root, generates the existing 600-frame `nesrunner-pos` NMOVIE1 movie, and runs the public CLI twice. The comparator requires frames 1–600 in order and compares the full tick/hash records plus separate transition and PCM hashes.
- Added output and document controls that reject changed frame/audio values, missing audio, duplicate/reordered/truncated frames, missing report fields, stale CTest names or build targets, and a removed privacy claim.
- Kept `runner.boot_to_play` as the separate game-behavior oracle for its positive run and none, attract-only and Start-only controls.

## Helper-Gap Evidence

The generated CTest graph had no route that executed the public runner movie twice, checked all 600 frame records and both audio hashes, and imposed an outer deadline. `runner.dump.nesrunner.play` ran one movie and dumped frame 600; `runner.boot_to_play` checked gameplay through the library path and retained input controls. The new wrapper supplies only the missing bounded CLI repeatability comparison.

## Measured Fixture Evidence

The permitted `tests/roms/runner.nes` bytes matched manifest SHA-256 `1a67bfabdde2d96eacb04eb7f9d17e8da6c082d4976c318315fe31e2d19b4384`. The local native runner reported `nesturbator-run 0.1.11`; its core source revision was `d574549`. The test used the generator-owned Start input at zero-based movie indexes 60–63, port 1 neutral, and generator-owned jump spans on port 0. Existing `runner.boot_to_play` passed its scoring-play predicate and independent controls.

Two 600-frame cold runs matched on the local macOS arm64 CI build:

| Representation | Identity |
| --- | --- |
| Ordered native frame/tick/hash records, frames 1–600 | `a90cc60caeea1fc4e4f159e3341b5f1b446022281c018b6d609eebf702175d9b` |
| Audio transitions | `33e4bce709976dca002689e4cc164f694a677d9949fdfa325b7dd6cfa5cc4006` |
| Audio PCM | `255636674903d03dd9cb7ca632246c3375dff8c88300155b052ac5c273d4dcca` |

These establish repeatability on this local build and platform. They do not establish host presentation or hardware correctness. Scratch movies and raw runner output stayed under ignored build/temp storage; no ROM, movie, save or checkpoint bytes were added to Git.

## Verification

All commands completed successfully:

```text
cmake --preset ci
cmake --build --preset ci --target nesturbator-run runner.game_movie runner.boot_to_play
ctest --test-dir build/ci --output-on-failure --no-tests=error -R '^(runner.reproduction.cold|runner.boot_to_play|docs.links|docs.links.self_test|cmake.script_policy)$'
ctest --test-dir build/ci --output-on-failure --no-tests=error -R '^(runner.reproduction.cold|runner.reproduction.compare_self_test|docs.reproduction|docs.reproduction.self_test|docs.links|docs.links.self_test|cmake.script_policy)$'
```

The second focused selection passed all seven tests. The published POSIX scratch-build commands were also run from a newly created `$scratch` directory, without reusing `build/ci`:

```text
scratch="$(mktemp -d "${TMPDIR:-/tmp}/nesturbator-reproduction.XXXXXX")"
cmake --preset ci -B "$scratch/build"
cmake --build "$scratch/build" --target nesturbator-run runner.game_movie runner.boot_to_play
ctest --test-dir "$scratch/build" --output-on-failure --no-tests=error -R '^(runner.reproduction.cold|runner.reproduction.compare_self_test|runner.boot_to_play|docs.links|docs.links.self_test|docs.reproduction|docs.reproduction.self_test|cmake.script_policy)$'
```

That clean source-build route passed all 12 selected CTest cases. The documented PowerShell variant was checked by the contract test but was not executed on this machine. No full workflow, host, device or private-content run was part of this plan.

## Task Commits

1. **Task 1: Reach a completed public scene report and repeat one bounded cold movie end to end** — `410360f` (`feat`)
2. **Task 2: Reject incomplete reports and false output comparisons through targeted controls** — `d3e2dba` (`test`)

**Plan metadata commit:** recorded after summary and state updates.

## Decisions Made

- Keep one report route with explicit unknown values and private-artifact limits; leave raw logs and captures local.
- Treat repeated frame and audio hashes as repeatability evidence, and keep gameplay sensitivity controls independent.

## Deviations from Plan

None in product scope. The sandbox denied writing the commit-count ledger inside `.git`; the base hash was persisted in ignored `build/ci` storage and the commit count was measured from that file.

## Preserved Assumption and Prohibition Flags

The five heuristic fallback edges remain unresolved with no verification claim: REPRO-01 idempotency and concurrency; REPRO-02 unclassified; REPRO-03 unclassified; and REPRO-04 unclassified. This plan does not add a deduplication guarantee or classify those edges.

The plan's four prohibition dispositions remain judgment-based and are not mechanically established by these tests:

- Do not publish commercial media, saves, checkpoints, raw private logs/captures or personal identifiers as report evidence.
- Do not imply a scene was reached or an identity is known when access or identity is unavailable.
- Do not represent movie-plus-load-state, host recording or foreign movie conversion as supported runner capabilities.
- Do not label repeated frame/audio hashes as independent hardware correctness.

The report and contract tests preserve these limits in documentation; exact commercial-scene access and identity remain unknown.

## Issues Encountered

- The initial summary-ledger write under `.git` was denied by the sandbox's read-only Git metadata boundary. The ignored build-tree ledger provided the same persisted base for measuring the two plan commits.
- CMake wraps long fatal diagnostics, so negative controls match short, specific diagnostic phrases. All intended negative cases pass.

## User Setup Required

None.

## Next Phase Readiness

Plan 23-02 can extend the canonical guide with native checkpoint preflight and scratch safety. No user action is needed. The live host/device/private-content boundary and all prior safety stops remain unchanged.

## Self-Check: PASSED

- Summary file exists at the required phase path.
- Task commits `410360f` and `d3e2dba` are ancestors of the current phase branch HEAD.

---
*Phase: 23-reproduce-and-report-a-scene*
*Plan: 01*
