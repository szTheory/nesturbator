---
phase: 24-correct-remaining-ntsc-dma-behavior
reviewed: 2026-11-01T01:30:02Z
depth: standard
files_reviewed: 32
files_reviewed_list:
  - AGENTS.md
  - CMakePresets.json
  - README.md
  - docs/api.md
  - docs/progress.md
  - docs/reproduction.md
  - docs/testing.md
  - include/nesturbator.h
  - src/apu.c
  - src/bus.c
  - src/internal.h
  - src/state.c
  - tests/accuracy/baseline-metadata.txt
  - tests/accuracy/measurements.md
  - tests/accuracy/scoreboard.txt
  - tests/cmake/accuracycoin_activation.cmake
  - tests/cmake/accuracycoin_baseline_fetch.cmake
  - tests/cmake/accuracycoin_regression_controls.cmake
  - tests/cmake/phase20_hosted_evidence_controls.cmake
  - tests/cmake/phase21_hosted_evidence_controls.cmake
  - tests/cmake/reproduction_contract.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/core/test_api.c
  - tests/core/test_apu.c
  - tests/core/test_state_fork.c
  - tests/core/test_state_reject.c
  - tests/cpu/dma_abort_sweep.h
  - tests/cpu/test_bus.c
  - tests/header/header_c.c
  - tests/header/header_cxx.cpp
  - tests/ppu/test_sprites.c
  - tests/runner/hashes.txt
findings:
  critical: 0
  warning: 1
  info: 0
  total: 1
status: issues_found
---

# Phase 24: Code Review Report

**Reviewed:** 2026-11-01T01:30:02Z  
**Depth:** standard  
**Files Reviewed:** 32  
**Status:** issues_found

## Summary

The supplied 32-file scope was reviewed. The evaluation-scope resolver was degraded (`phase-range`, no task-commit rows), so this review used the complete supplied file list. The bus regression observer can silently truncate a long test trace before assertions inspect its final DMA events.

## Narrative Findings (AI reviewer)

### Warnings

### WR-01: Long DMA timing test silently loses the trace tail

**Severity:** WARNING  
**File:** `tests/cpu/test_bus.c:46`  
**Issue:** `dma_observe` returns without recording or reporting overflow once the 32,768-event buffer is full. `run_landing_test9_reload_boundary` resets the trace before a verified 30,012-cycle wait, then checks the final `$FFC0` sample GET after that wait. The bus observer records two APU events per CPU cycle, so this path produces over 60,024 events before the tail checks; those checks therefore cannot inspect a complete trace and can fail or give incomplete evidence depending on where the cap is reached.
**Fix:** Reset the trace after the 30,012-cycle wait and before the final DMA sequence, and track an overflow flag that fails any trace-dependent assertion. Alternatively, size the buffer for the full run plus bus events and still fail explicitly on overflow.

---

_Reviewed: 2026-11-01T01:30:02Z_  
_Reviewer: the agent (gsd-code-reviewer)_  
_Depth: standard_
