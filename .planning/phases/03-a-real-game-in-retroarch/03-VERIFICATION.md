---
phase: 03-a-real-game-in-retroarch
verified: 2026-10-09T21:50:41Z
status: passed
score: 31/31 must-haves verified
covered_files:
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
  - .planning/phases/03-a-real-game-in-retroarch/03-15-PLAN.md
  - .planning/phases/03-a-real-game-in-retroarch/03-15-SUMMARY.md
  - README.md
  - include/nesturbator.h
  - src/internal.h
  - src/ppu.c
  - tests/ppu/test_sprites.c
covered_digest: "v3:sha256:b5f31f2d97eeb890004e1d3865d6dd1f4246160a700fc8d53e1b740a3816f564"
behavior_unverified: 0
overrides_applied: 0
re_verification:
  previous_status: gaps_found
  previous_score: 25/28
  gaps_closed:
    - "Sprite overflow and related sprite outcomes match the documented 2C02 scan behavior."
    - "Exact-remediation-SHA six-platform hash equality and RetroArch capture evidence are available."
  gaps_remaining: []
  regressions: []
---

# Phase 3: A Real Game in RetroArch — Verification Report

**Phase Goal:** A player loads an NROM game in RetroArch, sees it render and controls it, and its frames are identical on every platform.
**Verified:** 2026-10-09T21:50:41Z
**Status:** passed
**Re-verification:** Yes — after diagonal sprite-overflow and exact-SHA evidence gap closure

## Goal Achievement

### Observable Truths

