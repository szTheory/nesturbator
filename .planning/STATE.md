---
gsd_state_version: "1.0"
current_phase: 2
current_phase_name: The CPU matches the public vectors
status: planning
stopped_at: Phase 01 complete, ready to plan Phase 2
last_updated: "2026-10-03T13:20:33.240Z"
last_activity: 2026-10-03
last_activity_desc: Phase 01 complete, transitioned to Phase 2
state_head: dd16c532330a55ae65aa40c86b62089b03fd2735
progress:
  total_phases: 4
  completed_phases: 1
  total_plans: 12
  completed_plans: 12
  percent: 25
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-10-03)

**Core value:** Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.
**Current focus:** Phase 2 — The CPU matches the public vectors

## Current Position

Phase: 2 — The CPU matches the public vectors
Plan: Not started
Status: Ready to plan
Last activity: 2026-10-03 — Phase 01 complete, transitioned to Phase 2

Progress: [███░░░░░░░] 25%

## Performance Metrics

**Velocity:**
- Total plans completed: 12
- Average duration: - min
- Total execution time: 0.0 hours

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 01 | 12 | - | - |

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

## Accumulated Context

### Decisions

Decisions are logged in PROJECT.md Key Decisions table; the full list with sources is `.planning/preparation/DECISIONS.md`.
Recent decisions affecting current work:

- Roadmap: four phases, one per requirement category; merging each phase's pull request publishes a release.
- Roadmap: each phase's detail section in ROADMAP.md lists its canonical refs under `.planning/preparation/`; open them before designing or building.
- [Phase 01]: Runner tests check exact stdout and exit status through tests/cmake/expect_output.cmake; EXPECT is a list of lines
- [Phase 01]: Runner numbers are decimal 1 to 4294967295; repeated --hash-frame N prints frame N once
- [Phase 01]: header.cxx compiles the public header as C++11, the oldest standard with static_assert
- [Phase 01]: Core tests are plain programs on tests/check.h registered with nesturbator_core_test()
- [Phase 01]: palgen samples phase p at 75+30p degrees with U on the sine and V on the cosine, so colorburst (hue 8) lands on -U
- [Phase 01]: Generated palette banner is ASCII (--) so all sources stay ASCII for MSVC /WX
- [Phase 01]: 01-04: --dump-frame splits N:FILE at the first colon so FILE may contain colons
- [Phase 01]: 01-04: host/ holds code shared by runner and adapter (target nesturbator_host, hidden visibility, PIC, not installed)
- [Phase 01]: libretro.vendored pins libretro.h via a reusable cmake -P driver (tests/cmake/vendored_sha256.cmake)
- [Phase 01]: retro_load_game refuses a second load while an instance exists, so no instance leaks
- [Phase 01]: 01-06: nesturbator.pc derives its prefix from CMAKE_INSTALL_LIBDIR via file(RELATIVE_PATH), so it relocates under lib, lib64 or deeper
- [Phase 01]: 01-06: check_archives.cmake allow-lists each component's entries; a missing or extra file fails
- [Phase 01]: abi.undefined_symbols ignores names defined by another member of the same archive
- [Phase 01]: bzero is allowed on Darwin only (Apple clang lowers a zeroing memset to it); ELF keeps the exact allowlist
- [Phase 01]: Core comments state numbers as integers or words so the float scan needs no comment exception
- [Phase 01]: 01-08: hygiene.sh treats a file holding a NUL byte as binary (git's rule), using LC_ALL=C grep -I so every grep agrees
- [Phase 01]: 01-08: palgen fences the colour table with bare clang-format off/on markers; clang-format 18 ignores block-comment markers with trailing text
- [Phase 01]: 01-08: NESTURBATOR_CONFIG_INIT is fenced from clang-format, which would lay its braces out as a block
- [Phase 01]: compare_frame logic lives in nesturbator_test_compare_files so retroarch.compare checks the exact CLI messages; a too-large BMP returns 1 with its size so 257x240 is a size mismatch
- [Phase 01]: test.cfg.in sets all 34 path keys of the pinned v1.22.2 retroarch.cfg under the build directory; run_retroarch.cmake snapshots the user's RetroArch directory (size + microsecond mtime) and fails on any change
- [Phase 01]: 01-10: ci.yml's concurrency group includes github.workflow, so release.yml's call to ci.yml never queues behind or is replaced by the push run on the same SHA
- [Phase 01]: 01-10: create-github-app-token takes vars.NESTURBATOR_APP_ID through client-id (app-id is deprecated); GitHub accepts the App's Client ID or App ID
- [Phase 01]: 01-10: write_hashes.cmake is checked in the ci lane by runner.write_hashes and runner.write_hashes.content against tests/runner/hashes.txt
- [Phase 01]: RetroArch test config disables bundle_assets_extract so the macOS app leaves the user directory unchanged
- [Phase 01]: README install-line check runs as a CI step after packaging, not as a ctest test
- [Phase 01]: Owner hand-offs are scripted down to owner-only clicks
- [Phase 01]: hashes.txt written with file(CONFIGURE NEWLINE_STYLE LF) so every platform writes the same bytes
- [Phase 01]: CI job timeouts are twice the durations in run 37084642541, rounded up to whole minutes

### Pending Todos

- Phase 2, first task (v0.1.0 is published, so this is due now, before any `feat:` or `fix:` merge): remove the one-time `release-as` pin with `jq 'del(.packages["."]["release-as"])' release-please-config.json > tmp && mv tmp release-please-config.json`, and confirm `jq -e '.packages["."] | has("release-as") | not' release-please-config.json` exits 0. Left in place, every later release would again be 0.1.0. (Recorded by plan 01-10.)

### Blockers/Concerns

- [Phase 1] WR-08: the release-as pin is still in release-please-config.json; the Phase 2 todo above removes it and adds an automated check that it stays gone.

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

Last session: 2026-10-03T13:25:00Z
Stopped at: Phase 01 complete, ready to plan Phase 2
Resume file: None
