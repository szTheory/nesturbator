---
gsd_state_version: "1.0"
milestone: v11
milestone_name: MMC2 mainline compatibility port
status: executing
stopped_at: Plan 30-03 remediation; owner authorized exactly one follow-on full workflow conditional on amended plan/preflight review and passing gates
last_updated: "2026-11-19T21:41:26Z"
last_activity: 2026-11-19
last_activity_desc: Owner authorized one conditional follow-on run; amended plan/preflight is under independent review after clean header-gate evidence
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
Plan: 30-02 complete; 30-03 remediation and conditional acceptance
Status: Plans 30-01 and 30-02 are complete. Plan 30-03's initial full workflow failed in the build because two public-header smoke tests asserted state format 3. Both now assert format 4 and the clean targeted build and CTest pass. The owner authorized exactly one follow-on full workflow. Independent review of the amended plan/preflight passed; corrected-candidate gates are next.
Last activity: 2026-11-19 — Amended plan/preflight passed independent review; header clean build and exact CTest gate passed.

Progress: 0% — 0 of 1 phases complete; 2 of 3 plans complete

Planning: 3 plans reviewed; Plans 30-01 and 30-02 executed and summarized. The 30-03 scope and preflight amendment passed independent review; source-bound/focused gates are pending.

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
- Plan 30-03's one authorized `cmake --workflow --preset ci` ran on 2026-11-19 and failed during build in 0.971 seconds: `tests/header/header_c.c` and `tests/header/header_cxx.cpp` asserted state format 3 while the public contract is format 4. No full CTest, AccuracyCoin campaign or packaging stage ran. The attempt is terminal; elapsed time does not renew its allowance. See the [30-03 checkpoint](phases/30-mmc2-mainline-compatibility-and-delivery/30-03-SUMMARY.md).
- After the failed run, only those two assertions were changed to 4. `cmake --build build/ci --target header.c header.cxx` succeeded and the targeted `header.c`/`header.cxx` CTest set passed 2/2. The corrected files were outside the reviewed Plan 30-03 scope, so the prior fingerprint/preflight does not qualify a follow-on candidate. No new full workflow, PR, merge, package or release is authorized by this checkpoint.
- A fresh public LibFuturo check found no current NES consumer/playback acceptance; its main remains a synthetic-v1 proof and older rev9/state3 create/destroy seam. The 2026-11-19 publisher receipt is still missing and remains a separate gate.

### Pending Todos

Refresh the review digest to the independently reviewed product diff, run focused/preflight gates, freeze the corrected candidate, and use exactly one owner-authorized follow-on full workflow with the existing 1500-second cap. Continue only in this checkout; no further full attempt is authorized.

### Blockers and Preserved Stops

- The initial Plan 30-03 full workflow failed before CTest/campaign/package on two stale public-header test pins. The owner authorized exactly one follow-on attempt; independent plan/preflight review passed. If any corrected-candidate gate fails, preserve its evidence and stop without another full run. No full acceptance, package, PR or release gate has passed for the corrected candidate. Preserve the project's 30-minute session limit and all historical stops.
- The five inherited v10 unverified-prohibition flags remain visible and unwaived; v10 WR-01 remains historical documentation debt. Technical checks may be assessed under the verification policy without converting an old flag into a waiver. Their completed disposition and limits are summarized in the [v10 handoff](V10-HANDOFF.md).
- Phase 29 CI is complete and must not be repeated for this port. Attempts 01–08 and halted recovery scopes remain terminal; no attempt-09, recovery-wrapper reuse or allowance renewal is authorized.
- Protected AccuracyCoin baselines, source/evidence bindings and acceptance rules cannot be re-anchored to hide a port mismatch. Diagnose a mismatch before long acceptance; a new material consequence requires its concrete scope decision.
- RM-017's publisher receipt, licensed profile/scenario and LibFuturo consumer gates remain separate. D-39/28-18, spent capture-3, untouched candidate-only capture-4, the retired APU route and older interrupted campaign retain their stops. This milestone supplies no substitute consumer or performance evidence.
- Project-owned dirty worktrees remain preserved. Cleanliness and ownership must be resolved before publishing through the existing release gate; do not discard unrelated work to pass it.

## Deferred Items

Inherited audit debt and future direction remain in the [product roadmap](../../PRODUCT-ROADMAP.md#unresolved-debt-and-delivered-tune-up). The [v10 handoff](V10-HANDOFF.md) records its completed outcome, transfer audit and preserved limits; detailed historical records remain in the originating workstream. Historical next commands in those records are superseded by this current state.

## Session Continuity

Last session: 2026-11-19
Stopped at: Amended Plan 30-03/preflight passed independent review; refresh exact diff digest and run corrected-candidate preflight/focused gates.
Next command: Continue the current `$gsd-autonomous --ws mmc2` run after review.
Next actor: Orchestrator.
Next action: Commit the approved amendment, pin the reviewed exact candidate diff, rerun source-bound preflight and focused controls, then run the one authorized follow-on full workflow. Continue review/delivery only if exact-candidate acceptance passes.
Automatic continuation now: Yes — the owner authorized the conditional follow-on run; no more user input is needed unless a gate fails or scope changes.
Resume file: .planning/workstreams/mmc2/V11-HANDOFF.md
The original full workflow failed before tests or packages; exactly one follow-on is now authorized subject to amended gates. The header-test-only fix is committed in this same checkout. No package, PR or merge has occurred; preserve the failed attempt and do not start any further full workflow after the authorized one.
