---
gsd_state_version: "1.0"
milestone: v11
milestone_name: MMC2 mainline compatibility port
status: planning
stopped_at: Phase 30 discussion complete; planning next
last_updated: "2026-11-19T19:14:00Z"
last_activity: 2026-11-19
last_activity_desc: Phase 30 discussion completed; planning is next in the active autonomous run
current_phase: 30
progress:
  total_phases: 1
  completed_phases: 0
  total_plans: 0
  completed_plans: 0
  percent: 0
---

# Project State

## Project Reference

See the [shared project](../../PROJECT.md), [v11 requirements](REQUIREMENTS.md),
[v11 roadmap](ROADMAP.md) and [v11 handoff](V11-HANDOFF.md).

**Core value:** Console behavior reproduced identically across platforms by a small MIT-licensed C core.
**Current focus:** Port the completed bounded MMC2 capability to live GitHub `main` using that baseline's compatibility contract and normal qualified delivery path.

## Current Position

Phase: 30 — MMC2 mainline compatibility and delivery
Plan: Not yet planned
Status: Phase 30 discussion complete; planning next
Last activity: 2026-11-19 — Revalidated exact mainline and consumer evidence; recorded Phase 30 decisions. No source implementation or acceptance run has started.

Progress: 0% — 0 of 1 phases complete

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
- The current local CI preset does not enable the target's full AccuracyCoin campaign by default. Configure it on before running the unchanged standard workflow, and adapt only the exact source-closure admission needed for MMC2 while preserving historical evidence and same-run source fingerprinting.
- A fresh public LibFuturo check found no current NES consumer/playback acceptance; its main remains a synthetic-v1 proof and older rev9/state3 create/destroy seam. The 2026-11-19 publisher receipt is still missing and remains a separate gate.

### Pending Todos

No new todo is created by the roadmap. Phase 30 discussion is recorded in [30-CONTEXT.md](phases/30-mmc2-mainline-compatibility-and-delivery/30-CONTEXT.md) and [30-DISCUSSION-LOG.md](phases/30-mmc2-mainline-compatibility-and-delivery/30-DISCUSSION-LOG.md). Planning is the next internal step.

### Blockers and Preserved Stops

- No v11 implementation blocker or unresolved owner decision has been established. The exact-main phase worktree is active and pinned correctly; proceed to planning, then source work under its source-bound preflight.
- The five inherited v10 unverified-prohibition flags remain visible and unwaived; v10 WR-01 remains historical documentation debt. Technical checks may be assessed under the verification policy without converting an old flag into a waiver. Their completed disposition and limits are summarized in the [v10 handoff](V10-HANDOFF.md).
- Phase 29 CI is complete and must not be repeated for this port. Attempts 01–08 and halted recovery scopes remain terminal; no attempt-09, recovery-wrapper reuse or allowance renewal is authorized.
- Protected AccuracyCoin baselines, source/evidence bindings and acceptance rules cannot be re-anchored to hide a port mismatch. Diagnose a mismatch before long acceptance; a new material consequence requires its concrete scope decision.
- RM-017's publisher receipt, licensed profile/scenario and LibFuturo consumer gates remain separate. D-39/28-18, spent capture-3, untouched candidate-only capture-4, the retired APU route and older interrupted campaign retain their stops. This milestone supplies no substitute consumer or performance evidence.
- Project-owned dirty worktrees remain preserved. Cleanliness and ownership must be resolved before publishing through the existing release gate; do not discard unrelated work to pass it.

## Deferred Items

Inherited audit debt and future direction remain in the [product roadmap](../../PRODUCT-ROADMAP.md#unresolved-debt-and-delivered-tune-up). The [v10 handoff](V10-HANDOFF.md) records its completed outcome, transfer audit and preserved limits; detailed historical records remain in the originating workstream. Historical next commands in those records are superseded by this current state.

## Session Continuity

Last session: 2026-11-19
Stopped at: Phase 30 discussion complete; autonomous run continuing through planning
Next command: $gsd-autonomous --ws mmc2
Next actor: The active orchestrator; if this run is interrupted, the owner starts the same autonomous entry and it resumes from planning.
Next action: Plan Phase 30 on the exact-main branch, then continue implementation, focused checks, full acceptance, review and delivery without per-step acknowledgments. Preserve exact stops and ask only for a real new scope, authority, cost/risk decision or unavailable human-only evidence.
Automatic continuation now: Yes — the autonomous run is active; no user action is needed.
Resume file: .planning/workstreams/mmc2/V11-HANDOFF.md
If interrupted, context may be cleared before rerunning the exact autonomous command. Phase 30 discussion completed; no implementation, candidate CI, release, protected-evidence change or historical retry has occurred yet.
