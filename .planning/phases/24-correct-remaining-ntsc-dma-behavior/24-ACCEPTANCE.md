# Phase24 acceptance — awaiting separate runtime authority

## Outcome and owner decision

All three implementation plans are committed, tested with bounded owned fixtures and reviewed. Current source7f31579 is native format6 / behavior13 / ABI1. Five row corrections have independent traces and wrong-model evidence; this is not a fresh AccuracyCoin result. Full phase/requirements remain incomplete.

**Recommend authorizing one local full workflow invocation, with a new39-minute resource ceiling and all existing per-test/page limits unchanged. The combined local completion time is unqualified; this is a proposed owner budget, not a claim that the suite will finish within it.** This would gather fresh campaign and local protected-page observations plus the remaining local checks, including existing RetroArch session tests. It cannot establish six-platform/performance acceptance by itself. No push, PR, hosted scheduling, merge or release is included. A decision to keep the existing stop requires no confirmation.

The prior campaign was interrupted and its result is unknown. The owner's latest approval covered the countdown/state incompatibility only, while explicitly retaining this stop. There is currently **no authority to run the command below**.

## Exact proposed local scope

Before execution, re-read current source and read-only CTest selection; stop if command, caps, selected tests or source differ materially from this record. Preserve existing ignored build/ci AccuracyCoin evidence and hash output in a separate timestamped ignored archive, because campaign collection replaces aggregate report files and protected setup recreates its own work directory. Do not delete or relabel the prior interrupted evidence. Clear only the newly generated hash output as required by the existing CI preparation.

The proposed single workflow command is:

```sh
cmake --workflow --preset ci
```

Keep the existing build/ci FULL=ON configuration. Do not configure FULL=OFF, exclude campaign/guards, raise limits or start a push/PR as a workaround. `ci` configures Release with warnings-as-errors, builds, tests with4 parallel jobs and packages. A single isolated invocation is allowed only after explicit approval; its new overall cap would be39 minutes as an explicit owner resource ceiling. There is no measured combined local duration, so the run may stop incomplete when this budget expires. This does not transfer either hosted job's runtime authority to the local run. Keep all current shorter test/page deadlines. Stop on actual interruption or overall expiry; target cleanup to that invocation's own processes, preserve partial results and do not retry automatically. Intentional timeout/failure fixtures remain selected; their expected simulated failures are not permission to retry an interrupted real campaign.

After a complete attributable result, Plan04 may update scoreboard/measurements/provenance only from those actual observations and record any failures/missing evidence. That is routine authorized follow-through to the approved measurement, not authority for another full workflow or campaign. If the run is interrupted or times out, record that result and stop; no baseline update can turn incomplete observations into acceptance.

## Actual existing selection, read without execution

`ctest --test-dir build/ci --show-only=json-v1` lists707 tests. The [complete test-name/timeout/fixture inventory](24-CI-SELECTION.tsv) is retained. Existing cache is FULL=ON, VECTORS_FULL=OFF and has no hosted full-vector inventory. It is a local configured inventory, not an executed candidate or authoritative hosted result; fresh configure-time comparison is required after authority. Source-derived hosted selection also registers this run's full-vector inventory.

Relevant transitive closures:

| Consumer | Required fixture | Setup that would execute |
|---|---|---|
| accuracycoin.activation, accuracycoin.nmi_targets, accuracycoin.v5_targets | accuracycoin_full_report | accuracycoin.campaign |
| accuracycoin.protected, accuracy.scoreboard.no_lost_pass, accuracycoin.regression_controls | accuracycoin_protected_observations | accuracycoin.protected_observation |
| accuracycoin.report_fresh | accuracycoin_report_fresh | accuracycoin.report_scratch |

The campaign executes all22 pages of pin6bee919529289c555803b612b8c9e4f7eaaeeabb, ROM SHA2568ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc. Current source and generated command agree: boot32 frames, navigation5 frames per page, execution1800 frames, page15seconds, collection10seconds, outer CTest370seconds. No cap is widened. A complete inventory contains146 scored test rows plus5 DRAW rows; each row needs terminal raw/attempt provenance, not inferred PASS or a missing-data skip.

Hosted protected setup retains old-pin pages2/14/17 and derives every necessary new-pin page from its independently prepared cold protected-main authority. The proposed local workflow does not execute the hosted prepare_scoreboard_baseline.cmake pre-step and has no freshly prepared cold-main authority. Both authority environment variables were confirmed unset during this read-only preflight. With unset authority environment variables it falls back to checkout scoreboard-main.txt/scoreboard-main-metadata.txt; local observations are therefore history/candidate-bounded and cannot establish the protected-main requirement. A separately attributable cold-main preparation or hosted result remains necessary. Its page cap is30seconds; CTest setup760seconds. The hosted authoritative page set is computed from that cold baseline during its separately authorized execution; local fallback page selection is not promoted to that authority. The attached [historical PASS catalog](24-PROTECTED-PASS-KEYS.tsv) lists119 current-pin and17 legacy PASS keys for review; it is not a fresh cold-main authority or a substitute for newly derived keys. Preserve all of them and any additional protected-main PASS key, including DMA + $4015 Read, Misaligned OAM DMA and INC $4014.

