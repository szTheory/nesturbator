---
gsd_state_version: "1.0"
milestone: v7
milestone_name: Remaining DMA accuracy failures
current_phase: 24
current_phase_name: Correct remaining NTSC DMA behavior
status: executing
stopped_at: Plan24-04 Task1 bounded review complete; Task2 awaits separate authority for the consumed full-workflow scope
last_updated: "2026-10-31T23:53:01Z"
last_activity: 2026-10-31
last_activity_desc: Plan24-04 Task1 evidence and bounded checks complete; commit blocked by read-only Git metadata
state_head: b9d4256a5a17bd8f519317704f06e5be0ddc7bd3
progress:
  total_phases: 1
  completed_phases: 0
  total_plans: 4
  completed_plans: 3
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated for v7 on 2026-10-31, and AGENTS.md.
**Core value:** Games behave as on the console, identically across platforms, from a small MIT C library.
**Current focus:** Behavior14 bounded evidence is reconciled; fresh acceptance remains gated

## Current Position

Phase: 24 (Correct remaining NTSC DMA behavior) — acceptance checkpoint
Plan: 4 of 4 — Task1 complete; Task2 awaits the owner's separate runtime decision
Status: Phase and requirements remain incomplete
Last activity: 2026-10-31 — Four bounded CTest entries and native `--dma` passed; commit is blocked by `.git` write denial; `git diff --check main...HEAD` reported159 inherited diagnostics
Progress: [███████░░░] 75% (plans; phase acceptance pending)

## Next Action

**Owner acts next only if deciding whether to renew the exact full-workflow scope.** The prior one-run authority was consumed by a complete444.6-second `cmake --workflow --preset ci` run with exit8; Task1 now has bounded owned checks, but no behavior14 ROM result. Its evidence edits are uncommitted because this sandbox denies Git metadata writes (`.git/index.lock`: `Operation not permitted`). If renewed, the proposed command is exactly `cmake --workflow --preset ci`, with the previous39-minute ceiling, every existing inner limit, no automatic retry, and no hosted/push/PR trigger. Keeping the stop needs no response. The current behavior14 full CTest inventory is source-derived because no matching generated `FULL=ON` tree exists. [Concrete selection, limits, evidence and gaps](phases/24-correct-remaining-ntsc-dma-behavior/24-ACCEPTANCE.md).

No ordinary implementation acknowledgment is needed. Task approval now durably covers local engineering, builds/tests/CI, reviews/fixes/bounded retests and approved compatibility follow-through. Those changes are recorded globally and in repository instructions. The additive `dev-check` preset provides fast owned feedback; existing CI objects and the complete707-test name/timeout/fixture/label inventory are unchanged. [Measured efficiency decisions](phases/24-correct-remaining-ntsc-dma-behavior/24-CI-EFFICIENCY.md).

Core correction a12a79a is behavior14 / native format6 / ABI1. All64 owned abort-key entries, bus matrix/status effects, focused byte/DAC wrong-model tests, genuine imminent/armed/fresh-buffer native cuts, short stress witnesses, current revision/document controls and homebrew hash consumers pass. Release builds with warnings as errors; no new saved field or dependency was needed. [Correction evidence](phases/24-correct-remaining-ntsc-dma-behavior/24-04-CORRECTIONS.md).

The actual behavior13 campaign retained all119 previous current-pin PASS keys but still failed bus-conflict and both abort rows. Do not claim fresh behavior14 ROM acceptance or replace canonical baseline metadata from owned tests. Required protected-main/hosted/platform/performance evidence and inherited branch whitespace remain unresolved. No second campaign, push/PR, hosted trigger, merge or release ran. [Actual workflow result](phases/24-correct-remaining-ntsc-dma-behavior/24-CI-RUN.md).

**Next OpenGSD command:** `$gsd-execute-phase 24` — resumes at Task2's blocking decision and preserves the stop unless the owner explicitly renews this command and scope. Context may be cleared; read this file and [24-ACCEPTANCE.md](phases/24-correct-remaining-ntsc-dma-behavior/24-ACCEPTANCE.md) first.

## Accumulated Context

### Decisions

- v7 covers only the five selected failing NTSC DMA rows at the existing pin and declared RP2A03G target; the three passing DMA rows and every other protected PASS key remain guards.
- One phase owns all four requirements: independently justified corrections, owned wrong-model fixtures, regression preservation and state/documentation consequences belong together. Shared causes require traces.
- Preserve integer-only instance state, clean-room sources and the library/runner/libretro deliverables. Review format/revision impact if in-flight fields change; tests and README/public-header comments accompany each behavior change.
- Existing cold replay and native checkpoint routes remain separate; exact-build support differs from loader checks. Declared provenance is not authentication; repeatability and native/adapter equality do not establish host or hardware correctness.

### Pending Todos

The three deferred property/mutation/signal proposals remain unlinked to v7; their records are preserved below. No new todo scope is approved.

### Blockers/Concerns

- **Abort/state policy settled:** the approved countdown remains format6. The follow-through correction is behavior14 with no new serialized field/meaning or ABI change. Apply the approved compatibility policy without a new prompt for each implementation detail.

- **Plan24-02 bus-latch decision resolved:** owner approved cpu_data/external open_bus, format5 and behavior12 before mutation; implemented/verified. The later approved abort change is format6 / behavior13.

- **Plan 24-01 native-state checkpoint resolved:** owner approved the explicit silence byte, continuously clocked bits, format 4 and behavior revision 10. Later compatibility follow-through follows the owner’s standing policy; only a materially new consequence outside it needs another decision.

- **Specific runtime authority consumed:** the newly authorized single local workflow completed normally. Preserve its result; do not automatically run another campaign or hosted trigger. Ordinary owned fixes/retests remain authorized; runtime alone is not a permission gate.
- **Phase24 run-specific acceptance boundary:** the one authorized full workflow is consumed. Stop before another AccuracyCoin execution/campaign or hosted trigger until the owner renews that specific scope. Ordinary required local verification is already authorized; no general runtime approval gate is created. Keep requirements/phase incomplete until actual acceptance exists.
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

v7: three of four plans complete; the behavior14 correction and bounded evidence are committed. The authorized full workflow completed normally in444.60seconds, with session controls dominating441.98seconds of CTest; campaign collection took4.97seconds. The new focused developer tier takes a few seconds locally and preserves all full-CI gates. Plan04 acceptance and inherited branch whitespace remain incomplete. See the [efficiency synthesis](phases/24-correct-remaining-ntsc-dma-behavior/24-CI-EFFICIENCY.md) and [actual result](phases/24-correct-remaining-ntsc-dma-behavior/24-CI-RUN.md).

## Session Continuity

**Last session:** 2026-10-31T23:53:01Z
**Stopped at:** Plan24-04 Task1 evidence and bounded verification complete; Task2 awaits separate owner authority
**Next actor:** Owner decides whether to renew the exact local full-workflow scope; keeping the existing stop needs no response
**Next command:** `$gsd-execute-phase 24`
**Context:** May be cleared; read STATE, resume and acceptance first. The command resumes at the checkpoint, not earlier correction work.
**Resume file:** .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-EXECUTION-RESUME.md
