---
phase: 20-nmi-line-and-vblank-flag
reviewed: 2026-10-27T01:50:21Z
depth: standard
files_reviewed: 40
files_reviewed_list:
  - README.md
  - include/nesturbator.h
  - src/bus.c
  - src/cpu.c
  - src/internal.h
  - src/ppu.c
  - src/state.c
  - tests/CMakeLists.txt
  - tests/accuracy/baseline-metadata.txt
  - tests/accuracy/measurements.md
  - tests/accuracy/migration.tsv
  - tests/accuracy/scoreboard.txt
  - tests/cmake/accuracycoin_activation.cmake
  - tests/cmake/accuracycoin_baseline_fetch.cmake
  - tests/cmake/accuracycoin_driver.cmake
  - tests/cmake/accuracycoin_nmi_targets_control.cmake
  - tests/cmake/accuracycoin_regression_controls.cmake
  - tests/cmake/nmi_targets.cmake
  - tests/cmake/runner_state.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/core/test_api.c
  - tests/core/test_nmi_poll.c
  - tests/core/test_nmi_state.c
  - tests/core/test_state.c
  - tests/core/test_state_fork.c
  - tests/core/test_state_reject.c
  - tests/core/test_state_runahead.c
  - tests/fuzz/state_loader.c
  - tests/header/header_c.c
  - tests/header/header_cxx.cpp
  - tests/libretro/libretro_host.c
  - tests/nmi_fixture.h
  - tests/ppu/test_nmi_races.c
  - tests/ppu/test_registers.c
  - tests/ppu/test_sprites.c
  - tests/retroarch/frame_index.c
  - tests/runahead_model.h
  - tests/runner/hashes.txt
  - tests/runner/nmi_fixture.c
  - tests/state_test.h
findings:
  critical: 0
  warning: 2
  info: 0
  total: 2
status: issues_found
---

# Phase 20: Code Review Report

**Reviewed:** 2026-10-27T01:50:21Z  
**Depth:** standard  
**Files Reviewed:** 40  
**Status:** issues_found

## Summary

The production NMI capture, instruction polling, and serialized history paths are internally consistent with the selected phase model. I found two warnings: a stale public API statement about disabling NMI, and a split-status regression fixture that does not make sprite status change between its early and late samples. No blocker was identified. The phase's documented hosted-evidence gap and unresolved exact-tie physical behavior are not source defects.

## Narrative Findings (AI reviewer)

### WR-01: Public API comment says disabling NMI cancels a captured request

**Severity:** WARNING  
**File:** `include/nesturbator.h:258-261`  
**Issue:** The `nesturbator_frame` comment says clearing PPUCTRL bit 7 “suppresses a pending NMI.” In the implemented model, `nmi_pending` records a captured edge and survives PPUCTRL disable; the same header states this explicitly at lines 513-517, and README.md describes the same behavior. Consumers following the stale public comment can make incorrect assumptions about save-state continuation or whether a later disable cancels an interrupt already captured by the CPU.
**Fix:** Replace the sentence with wording that distinguishes the live line from captured history: clearing bit 7 lowers the PPU NMI line and prevents a future capture, but does not revoke a request already captured by the CPU.

### WR-02: Split `$2002` tests do not exercise changing sprite flags across M2

**Severity:** WARNING  
**File:** `tests/ppu/test_nmi_races.c:136-158,168-185,195-221`  
**Issue:** `assert_m2_split_witness` advances PPU dots directly and checks only a vblank transition; it does not read `$2002`. The production bus-read cases do check `$2002`, but seed sprite flags to `0x60` for set and `0x00` for clear and leave those bits unchanged across the cycle. Consequently, those checks pass if sprite bits are sampled at M2 rise instead of the required M2 fall, so the local C regression fixture does not protect the split sampling behavior against an implementation regression.
**Fix:** Add a production-decoder `$2002` read case where bit 5 or bit 6 changes after M2 rise and before M2 fall, then assert that bit 7 reflects the early sample while sprite bits reflect the late sample.

---

_Reviewed: 2026-10-27T01:50:21Z_  
_Reviewer: the agent (gsd-code-reviewer)_  
_Depth: standard_
