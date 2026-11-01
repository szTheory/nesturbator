---
gsd_state_version: "1.0"
milestone: v7
milestone_name: Remaining DMA accuracy failures
current_phase: 24
current_phase_name: Correct remaining NTSC DMA behavior
status: executing
stopped_at: Plan 24-05 review fixes and local checks complete; replacement request v5 pending; owner decision required before any remote action
last_updated: "2026-11-01T12:27:00Z"
last_activity: 2026-11-01
last_activity_desc: Fixed three code-review findings, passed focused checks and a clean re-review; refreshed verification remains 8/9 with DMA-03 open, and a replacement request bound to the final committed candidate is next
state_head: dfd0bfcfc3a70f43b98435256bca8aba44a43b54
progress:
  total_phases: 1
  completed_phases: 0
  total_plans: 5
  completed_plans: 4
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated for v7 on 2026-10-31, and AGENTS.md.
**Core value:** Games behave as on the console, identically across platforms, from a small MIT C library.
**Current focus:** Behavior14 selected rows pass in fresh local full CI; phase verification is 8/9 with DMA-03 cold-main, hosted, hash and performance evidence outstanding

## Current Position

Phase: 24 (Correct remaining NTSC DMA behavior) — PLAN 24-05 TASK 3 AT REPLACEMENT-CANDIDATE REVIEW
Plan: 4 of 5 complete — Plans 24-01 through 24-04 completed; Plan 24-05 local repair, checks, and review are complete; candidate-bound request v5 is pending
Status: Goal verification scored 8/9; phase and requirements remain incomplete
Last activity: 2026-11-01 — Review hardening is committed, targeted checks passed, and the final review is clean; request v4 is stale after the source change
Progress: [████████░░] 80% (4 of 5 plans complete; Plan 24-05 remains active at candidate-bound request review)

## Next Action

