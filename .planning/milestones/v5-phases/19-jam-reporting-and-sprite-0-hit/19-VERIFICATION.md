---
phase: 19-jam-reporting-and-sprite-0-hit
verified: 2026-10-30T04:27:40Z
status: passed
score: 4/4 must-haves verified
covered_files:
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-01-PLAN.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-01-SUMMARY.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-02-PLAN.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-02-SUMMARY.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-03-PLAN.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-03-SUMMARY.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-04-PLAN.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-04-SUMMARY.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-05-PLAN.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-05-SUMMARY.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-06-PLAN.md"
  - ".planning/phases/19-jam-reporting-and-sprite-0-hit/19-06-SUMMARY.md"
  - "README.md"
  - "include/nesturbator.h"
  - "libretro/libretro.c"
  - "runner/accuracycoin.c"
  - "runner/accuracycoin.h"
  - "src/ppu.c"
  - "tests/CMakeLists.txt"
  - "tests/accuracy/baseline-metadata.txt"
  - "tests/accuracy/scoreboard.txt"
  - "tests/accuracy/test_conformance.c"
  - "tests/cmake/accuracycoin_activation.cmake"
  - "tests/cmake/accuracycoin_campaign.cmake"
  - "tests/cmake/accuracycoin_driver.cmake"
  - "tests/cmake/accuracycoin_jam.cmake"
  - "tests/cmake/accuracycoin_tracer.cmake"
  - "tests/cmake/vector_api_policy.cmake"
  - "tests/cmake/verify_scoreboard_regression.cmake"
  - "tests/core/test_api.c"
  - "tests/libretro/libretro_host.c"
  - "tests/ppu/test_sprites.c"
  - "tests/runahead_model.h"
  - "tests/runner/hashes.txt"
covered_digest: "v4:sha256:c6b4635011dcf22dbfefadbcbdb86dfd19f96d96d736eca4f5c6c7ece19be1da"
behavior_unverified: 0
overrides_applied: 0
decision_coverage:
  honored: 20
  total: 20
  not_honored: []
---

# Phase 19: JAM reporting and sprite-0 hit Verification Report

**Phase Goal:** A developer reading an AccuracyCoin run sees a guest JAM named as a JAM, and a game's sprite-0 status-bar split lands on the console's line.
**Verified:** 2026-10-30T04:27:40Z
**Status:** passed
**Fresh verification:** Yes. The prior report had no gaps, so the current-code checks were re-established from the roadmap and plans.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | An AccuracyCoin run stopping on a guest JAM reports guest-jam, and a control that drops JAM attribution fails. | VERIFIED | Current runner code returns guest-jam for an active raw 0x03, records jam_frame, and writes one fail/0x03/guest-jam row for the attributed key. Current CI ran accuracycoin.jam and accuracycoin.report successfully; the former patches the pinned guest image in memory and asserts the metadata, exit status, stderr/stdout, and single attributed report row. The report fixture rejects jam-cause-dropped, jam-row-dropped, and jam-frame-missing. |
| 2 | libretro.runahead shows that run-ahead across a JAM gives the same picture and silence as running without run-ahead. | VERIFIED | Current CI passed libretro.host and libretro.runahead against the built adapter. The latter reports JAM frame 12, all five JAM-image controls tripped, and nonzero coverage for every run-ahead window class at depths 1–3. In retro_run, STOP_JAM calls the video callback with the matching cached frame or initialized black and returns before audio callbacks. |
| 3 | Sprite 0 Hit behavior and Arbitrary Sprite zero pass in the canonical scoreboard within 1800 frames, with owned tests covering the hit edge and OAM identity. | VERIFIED | The fresh full AccuracyCoin campaign and activation test passed at current behavior revision 9. Both page-19 rows are pass/0x01 at 537 frames, below the unchanged 1800 cap. Current ppu.sprites passed; its direct cases exercise the x=254/x=255 latch edge, OAMADDR $80 selecting sprite 32, misaligned $81 copy order, the $FD boundary, and the no-hit out-of-range case. |
| 4 | Rows passing on main remain protected by accuracy.scoreboard.no_lost_pass on every CI lane; any moved frame hash is re-pinned with its reason. | VERIFIED | Current local CI passed accuracy.scoreboard.no_lost_pass, accuracycoin.protected, runner.write_hashes.content, and accuracycoin.activation. The current campaign report is complete and matches its revision-9 baseline. Retained Phase 21 source-bound evidence records the 114 protected starting PASS keys on all six lanes; that later compatibility evidence is referenced here without re-auditing Phase 21. Phase 19's revision-7 re-pin changed 12 saved-state hash rows because the state header records the behavior revision; the frame-hash inventory did not move. |

