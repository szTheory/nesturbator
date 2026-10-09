---
phase: 03-a-real-game-in-retroarch
verified: 2026-10-09T19:47:24Z
status: gaps_found
score: 25/28 must-haves verified
covered_files:
  - .github/workflows/ci.yml
  - .github/workflows/nightly.yml
  - README.md
  - include/nesturbator.h
  - libretro/libretro.c
  - runner/CMakeLists.txt
  - runner/main.c
  - runner/movie.c
  - runner/movie.h
  - src/bus.c
  - src/cartridge.c
  - src/cpu.c
  - src/frame.c
  - src/instance.c
  - src/internal.h
  - src/palette_ntsc.c
  - src/ppu.c
  - tests/CMakeLists.txt
  - tests/accuracy/scoreboard-main.txt
  - tests/accuracy/scoreboard.txt
  - tests/accuracy/test_scoreboard.c
  - tests/cmake/fuzz_registration.cmake
  - tests/cmake/hash_inventory.cmake
  - tests/cmake/prepare_scoreboard_baseline.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/cmake/verify_scoreboard_regression.cmake
  - tests/cmake/write_hashes.cmake
  - tests/core/test_api.c
  - tests/core/test_cartridge.c
  - tests/core/test_controller.c
  - tests/core/test_palette.c
  - tests/fuzz/corpus/empty
  - tests/fuzz/corpus/header-only
  - tests/fuzz/corpus/mapper-bits
  - tests/fuzz/corpus/short-prg
  - tests/fuzz/corpus/single-byte
  - tests/fuzz/corpus/trailing-byte
  - tests/fuzz/corpus/truncated-header
  - tests/fuzz/corpus/valid-nrom
  - tests/fuzz/rom_loader.c
  - tests/libretro/libretro_host.c
  - tests/ppu/test_registers.c
  - tests/ppu/test_render.c
  - tests/ppu/test_sprites.c
  - tests/retroarch/CMakeLists.txt
  - tests/retroarch/run_retroarch.cmake
  - tests/retroarch/test.cfg.in
  - tests/roms/accuracycoin.nes
  - tests/roms/dabg.nes
  - tests/roms/manifest.txt
  - tests/roms/nesteroids.nes
  - tests/roms/rhde.nes
  - tests/runner/game_movie.c
  - tests/runner/hashes.txt
  - tests/runner/movie_fixture.h
  - tests/runner/test_movie.c
  - tools/palgen/palgen.c
  - .planning/phases/03-a-real-game-in-retroarch/03-01-PLAN.md
  - .planning/phases/03-a-real-game-in-retroarch/03-01-SUMMARY.md
  - .planning/phases/03-a-real-game-in-retroarch/03-02-PLAN.md
  - .planning/phases/03-a-real-game-in-retroarch/03-02-SUMMARY.md
  - .planning/phases/03-a-real-game-in-retroarch/03-03-PLAN.md
  - .planning/phases/03-a-real-game-in-retroarch/03-03-SUMMARY.md
  - .planning/phases/03-a-real-game-in-retroarch/03-04-PLAN.md
  - .planning/phases/03-a-real-game-in-retroarch/03-04-SUMMARY.md
  - .planning/phases/03-a-real-game-in-retroarch/03-05-PLAN.md
  - .planning/phases/03-a-real-game-in-retroarch/03-05-SUMMARY.md
  - .planning/phases/03-a-real-game-in-retroarch/03-06-PLAN.md
  - .planning/phases/03-a-real-game-in-retroarch/03-06-SUMMARY.md
  - .planning/phases/03-a-real-game-in-retroarch/03-07-PLAN.md
  - .planning/phases/03-a-real-game-in-retroarch/03-07-SUMMARY.md
  - .planning/phases/03-a-real-game-in-retroarch/03-08-PLAN.md
  - .planning/phases/03-a-real-game-in-retroarch/03-08-SUMMARY.md
  - .planning/phases/03-a-real-game-in-retroarch/03-09-PLAN.md
  - .planning/phases/03-a-real-game-in-retroarch/03-09-SUMMARY.md
  - .planning/phases/03-a-real-game-in-retroarch/03-10-PLAN.md
  - .planning/phases/03-a-real-game-in-retroarch/03-10-SUMMARY.md
  - .planning/phases/03-a-real-game-in-retroarch/03-11-PLAN.md
  - .planning/phases/03-a-real-game-in-retroarch/03-11-SUMMARY.md
  - .planning/phases/03-a-real-game-in-retroarch/03-12-PLAN.md
  - .planning/phases/03-a-real-game-in-retroarch/03-12-SUMMARY.md
  - .planning/phases/03-a-real-game-in-retroarch/03-13-PLAN.md
  - .planning/phases/03-a-real-game-in-retroarch/03-13-SUMMARY.md
  - .planning/phases/03-a-real-game-in-retroarch/03-14-PLAN.md
  - .planning/phases/03-a-real-game-in-retroarch/03-14-SUMMARY.md
