---
phase: 19-jam-reporting-and-sprite-0-hit
plan: "02"
subsystem: runner
tags: [accuracycoin, jam, strict, collector, conformance]
requires: ["19-01"]
provides:
  - "guest-jam naming in the boot, navigation and start observe stages (no report, exit 1)"
  - "strict page mode JAM stderr line"
  - "campaign collector guest-jam whitelist and consistency checks"
  - "accuracycoin.jam boot and strict legs; tracer jam_frame=- pin"
affects: [19-06]
key-files:
  modified:
    - runner/accuracycoin.c
    - tests/accuracy/test_conformance.c
    - tests/cmake/accuracycoin_jam.cmake
    - tests/cmake/accuracycoin_tracer.cmake
    - README.md
key-decisions:
  - "Early-stage JAM reuses observe_note_jam through observe_early_jam (stderr with active_key=-), then goes straight to done: no report, status stays 1"
  - "Strict mode uses one static strict_frame wrapper (separate from the observe formatter, since its message has no active_key)"
  - "Collector resolves the attributed row by full key (jam_row) and passes ACCURACYCOIN_ACTIVE_JAM for it via campaign_row_active_state; the report equality check then proves exactly one fail/0x03/guest-jam row"
requirements-completed: [JAM-01]
completed: 2026-10-25
status: complete
commits: 1
plan_head_before: 2c1a4bec1b5460ce6de2b7bb4b13736f26139eeb
plan_head_after: 48b4fa3
actuals:
  tokens: 40000
  tasks: 3
  commits: 1
---

# Phase 19 Plan 02: JAM naming in every AccuracyCoin stage Summary

A guest JAM is now named in the boot, navigation and start stages (guest-jam metadata, stderr line, no report, exit 1) and in strict page mode (stderr line, exit 1); the campaign collector accepts and cross-checks guest-jam pages and rejects each attribution-dropped variant.

## Evidence per leg (`accuracycoin.jam`)

| Leg | Patch (in memory only) | Exit | Evidence |
|-----|------------------------|------|----------|
| observe page 3, execution stage | last TEST row of page 3 (Plan 19-01) | 0 | report with one fail/0x03/guest-jam row |
| observe page 3, boot stage | byte at the reset-vector target set to `0x02` | 1 | metadata verdict=guest-jam, stop_cause=guest-jam, `jam_frame=0`, `active_key=-`; stderr `... page 3 at frame 0 (active_key=-)`; no report.tsv (only diagnostics.tsv, metadata.txt, raw.tsv) |
| strict page 2, old ROM | last TEST row of the old inventory's page 2 | 1 | stderr `nesturbator-run: JAM stopped AccuracyCoin page 2 at frame N` |

The subcommand prints `jam-observe-exit=0`, `jam-boot-exit=1`, `jam-strict-exit=1`; the CMake script asserts each. Navigation- and start-stage JAMs share the boot branch shape and are not separately constructed (flagged assumption in the plan).

## Fixture controls (`accuracycoin.report`)

- `jam` (page 1, verdict and stop_cause guest-jam, `jam_frame=2`, one fail/0x03/guest-jam row): accepted.
- `jam-cause-dropped` (stop_cause execution-frame-failure): rejected.
- `jam-row-dropped` (attributed row reported as a budget timeout): rejected.
- `jam-frame-missing` (verdict guest-jam, `jam_frame=-`): rejected.
- Each control was proven sensitive: temporarily disabling the pairing check, the `jam_frame` check, or the JAM row classification made `accuracycoin.report` fail (the matching mutation was then accepted); the file was restored afterwards.
- Existing fixtures now emit `jam_frame=-` and classify exactly as before.

## Other changes

- `accuracycoin.tracer` asserts `\njam_frame=-\n` present and `guest-jam` absent on the unpatched pin.
- README observe section: earlier-stage JAM (no report, exit 1), JAM row status, strict-mode message, in-memory patch; the `--accuracycoin-observe` option text notes the exit 1.

## Commit

- `48b4fa3` feat(runner): name a guest JAM in every AccuracyCoin stage (Tasks 1-3, second half of D-20 commit 1). `git show --stat`: README.md 14, runner/accuracycoin.c 160, tests/accuracy/test_conformance.c 209, tests/cmake/accuracycoin_jam.cmake 43, tests/cmake/accuracycoin_tracer.cmake 9 (5 files, +360/-75).

## Deviations from Plan

None - plan executed exactly as written. Two notes: the plan's "write the mutations first and see the rejected ones accepted" could not literally show acceptance before the collector change (an unknown verdict is already rejected), so the controls were instead verified by disabling each new check afterwards. The plan commit ledger file (protocol 0c) was not created; `plan_head_before` is the HEAD at plan start.

## Verification

- `ctest --preset ci -j8`: all tests pass except `accuracycoin.activation` (expected red until Plan 19-06). `scripts/hygiene.sh` exits 0.
- Not run: asan workflow (left to Plan 19-06); `clang-format` is not installed, long lines were wrapped by hand to the 100-column limit.

## Known Stubs

None.

## Threat Flags

None. No ROM bytes are written to any path; patches exist only in memory in `accuracycoin.conformance`.

## Self-Check: PASSED

- All five modified files exist; commit 48b4fa3 is an ancestor of HEAD.