**Score:** 4/4 truths verified, 0 present-but-behavior-unverified.

### Historical Phase 19 result and current compatibility

These are separate campaign snapshots. The Phase 19 merge commit ab9c2e6 contains behavior revision 7, a 146-row scoreboard with 108 PASS / 38 FAIL / no SKIP, NOT_RUN, or TIMEOUT, and both target sprite rows pass/0x01 at 521 frames. The fresh current campaign is behavior revision 9 and has 119 PASS / 27 FAIL / no SKIP, NOT_RUN, or TIMEOUT, plus five inventory-only DRAW rows; both target rows pass/0x01 at 537 frames. The current header remains ABI 1 and uses state format 3 / behavior revision 9 after later phases. These later revisions preserve the Phase 19 behavior; the 108/38 revision-7 numbers are not presented as the current score.

## Plan must-haves, artifacts, and links

All six Phase 19 plan frontmatters were checked. The artifact query found all 18 declared artifacts present and substantive. The key-link query directly parsed 2 of 11 entries; the other entries use symbol-qualified source names or dynamic report/state paths, which that query does not treat as file paths. Manual source tracing below verified those connections, and the corresponding current CTests passed.

### Required Artifacts

| Artifact | Expected | Status | Evidence |
|---|---|---|---|
| runner/accuracycoin.c and runner/accuracycoin.h | JAM status propagation, row classifier, metadata, collector validation | VERIFIED | 19-01 and 19-02 artifact checks passed. The active-JAM classifier, early-stage exits, numeric jam_frame, and collector consistency checks are present and exercised by accuracycoin.classifier, accuracycoin.jam, accuracycoin.tracer, and accuracycoin.report. |
| tests/accuracy/test_conformance.c and tests/cmake/accuracycoin_jam.cmake | In-memory guest patch and JAM behavior controls | VERIFIED | Test code authenticates the unmodified pinned ROM, chooses a row by parsed inventory, patches only an allocated copy, and runs observe, boot, and strict legs. CMake registers accuracycoin.jam; the test passed. tests/roms has no working-tree changes. |
| libretro/libretro.c | JAM presentation cache tagged by frame number and timeline | VERIFIED | Substantive adapter logic saves cache identity, retains it across a same-timeline state load, changes generation on a diverged state, and sends the matching frame or black on JAM without audio. |
| tests/runahead_model.h and tests/libretro/libretro_host.c | JAM-aware oracle, adapter callbacks, silence and diverged-timeline cases | VERIFIED | Host tests call the JAM-aware oracle and the dynamically built libretro adapter. libretro.host and libretro.runahead passed in the fresh workflow. |
| src/ppu.c and tests/ppu/test_sprites.c | OAMADDR-driven evaluation and sprite-0 hit status behavior | VERIFIED | The live PPU starts evaluation at OAMADDR on dot 65, marks the dot-66 in-range sprite as sprite zero, copies secondary OAM in order, clears OAMADDR during rendering tile loading, and raises hit status through the PPU tick path. Direct owned cases and the CTest passed. |
| tests/accuracy/scoreboard.txt and tests/cmake/accuracycoin_activation.cmake | Canonical campaign results, cap and page-19 assertions | VERIFIED | The current full campaign produced both target rows at 537 frames. accuracycoin.campaign and accuracycoin.activation passed; activation checks the full report digest, canonical row statuses, page-19 completion, and the 32/5/1800 budgets. |
| include/nesturbator.h, tests/cmake/vector_api_policy.cmake, and tests/runner/hashes.txt | Public compatibility and saved-state/frame hash pins | VERIFIED | Current declarations pin ABI 1, format 3, and behavior revision 9; vectors.api_policy and runner.write_hashes.content passed. Historical revision-7 state-row changes are identified separately above. |

### Key Link Verification

