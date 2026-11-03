---
phase: 27-pal-host-presentation-and-ntsc-preservation
verified: "2026-11-03T04:23:02Z"
status: passed
score: "16/16 must-have truths verified"
covered_files:
  - .planning/phases/27-pal-host-presentation-and-ntsc-preservation/27-01-PLAN.md
  - .planning/phases/27-pal-host-presentation-and-ntsc-preservation/27-01-SUMMARY.md
  - .planning/phases/27-pal-host-presentation-and-ntsc-preservation/27-02-PLAN.md
  - .planning/phases/27-pal-host-presentation-and-ntsc-preservation/27-02-SUMMARY.md
  - .planning/phases/27-pal-host-presentation-and-ntsc-preservation/27-03-PLAN.md
  - .planning/phases/27-pal-host-presentation-and-ntsc-preservation/27-03-SUMMARY.md
  - .planning/phases/27-pal-host-presentation-and-ntsc-preservation/27-04-PLAN.md
  - .planning/phases/27-pal-host-presentation-and-ntsc-preservation/27-04-SUMMARY.md
  - .planning/phases/27-pal-host-presentation-and-ntsc-preservation/27-ACCEPTANCE.md
  - .planning/phases/27-pal-host-presentation-and-ntsc-preservation/27-AUTOMATION-DISPOSITION.md
  - .planning/phases/27-pal-host-presentation-and-ntsc-preservation/27-REVIEW-DISPOSITION.md
  - .planning/phases/27-pal-host-presentation-and-ntsc-preservation/27-REVIEW.md
  - .planning/phases/27.1-close-state-01-regional-state-validation-gaps/27.1-ACCEPTANCE.md
  - README.md
  - docs/api.md
  - docs/runner.md
  - docs/testing.md
  - include/nesturbator.h
  - runner/main.c
  - src/palette.c
  - src/palette_pal.inc
  - tests/CMakeLists.txt
  - tests/accuracy/baseline-metadata.txt
  - tests/accuracy/measurements.md
  - tests/cmake/accuracycoin_activation.cmake
  - tests/cmake/palette_regen.cmake
  - tests/core/test_palette.c
  - tests/host/test_convert.c
  - tests/libretro/libretro_host.c
  - tools/palgen/palette_pal_values.inc
  - tools/palgen/palgen.c
covered_digest: "v4:sha256:981a00a2b9b1a715fd2698a3039b9a053c8252b9d28213f444f908cf29da4926"
behavior_unverified: 0
overrides_applied: 0
flagged_prohibitions: 3
decision_coverage:
  honored: 10
  total: 10
  not_honored: []
human_verification: []
---

# Phase 27: PAL Host Presentation and NTSC Preservation Verification

**Phase Goal:** A host receives consistent PAL selection, timing and modeled display colors through all three deliverables while existing NTSC behavior and output remain protected.

**Verified:** 2026-11-03T04:23:02Z

**Status:** passed

