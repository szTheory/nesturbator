---
phase: 02-the-cpu-matches-the-public-vectors
reviewed: 2026-10-09T13:13:04Z
depth: standard
files_reviewed: 60
files_reviewed_list:
  - .github/workflows/ci.yml
  - .github/workflows/nightly.yml
  - .github/workflows/release.yml
  - .release-please-manifest.json
  - CHANGELOG.md
  - CMakeLists.txt
  - README.md
  - include/nesturbator.h
  - libretro/libretro.c
  - libretro/nesturbator_libretro.info
  - runner/CMakeLists.txt
  - runner/audio_hash.c
  - runner/audio_hash.h
  - runner/main.c
  - runner/movie.c
  - runner/movie.h
  - src/apu.c
  - src/bus.c
  - src/cartridge.c
  - src/cpu.c
  - src/frame.c
  - src/instance.c
  - src/internal.h
  - src/palette_ntsc.c
  - src/ppu.c
  - src/synth.c
  - tests/CMakeLists.txt
  - tests/accuracy/scoreboard-main.txt
  - tests/accuracy/scoreboard.txt
  - tests/accuracy/test_scoreboard.c
  - tests/cmake/fuzz_registration.cmake
  - tests/cmake/prepare_scoreboard_baseline.cmake
  - tests/cmake/release_credentials_policy.cmake
  - tests/cmake/release_policy.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/cmake/verify_scoreboard_regression.cmake
  - tests/cmake/write_hashes.cmake
  - tests/core/test_api.c
  - tests/core/test_apu.c
  - tests/core/test_cartridge.c
  - tests/core/test_controller.c
  - tests/core/test_palette.c
  - tests/core/test_synth.c
  - tests/cpu/test_vectors.c
  - tests/fuzz/rom_loader.c
  - tests/libretro/libretro_host.c
  - tests/ppu/test_registers.c
  - tests/ppu/test_render.c
  - tests/ppu/test_sprites.c
  - tests/retroarch/CMakeLists.txt
  - tests/retroarch/run_retroarch.cmake
  - tests/roms/manifest.txt
  - tests/runner/game_movie.c
  - tests/runner/hashes.txt
  - tests/runner/movie_fixture.h
  - tests/runner/test_audio_hash.c
  - tests/runner/test_movie.c
  - tests/runner/test_spectral.c
  - tests/test_process.h
  - tools/palgen/palgen.c
  - version.txt
findings:
  critical: 1
  warning: 0
  info: 0
  total: 1
status: issues_found
---

# Phase 02: Code Review Report

**Reviewed:** 2026-10-09T13:13:04Z
**Depth:** standard
**Files Reviewed:** 60 text files
**Status:** issues_found

## Summary

Reviewed the 60 text files in the supplied 73-path scope at standard depth. The scope provenance is degraded (`phase-range`, reason `no-reachable-task-commits`) and uses base `c3e96382ae991d18f69f88096611d42e6a076e0e`; as supplied, it includes work after Phase 02, so findings cannot be attributed exclusively to that phase. Excluded five `.nes` ROMs and eight fuzz corpus samples from content inspection because they are binary assets. No tests were run.

The libretro adapter drops audio for frontends that provide only the single-sample callback, despite storing that callback.

## Critical Issues

### CR-01: Single-sample libretro audio callback is never called

**File:** `libretro/libretro.c:58-62,206-208`
**Classification:** BLOCKER
**Issue:** `retro_set_audio_sample` saves `audio_cb`, but `retro_run` emits samples only when `audio_batch_cb` is non-NULL. A conforming host that supplies the single-sample callback without a batch callback receives no audio for every frame.
**Fix:** Use the batch callback when available and otherwise deliver each mono sample to the registered callback as stereo:

```c
if (audio_batch_cb != NULL) {
    (void)audio_batch_cb(stereo, io.audio_count);
} else if (audio_cb != NULL) {
    for (i = 0; i < io.audio_count; i++) {
        audio_cb(mono[i], mono[i]);
    }
}
```

---

_Reviewed: 2026-10-09T13:13:04Z_
_Reviewer: the agent (gsd-code-reviewer)_
_Depth: standard_
