---
phase: 03-a-real-game-in-retroarch
verified: 2026-10-09T14:48:45Z
status: passed
score: 13/13 must-haves verified
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
  - .planning/phases/03-a-real-game-in-retroarch/03-14-PLAN.md
  - .planning/phases/03-a-real-game-in-retroarch/03-14-SUMMARY.md
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
  - tests/cmake/hash_inventory.cmake
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
covered_digest: "v3:sha256:88c5bc96e4921dfe8887256390d028f259fe47eccff141f1a5138be825aec3ee"
behavior_unverified: 0
overrides_applied: 0
re_verification:
  previous_status: gaps_found
  previous_score: 11/13
  gaps_closed:
    - "Sprites render on every visible scanline, including scanline 0 after the pre-render transition."
  gaps_remaining: []
  regressions: []
human_verification: []
---

# Phase 3: A Real Game in RetroArch Verification Report

**Phase Goal:** A player loads an NROM game in RetroArch, sees it render and controls it, and its frames are identical on every platform.
**Verified:** 2026-10-09T14:17:58Z
**Status:** passed
**Re-verification:** Yes — after gap closure

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | The mapper-0 loader validates supported iNES/NES 2.0 images before allocation; the corpus regression and bounded nightly fuzz job are wired. | ✓ VERIFIED | The local CI workflow passed all runnable tests; hosted CI on exact HEAD `80bea33ad2545639dc002c977321ef4e1aead141` also passed. |
| 2 | Pinned game hashes agree across all six platforms, libretro receives equal frames, and hosted RetroArch compares and retains a game capture. | ✓ VERIFIED | Hosted CI run [37946197394](https://github.com/szTheory/nesturbator/actions/runs/37946197394) passed on exact HEAD. `hash-equality` validated six byte-identical 36-key inventories; `retroarch-e2e` installed pinned RetroArch 1.22.2, compared the game frame, and retained `retroarch-e2e-frames`. |
| 3 | Movie replay is deterministic and matching controller masks produce equal libretro frames. | ✓ VERIFIED | Regression check: movie/hash tests and host parity tests remain wired; supplied workflow passed all applicable tests. |
| 4 | AccuracyCoin results come from RAM, match the scoreboard, and protect existing passing rows. | ✓ VERIFIED | Regression check: page 2/page 17, scoreboard, and no-lost-pass tests remain wired; supplied workflow passed. |
| 5 | The generated NROM path produces matching nonblank frames through runner and libretro, with frame-edge timing conserved. | ✓ VERIFIED | Regression check: `libretro.host`, frame timing and API tests remain registered; supplied workflow passed all applicable tests. |
| 6 | PPU registers, mirroring, vblank/NMI, timing, and background rendering are implemented and exercised. | ✓ VERIFIED | Regression check: `ppu.registers` and `ppu.render` remain wired; supplied workflow passed them. |
| 7 | Sprite composition, priority, hit/overflow, DMA and adjacent rows work, including visible row 0 after pre-render. | ✓ VERIFIED | `src/ppu.c` evaluates scanline 261 for target row 0, wraps OAM Y=$FF only there, fetches row data at dot 257, and stores the next row separately from current composition. `tests/ppu/test_sprites.c::test_prerender_wraps_sprite_rows_into_visible_scanline_zero` advances the normal PPU clock across even and odd pre-render lengths, asserts row-zero palette pixels, confirms Y=$00 first appears on row 1, and checks the next row. The supplied full workflow passed. |
| 8 | Both controller ports receive deterministic runner and libretro input. | ✓ VERIFIED | Regression check: controller and libretro host tests remain wired and passed in supplied CI. |
| 9 | Three manifest-pinned games and movie milestones have a sorted recorded hash inventory. | ✓ VERIFIED | Regression check: manifest and generated hash inventory remain in place; generator/content checks passed. |
| 10 | Palette generation is reproducible and preserves native frame hashes. | ✓ VERIFIED | Regression check: palette generator and `palette.regen` remain wired; supplied CI passed. |
| 11 | Six-platform CI compares complete inventories and requires the RetroArch E2E job. | ✓ VERIFIED | Exact-HEAD hosted run passed all six build lanes, complete inventory equality, and required `retroarch-e2e` (run [37946197394](https://github.com/szTheory/nesturbator/actions/runs/37946197394)). |
| 12 | Loader corpus replay is safe, and nightly libFuzzer is bounded and fail-closed. | ✓ VERIFIED | Regression check: CTest corpus registration and nightly workflow configuration remain present; local corpus test passed. |
| 13 | CI scoreboard checks use a protected-main baseline and detached local checks use a deterministic snapshot. | ✓ VERIFIED (wiring) | Regression check: protected-main handoff, required CI variable, and detached snapshot path remain wired; supplied scoreboard regressions passed. |

**Score:** 13/13 truths verified (0 present, behavior-unverified; 0 uncertain).

### Required Artifacts

| Artifact group | Expected | Status | Details |
|---|---|---|---|
| `src/cartridge.c`, `tests/core/test_cartridge.c` | Bounded loader and allocation-before-validation | ✓ VERIFIED | Existing implementation and tests remain registered; applicable supplied CI tests passed. |
| `src/frame.c`, `src/bus.c`, `src/ppu.c`, `tests/ppu/*` | Shared timing, background and sprite output | ✓ VERIFIED | The carried scanline-zero defect is fixed and tested through normal clock progression. All applicable local tests passed. |
| `runner/main.c`, `runner/movie.c`, `tests/runner/*` | CLI content/movie loading and deterministic frame output | ✓ VERIFIED | Existing runner and movie tests remained wired and passed. |
| `libretro/libretro.c`, `tests/libretro/libretro_host.c` | Content, input, video and host frame parity | ✓ VERIFIED | Host test passed against the runner output. |
| `tests/roms/manifest.txt`, `tests/runner/hashes.txt` | Licensed fixture list and generated baselines | ✓ VERIFIED (local) | Hash generation and committed content comparison passed locally. Cross-platform equality execution remains part of truth 2. |
| `.github/workflows/ci.yml`, `.github/workflows/nightly.yml`, `tests/cmake/hash_inventory.cmake`, `tests/retroarch/run_retroarch.cmake` | Cross-platform gate, hosted screenshot proof and nightly fuzz | ✓ VERIFIED | Exact-HEAD CI run passed hash inventory equality and RetroArch frame comparison; `retroarch-e2e-frames` was retained. |

### Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| `runner/main.c` | public cartridge/frame API | load, run, emit frame/hash | WIRED | Runner tests and generated-game fixture exercise the API. |
| `libretro/libretro.c` | public cartridge/frame/input API | load, joypad poll, frame, callback | WIRED | Host integration test passed. |
| `src/frame.c` / `src/bus.c` | `src/ppu.c` | shared instance clock | WIRED | Frame and PPU tests passed. |
| `tests/cmake/write_hashes.cmake` | six-platform CI | hash artifacts and equality gate | ✓ VERIFIED | Exact-HEAD CI run validated all 36 keys and byte-identical contents across six platform artifacts. |
| `tests/retroarch/run_retroarch.cmake` | released RetroArch + runner | required capture and comparison | ✓ VERIFIED | Exact-HEAD `retroarch-e2e` passed; pinned RetroArch 1.22.2 frame comparison passed and the evidence artifact was retained. |
| `tests/cmake/prepare_scoreboard_baseline.cmake` | scoreboard checks | protected baseline handoff | WIRED | Required baseline preparation and regression test paths remain present. |

### Data-Flow Trace (Level 4)

| Artifact | Data variable | Source | Produces real data | Status |
|---|---|---|---|---|
| `runner/main.c` | native frame pixels | loaded ROM → instance → frame API | Yes | ✓ FLOWING |
| `libretro/libretro.c` | video buffer | instance frame → host conversion → callback | Yes | ✓ FLOWING |
| `src/ppu.c` | native framebuffer | instance CHR/nametable/palette/OAM | Yes; row-zero sprite path now covered | ✓ FLOWING |
| AccuracyCoin runner mode | result rows | emulated CPU RAM | Yes | ✓ FLOWING |
| hash generator | game/movie hashes | runner output for manifest ROMs and movies | Yes | ✓ FLOWING |

### Behavioral Spot-Checks

| Behavior | Command/evidence | Result | Status |
|---|---|---|---|
| Local CI workflow | `cmake --workflow --preset ci` after adding the checker self-test | 355 of 357 tests passed; `retroarch.testframe` and `retroarch.game` were skipped because RetroArch could not start locally | ✓ PASS with explicit skips |
| Pre-render Y=$FF row-zero sprite on both parities; Y=$00 adjacent-row behavior | `ppu.sprites` normal clock-path regression, also included in supplied full workflow | Assertions cover row-zero pixel, even/odd pre-render, transparency, row 1 and adjacent row; passed | ✓ PASS |
| Hash inventory, AccuracyCoin, libretro parity and loader corpus | Applicable named CTests in supplied full workflow | Passed as part of 357 tests | ✓ PASS |
| Hash inventory checker contract | `hash.inventory.self_test`, plus checker run against downloaded hosted artifacts | Valid inventory accepted; malformed row and platform mismatch rejected; exact-HEAD artifacts contain the same complete 36-key inventory | ✓ PASS |
| Exact-HEAD hosted six-platform and RetroArch capture evidence | [CI run 37946197394](https://github.com/szTheory/nesturbator/actions/runs/37946197394) on `80bea33ad2545639dc002c977321ef4e1aead141`; downloaded `retroarch-e2e-frames` | All required jobs passed; six inventories matched byte-for-byte; retained screenshot shows Nesteroids title screen and hosted frame comparison passed | ✓ PASS |

### Probe Execution

No phase plan declares a `probe-*.sh` path, and this phase has no conventional project probe. Not applicable.

### Requirements Coverage

| Requirement | Source plans | Status | Evidence |
|---|---|---|---|
| GAME-01 | 03-01, 03-02, 03-09 | ✓ SATISFIED | Loader validation, preallocation rejection tests and fuzz corpus regression remain wired and passed locally. |
| GAME-02 | 03-01, 03-03, 03-04, 03-06, 03-08, 03-09, 03-12, 03-14 | ✓ SATISFIED | Local game hashes, runner/libretro parity, and row-zero sprite regression passed; exact-HEAD six-platform equality and hosted RetroArch capture passed in run [37946197394](https://github.com/szTheory/nesturbator/actions/runs/37946197394). |
| GAME-03 | 03-05, 03-06, 03-08, 03-11 | ✓ SATISFIED (local) | Movie replay and libretro input parity tests passed in supplied CI. |
| GAME-04 | 03-03, 03-04, 03-07, 03-13, 03-14 | ✓ SATISFIED (local) | RAM-derived AccuracyCoin and scoreboard/no-lost-pass tests passed in supplied CI. |
| GAME-05 | 03-08, 03-10 | ✓ SATISFIED (wiring/local corpus) | Loader corpus regression passed; bounded nightly job remains configured. Nightly hosted execution was not claimed. |
| GAME-06 | 03-06, 03-08, 03-12, 03-14 | ✓ SATISFIED | Hosted run [37946197394](https://github.com/szTheory/nesturbator/actions/runs/37946197394) compared the complete 36-key inventories byte-for-byte across all six platforms and passed the required RetroArch check. |

All six Phase 03 requirements are claimed by plans; none are orphaned in `REQUIREMENTS.md`.

### Anti-Patterns Found

| File | Pattern | Severity | Impact |
|---|---|---|---|
| `libretro/libretro.c` | Per-sample audio callback is stored but not invoked | ⚠️ WARNING (out of scope) | Existing Phase 03 review item; not required by GAME-01 through GAME-06. |
| `libretro/libretro.c` | `retro_reset()` is empty for a loaded game | ⚠️ WARNING (out of scope) | Existing review item; no Phase 03 criterion requires frontend reset behavior. |
| `libretro/libretro.c` | `return NULL` in no-game/system-info path | ℹ️ INFO | Valid no-content behavior, not a stub. |

No unreferenced `TBD`, `FIXME`, or `XXX` debt markers were found in the reviewed Phase 03 implementation scope. The previous scanline-zero blocker is closed; no regressions were identified. No new-scope advisory findings.

### Automated Verification Evidence

The exact-HEAD hosted run passed the required six-platform builds, inventory comparison, and RetroArch E2E. Its `retroarch-e2e-frames` artifact contains the verified 256×240 game capture, runner reference frame, BMP comparison input, and pinned-release SHA evidence. No human UAT remains for Phase 03.

### Gaps Summary

The carried code gap is closed. Pre-render now evaluates and fetches sprites for visible row zero; a registered full clock-path regression checks Y=$FF on even and odd transitions and preserves Y=$00 adjacent-row behavior. The local workflow passed every runnable test, and exact-HEAD hosted CI passed six-platform equality and the pinned RetroArch frame comparison. No code gaps or human UAT items remain.

Decision coverage: all 11 trackable CONTEXT.md decisions honored (non-blocking gate).

---

_Verified: 2026-10-09T14:48:45Z_
_Verifier: inline GSD verification workflow_
