---
gsd_state_version: "1.0"
milestone: v2
milestone_name: Most of the library plays
current_phase: 05
current_phase_name: Tune-up and v1 debt
status: executing
stopped_at: Completed 05-05-PLAN.md
last_updated: "2026-10-10T13:33:37.033Z"
last_activity: 2026-10-10
last_activity_desc: Phase 05 execution started
state_head: d17af0cc2c66416865792115b0307d4d704bea5a
progress:
  total_phases: 6
  completed_phases: 6
  total_plans: 7
  completed_plans: 5
  percent: 71
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-10-10)

**Core value:** Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.
**Current focus:** Phase 05 — Tune-up and v1 debt

## Current Position

Phase: 05 (Tune-up and v1 debt) — EXECUTING
Plan: 6 of 7
Status: Ready to execute
Last activity: 2026-10-10 — Phase 05 execution started

Progress: [███████░░░] 71% of v2

## Performance Metrics

**Velocity:**
- Total plans completed: 44
- Average duration: - min
- Total execution time: 0.0 hours

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 01 | 12 | - | - |
| 02 | 11 | - | - |
| 03 | 15 | - | - |
| 04 | 4 | - | - |
| 04.1 | 1 | - | - |
| 04.2 | 1 | - | - |

**Recent Trend:**
- Last 5 plans: -
- Trend: -

*Updated after each plan completion*
**Per-Plan Metrics:**

| Plan | Duration | Tasks | Files |
|------|----------|-------|-------|
| Phase 01 P01 | 4 min | 2 tasks | 15 files |
| Phase 01 P02 | 3 min | 2 tasks | 9 files |
| Phase 01 P03 | 1 min | 2 tasks | 13 files |
| Phase 01 P04 | 2 min | 2 tasks | 11 files |
| Phase 01 P05 | 3 min | 2 tasks | 13 files |
| Phase 01 P06 | 3 min | 2 tasks | 12 files |
| Phase 01 P07 | 6 min | 2 tasks | 10 files |
| Phase 01 P08 | 4 min | 2 tasks | 27 files |
| Phase 01 P09 | 7 min | 2 tasks | 9 files |
| Phase 01 P10 | 6 min | 2 tasks | 13 files |
| Phase 01 P11 | 15min | 3 tasks | 4 files |
| Phase 01 P12 | 10min | 2 tasks | 3 files |
| Phase 02 P01 | 8 min | 3 tasks | 13 files |
| Phase 02 P02 | 5 min | 2 tasks | 12 files |
| Phase 02 P03 | 9 min | 3 tasks | 14 files |
| Phase 02 P04 | 3 min | 2 tasks | 3 files |
| Phase 02 P05 | 3 min | 2 tasks | 3 files |
| Phase 02 P06 | 6 min | 3 tasks | 9 files |
| Phase 02 P07 | 11 min | 3 tasks | 11 files |
| Phase 02 P08 | 15 min | 2 tasks | 6 files |
| Phase 02 P09 | 15 min | 3 tasks | 6 files |
| Phase 02 P10 | 15min | 2 tasks | 6 files |
| Phase 02 P11 | 16min | 2 tasks | 5 files |
| Phase 03 P01 | 13 min | 2 tasks | 12 files |
| Phase 03 P09 | 7 min | 2 tasks | 7 files |
| Phase 03 P02 | 5min | 1 tasks | 6 files |
| Phase 03 P10 | 7 min | 1 tasks | 14 files |
| Phase 03 P03 | 11min | 2 tasks | 8 files |
| Phase 03 P04 | 15min | 2 tasks | 7 files |
| Phase 03 P05 | 7min | 1 tasks | 11 files |
| Phase 03 P11 | 8min | 1 tasks | 10 files |
| Phase 03 P06 | 16min | 1 tasks | 10 files |
| Phase 03 P12 | 12min | 1 tasks | 7 files |
| Phase 03 P07 | 57min | 1 tasks | 15 files |
| Phase 03 P13 | 12 min | 2 tasks | 7 files |
| Phase 04 P01 | 14 min | 2 tasks | 11 files |
| Phase 04 P02 | 122 min | 2 tasks | 15 files |
| Phase 04 P03 | 24 min | 1 task | 15 files |
| Phase 04 P04 | 10min | 2 tasks | 14 files |
| Phase 03 P14 | 11min | 2 tasks | 11 files |
| Phase 04.1 P01 | 11 min | 2 tasks | 6 files |
| Phase 03 P15 | 13min | 2 tasks | 5 files |
| Phase 04.2 P01 | 6 min | 2 tasks | 4 files |
| Phase 05 P01 | 15 min | 2 tasks | 1 files |
| Phase 05 P02 | 10 min | 2 tasks | 2 files |
| Phase 05 P03 | 15 min | 3 tasks | 9 files |
| Phase 05 P04 | 12 min | 3 tasks | 10 files |
| Phase 05 P05 | 20 min | 2 tasks | 8 files |

## Accumulated Context

### Decisions

Decisions are logged in PROJECT.md Key Decisions table; the full list with sources is `.planning/preparation/DECISIONS.md`.
Recent decisions affecting current work:

