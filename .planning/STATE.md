---
gsd_state_version: "1.0"
milestone: v5
milestone_name: NMI, VBlank and sprite-0 timing
current_phase: 20
current_phase_name: NMI line and VBlank flag
status: Phase 20 context gathered; ready for planning
stopped_at: Phase 20 context gathered; ready for planning
last_updated: "2026-10-26T17:49:27.170Z"
last_activity: 2026-10-26
last_activity_desc: "Phase 20 context gathered; next: $gsd-plan-phase 20"
state_head: 9c4f188c313cdd13349a715c9b9a6a0860dc6c0f
progress:
  total_phases: 3
  completed_phases: 1
  total_plans: 6
  completed_plans: 6
  percent: 33
---

# Project State

## Project Reference

See [PROJECT.md](PROJECT.md), updated 2026-10-25 after Phase 19, and AGENTS.md.
Core value: Games behave as on the console, identically across platforms, from a small MIT C library.
Current focus: v5 NMI, VBlank and sprite-0 timing (RM-004 in [PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md)); Phase 20 NMI line and VBlank flag.

## Current Position

Phase: 20 — NMI line and VBlank flag
Plan: Not started
Status: Phase 20 context gathered; ready for planning
Last activity: 2026-10-26 — Phase 20 context gathered; next: $gsd-plan-phase 20

Progress: [███░░░░░░░] 33%

## Accumulated Context

### Decisions

- v5 build order follows research/SUMMARY.md: JAM attribution first; each phase writes its owned reproducers before its fix so main stays green; sprite-0 in its own phase before the NMI model; NMI line and edge latch before the poll; shared interrupt entry and hijack last.
- KEEP-02 and KEEP-03 sit in Phase 20 (hash moves, state format 2); KEEP-01 sits in Phase 21 (shared interrupt entry touches CPU vectors and the MMC3 IRQ). Every phase keeps the protected PASS set.
- [Phase 19]: JAM attribution: frame_and_sample returns nesturbator_status; ACCURACYCOIN_ACTIVE_RUNNING/BUDGET/JAM three-state classifier; execution-stage JAM is fail/0x03/guest-jam with exit 0
- [Phase 19]: 19-02: collector resolves the attributed JAM row by full key and classifies it with ACCURACYCOIN_ACTIVE_JAM; earlier-stage JAM writes no report and exits 1
- [Phase 19]: 19-03: STOP_JAM shows a never-written blank buffer for black so the cached picture survives; cache tag = frame_number + state-CRC timeline generation (D-16 amendment)
- [Phase 19]: D-10 amendment applied: cases 2, 4, 8, 9 and all-Y=$FF are guards; 1, 3, 5, 6, 7 and $FD are reproducers run through expect_red until their fix lands
- [Phase 19]: 19-05: page-19 driver case uses case-local execution cap 300; campaign caps unchanged; state/* hash rows re-pinned for behaviour revision 7
- [Phase 19]: 19-06: AccuracyCoin baseline re-recorded from measured full campaign (108 pass/38 fail/0 timeout); matched research prediction

### Blockers/Concerns

- Whether AccuracyCoin assumes a CPU/PPU alignment constant is untested (Phase 20).
- The pause handoff records three Nightly vectors-full fetch failures on 2026-10-26 (curl 28 / bad pack header from the upstream 65x02 download). This was external to Phase 19 and not a required PR check. Quick task 261026-i8z preserved the cold-fetch policy; no fresh hosted recheck was run.

### Pending Todos

- [2026-10-26] [testing] Add reproducible owned property tests — [todo file](.planning/todos/pending/2026-10-26-add-reproducible-owned-property-tests.md)
- [2026-10-26] [testing] Evaluate scoped nightly mutation testing — [todo file](.planning/todos/pending/2026-10-26-evaluate-scoped-nightly-mutation-testing.md)
- [2026-10-26] [testing] Make runner signal checks deterministic — [todo file](.planning/todos/pending/2026-10-26-make-runner-signal-checks-deterministic.md)

### Quick Tasks Completed

| # | Description | Date | Commit | Directory |
|---|-------------|------|--------|-----------|
| 261026-i8z | Reduce repeated CI suites, session controls and vector inventory work | 2026-10-26 | dbbf04f | [261026-i8z-reduce-ci-overhead-using-the-recovered-c](./quick/261026-i8z-reduce-ci-overhead-using-the-recovered-c/) |

## Delivery

Phase 16 is on main and released as v0.1.11. Phases 17 and 18 merged as PR #55 (exact-head CI 40150921041 green; Nightly 40150888217 success); PR #54 was folded into it. Release PR #56 (0.1.12) was open at the v5 opening; record its publication in MILESTONES.md once observed. Phase 19 squash-merged as PR #57 on 2026-10-26 (`ab9c2e6`), after the CI repairs documented in `debug/resolved/pr57-ci-failures.md`. Branch `gsd/phase-20-nmi-line-and-vblank-flag` was created and pushed from that main. Phase 20 has no context or plans yet.

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
- [Phase 19] Code review advisories open in 19-REVIEW-DISPOSITION.md: sprite-0 hit set only inside compose_pixel (held by NULL-video rejection), single jam_state_crc may blank an earlier jammed state under run-ahead, per-frame save_state cost in the adapter.
- [Phase 19] Navigation- and start-stage JAM branches share the boot-stage code but only boot is constructed in a test.
- [Phase 19] 19-06 baseline-metadata records a local dirty-tree run (source_dirty=1); a clean hosted run should confirm it.
- Real-campaign minimization is proven only by controls.
- Historical metadata warnings (16-05 command strings as paths; three Phase 17 references) do not change any verdict.
- Preexisting `.planning/config.json` edits remain unstaged.

## Next Action

Run `$gsd-discuss-phase 20` to gather context for NMI line and VBlank flag work. Three testing proposals are captured under Pending Todos with confirmed minor severity: reproducible owned property tests, deterministic runner signal checks, and later scoped nightly mutation testing. They remain proposed scope for future prioritization. The CI speed-up quick task is implemented in `dbbf04f`; its separate title workflow remains deferred pending a coordinated required-check ruleset migration. No Phase 20 discussion or implementation ran during capture.

## Session

Last session: 2026-10-26T17:49:27.123Z
Stopped at: Phase 20 context gathered; ready for planning
Resume file: .planning/phases/20-nmi-line-and-vblank-flag/20-CONTEXT.md

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
| Phase 19 P06 | 40 min | 3 tasks | 10 files |
