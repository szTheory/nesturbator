---
phase: 22-install-and-protect-progress
plan: "03"
subsystem: documentation
tags: [release, testing, cmake, ctest, evidence]
dependency-graph:
  requires:
    - phase: 22-01
      provides: canonical API guide and README documentation-link checks
    - phase: 22-02
      provides: canonical runner guide and runner contract checks
  provides:
    - canonical-release-guide
    - canonical-testing-guide
    - concise-readme-developer-route
  affects: [player-readme, plan-22-04]
tech-stack:
  added: []
  patterns: [existing-document-and-package-controls, source-bound-evidence-migration]
key-files:
  created: [docs/releases.md, docs/testing.md]
  modified: [README.md]
key-decisions:
  - "docs/releases.md owns archive inventory, install-line checks, marker mechanics, publication order and attestation provenance; the README keeps its marked install command."
  - "docs/testing.md owns engineering checks and evidence limits, with API, runner and release semantics linked to their canonical guides."
  - "Built-module tests, build-tree archive checks, six-target CI packaging and live host evidence remain distinct."
metrics:
  duration: 9 min
  completed: 2026-10-31
status: complete
actuals:
  tokens: 61101
  tasks: 2
  commits: 2
  plan_head_before: 1067bf6e8927fc2801a8b356bc9bc455e38583d6
  plan_head_after: e9d9449603e28ba45b1689a33c9d176b321c8a50
requirements-completed: []
---

# Phase 22 Plan 03: Release and Testing Guide Migration Summary

The README now routes maintainers to canonical release and testing guides while preserving its checked support clause and literal Apple Silicon install block.

## Accomplishments

- Added [docs/releases.md](../../../docs/releases.md) for the three source-build components, exact archive inventory, install-line validation, version-marker ownership, release workflow, checksums and attestation order.
- Added [docs/testing.md](../../../docs/testing.md) for the README's build, CTest, conformance, CI, hosted collector, performance, libretro and RetroArch evidence. Repository paths are rebased for a guide under docs/.
- Kept API and runner semantics in [docs/api.md](../../../docs/api.md) and [docs/runner.md](../../../docs/runner.md); testing material links to those owners.
- Reduced [README.md](../../../README.md) to the player entry, supported-content summary, checked MMC3 clause, installation route and short developer links.
- Preserved the literal marked install block and MMC3 support clause byte-for-byte against the Task 1 baseline. release-please-config.json, CMake packaging and archive allowlists were unchanged.

## Consolidated README migration ledger

This ledger covers the original README across Plans 22-01 through 22-03. Player installation, controls and progress remain in README for Plan 22-04.

| Original README material | Final maintained home |
|---|---|
| Project identity, three deliverables, no-cartridge test card, supported mappers and short PPU capability summary | README; detailed native behavior in docs/api.md |
| Unheaded trainer RAM, cartridge refusal and reload claims | docs/api.md owns the API contract; conformance evidence is described in docs/testing.md |
| Unheaded fixture list, mapper test ROM results, frame/movie hashes, AccuracyCoin pins, inventory, raw result codes, page observations, campaign caps and protected PASS evidence | docs/testing.md; fixture identities and independent oracles remain in tests/ and tests/roms/manifest.txt |
| Unheaded CPU RAM inspection, frame/audio format, APU/PPU timing, mapper interface and behavior-revision details | docs/api.md owns the contract; docs/testing.md records linked test evidence and its limits |
| Building, Checks and Continuous integration | docs/testing.md — build prerequisites, presets, focused commands, no-skip policy, CTest groups and CI lanes |
| Nested Phase 20 hosted evidence collection and Performance allow file | docs/testing.md, preserving source identity, collector debt, thresholds, hash policy and historical qualifications |
| Using the library, Battery saves, Save states, Status codes and The colour table (Plan 22-01) | docs/api.md |
| Runner options, output and movie examples, Stopping the runner and Exit status (Plan 22-02) | docs/runner.md |
| Downloads and archives | docs/releases.md for component contents and checks; README links to the guide for player installation |
| Releases, release-marker formatting, version policy, credentials, exact CI gate, checksums, provenance and publication order | docs/releases.md |
| The runner and its nested stop/exit contracts | docs/runner.md; the independently checked NTSC MMC3 support clause remains in README |
| The libretro core, including metadata claims, load/unload, battery span, state, JAM, audio, run-ahead and fixture-identity checks | docs/testing.md for test scope; docs/api.md for native API semantics; README keeps a short adapter route |
| Install in RetroArch (Apple Silicon) | The marked version line, download pipeline, extraction operands and player steps remain in README. Marker maintenance and the macOS source-build extraction check are in docs/releases.md. |
| Try it in RetroArch (from a build) | docs/testing.md, preserving the build-tree command and exact hosted/local evidence limits |
| What it is, asset ownership and Licence | Short project/asset/license material remains in README; test fixture licensing and manifest provenance are in docs/testing.md |
| ROM test inventory, vector fixture provenance and Regenerating the vector sample | docs/testing.md, including the literal regeneration command and manifest update procedure |
| How the code is written | README keeps the short ownership statement and PROVENANCE.md link; test and clean-room context is in docs/testing.md |

## Verification

- Task 1 focused selection passed 6/6 before commit and again after the tracer commit. It covered README guide links, release version markers and independent release/archive controls.
- Task 2 exact selection passed 16/16 after the final documentation edits: status table and self-test, README links and self-test, runner option/exit source comparisons and relocation controls, release marker scan and negative fixture, info claim/support/staging checks and archive self-test.
- git diff --check passed. A direct comparison confirmed the marked install block and checked MMC3 clause are unchanged from the Task 1 baseline.
- A local-link pass over README.md, docs/releases.md and docs/testing.md checked 38 local targets; none were missing. The release guide contains no manually maintained semantic-version literal.
- The existing native test source and recorded evidence were reused. This plan did not run full CI, a conformance or AccuracyCoin campaign, package creation or extraction, a live RetroArch session, private scene replay, or player-progress operations. Those limits are stated in docs/testing.md.

## Deviations from Plan

None. No runtime code, CMake package inventory, archive allowlist, release-marker owner, dependency or test registration changed.

## Authentication Gates

None.

## User Setup Required

None.

## Next Plan Readiness

Plan 22-03 is complete. Plan 22-04 remains the next sequential plan for the player installation, controls and progress route. READY requirements remain pending until the phase is complete. Full CI remains with the serial phase executor. No package publication, host setup or gameplay acceptance is claimed.

## Self-Check: PASSED

- docs/releases.md, docs/testing.md and this summary exist.
- Task commits da972fe and e9d9449 are ancestors of HEAD.

## Task Commits

1. **Task 1: release guide** - `da972fe` (docs/test)
2. **Task 2: testing guide** - `e9d9449` (docs/test)