Local full `ci` includes retroarch.session.source_root (120seconds), retroarch.session.controls (900seconds) and retroarch.session.readiness (60seconds). They were not run in the bounded work. Hosted platform builds use `ci-build` where specified, with the existing separate session-controls job; this source-defined partition is not the same as claiming the local full workflow establishes hosted coverage.

## Fresh evidence still required

| Requirement | Missing evidence |
|---|---|
| Five selected rows | DMA Landing on Write; DMC DMA + OAM DMA; DMC DMA Bus Conflicts; Explicit DMA Abort; Implicit DMA Abort individually PASS with unchanged classifier-accepted raw values. Both early/later implicit outcomes stay accepted. |
| Completeness | Attributable source/build/ROM/report identity, all146 scored rows terminal, all22 pages launched/completed with no real interruption/timeout. Retain DRAW inventory and raw results separately. |
| Regression | Every cold protected-main PASS key stays PASS on fresh old/new-pin observations; never reduce to a minimum count or selected DMA rows. |
| Native determinism | Full hash-key inventory from tests/cmake/hash_keys.cmake: game boot/movies, mapper/.saved variants, frame/PCM/pre-synthesis transitions, format and native states; actual equality across Linux/macOS/Windows x64/arm64. |
| Performance | Existing Cachegrind base_a/base_b/head measurements at60/360 frames for nesteroids, runner and M4_P256K_C256K; +1.00% threshold unchanged. No exception is added to tests/perf/allow.txt. Missing head/unstable base is not a pass. |
| Required CI | Current-head hosted lanes, hash-equality and perf/rollup results; no hosted trigger is authorized by the proposed local-only run. |
| Whitespace | Literal main...HEAD check remains red for inherited planning records; resolve its base/legacy handling separately without silently waiving it or corrupting patch fixtures. |

Hosted source enables FULL=ON on primary Linux x64 and runs its existing ci-build workflow, after preparing a cold protected-main baseline and this run's full-vector inventory. Other five platform lanes, the session job, hash-equality job (40-minute cap) and PR/control performance job (15-minute cap) remain separate evidence. This local authority proposal includes none of those remote triggers. Do not claim all-platform qualification from local equality or prior releases.

## Baseline provenance consequence

Historical baseline-metadata.txt remains behavior9 and points to the exact historic report. It was not relabeled13. accuracycoin.activation compares a full report's checksum and every canonical status/raw/frame value with that provenance. A genuinely changed fresh report will require a corresponding evidence-derived baseline refresh; the old activation comparison can therefore fail on the first authorized workflow. Record that honestly, update only from complete attributable observations, and do not rerun the campaign automatically to turn it green. No expected status was edited in advance.

## Completed bounded work

[Plan01](24-01-SUMMARY.md), [Plan02](24-02-SUMMARY.md), [Plan03](24-03-SUMMARY.md) and [DMA ledger](24-DMA-EVIDENCE.md) record independently corrected causes, pre/post source identities, wrong-model rejection, normal/observed parity, field/revision decisions and hardware/test/model limits. Test-only observer macros disappear in production preprocessing. No dependency, global state, scheduler, public cycle API, ROM pin, classifier, threshold or unlisted ROM bytes were added.

Fresh final owned closure: bus.unit, core.apu, core.controller and core.state_fork.entry all pass, followed by --dma. The earlier selected API/header/format/policy/doc checks and mutation controls pass; --short --stress retains all NROM/MMC3 OAM/DMC/IRQ witnesses, and --short state refusal passes the new countdown bound plus old/future atomic refusals. All12 native hashes normalize exactly to their pre-countdown payloads; every six-board size grows1, ticks/non-state rows/inventory stay unchanged. Broad source/consumer review has no active finding. These checks contain no campaign fixture and do not replace the table above.

`git diff --check main...HEAD` was run as required and fails with159 inherited planning whitespace diagnostics across50 files. Exactly the same normalized diagnostics already existed at89a042b before execution. Local main is older than the recorded remote-main ref. `git diff --check 89a042b..HEAD` passes. No unrelated archived planning files or meaningful patch-context whitespace was altered to hide the inherited failure. Required branch-wide hygiene is explicitly unresolved rather than reported green.

The [read-only acceptance review](24-04-REVIEW.md) has no active finding after correcting the local budget/authority distinctions and listing every campaign fixture consumer. The reviewer ran no tests or workflows. The [evidence scope amendment](24-04-AMENDMENT.md) records companion baseline consumers that can change only with complete, qualifying fresh observations after separate runtime authority.

## Handoff

Plan04 Task1's implementation/selection audit is recorded, with the inherited whitespace guard still unresolved. Task2's separate runtime authority remains absent; Task3 did not execute. No full workflow, AccuracyCoin ROM, campaign, push or PR ran during execution. All requirements and the phase remain incomplete.

**Owner acts next:** approve the concrete single local workflow/new39-minute window, or leave the stop in place. Ordinary language is sufficient. This is a new runtime decision, not another state-format approval.

After the decision: `$gsd-execute-phase 24`. Context may be cleared; read STATE.md and this file before continuing. Re-inspect command/selection/caps and recorded authority immediately before any execution.
