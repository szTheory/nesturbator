---
gsd_state_version: "1.0"
milestone: v7
milestone_name: Remaining DMA accuracy failures
current_phase: 24
current_phase_name: Correct remaining NTSC DMA behavior
status: executing
stopped_at: "Phase 24-01 Task 1: awaiting concrete native-state format decision"
last_updated: "2026-10-31T19:32:23Z"
last_activity: 2026-10-31
last_activity_desc: Owned DMA traces retained; native-state change gated before implementation; no behavior correction
state_head: 89a042ba742ef24679666deac8606ef8e54e67a4
progress:
  total_phases: 1
  completed_phases: 0
  total_plans: 4
  completed_plans: 0
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated for v7 on 2026-10-31, and AGENTS.md.
**Core value:** Games behave as on the console, identically across platforms, from a small MIT C library.
**Current focus:** Phase 24-01 tracer — awaiting native-state decision before the first correction

## Current Position

Phase: 24 (Correct remaining NTSC DMA behavior) — CHECKPOINT
Plan: 1 of 4 — Task 1 incomplete
Status: Waiting for owner decision on concrete native-state incompatibility
Last activity: 2026-10-31 — Owned traces localized a stopped DMC silent bit clock; bounded default bus/APU checks pass. No behavior correction or schema mutation.
Progress: [░░░░░░░░░░] 0%

## Next Action

**Waiting for the owner.** Plan 24-01 reached its required pre-mutation native-state checkpoint: the DMC output counter freezes during silence, while the source model needs an eight-clock silent phase and a separate silence flag. Recommended decision: approve the explicit silence byte and continuous bit-count meaning, state format 4 and behavior revision 10, with native continuation, old-format refusal tests and matching documentation. Existing format-3 native states would be refused; no converter is proposed. [Concrete field diff, alternatives and evidence](phases/24-correct-remaining-ntsc-dma-behavior/24-01-CHECKPOINT.md).

No behavior correction, serializer change, completed plan, or requirement acceptance is claimed. Owned diagnostic hooks/fixtures and partial row evidence are retained; default bus/APU regressions pass, while explicit silence/landing diagnostics intentionally fail. No full workflow, AccuracyCoin ROM, campaign, push or PR ran. The existing campaign stop remains active.

After the explicit decision, next command: `$gsd-execute-phase 24`. It resumes the incomplete first plan, then the dependent plans in order, while preserving all state/campaign gates. Context may be cleared; read this file and [24-EXECUTION-RESUME.md](phases/24-correct-remaining-ntsc-dma-behavior/24-EXECUTION-RESUME.md) first.

## Accumulated Context

### Decisions

- v7 covers only the five selected failing NTSC DMA rows at the existing pin and declared RP2A03G target; the three passing DMA rows and every other protected PASS key remain guards.
- One phase owns all four requirements: independently justified corrections, owned wrong-model fixtures, regression preservation and state/documentation consequences belong together. Shared causes require traces.
- Preserve integer-only instance state, clean-room sources and the library/runner/libretro deliverables. Review format/revision impact if in-flight fields change; tests and README/public-header comments accompany each behavior change.
- Existing cold replay and native checkpoint routes remain separate; exact-build support differs from loader checks. Declared provenance is not authentication; repeatability and native/adapter equality do not establish host or hardware correctness.

### Pending Todos

The three deferred property/mutation/signal proposals remain unlinked to v7; their records are preserved below. No new todo scope is approved.

### Blockers/Concerns

- **Plan 24-01 native-state checkpoint:** awaiting the owner’s decision on the proposed DMC silence byte/bit-count meaning and format-4 refusal of existing format-3 states. Do not mutate layout or meaning before that decision; see the linked checkpoint.

