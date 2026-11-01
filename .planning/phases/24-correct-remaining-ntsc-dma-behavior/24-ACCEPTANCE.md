# Phase24 acceptance — corrected candidate awaits fresh acceptance

## Actual outcome

The owner explicitly authorized one local `cmake --workflow --preset ci`
invocation with a39-minute overall ceiling and unchanged inner limits. It
completed normally in444.60seconds with exit8. Its full22-page campaign
retained every119 previous current-pin PASS key, but bus-conflict, explicit-
abort and implicit-abort still failed. The other selected landing/OAM rows
passed. Two historical-baseline checks and one stale sprite fixture also
failed. Packaging did not run. [Actual attributed result](24-CI-RUN.md).

The sprite fixture is repaired. The three remaining core divergences now
have source-shaped owned regressions and repairs, including full64-entry
abort keys, focused byte/bus/audio checks, distinct wrong models and native
continuation. Current candidate is behavior14 / native format6 / ABI1.
[Correction record](24-04-CORRECTIONS.md). These are owned results, not a
fresh behavior14 ROM observation. Phase24 and all four requirements remain
incomplete. No second full workflow/campaign or hosted trigger ran.

## Authority and next concrete decision

Approval friction is resolved in standing instructions: task/phase approval
includes required local engineering, builds/tests/CI, reviews/fixes and bounded
retests. Runtime length alone and each compatibility implementation slice do
not require another prompt. [Applied lesson](24-APPROVAL-LESSONS.md).

A separate, specific limit still exists here: the owner approved exactly one
full workflow, now consumed. Do not infer another AccuracyCoin execution,
campaign, hosted trigger, push/PR, merge or release from the ordinary repair
authority. The next useful acceptance action is one fresh local full workflow
on the committed corrected candidate, preserving the previous39-minute
ceiling and all current inner caps. This is a recommendation, not yet a new
authorization. The reason for a decision is that explicit one-invocation
limit, not concern about how long ordinary tests take. Keeping the stop
requires no response.

## Latest checkpoint decision

On 2026-10-31 the owner chose to keep the stop. This preserves the consumed
one-run limit and grants no new local workflow, campaign, hosted, push or PR
authority. No new acceptance command ran. No action is needed while the stop
remains in place; a later renewal must explicitly cover the listed local
command, selection and unchanged limits.

```sh
cmake --workflow --preset ci
```

Before any newly authorized execution, record the candidate identity and
read-only test/fixture selection. Preserve prior logs/reports separately.
Existing attributable results, including the completed report, are copied
under ignored `build/phase24-acceptance/20261031T223038Z/observed`; its report
digest was verified against24-CI-RUN.md. Never overwrite that history with
a later source identity. Stop on real interruption or overall expiry, target
cleanup to this invocation's own processes and preserve partial evidence;
no automatic campaign retry. Expected simulated-timeout controls do not
relax a real stop.

After a complete qualifying observation, evidence-derived canonical
scoreboard/measurements/provenance updates and bounded validator-only retests
are routine follow-through. Do not relaunch a campaign implicitly through
a CTest fixture closure to make activation green. Do not update expected
statuses or claim qualification before the actual observations exist.

## Selection and unchanged limits

The checked-in [selection inventory](24-CI-SELECTION.tsv) is a historical
read-only `FULL=ON` CTest inventory for source7f31579, not a generated
behavior14 inventory. Test registration files have not changed since that
source; the `ci` workflow still enables `NESTURBATOR_ACCURACYCOIN_FULL=ON`.
However, no matching current-candidate `FULL=ON` CTest tree is available: the
existing `build/phase24-owned` tree is Debug with `FULL=OFF` and
`VECTORS_FULL=OFF`, and the archived prior full-run directory has only its
old cache, not a generated CTest inventory. Its read-only JSON contains706
tests. The exact bounded selection resolves to `bus.unit`, `core.apu`,
`core.controller` and `core.state_fork.entry`, with no setup/required fixture
dependencies; `accuracycoin.campaign` is absent there. This proves only the
bounded owned closure. The behavior14 full selection is **source-derived**
from the current workflow and `tests/CMakeLists.txt`: those test registrations
are unchanged since the recorded 707-entry inventory. Its current generated
inventory remains unavailable and was not claimed as executed. The current
workflow enables the campaign fixture, so the full selection would run it;
this is why the stopped scope cannot be inferred from the bounded selection.

| Consumer | Required fixture | Setup in FULL=ON CTest selection |
|---|---|---|
| accuracycoin.activation / nmi_targets / v5_targets | accuracycoin_full_report | accuracycoin.campaign |
| accuracycoin.protected / accuracy.scoreboard.no_lost_pass / regression_controls | accuracycoin_protected_observations | accuracycoin.protected_observation |
| accuracycoin.report_fresh | accuracycoin_report_fresh | accuracycoin.report_scratch |