**Re-verification:** No — the prior report had no `gaps:` section, so this used initial-mode criteria against the current source.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---:|---|---|---|
| 1 | Library, runner, and libretro resolve the same content and eligible preference to the same region; library and runner expose the active region and exact profile rates. | ✓ VERIFIED | `src/cartridge.c` resolves the loaded region; `runner/main.c` reads region and `nesturbator_info` from the same instance for `--info`. `tests/libretro/libretro_host.c` exercises explicit PAL/NTSC metadata with opposing preferences and checks the direct library, runner, and loaded module. |
| 2 | Omitting `--info` preserves runner execution output byte-for-byte. | ✓ VERIFIED | The host test removes the single opt-in record and compares all remaining output. CTest also registers exact-output info and runner-option cases. |
| 3 | Libretro reports the loaded region, exact PAL/NTSC frame cadence, 48000 Hz audio, and fixed 256×240 geometry through load lifecycle changes. | ✓ VERIFIED | `retro_get_region()` reads the active instance; `retro_get_system_av_info()` converts the instance's exact rate fractions at the adapter boundary. The six-mapper host test checks PAL 322445/6448, NTSC 39375000/655171, 48000/1, geometry, reload, and refusal behavior. |
| 4 | A loaded PAL instance returns deterministic output from the named 2C07 model, including raw emphasis mapping, and the accuracy claim stays limited to that model. | ✓ VERIFIED | `src/palette.c` selects the per-instance table; `tests/core/test_palette.c` serializes all 512 entries as RGB and checks the independent 1536-byte model digest and emphasis samples. README, public header, API guide, and testing guide identify the model and its limits. |
| 5 | Independent model samples and getter copy/lifecycle controls distinguish PAL from the retained NTSC table. | ✓ VERIFIED | `tests/core/test_palette.c` checks the nine published PAL samples, NTSC literals, null/unloaded fallback, two loaded regions, repeated queries, reload/unload, clipped/partial/zero/NULL copies, and state non-mutation. |
| 6 | Native pixels and 256×240 dimensions remain unchanged, as do the NTSC table bytes and RGB assertions. | ✓ VERIFIED | The palette is downstream of native pixels. The PAL conversion tests retain source-index semantics; the NTSC source table and fixed runner/scoreboard files compare unchanged against the pre-phase commit. |
| 7 | The owned offline generator and public documentation identify the selected model, recipe, and model-only accuracy limits. | ✓ VERIFIED | `tools/palgen/palgen.c`, `tools/palgen/palette_pal_values.inc`, README, header, API guide, and testing guide describe the pinned NESdev 2C07 output and offline regeneration. The docs state the data is a model, not hardware capture or universal calibration. |
| 8 | The strict 56-input identity gate and historical evidence boundaries remain intact while fingerprinted inputs change. | ✓ VERIFIED | `tests/cmake/accuracycoin_activation.cmake` still compares the live 56-input digest/count and fixed report/scoreboard expectations. The historical behavior-14 receipt and Plan 27-01 intermediate capture are retained; the older interrupted campaign remains stopped and unknown. Current-source fingerprint evidence is detailed under WR-01 below. |
| 9 | PAL emphasis mapping, getter lifecycle, and bounded copy behavior stay deterministic without changing NTSC entries. | ✓ VERIFIED | The PAL and NTSC tables are selected from the loaded instance only; public-getter tests cover all emphasis groups, repeated/interleaved instances and buffer sentinels. `host.convert` checks masking, source/destination pitch, and untouched padding with a real PAL table. |
| 10 | Owned offline generation reproduces both tables and preserves default NTSC output byte-for-byte. | ✓ VERIFIED | `palette.regen` keeps the existing NTSC invocation/reference; `palette.regen.pal` exercises `--pal` against the checked-in PAL table. Both passed in the current workflow. |
| 11 | Owned content on mapper IDs 0, 1, 2, 3, 4, and 7 agrees on resolved PAL/NTSC behavior through all three deliverables. | ✓ VERIFIED | The real runner/module matrix loads owned fixtures for all six mapper IDs in both regions and tests metadata precedence, preference, reload, and refusal behavior. `libretro.host` passed in the current full run. |
| 12 | Adapter RGB matches independent indexing into the selected library palette and runner conversion, while native frame/audio output remains deterministic. | ✓ VERIFIED | The host test indexes callback pixels by each native pixel's low nine bits, checks runner PPM bytes and direct/runner frame and PCM hashes, and requires nontrivial color/emphasis and audio. `host.convert` also exercises the real PAL table across pitched rows. |
| 13 | Existing NTSC palette, frame/audio hash inventory, callback output, and regressions remain protected without silent baseline re-pinning. | ✓ VERIFIED | `git diff --exit-code ad24e05247f4ebe5cb7efb8212b632c2f8cf4f8f -- src/palette_ntsc.c tests/runner/hashes.txt tests/accuracy/scoreboard.txt` returned no diff. The current unfiltered workflow passed without changing these references. |
| 14 | The combined current candidate passes the bounded local workflow, with current source identity and evidence limits observable. | ✓ VERIFIED | The current `build/ci/accuracycoin/full/metadata.txt` records source revision `b266ce79cfd8`, dirty status, the exact 56-input digest/count, and behavior revision 17. The current report digest matches the pinned report; activation and input-fingerprint checks passed. The current full workflow passed configure/build, 722/722 CTest tests, and all three packages. This does not qualify physical hardware or restart the older campaign. |
| 15 | The unchanged 56-input gate binds the final candidate report after the behavior-17 activation check and fingerprinted inputs are frozen. | ✓ VERIFIED | The current campaign metadata records 56 inputs and digest `e944fa324cc62b2685ce478d26c0eaded021f363c95bffa773d9699519cd522a`; the fresh report hash is `f3b5bbf7ddcf4d18deabda6d875b441330dc71c97b8aca5fdcc38bf471f98f6c`. Current `accuracycoin.input_fingerprint` and `accuracycoin.activation` passed. |
| 16 | Final campaign results and hashes remain at established expectations, with the historical receipt, intermediate capture, and stopped run kept distinct. | ✓ VERIFIED | The current report and metadata show 22/22 pages, 146 terminal scored rows, 126 pass / 20 fail, 5 inventory-only draws, and zero skip/not-run/timeout. Report and canonical scoreboard hashes match the pinned values; no scores or hashes were re-pinned. The old interrupted campaign was not resumed. |