The four roadmap success criteria were verified in addition to all plan must-haves. The 31 plan truths include the roadmap behaviors; repeated statements were merged by intent.

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | iNES and NES 2.0 mapper-0 images validate before allocation; malformed input is rejected; corpus replay and nightly fuzzing are wired. | ✓ VERIFIED | `src/cartridge.c` validates the complete supported layout before cartridge allocation; `tests/core/test_cartridge.c` covers formats, rejection-before-allocation, and safe lifecycle. `fuzz.regress` passed locally and the exact-SHA six-platform CI run passed. Nightly run [37962202123](https://github.com/szTheory/nesturbator/actions/runs/37962202123) also passed `rom-loader-fuzz`, including sanitized build and bounded mutation steps. |
| 2 | The PPU renders game-derived frames and implements NTSC register, memory, timing, background, sprite, overflow, DMA, and scanline behavior. | ✓ VERIFIED | `src/ppu.c` and `src/bus.c` connect cartridge/PPU state to the shared frame clock. The focused local run passed `ppu.sprites`, including non-Y false-positive, skipped-Y, ninth-sprite, status timing/clear, and row-zero cases; `ppu.registers` and `ppu.render` are covered by the supplied 357/357 local full-workflow result. |
| 3 | Standard two-port controller input reaches the core and libretro; recorded movies replay deterministic frame hashes and same-input libretro frames. | ✓ VERIFIED | `$4016/$4017` latch/shift is implemented in `src/bus.c`; `libretro/libretro.c` polls joypad callbacks and submits frame input. `tests/core/test_controller.c`, `tests/libretro/libretro_host.c`, `runner/movie.c`, and `tests/runner/test_movie.c` cover serial input, host parity, valid replay, and malformed movies. All applicable tests passed in the supplied local workflow. |
| 4 | Pinned licensed NROM games have recorded native frame/hash milestones, the calibrated palette preserves native hashes, and the exact hosted six-platform inventories match. | ✓ VERIFIED | `tests/roms/manifest.txt` pins ROM provenance/checksums; `tests/runner/hashes.txt` contains 36 rows and `runner.write_hashes.content` passed locally. `src/palette_ntsc.c` is generated by `tools/palgen/palgen.c` and palette tests check regeneration. CI run [37994563525](https://github.com/szTheory/nesturbator/actions/runs/37994563525) has head SHA `20331371c8036cd5fccec6491d25262e3a2c0364`; all six Linux/macOS/Windows x64/arm64 build legs and byte-for-byte `hash-equality` succeeded. |
| 5 | RetroArch loads a pinned open-license game, renders the runner-matched frame, and retains its evidence artifact. | ✓ VERIFIED | In run 37994563525, `retroarch-e2e` and `CI required` succeeded for the exact remediation SHA. Downloaded artifact `retroarch-e2e-frames` contains nonempty 180 KiB `run/runner.ppm`, 1.5 KiB `run/shot.png`, 180 KiB `run/shot.bmp`, and `asset-evidence.txt`; the job's successful frame-comparison step compares capture to runner output. Evidence records RetroArch 1.22.2 and matching expected/actual SHA-256 `81b79121ba26d539064ae13b4d0419a120c3d165afbe656cf5f5412b15fdb434`. |
| 6 | AccuracyCoin pages 2 and 17 report RAM-derived results equal to the committed scoreboard, and no prior pass can disappear unnoticed. | ✓ VERIFIED | `runner/main.c` reads the emulated RAM results; `tests/accuracy/test_scoreboard.c` verifies named rows and the protected baseline; `tests/cmake/prepare_scoreboard_baseline.cmake` wires the fetched main baseline. The focused local run passed both page checks; all six hosted lanes prepared the protected-main baseline and succeeded. |

**Score:** 31/31 plan must-haves verified; all 4 roadmap success criteria satisfied. Behavior-unverified truths: 0.

### Required Artifacts

| Artifact group | Expected | Status | Evidence |
|---|---|---|---|
| Loader and fuzz | `src/cartridge.c`, loader tests, corpus, fuzz target | ✓ VERIFIED | All declared paths exist and contain implementation/test inputs. Loader APIs are reached by runner and libretro paths; CTest and nightly workflow invoke the owned loader/fuzzer. |
| PPU and rendering | `src/ppu.c`, register/render/sprite tests | ✓ VERIFIED | Real PPU state is clocked from frame execution and CPU bus writes. Focused `ppu.sprites` passed 1/1; local workflow passed all other registered checks. |
| Controllers and movies | bus/input adapter, movie parser/runner, seam tests | ✓ VERIFIED | Input callback values flow to public per-frame input; movie masks feed the same API. Controller, movie, and libretro host tests are registered and passed. |
| ROMs and deterministic inventory | manifest, three NROM images, `tests/runner/hashes.txt`, palette source/tool | ✓ VERIFIED | All declared artifacts exist; hashes and palette regeneration are checked in CI. Six-host equality evidence succeeded on the remediation SHA. |
| AccuracyCoin | ROM, scoreboard, regression test, baseline handoff | ✓ VERIFIED | RAM results flow into sorted scoreboard comparison; no-lost-PASS regression is connected to CI baseline setup. |
| Hosted RetroArch gate | `.github/workflows/ci.yml`, `tests/retroarch/run_retroarch.cmake`, pinned binary evidence | ✓ VERIFIED | Required job installs/checks pinned released RetroArch, compares captured output with runner output, fails closed, and uploads the inspected artifact. |

All declared plan artifacts were present. No stub or orphan was found among the artifacts supporting the must-haves.

### Key Link Verification

| From | To | Via | Status | Evidence |
|---|---|---|---|---|
| Runner and libretro content paths | Public cartridge/frame/input API | Load cartridge, set input, run frame | ✓ WIRED | `runner/main.c` and `libretro/libretro.c` call the shared public API; `tests/libretro/libretro_host.c` asserts parity. |
| CPU bus and frame clock | Instance PPU | Register/memory side effects and per-dot clock | ✓ WIRED | `src/bus.c` and `src/frame.c` reach `src/ppu.c`; register/render/sprite tests exercise production clock paths. |
| Movie reader and libretro host | Per-frame input API | Two-port masks | ✓ WIRED | `runner/main.c` takes the validated movie masks per frame; host tests feed matching input masks. |
| Hash generation and six CI lanes | Complete sorted hash inventories | Per-lane artifacts plus `hash-equality` fan-in | ✓ WIRED | CI run 37994563525's `hash-equality` succeeded after the six platform build jobs. |
| RetroArch launch harness | Pinned game and runner frame oracle | Required CI job, screenshot capture, `compare_frame` | ✓ WIRED | Exact-SHA `retroarch-e2e` succeeded; its retained runner/screenshot files and binary checksum evidence were inspected. |
| AccuracyCoin runner | Emulated result RAM and scoreboard | Required page execution and row comparison | ✓ WIRED | `runner/main.c` reads CPU RAM; scoreboard tests and six-lane protected-main baseline step succeeded. |
| Fuzz corpus and nightly target | Owned loader and teardown | `fuzz.regress` and bounded libFuzzer invocation | ✓ WIRED | Registration and workflow invoke `tests/fuzz/rom_loader.c`; local regression passed and hosted jobs succeeded. |

### Data-Flow Trace (Level 4)

| Artifact | Data | Source | Produces real data | Status |
|---|---|---|---|---|
| `runner/main.c` | Native frame and hash | Manifest ROM → instance → frame API | Yes | ✓ FLOWING |
| `libretro/libretro.c` | Host video frame | Instance framebuffer → conversion → video callback | Yes | ✓ FLOWING |
| `src/ppu.c` | Pixels and sprite status | Cartridge CHR, PPU memory/registers, OAM | Yes | ✓ FLOWING |
| AccuracyCoin mode | Result rows | Emulated CPU RAM | Yes | ✓ FLOWING |
| Hash generator | Game/movie inventory | Runner output for manifest games and movie fixtures | Yes | ✓ FLOWING |
| RetroArch capture | Screenshot | Pinned released RetroArch loading the test game | Yes | ✓ FLOWING |

### Behavioral Spot-Checks

| Behavior | Command/evidence | Result | Status |
|---|---|---|---|
| Focused GAME-01 through GAME-05 seams and hash content | `ctest --preset ci -R '^(core\.cartridge|core\.controller|libretro\.host|runner\.movie|runner\.write_hashes\.content|fuzz\.regress|ppu\.sprites|accuracycoin\.page2|accuracycoin\.page17)$' --output-on-failure` | 11/11 passed (CTest also selected `runner.dump` and `runner.write_hashes` by registered test properties) | ✓ PASS |
| Full local CI workflow | Supplied `cmake --workflow --preset ci` result | 357/357 tests passed; two RetroArch checks skipped because RetroArch is unavailable locally | ✓ PASS |
| Diagonal sprite-overflow timing | `ctest --test-dir build/ci -R 'ppu.sprites' --output-on-failure` | 1/1 passed against production PPU clock | ✓ PASS |
| Exact-SHA platform inventory equality | Hosted run 37994563525, `hash-equality` job | Successful for SHA `20331371c8036cd5fccec6491d25262e3a2c0364` | ✓ PASS |
| Exact-SHA RetroArch capture | Hosted run 37994563525, `retroarch-e2e` job and downloaded artifact | Job passed; screenshot, runner frame, and pinned-binary checksum evidence present and inspected | ✓ PASS |
| Scheduled loader mutation fuzzing | Nightly run 37962202123, `rom-loader-fuzz` job | Sanitized target built and bounded mutation test passed | ✓ PASS |
| Hosted required status | Hosted run 37994563525, `CI required` job | Success | ✓ PASS |

### Probe Execution

No plan declares a probe, and no phase probe script exists. Not applicable.

### Requirements Coverage

| Requirement | Source plans | Status | Evidence |
|---|---|---|---|
| GAME-01 | 03-01, 03-02, 03-09 | ✓ SATISFIED | Bounded mapper-0 loading, rejection-before-allocation tests, corpus regression, and nightly fuzz wiring. |
| GAME-02 | 03-01, 03-03, 03-04, 03-06, 03-08, 03-09, 03-12, 03-14, 03-15 | ✓ SATISFIED | Game frame hashes, libretro parity, PPU/sprite behavior, exact-SHA RetroArch frame comparison. |
| GAME-03 | 03-05, 03-06, 03-08, 03-11 | ✓ SATISFIED | Deterministic movie replay and same-input host parity; hosted hash equality includes the complete movie/game inventory. |
| GAME-04 | 03-03, 03-04, 03-07, 03-13, 03-14, 03-15 | ✓ SATISFIED | AccuracyCoin pages 2/17 RAM results, scoreboard equality and regression gate; sprite overflow cases pass. |
| GAME-05 | 03-08, 03-10 | ✓ SATISFIED | `fuzz.regress` passed locally; nightly run 37962202123 passed the sanitized, bounded `rom-loader-fuzz` job. |
| GAME-06 | 03-06, 03-08, 03-12, 03-14, 03-15 | ✓ SATISFIED | Six complete inventories compared byte-for-byte in successful exact-SHA `hash-equality`. |

No requirement assigned to Phase 03 was orphaned in `REQUIREMENTS.md`.

### Test Quality Audit

| Test area | Active / skipped | Circular | Assertion strength | Verdict |
|---|---|---|---|---|
| Loader, controller, PPU, libretro, movie, scoreboard, fuzz | Active registered tests; no disabled-test markers found in the inspected requirement-linked files | No expected-output generation from the tested implementation found | Value and behavioral assertions, including RAM rows, hashes, pixel equality, and clocked PPU status | ✓ ADEQUATE |
| Cross-platform frame inventory | Hosted `hash-equality` passed on exact SHA | Inventory is a deterministic regression baseline; hosted comparison independently runs each platform build | Byte-for-byte equality of complete ordered output | ✓ ADEQUATE for GAME-02/GAME-06 |
| RetroArch capture | Required hosted test passed on exact SHA | Runner is the specified pixel oracle; the check establishes host/render integration and exact capture equality | Pixel-by-pixel frame comparison plus pinned binary checksum | ✓ ADEQUATE for the stated integration criterion |

Disabled requirement-linked tests: 0. Circular expected-value patterns: 0. No insufficient assertion blocker found.

### Anti-Patterns Found

| File | Pattern | Severity | Impact |
|---|---|---|---|
| None in Phase 03 must-have implementation scope | No unresolved `TBD`/`FIXME`/`XXX`, placeholder, hollow-data, or console-only implementation found | — | No blocker. |

The current code review is clean (`03-REVIEW.md`: zero findings; `03-REVIEW-DISPOSITION.md`: no open dispositions). Previously deferred audio callback and reset observations are outside GAME-01 through GAME-06 and tracked beyond this phase; they do not fail a Phase 03 truth.

### Decision Coverage

The verification gate reports all 11 trackable CONTEXT.md decisions honored; no unhonored decisions.

### Human Verification Required

None. The phase CONTEXT explicitly makes command-verifiable checks automatic and says no behavior requires owner UAT. The only former manual evidence need is now satisfied by the exact-SHA hosted hash/RetroArch gates and inspected retained artifact.

### Gaps Summary

The two previously open verification items are closed. The corrected per-instance sprite evaluator now performs the post-eighth-sprite diagonal OAM byte walk; its production-clock regression passes in the 11-test focused selection. Run 37994563525 is successful on exact remediation SHA `20331371c8036cd5fccec6491d25262e3a2c0364`, with all six platform legs, hash equality, RetroArch screenshot comparison, and required CI status green. Nightly run 37962202123 additionally passed the sanitized bounded ROM-loader fuzz job. Current HEAD is a descendant of the remediation SHA; subsequent changes are phase summary/review/disposition records, not implementation, tests, or public docs. The retained RetroArch artifact is present and inspected, including matching pinned binary checksum evidence. All GAME-01 through GAME-06 requirements and roadmap success criteria are satisfied.

---

_Verified: 2026-10-09T21:50:41Z_
_Verifier: the agent (gsd-verifier)_
