---
gsd_state_version: "1.0"
milestone: v6
milestone_name: Player-ready NES and reproducible feedback
status: planning
current_phase: 22
current_plan: null
stopped_at: v6 requirements and roadmap approved and initialized; Phase 22 discussion next
last_updated: "2026-10-31T00:07:36Z"
last_activity: 2026-10-30
progress:
  total_phases: 2
  completed_phases: 0
  total_plans: 0
  completed_plans: 0
  percent: 0
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated 2026-10-30 after v6 requirements and roadmap approval, and AGENTS.md.
Core value: Games behave as on the console, identically across platforms, from a small MIT C library.
Current focus: Phase 22, Install and protect progress; milestone opening is complete and discussion is next. Existing research/playtesting is sufficient.

## Current Position

Phase: 22 (Install and protect progress), first of two v6 phases
Plan: —
Status: Ready for phase discussion; implementation not started
Last activity: 2026-10-30 — Owner approved all ten requirements and Phases 22–23; milestone initialized

## Next Action

The owner selected **v6: Player-ready NES and reproducible feedback (RM-014)**
with “follow ur recs” after the recommended scope and alternatives were presented.
Version v6 continues phase numbering after 21. SEED-006 contributes only the
measured documentation/contract delta; other delivered seed outcomes are credited.
Reuse the [discovery synthesis](research/milestone-v6-discovery/SYNTHESIS.md),
[postmortem](playtesting/POSTMORTEM.md) and existing 22 title reports. No new broad
research fan-out or owner gameplay is needed.

The owner approved the concrete [requirements](REQUIREMENTS.md) and
[two-phase roadmap](ROADMAP.md) with “yes i approve”. All ten requirements are
mapped exactly once to Phases 22–23. The milestone-opening artifacts are committed
locally; no push, release, phase discussion, planning or execution is part of this
step. [PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md), direction revision 9, reconciles
all four horizons, triggers, partial evidence and debt within this same opening.

The next actor is the owner invoking the next command when ready:

`$gsd-discuss-phase 22`

This gathers the implementation approach for **Install and protect progress**
before phase planning. Context may be cleared first; read
[continue.md](continue.md). No further input is needed to finish this opening.
Preserve the pre-existing unstaged AGENTS.md/config edits. Overscan, core accuracy
fixes, PAL/Dendy, new mappers and actual Playstead integration stay outside v6.
Existing live-testing stops and runtime allowances remain unchanged.

v5 stays complete and archived: all eleven requirements satisfied, canonical
119 PASS / 27 FAIL over 146 scored rows plus five unscored DRAW, ABI 1 / state
format 3 / behavior revision 9. [Closeout](milestones/v5-CLOSEOUT.md) and
[audit](milestones/v5-MILESTONE-AUDIT.md) retain debt and evidence limits.
Release PR #56 was rechecked open; latest published package is v0.1.11.
Root causes of the reported seams, slowdown and music interruption remain
unclassified. Private replay establishes repeatability, not correctness; native
checkpoints are not qualified RetroArch containers. Operational limits and
private-artifact boundaries remain in [automation](playtesting/AUTOMATION.md),
[scene cards](playtesting/SCENE-PLAYLIST.md) and [controls](playtesting/CONTROLS.md).

## Preserved Debt and Boundaries

- Recurring collector Git/filesystem-mode enforcement, ignored-fixture/relative-root handling and duplicate validator debt remain open. Independent accepted-run checks do not fix future collections.
- Historical measurement attribution, README revision advisory, Phase 19 state-save cost/citation/early-stage coverage limits and summary/parser metadata warnings remain visible in the audit.
- macOS nofp Apple-toolchain symbol handling remains platform debt. Accepted hosted Linux results remain distinct.
- Real-campaign minimization remains proven only by controls; acknowledged debug sessions and three testing proposals stay preserved.
- WINDOWS entry 18 now agrees across JSON/table/counters, supported by existing fixed source evidence; no previously refused deviation append was invented. The archived deferred-item text records the original refusal.
- Physical zero-margin CPU/PPU precedence is unmeasured; judgment-tier provenance limits remain flagged. No safety stop, budget, threshold or new scope is waived by closeout.
- Historic Phase 16 safety stops and runtime windows remain unchanged in archived records. This milestone boundary grants no new runtime authority.

### Quick Tasks Completed

| # | Description | Date | Commit | Directory |
|---|-------------|------|--------|-----------|
| 261026-i8z | Reduce repeated CI suites, session controls and vector inventory work | 2026-10-26 | dbbf04f | [261026-i8z-reduce-ci-overhead-using-the-recovered-c](./quick/261026-i8z-reduce-ci-overhead-using-the-recovered-c/) |
| 261029-sdj | Reduce inventory parsing and reuse CTest-validated hash artifacts | 2026-10-29 | dc3451b | [261029-sdj-optimize-no-skip-inventory-parsing-and-r](./quick/261029-sdj-optimize-no-skip-inventory-parsing-and-r/) |

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

## Session

**Last session:** 2026-10-31T00:07:36Z
**Stopped at:** v6 opening complete; approved requirements/roadmap committed; Phase 22 discussion next.
**Next command:** `$gsd-discuss-phase 22`
**Resume file:** `.planning/continue.md`
