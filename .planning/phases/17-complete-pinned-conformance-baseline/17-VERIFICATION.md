---
phase: 17-complete-pinned-conformance-baseline
verified: 2026-10-24T08:47:27Z
status: passed
score: 38/38 must-haves verified
covered_files:
  - .github/workflows/ci.yml
  - .planning/phases/17-complete-pinned-conformance-baseline/17-01-PLAN.md
  - .planning/phases/17-complete-pinned-conformance-baseline/17-01-SUMMARY.md
  - .planning/phases/17-complete-pinned-conformance-baseline/17-02-PLAN.md
  - .planning/phases/17-complete-pinned-conformance-baseline/17-02-SUMMARY.md
  - .planning/phases/17-complete-pinned-conformance-baseline/17-03-PLAN.md
  - .planning/phases/17-complete-pinned-conformance-baseline/17-03-SUMMARY.md
  - .planning/phases/17-complete-pinned-conformance-baseline/17-04-PLAN.md
  - .planning/phases/17-complete-pinned-conformance-baseline/17-04-SUMMARY.md
  - .planning/phases/17-complete-pinned-conformance-baseline/17-CONTEXT.md
  - .planning/phases/17-complete-pinned-conformance-baseline/17-REGRESSION.md
  - .planning/phases/17-complete-pinned-conformance-baseline/17-RESEARCH.md
  - .planning/phases/17-complete-pinned-conformance-baseline/17-REVIEW-DISPOSITION.md
  - .planning/phases/17-complete-pinned-conformance-baseline/17-REVIEW-FIX.md
  - .planning/phases/17-complete-pinned-conformance-baseline/17-REVIEW.md
  - .planning/preparation/CONFORMANCE.md
  - .planning/preparation/ENGINEERING.md
  - .planning/preparation/LIBRETRO-AND-RUNNER.md
  - .planning/preparation/README.md
  - README.md
  - include/nesturbator.h
  - runner/CMakeLists.txt
  - runner/accuracycoin.c
  - runner/accuracycoin.h
  - runner/main.c
  - scripts/hygiene.sh
  - tests/CMakeLists.txt
  - tests/accuracy/baseline-metadata.txt
  - tests/accuracy/budgets.txt
  - tests/accuracy/measurements.md
  - tests/accuracy/migration.tsv
  - tests/accuracy/old-673ef-metadata.txt
  - tests/accuracy/old-673ef-scoreboard.txt
  - tests/accuracy/scoreboard-main-metadata.txt
  - tests/accuracy/scoreboard-main.txt
  - tests/accuracy/scoreboard.txt
  - tests/accuracy/test_conformance.c
  - tests/accuracy/test_scoreboard.c
  - tests/cmake/accuracycoin_activation.cmake
  - tests/cmake/accuracycoin_baseline_fetch.cmake
  - tests/cmake/accuracycoin_campaign.cmake
  - tests/cmake/accuracycoin_driver.cmake
  - tests/cmake/accuracycoin_input_fingerprint.cmake
  - tests/cmake/accuracycoin_partial_failure.cmake
  - tests/cmake/accuracycoin_protected.cmake
  - tests/cmake/accuracycoin_regression_controls.cmake
  - tests/cmake/accuracycoin_tracer.cmake
  - tests/cmake/hash_keys.cmake
  - tests/cmake/prepare_scoreboard_baseline.cmake
  - tests/cmake/verify_scoreboard_regression.cmake
  - tests/hygiene/scan_selftest.sh
  - tests/retroarch/test_state_session.sh
  - tests/roms/accuracycoin-673ef.nes
  - tests/roms/accuracycoin.nes
  - tests/roms/manifest.txt
covered_digest: "v3:sha256:1e4c492e05304b65b7d03b9066dd8fcf9160c43c0f87e73eb48f0c25ede75690"
behavior_unverified: 0
overrides_applied: 0
decision_coverage:
  honored: 15
  total: 15
  not_honored: []
human_verification: []
---

# Phase 17: Complete pinned conformance baseline Verification Report