covered_digest: "v3:sha256:8bbb9a829484bac8a7486c70bf7e46b321a9a643ab68ed5a5a24d5ab22c43e16"
behavior_unverified: 0
overrides_applied: 0
gaps:
  - truth: "Sprite overflow and related sprite outcomes match the documented 2C02 scan behavior."
    status: failed
    reason: "After the first eight sprites, the evaluator continues testing only each candidate's Y byte. The 2C02 diagonal scan also interprets tile, attribute, and X bytes as candidate Y values, so status bit 5 can be wrong for real programs."
    artifacts:
      - path: "src/ppu.c"
        issue: "Post-eighth-sprite evaluation does not advance the diagonal n/m scan through OAM bytes."
      - path: "tests/ppu/test_sprites.c"
        issue: "Tests cover an in-range ninth Y entry but not a false positive from a non-Y byte or diagonal candidate skipping."
    missing:
      - "Implement the documented post-eighth-sprite diagonal scan and add regression cases for non-Y-byte false positives and skipped candidates."
human_verification:
  - test: "Run the required hosted CI for commit 124dc96159596c3938433d5872805e21d5e2ca6c and inspect hash-equality plus retroarch-e2e evidence."
    expected: "All six complete game/movie hash inventories compare byte for byte; pinned RetroArch loads the game, its captured frame equals the runner frame, and retroarch-e2e-frames is retained."
    why_human: "No hosted run exists for current HEAD. The latest supplied local workflow skipped both RetroArch tests because RetroArch was unavailable, and the prior hosted run predates the row-zero PPU and frame-hash change."
---

# Phase 3: A Real Game in RetroArch Verification Report

