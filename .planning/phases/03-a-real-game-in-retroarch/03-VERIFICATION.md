---
phase: 03-a-real-game-in-retroarch
verified: 2026-10-09T13:30:25Z
status: gaps_found
score: 11/13 must-haves verified
covered_files:
  - .github/workflows/ci.yml
  - .github/workflows/nightly.yml
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
  - README.md
  - include/nesturbator.h
  - libretro/libretro.c
  - runner/main.c
  - runner/movie.c
  - runner/movie.h
  - src/bus.c
  - src/cartridge.c
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
  - tests/cmake/prepare_scoreboard_baseline.cmake
  - tests/cmake/verify_scoreboard_regression.cmake
  - tests/cmake/write_hashes.cmake
  - tests/core/test_cartridge.c
  - tests/core/test_controller.c
  - tests/fuzz/rom_loader.c
  - tests/libretro/libretro_host.c
  - tests/ppu/test_registers.c
  - tests/ppu/test_render.c
  - tests/ppu/test_sprites.c
  - tests/retroarch/CMakeLists.txt
  - tests/retroarch/run_retroarch.cmake
  - tests/retroarch/test.cfg.in
  - tests/roms/manifest.txt
  - tests/runner/game_movie.c
  - tests/runner/hashes.txt
  - tests/runner/test_movie.c
  - tools/palgen/palgen.c
covered_digest: "v3:sha256:b3c25aac505243e4f219813a087943ae210713c0c1345900c6599e0b994f4870"
behavior_unverified: 0
overrides_applied: 0
gaps:
  - truth: "Sprites render on every visible scanline, including scanline 0 after the pre-render transition."
    status: failed
    reason: "Sprite evaluation returns on scanline 261, while the PPU begins at scanline 0 without preparing the first visible row. OAM Y=255 sprites that wrap to scanline 0 are therefore omitted."
    artifacts:
      - path: "src/ppu.c"
        issue: "sprite_evaluate() exits for scanlines above 239, and sprite_fetch() only prepares scanline + 1; there is no pre-render setup for target scanline 0."
      - path: "tests/ppu/test_sprites.c"
        issue: "The adjacent-scanline and synthetic pixel tests do not advance through pre-render or check Y=255 wraparound, so this gap is untested."
    missing:
      - "Prepare sprite evaluation and pattern data for scanline 0 during pre-render, preserving NES Y-coordinate wrap semantics."
      - "Add an automated full-frame boundary test that asserts a Y=255 sprite appears on visible row 0."
re_verification: false
---

# Phase 3: A Real Game in RetroArch Verification Report

