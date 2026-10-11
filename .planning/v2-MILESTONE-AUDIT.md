---
milestone: v2
milestone_name: Most of the library plays
audited: 2026-10-11
status: tech_debt
scores:
  requirements: 19/19
  phases: 6/6
  integration: 13/13
  flows: 6/6
gaps:
  requirements: []
  integration: []
  flows: []
tech_debt:
  - phase: cross-phase
    items:
      - "G1: soft-reset tests miss UxROM, CNROM and AxROM; the MMC3 reset test does not assert that PRG RAM bytes survive; libretro reset parity uses only a synthetic NROM image (TUNE-06, BOARD-01, BOARD-03)"
      - "G2: retroarch-e2e round-trips a .srm for MMC1 only; MMC3 saves are covered by the simulated host only (SAVE-05, BOARD-03)"
      - "G3: the MMC3 IRQ is checked against the blargg ROMs only in the nightly mmc3-oracle lane (BOARD-03, MAP-02)"
      - "G4: AccuracyCoin has no libretro parity test, by design (D-14) (BOARD-04)"
      - "G6: v1 exports nesturbator_get_version and nesturbator_unload_cartridge have no runner or libretro consumer, and the README does not mention them"
  - phase: ci
    items:
      - "asan timeout-minutes 20 is provisional; set it to twice the measured 16m45s run (about 34)"
      - "nightly suite-flake timeout-minutes 30 is a ceiling; Backlog 999.1 lowers it to twice the measured cold run"
  - phase: 05-tune-up-and-v1-debt
    items:
      - "Review open: WR-01, WR-03 (APU frame-counter timing after reset is approximate), IN-01..03"
  - phase: 06-mapper-seam-and-ppu-fetch-pipeline
    items:
      - "Review open: WR-02 ($2007 treats v >= $4000 as non-palette), IN-01"
      - "Frame rate fell about 19 percent at the seam; measured, not gated"
  - phase: 07-uxrom-cnrom-and-axrom
    items:
      - "Review open: WR-01 (header, README and mapper_axrom.c say the AxROM reset vector comes from bank 0, true only at power-on), IN-01 (stale README status paragraph), IN-02, IN-03"
  - phase: 08-mmc1-and-battery-saves
    items:
      - "Review open: WR-01 (a failed interval flush is never retried at exit), WR-02 (MMC1 submapper 5 emulated as ordinary MMC1), WR-03 (a directory at the .sav path reports a size mismatch), IN-01, IN-02 (stale .tmp after a failed flush)"
  - phase: 09-mmc3
    items:
      - "Review open: IN-01, IN-02; WR-01 accepted, with an optional header sentence and a test that MMC3 ignores the header mirroring bit"
  - phase: 10-close-out-and-boot-to-play
    items:
      - "Review open: WR-01 (--rom-parity continues after a failed retro_load_game), WR-02, WR-03, IN-01..03, all in test code"
nyquist: skipped (validate-phase capability inactive)
---

# Milestone v2 audit: Most of the library plays

**Status:** tech_debt. All 19 requirements are satisfied, and all six phases (5–10) passed verification. Every cross-phase connection is wired and every end-to-end flow completes. No item blocks the milestone. The debt below is advisory.

