---
gsd_state_version: "1.0"
milestone: v5
milestone_name: NMI, VBlank and sprite-0 timing
current_phase: 19
current_phase_name: JAM reporting and sprite-0 hit
status: executing
stopped_at: Completed 19-05-PLAN.md
last_updated: "2026-10-25T18:14:49.279Z"
last_activity: 2026-10-25
last_activity_desc: Phase 19 execution started
state_head: 148913c0d4443aca90754680172a932b6989340c
progress:
  total_phases: 3
  completed_phases: 0
  total_plans: 6
  completed_plans: 5
  percent: 0
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated 2026-10-25 at the v5 milestone start, and AGENTS.md.
Core value: Games behave as on the console, identically across platforms, from a small MIT C library.
Current focus: v5 NMI, VBlank and sprite-0 timing (RM-004 in [PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md)); Phase 19 JAM reporting and sprite-0 hit.

## Current Position

Phase: 19 (JAM reporting and sprite-0 hit) — EXECUTING
Plan: 6 of 6
Status: Ready to execute
Last activity: 2026-10-25 — Phase 19 execution started

Progress: [░░░░░░░░░░] 0%

## Accumulated Context

### Decisions

- v5 build order follows research/SUMMARY.md: JAM attribution first; each phase writes its owned reproducers before its fix so main stays green; sprite-0 in its own phase before the NMI model; NMI line and edge latch before the poll; shared interrupt entry and hijack last.
- KEEP-02 and KEEP-03 sit in Phase 20 (hash moves, state format 2); KEEP-01 sits in Phase 21 (shared interrupt entry touches CPU vectors and the MMC3 IRQ). Every phase keeps the protected PASS set.
- [Phase 19]: JAM attribution: frame_and_sample returns nesturbator_status; ACCURACYCOIN_ACTIVE_RUNNING/BUDGET/JAM three-state classifier; execution-stage JAM is fail/0x03/guest-jam with exit 0
- [Phase 19]: 19-02: collector resolves the attributed JAM row by full key and classifies it with ACCURACYCOIN_ACTIVE_JAM; earlier-stage JAM writes no report and exits 1
- [Phase 19]: 19-03: STOP_JAM shows a never-written blank buffer for black so the cached picture survives; cache tag = frame_number + state-CRC timeline generation (D-16 amendment)
- [Phase 19]: D-10 amendment applied: cases 2, 4, 8, 9 and all-Y=$FF are guards; 1, 3, 5, 6, 7 and $FD are reproducers run through expect_red until their fix lands
- [Phase 19]: 19-05: page-19 driver case uses case-local execution cap 300; campaign caps unchanged; state/* hash rows re-pinned for behaviour revision 7

### Blockers/Concerns

- Sprite-0 basic-hit cause is not isolated; a trace of subtest 1 comes before any fix (Phase 19).
- Whether AccuracyCoin assumes a CPU/PPU alignment constant is untested (Phase 20).

## Delivery

Phase 16 is on main and released as v0.1.11. Phases 17 and 18 merged as PR #55 (exact-head CI 40150921041 green; Nightly 40150888217 success); PR #54 was folded into it. Release PR #56 (0.1.12) was open at the v5 opening; record its publication in MILESTONES.md once observed. v5 work is on branch `gsd/phase-19-jam-reporting-and-sprite-0-hit`.

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
- AccuracyCoin observe records a JAM only as a generic frame failure; no AccuracyCoin JAM control. (Scheduled: Phase 19, JAM-01.)
- No explicit run-ahead-under-JAM test in `libretro.runahead`. (Scheduled: Phase 19, JAM-02.)
- Real-campaign minimization is proven only by controls.
- Historical metadata warnings (16-05 command strings as paths; three Phase 17 references) do not change any verdict.
- Preexisting `.planning/config.json` edits remain unstaged.

## Next Action

Plan Phase 19 with `/gsd-plan-phase 19` (or `/gsd-discuss-phase 19` first) on the phase branch `gsd/phase-19-jam-reporting-and-sprite-0-hit`.

## Session

Last session: 2026-10-25T18:14:49.256Z
Stopped at: Completed 19-05-PLAN.md
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
| Phase 19 P01 | 35 min | 3 tasks | 6 files |
| Phase 19 P02 | 40 min | 3 tasks | 5 files |
| Phase 19 P03 | 75 min | 3 tasks | 6 files |
| Phase 19 P04 | 50 min | 2 tasks | 2 files |
| Phase 19 P05 | 40min | 3 tasks | 9 files |