**Score:** 16/16 truths verified (0 present, behavior-unverified).

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|---|---|---|---|---|
| VID-01 | 27-02, 27-03, 27-04 | Deterministic named modeled 2C07 palette and emphasis output, with model-only limits. | ✓ SATISFIED | Full public-table digest, independent samples, real conversion and adapter comparisons, and documentation caveats. |
| AV-01 | 27-01, 27-04 | Shared effective region and timing through library, runner, and libretro. | ✓ SATISFIED | Exact-rate and lifecycle assertions in `libretro.host`; runner profile output and active library queries. |
| NTSC-01 | 27-01 through 27-04 | Preserve NTSC behavior and output hashes across six supported mapper families. | ✓ SATISFIED | Fixed source/hash/scoreboard comparisons, cross-deliverable matrix, palette regression and current full workflow. |

All plan requirement IDs are in `REQUIREMENTS.md` and mapped to Phase 27. No additional requirement is mapped to this phase without a plan.

### Decision Coverage

The decision-coverage query found all 10 trackable decisions in `27-CONTEXT.md` honored by shipped artifacts. The three plan-level transparency prohibitions remain flagged and unresolved as Plan 27-04 directs; they are evidence limits, not owner acceptance prompts. See [the automation disposition](27-AUTOMATION-DISPOSITION.md). This gate is advisory and does not affect status.

## Required Artifacts

| Artifact | Expected | Status | Details |
|---|---|---|---|
| `runner/main.c` | Opt-in resolved-profile report | ✓ VERIFIED | Reads effective region and exact rates from one active instance; runner behavior is exercised by the real-process matrix. |
| `tests/libretro/libretro_host.c` | Direct-library, real-runner, and loaded-adapter profile oracle | ✓ VERIFIED | Dynamically loads the adapter, launches the runner, and checks profile, output, callback, and lifecycle behavior. |
| `src/palette_pal.inc` | Generated static 512-entry modeled PAL table | ✓ VERIFIED | Private table is consumed through the public palette getter and checked against independent model bytes. |
| `tools/palgen/palgen.c` | Offline PAL mode retaining default NTSC output | ✓ VERIFIED | PAL mode formats pinned RGB data; default invocation remains the existing NTSC generator. |
| `src/palette.c` | Instance-selected palette copy | ✓ VERIFIED | Uses loaded effective region and preserves NULL/NTSC fallback and count clipping. |
| `tests/core/test_palette.c` | Full model digest and getter/emphasis coverage | ✓ VERIFIED | Independent sample/digest constants plus lifecycle and copy-bound assertions. |
| `tests/cmake/palette_regen.cmake` | Separate PAL regeneration mode | ✓ VERIFIED | CTest compares PAL and NTSC generator output with checked-in reference data. |
| `include/nesturbator.h` | Public region/palette contract | ✓ VERIFIED | Documents separate region/info queries, palette selection, model provenance and accuracy limits. |
| `tests/host/test_convert.c` | Pitched converter behavior with real PAL table | ✓ VERIFIED | Checks index masking, 256×240 extent and row-padding sentinels. |
| `27-ACCEPTANCE.md` | Phase candidate workflow receipt and limits | ✓ VERIFIED | Accurately records the Phase 27 candidate at its acceptance point and distinguishes the historical/intermediate runs. |
| `27-REVIEW-DISPOSITION.md` | Review finding disposition | ✓ VERIFIED | WR-01 remains recorded open; its effect on this verification is assessed below. |
| `tests/cmake/accuracycoin_activation.cmake` | Strict fingerprint and campaign activation checks | ✓ VERIFIED | Compares the live input digest/count and current report/scoreboard expectations; current activation passed. |
| `tests/accuracy/baseline-metadata.txt` | Campaign provenance and fixed expectation metadata | ⚠️ WARNING | The 56-input digest is current, but `source_revision` and `campaign_artifact` still name the earlier Phase 27 capture. Current-run identity is present in the fresh `build/ci` campaign metadata; this checked-in pointer remains stale. |
| `tests/accuracy/measurements.md` | Preserved historical receipt and current-phase measurement | ✓ VERIFIED | Prior behavior-14 evidence is retained, with later phase evidence appended rather than replacing history. |

