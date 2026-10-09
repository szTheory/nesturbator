---
phase: 01-a-test-frame-in-retroarch
reviewed: 2026-10-09T11:40:58Z
depth: standard
files_reviewed: 31
files_reviewed_list:
  - .release-please-manifest.json
  - CHANGELOG.md
  - CMakeLists.txt
  - README.md
  - include/nesturbator.h
  - libretro/nesturbator_libretro.info
  - runner/CMakeLists.txt
  - runner/audio_hash.c
  - runner/audio_hash.h
  - runner/main.c
  - src/apu.c
  - src/bus.c
  - src/cartridge.c
  - src/cpu.c
  - src/frame.c
  - src/instance.c
  - src/internal.h
  - src/synth.c
  - tests/CMakeLists.txt
  - tests/accuracy/scoreboard-main.txt
  - tests/accuracy/scoreboard.txt
  - tests/cmake/vector_api_policy.cmake
  - tests/cmake/write_hashes.cmake
  - tests/core/test_api.c
  - tests/core/test_apu.c
  - tests/core/test_synth.c
  - tests/libretro/libretro_host.c
  - tests/runner/hashes.txt
  - tests/runner/test_audio_hash.c
  - tests/runner/test_spectral.c
  - version.txt
findings:
  critical: 1
  warning: 0
  info: 0
  total: 1
status: issues_found
scope_status: degraded
---

# Phase 01: Code Review Report

**Reviewed:** 2026-10-09T11:40:58Z
**Depth:** standard
**Files Reviewed:** 31
**Status:** issues_found

## Summary

Reviewed the supplied 31-file scope at standard depth. The evaluation-scope resolver reported degraded coverage because Phase 01 task commits were not reachable, so this review covers the reviewable files changed since the previous Phase 01 review. A blocker exists in cartridge loading: images marked as containing an iNES trainer are accepted, but trainer bytes are skipped instead of initialized into CPU RAM, so affected games boot with incorrect state.

## Critical Issues

### CR-01: BLOCKER — Accepted iNES trainers are silently discarded

**File:** `src/cartridge.c:151-170`
**Issue:** `validate_image` accepts the iNES trainer flag and `offset` skips the 512 trainer bytes before pointing `cart.prg` at PRG ROM. The accepted trainer bytes are then never copied into the CPU-visible `$7000-$71FF` PRG-RAM area. An image relying on its trainer therefore loads successfully but starts with different memory contents than the cartridge specifies. The public header explicitly promises that trainers are accepted (`include/nesturbator.h:227-231`), making this a supported-input correctness failure.
**Fix:** Either implement the supported mapper-0 PRG-RAM window and copy the trainer bytes into `$7000-$71FF` during load, or reject trainer-flagged images until that memory is implemented. Do not report successful loading while dropping the trainer.

---

_Reviewed: 2026-10-09T11:40:58Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
