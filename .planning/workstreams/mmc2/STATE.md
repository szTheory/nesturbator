---
gsd_state_version: "1.0"
milestone: v11
milestone_name: MMC2 mainline compatibility port
status: blocked
stopped_at: Plan 30-03's single full workflow failed during build; targeted header fix passed, but another full run needs plan/preflight review and owner authorization
last_updated: "2026-11-19T21:23:24Z"
last_activity: 2026-11-19
last_activity_desc: Full workflow stopped on stale header format pins; the narrow C/C++ header test fix passed targeted checks
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
Plan: 30-02 complete; 30-03 blocked at acceptance
Status: Plans 30-01 and 30-02 are complete. Plan 30-03's sole authorized full workflow failed in the build because two C/C++ public-header smoke tests still asserted state format 3. The assertions now match format 4 and both targeted tests pass. The plan's file scope and preflight omitted these tests, so a new full run requires a reviewed plan/preflight update and explicit owner authorization.
Last activity: 2026-11-19 — Full workflow stopped before CTest or packaging; the two stale header assertions were corrected and their targeted checks passed.

Progress: 0% — 0 of 1 phases complete; 2 of 3 plans complete

Planning: 3 plans reviewed; Plans 30-01 and 30-02 executed and summarized. Plan 30-03 needs its source scope and cheap preflight coverage amended before any additional full acceptance attempt.

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

Await owner disposition on whether to amend/review Plan 30-03 and its preflight/build coverage for both header test files, then authorize exactly one additional full workflow with the existing 1500-second cap. If authorized, continue only in this checkout. If not, leave Plan 30-03 blocked and the phase incomplete.

### Blockers and Preserved Stops

- Plan 30-03 is blocked: its one full-workflow allowance was consumed by a build failure on two stale public-header test pins. The pins have a narrow targeted fix, but the plan/preflight scope omission must be corrected and independently reviewed, and an additional full run needs explicit owner authorization. No CTest/campaign/package/PR/release gate has passed for the corrected candidate. Preserve the project's 30-minute session limit and all historical stops.
- The five inherited v10 unverified-prohibition flags remain visible and unwaived; v10 WR-01 remains historical documentation debt. Technical checks may be assessed under the verification policy without converting an old flag into a waiver. Their completed disposition and limits are summarized in the [v10 handoff](V10-HANDOFF.md).
- Phase 29 CI is complete and must not be repeated for this port. Attempts 01–08 and halted recovery scopes remain terminal; no attempt-09, recovery-wrapper reuse or allowance renewal is authorized.
- Protected AccuracyCoin baselines, source/evidence bindings and acceptance rules cannot be re-anchored to hide a port mismatch. Diagnose a mismatch before long acceptance; a new material consequence requires its concrete scope decision.
- RM-017's publisher receipt, licensed profile/scenario and LibFuturo consumer gates remain separate. D-39/28-18, spent capture-3, untouched candidate-only capture-4, the retired APU route and older interrupted campaign retain their stops. This milestone supplies no substitute consumer or performance evidence.
- Project-owned dirty worktrees remain preserved. Cleanliness and ownership must be resolved before publishing through the existing release gate; do not discard unrelated work to pass it.

## Deferred Items

Inherited audit debt and future direction remain in the [product roadmap](../../PRODUCT-ROADMAP.md#unresolved-debt-and-delivered-tune-up). The [v10 handoff](V10-HANDOFF.md) records its completed outcome, transfer audit and preserved limits; detailed historical records remain in the originating workstream. Historical next commands in those records are superseded by this current state.

## Session Continuity

Last session: 2026-11-19
Stopped at: Plan 30-03's sole full workflow failed in build; the narrow header-test fix passed targeted checks.
Next command: None until owner disposition; if an additional full acceptance is authorized, resume with `$gsd-autonomous --ws mmc2` from this same checkout.
Next actor: Owner. Decide whether to authorize a reviewed Plan 30-03/preflight scope update and exactly one additional full workflow capped at 1500 seconds.
Next action: If authorized, add both header smoke-test files to the plan scope and cheap preflight/build, obtain independent review, rerun the source-bound preflight for the corrected candidate, then spend the newly authorized single full workflow. Continue review/delivery only if exact-candidate acceptance passes.
Automatic continuation now: No — the original one-run allowance is spent. Work is waiting for the owner on a material acceptance decision.
Resume file: .planning/workstreams/mmc2/V11-HANDOFF.md
The original full workflow failed before tests or packages. The header-test-only fix is checked in this same checkout; no retry, release, protected-evidence change, PR or merge has occurred. Context may be cleared while waiting for owner disposition.
