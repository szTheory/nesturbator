---
phase: 03-a-real-game-in-retroarch
plan: 15
subsystem: ppu
tags: [ppu, sprite-overflow, oam, deterministic-hashes, retroarch]
requires:
  - phase: 03-04
    provides: Per-dot sprite selection and status timing
  - phase: 03-08
    provides: Six-platform hash equality and RetroArch CI gates
  - phase: 03-14
    provides: Pre-render row-zero sprite preparation
provides:
  - Diagonal primary-OAM byte comparisons after eight selected sprites
  - Clocked regressions for false-positive and skipped-Y overflow cases
  - Exact-SHA hosted hash and RetroArch evidence
affects: [GAME-02, GAME-04, GAME-06, ppu, frame-hashes]
actuals:
  tokens: 2247
  tasks: 2
  commits: 5
commits: 5
plan_head_before: 70f06710aa01d009c0e31bd843c719c310618780
plan_head_after: 20331371c8036cd5fccec6491d25262e3a2c0364
tech-stack:
  added: []
  patterns:
    - Keep post-eight OAM byte position and remaining-copy state inside each PPU instance
key-files:
  created: []
  modified: [src/ppu.c, src/internal.h, tests/ppu/test_sprites.c, README.md, include/nesturbator.h]
key-decisions:
  - "Keep the first eight selected sprite slots and pre-render row-zero path intact while scanning n and m after selection fills."
  - "Keep behavior revision 4 because the generated frame and audio inventory is byte-identical."
requirements-completed: [GAME-02, GAME-04, GAME-06]
coverage:
  - id: D1
    description: "Visible sprite overflow compares diagonal OAM bytes after eight selections, including false positives and skipped Y values."
    requirement: GAME-02
    verification:
      - kind: unit
        ref: "ctest --preset ci -R '^ppu\\.sprites$' --output-on-failure"
        status: pass
    human_judgment: false
  - id: D2
    description: "Pre-render clears overflow without losing row-zero sprite preparation or first-eight rendering."
    requirement: GAME-04
    verification:
      - kind: integration
        ref: "cmake --workflow --preset ci: 357/357 passed, two local RetroArch checks skipped by environment"
        status: pass
    human_judgment: false
  - id: D3
    description: "All six hosted builds produce equal hashes and the pinned RetroArch capture passes on the remediation SHA."
    requirement: GAME-06
    verification:
      - kind: e2e
        ref: "https://github.com/szTheory/nesturbator/actions/runs/37994563525"
        status: pass
    human_judgment: false
duration: 13min
completed: 2026-10-09
status: complete
---

# Phase 03 Plan 15: Diagonal Sprite Overflow Summary

**After eight selected sprites, visible-line overflow now follows the 2C02 diagonal OAM byte walk, with exact-commit six-platform and RetroArch evidence.**

## Performance

- **Started:** 2026-10-09T21:06:37Z
- **Completed:** 2026-10-09T21:19:00Z
- **Tasks:** 2
- **Files modified:** 5

## Accomplishments

- The evaluator copies the first eight selected sprites through odd-dot reads and even-dot secondary-OAM writes, eight dots per sprite. The ninth Y is read at dot 129 and compared at dot 130. After eight selections, misses advance both n and m without carry; hits set visible overflow and consume three bytes with carry on m wrap. Reads stop at n=64.
- Production-clock regressions check tile, attribute, X, and wrapped-Y comparisons at their dots, a skipped in-range Y, direct ninth-Y overflow, sticky status, pre-render clearing, and retained first-eight slots. Existing row-zero, sprite pixel, and DMA cases pass.
- `runner.write_hashes` generated 36 sorted rows byte-identical to `tests/runner/hashes.txt` (SHA-256 `40ed2f2a277b4c93a4ace4c7a273a53747bee61d878eee8451ee7e6b7407f0e2`). The inventory and behavior revision remain unchanged.

## Task Commits