**Phase Goal:** Users can reproduce a complete, immutable AccuracyCoin inventory and read an honest bounded NTSC baseline without losing previously protected passing results.
**Verified:** 2026-10-24T08:47:27Z
**Status:** passed
**Re-verification:** No — initial verification

## Goal Achievement

The four roadmap success criteria and every truth in all four plans were checked against implementation, tests, and captured run evidence. The phase has 38 distinct checks: four roadmap truths and 34 plan truths. All are verified. State transitions, campaign classification, and protected-pass failure behavior have named tests and passed in the exact-source hosted run.

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | The new immutable pin, license/hash, compressed-name decoder and protected migration are validated together; the prior pin and changed meanings remain comparison evidence. | VERIFIED | Manifest and fixture hashes match the two exact MIT pins; parser and migration checks plus activation test passed; migration has 17 source-backed rows. |
| 2 | The runner inventories every scored test and DRAW entry, while malformed, missing, or duplicate inventory data is rejected. | VERIFIED | Inventory output is 151 entries (146 scored, five DRAW); production parser mutation tests and campaign/activation tests passed. |
| 3 | The bounded run classifies each scored row and retains raw results and diagnostics without scoring DRAW entries. | VERIFIED | Independent 22-page campaign report has 146 rows, 22 attributed starts, raw reports and diagnostics; all mutually exclusive status counts are retained. |
| 4 | Prior protected passes remain mapped and measured; same-pin regressions fail; upstream meaning or denominator changes require reviewed dispositions. | VERIFIED | All 17 old keys are present once in migration with old/new measurements and source references; protected observation and regression controls passed. |
| 5 | 17-01 D-01: The comparison fixture, decoder, complete inventory and migration were admitted while old strict/protected checks remained intact until atomic activation. | VERIFIED | Exact new fixture was authenticated; old strict pages 2/14/17 still select old authenticated bytes; 17-04 activated the canonical fixture and expectations together in c23d0ac. |
| 6 | 17-01 D-02: All selected bytes bind to the manifest identity and decode all 22 pages, 146 scored entries and five DRAW records with field validation. | VERIFIED | SHA-256 and pin checks precede parsing; inventory test validates full shape and fingerprint; direct inventory output and hosted activation passed. |
| 7 | 17-01 D-03: Decoded ASCII names are pin-scoped; order/address are provenance; duplicate scored identities, addresses and expanded-name collisions fail. | VERIFIED | Parser retains page/row/address identity and rejects duplicates/collisions; conformance controls passed against immutable fixtures. |
| 8 | 17-01 D-04: All 17 old passes retain measured raw values and source-grounded dispositions; the old 144+5 inventory is distinct from its 17-row measured subset and new 146+5 inventory. | VERIFIED | Old metadata records 144+5, 17 protected results and their raw codes; new metadata records 146+5; migration contains all 17 observations and source dispositions. |
| 9 | 17-01 CONF-01 empty edge: absent, empty, truncated inputs and empty/oversized expanded names fail with the intended inventory reason. | VERIFIED | accuracycoin.inventory tests empty/truncated inputs, terminators and exact/overflow name bounds through production parser helpers; test passed. |
| 10 | 17-01 CONF-01 encoding edge: F0–F4 decode to exact ASCII; unsupported tokens and truncation cannot silently shorten or collide keys. | VERIFIED | Decoder and production controls cover F0–F4, unknown tokens, exact capacity and one-byte overflow; accuracycoin.inventory passed. |
| 11 | 17-01 CONF-02 adjacency edge: only the five DRAW entries may share 0x03FF; scored duplicates fail. | VERIFIED | Parser permits the shared sentinel only for the five DRAW rows; inventory tests assert duplicate scored key/address rejection. |
| 12 | 17-01 CONF-02 empty edge: empty/missing pages or scored/DRAW records fail even if aggregate row count is unchanged. | VERIFIED | Mutated page/row and count-preserving substitution controls call the production parser and assert the specific rejection; passed. |
| 13 | 17-01 CONF-02 ordering edge: navigation retains source page/row order while any sorted report view does not alter it. | VERIFIED | Parsed entries retain source order and provenance; campaign actions use those page/row positions; production tracer and inventory tests passed. |
| 14 | 17-02 D-05: Each of 22 page attempts cold-loads independently, uses menu/header A, qualifies start, preserves pretests, and releases input before execution. | VERIFIED | Campaign driver performs 22 independent runner invocations; page reports retain start attribution and pretests; hosted campaign completed all 22. |
| 15 | 17-02 D-06: Measured guest caps and outer process deadlines bound the campaign, and one page timeout cannot hide later pages. | VERIFIED | budgets.txt and measurements.md record 32 boot, 5 navigation, 1800 execution and 41503 total guest frames; campaign continues after per-page child bounds; actual run completed. |
| 16 | 17-02 D-07: Each of 146 scored entries gets one of five final statuses; FF is skip before running-bit interpretation and valid 41 remains pass. | VERIFIED | accuracycoin.classifier and campaign tests passed; full report uses five statuses and preserves 0x41 in protected data. |
| 17 | 17-02 D-08: Inventory, attempt, observed and terminal coverage remain distinct; outer termination cannot invent raw bytes or active identity. | VERIFIED | Reports separate attempted/observed/terminal totals and retain attempts; outer-failure controls assert infrastructure failure and unobserved raw as “-”; passed. |
| 18 | 17-02 D-09: Reports bind results to ROM/source/behavior/schema/input/budgets and check output writes while retaining sanitized diagnostics. | VERIFIED | Sidecar includes pin/hash, inventory, source revision, mode, budgets, fingerprints and report hash; report, raw and diagnostic paths are relative; activation passed. |
| 19 | 17-02 D-10: Summary counts total 146, show pass/146 and attempted/observed coverage, and list five DRAW rows outside the denominator. | VERIFIED | Full report: 101 pass, 42 fail, 0 skip, 2 not-run, 1 timeout; 146 observed; five DRAW; tracked baseline says pass/146 without claiming all-pass. |
| 20 | 17-02 D-11: Trustworthy bounded observation may succeed with guest nonpasses/timeouts; malformed, incomplete, untrusted or outer-failed evidence and strict regressions fail. | VERIFIED | Collector separates observation completeness from regression verdict; partial/outer failure controls, strict page tests, activation and full campaign passed. |
| 21 | 17-02 CONF-03 boundary edge: work observed at the guest cap is counted before stopping; cap-1/cap/cap+1 and guest/outer causes are distinct. | VERIFIED | Production cap-boundary and outer-deadline controls assert the timing boundary and stop reason; classifier/campaign control suite passed. |
| 22 | 17-02 CONF-03 adjacency edge: an observed active row at cap is timeout, untouched rows are not-run, and FF is not timeout. | VERIFIED | Classifier and report controls exercise the exact raw/status cases; final campaign distinguishes the active guest timeout from untouched rows. |
| 23 | 17-02 CONF-03 empty edge: startup failure links unobserved rows to its cause; missing raw is “-”; no empty report establishes completion. | VERIFIED | Startup/missing-raw and checked-output controls assert failure cause and absent raw; activation and outer-failure tests passed. |
| 24 | 17-02 CONF-03 ordering edge: source order drives actions; fallback and contradictory observations remain linked without choosing a last success. | VERIFIED | Attempt/row order and attempt identifiers flow into reports; regression controls reject contradictory/unattributed output; passed. |
| 25 | 17-02 CONF-03 precision edge: checked integer frame totals reject overflow; raw bytes are exact and host elapsed time is diagnostic only. | VERIFIED | Fixed-width checked accumulation and raw parser controls passed; host elapsed appears only in metadata, never in classification. |
| 26 | 17-03 D-12 authority: protected scoreboard and pin metadata are fetched from one immutable protected-main revision before candidate configuration; unavailable authority fails closed. | VERIFIED | CI fetches protected main before configure; preparation script selects scoreboard and metadata from FETCH_HEAD; baseline-fetch controls passed. |
| 27 | 17-03 D-12 measurement: every same-pin protected pass must appear and be measured passing in this invocation; loss of one fails even if candidate file says pass. | VERIFIED | Protected runner consumes current raw observations; regression controls cover missing/fail/skip/not-run/timeout; hosted protected and regression tests passed. |
| 28 | 17-03 D-04 migration: every old key has a source-backed disposition and actual new observation; changed meanings retain old evidence without reset. | VERIFIED | 17 unique migration rows include old/new symbols, assembly links, changed assertions and actual row/raw/frame references; migration_source and activation passed. |
| 29 | 17-03 D-11 variants: protocol-valid pass variants remain passing with visible meaning while malformed results and protected regressions fail. | VERIFIED | Pinned classifier accepts only source-defined variants including 0x41; malformed and regression reason controls passed. |
| 30 | 17-03 D-13: Protected execution and parser/inventory/classifier/reason-specific controls run unconditionally in normal CI. | VERIFIED | CI always registers these checks; the exact-source workflow ran the configured CMake workflow and reported 663/663 tests passed. |
| 31 | 17-03 CONF-04 flagged assumption: source-grounded review confirms complete protected-key/migration policy independent of deterministic probe classification. | VERIFIED | Both exact MIT assemblies were inspected for all 17 protected labels; four changed-meaning dispositions and the remaining unchanged assertions are explicit in migration.tsv and backed by measured rows. |
| 32 | 17-04 D-01: Canonical pin/hash/license, derived inventory, measured baseline and migration activate together without a broken intermediate strict/protected state. | VERIFIED | Canonical rename, manifest, parser fingerprint, baselines and old strict fixture selection are in the tested atomic implementation commit; accuracycoin.activation passed. |
| 33 | 17-04 D-04: Compact old/new comparison retains all 17 source/measurement dispositions and separate 144+5 / 146+5 summaries. | VERIFIED | Tracked old/new metadata and migration.tsv preserve all counts, raw values, sources and per-key observations. |
| 34 | 17-04 D-09: Compact baseline survives artifact expiry; raw/start/first-failure evidence is retained in CI artifacts with exact identity. | VERIFIED | Canonical report, metadata, old comparison and fingerprints are tracked; exact PR run uploaded and validation confirmed complete raw/report artifact. |
| 35 | 17-04 D-10: Published baseline says pass/146 and all five counts with five separate DRAW rows, without ranking or hardware/playability claims. | VERIFIED | README and measurements state the bounded NTSC result and explicit limitations; counts match actual full report and metadata. |
| 36 | 17-04 D-13: Normal CI always checks parser/protected passes; measured cost selects a full PR or nightly run, and a real full campaign runs. | VERIFIED | A complete required PR campaign was selected and run: 23.18 seconds; full configured Linux workflow was 630 seconds, inside its 1500-second budget. Nightly fallback condition was false. |
| 37 | 17-04 D-14: Named rejection controls exercise production seams and assert intended failure reasons. | VERIFIED | Parser, classifier, protected, baseline-fetch, outer-failure, activation and regression controls all ran in the exact-source CI job; no setup-only substitutes. |
| 38 | 17-04 D-15: Tests, README and public-header comments accompany the change; parity/state/determinism/sanitizer/performance/hygiene gates and dependency policy remain intact. | VERIFIED | README/header updated; exact PR run passed all 15 jobs including six platforms, ASan, both no-FP lanes, hash parity, RetroArch, performance, hygiene and required rollup. |

