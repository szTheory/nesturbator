---
phase: 27-pal-host-presentation-and-ntsc-preservation
plan: "03"
subsystem: host-presentation
tags: [pal, palette, display, documentation]
dependency_graph:
  requires: [27-02]
  provides: [exhaustive-pal-model-check, dual-palette-regeneration, host-palette-guidance]
  affects: [27-04]
tech_stack:
  added: []
  patterns: [explicit-rgb-byte-serialization, shared-owned-sha256]
key_files:
  created: []
  modified:
    - tests/core/test_palette.c
    - tests/CMakeLists.txt
    - tests/cmake/palette_regen.cmake
    - README.md
    - include/nesturbator.h
    - docs/api.md
    - docs/testing.md
decisions:
  - Serialize XRGB lookup results as explicit R, G, B bytes before hashing so the oracle is independent of host endianness.
  - Link the runner's existing SHA-256 implementation only into core.palette; do not add a dependency or core symbol.
  - Document the pally recipe as provenance while stating that the owned PAL generator formats pinned published RGB data.
requirements_completed: []
metrics:
  duration: 5min
  completed: 2026-11-02
  tasks: 2
  files: 7
  commits: 2
  plan_head_before: 0979343099a908a1f3e7281ad44253c3b91d7d8e
  plan_head_after: 539481cb4f0bc0c40388110ca6d8f83e4cbdd419
status: complete
actuals:
  tokens: 8000
  tasks: 2
  commits: 2
coverage:
  - id: complete-pal-model
    description: All 512 public PAL getter entries serialize to the fixed official 1536-byte RGB digest.
    requirement: VID-01
    verification:
      - kind: unit
        ref: tests/core/test_palette.c#nine published samples, emphasis ordering, full digest and repeat-query checks
        status: pass
      - kind: command
        ref: "cmake --preset dev && cmake --build --preset dev --target core.palette palgen && ctest --test-dir build/dev -R '^(core\\.palette|palette\\.regen(\\.pal)?)$' --output-on-failure --no-tests=error"
        status: pass
  - id: reproducible-tables
    description: Independent generation modes reproduce the PAL table and preserve the default NTSC output.
    requirement: NTSC-01
    verification:
      - kind: command
        ref: "palette.regen and palette.regen.pal"
        status: pass
  - id: documented-display-contract
    description: README, public header, API guide and test guide name the model and its evidence limits.
    requirement: VID-01
    verification:
      - kind: command
        ref: "cmake --build --preset dev --target core.api header.c header.cxx && ctest --test-dir build/dev -R '^(core\\.api|header\\.(c|cxx)|vectors\\.api_policy(\\.self_test)?|docs\\.links)$' --output-on-failure --no-tests=error"
        status: pass
human_judgment: false
---

# Phase 27 Plan 03: Exhaustive Palette and Documentation Summary

**All 512 public PAL lookup entries now match the fixed published model digest, both palette tables have separate regeneration checks, and host documentation explains the model boundary.**

## Accomplishments

- Added an endian-independent R/G/B byte serialization and verified the 1536-byte public PAL table against the official model digest. The test also pins all nine published sample values and explicit PPUMASK bit 5/6/7 emphasis examples.
- Reused the runner's owned SHA-256 implementation only in `core.palette`; no core linkage, dependency or public symbol was added.
- Added `palette.regen.pal` while retaining the original `palette.regen` NTSC command and reference unchanged.
- Expanded the README, public header, API guide and testing guide with the exact pally provenance recipe, PAL emphasis mapping, native/display distinction, offline generation command, and model-only accuracy limits.

## Focused Checks

- `core.palette`, `palette.regen`, and `palette.regen.pal` passed (3/3).
- `core.api`, `header.c`, `header.cxx`, `vectors.api_policy`, `vectors.api_policy.self_test`, and `docs.links` passed (6/6).
- No full workflow, AccuracyCoin campaign, hosted workflow, or device test was run; final source-bound acceptance remains Plan 27-04.

## Model and Evidence Limits

The digest establishes byte-for-byte identity with the named NESdev 2C07 wiki software model. It does not establish direct hardware output or universal television calibration. The PAL source table remains pinned published RGB data, and the owned generator formats that data rather than independently recomputing the decoder; this carry-forward limitation is detailed in [27-02-SUMMARY.md](27-02-SUMMARY.md).

## Deviations from Plan

No new deviations. The pinned-data generation approach and its limitation remain as recorded for Plan 27-02. The strict 56-input source gate, historical receipt, intermediate report, and older stopped/unknown campaign were left unchanged.

## Next Plan

Plan 27-04 finishes fingerprinted inputs, freezes the candidate, captures the configured `accuracycoin.campaign`, refreshes provenance only, and runs the bounded full workflow against unchanged inputs.

## Self-Check: PASSED

- Summary exists at the required phase path; all seven modified paths exist.
- Two reachable Plan 27-03 commits match `actuals.commits`; the before/after heads match the plan commit range.
- Both focused command groups passed, and no final source-bound acceptance is claimed here.
