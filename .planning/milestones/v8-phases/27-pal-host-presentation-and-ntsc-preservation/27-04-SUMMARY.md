---
phase: 27-pal-host-presentation-and-ntsc-preservation
plan: "04"
subsystem: host-presentation
tags: [pal, libretro, runner, acceptance, ntsc]
dependency_graph:
  requires: [27-03]
  provides: [six-mapper-presentation-matrix, final-source-bound-acceptance, ntsc-preservation-receipt]
  affects: [phase-27-verification, v8-closeout]
tech_stack:
  added: []
  patterns: [cross-deliverable-owned-fixture-parity, exact-candidate-campaign-receipt]
key_files:
  created:
    - .planning/phases/27-pal-host-presentation-and-ntsc-preservation/27-ACCEPTANCE.md
    - .planning/phases/27-pal-host-presentation-and-ntsc-preservation/27-REVIEW.md
    - .planning/phases/27-pal-host-presentation-and-ntsc-preservation/27-REVIEW-DISPOSITION.md
  modified:
    - tests/libretro/libretro_host.c
    - tests/host/test_convert.c
    - tests/CMakeLists.txt
    - docs/testing.md
    - tests/cmake/accuracycoin_activation.cmake
    - tests/accuracy/baseline-metadata.txt
    - tests/accuracy/measurements.md
decisions:
  - Keep the six-mapper PAL/NTSC matrix inside the existing real module/runner test and compare callback RGB through independent native-index lookups.
  - Bind final AccuracyCoin provenance only after the behavior-17 activation assertion and all 56 fingerprinted inputs were frozen.
  - Reuse the successful full workflow as the regression-gate superset; do not run a second identical 722-test workflow.
  - Preserve the documented pinned-data palette-generator limitation and all existing hardware, host-pacing, and stopped-campaign limits.
requirements_completed: [VID-01, AV-01, NTSC-01]
coverage:
  - id: six-mapper-regional-presentation
    description: Library, runner, and real libretro module agree on PAL/NTSC region, exact rates, native frame/audio hashes, and modeled RGB for all six mapper families.
    requirement: AV-01
    verification:
      - kind: integration
        ref: "libretro.host#six-mapper PAL/NTSC matrix and real callback parity"
        status: pass
      - kind: command
        ref: "cmake --workflow --preset ci"
        status: pass
    human_judgment: false
  - id: modeled-pal-display
    description: Real PAL table values, emphasis conversion, and pitched host output match the named model across the existing API boundaries.
    requirement: VID-01
    verification:
      - kind: unit
        ref: "core.palette, palette.regen.pal, host.convert"
        status: pass
      - kind: integration
        ref: "libretro.host#independent callback pixel indexing and runner PPM parity"
        status: pass
    human_judgment: false
  - id: unchanged-ntsc-outputs
    description: Existing NTSC table, frame/audio inventory, callbacks, and canonical AccuracyCoin expectations remain intact.
    requirement: NTSC-01
    verification:
      - kind: command
        ref: "git diff --exit-code ad24e05247f4ebe5cb7efb8212b632c2f8cf4f8f -- src/palette_ntsc.c tests/runner/hashes.txt tests/accuracy/scoreboard.txt"
        status: pass
      - kind: command
        ref: "cmake --workflow --preset ci"
        status: pass
    human_judgment: false
metrics:
  duration: 42min
  completed: 2026-11-02
  tasks: 3
  files: 11
  commits: 4
  plan_head_before: 539481cb4f0bc0c40388110ca6d8f83e4cbdd419
  plan_head_after: ae2959caaa70fb690689a087eef58df988a63f2d
status: complete
actuals:
  tasks: 3
  commits: 4
---

# Phase 27 Plan 04: Cross-Deliverable PAL Acceptance Summary

**All six mapper families now have final PAL/NTSC presentation parity evidence, with the frozen candidate passing the bounded local workflow and retaining NTSC expectations.**

## Accomplishments

- Added an owned colored PAL fixture that checks the loaded library, actual runner process, and dynamically loaded module. Adapter callback RGB is checked by independent indexing into the loaded instance's selected palette; native frame and audio hashes, exact profile rates, callback audio, and runner PPM bytes agree.
- Expanded the regional matrix to mapper IDs 0, 1, 2, 3, 4, and 7 for both PAL and NTSC. Host and runner preferences conflict with explicit region metadata, and reload/refusal behavior keeps the active profile consistent.
- Exercised pitched conversion against a real PAL table while preserving masking, exact 256×240 output, and padding sentinels.
- Raised the AccuracyCoin activation behavior assertion to revision 17, captured the frozen candidate, and refreshed provenance without changing the 56-input fingerprint or canonical scoreboard.
- Preserved the historical behavior-14 receipt, the Plan 27-01 intermediate capture, and the older interrupted campaign's stopped/unknown status.
- Recorded the candidate-bound workflow and review findings in [27-ACCEPTANCE.md](27-ACCEPTANCE.md), [27-REVIEW.md](27-REVIEW.md), and [27-REVIEW-DISPOSITION.md](27-REVIEW-DISPOSITION.md).

## Task Commits

1. **Task 1: Exercise one colored PAL fixture through native API, runner dump and adapter callback** — `5022560`.
2. **Task 2: Expand six-mapper PAL/NTSC acceptance and documentation** — `fed3db1`.
3. **Task 3: Freeze activation, capture provenance, and complete bounded acceptance** — `ae2959c`.

**Plan metadata:** This summary and its review/acceptance records are committed as the final phase documentation update.

## Focused Checks

- `libretro.host` verifies the six-mapper PAL/NTSC matrix, exact rates, native frame/audio hashes, independently indexed callback pixels, runner PPM bytes, and reload/refusal behavior.
- `host.convert` verifies real PAL table conversion across the pitched frame with masking and untouched row padding.
- The final `cmake --workflow --preset ci` run passed all 722 tests and produced the library, runner, and libretro packages. Its AccuracyCoin campaign report SHA-256 matches the retained candidate report.
- The initial full-workflow attempt exposed five metadata-reader failures and one transient inventory-process startup timeout. The metadata cause was corrected and the focused controls passed; the inventory test passed immediately on focused retry. The subsequent full workflow passed without failures. Diagnosis and receipts are detailed in [27-ACCEPTANCE.md](27-ACCEPTANCE.md).
- The full workflow also covered the earlier phases' regression suites, so no duplicate identical full workflow was run.
- A final focused input-fingerprint/activation check passed after writing the closeout docs. CTest pulled in the current-candidate campaign fixture; its report remained byte-identical to the retained capture.

## NTSC and Evidence Limits

The NTSC palette source, frame/audio hash inventory, and canonical scoreboard match the pre-phase commit. No baseline was re-pinned. The 2C07 table is the named software model's published output, while the owned generator formats pinned RGB data rather than recomputing the analog decoder. Nothing in this acceptance qualifies silicon color output, universal calibration, physical host pacing, or the old stopped campaign.

## Review and User Setup

The inline code review found no actionable issues. No human-only verification is required for the scoped machine-checkable criteria; no external workflow, device test, push, PR, merge, or release was performed.

## Self-Check: PASSED

- All three tasks have reachable implementation commits, and their focused tests pass.
- The exact candidate's 56-input digest, report hash, and behavior revision are recorded in the acceptance artifact.
- The full workflow passed 722/722 checks and packaging; NTSC expectations and historical evidence remain unchanged.

---
*Phase: 27-pal-host-presentation-and-ntsc-preservation*
*Completed: 2026-11-02*
