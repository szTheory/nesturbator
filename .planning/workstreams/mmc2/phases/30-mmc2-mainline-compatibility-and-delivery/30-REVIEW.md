---
phase: 30-mmc2-mainline-compatibility-and-delivery
reviewed: 2026-11-19T21:12:56Z
depth: standard
diff_base: 8923b66cca16739c5b402643798515cf5537ae22
files_reviewed: 23
files_reviewed_list:
  - CMakeLists.txt
  - README.md
  - docs/coordination/milestone-context-2026-11-03.md
  - docs/provider-ntsc.json
  - include/nesturbator.h
  - runner/main.c
  - src/cartridge.c
  - src/mapper.h
  - src/mapper_mmc2.c
  - src/ppu.c
  - tests/CMakeLists.txt
  - tests/accuracy/input-binding-v1.json
  - tests/cmake/accuracycoin_activation.cmake
  - tests/cmake/accuracycoin_input_fingerprint.cmake
  - tests/cmake/accuracycoin_input_fingerprint_controls.cmake
  - tests/cmake/provider_profile.cmake
  - tests/core/test_api.c
  - tests/core/test_cartridge.c
  - tests/core/test_mapper_mmc2.c
  - tests/core/test_state_reject.c
  - tests/libretro/libretro_host.c
  - tests/mmc2_fixture.h
  - tests/runner/hashes.txt
findings:
  critical: 0
  warning: 0
  info: 0
  total: 0
  necessity: 0
status: clean
---

# Phase 30: Code Review Report

## Summary

Reviewed the changed source, public claims, owned fixtures, parity assertions, state compatibility changes and current uncommitted evidence-binding implementation against the pinned mainline base. Traced the PPU hook through pattern reads and the mapper state visitor through validation, application and map rebuilding. The two initial verification robustness findings have been corrected and independently rechecked. No remaining actionable finding was established in this bounded follow-up review.

The review includes the in-progress Plan 30-03 binding changes observed at the follow-up timestamp. No project agent skills were configured. Necessity guidance was read; no necessity finding was established. This is a source-review verdict, not full workflow or delivery acceptance. Neither CTest nor a campaign nor the full workflow was run during the follow-up.

## Narrative Findings (AI reviewer)

No current findings.

## Resolved findings

The initial escaped-duplicate-member warning is resolved. `tests/cmake/accuracycoin_input_fingerprint.cmake:168` now rejects escaped member names before JSON decoding. The standalone controls include escaped duplicate top-level and current-input path members at `tests/cmake/accuracycoin_input_fingerprint_controls.cmake:212`. The fixed ASCII schema makes this rejection compatible with every legitimate member name. The initial CMake probe demonstrated why the former literal-key counting guard was insufficient; no acceptance run was used for that probe.

The initial restored-libretro audio-bound warning is resolved. The sample loop at `tests/libretro/libretro_host.c:4130` now requires both equality with the known direct sample count and a count within `AUDIO_CAPTURE_CAPACITY`. Direct-library restored execution also exits through cleanup on an oversized output count at line 4160 before sample comparison or PCM hashing. Cold direct execution already has the corresponding capacity check before its hash calls. The callbacks' counts remain assertions, so an invalid count still fails the test even when sample traversal is skipped.

## Follow-up scope and evidence

Rechecked mapper 9 support and format/revision clauses in the README, public header and runner help/diagnostics. They describe the bounded NES 2.0 submapper-0 profile and do not establish a commercial-title, hardware or consumer-integration claim. Recomputed the current binding's seven admitted product hashes, both external checker hashes and registration hash against the current files, with the existing release-version header normalization; all matched. Historical report and provenance constants remain pinned. The wider mapper/PPU/state implementation was also scanned against the base for additional concrete issues; none was established. Existing stale internal hook comments are inherited documentation debt and do not affect the runtime dispatch or the public support boundary.

---

Reviewed read-only; only this report was created. Product and evidence source files were not modified by the reviewer.