| From | To | Via | Status | Evidence |
|---|---|---|---|---|
| tests/accuracy/test_conformance.c | runner/accuracycoin.c | In-memory JAM legs call observe and strict-page APIs | WIRED | The test selects and patches guest bytes, then invokes production runner functions. accuracycoin.jam passed. |
| runner/accuracycoin.c metadata writer | runner/accuracycoin.c campaign collector | jam_frame, verdict, stop cause, active key, and report row consistency | WIRED | Collector reads the metadata fields, resolves the key to the final row, and validates the single fail/0x03/guest-jam report row. accuracycoin.report and the campaign passed. |
| libretro/libretro.c retro_unserialize | include/nesturbator.h | Successful load validates state before reading CRC at offset 28 and frame number at offset 32 | WIRED | Current code reads the documented offsets only after a successful native state load; host serialization and run-ahead tests passed. |
| libretro/libretro.c retro_run | libretro/libretro.c retro_unserialize | Cache frame/timeline identity decides which picture a later JAM presents | WIRED | Successful loads preserve or advance timeline generation based on state CRC; STOP_JAM requires matching frame and generation. The diverged-timeline host case and run-ahead matrix passed. |
| tests/libretro/libretro_host.c | tests/runahead_model.h | JAM oracle frame records are compared against adapter callbacks per tick | WIRED | The host includes and calls the JAM-aware oracle; current output reports the expected frame, window classes, and tripped controls. |
| src/ppu.c sprite_access | src/ppu.c sprite_evaluate and hit detector | Tile-loading reset feeds next-line OAMADDR start; dot-66 identity reaches status bit 6 | WIRED | Evaluation starts from OAMADDR at dot 65; sprite fetch transfers identity into the sprite-zero slot mask; the PPU tick invokes the hit detector before fetch shifting. ppu.sprites passed. |
| include/nesturbator.h revision define | tests/cmake/vector_api_policy.cmake | Public declaration digest pins the revision contract | WIRED | Current revision 9 declaration and pinned digest agree; vectors.api_policy passed. |
| AccuracyCoin campaign report | tests/accuracy/scoreboard.txt | Activation validates report digest and keyed status/raw/frame rows | WIRED | The fresh full report matches the canonical revision-9 campaign baseline; campaign and activation tests passed. |
| tests/cmake/accuracycoin_activation.cmake | tests/accuracy/baseline-metadata.txt | Measured counts, budgets, revision, and build-input fingerprint | WIRED | Activation checks those metadata fields and completed successfully against the current campaign. |

### Data-Flow Trace

| Artifact | Data variable | Source | Produces real data | Status |
|---|---|---|---|---|
| AccuracyCoin observe report | current row bytes and report.tsv rows | Production frame execution and reads of the guest's result RAM, then the active-state classifier | Yes; the current campaign runs the pinned AccuracyCoin image | FLOWING |
| libretro presentation | io.video, xrgb cache, callback frame and audio count | Native nesturbator_run_frame output; STOP_JAM uses a same-timeline cached frame or initialized black | Yes; host test calls the built adapter and compares callbacks with the oracle | FLOWING |
| PPU sprite status | OAM, secondary OAM, sprite pattern bytes, status bit 6 | Current PPU memory and per-dot evaluation/fetch | Yes; owned synthetic sprite cases drive the PPU and assert status/pixels | FLOWING |
| AccuracyCoin baseline | full report and keyed scoreboard | Fresh current runner campaign compared with the measured revision-9 canonical baseline | Yes; current campaign artifact and activation check passed | FLOWING |

## Requirements Coverage

| Requirement | Source plans | Description | Status | Evidence |
|---|---|---|---|---|
| JAM-01 | 19-01, 19-02 | A guest JAM is named as a JAM in AccuracyCoin rather than a generic frame failure. | SATISFIED | accuracycoin.jam covers execution, boot, and strict mode; accuracycoin.report rejects attribution-dropped controls. |
| JAM-02 | 19-03 | Run-ahead across JAM keeps the same picture and silence. | SATISFIED | libretro.host and libretro.runahead pass on the current adapter; the JAM matrix compares video, audio, state, and controls. |
| SPR-01 | 19-04, 19-05, 19-06 | Sprite-0 hit is set under console conditions and on its dot. | SATISFIED | Current page-19 target passes at 537 frames; ppu.sprites asserts the boundary and hit latch timing using NESdev-derived expected values. |
| SPR-02 | 19-04, 19-05, 19-06 | First sprite evaluated acts as sprite 0 wherever it sits in OAM and completes within the frame cap. | SATISFIED | Current Arbitrary Sprite zero row passes at 537 frames; owned tests cover OAMADDR $80, $81, and $FD evaluation paths. |