1. **Task 1 RED regression:** `c9ab9c8` — `test(03-15): expose diagonal sprite overflow comparisons`.
2. **Task 1 GREEN implementation and documentation:** `868ab69` — `fix(03-15): scan diagonal OAM bytes after eight sprites`.
3. **Task 2 hosted GCC portability fix:** `d0b4932` — `fix(03-15): keep sprite fixture indices wide on GCC`.
4. **Task 2 hosted format fix:** `4fce434` — `style(03-15): format sprite scan regression`.
5. **Code-review timing correction:** `20331371c8036cd5fccec6491d25262e3a2c0364` — `fix(03-15): correct sprite selection timing`.

## TDD Gate Compliance

- RED: `ppu.sprites` exited 8 on the planned non-Y positive and skipped-Y negative assertions. The JUnit record at `build/ci/ppu-diagonal-red.json` returned `RED_EVIDENCE_OK`; semantic inspection confirmed status `0` where `32` was expected on a non-Y byte. RED was committed before implementation.
- GREEN: The focused timing regression failed against the earlier evaluator and passed after the correction. The final local workflow passed: 357/357 tests, with only the two existing RetroArch checks skipped because this host has no RetroArch setup. No separate refactor was needed.

## Hosted Evidence

- Original Task 2 CI run: https://github.com/szTheory/nesturbator/actions/runs/37992224142. Its head SHA was `4fce43491b1b058491eff7cb2fe1fa68b85a036e`; it passed before review found the first-eight selection timing defect.
- Remediation CI run: https://github.com/szTheory/nesturbator/actions/runs/37994563525. Its `headSha` exactly matches `20331371c8036cd5fccec6491d25262e3a2c0364`; conclusion: `success`.
- All six Linux, macOS, and Windows x64/arm64 builds succeeded on the remediation SHA. `hash-equality`, `retroarch-e2e`, and `CI required` each succeeded; ASan, hygiene, no-FP, and Conventional Commit title checks also succeeded.
- Retained artifact `retroarch-e2e-frames` was downloaded and inspected: `run/runner.ppm` (184335 bytes), `run/shot.png` (1579 bytes), `run/shot.bmp` (184374 bytes), and `asset-evidence.txt` (285 bytes). The evidence records matching expected and actual SHA-256 values for pinned RetroArch 1.22.2.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Widened the regression fixture's OAM index**
- **Found during:** Task 2 hosted Linux build on run 37991729856.
- **Issue:** GCC's conversion warning was an error for a narrowed `uint16_t` index expression.
- **Fix:** Used `uint32_t` for the bounded fixture index.
- **Verification:** Local CI passed and the next hosted Linux build succeeded.
- **Committed in:** `d0b4932`.

**2. [Rule 3 - Blocking] Applied the required clang-format layout**
- **Found during:** Task 2 hosted hygiene job on run 37991987014.
- **Issue:** Two line breaks failed `hygiene.format`.
- **Fix:** Adjusted the test expression layout.
- **Verification:** Local CI passed and the final hosted hygiene job succeeded.
- **Committed in:** `4fce434`.

## Issues Encountered

The local RetroArch checks skipped because RetroArch was unavailable in the local environment. The exact-SHA hosted RetroArch job passed and retained the capture artifact.

The original gap fix reached the ninth sprite 48 dots early. Selection now takes eight dots per selected sprite, so the ninth Y read/compare occurs at dots 129/130. The post-fix code review is clean; the disposition records the sprite findings and duplicate-comment cleanup as fixed, with audio/reset observations deferred outside Phase 03.

## User Setup Required

None.

## Next Phase Readiness

The structured Phase 03 sprite-overflow gap has implementation and hosted evidence for verification. Phase 03 can proceed to `$gsd-verify-work 03`.

## Self-Check: PASSED

- All five modified source, test, and documentation files exist; all five task/remediation commits are ancestors of HEAD.
- The 36-row generated inventory matches the committed file byte for byte.
- The hosted run's `headSha`, successful jobs, and retained capture artifact were checked directly.
