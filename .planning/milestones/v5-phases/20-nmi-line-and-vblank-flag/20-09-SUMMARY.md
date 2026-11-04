---
phase: 20-nmi-line-and-vblank-flag
plan: "09"
subsystem: testing
tags: [cmake, github-actions, accuracycoin, provenance]
requires:
  - phase: 20-08
    provides: [six-platform hash validator, protected-main PASS baseline, measured campaign provenance]
provides:
  - Read-only source-bound collector for retained CI, performance, host, and Nightly evidence
  - Bounded offline controls through the same validator used by live collection
  - CI retention for executed LastTest logs, cold protected-main data, and raw performance measurements
  - Refreshed local campaign provenance and concrete pending-delivery instructions
affects: [20-10, Phase 20 verification, AccuracyCoin]
actuals:
  tokens: 27861
  tasks: 2
  commits: 3
plan_head_before: 5006e66ecf462a258889cd1368da3b9b7eb59c86
plan_head_after: 392b8f5995e4992dcc55f818d6e6b25d8ae9b9df
tech-stack:
  added: []
  patterns:
    - One content validator handles offline fixtures and read-only hosted collection
    - Hosted source identity separates candidate head, tested merge commit, protected base, and full effective-input digest
key-files:
  created:
    - tests/cmake/phase20_hosted_evidence.cmake
    - tests/cmake/phase20_hosted_evidence_controls.cmake
  modified:
    - tests/CMakeLists.txt
    - .github/workflows/ci.yml
    - README.md
    - tests/accuracy/baseline-metadata.txt
    - tests/accuracy/measurements.md
    - .planning/phases/20-nmi-line-and-vblank-flag/20-EVIDENCE.md
key-decisions:
  - "Offline fixtures retain fixture provenance and can never satisfy hosted acceptance."
  - "Derive the tested pull-request merge SHA and protected base from GitHub metadata; do not claim the candidate head is the checked-out SHA."
  - "Keep all Phase 20 requirements and Plan 20-10 pending until actual hosted results pass."
patterns-established:
  - "Hash all effective source paths and contents independently of the narrower AccuracyCoin build-input fingerprint."
  - "Re-evaluate downloaded raw performance samples with the existing +1% comparator and verify retained evidence contents rather than job labels alone."
requirements-completed: []
requirements-pending: [NMI-01, NMI-02, KEEP-02, KEEP-03]
coverage:
  - id: D1
    description: "A shared read-only collector validates source identity and retained CI, performance, host, and Nightly evidence; offline fixtures exercise named rejection paths."
    verification:
      - kind: integration
        ref: "phase20.hosted_evidence.controls; 26-test hosted/hash/performance/no-skip gate"
        status: pass
    human_judgment: false
  - id: D2
    description: "Existing workflows retain executed test logs, cold protected-main authority, and raw performance evidence with their existing jobs and measurement rules."
    verification:
      - kind: integration
        ref: "hygiene.ci_required_rollup; hygiene.nightly_workflow_policy; hygiene.nightly_workflow_policy.selftest"
        status: pass
    human_judgment: false
  - id: D3
    description: "Local campaign provenance and the pending delivery contract are recorded without claiming hosted completion."
    verification:
      - kind: integration
        ref: "AccuracyCoin campaign, exact-six/protected/hash focused checks, and cmake --workflow --preset ci"
        status: pass
    human_judgment: false
duration: 25min
completed: 2026-10-27
status: complete
commits: 3
---

# Phase 20 Plan 09: Hosted Evidence Preparation Summary

**A read-only collector now binds retained CI and Nightly evidence to the tested source tree, with local provenance refreshed and hosted delivery still pending.**

## Performance

- **Duration:** about 25 minutes
- **Started:** 2026-10-27T04:35:00Z
- **Completed:** 2026-10-27T05:00:00Z
- **Tasks:** 2
- **Files modified:** 8

## Accomplishments

- Added a read-only collector that checks retained artifact contents, CI/Nightly run and attempt identity, candidate-versus-merge SHA/base mapping, source-tree digest, six NMI targets, protected PASS keys, native hash inventories, raw performance data, RetroArch marker evidence, and Nightly results.
- Added bounded offline positive and named negative controls through the same content validator. Fixture provenance cannot be promoted to hosted success.
- Extended existing workflow uploads to retain each build lane's executed `LastTest.log` and cold protected-main snapshot, and the performance job's raw base/head samples, summary, and identity for 60 days. Existing jobs, pins, timeouts, campaign placement, caps, and the +1% policy remain intact.
- Documented the `delivery.json` contract and collector commands, and refreshed the local AccuracyCoin metadata and evidence notes from one measured full campaign.

## Task Commits