**Score:** 38/38 truths verified (0 present, behavior-unverified)

## Required Artifacts

GSD artifact verification returned passed with no issues for every declared plan artifact (3/3, 3/3, 3/3 and 4/4). Manual review confirmed substantive behavior, wiring, and live data flow:

| Artifact | Expected | Status | Details |
|---|---|---|---|
| runner/accuracycoin.c | Pin-bound parser, classifier, observer, report and collector | VERIFIED | Compiled into runner and called from CLI; source reads selected ROM bytes, classifies guest RAM results and writes checked reports. |
| tests/accuracy/test_conformance.c | Production parser/classifier/tracer checks | VERIFIED | Mutations reach production helpers; named CTest cases passed. |
| tests/accuracy/migration.tsv | Full protected-key migration | VERIFIED | 17 unique old keys; each has exact source references and measured new observation. |
| tests/cmake/accuracycoin_campaign.cmake | Bounded 22-page execution and final collection | VERIFIED | Invokes production runner per page and collector; actual hosted report completed. |
| tests/accuracy/budgets.txt | Finite campaign bounds | VERIFIED | Caps match full run metadata and test policy. |
| tests/accuracy/measurements.md | Honest observed campaign and CI cost evidence | VERIFIED | Counts and hashes match retained metadata/artifact; explains selected PR lane. |
| tests/cmake/accuracycoin_protected.cmake | Fresh protected-key observation | VERIFIED | Uses current invocation raw evidence before scoreboard verdict. |
| tests/accuracy/scoreboard-main-metadata.txt | Protected-main pin authority | VERIFIED | Read from the same protected revision as scoreboard by pre-configuration fetch. |
| tests/cmake/prepare_scoreboard_baseline.cmake | Protected baseline fetch and handoff | VERIFIED | Fails closed and exports the immutable revision data before candidate CTest setup. |
| tests/accuracy/scoreboard.txt and baseline-metadata.txt | Canonical measured baseline and provenance | VERIFIED | 146 score rows and matching pin, ROM hash, inventory hash, report hash and input fingerprint. |
| tests/roms/accuracycoin-673ef.nes | Retained old pin | VERIFIED | Hash matches old manifest identity; strict pages remain bound to this fixture. |
| tests/roms/accuracycoin.nes and manifest.txt | Current immutable canonical pin | VERIFIED | New MIT source pin and exact byte hash match; parser fingerprint and activation control agree. |
| .github/workflows/ci.yml | Required placement and retained gates | VERIFIED | Full campaign on one required Linux PR job; focused and protected checks remain on normal lanes. |

