---
phase: 08-mmc1-and-battery-saves
verified: 2026-10-10T00:00:00Z
status: passed
score: 5/5 must-haves verified
covered_files:
  - ".planning/phases/08-mmc1-and-battery-saves/08-01-PLAN.md"
  - ".planning/phases/08-mmc1-and-battery-saves/08-01-SUMMARY.md"
  - ".planning/phases/08-mmc1-and-battery-saves/08-02-PLAN.md"
  - ".planning/phases/08-mmc1-and-battery-saves/08-02-SUMMARY.md"
  - ".planning/phases/08-mmc1-and-battery-saves/08-03-PLAN.md"
  - ".planning/phases/08-mmc1-and-battery-saves/08-03-SUMMARY.md"
  - ".planning/phases/08-mmc1-and-battery-saves/08-04-PLAN.md"
  - ".planning/phases/08-mmc1-and-battery-saves/08-04-SUMMARY.md"
  - ".planning/phases/08-mmc1-and-battery-saves/08-05-PLAN.md"
  - ".planning/phases/08-mmc1-and-battery-saves/08-05-SUMMARY.md"
  - ".planning/phases/08-mmc1-and-battery-saves/08-06-PLAN.md"
  - ".planning/phases/08-mmc1-and-battery-saves/08-06-SUMMARY.md"
  - ".planning/phases/08-mmc1-and-battery-saves/08-07-PLAN.md"
  - ".planning/phases/08-mmc1-and-battery-saves/08-07-SUMMARY.md"
  - ".planning/phases/08-mmc1-and-battery-saves/08-08-PLAN.md"
  - ".planning/phases/08-mmc1-and-battery-saves/08-08-SUMMARY.md"
  - "include/nesturbator.h"
  - "libretro/libretro.c"
  - "runner/main.c"
  - "runner/save.c"
  - "src/cartridge.c"
  - "src/mapper_mmc1.c"
  - "tests/CMakeLists.txt"
  - "tests/core/test_mapper_mmc1.c"
  - "tests/core/test_save.c"
covered_digest: "v3:sha256:e65cab087db6cacf8a7a68f57448c78292836bf60b139fed5df455ef747a72b9"
behavior_unverified: 0
overrides_applied: 0
---

# Phase 8: MMC1 and battery saves Verification Report

**Phase Goal:** MMC1 games run, and their battery saves persist across runs as raw `.sav` bytes through the runner and through RetroArch.
**Verified:** 2026-10-10
**Status:** passed
**Re-verification:** No, initial verification

## Goal Achievement

### Observable Truths (ROADMAP success criteria)

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | Holy Mapperel mapper 1 ROMs report `0000`; synthetic cases for adjacent-write ignore, reset bit, RAM enabled at power-on, SNROM/SOROM/SUROM/SXROM variants, registers and battery RAM kept across reset | VERIFIED | `src/mapper_mmc1.c` implements the write-stamp adjacency rule (`last_write`, zero-guarded), reset bit never ignored, Control stored XOR $0C so zeroed state is power-on $0C with RAM enabled, and variant banking by allocation size and PRG size. `tests/core/test_mapper_mmc1.c` has named cases: `hook_adjacency`, `hook_first_write_not_adjacent`, `hook_ram_write_then_serial`, `hook_reset_bit`, `cpu_inc_*` (CPU-level RMW), `reset_keeps_state`, `ram_enable`, `snrom_ram_disable`, `sorom_banking`, `sxrom_ram_banks`, `surom_sxrom_prg_halves`. CTest has `holymapperel.m1{sgrom,sjrom,skrom,surom}.decode` and `m1sxrom.run1/run2.decode` with `PASS_REGULAR_EXPRESSION "^holymapperel: code 0000 \("`. Local `cmake --workflow --preset ci` passes 417/417; CI run 38096603810 all jobs success. |
| 2 | `nesturbator_get_memory()` returns the battery span (NES 2.0 NV size; 8 KiB iNES 1; 32 KiB for mapper 1 over 256 KiB PRG), NULL/0 without battery; pointer stable load to unload; `nesturbator_save_generation()` rises only on writes reaching the span | VERIFIED | `include/nesturbator.h` documents the API (lines ~338-369). `tests/core/test_save.c`: `span_and_generation`, `no_ram_is_open_bus`, `arguments_and_empty_cases`, `lifetime_and_generation`, `sorom_span_offset`, `trainer_without_declared_ram`; `src/cartridge.c` lays out one allocation [V work][N NVRAM] and the loader size rows. `core.save` and `core.mapper_mmc1` are in the passing CTest list. |
| 3 | Holy Mapperel's MMC1 battery ROM passes on a second `--save-dir` run that starts from the first run's save; write via temp file and rename only when changed; no `--save-dir` means no save I/O; wrong-length `.sav` prints both sizes and exits 4, file byte-identical | VERIFIED | `runner/main.c` (load after `nesturbator_load_cartridge`, before first frame; `NESTURBATOR_RUN_SAVE_MISMATCH` returns 4) and `runner/save.c` (`.tmp`, fsync, `rename` / `MoveFileExA` with WRITE_THROUGH). Writes go through `save_if_changed` (generation plus memcmp against shadow). Tests: `holymapperel.m1sxrom.run1.dump` (cleans its dir), `run1.sav` (32768 bytes, marker `SAVEDATA` at offset 256), `run2.dump/decode/prgram` (fixture-gated on run1 `.sav`, regex requires `+ BATTERY`), `runner.save.{roundtrip,fresh,mismatch,mismatch_empty,unchanged,nodir,batteryless,unwritable,jam}`, `runner.save_path`, usage-error cases. All pass locally. |
| 4 | A CTest case stops `--save-interval N` after a flush and finds current bytes on disk | VERIFIED | `runner.save.interval` and `runner.save.interval_long` in `tests/CMakeLists.txt`; flush at `f % opt.save_interval == 0` in `runner/main.c:820`; both pass. |
| 5 | libretro core returns the span for `RETRO_MEMORY_SAVE_RAM`; `retroarch-e2e` shows RetroArch writing a `.srm` of span size and a second session loading it back | VERIFIED | `libretro/libretro.c:325-333` return `save_data`/`save_size` only for `RETRO_MEMORY_SAVE_RAM`, set in `retro_load_game` from `nesturbator_get_memory`, cleared on unload. `tests/retroarch/run_retroarch.cmake` asserts the `.srm` is 32768 bytes, `SAVEDATA` marker at 0x100, then session 2 loads it back. CI run 38096603810 retroarch-e2e job log: "RetroArch battery save round trip: M1_P512K_CR8K_S32K.srm written, loaded back, frames equal the runner's". |