**Phase Goal:** A player loads an NROM game in RetroArch, sees it render and controls it, and its frames are identical on every platform.
**Verified:** 2026-10-09T13:30:25Z
**Status:** gaps_found
**Re-verification:** No — initial verification

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | The runner safely loads supported mapper-0 iNES/NES 2.0 games and rejects invalid or oversized images before cartridge allocation; corpus regression is wired into CI and nightly fuzzing is configured. | ✓ VERIFIED | `src/cartridge.c` validates the full image before allocator use; `tests/core/test_cartridge.c` uses a counting allocator; `fuzz.regress` is registered in `tests/CMakeLists.txt`; nightly workflow contains the bounded fuzz job. Supplied CI passed the local corpus test. |
| 2 | Pinned game frames match recorded hashes on all six platforms, libretro matches the runner, and hosted RetroArch captures and compares the real game screenshot. | ? UNCERTAIN | `tests/cmake/write_hashes.cmake` builds complete inventories and `.github/workflows/ci.yml` defines six-platform hash equality and required `retroarch-e2e`; local `libretro.host` passed. However, this checkout has no exact-HEAD hosted run (`gh run list --workflow ci.yml --commit c790d993c1cd8c9023f5ea8cac6391b2d4490bdb` returned `[]`), and the supplied local run skipped both `retroarch.testframe` and `retroarch.game`. No screenshot or six-platform run result was observed. |
| 3 | Movie input replays deterministically and the same masks produce equal libretro frames. | ✓ VERIFIED | Supplied full CI passed `runner.movie` and `libretro.host`; the tests are registered in `tests/CMakeLists.txt`, and `runner/movie.c` validates the bounded movie before replay. |
| 4 | AccuracyCoin results are read from emulated RAM, match the scoreboard, pages 2 and 17 pass, and scoreboard regression prevents lost protected-main passes. | ✓ VERIFIED | Supplied full CI passed `accuracycoin.page2`, `accuracycoin.page17`, `accuracy.scoreboard`, and `accuracy.scoreboard.no_lost_pass`; `runner/main.c`, `tests/accuracy/test_scoreboard.c`, and the protected-main handoff in `.github/workflows/ci.yml` implement the corresponding paths. |
| 5 | The generated NROM path produces the same nonblank deterministic frame through runner and libretro, with CPU/PPU time conserved across frame edges. | ✓ VERIFIED | `tests/libretro/libretro_host.c` loads the generated cartridge through both hosts and compares pixels; `tests/core/test_api.c` and frame tests cover boundary timing. Supplied full CI passed `libretro.host` and `core.frame`. |
| 6 | The PPU exposes register, mirroring, vblank/NMI, timing, and background pixel behavior used by NROM games. | ✓ VERIFIED | `src/ppu.c`, `src/bus.c`, `tests/ppu/test_registers.c`, and `tests/ppu/test_render.c` are substantive and included in the full CI run; `ppu.registers` and `ppu.render` were among the passing tests. |
| 7 | Sprite composition, priority, hit/overflow, DMA, and adjacent-scanline outcomes behave correctly, including row 0. | ✗ FAILED | The automated sprite suite covers priority, x boundaries, overflow, DMA, and adjacent rows, but `src/ppu.c` never prepares sprite data for scanline 0. The review finding CR-03 is reproducible by inspection: `sprite_evaluate()` excludes scanline 261 and the initial state starts on scanline 0. See the structured gap above. |
| 8 | The two standard controller ports accept deterministic input from the runner and libretro joypad callbacks. | ✓ VERIFIED | `src/bus.c`, `libretro/libretro.c`, `tests/core/test_controller.c`, and `tests/libretro/libretro_host.c` connect and exercise the input path; supplied CI passed `core.controller` and `libretro.host`. |
| 9 | Three manifest-pinned open-licence NROM games and their movie milestones have a sorted recorded hash inventory. | ✓ VERIFIED | `tests/roms/manifest.txt`, `tests/runner/hashes.txt`, and `tests/cmake/write_hashes.cmake` are present; `runner.write_hashes` and `runner.write_hashes.content` passed in supplied CI. |
| 10 | The calibrated display palette regenerates identically and does not change native frame hashes. | ✓ VERIFIED | `tools/palgen/palgen.c`, `src/palette_ntsc.c`, and `tests/core/test_palette.c` are connected by `palette.regen`; supplied CI passed that test. |
| 11 | The six-platform CI configuration compares complete byte-identical frame/hash inventories and requires the RetroArch E2E job. | ✓ VERIFIED (wiring) | `.github/workflows/ci.yml` has six build lanes, hash artifact fan-in, `retroarch-e2e`, and `ci-required` dependency wiring. This verifies the gate configuration only; actual hosted output remains UNCERTAIN in truth 2. |
| 12 | The loader corpus replays without crash, and nightly libFuzzer is bounded and fail-closed. | ✓ VERIFIED | `tests/fuzz/rom_loader.c`, CTest `fuzz.regress`, and `.github/workflows/nightly.yml` contain the wired implementation. Supplied CI passed `fuzz.regress`; nightly execution itself was not observed in this local run. |
| 13 | The scoreboard test uses a protected-main baseline in CI and a deterministic committed snapshot for detached local runs. | ✓ VERIFIED (wiring) | `tests/cmake/prepare_scoreboard_baseline.cmake`, `.github/workflows/ci.yml`, and `tests/accuracy/test_scoreboard.c` connect the handoff and enforcement; local tests include empty/missing/lost-PASS cases. |

**Score:** 11/13 truths verified (0 present, behavior-unverified; 1 uncertain; 1 failed). The 13 rows combine the four roadmap criteria with non-duplicative plan must-haves. Repeated plan truths (for example, movie replay in Plans 05/11 and host parity in Plans 01/09) are represented once.

## Required Artifacts

