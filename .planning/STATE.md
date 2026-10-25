---
gsd_state_version: "1.0"
milestone: v4
milestone_name: Measured NTSC accuracy and robustness
status: Awaiting next milestone
stopped_at: Phase 18 complete — all phases complete
last_updated: "2026-10-25T02:32:18.508Z"
last_activity: 2026-10-24
last_activity_desc: Milestone v4 completed and archived
state_head: 39401b75ab3be6c4f071759cb0dc33ee0828d7d3
progress:
  total_phases: 3
  completed_phases: 3
  total_plans: 21
  completed_plans: 21
  percent: 100
current_phase: 18
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated 2026-10-24 after the v4 milestone, and AGENTS.md.
Core value: Games behave as on the console, identically across platforms, from a small MIT C library.
Current focus: planning the next milestone. The preferred proposal is RM-004 (NMI/VBlank and sprite-0 timing) in [PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md).

## Current Position

Phase: Milestone v4 complete (Phases 16–18 archived to [milestones/v4-phases/](milestones/v4-phases/))
Plan: —
Status: Awaiting next milestone
Last activity: 2026-10-24 — Milestone v4 completed and archived

Closeout type: override closeout. Phases 16 and 17 are marked stale only because later phases edited files they cover; the post-phase [v4 audit](milestones/v4-MILESTONE-AUDIT.md) passed 14/14 requirements, 5/5 integration and 5/5 flows. The next milestone's phases continue at Phase 19. Decisions are in PROJECT.md Key Decisions and `.planning/preparation/DECISIONS.md`.

## Delivery

Phase 16 is on main and released as v0.1.11. Phases 17 and 18 merge through PR #55 (exact-head CI 40150921041 green; Nightly 40150888217 success). PR #54 is folded into #55. Record the publication of the next release in MILESTONES.md once it is observed.

## Deferred Items

Items acknowledged and deferred at milestone close, most recent first:

| Category | Item | Status | Deferred At | Milestone |
|----------|------|--------|-------------|-----------|
| debug_sessions | phase16-cleanup-fixture | verifying | 2026-10-24 | v4 |
| debug_sessions | phase16-interruption | investigating | 2026-10-24 | v4 |
| deferred_items | 16-host-evidence-and-roadmap/deferred-items.md: local macOS nofp reports Apple-toolchain stack_chk_fail/stack_chk_guard symbols | acknowledged | 2026-10-24 | v4 |

Seven seed acknowledgements from earlier closes remain suppressed by their own `audit_acknowledged` markers.

## Preserved Debt

- Nightly run 39993108154: vectors-full failed (not a Release gate).
- AccuracyCoin observe records a JAM only as a generic frame failure; no AccuracyCoin JAM control.
- No explicit run-ahead-under-JAM test in `libretro.runahead`.
- Real-campaign minimization is proven only by controls.
- Historical metadata warnings (16-05 command strings as paths; three Phase 17 references) do not change any verdict.
- Preexisting `.planning/config.json` edits remain unstaged.

## Next Action

Start the next milestone with `/gsd-new-milestone`. It reads PRODUCT-ROADMAP.md, presents RM-004 as preferred with the DMA cluster and RM-005 (PAL) as alternatives, and promotes only the approved scope.

## Session

Last session: 2026-10-24
Stopped at: Milestone v4 completed and archived
Resume file: None

## Performance Metrics

| Plan | Duration | Tasks | Files |
|------|----------|-------|-------|
| Phase 17 P01 | 54min | 3 tasks | 15 files |
| Phase 17 P02 | 67min | 3 tasks | 12 files |
| Phase 17 P03 | 42min | 2 tasks | 15 files |
| Phase 17 P04 | 7h22m | 2 tasks | 38 files |
| Phase 18 P01 | 59min | 2 tasks | 12 files |
| Phase 18 P02 | 60min | 3 tasks | 13 files |
| Phase 18 P03 | 20min | 2 tasks | 13 files |
| Phase 18 P4 | 3h20m | 3 tasks | 20 files |