Hosted evidence: CI, Nightly and Release are all green on `main` at d9084bf (PR #37), covering the six-platform matrix, asan, nofp, hygiene, hash equality and retroarch-e2e. That run confirms on hosted runners what Phase 5 (TUNE-01, TUNE-05) and Phase 10 (BOARD-04) had shown only locally.

## Requirements (three-source cross-reference)

| Requirement | Phase | VERIFICATION | SUMMARY claim | REQUIREMENTS.md | Final |
|---|---|---|---|---|---|
| TUNE-01 | 5 | passed | 05-04 | [x] | satisfied |
| TUNE-02 | 5 | passed | 05-04..07 | [x] | satisfied |
| TUNE-03 | 5 | passed | 05-01 | [x] | satisfied |
| TUNE-04 | 5 | passed | 05-02 | [x] | satisfied |
| TUNE-05 | 5 | passed | 05-03 | [x] | satisfied |
| TUNE-06 | 5 | passed | 05-05..07 | [x] | satisfied |
| MAP-01 | 6 | passed | 06-01 | [x] | satisfied |
| MAP-02 | 6 | passed | 06-02 | [x] | satisfied |
| MAP-03 | 9 | passed | 09-01, 02, 04..07 | [x] | satisfied |
| BOARD-01 | 7 | passed | 07-01 | [x] | satisfied |
| BOARD-02 | 8 | passed | 08-01..05, 07 | [x] | satisfied |
| BOARD-03 | 9 | passed | 09-01..03, 08..12 | [x] | satisfied |
| BOARD-04 | 10 | passed | 10-01, 05, 07 | [x] | satisfied |
| SAVE-01 | 8 | passed | 08-01, 02, 04 | [x] | satisfied |
| SAVE-02 | 8 | passed | 08-03, 06, 07 | [x] | satisfied |
| SAVE-03 | 8 | passed | 08-06 | [x] | satisfied |
| SAVE-04 | 8 | passed | 08-06 | [x] | satisfied |
| SAVE-05 | 8 | passed | 08-08 (VERIFICATION) | [x] | satisfied |
| PLAY-01 | 10 | passed | 10-01, 03, 05, 07 | [x] | satisfied |

No requirement is orphaned. SAVE-05 is missing from the 08-08 SUMMARY frontmatter, but 08-VERIFICATION credits it to 08-08 with retroarch-e2e evidence from run 38096603810, so I count it as satisfied.

## Phases

| Phase | Status | Score | Notes |
|---|---|---|---|
| 05 Tune-up and v1 debt | passed | 5/5 | The report body still reads human_needed. The pending hosted checks later passed: PR #25 merged green, and 05-CI-RECORD "After" shows 191 s against 181 s, within runner noise |
| 06 Mapper seam and PPU fetch pipeline | passed | 5/5 | |
| 07 UxROM, CNROM and AxROM | passed | 3/3 | |
| 08 MMC1 and battery saves | passed | 5/5 | |
| 09 MMC3 | passed | 4/4 | |
| 10 Close-out and boot-to-play | passed | 4/4 | Six-platform equality confirmed by hosted CI on d9084bf |

## Integration (gsd-integration-checker)

All 13 expected cross-phase connections are wired:

| Connection | Requirements |
|---|---|
| Loader → ops table, the only mapper-id switch (`src/cartridge.c`) | MAP-01, MAP-03 |
| PPU `set_bus` A12 edges → MMC3 counter | MAP-02, BOARD-03 |
| MMC3 IRQ → shared IRQ OR (`src/apu.c`) | BOARD-03 |
| `nesturbator_reset` keeps mapper state and the battery span | TUNE-06 |
| `retro_reset` → `nesturbator_reset` | TUNE-06 |
| Battery span for MMC1 and MMC3 PRG RAM | SAVE-01 |
| Runner save load before frame 1, save-if-changed and exit 4 | SAVE-02..04 |
| libretro SAVE_RAM | SAVE-05 |
| Parity tests for every board | BOARD-04 |
| Hash inventory drift guard | BOARD-04 |
| v2 API documented and consumed | SAVE-01..05 |
| Runner reject messages | MAP-03 |
| Boot-to-play | PLAY-01 |

## End-to-end flows

| Flow | Status |
|---|---|
| Load, run and hash a frame for each board (0, 1, 2, 3, 4, 7) | complete |
| Two `--save-dir` runs restore the save (MMC1 and MMC3) | complete |
| RetroArch `.srm` round trip | complete for MMC1; MMC3 runs through the simulated host only (G2) |
| Soft reset through libretro | complete; the reset tests cover only some boards (G1) |
| Rejected mapper gives a message and a non-zero exit | complete |
| Boot-to-play | complete |

## Tech debt

The frontmatter lists every item by phase. The ones worth acting on:
1. G1: reset-retention tests for UxROM, CNROM, AxROM and MMC3 PRG RAM. Project rule 6 requires a test for each behaviour.
2. 07 WR-01: the AxROM reset-vector docs say "bank 0" without "at power-on".
3. 08 WR-01: a failed interval flush is not retried at exit.
4. CI timeouts: asan should move to about 34 minutes, measured. Nightly suite-flake is Backlog 999.1.