Plan 17-01 named an intermediate comparison file accuracycoin-6bee.nes. Plan 17-04 explicitly replaces that transitional name with canonical accuracycoin.nes while retaining the old pin as accuracycoin-673ef.nes. Current manifest, wiring and activation test verify that intended atomic rename.

## Key Link Verification

The automated key-link query returned “Target not referenced in source” for links expressed through compilation, generated CTest commands, CMake variables or environment handoff. I manually followed each connection. These are linked; the one nightly link is a conditional fallback that is not selected because the full required PR campaign fits its measured budget.

| From | To | Via | Status | Details |
|---|---|---|---|---|
| runner/main.c | runner/accuracycoin.c | CLI dispatch and shared authenticated ROM bytes | WIRED | runner/CMakeLists.txt compiles both units; main calls pin parser, page observer and production collector helpers. |
| tests/CMakeLists.txt | tests/roms/accuracycoin.nes | Authenticated tracer and canonical activation | WIRED | Registered tests pass fixture and manifest paths to the production runner; the transitional 6bee name is intentionally consolidated. |
| tests/cmake/accuracycoin_campaign.cmake | runner/main.c | 22 observe invocations followed by collection | WIRED | Generated command executes nesturbator-run with page, ROM, frame bound and report directory; hosted full campaign produced its report. |
| runner/accuracycoin.c | build/ci/accuracycoin/full | Per-attempt data and checked report | WIRED | Output paths, metadata and report are written from actual observations; final artifact contains 146 scored rows, raw snapshots and diagnostics. |
| tests/cmake/prepare_scoreboard_baseline.cmake | tests/accuracy/test_scoreboard.c | Protected revision data before candidate configuration | WIRED | CMake exports protected scoreboard and metadata paths; production scoreboard verifier consumes the same-revision authority. |
| tests/cmake/accuracycoin_protected.cmake | tests/accuracy/test_scoreboard.c | Current raw observations | WIRED | Protected observation is built from this invocation’s reports, then compared with the protected baseline; it does not trust candidate pass labels. |
| tests/roms/manifest.txt | tests/accuracy/baseline-metadata.txt | Pin/hash/inventory binding | WIRED | Activation test compares both fixtures, manifest, inventory identity, scoreboard and metadata endpoints. |
| .github/workflows/ci.yml | tests/cmake/accuracycoin_protected.cmake | Pre-configuration fetch and unconditional checks | WIRED | Normal CI runs protected fetch before configure and executes the protected tests in the required Linux job. |
| .github/workflows/nightly.yml | tests/cmake/accuracycoin_campaign.cmake | Use only if full PR campaign exceeds policy budget | N/A — condition false | The tested PR path completes the full campaign in 23.18 seconds, so the roadmap’s full-PR option is satisfied and the nightly fallback is inactive. |