- Phase 04: Keep APU, synthesis, sample scheduling, and filter history in each instance and clock them from the CPU cycle timeline.
- Phase 04: Preserve frame-counter and DMC IRQ independence, DMA phase behavior, and the six named AccuracyCoin results in automated tests.
- Phase 04: Use the checked-in integer mixer and 16-tap/32-phase Q15 synthesis kernel with a bounded per-instance PCM ring; gate five tones below -80 dB.
- Phase 04: Hash mixed-level transitions before synthesis and signed PCM separately using explicit little-endian serialization.
- Phase 04: Require local CTest and hosted six-platform hash, sanitizer, no-float, hygiene, and RetroArch smoke checks; no listening UAT is required.
- [Phase 04.1]: Allocate 8 KiB PRG RAM only for accepted trainer-bearing images; preserve trainerless open-bus behavior.
- [Phase 04.1]: Copy trainer bytes during load before reset-vector setup, and base NROM mirroring on validated PRG size.
- [Phase 03]: Copy selected sprite bytes over odd-read/even-write pairs; after eight selections, follow the 2C02 diagonal n/m overflow scan and keep pre-render row-zero preparation intact.
- [Phase 03]: Keep behavior revision 4 because generated frame and audio hashes are byte-identical.
- [Phase 04.2]: Preserve batch audio as the primary libretro path; use the single-sample callback only when batch audio is unavailable. — Batch delivery remains efficient for capable hosts, while the mutually exclusive fallback supports sample-only frontends without dropping PCM or duplicating output.
- [Phase 05]: Release publish gate compared by exact job-scoped line equality to canonical CMake variables
- [Phase 05]: policy.no-skip has no allowlist; unrunnable tests are not registered and host-app checks live in retroarch-e2e
- [Phase 05]: Parallel CTest uses execution.jobs 4 with COST on the three long tests; no ccache (compile share 4 to 22 percent)
- [Phase 05]: nesturbator_reset clears the CPU poll latch after the last reset cycle because each bus cycle resamples the IRQ line

### Pending Todos

None.

### Blockers/Concerns

Research flags for v2: Phase 6 (HIGH, PPU fetch timing), Phase 9 (HIGH, MMC3 A12 filter), Phase 8 (MEDIUM, SxROM bits and RetroArch SRAM ordering), Phase 10 (MEDIUM, nes-runner reaching gameplay). v1 closed with an override closeout. Its deferred tech debt (release-policy substring check, trainer host-path coverage, local RetroArch self-skip on the owner's Mac, no-op `retro_reset()`) is listed in `.planning/MILESTONES.md` and `.planning/milestones/v1-MILESTONE-AUDIT.md`.

### Quick Tasks Completed

| # | Description | Date | Commit | Directory |
|---|-------------|------|--------|-----------|
| 261002-uu0 | Fix WR-03: callers zero size-tagged structs with memset; big-frame test; disposition marks WR-03 fixed | 2026-10-03 | 922a56f | [261002-uu0-fix-wr-03-from-planning-phases-01-a-test](./quick/261002-uu0-fix-wr-03-from-planning-phases-01-a-test/) |
| 261003-9tb | Fix README install block so release-please bumps every version: NESTURBATOR_VERSION line, check_install_line update, one-version-per-line ctest; PR #3 | 2026-10-03 | 36a971f | [261003-9tb-fix-the-readme-install-block-so-release-](./quick/261003-9tb-fix-the-readme-install-block-so-release-/) |

### Roadmap Evolution

- Phase 04.1 inserted after Phase 4: Close gap: GAME-01 — initialize accepted iNES trainers (URGENT)
- Phase 04.2 inserted after Phase 4: Close gap: SND-01 — deliver single-sample libretro audio (URGENT)
- v2 roadmap created 2026-10-09: Phases 5–10 (tune-up, mapper seam and PPU fetch pipeline merged into one phase so every phase releases, UxROM/CNROM/AxROM, MMC1 and battery saves, MMC3, close-out and boot-to-play)

## Deferred Items

Items acknowledged and deferred at milestone close, most recent first:

| Category | Item | Status | Deferred At | Milestone |
|----------|------|--------|-------------|-----------|
| debug_sessions | DEBUG-automation-first-uat | diagnosed (addressed by plans 02-09 to 02-11) | 2026-10-10 | v1 |
| seeds | SEED-001 | dormant | 2026-10-10 | v1 |
| seeds | SEED-002 | dormant | 2026-10-10 | v1 |
| seeds | SEED-003 | dormant | 2026-10-10 | v1 |
| seeds | SEED-004 | dormant | 2026-10-10 | v1 |
| seeds | SEED-005 | dormant | 2026-10-10 | v1 |
| seeds | SEED-006 | dormant | 2026-10-10 | v1 |
| seeds | SEED-261009-zs2 | dormant | 2026-10-10 | v1 |
| deferred_items | 03/deferred-items.md: local `retroarch.testframe` harness aborts on the owner's Mac | acknowledged | 2026-10-10 | v1 |

## Session Continuity

Last session: 2026-10-10T13:33:37.005Z
Stopped at: Completed 05-05-PLAN.md
Resume file: None

Next GSD command: `/gsd-plan-phase 5`.

## Operator Next Steps

- Plan Phase 5 with /gsd-plan-phase 5