## Key Link Verification

The automated key-link query did not resolve several links because plan `to` values name functions or behavior rather than a file path. Direct source/test tracing confirms the links below:

| From | To | Via | Status | Details |
|---|---|---|---|---|
| `runner/main.c` | `nesturbator_get_region` and `nesturbator_get_info` | One active loaded instance | ✓ WIRED | `print_profile_info()` queries both on the same `inst`; tests launch this runner path. |
| `tests/libretro/libretro_host.c` | `libretro/libretro.c` | Dynamic module load and region/AV exports | ✓ WIRED | The test resolves `retro_get_region`, `retro_get_system_av_info`, and load/run callbacks from the actual module. |
| `tools/palgen/palgen.c` | `src/palette_pal.inc` | Offline generation and byte comparison | ✓ WIRED | `palette.regen.pal` runs the generator and compares output to the checked-in table. The PAL source values are pinned model data, not an independent decoder implementation. |
| `src/palette.c` | `libretro/libretro.c` | Load-time palette refresh | ✓ WIRED | Successful `retro_load_game()` refreshes the adapter table from the active library instance. |
| `tests/core/test_palette.c` | `src/palette_pal.inc` | Public getter RGB digest | ✓ WIRED | The linked core getter serializes the selected table to RGB and checks the fixed independent digest. |
| `tests/cmake/palette_regen.cmake` | `tools/palgen/palgen.c` | Separate NTSC and PAL invocations | ✓ WIRED | Both CTest registrations execute the tool and compare with checked-in references. |
| `tests/libretro/libretro_host.c` | `runner/main.c` | Real runner process invocation | ✓ WIRED | The presentation matrix launches the runner with profile, hash, and dump options. |
| `tests/libretro/libretro_host.c` | `nesturbator_get_palette` | Independent callback indexing | ✓ WIRED | Callback RGB is compared to the direct instance's selected table indexed by native pixels. |
| `tests/runner/hashes.txt` | `runner.write_hashes.content` | Existing NTSC inventory comparison | ✓ WIRED | The registered runner test compares the canonical file; the current workflow ran it. |
| `tests/accuracy/baseline-metadata.txt` | `tests/cmake/accuracycoin_activation.cmake` | Fingerprint and report identity checks | ✓ WIRED | Activation reads metadata and verifies current digest/count and report hash; the descriptive source/path fields are not equality-checked, which is the scope of WR-01. |

## Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
|---|---|---|---|---|
| Runner `--info` | Region and exact rate fractions | Successfully loaded library instance via public queries | Yes; current cartridge/profile determines the query result. | ✓ FLOWING |
| Libretro AV and region | FPS, sample rate, region | Same loaded instance; exact fractions convert only at the libretro API boundary | Yes; the module queries the current instance. | ✓ FLOWING |
| Palette / callback RGB | 512 RGB values and native pixel index | Checked-in named PAL/NTSC model tables selected by the loaded instance, then indexed by native pixel low nine bits | Yes; static lookup data is the intentional palette source, with PAL bytes independently pinned and tested. | ✓ FLOWING |

## Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|---|---|---|---|
| Full current-source regression, packages, and configured campaign | `cmake --workflow --preset ci` | Configure/build passed, 722/722 CTest tests passed in 467.34 seconds, and library, runner, and libretro packages were produced. `retroarch.session.controls` passed in 275.10 seconds. | ✓ PASS |
| PAL/NTSC six-mapper library/runner/adapter behavior | `libretro.host` within the current full workflow | Passed; current log shows the test completed successfully. | ✓ PASS |
| Full PAL RGB digest, NTSC palette checks, and both generator modes | `core.palette`, `palette.regen`, `palette.regen.pal`, and `host.convert` within the current full workflow | All passed; the table digest, regeneration comparisons, real-table conversion, and padding checks ran. | ✓ PASS |
| Current campaign binding and NTSC expectations | `accuracycoin.campaign`, `accuracycoin.input_fingerprint`, and `accuracycoin.activation` within the current full workflow | Passed; fresh report digest matches `f3b5bbf7ddcf4d18deabda6d875b441330dc71c97b8aca5fdcc38bf471f98f6c`, with 22 pages, 126 pass, 20 fail, 5 draws, and no skipped/not-run/timeout rows. | ✓ PASS |
| Fixed NTSC sources and expected output inventory | `git diff --exit-code ad24e05247f4ebe5cb7efb8212b632c2f8cf4f8f -- src/palette_ntsc.c tests/runner/hashes.txt tests/accuracy/scoreboard.txt` | Exit 0; no diff. | ✓ PASS |

