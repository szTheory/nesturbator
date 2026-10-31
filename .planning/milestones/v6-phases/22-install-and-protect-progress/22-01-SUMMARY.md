---
phase: 22
plan: "01"
subsystem: documentation
tags: [api, state, cmake, documentation]
dependency-graph:
  requires: []
  provides: [canonical-api-guide, README-guide-link-check, status-contract-controls]
  affects: [runner-guide, release-testing-guides, player-readme]
tech-stack:
  added: []
  patterns: [focused CMake/CTest documentation checks]
key-files:
  created:
    - docs/api.md
    - tests/cmake/doc_links.cmake
  modified:
    - README.md
    - tests/cmake/status_table.cmake
    - tests/CMakeLists.txt
decisions:
  - "docs/api.md is the canonical source-tree API, status, and native-state guide."
  - "Exact core-build and content pairing is documented as support guidance, separately from native loader checks."
metrics:
  duration: 13 min
  completed: 2026-10-31
status: complete
actuals:
  tokens: 15756
  tasks: 2
  commits: 3
  plan_head_before: fc1af9c02e19994adc3550d28ebbfd00986a4551
  plan_head_after: 88a5e28ff779eafc3769fd67280f52063756a6d6
---

# Phase 22 Plan 01: Canonical API Guide Summary

README now routes API, battery, native-state, status, and color-table detail to `docs/api.md`; independent CMake checks validate that guide and README's local guide links.

## Completed work

- Added `docs/api.md` for the C API, reset and memory ownership, battery RAM and generation, status values, native save-state format and refusal rules, CPU/PPU behavior notes, and the calibrated display palette.
- Updated the status-table checker to require an explicit canonical `DOC` input. Its isolated controls cover a valid table and stale README copy alongside missing/changed/duplicate/extra rows, missing state heading, and missing rewind warning.
- Added the README documentation-link checker for local `docs/*.md` targets and ordinary ATX heading fragments. Its fixtures distinguish a missing guide from a bad fragment; fenced examples are excluded.
- Preserved the checked README MMC3 support clause and release-marked Apple Silicon installation block byte-for-byte. The README keeps the current supported-content summary and links to the canonical API guide.
- Corrected current-source state prose: format 3 rejects earlier formats, the behavior revision is recorded but not enforced, and exact-build/exact-content pairing is explicitly support guidance rather than a new loader check.
- No runtime code, public symbol, package, live host, or player progress state changed.

## README migration ledger

The ledger covers every original README heading and its unique opening claims. Later destinations refer to the sequential plans in Phase 22; those migrations remain pending.