**Local CMake approval is durable; no per-run approval is needed.** The completed local behavior14 workflow and campaign remain historical evidence in the [dated run record](phases/24-correct-remaining-ntsc-dma-behavior/24-CI-RUN-20261101.md) and [campaign measurements](../tests/accuracy/measurements.md#phase-24-behavior14-local-acceptance-2026-11-01). The source fixes from review are committed as `dfd0bfc`; focused checks, formatting and the final code review passed. Request v4 is stale after that source change. The executor initializer reports zero incomplete plans because Plan 24-05 already has a summary; finish goal verification and prepare a new candidate-bound request locally. Do not perform remote actions: the older push authorization is bound to an earlier candidate. The owner reviews the exact new request and decides whether to authorize its push, new PR and one configured CI run. See the [Task 3 summary](phases/24-correct-remaining-ntsc-dma-behavior/24-05-SUMMARY.md) and [acceptance record](phases/24-correct-remaining-ntsc-dma-behavior/24-ACCEPTANCE.md).

Local protected-observation tests used the checkout `scoreboard-main` fallback, not independent cold-main authority. DMA-03 still lacks fresh observations for every cold protected PASS key, current-candidate hosted CI and the full campaign, six-platform hash equality, and real base_a/base_b/head Cachegrind evidence. Inherited branch-wide planning whitespace is also unresolved; most findings are in archived records outside this scoped update. The earlier push/PR/CI authorization was bound to the pre-rewrite candidate and does not carry over. The owner authorized only local history repair and replacement-request preparation; no remote action, merge, release, or retry is currently authorized. The full suite has no safe coverage-preserving shortcut; `dev-check` remains the fast iteration tier. [Acceptance and efficiency record](phases/24-correct-remaining-ntsc-dma-behavior/24-ACCEPTANCE.md).

The required goal re-verification remains 8/9 with DMA-03 open; see [24-VERIFICATION.md](phases/24-correct-remaining-ntsc-dma-behavior/24-VERIFICATION.md). The final [code review](phases/24-correct-remaining-ntsc-dma-behavior/24-REVIEW.md) is clean after fixing the symlink-root escape, nullable CPU-read trace and optional CPU-PC trace compile path. The README acceptance wording is current and the three prohibition flags have explicit `human_judgment` tiers with unresolved status. Historical whitespace debt remains bounded.

Gap planning is complete. New [Plan 24-05](phases/24-correct-remaining-ntsc-dma-behavior/24-05-PLAN.md) covers trace completeness, the stale README wording, and the independently sourced DMA-03 evidence. The plan checker found no blockers or warnings; its one informational advisory concerns schema gaps in completed Plans 24-01 through 24-03 and does not justify reopening them. No tests, builds, CI, or remote actions ran during planning.

No ordinary implementation acknowledgment is needed. Task approval now durably covers local engineering, builds/tests/CI, reviews/fixes/bounded retests and approved compatibility follow-through. Those changes are recorded globally and in repository instructions. The additive `dev-check` preset provides fast owned feedback; existing CI objects and the complete707-test name/timeout/fixture/label inventory are unchanged. [Measured efficiency decisions](phases/24-correct-remaining-ntsc-dma-behavior/24-CI-EFFICIENCY.md).

Core correction a12a79a is behavior14 / native format6 / ABI1. All64 owned abort-key entries, bus matrix/status effects, focused byte/DAC wrong-model tests, genuine imminent/armed/fresh-buffer native cuts, short stress witnesses, current revision/document controls and homebrew hash consumers pass. Release builds with warnings as errors; no new saved field or dependency was needed. [Correction evidence](phases/24-correct-remaining-ntsc-dma-behavior/24-04-CORRECTIONS.md).

The earlier behavior13 campaign is historical at [24-CI-RUN.md](phases/24-correct-remaining-ntsc-dma-behavior/24-CI-RUN.md); the final behavior14 result supersedes it for local selected-row status. Baseline metadata now matches the observed campaign. No silicon or universal target claim is made.

The prior request digest `c772cfb2a8e0d46dbdbc58fb54623fc06be15b5724060fa2b93da0b03a77fe2e` was bound to pre-rewrite candidate `e2e24a6709aab6332267f414101fb7c2c05bb8bc`; its push authorization does not carry over. Replacement requests v1-v4 and their outcomes are detailed in the [Task 3 summary](phases/24-correct-remaining-ntsc-dma-behavior/24-05-SUMMARY.md). Request v4 was locally validated with `remote_action_authorized=false` but is stale after commit `dfd0bfcfc3a70f43b98435256bca8aba44a43b54`. The next request must bind the final committed verification and phase artifacts. The owner authorized local history repair and request preparation only. No push, PR, CI, retry, merge, or release is authorized for the replacement candidate.

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

- **Phase24 local CMake authority:** the owner explicitly authorized local CMake commands without per-run approval, including full CI and configured acceptance campaign fixtures. Preserve existing limits and do not automatically retry real interruptions/timeouts; investigate and resolve their cause first. Hosted workflow triggers, pushes, PRs, merges and releases remain separately unauthorized. Local selected-row evidence now exists; keep requirements/phase incomplete until independent cold protected-main, hosted cross-platform/hash, and unchanged performance evidence exists.
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

The final behavior14 local workflow passed all707 entries; campaign guest execution took6.35 seconds and CTest took517.82 seconds. Session-control work accounts for336.80 proc-seconds across three tests; skipping that coverage is not a safe acceptance optimization. `dev-check` remains the quick iteration tier. Hosted platform/hash/performance evidence remains missing, along with inherited branch whitespace. See the [efficiency synthesis](phases/24-correct-remaining-ntsc-dma-behavior/24-CI-EFFICIENCY.md) and [dated result](phases/24-correct-remaining-ntsc-dma-behavior/24-CI-RUN-20261101.md).

The final review found and fixed three test/evidence-tooling issues in commit `dfd0bfc`; `bus.unit`, the symlink and missing-read controls, the optional CPU-PC build, formatting, and the clean re-review passed. The refreshed verification remains `gaps_found` at 8/9 because DMA-03 lacks independent cold-main, current-candidate hosted, six-platform hash and Cachegrind evidence. This changes the candidate, so request v4 is stale. The next request will bind the final committed review and verification artifacts; remote authority remains false.

## Session Continuity

**Last session:** 2026-11-01T12:27:00Z
**Stopped at:** Source review fixes and targeted checks are complete; refreshed goal verification remains 8/9, and candidate-bound request v5 preparation remains. No remote branch, PR, or CI run exists.
**Next actor:** Local execution continues through request preparation; then the owner reviews the exact request and decides whether to authorize its push/PR/CI path
**Next command:** `$gsd-execute-phase 24 --gaps-only` — resumes the remaining local evidence-request work, then stops for owner review before any remote action
**Context:** Context may be cleared at this handoff. Read STATE, the resume file, and the acceptance record before resuming.
**Resume file:** .planning/phases/24-correct-remaining-ntsc-dma-behavior/24-EXECUTION-RESUME.md