Campaign pin6bee919529289c555803b612b8c9e4f7eaaeeabb and ROM digest remain
unchanged. Existing bounds:32 boot frames;5 navigation frames per page;1800
execution frames;15-second page and10-second collection caps;370-second
outer CTest limit. Complete evidence has22 launched/completed pages,146
terminal scored rows and5 DRAW rows, with raw/attempt provenance. Protected
setup keeps its30-second page /760-second outer caps. Full CI uses four CTest
workers. Local session source-root/controls/readiness caps remain120/900/60
seconds. Do not disable suites, widen limits or switch FULL=OFF.

Local authority environment variables are unset, so protected observations
use the checkout scoreboard-main fallback. This does not establish an
independently prepared cold protected-main authority. Hosted preparation
derives necessary old/new-pin pages from that baseline; local catalog counts
are not a substitute. [Historical key catalog](24-PROTECTED-PASS-KEYS.tsv).

## Remaining acceptance evidence

| Requirement | Still required |
|---|---|
| Selected rows | Fresh individual PASS for all five at the unchanged pin and classifier; both historical implicit outcomes remain accepted |
| Completeness | Attributable current source/build/ROM/report; every scored row terminal; no real interruption/timeout |
| Regression | Every cold protected-main PASS retained, including the three named DMA neighbors; both pins as applicable |
| Determinism | Actual Linux/macOS/Windows x64/arm64 equality of complete frame/PCM/transition/native hash inventory |
| Performance | Existing Cachegrind base_a/base_b/head checks at60/360 frames and unchanged+1% threshold; no exception added |
| Required CI | Current-head hosted lanes, full-vector inventory, session jobs, hash equality, perf and rollup |
| Whitespace | Resolve inherited branch-wide planning diagnostics without corrupting archived patch fixtures or silently waiving the guard |

Historical baseline-metadata.txt remains behavior9 and identifies its exact
report. It was not relabeled13/14. Input-fingerprint and activation checks
correctly reject stale identity; refresh only from qualifying actual evidence.

The prior Task1 review recorded159 inherited `main...HEAD` diagnostics and41
against cached `origin/main`; those are historical counts. At the current
phase branch head, the plan's `git diff --check main...HEAD` recheck exits2
with133 trailing-whitespace findings in the committed branch diff. The scoped
STATE/acceptance edits pass `git diff --check`. No broad cleanup, network
refresh or bulk historical rewrite was performed.

## Bounded Plan24-04 review

The current correction source is `a12a79a2cbf0d872150ada9c91754b5345ded38a`
(behavior14 / format6 / ABI1). Only the plan's bounded command ran:

```sh
cmake --build build/phase24-owned --target bus.unit core.apu core.controller core.state_fork && ctest --test-dir build/phase24-owned -R '^(bus\.unit|core\.apu|core\.controller|core\.state_fork\.entry)$' --no-tests=error --timeout 30 --output-on-failure && build/phase24-owned/tests/core.state_fork --dma && git diff --check main...HEAD
```

All four targets built, the four selected CTest entries passed in0.82s, and
`core.state_fork --dma` exited successfully. The final whitespace command
returned2 with159 diagnostics already recorded for the inherited branch-wide
planning diff; no broad cleanup was made. Detailed row identities and current
binary SHA256 values are in [24-DMA-EVIDENCE.md](24-DMA-EVIDENCE.md).
The bounded checks do not execute the stopped campaign fixture and do not
establish behavior14 ROM acceptance.
The Task1 evidence/state edits were committed as `661a45a` after the default
sandbox denied Git metadata writes. The scoped commit includes only this
acceptance record, the DMA evidence ledger and STATE; no unrelated path was
staged.

The five selected rows still require fresh individual PASS observations at
pin `6bee919529289c555803b612b8c9e4f7eaaeeabb`, with complete terminal raw
observations and source/build/ROM/report provenance. The complete protected
PASS set (including the three DMA neighbors), fresh cold protected-main
authority, cross-platform frame/PCM/transition hashes, unchanged Cachegrind
`+1%` evidence and current hosted lanes are also missing. No scoreboard or
measurement was changed. The one authorized local workflow is consumed; the
full behavior14 `FULL=ON` selection remains stopped.

## Handoff

The agent completes the owned correction/check/review/commit work before
stopping. No action is needed to keep the stop. If the owner later renews the
exact local acceptance scope, they act next. Exact next OpenGSD command:
`$gsd-execute-phase 24`. Context may be cleared; read the current STATE and
this acceptance record before resuming. The specific run limit stays active
until a clear decision changes it.
