---
gsd_state_version: "1.0"
current_phase: 03
current_phase_name: A real game in RetroArch
current_plan: 12
status: executing
stopped_at: Completed 03-07-PLAN.md
last_updated: "2026-10-08T19:36:04.809Z"
last_activity: 2026-10-08
last_activity_desc: Phase 01 code-review fixes recorded; Phase 03 ready to plan; local RetroArch test failure carried forward
state_head: 14d006c511098cf72d5d5ea1886ebd0fb4674f56
progress:
  total_phases: 4
  completed_phases: 2
  total_plans: 36
  completed_plans: 34
  percent: 50
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-10-06)

**Core value:** Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.
**Current focus:** Phase 3 — A real game in RetroArch

## Current Position

Phase: 03 (A real game in RetroArch) — READY TO EXECUTE
Current Plan: 12
Total Plans in Phase: 13
Plan: 12 of 13 (03-01 complete)
Status: Ready to execute
Last activity: 2026-10-08 — Phase 01 complete, transitioned to Phase 03

Progress: [█████░░░░░] 50%

## Performance Metrics

**Velocity:**
- Total plans completed: 23
- Average duration: - min
- Total execution time: 0.0 hours

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 01 | 12 | - | - |
| 02 | 11 | - | - |

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

## Accumulated Context

### Decisions

Decisions are logged in PROJECT.md Key Decisions table; the full list with sources is `.planning/preparation/DECISIONS.md`.
Recent decisions affecting current work:

- Phase 02: all 256 opcodes match the committed public vectors on final state and each bus cycle; verification passed 62/62 truths.
- Phase 02: full-vector runs are cold and retain run-ID/SHA-bound CTest inventory plus JUnit results; validate both before reporting a pass.
- Phase 02: keep the public API declaration guard only through the CPU phase; Phase 3 must revise or retire it if public declarations change.
- Phase 02: the owner-authorized manual dispatch supplied the post-merge full-vector backstop; no cron event is claimed, and the reporter passed on the exact-merge main-push.
- Phase 02: clean-room source provenance remains an irreducible owner judgment; machine-verifiable behavior and product UAT are automated.
- [Phase 03]: Support one 16 KiB PRG and one 8 KiB CHR mapper-0 iNES geometry in the tracer.
- [Phase 03]: Carry frame overshoot from the instance tick cursor; JAM reports a latched nonzero stop status.
- [Phase 03]: Refresh the temporary public declaration guard to the Phase 3 API baseline.
- [Phase 03]: Keep CPU bus M2 scheduling in bus.c while PPU state transitions and register effects live in ppu.c.
- [Phase 03]: Keep cartridge parsing, allocator ownership, and frame state reset behavior unchanged during extraction.
- [Phase 03]: Keep mapper-0 support bounded to 16/32 KiB PRG and 8 KiB CHR ROM or declared CHR RAM.
- [Phase 03]: Validate exact complete-file geometry before allocation; reject unsupported header profiles and trailing data.
- [Phase 03]: Replay and libFuzzer call the same LLVMFuzzerTestOneInput function through public cartridge load, unload, and instance destruction.
- [Phase 03]: Use synthetic, manifest-listed corpus bytes and do not fetch a ROM or fuzz corpus at runtime.
- [Phase 03]: Require Clang for the optional libFuzzer build and instrument the core with fuzzer coverage plus ASan/UBSan in Linux nightly.
- [Phase 03]: Write native pixels into the caller-owned pitched output buffer at visible PPU dots to preserve the 64 KiB instance allocation contract.
- [Phase 03]: Select mapper-0 CIRAM horizontal or vertical mirroring from iNES flags 6 bit 0.
- [Phase 03]: Sprite evaluation uses secondary OAM and next-scanline pattern fetches.
- [Phase 03]: OAM DMA keeps PPU advancement on the shared CPU tick cursor.
- [Phase 03]: Controller masks are sampled once at successful frame entry so boundary-crossing instructions retain one deterministic frame input.
- [Phase 03]: The two standard controller ports keep independent per-instance latches and shift registers at $4016/$4017.
- [Phase 03]: Use a 16-byte NMOVIE1 header and little-endian version/count, then two little-endian 16-bit masks per frame; validate all bounds before replay.
- [Phase 03]: Movie replay hashes every frame in ascending order and reports the one-based JAM stop frame.
- [Phase 03]: DABG uses exact zlib-licensed default-branch commit 5ecc60b6af3f726851bfeb1c2555388c5953e0c0 because v2 resolves to GPL-3.0; DABG remains the two-player fixture.
- [Phase 03]: Use 525-line BT.601 2.4 transfer and primaries followed by deterministic offline sRGB/D65 encoding; native pixels remain the hash input.
- [Phase 03]: Expose a read-only CPU RAM peek for conformance tooling; accept RAM mirrors through $1FFF and reject other addresses without side effects.
- [Phase 03]: Use the exact ROM-resident AccuracyCoin directory as the source of names and result locations.
- [Phase 03]: Model PPU I/O bus decay at the conservative end of the documented 3–30 ms range using integer dot counts.

### Pending Todos

None.

### Blockers/Concerns

- **Local CI concern to carry into Phase 03 planning:** On 2026-10-08, two runs of `cmake --workflow --preset ci` configured and built successfully, and the final run passed 330/331 tests. `retroarch.testframe` exited with `Subprocess aborted` and empty stdout/stderr; the harness notes it requires a logged-in macOS GUI session, but the cause is not yet determined. Both new vector-count overflow regression tests passed. See [Phase 01 review fix report](./phases/01-a-test-frame-in-retroarch/01-REVIEW-FIX.md). Phase 01's recorded verification remains passed; do not treat this latest local workflow run as fully green.
- WR-08 is resolved in plan 02-01: `release-as` was removed in `30197fe` and guarded by `release.no_release_as`.

### Quick Tasks Completed

| # | Description | Date | Commit | Directory |
|---|-------------|------|--------|-----------|
| 261002-uu0 | Fix WR-03: callers zero size-tagged structs with memset; big-frame test; disposition marks WR-03 fixed | 2026-10-03 | 922a56f | [261002-uu0-fix-wr-03-from-planning-phases-01-a-test](./quick/261002-uu0-fix-wr-03-from-planning-phases-01-a-test/) |
| 261003-9tb | Fix README install block so release-please bumps every version: NESTURBATOR_VERSION line, check_install_line update, one-version-per-line ctest; PR #3 | 2026-10-03 | 36a971f | [261003-9tb-fix-the-readme-install-block-so-release-](./quick/261003-9tb-fix-the-readme-install-block-so-release-/) |

## Deferred Items

Items acknowledged and deferred at milestone close, most recent first:

| Category | Item | Status | Deferred At |
|----------|------|--------|-------------|
| *(none)* | | | |

## Session Continuity

Last session: 2026-10-08T19:36:04.646Z
Stopped at: Completed 03-07-PLAN.md
Resume file: None
