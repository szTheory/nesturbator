---
gsd_state_version: "1.0"
milestone: v6
milestone_name: Player-ready NES and reproducible feedback
current_phase: 22
current_phase_name: Install and protect progress
status: executing
stopped_at: Completed 22-02-PLAN.md; Plan 22-03 is next
last_updated: "2026-10-31T01:21:24.865Z"
last_activity: 2026-10-30
last_activity_desc: Plan 22-02 completed
state_head: 8a01df75e4e843826f981f88f00f7e4c453f5d6a
progress:
  total_phases: 2
  completed_phases: 0
  total_plans: 4
  completed_plans: 2
  percent: 50
current_plan: null
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated 2026-10-30 after v6 requirements and roadmap approval, and AGENTS.md.
Core value: Games behave as on the console, identically across platforms, from a small MIT C library.
Current focus: Phase 22 is in progress. Plans 22-01 and 22-02 are complete; plans 22-03 and 22-04 remain.

## Current Position

Phase: 22 (Install and protect progress) — EXECUTING
Plan: 3 of 4
Status: Ready to execute
Last activity: 2026-10-30 — Plan 22-02 completed and committed

## Next Action

Plans 22-01 and 22-02 are complete. Plan 22-02 added the canonical runner guide
and focused option, exit, stop, status and link controls. Its runner migration
ledger and scoped evidence are in
[22-02-SUMMARY.md](phases/22-install-and-protect-progress/22-02-SUMMARY.md).
READY-01 and READY-06 remain pending until the other sequential Phase 22 plans
finish.

The phase executor continues with Plan 22-03 when the owner invokes:

`$gsd-execute-phase 22`

This resumes the remaining sequential Phase 22 plans, starting with release and
engineering/testing guide migration. Context may be cleared first; read this
state, the linked summary and the next plan. Do not start Plan 22-04 or Phase 23
early. The focused documentation and contract checks passed; full CI remains
with the phase executor. No runtime behavior, package, host operation, push,
release or gameplay acceptance is claimed.

Six fallback edge assumptions and six descriptor-less prohibitions remain
explicitly flagged for downstream review; they are not implementation passes.

Work is on `gsd/phase-22-install-and-protect-progress`. Preserve the owner's
pre-existing unstaged AGENTS.md/config edits. Snapshot support guidance pairs
exact build/content; behavior revision is recorded but not enforced. Native
refusal evidence is separate from unqualified cross-build SRAM and incompatible
host-autoload recovery. The context preserves existing evidence, deferred work,
private-artifact boundaries and unchanged live-testing stops/allowances.

v5 remains complete and archived. Its [closeout](milestones/v5-CLOSEOUT.md)
and [audit](milestones/v5-MILESTONE-AUDIT.md) retain package-publication history,
collector/platform/provenance debt and source-bound acceptance. Recheck release
availability during Phase 22 implementation rather than treating opening status
as current. Phase 23 report/replay/triage remains the following approved phase.

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

**Last session:** 2026-10-31T01:21:24.849Z
**Stopped at:** Completed 22-02-PLAN.md; Plan 22-03 is next
**Next command:** `$gsd-execute-phase 22`
**Resume file:** .planning/continue.md

## Performance Metrics

| Plan | Duration | Tasks | Files |
|------|----------|-------|-------|
| Phase 22 P01 | 13 min | 2 tasks | 5 files |
| Phase 22 P02 | 12min | 2 tasks | 5 files |

## Decisions

- [Phase 22]: docs/api.md is the canonical source-tree API, status, and native-state guide.
- [Phase 22]: Exact core-build and content pairing is support guidance, separate from the native loader checks.
- [Phase 22]: docs/runner.md is the canonical home for runner options, examples, stop behavior and exit status.
- [Phase 22]: Runner save/state file behavior stays in the runner guide; native payload and library span contracts link to docs/api.md.
- [Phase 22]: Keep the checked MMC3 support clause in README while linking its detailed runner contract.