**Score:** 5/5 truths verified (0 behavior-unverified)

### Requirements Coverage

| Requirement | Source Plans | Status | Evidence |
|-------------|--------------|--------|----------|
| BOARD-02 | 08-01, 02, 03, 04, 05, 07 | SATISFIED | MMC1 board, variants, Holy Mapperel 0000 decode, reject seeds |
| SAVE-01 | 08-01, 02, 04 | SATISFIED | Span API, `core.save` |
| SAVE-02 | 08-03, 06, 07 | SATISFIED | Runner `--save-dir`, Holy Mapperel run 2 |
| SAVE-03 | 08-06 | SATISFIED | Exit 4 on size mismatch, `runner.save.mismatch*` |
| SAVE-04 | 08-06 | SATISFIED | `runner.save.interval*` |
| SAVE-05 | 08-08 | SATISFIED | libretro span plus retroarch-e2e round trip |

All six IDs are claimed by at least one plan and by no unclaimed extras; no orphaned Phase 8 IDs in REQUIREMENTS.md. Final traceability is correct: REQUIREMENTS.md lines 33, 46, 50-53 are `[x]` and the traceability table (lines 113-120) shows all six as `Phase 8 | Complete`. The early check-offs by plans 01/02 were later confirmed by plans 03-08 that complete the remaining parts of the requirements; the end state is consistent.

### Behavioral Evidence

| Check | Result |
|-------|--------|
| `cmake --workflow --preset ci` (this verification) | 417/417 passed, package step OK |
| GitHub CI run 38096603810 | success on every job incl. 6 platform builds, asan, nofp x2, hygiene, hash-equality, retroarch-e2e, CI required |
| Nightly run 38096603867 | success per orchestrator (not re-fetched) |

### Anti-Patterns / Review Findings

No TBD/FIXME/XXX markers sought in the changed files beyond reading; none surfaced in the files read. Code review (08-REVIEW.md) has 3 warnings and 2 info items, all `open` in the disposition file:

| ID | Severity | Note |
|----|----------|------|
| WR-01 | Warning | `runner/main.c:261-272`: `sv->generation` is advanced before the flush, so after a failed interval flush the exit flush does not retry. Confirmed in code. The failure still sets exit status 1 and prints an error, so it is not silent; it loses a transient-fault retry. Does not contradict any success criterion. |
| WR-02 | Warning | NES 2.0 MMC1 submapper 5 accepted but emulated as ordinary MMC1. Outside the phase's criteria. |
| WR-03 | Warning | A directory at the `.sav` path reports a size mismatch. Edge case. |
| IN-01, IN-02 | Info | Interval test wall-clock timeout; stale `.tmp` after failed flush. |

These are advisory warnings, not blockers; recommend dispositioning them (fix WR-01 in a gap-closure or follow-up) before release.

### Human Verification Required

None. Every criterion is covered by an automated check run by `cmake --workflow --preset ci` or the hosted `retroarch-e2e` job.

### Gaps Summary

No gaps. The phase goal is achieved: MMC1 is implemented and tested at hook, CPU and ROM level; the battery span API, runner save/restore with atomic write, exit-4 mismatch, interval flush, and the libretro/RetroArch `.srm` round trip are all present, wired and exercised by passing tests.

---

_Verified: 2026-10-10_
_Verifier: Claude (gsd-verifier)_