## Data-Flow Trace

| Artifact | Data variable | Source | Produces real data | Status |
|---|---|---|---|---|
| AccuracyCoin inventory | Selected ROM bytes and pin digest | Manifest-authenticated fixture | Yes; parsed across all 22 source pages and fingerprinted | FLOWING |
| Per-page observation | Guest result RAM, page/row provenance, frame counter | Independently cold-loaded production runner instance | Yes; 22 attempts produce raw snapshots, status and causes | FLOWING |
| Full campaign report | Per-page reports and attempt metadata | Production collector | Yes; 146 scored rows plus five separate DRAW rows | FLOWING |
| Canonical scoreboard | Captured full observation | tests/accuracy/scoreboard.txt plus baseline metadata | Yes; 101 pass rows out of 146 and raw/frame evidence | FLOWING |
| Protected verdict | Same-revision protected rows and this run’s raw rows | Protected-main fetch plus current report | Yes; each protected pass is freshly measured | FLOWING |

The retained full report’s actual counts are 101 pass, 42 fail, 0 skip, 2 not-run and 1 guest timeout, summing to 146. It records all 22 pages launched, start-attributed and completed; 146 attempted and observed rows; 143 terminal rows; and five DRAW records outside the denominator. The report hash is 6d0a811960f6964e7fc66802c6e4ade0c4ffb8772c40d88d50cf79e1f8463652. The compact canonical baseline records the same counts and identifies its pre-commit source capture honestly as revision d4a8a4aaf123 with source_dirty=1; it does not relabel that baseline as a clean c23 build.