| Artifact group | Expected | Status | Details |
|---|---|---|---|
| `src/cartridge.c`, `tests/core/test_cartridge.c` | Bounded loader and allocation-before-validation prevention | ✓ VERIFIED | Real parser; counting allocator and format/rejection cases are registered. |
| `src/frame.c`, `src/bus.c`, `src/ppu.c`, `tests/ppu/*` | Shared timed CPU/PPU frame, background and sprite rendering | ⚠️ PARTIAL | Substantive and wired; sprite state for visible scanline 0 is missing. |
| `runner/main.c`, `runner/movie.c`, `tests/runner/*` | CLI ROM/movie loading, deterministic frame output | ✓ VERIFIED | Linked to public core API; tests are registered and supplied run passed. |
| `libretro/libretro.c`, `tests/libretro/libretro_host.c` | Libretro content, input, video and frame parity | ✓ VERIFIED | Host calls exported adapter and compares output to runner; test passed. Per-sample audio and reset limitations are listed below as out-of-scope review findings. |
| `tests/roms/manifest.txt`, `tests/runner/hashes.txt`, `tests/accuracy/scoreboard.txt` | Licensed fixture inventory and committed baselines | ✓ VERIFIED | Manifest and expected inventories exist; full CI hash and AccuracyCoin tests passed. |
| `.github/workflows/ci.yml`, `.github/workflows/nightly.yml`, `tests/retroarch/run_retroarch.cmake` | Cross-platform hashes, hosted RetroArch proof and nightly fuzz | ✓ VERIFIED (structure) | Wiring is present and fail-closed. Hosted RetroArch execution and artifacts are not available for this exact HEAD. |

The artifact query did not parse these plans' inline/list artifact declarations (`verify.artifacts` returned `total: 0` for all 13 plans), so artifact status above is from direct source, build registration, and test wiring inspection rather than a green artifact-query verdict.

## Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| `runner/main.c` | public cartridge/frame API | load file bytes, run instance, emit native frame/hash | WIRED | Runner hash tests and generated-game host fixture exercise the path. |
| `libretro/libretro.c` | public cartridge/frame/input API | load game, poll joypads, run frame, convert pixels, call video callback | WIRED | `retro_run()` maps both joypads and passes core output to conversion/video callback; `libretro.host` passed. |
| `src/frame.c` / `src/bus.c` | `src/ppu.c` | CPU bus cycles advance the same instance PPU cursor | WIRED | Core frame and PPU tests are registered and passed in supplied CI. |
| `tests/cmake/write_hashes.cmake` | six-platform CI | produce artifacts consumed by equality job | WIRED | YAML fan-in checks six nonempty hash artifacts and compares them byte-for-byte. |
| `tests/retroarch/run_retroarch.cmake` | released RetroArch + runner | required mode loads pinned content, captures and compares screenshot | WIRED, RUN UNVERIFIED | CI workflow pins the app/checksum, enables required mode, and uploads evidence; no exact-HEAD hosted run was found. |
| `tests/cmake/prepare_scoreboard_baseline.cmake` | CTest scoreboard checks | CI exports baseline path, test checks no lost PASS | WIRED | Present in matrix before CTest; local baseline-specific regression tests passed. |

## Data-Flow Trace (Level 4)

| Artifact | Data variable | Source | Produces real data | Status |
|---|---|---|---|---|
| `runner/main.c` | native frame pixels | loaded ROM → instance → `nesturbator_run_frame()` | Yes | ✓ FLOWING |
| `libretro/libretro.c` | `video`, `xrgb` | instance frame output → `nesturbator_host_convert()` → frontend callback | Yes | ✓ FLOWING |
| `src/ppu.c` | native framebuffer | CHR/nametable/palette/OAM in the loaded instance | Yes, with scanline 0 sprite omission | ⚠️ PARTIAL |
| `runner/main.c` AccuracyCoin mode | test result rows | emulated CPU RAM after bounded scripted execution | Yes | ✓ FLOWING |
| `tests/cmake/write_hashes.cmake` | game/movie hashes | runner output from manifest-listed ROMs and generated movies | Yes | ✓ FLOWING |

## Behavioral Spot-Checks

| Behavior | Command/evidence | Result | Status |
|---|---|---|---|
| Full local workflow | Orchestrator supplied `cmake --workflow --preset ci`: 356/356 tests, 354 passed, 2 skipped, 0 failed; package step emitted library, runner, and libretro macOS arm64 ZIPs | RetroArch tests skipped because the host was unavailable; no local RetroArch claim made | ✓ PASS with explicit skips |
| Loader, core, PPU, controller, movie, game hashes, AccuracyCoin | Named CTest inventory in `build/ci`; supplied workflow result above | Relevant cases passed; all 256 CPU vectors and existing PPU/controller/game tests were included | ✓ PASS |
| Required real-game RetroArch screenshot | `gh run list --workflow ci.yml --commit c790d993c1cd8c9023f5ea8cac6391b2d4490bdb ...` | No exact-HEAD run listed; local `retroarch.testframe` and `retroarch.game` were skipped | ? NEEDS HOSTED EVIDENCE |
| Scanline 0 sprite rendering | Source trace of `sprite_evaluate()` and registered `ppu.sprites` test | No scanline 261 preparation and no full-frame Y=255 test | ✗ FAIL |