| Original README heading or claim | Maintained home after this plan |
|---|---|
| `# nesturbator`, three deliverables, no-content card/silence, and current supported-content summary | README; detailed CPU/PPU behavior and behavior revision history in `docs/api.md` |
| Opening supported mapper/cartridge limits and battery/soft-reset availability | Concise limits remain in README; battery/reset detail is in `docs/api.md` |
| Opening save-state API and runner state options (`--state-at-frame`, `--load-state`, `--hash-state`) | Native contract in `docs/api.md`; runner switches in `docs/runner.md` in Plan 22-02 |
| Opening RetroArch rewind/run-ahead and deterministic state claim | README summary; state mechanics in `docs/api.md`, independent host evidence in `docs/testing.md` in Plan 22-03 |
| Opening runner signal-stop, frame-timing, perf gate, and no-skip claims | Runner stop/exit contract in `docs/runner.md` in Plan 22-02; performance and test-policy evidence in `docs/testing.md` in Plan 22-03 |
| Opening 6502 vector match and sample-regeneration claim | Vector match remains in README; fixture, oracle, and regeneration details in `docs/testing.md` in Plan 22-03 |
| Opening PPU sprite/OAM timing and per-board mapper interface | `docs/api.md` — Emulated video and mapper interface |
| Opening behavior revision 8 facts and mid-frame scroll-write claim | Corrected current revision history in `docs/api.md` — Behavior revision history |
| Opening runner invocation and trainer behavior | README; runner details remain for `docs/runner.md` in Plan 22-02 |
| `## Building` | `docs/testing.md` in Plan 22-03 |
| `## Checks` and `## Continuous integration` | `docs/testing.md` in Plan 22-03 |
| `### Phase 20 hosted evidence collection` | `docs/testing.md` in Plan 22-03 |
| `### Performance allow file` | `docs/testing.md` in Plan 22-03 |
| `## Using the library` | `docs/api.md` — Using the library |
| `### Battery saves` | `docs/api.md` — Battery saves; player-facing save actions remain for the progress guide in Plan 22-04 |
| `### Save states` | `docs/api.md` — Save states; player restore and backup guidance remain for Plan 22-04 |
| `### Status codes` | `docs/api.md` — Status codes, checked against the public enum |
| `## Downloads and archives` | Player choices stay in README and release details move to `docs/releases.md` in Plans 22-03/22-04 |
| `## Releases` | `docs/releases.md` in Plan 22-03; README's version-marker ownership remains |
| `## The colour table` | `docs/api.md` — The colour table |
| `## The runner`, `### Stopping the runner`, and `### Exit status` | `docs/runner.md` in Plan 22-02 |
| `## The libretro core` | README capability/control summary; adapter evidence in `docs/testing.md` and API semantics in `docs/api.md`; player guidance remains for Plan 22-04 |
| `## Install in RetroArch (Apple Silicon)` | Literal marked install block and player steps stay in README; release-marker maintenance moves to `docs/releases.md` in Plans 22-03/22-04 |
| `## Try it in RetroArch (from a build)` | `docs/testing.md` in Plan 22-03 |
| `## What it is` | Concise project description stays in README; detailed implementation/testing claims move to `docs/testing.md` in Plan 22-03 |
| `## ROMs` | Player-facing support limits stay in README through Plan 22-04; fixture and conformance details move to `docs/testing.md` in Plan 22-03 |
| `### Regenerating the vector sample` | `docs/testing.md` in Plan 22-03 |
| `## How the code is written` | `docs/testing.md` in Plan 22-03 |
| `## Licence` | README |

The native-state contract retains buffer ownership, frame-boundary calls, all-or-nothing refusal, cartridge identity checks, state lifetime, battery restoration and generation behavior, and rewind consequences. The support recommendation does not imply a loader build-ID check. Published binaries must still be identified from their own release or CI artifacts.

## Verification

- TDD RED: before the implementation, `cmake --preset ci` configured and the status consumer accepted missing `DOC` because the stale README copy remained. `gsd_run check tdd-red-evidence` returned `RED_EVIDENCE_OK` for `state.status_table.self_test`; semantic inspection confirmed the failure was the intended stale-copy gap. The regression was committed as `d536da3`.
- Tracer implementation commit: `2d4f049`. `cmake --preset ci` passed. CTest selection `^(state.status_table|state.status_table.self_test|docs.links|docs.links.self_test|libretro.info_support|cmake.script_policy)$` passed 6/6, including the independent negative controls. The same scoped set was rerun after the tracer commit and passed 6/6 before Task 2.
- Task 2: `git diff --check` passed. CTest selection `^(state.status_table|state.status_table.self_test|docs.links|docs.links.self_test|libretro.info_support)$` passed 5/5. A direct comparison against the pre-migration README confirmed the marked install block and checked MMC3 clause are byte-identical.
- The full `cmake --workflow --preset ci` suite is deferred to the serial phase orchestrator, as directed. No live runtime, host, package installation, or player-progress operation was run.

## TDD Gate Compliance

RED evidence came from the required failing CTest invocation and was registered by the TDD evidence checker. The test failed because the production consumer did not require `DOC`, while a stale README still held the old table. The implementation added the explicit canonical path and negative controls; the focused tracer suite then passed. Task 2 preserved the guide contract and reran the plan's required focused suite.

## Deviations from Plan

None. The repository hook initially rejected a trailing blank line in Task 1; that formatting issue was corrected before its commit, and the commit succeeded with hooks enabled.

## Known Stubs

None found in files created or modified by this plan.

## Threat Flags

None. This plan changes documentation and build-tree-only documentation checks; it adds no runtime trust boundary or external interface.

## Self-Check: PASSED

The summary file exists, and commits `d536da3`, `2d4f049`, and `88a5e28` are ancestors of HEAD.


## Task Commits

1. **Task 1: regression** - `d536da3` (docs/test)
2. **Task 1: canonical API tracer** - `2d4f049` (docs/test)
3. **Task 2: complete API guide** - `88a5e28` (docs/test)