## Behavioral Spot-Checks

| Behavior | Command/evidence | Result | Status |
|---|---|---|---|
| New-pin inventory reports every page and excludes DRAW from scored count | build/ci/runner/nesturbator-run --accuracycoin-inventory --rom tests/roms/accuracycoin.nes | 151 rows: 146 scored, five DRAW; exit 0 | PASS |
| Inventory malformed-input and exact-name boundaries | ctest --test-dir build/ci --output-on-failure -R ^accuracycoin\.inventory$ | 1/1 passed | PASS |
| Five-way raw classification and valid variants | ctest --test-dir build/ci --output-on-failure -R ^accuracycoin\.classifier$ | 1/1 passed | PASS |
| Protected migration matches exact source assertions | ctest --test-dir build/ci --output-on-failure -R ^accuracycoin\.migration_source$ | 1/1 passed | PASS |
| Exact source on required PR head | GitHub Actions run 40075421001, head c23d0ac845ed405147c7006e87400abeda94b8cb | All 15 jobs success; run URL: https://github.com/szTheory/nesturbator/actions/runs/40075421001 | PASS |
| Full workflow and bounded campaign | Primary Linux x64 job 120817601380 ran cmake --workflow --preset ci | 663/663 CTest tests passed; full campaign 23.18s; CTest 616.97s; configured job 630s under the existing 1500s budget | PASS |
| Campaign and protected controls | Same hosted CTest log | campaign, inventory, classifier, migration_source, activation, protected_observation, protected, baseline_fetch and regression_controls all passed | PASS |