Requirements traceability lists exactly these four IDs for Phase 19; there are no orphaned requirements.

### Decision Coverage

All 20 trackable decisions in 19-CONTEXT.md are honored by shipped artifacts. The decision-coverage query returned 20/20, with no unhonored decisions.

## Test Quality Audit

| Test File | Linked Req | Active | Skipped | Circular | Assertion level | Verdict |
|---|---|---:|---:|---:|---|---|
| tests/accuracy/test_conformance.c and tests/cmake/accuracycoin_jam.cmake | JAM-01 | Yes | 0 | 0 | Behavioral, including row attribution, metadata, and exit code | PASS |
| tests/libretro/libretro_host.c and tests/runahead_model.h | JAM-02 | Yes | 0 | 0 | Per-tick frame, silence, state, timeline, and negative-control comparisons | PASS |
| tests/ppu/test_sprites.c | SPR-01, SPR-02 | Yes | 0 | 0 | Specific pixel/status values and OAM state at defined dots | PASS |
| tests/cmake/accuracycoin_campaign.cmake and tests/cmake/accuracycoin_activation.cmake | SPR-01, SPR-02 | Yes | 0 | 0 | Live pinned campaign values and keyed regression checks | PASS |

The relevant tests contain no disabled cases. Synthetic fixtures supply test inputs, not expected outputs. PPU expected values are labeled as NESdev-derived. The AccuracyCoin scoreboard is a measured regression baseline, not independent proof of physical hardware behavior; the preparation notes explicitly state that AccuracyCoin has no per-test hardware certification.

## Behavioral Spot-Checks

The fresh command was cmake --workflow --preset ci. The retained log records exit 0, 695/695 tests passed in 405.61 seconds, and generated library, runner, and libretro packages.

| Behavior | Current run result | Status |
|---|---|---|
| Guest JAM is named, attributed, and produces the correct report | accuracycoin.jam passed; accuracycoin.report and accuracycoin.tracer passed | PASS |
| Campaign and page-19 target rows match the current baseline | accuracycoin.campaign and accuracycoin.activation passed; both sprite rows are pass/0x01 at 537 frames | PASS |
| JAM picture, silence, and diverged-timeline protection | libretro.host and libretro.runahead passed; JAM frame 12, five controls tripped, all four window classes covered at depths 1–3 | PASS |
| OAMADDR start, sprite-0 identity, and hit edge | ppu.sprites passed with all current owned sprite cases | PASS |
| No lost protected pass and current state/frame hashes | accuracy.scoreboard.no_lost_pass, accuracycoin.protected, and runner.write_hashes.content passed | PASS |

### Probe Execution

Skipped: Phase 19 is not a migration/tooling phase, and its plans declare no probe scripts.

## Human Verification Required

N/A. This phase has no manual visual or external-service step. The end-user-visible runner output and adapter callbacks are asserted by automated integration tests. No truth was left behavior-unverified.

## Anti-Patterns Found

No TBD, FIXME, XXX, TODO, HACK, placeholder, disabled test, or implementation stub was found in the scoped code and tests. The NULL returns found in test helpers are error paths for invalid input/allocation and are not empty implementations.

| File | Line | Finding | Severity | Impact |
|---|---:|---|---|---|
| src/ppu.c | 428-429 | Current review IN-02: the sprite-0 collision comment cites HWP.03 and HWP.07 rather than sprite-evaluation reference HWP.06. | Info | Citation quality only; no runtime behavior impact. |
| libretro/libretro.c | 60-78 | Historical review IN-01 remains open: saving full native state on each converted frame has per-frame cost. | Info | The plan measured the cost; no performance gate is required by this phase. |

Current review reports 0 critical, 0 warning, and 1 info. Historical WR-01 and WR-02 are recorded fixed in 19-REVIEW-FIX.md; they are not current functional gaps. The initial Phase 19 revision-7 plan evidence records the per-frame save cost separately.

## Gaps Summary

No gaps. All four roadmap truths and all four mapped requirements are verified in the current code and current CI run. Later phases changed the scoreboard and native state format; their revision-9 compatibility results are kept distinct from Phase 19's revision-7 result above.

---

_Verified: 2026-10-30T04:27:40Z_
_Verifier: the agent (gsd-verifier)_
