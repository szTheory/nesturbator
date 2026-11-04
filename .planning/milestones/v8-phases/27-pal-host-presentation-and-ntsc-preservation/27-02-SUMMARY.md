---
phase: 27-pal-host-presentation-and-ntsc-preservation
plan: "02"
subsystem: host-presentation
tags: [pal, palette, display, ntsc]
dependency_graph:
  requires: [27-01]
  provides: [modeled-pal-display-lookup, region-selected-palette-getter]
  affects: [27-03, 27-04]
tech_stack:
  added: []
  patterns: [instance-selected-static-table, offline-data-table-generation]
key_files:
  created:
    - src/palette_pal.inc
    - tools/palgen/palette_pal_values.inc
  modified:
    - tools/palgen/palgen.c
    - src/palette.c
    - tests/core/test_palette.c
    - README.md
    - include/nesturbator.h
decisions:
  - Select the named NESdev 2C07 wiki modeled display table only for a loaded PAL instance; NULL and unloaded instances retain NTSC.
  - Keep table data private, native pixels unchanged, and display RGB outside frame hashes.
  - Keep the published 2C07 model output pinned as text data and generate its C include offline; this avoids using third-party decoder implementation code.
requirements_completed: []
metrics:
  duration: 11min
  completed: 2026-11-02
  tasks: 2
  files: 7
  commits: 4
  plan_head_before: 692942eb8bb267e33a417e38304969748f1f10dc
  plan_head_after: fed8c471b9262f1f63e54662ef9db437332f9e60
status: complete
actuals:
  tokens: 15000
  tasks: 2
  commits: 4
coverage:
  - id: modeled-pal-table
    description: A real PAL load selects independently sampled 2C07 wiki modeled RGB values through the existing getter.
    requirement: VID-01
    verification:
      - kind: unit
        ref: tests/core/test_palette.c#independent 2C07 sample and region-selection assertions
        status: pass
      - kind: command
        ref: "cmake --preset dev && cmake --build --preset dev --target core.palette palgen header.c header.cxx && ctest --test-dir build/dev -R '^(core\\.palette|palette\\.regen|header\\.(c|cxx)|docs\\.links)$' --output-on-failure --no-tests=error"
        status: pass
  - id: ntsc-preservation
    description: NULL/unloaded and NTSC instances retain the original NTSC table and bounded copy behavior.
    requirement: NTSC-01
    verification:
      - kind: unit
        ref: tests/core/test_palette.c#existing NTSC expectations, fallback, bounds and interleaved lifecycle checks
        status: pass
      - kind: command
        ref: "build/dev/tools/palgen/palgen --pal /private/tmp/nesturbator-palette-regenerated.inc; cmp src/palette_pal.inc /private/tmp/nesturbator-palette-regenerated.inc"
        status: pass
human_judgment: false
---

# Phase 27 Plan 02: Modeled PAL Display Lookup Summary

**The public palette getter now selects the named 2C07 modeled RGB table for loaded PAL content while preserving the existing NTSC lookup and copy contract.**

## Accomplishments

- Added an instance-local selection between the existing NTSC table and a private 512-entry PAL display table. Native frame pixels, hashes, API layout, state format, and behavior revision remain unchanged.
- Added independent PAL sample checks, NULL/unloaded fallback, copy bounds, simultaneous PAL/NTSC queries, reload/unload behavior, and checks that palette queries do not mutate either machine state.
- Added an offline `palgen --pal <output.inc>` mode and documented the selected model, its pally v0.24.1 provenance, regeneration command, and color-accuracy limits in the README and public header.

## Focused Checks

- RED: Before implementation, the new PAL getter test failed on the expected PAL values because the getter still returned NTSC colors.
- GREEN: The exact Plan 27-02 focused command passed all five selected checks: `core.palette`, `palette.regen`, `header.c`, `header.cxx`, and `docs.links`.
- The standalone PAL generation check reproduced `src/palette_pal.inc` byte-for-byte. Calling `palgen --pal` without an output path returned the expected usage error.
- No full workflow, AccuracyCoin campaign, hosted workflow, or device test was run; final source-bound acceptance remains in Plan 27-04.

## Model Evidence and Limits

The named model is the NESdev 2C07 wiki palette output associated with pally v0.24.1 and the recorded recipe. Its published 1536-byte RGB output has SHA-256 `cb052e4ef8c738c19c2304804e3851d5afd2ba8bf68ed810cabd69de0fe9e9e5`. The independent tests pin the published samples `00=626262`, `01=002365`, `06=540501`, `0f=000000`, and `16=932824`. These values identify the named model; they do not establish hardware capture or universal calibration.

## Deviations from Plan

The plan called for an owned PAL signal/decode implementation in `palgen`. The official output recipe does not specify the decoder's exact implementation, and the project clean-room rule excludes consulting third-party emulator/decoder implementation source. To preserve the selected model exactly without importing such code, the implementation keeps the published RGB values in `tools/palgen/palette_pal_values.inc` and has the owned offline tool emit the C table from those values. This makes generation deterministic and offline, but it reproduces the pinned published output rather than independently recomputing the PAL decoder. Plan 27-03 adds full-table identity and regeneration coverage; the model caveat remains explicit.

## TDD and Commits

The PAL sample test was committed first and failed against the unchanged getter. The lifecycle/bounds test and its snapshot-check robustness follow-up were committed separately, followed by the implementation and documentation. Plan commits: `97b9034`, `7bef74d`, `bedb0fd`, and `fed8c47`.

## Next Plan

Plan 27-03 expands all-entry palette verification, both generator-mode checks, and developer documentation before Plan 27-04 freezes the source candidate for final acceptance.

## Self-Check: PASSED

- Summary exists at the required phase path; the seven changed product files are present.
- Four reachable Plan 27-02 commits match `actuals.commits`; the recorded before/after heads match the plan commit range.
- Focused verification passed, and no final source-bound acceptance is claimed here.
