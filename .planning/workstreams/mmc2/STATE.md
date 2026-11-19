---
gsd_state_version: "1.0"
milestone: v11
milestone_name: MMC2 mainline compatibility port
status: executing
stopped_at: Phase 30 Plans 30-01 and 30-02 complete; Plan 30-03 remains
last_updated: "2026-11-19T20:49:00Z"
last_activity: 2026-11-19
last_activity_desc: Core port, three-client parity and state/provider compatibility checks completed
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
Plan: 30-02 complete; 30-03 remains
Status: Plans 30-01 and 30-02 are complete on the exact-main branch. Plan 30-03 owns final preflight, the one full workflow and delivery.
Last activity: 2026-11-19 — The bounded MMC2 core path, actual runner/libretro parity, provider contract and state pins passed their focused checks.

Progress: 0% — 0 of 1 phases complete; 2 of 3 plans complete

Planning: 3 plans, all reviewed; Plans 30-01 and 30-02 executed and summarized.

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
- Plans 30-01 and 30-02 are implemented and summarized in [30-01](phases/30-mmc2-mainline-compatibility-and-delivery/30-01-SUMMARY.md) and [30-02](phases/30-mmc2-mainline-compatibility-and-delivery/30-02-SUMMARY.md). Focused core tests, three-client parity, provider controls and state-format checks passed. Existing-board state payloads matched after a version/revision/CRC transform; other pinned output rows stayed unchanged. No full workflow has run.
- A fresh public LibFuturo check found no current NES consumer/playback acceptance; its main remains a synthetic-v1 proof and older rev9/state3 create/destroy seam. The 2026-11-19 publisher receipt is still missing and remains a separate gate.

### Pending Todos

Complete reviewed Plan 30-03: source-bound preflight, evidence binding, the single full workflow, required review and delivery gates.

### Blockers and Preserved Stops

- No v11 implementation blocker or unresolved scope decision has been established. Plans 30-01 and 30-02 are complete; the remaining acceptance and delivery work is owned by Plan 30-03. Preserve its single-full-workflow allowance and the project's 30-minute session limit.
- The five inherited v10 unverified-prohibition flags remain visible and unwaived; v10 WR-01 remains historical documentation debt. Technical checks may be assessed under the verification policy without converting an old flag into a waiver. Their completed disposition and limits are summarized in the [v10 handoff](V10-HANDOFF.md).
- Phase 29 CI is complete and must not be repeated for this port. Attempts 01–08 and halted recovery scopes remain terminal; no attempt-09, recovery-wrapper reuse or allowance renewal is authorized.
- Protected AccuracyCoin baselines, source/evidence bindings and acceptance rules cannot be re-anchored to hide a port mismatch. Diagnose a mismatch before long acceptance; a new material consequence requires its concrete scope decision.
- RM-017's publisher receipt, licensed profile/scenario and LibFuturo consumer gates remain separate. D-39/28-18, spent capture-3, untouched candidate-only capture-4, the retired APU route and older interrupted campaign retain their stops. This milestone supplies no substitute consumer or performance evidence.
- Project-owned dirty worktrees remain preserved. Cleanliness and ownership must be resolved before publishing through the existing release gate; do not discard unrelated work to pass it.

## Deferred Items

Inherited audit debt and future direction remain in the [product roadmap](../../PRODUCT-ROADMAP.md#unresolved-debt-and-delivered-tune-up). The [v10 handoff](V10-HANDOFF.md) records its completed outcome, transfer audit and preserved limits; detailed historical records remain in the originating workstream. Historical next commands in those records are superseded by this current state.

## Session Continuity

Last session: 2026-11-19
Stopped at: Plans 30-01 and 30-02 complete; Plan 30-03 remains
Next command: $gsd-autonomous --ws mmc2
Next actor: Owner, who starts a fresh bounded autonomous run with the command above.
Next action: Execute reviewed Plan 30-03 on the exact-main branch: preflight and bind evidence, then use the one approved full workflow and continue its review and delivery gates without per-step acknowledgments. Preserve exact stops and ask only for a real new scope, authority, cost/risk decision or unavailable human-only evidence.
Automatic continuation now: No — this session is handing off after completing two plans; the command starts a fresh bounded run for the remaining plan.
Resume file: .planning/workstreams/mmc2/V11-HANDOFF.md
Context may be cleared before the next command. The local full workflow, release, protected-evidence change and historical retry have not occurred.