**Phase Goal:** A player loads an NROM game in RetroArch, sees it render and controls it, and its frames are identical on every platform.
**Verified:** 2026-10-09T19:47:24Z
**Status:** gaps_found
**Re-verification:** Yes — after the row-zero sprite gap closure

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | Mapper-0 iNES/NES 2.0 loading validates complete bounded geometry before allocation; malformed input rejects with a diagnostic; fuzz corpus regression and bounded nightly fuzz are wired. | ✓ VERIFIED | `src/cartridge.c`, `runner/main.c`, `tests/core/test_cartridge.c`, `tests/fuzz/rom_loader.c`, `tests/cmake/fuzz_registration.cmake`, and `.github/workflows/nightly.yml` provide validation, preallocation rejection, corpus replay registration, and the bounded sanitizer fuzz job. The supplied current CI run passed 355 tests, with only the two RetroArch tests skipped. |
| 2 | NROM frames match recorded hashes, libretro receives equal frames, and released RetroArch capture matches the runner; current code has byte-identical hashes on all six platforms. | ⚠️ UNCERTAIN | Local runner hash-content and libretro host tests passed in the supplied CI run. The hosted `retroarch-e2e` and six-platform equality gates are wired in `.github/workflows/ci.yml`; prior hosted evidence exists, but it predates the Phase 03-14 row-zero rendering and native-hash update. No hosted run for current HEAD exists (`gh run list --workflow ci.yml --commit 124dc96159596c3938433d5872805e21d5e2ca6c` returned `[]`). See human verification. |
| 3 | Movie replay produces repeatable ordered hashes and the same masks produce equal libretro frames. | ✓ VERIFIED | `runner/movie.c`, `runner/main.c`, `tests/runner/test_movie.c`, and `tests/libretro/libretro_host.c` implement and exercise bounded movie replay and pixel parity; current supplied CI passed the applicable tests. |
| 4 | AccuracyCoin page 2/17 results are read from emulated RAM, match named sorted scoreboard rows, and protect prior passing rows. | ✓ VERIFIED | `runner/main.c` obtains emulated RAM results; `tests/accuracy/test_scoreboard.c` checks rows and `tests/cmake/verify_scoreboard_regression.cmake` covers loss of a prior PASS. Protected-main baseline handoff is wired through `tests/cmake/prepare_scoreboard_baseline.cmake` and the CI matrix. Current supplied CI passed the runnable AccuracyCoin and scoreboard checks. |
| 5 | PPU registers, mirroring, vblank/NMI, timing, and background pixel state follow the selected NTSC behavior. | ✓ VERIFIED | `src/ppu.c`, `src/bus.c`, `tests/ppu/test_registers.c`, and `tests/ppu/test_render.c` implement the path; the supplied CI run passed registered PPU tests. |
| 6 | Sprite priority, clipping, hit, overflow, DMA, and scanline-adjacent behavior match documented results. | ✗ FAILED | The current review's WR-01 is confirmed in code: `src/ppu.c` checks only the latched Y byte after eight sprites. `tests/ppu/test_sprites.c::test_ninth_in_range_sprite_sets_overflow` covers only the ninth in-range Y case, not diagonal false positives or skipped entries. This contradicts the explicit sprite overflow must-have in plans 03-04 and 03-14. |
| 7 | Y=$FF with an opaque first pattern row appears on framebuffer row 0 after pre-render on even and odd frames; Y=$00 begins on row 1. | ✓ VERIFIED | `tests/ppu/test_sprites.c::test_prerender_wraps_sprite_rows_into_visible_scanline_zero` advances the real PPU clock through even/odd pre-render transitions and checks row 0, row 1, and neighboring output. The supplied CI run passed. |
| 8 | Two standard controller ports latch and shift deterministic masks through $4016/$4017, and scripted inputs match libretro frames. | ✓ VERIFIED | `src/bus.c`, `src/instance.c`, `libretro/libretro.c`, `tests/core/test_controller.c`, and `tests/libretro/libretro_host.c` connect and test controller input; applicable tests passed in supplied CI. |
| 9 | Three licensed, manifest-pinned games (including DABG and CHR-RAM RHDE) have stable native-frame hash milestones. | ✓ VERIFIED | `tests/roms/manifest.txt` pins fixture checksums and `tests/runner/hashes.txt` records generated milestones. `runner.write_hashes.content` passed in supplied CI. |
| 10 | NTSC-to-sRGB palette regeneration is byte-identical and calibration preserves canonical native frame hashes. | ✓ VERIFIED | `tools/palgen/palgen.c`, `src/palette_ntsc.c`, and palette regeneration tests implement the offline path; the supplied CI run passed. |
| 11 | The generated row-zero game hash inventory matches its committed baseline, and the normal workflow retains hosted six-platform and RetroArch gates. | ✓ VERIFIED | `runner.write_hashes.content` passed in supplied CI; `.github/workflows/ci.yml` still makes `ci-required` depend on `hash-equality` and `retroarch-e2e`. Exact-current hosted execution is tracked separately under truth 2. |
| 12 | Scoreboard CI uses fetched protected-main bytes (or a confirmed empty baseline); detached local runs use the committed snapshot. | ✓ VERIFIED | `tests/cmake/prepare_scoreboard_baseline.cmake`, `tests/accuracy/test_scoreboard.c`, and the six-lane `.github/workflows/ci.yml` handoff enforce the source contract. The scoreboard regression checks passed locally. |

**Score:** 25/28 plan must-have truths verified; 2 failed (one shared overflow defect) and 1 uncertain. The two failed plan truths are the same diagonal-overflow defect described under truth 6 (plans 03-04 and 03-14).

### Required Artifacts

