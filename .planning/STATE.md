---
gsd_state_version: "1.0"
current_phase: 03
current_phase_name: A real game in RetroArch
current_plan: 8
status: executing
stopped_at: 03-08 hosted RetroArch candidate passed on c5ec53b; final local workflow checks pass, publish the required-gate promotion and rerun the full matrix
last_updated: "2026-10-08T23:26:44.000Z"
last_activity: 2026-10-08
last_activity_desc: RetroArch candidate proof is recorded; required-gate promotion, matrix inputs, and target-scoped MSVC fixes pass local checks and are ready to publish
state_head: c5ec53b5402b3141f0fef13d4724f436d13f48c6
progress:
  total_phases: 4
  completed_phases: 2
  total_plans: 36
  completed_plans: 35
  percent: 50
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-10-06)

**Core value:** Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.
**Current focus:** Phase 3 — A real game in RetroArch

## Current Position

Phase: 03 (A real game in RetroArch) — EXECUTING
Current Plan: 03-08 (Hosted RetroArch validation checkpoint)
Total Plans in Phase: 13
Plans complete: 12 of 13; Plan 03-08 has local implementation, all four local workflow presets pass, and the hosted RetroArch candidate passes exact screenshot equality on `c5ec53b`.
Status: Checkpoint — candidate proven; final required-matrix run pending
Last activity: 2026-10-08 — hosted frame-60 comparison passed; promote the tested job and repair the unrelated baseline/hash-inventory/MSVC CI issues

Progress: [█████░░░░░] 50%

## Performance Metrics

**Velocity:**
- Total plans completed: 35
- Average duration: - min
- Total execution time: 0.0 hours

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 01 | 12 | - | - |
| 02 | 11 | - | - |
| 03 | 12 | - | - |

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
- [Phase 03]: CI uses the fetched protected-main scoreboard bytes; confirmed absence after successful lookup supplies an empty baseline.
- [Phase 03]: Detached local runs use the committed scoreboard-main.txt snapshot; CI requires the workflow-provided baseline path.

### Pending Todos

None.

### Blockers/Concerns

- **Phase 03 local CI:** after the hosted-build fixes, `cmake --workflow --preset ci` passed 346/346, ASan 344/344, no-float 5/5, and hygiene 8/8. The two local RetroArch launch tests skip after the macOS GUI session aborts with empty output; hosted `retroarch-e2e` supplies the required released-app proof.
- **Phase 03 hosted CI history:** run 37857200684 for `389e4c7` initially failed before candidate asset installation because its job lacked the protected-main scoreboard baseline; GCC conversion/format-truncation warnings and MSVC C4310 were also fixed in follow-up commits. Run 37858136711 for `f5888f2` exposed further test fixture warnings, fixed in `c5ec53b`.
- **Phase 03 sanitizer timeout:** run 37857200684 exceeded the old two-minute ASan job limit (2m14s). The workflow limit is now five minutes.
- **Phase 03 hosted matrix follow-up:** PR run 37858960926 and candidate run 37859023685 exposed three independent CI wiring/portability issues: the ASan job lacked the protected-main scoreboard baseline, six-platform hash collection omitted `MOVIE_WRITER`, and two movie test targets hit MSVC's C4996 warning. Local fixes pass the exact hash command; hosted full-matrix evidence is pending.
- **Phase 03 RetroArch E2E:** exact-commit run 37859023685 on `c5ec53b5402b3141f0fef13d4724f436d13f48c6` passed the local suite, verified the official DMG SHA-256 and RetroArch 1.22.2, launched Nesteroids frame 60 without changing the real home, captured a 1503-byte screenshot, and proved pixel equality with the runner. Artifact `retroarch-candidate-37859023685` (ID `11584984604`) contains the asset and frame evidence. The workflow changes configure this proven job as required `retroarch-e2e`, retaining `retroarch-e2e-frames`.
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

Last session: 2026-10-08T23:26:44.000Z
Stopped at: 03-08 candidate passed and local workflow checks pass; publish required-gate promotion and CI matrix fixes, then verify all exact-commit checks
Resume file: .planning/phases/03-a-real-game-in-retroarch/03-08-CHECKPOINT.md