- **Active runtime stop:** The interrupted full-CI/AccuracyCoin campaign outcome is unknown. Do not rerun full CI or a campaign, including through automatic verification or a push/PR side effect, without separate owner authorization. Planning and milestone approval do not renew that authority.
- **Phase 24 completion gate:** Project CI remains required. If its configuration schedules `accuracycoin.campaign`, stop execution before that command; record selected tests, missing evidence and required authorization. Keep phase/requirements incomplete; do not disable or weaken gates or substitute bounded passes for full acceptance.
- Phase 23 document-test timeouts/wall-time jumps, five heuristic edges and Phase 22/23 judgment-tier prohibitions remain unresolved/flagged. No new timeout or restart is justified. Windows/PowerShell, host recovery and cross-build compatibility remain unqualified; commercial reports remain unclassified and daily progress untouched.
- Collector Git/filesystem-mode, ignored-fixture/relative-root and duplicate-validator debt remain open. Historical attribution/revision advisories, Phase 19 cost/citation/early-stage coverage, real-campaign minimization controls and macOS nofp limits remain in the [v5 audit](milestones/v5-MILESTONE-AUDIT.md) and [product ledger](PRODUCT-ROADMAP.md#unresolved-debt-and-delivered-tune-up). New evidence cannot inherit old-run qualification.
- WINDOWS entry 18 was reconciled from fixed existing evidence; the original refused append remains historical. Physical exact-timestamp precedence is unmeasured. Historic Phase 16 safety stops, runtime windows and thresholds remain unchanged; no closeout or approval waives them.

## Delivery and Carried Evidence

v6 is archived: both phases, all ten requirements and three flows passed; its [audit](milestones/v6-MILESTONE-AUDIT.md) and [closeout](milestones/v6-CLOSEOUT.md) retain non-blocking debt. Release PR #56 later published v0.1.12 with 18 archives and SHA256SUMS; milestone completion and package publication remain separate.
Quick tasks 261026-i8z, 261029-sdj and 261031-ecd remain delivered. [261031-fq6](quick/261031-fq6-skip-the-redundant-main-push-ci-run-for-/261031-fq6-SUMMARY.md) merged; PR/main-push CI passed, while its exact generated Release Please main-push skip event remains unobserved. No user action is needed for that completed task.
Pre-existing local edits to AGENTS.md, config, continue and quick-task records remain outside this roadmap's scope; they were not reverted.

## Deferred Items

Items acknowledged and deferred at milestone close, most recent first:

| Category | Item | Status | Deferred At | Milestone |
|----------|------|--------|-------------|-----------|
| debug_sessions | entry-review-inhibition | verifying | 2026-10-30 | v5 |
| todos | 2026-10-26-add-reproducible-owned-property-tests.md | presence-only | 2026-10-30 | v5 |
| todos | 2026-10-26-evaluate-scoped-nightly-mutation-testing.md | presence-only | 2026-10-30 | v5 |
| todos | 2026-10-26-make-runner-signal-checks-deterministic.md | presence-only | 2026-10-30 | v5 |
| deferred_items | 21/deferred-items.md: `.planning/WINDOWS.md` could not accept the prior plan's deviation entry: its frontmatter counts 18 entries, while the authoritative fenced JSON and table contain 17 records (table row 18 has no JSON entry). `gsd_run windows append` refused the update. The pre-existing ledger consistency issue remains deferred to its owning planning-health work. | acknowledged | 2026-10-30 | v5 |
| deferred_items | 21/deferred-items.md: macOS arm64 `nofp` undefined-symbol allowlist  - **Reproduced at:** final candidate `1d27a0f12542a235d8d79b9c97591168fbd77d8e`. - **Command:** `cmake --workflow --preset nofp` on macOS arm64. - **Result:** `abi.float_scan`, `abi.float_scan.selftest`, `abi.global_symbols`, and `abi.float_fixture` passed. `abi.undefined_symbols` rejected the Apple stack-protector references `stack_chk_fail` and `stack_chk_guard`. - **Scope:** the configured hosted `nofp` matrix uses Linux x64 and Linux arm64. This local macOS failure does not establish either hosted result. Keep the allowlist gate intact and require the actual Linux CI results before treating the `nofp` requirement as verified.  The initial local `nofp` run also reported the decimal `2.3` in the state validation comment. That source citation was changed to integer-only section wording in `1d27a0f`; the final source scan passes. The scanner and its hard gate were not weakened. | acknowledged | 2026-10-30 | v5 |
| debug_sessions | phase16-cleanup-fixture | verifying | 2026-10-24 | v4 |
| debug_sessions | phase16-interruption | investigating | 2026-10-24 | v4 |
| deferred_items | 16-host-evidence-and-roadmap/deferred-items.md: local macOS nofp reports Apple-toolchain stack_chk_fail/stack_chk_guard symbols | acknowledged | 2026-10-24 | v4 |

Seven seed acknowledgements from earlier closes remain suppressed by their own `audit_acknowledged` markers.

## Performance Metrics

v7: first plan partially investigated; zero completed plans. Diagnostic evidence is retained at the native-state checkpoint. v6 completed seven plans; detailed timing remains in the [archived phase summaries](milestones/v6-phases/) and [closeout](milestones/v6-CLOSEOUT.md).

## Session Continuity

**Last session:** 2026-10-31T19:32:23Z
**Stopped at:** Phase 24-01 Task 1 — required native-state decision, before behavior/schema mutation
**Next actor:** Owner decides the concrete compatibility consequence; execution waits.
**Next command after decision:** `$gsd-execute-phase 24`
**Resume file:** .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-EXECUTION-RESUME.md
