---
gsd_state_version: "1.0"
milestone: v11
milestone_name: MMC2 mainline compatibility port
status: executing
stopped_at: Plan 30-03 local acceptance and code review complete; PR/main/release delivery gates remain
last_updated: "2026-11-19T22:42:03Z"
last_activity: 2026-11-19
last_activity_desc: Third full workflow passed all 702 tests, AccuracyCoin campaign and package step; final source review found no actionable issues
current_phase: 30
progress:
  total_phases: 1
  completed_phases: 0
  total_plans: 3
  completed_plans: 2
  percent: 67
---

# Project State

## Project Reference

See the [shared project](../../PROJECT.md), [v11 requirements](REQUIREMENTS.md),
[v11 roadmap](ROADMAP.md) and [v11 handoff](V11-HANDOFF.md).

**Core value:** Console behavior reproduced identically across platforms by a small MIT-licensed C core.
**Current focus:** Port the completed bounded MMC2 capability to live GitHub `main` using that baseline's compatibility contract and normal qualified delivery path.

## Current Position

Phase: 30 — MMC2 mainline compatibility and delivery
Plan: 30-02 complete; 30-03 local acceptance and code review passed, delivery gates pending
Status: The first full workflow failed on two stale header state-format pins. The second passed its campaign but failed seven compatibility checks before packaging. Those test contracts were fixed. The owner-authorized third workflow passed all 702 CTest entries, the AccuracyCoin campaign and activation checks, and generated all three local packages. The frozen commit and 57-input source fingerprint matched after the run. The final standard-depth source review found no blockers or warnings. No PR, merge or release has started; no further local full workflow is authorized by the one-run decision.
Last activity: 2026-11-19 — Local acceptance and code review passed; library, runner and libretro archives were inspected and hashed.

Progress: 0% — 0 of 1 phases complete; 2 of 3 plans complete

Planning: Plans 30-01 and 30-02 executed and summarized. Both Plan 30-03 amendments passed independent review. The final source-bound preflight and one newly authorized full workflow passed. Plan 30-03 delivery remains open.

## Performance Metrics

No v11 runtime evidence exists. v10's accepted workflow and all historical
attempt records remain bound to their original candidates; they are not
acceptance for the mainline port.

## Accumulated Context

### Decisions

- The owner selected the recommended scope and autonomous routine choices during new-milestone scoping. Continue Phase 30 through discussion, planning, implementation, useful automated verification, review and normal delivery after the next autonomous entry, subject to preserved stops and real authority/risk decisions.
- One cohesive Phase 30 owns MMC2-07 through MMC2-12. Continue project-wide numbering; Phase 28 remains v9-owned and Phase 29 remains the archived v10 outcome.
- The read-only opening review observed live GitHub `main` at `8923b66cca16739c5b402643798515cf5537ae22`, release 0.1.13, behavior revision 9, state format 3, ABI 1. Local `origin/main` was stale. Revalidate the live head before planning the source delta and before source work.
- For that reviewed baseline, the minimal compatibility result is behavior revision 10 and state format 4 with ABI 1. Re-evaluate if main moves; never copy v10's revision 18/format 7 blindly.
- Port only the owned MMC2 implementation, fixtures, necessary integration/compatibility changes and matching public documentation. Use a clean exact-main phase worktree, preserve unrelated dirty work, and carry the selected planning records deliberately. `--ws mmc2` chooses state relative to the current checkout; it does not switch Git checkouts.
- Retain the v10 exact profile and adopted secondary-reference latch model. Use direct markers, native continuation and actual video/PCM parity through all three deliverables. No title, silicon or broad mapper claim is selected.
- Use cheap source-bound consistency and focused checks before the standard full workflow. Preserve required coverage, test/resource caps and protected AccuracyCoin evidence. Inspect available CI timing for useful improvements; no new dependency, lane or general optimization is selected.
- Phase 30 discussion revalidated live `main` at `8923b66cca16739c5b402643798515cf5537ae22` with the expected rev9/state3/ABI1 compatibility contract; the isolated branch is pinned to that exact head. The selected minimal port remains rev10/state4/ABI1.
- Phase 30 plan review passed for three sequential plans in waves 1–3, with all six requirements covered, no human checkpoint, under-15-file plan scopes, and one full CI workflow after all implementation/evidence edits. See [30-01](phases/30-mmc2-mainline-compatibility-and-delivery/30-01-PLAN.md), [30-02](phases/30-mmc2-mainline-compatibility-and-delivery/30-02-PLAN.md), and [30-03](phases/30-mmc2-mainline-compatibility-and-delivery/30-03-PLAN.md).
- The current local CI preset does not enable the target's full AccuracyCoin campaign by default. Configure it on before running the unchanged standard workflow, and adapt only the exact source-closure admission needed for MMC2 while preserving historical evidence and same-run source fingerprinting.
- Plans 30-01 and 30-02 are implemented and summarized in [30-01](phases/30-mmc2-mainline-compatibility-and-delivery/30-01-SUMMARY.md) and [30-02](phases/30-mmc2-mainline-compatibility-and-delivery/30-02-SUMMARY.md). Focused core tests, three-client parity, provider controls and state-format checks passed. Existing-board state payloads matched after a version/revision/CRC transform; other pinned output rows stayed unchanged. No full workflow had run at the time Plan 30-02 completed.
- The initial Plan 30-03 `cmake --workflow --preset ci` ran on 2026-11-19 and failed during build in 0.971 seconds: `tests/header/header_c.c` and `tests/header/header_cxx.cpp` asserted state format 3 while the public contract is format 4. No full CTest or package stage ran. Evidence is retained in the [30-03 checkpoint](phases/30-mmc2-mainline-compatibility-and-delivery/30-03-SUMMARY.md).
- The owner authorized one follow-on full workflow. It completed 702 CTest entries in 388.8 seconds, with `accuracycoin.campaign` passing and seven compatibility tests failing: Phase 21 revision pin 9 vs 10; public declaration digest for old macros; and README/header MMC2 facts miscounted as duplicate MMC3 clauses, including one self-test expecting the old error. Those test-only causes were fixed; the eight directly affected checks passed 8/8 and the expanded focused set passed 24/24. The owner then authorized one third full workflow after independent plan/preflight review. It passed all 702 CTest entries and AccuracyCoin campaign/activation checks, then generated all three packages in 460.358 seconds. Frozen source commit `48f6f054ed363ea50b5da0bfe96f960230f673b7` and source fingerprint `d45734c098d790da3aad63eb25d1cba37e388e62b7e7f5d00aea5ec7525bbb01` (57 inputs) matched before and after. Package hashes and contents are recorded in the Plan 30-03 summary and local ignored manifest. Final source review covered the 27-file product scope with no blockers or warnings. The one-run authorization is spent; do not retry local full CI without new owner authorization.
- A fresh public LibFuturo check found no current NES consumer/playback acceptance; its main remains a synthetic-v1 proof and older rev9/state3 create/destroy seam. The 2026-11-19 publisher receipt is still missing and remains a separate gate.

