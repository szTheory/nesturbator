---
phase: 28-qualified-nes-gameplay-package
plan: "01"
subsystem: testing
tags: [c, cmake, ctest, installed-package, gameplay]

requires: []
provides:
  - Source-backed DABG gameplay evidence through a moved installed package
  - A recurring install.gameplay test with positive, negative, refusal, buffer, and package-boundary controls
affects: [provider-profile, package-release, gameplay-qualification]

actuals:
  tokens: 17870
  tasks: 2
  commits: 5
commits: 5
plan_head_before: 3f31b579ea16c795520c834e7293c1d63812749c
plan_head_after: d7fe22f

tech-stack:
  added: []
  patterns:
    - Public-header-only installed consumer configured from a moved prefix
    - Source-mapped RAM predicates combined with visible pixels and gameplay PCM

key-files:
  created:
    - docs/nrom-gameplay.md
    - tests/cmake/qualified_consumer.cmake
  modified:
    - README.md
    - include/nesturbator.h
    - tests/CMakeLists.txt
    - tests/consumer/CMakeLists.txt
    - tests/consumer/main.c

key-decisions:
  - "Use the permitted DABG image after the bounded Nesteroids rebuild produced a reproducible ROM that did not match the manifest digest and conflicting checked-in symbols; derive DABG state from its pinned source, linker map, and instructions."
  - "Keep the proof scoped to Nesturbator's installed NTSC package; it establishes no LibFuturo host, hardware, or published-package outcome."
  - "Register install.gameplay in the existing CTest workflow and keep upstream source/toolchain reconstruction out of recurring tests."

patterns-established:
  - "Relocated consumer qualification: move the install prefix, remove the first consumer build, and configure using only the moved package."
  - "Semantic controls: require complete valid-content runs and named verdict failures; crashes, wrong ROM identity, or zero frames cannot pass as negative controls."

requirements-completed: [INST-01, GAME-01]
coverage:
  - id: D1
    description: "A clean public C consumer discovers and links the installed library after its prefix is moved."
    requirement: INST-01
    verification:
      - kind: integration
        ref: "cmake -DSOURCE_DIR=. -DBINARY_DIR=build/ci -P tests/cmake/qualified_consumer.cmake"
        status: pass
    human_judgment: false
  - id: D2
    description: "The permitted DABG route proves input-caused gameplay, visible response, gameplay PCM, refused-load continuation, and named false-positive controls."
    requirement: GAME-01
    verification:
      - kind: integration
        ref: "ctest --test-dir build/ci -R '^install\\.gameplay$' --output-on-failure --no-tests=error"
        status: pass
    human_judgment: false

duration: 53min
completed: 2026-11-03
status: complete
---

# Phase 28 Plan 01: Qualified installed-library gameplay summary

**A permitted DABG ROM now proves source-mapped gameplay through a moved installed package, with independent false-positive and refused-load continuation controls.**

## Performance

- **Duration:** 53 minutes elapsed across the interrupted and resumed execution
- **Started:** 2026-11-03T15:02:22Z
- **Completed:** 2026-11-03T15:55:02Z
- **Tasks:** 2
- **Files modified:** 8

## Accomplishments

- Rebuilt the preferred Nesteroids candidate twice with identified cc65 tools. The reproducible output did not match the permitted manifest ROM digest, and its checked-in symbols disagreed; no Nesteroids address was treated as an oracle.
- Rebuilt the permitted DABG ROM twice with identical ROM, map, and debug outputs matching the manifest. The documented RAM allowlist is tied to linker allocations, source declarations, and assembled instruction operands.
- Added a public C consumer and CMake driver that move the installed prefix, remove the earlier consumer build, and verify fresh package/header/library discovery inside the moved prefix.
- Added no-input and Start-only semantic controls, synthetic verdict controls, malformed replacement refusal with twin continuation, exact buffer-boundary checks, wrong-ROM rejection, and an incomplete-package-prefix control.
- Registered the qualified consumer as `install.gameplay`; the latest run passed in 2.06 seconds. The earlier interrupted-session log and stale guide timing were reconciled to this latest run.

## Task Commits

1. **Task 1: Prove one source-backed NROM gameplay path through a moved installed package** - `30bbfb0` (red test), `d7fe957` (implementation and source proof)
2. **Task 2: Reject false gameplay signals and prove refused-load continuation in the recurring check** - `293a086` (control test), `de80840` (completed controls and documentation)
3. **Post-wave input-identity correction** - `d7fe22f` (refresh the AccuracyCoin build-input fingerprint after the public-header and CTest changes)

The GSD metadata commit for this summary and plan tracking is recorded after the tracking updates.

## Files Created/Modified

- `docs/nrom-gameplay.md` - Pinned source-to-ROM/map proof, bounded route, runtime predicates, and reproduction instructions.
- `tests/cmake/qualified_consumer.cmake` - Install/move/reconfigure driver with positive and negative controls.
- `tests/consumer/main.c` - Public API gameplay, control, buffer-edge, and refusal-continuation consumer.
- `tests/consumer/CMakeLists.txt` - Restricts package/header/library resolution to the moved prefix.
- `tests/CMakeLists.txt` - Registers the recurring `install.gameplay` test.
- `include/nesturbator.h` - Documents the fixture-scoped inspection evidence without changing declarations or compatibility versions.
- `README.md` - Links the gameplay qualification guide.

## Decisions Made

- Selected DABG only after the preferred Nesteroids source and build were investigated within the plan's bounds. Its rebuilt image did not match the allowed fixture and the checked-in symbol files conflicted. DABG's rebuilt ROM matched the fixture and its mapped gameplay state was independently tied to source and linker evidence.
- Kept the result limited to the upstream package and this exact permitted NTSC scenario. It is not a LibFuturo host result, hardware proof, or package-publication receipt.
- Kept one-time source/toolchain rebuild evidence local; recurring CTest uses the permitted checked-in ROM and committed source-backed facts without network or a recurring toolchain download.

## Deviations from Plan

None. The plan explicitly allowed the independently justified DABG fallback after the preferred Nesteroids candidate failed its source-to-ROM identity gate.

## Issues Encountered

The executor reached a no-progress checkpoint and was interrupted at the execution boundary before writing its summary. The owner chose to preserve and close out the current work. The partial changes were retained, both plan verification commands were rerun successfully, the guide's runtime was reconciled to the latest 2.06-second CTest result, and the remaining task changes were committed. A later phase-level full workflow ran the registered `accuracycoin.campaign` fixture and it passed; the report digest remained unchanged. The overall workflow failed only the two fingerprint checks because `build_inputs_sha256` still held the prior input identity. The baseline now contains the computed hash, the focused fingerprint test passes, and a direct activation check passed using the report without launching another campaign. The older interrupted run remains unknown and is not reclassified by this current workflow. Unrelated dirty work was preserved.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Plan 28-01 is complete and committed. Plan 28-02 is now unblocked. Phase 28 itself remains in progress, and PKG-01 remains pending until the separate shipping path produces and verifies the published receipt.

---
*Phase: 28-qualified-nes-gameplay-package*
*Completed: 2026-11-03*

## Self-Check: PASSED

All seven declared key files exist, and all four Plan 28-01 task commits plus the input-identity correction are reachable from the current phase branch.
