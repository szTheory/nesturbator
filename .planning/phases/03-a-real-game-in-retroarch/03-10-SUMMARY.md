---
phase: 03-a-real-game-in-retroarch
plan: 10
subsystem: testing
tags: [c17, cmake, ctest, libfuzzer, asan, ubsan]
requires:
  - phase: 03-02
    provides: bounded public cartridge load and unload entry points
provides:
  - deterministic ROM loader corpus replay in regular CI
  - bounded Linux nightly libFuzzer target using the same entry point
affects: [03-11, 03-12, 03-13]
actuals:
  tokens: 21692
  tasks: 1
  commits: 2
commits: 2
plan_head_before: d813db3ed6bbda1f7b1c102bc74ebdecb8f5b58e
plan_head_after: c16e82ec9f8a19ae9656927ab93819c727e44a7c
tech-stack:
  added: []
  patterns: [shared-libfuzzer-and-corpus-replay-entrypoint, manifest-listed-synthetic-seeds]
key-files:
  created:
    - tests/cmake/fuzz_registration.cmake
    - tests/fuzz/rom_loader.c
    - tests/fuzz/corpus/empty
    - tests/fuzz/corpus/single-byte
    - tests/fuzz/corpus/truncated-header
    - tests/fuzz/corpus/header-only
    - tests/fuzz/corpus/mapper-bits
    - tests/fuzz/corpus/short-prg
    - tests/fuzz/corpus/valid-nrom
    - tests/fuzz/corpus/trailing-byte
  modified:
    - tests/CMakeLists.txt
    - tests/roms/manifest.txt
    - .github/workflows/nightly.yml
    - README.md
key-decisions:
  - "Replay and libFuzzer call the same LLVMFuzzerTestOneInput function, which exercises public cartridge load, unload, and instance destruction."
  - "Use synthetic, manifest-listed corpus bytes and do not fetch a ROM or fuzz corpus at runtime."
  - "Require Clang for the optional libFuzzer build and instrument the core with fuzzer coverage plus ASan/UBSan in Linux nightly."
requirements-completed: [GAME-05]
coverage:
  - id: D1
    description: "Deterministic cartridge corpus replay is registered in CTest and passes in regular CI."
    requirement: GAME-05
    verification:
      - kind: unit
        ref: "ctest: fuzz.regress"
        status: pass
    human_judgment: false
  - id: D2
    description: "Linux nightly builds and runs a bounded, sanitizer-instrumented libFuzzer target and fails when its target or corpus is absent."
    requirement: GAME-05
    verification:
      - kind: other
        ref: ".github/workflows/nightly.yml rom-loader-fuzz job; hosted run unavailable on local-only branch"
        status: unknown
    human_judgment: false
duration: 7min
completed: 2026-10-08
status: complete
---

# Phase 03 Plan 10: Cartridge loader fuzz gates summary

**Regular CI now replays a checked-in malformed cartridge corpus through the owned loader lifecycle, and Linux nightly runs the same entry point with bounded libFuzzer under ASan/UBSan.**

## Accomplishments

- Added a deterministic `fuzz.regress` CTest that replays empty, single-byte, truncated, malformed, short-length, valid-shaped, and trailing-byte synthetic seeds on each CI platform.
- Added a Clang-only `fuzz.rom_loader.libfuzzer` target. The nightly config instruments the core and fuzzer with libFuzzer coverage, ASan, and UBSan, then bounds the run to 60 seconds, a two-second input timeout, and 64 KiB per input.
- Made missing corpus files fail configuration and made the nightly job check for corpus files and the built executable before running it. The nightly report job now accounts for both vector and fuzzer results.
- Added SHA-256 manifest entries for every corpus seed and documented the recurring gates in README.

## Task Commits

1. **RED: Assert loader replay registration** — `e88be06` (`test`). `fuzz.registration` failed with exit 8 because the CTest inventory lacked `fuzz.regress`. The JUnit report was accepted by `gsd_run check tdd-red-evidence` as `RED_EVIDENCE_OK`; the failure was the intended missing-registration assertion.
2. **GREEN: Replay and fuzz cartridge corpus** — `c16e82e` (`feat`). `fuzz.regress` and `fuzz.registration` passed after adding the replay target and corpus.

## Verification

- `cmake --workflow --preset ci`: 333 of 334 tests passed. `fuzz.regress`, `fuzz.registration`, and `manifest.sha256` passed. `retroarch.testframe` failed with `Subprocess aborted` and empty stdout/stderr, matching the known local macOS GUI-session issue recorded in prior phase work.
- `scripts/hygiene.sh --tree`: passed.
- Workflow YAML parsing: passed.
- The local AppleClang 21 installation cannot link `libclang_rt.fuzzer_osx.a`; the libFuzzer target is Linux-only. No hosted nightly run is claimed because this branch is local-only and no exact GitHub Actions run exists.

## TDD Gate Compliance

- **RED:** `fuzz.registration` executed and failed on the assertion that `fuzz.regress` was missing. The classifier returned `RED_EVIDENCE_OK`; evidence is in ignored build output at `build/ci/fuzz-registration-red.xml` with its record at `build/ci/fuzz-registration-red.json`.
- **GREEN:** `fuzz.regress` replayed all corpus files and `fuzz.registration` found its registration; both passed in the final workflow.
- **REFACTOR:** None required.

## Deviations from Plan

### Verification limitation

- The hosted Linux nightly verification could not be run from this local-only branch. The workflow is configured fail-closed; hosted evidence remains unknown until an exact Actions run is available.
- The required local workflow completed with the known `retroarch.testframe` GUI abort; the new corpus and manifest tests passed.

## Next Phase Readiness

- `GAME-05` now has a regular CI corpus replay gate and a bounded Linux nightly mutation gate. Hosted nightly execution remains to be observed after the branch is pushed.

---
*Phase: 03-a-real-game-in-retroarch*
*Completed: 2026-10-08*

## Self-Check: PASSED

- The summary and all listed source, workflow, and corpus files exist.
- Task commits `e88be06` and `c16e82e` are ancestors of the plan head.
- The persisted plan ledger measures 2 task commits from `d813db3ed6bbda1f7b1c102bc74ebdecb8f5b58e` through `c16e82ec9f8a19ae9656927ab93819c727e44a7c`; the token actual includes the binary corpus payload bytes.