1. **Task 1 RED: Add fail-first hosted evidence acceptance control** — `d1b2ba7` (`test`)
2. **Task 1 GREEN: Validate retained hosted evidence content** — `f85e1b1` (`feat`)
3. **Task 2: Record hosted evidence delivery preparation** — `392b8f5` (`docs`)

The plan commit ledger records three task commits from `5006e66ecf462a258889cd1368da3b9b7eb59c86` through `392b8f5995e4992dcc55f818d6e6b25d8ae9b9df`. The latter is the measured local source/documentation commit; the eventual hosted candidate head remains unresolved until the separately authorized delivery step.

## TDD Gate Compliance

- **RED:** `phase20.hosted_evidence.controls` executed and failed its planned positive-fixture assertion because the shared validator stub reported `incomplete: hosted evidence validation is not implemented`. The actual JUnit report recorded one test and one failure at `valid fixture must pass the content validator`; this was the intended assertion, with no setup, syntax, or discovery failure. The recorded RED evidence passed `gsd_run check tdd-red-evidence` with `RED_EVIDENCE_OK`.
- **GREEN:** Implemented the shared content validator and read-only GitHub collector, then reran the Task 1 gate after commit: 26/26 tests passed, including all 23 performance-gate self-tests, `hash.inventory.self_test`, `policy.no-skip`, and the hosted-evidence controls.
- **REFACTOR:** No separate refactor was needed. The implementation uses existing CMake, GitHub CLI, hash inventory, and performance comparison tools without adding dependencies.

## Verification

- Workflow policy checks: 3/3 passed (`hygiene.ci_required_rollup`, `hygiene.nightly_workflow_policy`, and its self-test).
- One full-enabled AccuracyCoin campaign: 1/1 passed; all 22 pages and 146 scored rows completed (114 pass, 32 fail, 0 skip, 0 not-run, 0 timeout, 5 DRAW).
- Focused local acceptance after refreshing metadata and disabling the full campaign: 9/9 passed, including the exact six NMI target keys, cold protected-main observation, all 108 original protected PASS keys, and unchanged native hash inventory/content.
- Final `cmake --workflow --preset ci`: 684/684 passed in 371.53 seconds and generated the library, runner, and libretro packages.

## Source and Campaign Identity

- Campaign source revision: `f85e1b15f176`; `source_dirty=1` truthfully records the local README update and preserved `.planning/config.json` change.
- Campaign build-input fingerprint: `ebdaef6447e2ad737de5367bc85eec6fca365d8070953a3b58663c6d5262d864` over 56 inputs.
- Campaign report SHA-256: `29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`.
- Measured `host_elapsed_guest_ns`: `5958989000`.
- Effective source digest after commit `392b8f5995e4992dcc55f818d6e6b25d8ae9b9df`: `684085be7b2ecccc55f30eb885387a111aecfb622950da3184a84bce1d489c43` over 415 files. It covers the complete Git-index plus nonignored working-tree file set, hashes file contents and symlink targets, and excludes only `.planning/**` and `tests/accuracy/measurements.md`. The separately retained full source manifest is `build/phase20-effective-source.tsv`.
- The six target rows, 108-key protected comparison, and native hashes were checked locally; the local campaign is not a clean-source or hosted result.

The collector's hosted `delivery.json` fields are `repository`, `ci_run_id`, `ci_run_attempt`, `nightly_run_id`, `nightly_run_attempt`, `candidate_head_sha`, and `pr_number`. `MODE=collect` and `MODE=validate` use `ROOT` under `build/` or `/tmp` and `SOURCE_REF` for the current checkout. With no authorized real run IDs, collection reports `pending_delivery` and writes no hosted-pass manifest. No push, PR, workflow dispatch, or merge occurred. Actual collection belongs to Plan 20-10.

## Decisions Made

- Hosted evidence must map the candidate head and protected base to the actual synthetic merge checkout; the collector validates full source content instead of relying only on the narrower campaign fingerprint.
- The local full campaign, synthetic controls, and CI workflow do not close hosted six-platform, performance, host, Nightly, or physical exact-tie claims.
- Keep `NMI-01`, `NMI-02`, `KEEP-02`, and `KEEP-03` pending for independent Phase 20 verification.

## Deviations from Plan

None — both tasks followed the plan. No auth gate occurred; no hosted delivery was attempted.

## Next Phase Readiness

Plan 20-10 can begin after a separately authorized hosted delivery provides actual CI/Nightly run IDs and attempts. It must resolve the delivered candidate head after planning-only metadata commits and compare its full effective-source digest with these measured local inputs. Keep Phase 20 and all four requirements open; physical exact-tie precedence, descriptor-less clean-room verification, unclassified NMI assumptions, and Nightly fetch debt remain unresolved.

## Self-Check: PASSED

The summary, all eight planned implementation/evidence files, and task commits `d1b2ba7`, `f85e1b1`, and `392b8f5` are present on the phase branch.