## Probe Execution

Not applicable. Phase 27 is not a migration/tooling phase, and no plan or summary declares a probe script.

## Test Quality Audit

| Test File | Linked Req | Active | Skipped | Circular Expected Values | Strongest Assertion | Verdict |
|---|---|---:|---:|---|---|---|
| `tests/core/test_palette.c` | VID-01, NTSC-01 | Yes (`core.palette`) | 0 | No; samples and full digest come from the recorded published model oracle. | Value: 512×RGB digest, independent samples, emphasis, region and bounded-copy behavior. | PASS |
| `tests/libretro/libretro_host.c` | AV-01, VID-01, NTSC-01 | Yes (`libretro.host`) | 0 | No; direct output, independent RGB indexing, runner bytes and literal rates are compared. | Behavioral: real module/runner load, frame/audio progression, callbacks, conversion and lifecycle. | PASS |
| `tests/host/test_convert.c` | VID-01, NTSC-01 | Yes (`host.convert`) | 0 | No; the real table is checked against indexed values and sentinels. | Behavioral: masking, pitched rows, fixed frame extent, and untouched padding. | PASS |
| `tests/cmake/palette_regen.cmake` | VID-01, NTSC-01 | Yes (`palette.regen`, `palette.regen.pal`) | 0 | No; generator output is compared with committed reference data. | Value: exact file comparison for both tables. | PASS |

The disabled-test scan found no skipped requirement-linked tests. Test-side writes create owned fixture inputs and runner output files; no test generates its expected PAL digest or NTSC reference values from the system under test.

## Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---:|---|---|---|
| `tests/accuracy/baseline-metadata.txt` | 6, 32, 54 | WR-01: refreshed input digest is paired with an older `source_revision` and `campaign_artifact` path. | ⚠️ Warning | This weakens the checked-in artifact pointer, but does not invalidate the current workflow evidence: the fresh `build/ci` campaign metadata records current revision `b266ce79cfd8`, the same 56-input digest/count, and the report hash verified by current activation. The current campaign was generated in this workflow, not resumed from the older stopped run. The baseline metadata warning remains open and was not modified here. |

No unreferenced `TBD`, `FIXME`, or `XXX` markers, empty implementations, disabled tests, or placeholder implementation were found in the reviewed phase files. The README's “not available with AccuracyCoin modes” is a documented option restriction, not a stub.

### WR-01 Impact Assessment

The warning does **not** change any Phase 27 must-have truth. The current workflow independently generated a campaign at `build/ci/accuracycoin/full/`; its `metadata.txt` identifies the current source revision and 56-input digest, and its `report.tsv` SHA-256 matches the pinned report. `accuracycoin.input_fingerprint` and `accuracycoin.activation` passed. The mismatch in tracked `baseline-metadata.txt` remains a real provenance-maintenance warning: the stored `source_revision` and capture path still describe the earlier Phase 27 receipt, so that path must not be presented as the current-worktree capture. This verification leaves the finding open and makes no source or metadata edits.

## Automation Disposition

The project verification policy removes avoidable owner checkpoints when existing evidence and
bounded review can settle the technical questions. Plan 27-04 explicitly directs that the three
transparency prohibitions remain flagged and unverified. They are limits on claims and authority,
not unresolved user choices or missing behavior checks. The [automation disposition](27-AUTOMATION-DISPOSITION.md)
records each boundary and the evidence supporting no UAT.

## Human Verification

Not required. The phase's criteria are machine-checkable through current source review and owned
tests. The three plan prohibitions are preserved as unresolved evidence boundaries by explicit
plan direction; they do not request physical testing, a new product preference, or new authority.
The [automation disposition](27-AUTOMATION-DISPOSITION.md) records why no owner checkpoint is
needed and keeps the older campaign stop intact.

## Gaps Summary

No code truth, key link, requirement, or current-workflow check failed. The open WR-01 warning does not invalidate the current candidate's exact digest/report checks, though the tracked baseline capture pointer remains stale and is left open for the v8 audit. All three plan-level transparency prohibitions remain flagged and unresolved; they are not converted into hardware claims or user acceptance decisions. No human UAT or implementation change is required for this phase.

---

_Verified: 2026-11-03T04:23:02Z_
_Verifier: the agent (gsd-verifier)_
