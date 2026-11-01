---
phase: 24-correct-remaining-ntsc-dma-behavior
plan: "04"
subsystem: testing
tags: [c, cmake, accuracycoin, ntsc-dma]
requires:
  - phase: 24-correct-remaining-ntsc-dma-behavior
    provides: behavior14 DMA corrections, owned wrong-model regressions, and native continuation evidence
provides:
  - Complete local full-CI and behavior14 campaign result
  - Evidence-derived scoreboard and measurement metadata
  - Explicit handoff for still-missing hosted acceptance evidence
affects: [phase-24-verification, hosted-ci]
actuals:
  tokens: 4200
  tasks: 3
  commits: 1
tech-stack:
  added: []
  patterns: [record complete local evidence separately from hosted qualification]
key-files:
  created:
    - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-CI-RUN-20261101.md
    - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-04-SUMMARY.md
  modified:
    - .planning/STATE.md
    - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-ACCEPTANCE.md
    - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-DMA-EVIDENCE.md
    - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-04-PARTIAL.md
    - .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-EXECUTION-RESUME.md
    - tests/accuracy/baseline-metadata.txt
    - tests/accuracy/measurements.md
key-decisions:
  - "Local CMake approval is standing and requires no per-run prompt; remote hosted triggers and publication remain separate."
  - "The five selected rows pass locally, but phase requirements stay open until cold protected-main, hosted platform/hash, and performance evidence exists."
patterns-established:
  - "Keep full raw campaign evidence under ignored build output and link a concise, source-identified result from planning records."
requirements-completed: []
duration: 12min
completed: 2026-11-01
status: complete
---

# Plan 24-04: Local behavior14 acceptance evidence

**The full local workflow passed all707 tests, packaged all three deliverables,
and passed each of the five selected behavior14 DMA ROM rows.**

## Performance

- **Duration:** Final CTest run took517.82 seconds; the whole workflow completed within the existing39-minute ceiling.
- **Completed:** 2026-11-01
- **Tasks:** 3 plan tasks executed through the local acceptance boundary
- **Files modified:** Acceptance, evidence, measurement, metadata, state, and resume records

## Accomplishments

- Recorded the exact full-CI command and campaign provenance in
  [24-CI-RUN-20261101.md](24-CI-RUN-20261101.md).
- Updated campaign metadata from the actual report: 126 PASS / 20 FAIL, all146
  scored rows terminal, no skip/not-run/timeout, and five inventory-only DRAW
  rows. All five selected DMA rows pass; all119 prior current-pin protected
  PASS keys remain passing.
- Updated acceptance and state records to distinguish completed local evidence
  from missing hosted acceptance. Local protected observation used the
  `scoreboard-main` fallback, so independent cold-main authority is absent.

## Commits

- The behavior14 source/data correction is recorded in `0c8cc53`.
- The acceptance evidence and state reconciliation are committed with this
  summary; no unrelated working-tree files are included.

## Decisions

- Standing local CMake authority requires no per-run approval and retains the
  established limits/no-retry policy.
- Current-head hosted lanes, independent cold protected-main evidence,
  six-platform frame/PCM/transition hashes, and unchanged Cachegrind `+1%`
  evidence remain required. Phase24 and DMA-01 through DMA-04 stay open.

## Issues and remaining work

The full workflow passed, but the campaign still reports20 unrelated failing
rows. No hosted CI, remote trigger, push, PR, merge, or release ran. Inherited
branch-wide planning whitespace remains and spans archived records outside
this plan's narrow evidence update.

## Next action

The owner acts next only if hosted PR/checks are to be triggered. The exact
next OpenGSD command after authorization is `$gsd-ship`; it prepares a PR and
the hosted review/check path. Context may be cleared after reading STATE and
[24-EXECUTION-RESUME.md](24-EXECUTION-RESUME.md).

---
*Plan: 24-04*
*Completed: 2026-11-01*