| Artifact | Expected | Status | Details |
|---|---|---|---|
| `src/cartridge.c`, `tests/core/test_cartridge.c` | Bounded loader and allocation-before-validation | ✓ VERIFIED | Substantive implementation and registered tests; supplied CI passed. |
| `runner/main.c`, `runner/movie.c`, `tests/runner/test_movie.c` | Content and movie replay with deterministic output | ✓ VERIFIED | Runner path, bounded parser and replay tests are connected; supplied CI passed. |
| `src/ppu.c`, `tests/ppu/test_registers.c`, `tests/ppu/test_render.c`, `tests/ppu/test_sprites.c` | PPU state and rendered game pixels | ⚠️ PARTIAL | Row-zero path and other tested PPU paths pass; diagonal sprite-overflow emulation remains incomplete. |
| `libretro/libretro.c`, `tests/libretro/libretro_host.c` | Content, controller, video and frame parity | ✓ VERIFIED | Adapter calls the shared public core and host parity test passed. |
| `.github/workflows/ci.yml`, `.github/workflows/nightly.yml`, `tests/retroarch/run_retroarch.cmake`, `tests/cmake/hash_inventory.cmake` | Six-platform equality, released RetroArch comparison, bounded nightly fuzz | ✓ VERIFIED (wiring) | Required jobs and consumers are connected. Historical exact-run proof predates current frame changes, so current hosted outcome remains human verification. |

### Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| `runner/main.c` | public cartridge/frame API | load, input, run, hash | WIRED | Runner and generated game/movie checks use the public instance API. |
| `libretro/libretro.c` | public cartridge/frame/input API | content load, joypad poll, video callback | WIRED | `libretro.host` passed host rendering and scripted-input parity. |
| `src/frame.c` / `src/bus.c` | `src/ppu.c` | shared instance timing and bus effects | WIRED | PPU and frame tests passed; state is per-instance. |
| `tests/cmake/write_hashes.cmake` | six-platform CI | upload inventories and compare complete ordered rows | WIRED | Required `hash-equality` job consumes six build artifacts; current commit's remote result is not available yet. |
| `tests/retroarch/run_retroarch.cmake` | pinned RetroArch and runner | capture and compare the same frame | WIRED | Required job and artifact upload remain in CI; current local RetroArch tests skipped and exact-current hosted run is absent. |
| `tests/cmake/prepare_scoreboard_baseline.cmake` | scoreboard tests | protected-main baseline handoff | WIRED | CI sets required baseline before CTest; regression suite passed locally. |

### Data-Flow Trace (Level 4)

| Artifact | Data | Source | Real data | Status |
|---|---|---|---|---|
| `runner/main.c` | Native frame pixels and hashes | Loaded manifest ROM → instance → frame API | Yes | ✓ FLOWING |
| `libretro/libretro.c` | Host video buffer | Same instance frame → conversion → callback | Yes | ✓ FLOWING |
| `src/ppu.c` | Framebuffer pixels | Cartridge CHR, nametable, palette, OAM | Yes | ✓ FLOWING |
| AccuracyCoin mode | Result rows | Emulated CPU RAM | Yes | ✓ FLOWING |
| Hash generator | Game/movie hashes | Runner output for manifest games and scripts | Yes | ✓ FLOWING |

### Behavioral Spot-Checks

| Behavior | Command/evidence | Result | Status |
|---|---|---|---|
| Current local CI workflow | Supplied `cmake --workflow --preset ci` result | 355 passed, 2 skipped (`retroarch.testframe`, `retroarch.game` because RetroArch unavailable), 0 failed; package step completed | ✓ PASS |
| Row-zero Y=$FF sprite path | Registered `ppu.sprites` clock-path test, included in supplied workflow | Even/odd pre-render and adjacent-row assertions passed | ✓ PASS |
| Game/movie hashes, libretro parity, AccuracyCoin, fuzz regression | Registered tests in supplied workflow | Applicable tests passed | ✓ PASS |
| Diagonal sprite-overflow behavior | Read `src/ppu.c:64-91` and `tests/ppu/test_sprites.c:75-90` | Evaluator tests only OAM Y bytes after the first eight; no diagonal byte scan case exists | ✗ FAIL |
| Exact-current hosted hashes and RetroArch screenshot | `gh run list --workflow ci.yml --commit 124dc96159596c3938433d5872805e21d5e2ca6c` | `[]`; prior hosted run predates row-zero/frame hash change | ? HUMAN |