## Probe Execution

No Phase 03 plan declares a `probe-*.sh` path, and no conventional phase probe is part of this C test/tooling phase. Step 7c: not applicable.

## Requirements Coverage

| Requirement | Source plan | Status | Evidence |
|---|---|---|---|
| GAME-01 | 03-01, 03-02, 03-09 | ✓ SATISFIED | Full parser validation precedes allocation; invalid file runner path and counting allocator tests passed. |
| GAME-02 | 03-01, 03-03, 03-04, 03-06, 03-08, 03-09, 03-12 | ⚠️ PARTIAL / HUMAN EVIDENCE | Runner/libretro equality and recorded hashes passed local CTest; exact-host RetroArch screenshot proof is unavailable for this HEAD; sprite scanline 0 rendering is a code gap. |
| GAME-03 | 03-05, 03-06, 03-08, 03-11 | ✓ SATISFIED | Repeatable ordered movie output and identical scripted host frames passed. |
| GAME-04 | 03-03, 03-04, 03-07, 03-13 | ✓ SATISFIED | RAM-derived scoreboard tests, pages 2/17, and lost-PASS regression passed; CI baseline handoff is wired. |
| GAME-05 | 03-08, 03-10 | ✓ SATISFIED (implementation/configuration) | Corpus test passed; nightly libFuzzer job and failure handling are wired. No hosted nightly execution claimed. |
| GAME-06 | 03-06, 03-08, 03-12 | ⚠️ NEEDS HOSTED EVIDENCE | Six platform artifacts and byte-equality gate are configured, but no six-platform result for this exact HEAD was observed. |

All six required IDs are mapped by Phase 03 plans; no orphaned Phase 03 requirement was found in `REQUIREMENTS.md`.

## Anti-Patterns Found

| File | Location | Pattern | Severity | Impact |
|---|---|---|---|---|
| `src/ppu.c` | `sprite_evaluate()`, `sprite_fetch()` | No sprite preparation for scanline 0 | 🛑 BLOCKER | Phase 03's timed sprite/rendering must-have fails for the top visible row. |
| `libretro/libretro.c` | lines 58–62, 206–208 | Stores but never invokes per-sample audio callback | ⚠️ WARNING (scope) | CR-01 affects frontends using only the per-sample audio API; GAME-01..06 do not require audio. Do not attribute Phase 04 SND-01 to Phase 03. |
| `libretro/libretro.c` | lines 151–154 | `retro_reset()` is empty for a loaded game | ⚠️ WARNING (scope) | CR-02 is a libretro API limitation, but no Phase 03 success criterion requires frontend reset behavior. |
| `libretro/libretro.c` | line 298 | `return NULL` | ℹ️ INFO | Valid no-game/system-info path; not an empty feature implementation. |

The grep scan found no unreferenced `TBD`, `FIXME`, or `XXX` markers in the reviewed implementation/test/workflow scope. CR-01 and CR-02 remain open per `03-REVIEW-DISPOSITION.md`; they are not counted as Phase 03 blockers because they do not falsify this phase's six requirements or roadmap truths.

## Human Verification Required

### 1. Exact-HEAD hosted RetroArch and six-platform evidence

**Test:** Run the required CI workflow for this exact commit and inspect the `retroarch-e2e` job, hash-equality result, and retained `retroarch-e2e-frames` artifact.
**Expected:** All six hash artifacts are present and byte-identical; the required job verifies the pinned RetroArch release checksum/version, captures the game frame, compares it equal to the runner frame, and uploads the screenshot evidence.
**Why human:** The local environment had no RetroArch installation. Both local RetroArch tests skipped, and no exact-HEAD GitHub Actions run was available to inspect. The workflow definition is wired but cannot prove an external hosted execution by presence alone.

This human verification item does not change the overall status because the failed sprite-rendering truth takes precedence (`gaps_found`).

## Gaps Summary

The core loader, game/input/movie paths, local deterministic tests, AccuracyCoin checks, fuzz corpus, and required CI wiring are present and substantive. One observable rendering defect blocks the phase: the PPU does not prepare sprites for visible scanline 0, and current tests do not cover the pre-render/Y=255 transition. Separately, the required hosted RetroArch screenshot and six-platform equality have not been observed for this exact HEAD; local tests explicitly skipped RetroArch. Phase 4 covers sound only, so neither issue is deferred to a later roadmap phase.

Decision coverage: all 11 trackable CONTEXT.md decisions honored (non-blocking gate).

---

_Verified: 2026-10-09T13:30:25Z_  
_Verifier: the agent (gsd-verifier)_
