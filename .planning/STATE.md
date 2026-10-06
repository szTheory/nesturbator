---
gsd_state_version: "1.0"
current_phase: 02
current_phase_name: The CPU matches the public vectors
status: verifying
stopped_at: Completed 02-09-PLAN.md
last_updated: "2026-10-06T14:53:04.355Z"
last_activity: 2026-10-06
last_activity_desc: Phase 02 plan 09 policy gates completed
state_head: 08c1df4596068f18a26bffe09aca2cdb4e88a255
progress:
  total_phases: 4
  completed_phases: 1
  total_plans: 23
  completed_plans: 21
  percent: 25
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-10-03)

**Core value:** Games behave as they do on the console, identically on every platform, from a small MIT-licensed C library that any host can embed.
**Current focus:** Phase 02 — The CPU matches the public vectors

## Current Position

Phase: 02 (The CPU matches the public vectors) — EXECUTING
Plan: 9 of 11
Status: Phase execution in progress
Last activity: 2026-10-06 — Phase 02 plan 09 policy gates completed

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
| Phase 02 P01 | 8 min | 3 tasks | 13 files |
| Phase 02 P02 | 5 min | 2 tasks | 12 files |
| Phase 02 P03 | 9 min | 3 tasks | 14 files |
| Phase 02 P04 | 3 min | 2 tasks | 3 files |
| Phase 02 P05 | 3 min | 2 tasks | 3 files |
| Phase 02 P06 | 6 min | 3 tasks | 9 files |
| Phase 02 P07 | 11 min | 3 tasks | 11 files |
| Phase 02 P08 | 15 min | 2 tasks | 6 files |
| Phase 02 P09 | 15 min | 3 tasks | 6 files |

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
- [Phase 02]: Fixture attribution goes in tests/vectors/fixtures/README.md with the upstream MIT LICENSE verbatim, correcting D-06's 'comment' wording (JSON has no comments)
- [Phase 02]: vecconv re-reads its output from disk and re-encodes it byte for byte; opcode argument is exactly two hex digits; --first N errors if the array closes early
- [Phase 02]: release-as removed from release-please-config.json; release.no_release_as guards it; the next feat: release is 0.1.1
- [Phase 02]: 02-02: core.api's counting-allocator arena is 65536 bytes so the instance (now with 2048 bytes of RAM) fits
- [Phase 02]: 02-02: cpu.vectors takes chunks 1 or 256 and tests-per-chunk 1..10000; stdout is only the '65x02/<xx>: <f> of <n> vectors failed' line
- [Phase 02]: 02-02: vector fixture tests pass on vectors_fixture.cmake's exit status, with no pass regex
- [Phase 02]: 02-03: the committed sample is 1,678,114 B (sha256 0c318cec...e109), not D-04's 1,677,602 B; it equals the research's independent encoder
- [Phase 02]: 02-03: manifest.sha256 recomputes the SHA-256 of every manifest file; pins are 40-hex commits or release tags
- [Phase 02]: 02-03: vecconv_negative.cmake builds malformed inputs from a committed fixture at test time; vecconv.* tests pass on script exit status, never a regex or WILL_FAIL
- [Phase 02]: 02-04: address helpers run as call arguments; no expression holds two bus calls of unspecified order (cpu.c header states the rule)
- [Phase 02]: 02-04: ea_absi and ea_izy share index_base for the page-cross dummy read; stores pass always_dummy 1
- [Phase 02]: rmw(nes, ea, op) takes the shift/rotate/inc/dec helper as a function pointer argument; no opcode table in data
- [Phase 02]: P bits 4 and 5 are written only by pull_p (PLP, RTI); push_p (PHP, BRK) ORs 0x30 into the pushed byte
- [Phase 02]: 02-06: all 256 opcodes are explicit cases with no default:; JAM in a jam helper, SHY/SHX/SHA/TAS through one store_sh
- [Phase 02]: 02-06: struct nesturbator__profile (ane_magic, lxa_magic) is set to 0xEE by nesturbator_create; the vector harness sets it itself
- [Phase 02]: 02-06: cpu.vectors.00-ff come from a nested foreach with no skip list; the per-group opcode lists are gone
- [Phase 02]: 02-07: vectors-full tests register only under NESTURBATOR_VECTORS_FULL (preset vectors-full); the fetch re-verifies 256 files by size and SHA-256 and fetches nothing when all match
- [Phase 02]: 02-07: the nightly has no cache and keeps one rolling issue labelled nightly on scheduled runs; timeout-minutes 30 is initial until plan 08 measures a cold run
- [Phase 02]: 02-07: CONFORMANCE corrected (JSON array, compact layout, 54,565 B, 8-byte chunk header, 1,678,114 B sample, about 168 MB full set)
- [Phase 02]: 02-08: main merged into the phase branch (no rebase, no force-push) to clear the PR conflict; branch side won in ROADMAP and STATE
- [Phase 02]: 02-08: pull_p narrows only the non-constant result, since MSVC /W4 /WX rejects C4310 on (uint8_t)~FLAG_B
- [Phase 02]: 02-08: nightly-only CMake scripts set cmake_minimum_required(VERSION 3.25) for the runners' CMake 3.31
- [Phase 02]: 02-08: nightly vectors-full timeout 3 min (twice 88 s cold, run 37136095512); ci build 3 min (twice 69 s, run 37136095554); 88 s is under the D-21 10-minute cache trigger
- [Phase 02]: 02-09 pins the Phase 1 normalized public API only as a temporary Phase 2 gate, with a Phase 3 retirement path.
- [Phase 02]: 02-09 checks vector registrations from CTest json-v1 metadata and configures the full preset without running its fetch fixture.

### Pending Todos

None.

### Blockers/Concerns

None. (WR-08 resolved in plan 02-01: `release-as` removed in 30197fe and guarded by `release.no_release_as`.)

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

Last session: 2026-10-06T13:51:37Z
Stopped at: Session resumed; Phase 02 verification remains at UAT test 1 of 4
Resume file: .planning/phases/02-the-cpu-matches-the-public-vectors/.continue-here.md