The full hosted PR run is the whole-workflow proof. A broader local workflow was not rerun; the preserved platform sleep interruptions were not treated as passing evidence, and the configured hosted gate was not weakened.

## Probe Execution

Not applicable. The plans and success criteria do not declare probes, and no conventional scripts/*/tests/probe-*.sh files exist for this phase.

## Requirements Coverage

| Requirement | Source plan | Description | Status | Evidence |
|---|---|---|---|---|
| CONF-01 | 17-01, 17-03, 17-04 | Validate immutable pin, license/hash, decoder and migration while retaining old comparison | SATISFIED | Exact fixture/manifest hashes, parser tests, 17-row migration and activation all verified. |
| CONF-02 | 17-01, 17-02, 17-04 | Inventory every scored test and DRAW; reject missing, duplicate and malformed table entries | SATISFIED | Complete 22-page production parse, five DRAW records and named malformed-input controls passed. |
| CONF-03 | 17-01–04 | Bounded run classifies all scored rows with retained results/diagnostics and excludes DRAW | SATISFIED | Actual 146-row campaign report, raw artifacts and five status totals validated. |
| CONF-04 | 17-01, 17-03, 17-04 | Protect existing passes, fail same-pin regressions and review semantic/denominator changes | SATISFIED | Immutable same-revision authority, fresh pass measurements and 17 source-backed migration dispositions passed. |

All four IDs map to Phase 17 and are declared by its plans; no orphaned Phase 17 requirement was found. The requirement checkboxes remain untouched as requested.

## Decision Coverage

All 15 trackable decisions in 17-CONTEXT.md are represented in the plans and shipped artifacts: 15 honored, 0 unhonored. This gate is advisory and does not change status.

## Test Quality Audit

| Test file/group | Linked requirements | Active | Disabled | Circular expected values | Assertion strength | Verdict |
|---|---|---:|---:|---:|---|---|
| tests/accuracy/test_conformance.c | CONF-01, CONF-02, CONF-03 | Yes | 0 | 0 | Exact parser/classifier values, mutation rejection reasons and behavior | PASS |
| tests/accuracy/test_scoreboard.c | CONF-03, CONF-04 | Yes | 0 | 0 | Current raw observations compared with separately fetched protected revision | PASS |
| tests/cmake/accuracycoin_*.cmake and tests/CMakeLists.txt | CONF-01–04 | Yes | 0 | 0 | End-to-end process, output, protected authority, report and failure behavior | PASS |

The tests call production helpers or the production runner. Expected identities come from exact pinned MIT source, immutable fixture hashes, prior protected-main evidence and the separately retained actual report. Harness output files used by negative controls are temporary inputs; the tests do not generate their expected baseline from the same invocation. No test-quality blocker or warning was found.

## Anti-Patterns Found

None in the changed text implementation, tests or documentation. No unreferenced TBD, FIXME or XXX marker was found. Empty return values in parser/test helpers serve explicit invalid-input or lookup-miss paths; they do not feed user-visible fallback data. The complete inventory and report are populated from authenticated ROM bytes and guest observations.

## Human Verification Required

N/A — infrastructure and test-tooling phase with no user-facing UI or external service flow. Every phase acceptance criterion was exercised through deterministic parser, runner, campaign and CI checks; no plan deferred a human-check item.

## Deferred Items

None. No identified Phase 17 gap is scheduled to a later roadmap phase. Separate Nightly vectors-full debt and Phase 18 execution-fuzz/JAM work remain outside this phase and are not conditions for its goal.

## Gaps Summary

No blocking gaps. The measured NTSC baseline is complete and bounded, but it is not an all-pass or hardware-accuracy claim: 101 of 146 scored rows passed, while 42 failed, two were not-run and one guest attempt timed out. All five DRAW entries are separately listed. The report retains the failed and incomplete outcomes, their raw evidence/causes and source identity rather than hiding them or changing the denominator.

---

Verified: 2026-10-24T08:47:27Z  
Verifier: the agent (gsd-verifier)