### Probe Execution

No phase plan declares a probe script, and no conventional phase probe is present. Not applicable.

### Requirements Coverage

| Requirement | Source plans | Status | Evidence |
|---|---|---|---|
| GAME-01 | 03-01, 03-02, 03-09 | ✓ SATISFIED | Loader validates before allocation; rejection and corpus tests passed. Nightly fuzz target is wired. |
| GAME-02 | 03-01, 03-03, 03-04, 03-06, 03-08, 03-09, 03-12, 03-14 | ? NEEDS HOSTED EVIDENCE | Runner hashes and libretro equality pass locally; no hosted RetroArch/hash-equality run for current HEAD after the frame-hash update. |
| GAME-03 | 03-05, 03-06, 03-08, 03-11 | ✓ SATISFIED | Repeatable movie hashes and same-input libretro pixel parity tests passed locally. |
| GAME-04 | 03-03, 03-04, 03-07, 03-13, 03-14 | ✓ SATISFIED (scope criteria) | RAM-derived page 2/17 results, scoreboard equality, and protected-main no-lost-PASS checks passed. The separate diagonal overflow accuracy gap is captured under the failed plan truth. |
| GAME-05 | 03-08, 03-10 | ✓ SATISFIED (local regression and wiring) | Fuzz corpus replay passed; bounded nightly libFuzzer workflow remains connected. Nightly execution was not claimed. |
| GAME-06 | 03-06, 03-08, 03-12, 03-14 | ? NEEDS HOSTED EVIDENCE | Current local output matches its committed inventory; byte-for-byte results across all six platforms require the not-yet-observed current hosted `hash-equality` run. |

No Phase 03 requirement is orphaned in `REQUIREMENTS.md`.

### Anti-Patterns Found

| File | Pattern | Severity | Impact |
|---|---|---|---|
| `src/ppu.c` | Post-eighth-sprite overflow evaluator only checks each candidate Y byte | 🛑 BLOCKER | Fails the explicit documented sprite overflow outcome; see gap. |
| `libretro/libretro.c` | Per-sample audio callback is stored but not invoked | ⚠️ ADVISORY (out of scope) | SOUND is a later phase requirement and not part of GAME-01 through GAME-06. |
| `libretro/libretro.c` | `retro_reset()` is empty for a loaded game | ⚠️ ADVISORY (out of scope) | No Phase 03 must-have requires frontend reset behavior. |

No unreferenced `TBD`, `FIXME`, or `XXX` debt markers were found in the inspected Phase 03 implementation scope. Review disposition rows for the audio callback/reset findings and duplicate comments were carried as outside this review's finding set; they are not verification gaps for this phase. WR-01 is blocking because its technical claim directly disproves the sprite-overflow must-have, independent of its review severity label.

### Human Verification Required

#### 1. Exact-current hosted CI evidence

**Test:** Run required CI for the current code revision and inspect `hash-equality`, `retroarch-e2e`, and the retained `retroarch-e2e-frames` artifact.
**Expected:** Six complete inventories are byte-identical and the pinned RetroArch screenshot matches the runner frame.
**Why human:** The local machine has no runnable RetroArch, and GitHub Actions has no run for current HEAD. Existing hosted evidence is from before the Phase 03-14 frame output change.

### Gaps Summary

The phase's observable NROM load, render, controls, deterministic local hashes, movie replay, libretro parity, AccuracyCoin selected-page results, and loader/fuzz behavior are present and exercised. One concrete implementation requirement remains unmet: sprite overflow does not reproduce the 2C02 post-eighth-sprite diagonal OAM scan. Add the scan and regression cases before treating this phase's emulation outcome as complete. Exact-current six-platform and RetroArch proof also remains outstanding; the workflow is wired, but no current-head hosted run can substantiate those outcomes yet.

Decision coverage: all 11 trackable CONTEXT.md decisions honored (non-blocking gate).

---

_Verified: 2026-10-09T19:47:24Z_
_Verifier: the agent (gsd-verifier)_