### Pending Todos

Read-only triage found no open MMC2 or mapper-9 PR. Provider PR #68 is open and unmergeable on a separate branch; preserve it unchanged. Revalidate live `main`, prepare/open the reviewed Conventional Commit PR for this exact candidate, and verify its required CI and eventual resulting-main/release identities. Do not run another local full workflow under the spent one-run authorization. Prepare all evidence before asking the owner to approve any irreversible merge or publication step.

### Blockers and Preserved Stops

- The first two full workflow attempts remain terminal failures and their causes are documented. The one newly authorized third workflow passed all 702 tests, the campaign and package step; source and fingerprint were unchanged. That one-run authorization is spent and the run is terminal. Do not start another local full workflow without new explicit owner authorization and independent plan/preflight review. No PR, merge or release has started. Preserve the project's 30-minute limit and all historical stops.
- The five inherited v10 unverified-prohibition flags remain visible and unwaived; v10 WR-01 remains historical documentation debt. Technical checks may be assessed under the verification policy without converting an old flag into a waiver. Their completed disposition and limits are summarized in the [v10 handoff](V10-HANDOFF.md).
- Phase 29 CI is complete and must not be repeated for this port. Attempts 01–08 and halted recovery scopes remain terminal; no attempt-09, recovery-wrapper reuse or allowance renewal is authorized.
- Protected AccuracyCoin baselines, source/evidence bindings and acceptance rules cannot be re-anchored to hide a port mismatch. Diagnose a mismatch before long acceptance; a new material consequence requires its concrete scope decision.
- RM-017's publisher receipt, licensed profile/scenario and LibFuturo consumer gates remain separate. D-39/28-18, spent capture-3, untouched candidate-only capture-4, the retired APU route and older interrupted campaign retain their stops. This milestone supplies no substitute consumer or performance evidence.
- Project-owned dirty worktrees remain preserved. Cleanliness and ownership must be resolved before publishing through the existing release gate; do not discard unrelated work to pass it.

## Deferred Items

Inherited audit debt and future direction remain in the [product roadmap](../../PRODUCT-ROADMAP.md#unresolved-debt-and-delivered-tune-up). The [v10 handoff](V10-HANDOFF.md) records its completed outcome, transfer audit and preserved limits; detailed historical records remain in the originating workstream. Historical next commands in those records are superseded by this current state.

## Session Continuity

Last session: 2026-11-19
Stopped at: Local acceptance and standard-depth source review passed; delivery to PR/main/release remains.
Next command: Continue the current `$gsd-autonomous --ws mmc2` run through the existing PR/main/release gates.
Next actor: Orchestrator.
Next action: Triage the open related provider PR read-only, recheck live main, prepare and open the exact reviewed feature PR, and verify current-head CI. Ask the owner only after the merge/publication state is concrete and reviewable.
Automatic continuation now: Yes through PR preparation and required checks; pause for owner approval before merge or publication.
Resume file: .planning/workstreams/mmc2/V11-HANDOFF.md
The first full workflow failed during build; the second failed seven compatibility checks before packaging. Their causes are fixed. The owner-authorized third run passed the full workflow, including 702 tests, campaign, and three packages. Source commit and fingerprint matched pre/post-run; the independent 27-file review had no blockers or warnings. The one-run authorization is spent, and no PR/merge/release has started. Continue delivery in the same Phase 30 checkout; do not create another clone or worktree.
