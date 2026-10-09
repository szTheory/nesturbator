---
gsd_state_version: "1.0"
current_phase: 04
current_plan: Not started
status: completed
stopped_at: Phase 04 complete — all phases complete
last_updated: "2026-10-09T13:18:20Z"
last_activity: 2026-10-09
last_activity_desc: Phase 02 verification refreshed; 62/62 must-haves passed
state_head: f329a51180e14f45681856c97dfd42683dfcc93c
progress:
  total_phases: 4
  completed_phases: 4
  total_plans: 40
  completed_plans: 40
  percent: 100
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-10-09)

**Core value:** Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.
**Current focus:** Refresh missing verification for Phase 03 before the Milestone 1 audit

## Current Position

Phase: 04
Current Plan: Not started
Total Plans in Phase: 4
Plans complete: 4 of 4.
Status: All phases complete; Phase 02 verification refreshed
Last activity: 2026-10-09 — Phase 02 verification passed (62/62; UAT 4/4)

Progress: [██████████] 100%

## Performance Metrics

**Velocity:**
- Total plans completed: 40
- Average duration: - min
- Total execution time: 0.0 hours

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 01 | 12 | - | - |
| 02 | 11 | - | - |
| 03 | 13 | - | - |
| 04 | 4 | - | - |

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
- [Phase 03]: Require hosted six-platform hash equality and released-RetroArch screenshot equality in `CI required`; retain evidence artifacts and skip owner UAT when these machine checks cover the behavior.
- [Phase 04]: Keep APU state instance-owned and clock it before bus register decode on each CPU cycle.
- [Phase 04]: Use the measured RP2A03G noise LFSR power-up state of zero, whose first clock shifts in one.
- [Phase 04]: Use checked-in integer pulse and 16 x 16 x 128 TND mixer cells for deterministic output.
- [Phase 04]: Preserve a resumed CPU read after DMC sample fetch, and use the observed GET phase for a deferred reload when `$4015` re-enables DMC with a full sample buffer.
- [Phase 04]: Keep frame and DMC IRQ status independent while aggregating the CPU IRQ line; behavior revision 2 records the resulting emulation change.
- [Phase 04]: Require six exact AccuracyCoin page-14 APU results in CI so these frame, length, and DMC acceptance checks need no owner UAT.
- [Phase 04]: Keep the 16-tap, 32-phase Q15 synthesis kernel, Q30 filter history, and bounded 1024-sample staging ring in per-instance state; advance audio behavior revision to 3.
- [Phase 04]: Verify the five coherent spectral fixtures in CI with the first 512 folded harmonic orders masked and no floating-point analyzer.
- [Phase 04]: Canonical audio hashing streams changed mixed levels before synthesis using CPU cycle cursor ticks/24, and serializes transition and PCM records explicitly little-endian. — This makes transition timing and signed PCM hashes platform-independent without exposing a public API or retaining a transition log; known-answer and callback-order tests pin the contract.

### Pending Todos

None.

### Blockers/Concerns

Phase 04's four plans passed structure and coverage checks. Phase 01 verification passes (69/69; UAT 4/4), and Phase 02 verification now passes (62/62; UAT 4/4). Phase 03 verification is missing and must be refreshed before the Milestone 1 audit. Phase 01 has one open advisory code-review warning in `01-REVIEW-DISPOSITION.md`: the release-policy test matcher can accept an expression containing `always()`. The Phase 02 review ledger has one open CR-01 about the libretro single-sample audio callback; the review scope was degraded and included later-phase files, so it is not attributed to Phase 02 and did not block CPU/vector verification.

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

Last session: 2026-10-09T13:18:20Z
Stopped at: Phase 02 verification passed; ready to refresh Phase 03 verification
Resume file: None
