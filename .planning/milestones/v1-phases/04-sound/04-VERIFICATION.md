---
phase: 04-sound
verified: 2026-10-09T22:33:58Z
status: passed
score: 15/15 plan must-haves verified; 4/4 roadmap success criteria verified
covered_files:
  - .github/workflows/ci.yml
  - .planning/phases/04-sound/04-01-PLAN.md
  - .planning/phases/04-sound/04-01-SUMMARY.md
  - .planning/phases/04-sound/04-02-PLAN.md
  - .planning/phases/04-sound/04-02-SUMMARY.md
  - .planning/phases/04-sound/04-03-PLAN.md
  - .planning/phases/04-sound/04-03-SUMMARY.md
  - .planning/phases/04-sound/04-04-PLAN.md
  - .planning/phases/04-sound/04-04-SUMMARY.md
  - CMakeLists.txt
  - README.md
  - include/nesturbator.h
  - runner/CMakeLists.txt
  - runner/audio_hash.c
  - runner/audio_hash.h
  - runner/main.c
  - src/apu.c
  - src/bus.c
  - src/cartridge.c
  - src/frame.c
  - src/instance.c
  - src/internal.h
  - src/synth.c
  - tests/CMakeLists.txt
  - tests/accuracy/scoreboard-main.txt
  - tests/accuracy/scoreboard.txt
  - tests/accuracy/test_scoreboard.c
  - tests/cmake/hash_inventory.cmake
  - tests/cmake/write_hashes.cmake
  - tests/core/test_apu.c
  - tests/core/test_synth.c
  - tests/libretro/libretro_host.c
  - tests/runner/hashes.txt
  - tests/runner/test_audio_hash.c
  - tests/runner/test_spectral.c
covered_digest: "v3:sha256:3c4c17083a5d04789e798196fee7f4aa2f6894586ad9833118c0614d287a58e8"
behavior_unverified: 0
overrides_applied: 0
---

# Phase 4: Sound Verification Report

**Phase Goal:** NROM games play with sound in RetroArch, and the audio is identical on every platform.
**Verified:** 2026-10-09T22:33:58Z
**Status:** passed
**Re-verification:** Yes — refreshed after the later Phase 03 sprite-timing and Phase 04.1 trainer changes. The complete local CI workflow passed on source commit `20331371c8036cd5fccec6491d25262e3a2c0364`: 355 tests passed, 2 RetroArch launch tests skipped because RetroArch is unavailable locally, 0 failed. Hosted CI run [37994563525](https://github.com/szTheory/nesturbator/actions/runs/37994563525) on the same source SHA passed six platform builds, cross-platform hash equality, RetroArch E2E capture, and the required aggregate. Later commits through HEAD contain planning documentation only.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | APU channels produce documented sequences; licensed NROM audio has pinned hashes; libretro delivers the same samples as the core. | ✓ VERIFIED | `tests/core/test_apu.c` drives both pulse units, triangle, noise and DMC, and covers DMC fetch, frame sequencing and IRQ behavior. `runner.write_hashes` regenerates three licensed NROM runs and `runner.write_hashes.content` compares the dual-hash inventory to committed `tests/runner/hashes.txt`. `libretro.host` runs an audible synthetic NROM through the adapter and a direct core instance, then compares every left and right sample across two frames. The full local workflow passed the relevant tests. |
| 2 | `nesturbator-run --hash-audio` emits canonical transition and PCM hashes whose representation is platform-independent. | ✓ VERIFIED | `runner/main.c` installs the per-instance APU observer and prints separate digests. `runner/audio_hash.c` serializes cycle/level records and signed PCM explicitly in little-endian byte order; `tests/runner/test_audio_hash.c` pins empty, event ordering and PCM byte cases. The no-cartridge known answer and three game baselines passed. Hosted CI run [37994563525](https://github.com/szTheory/nesturbator/actions/runs/37994563525) passed Linux, macOS and Windows x64/arm64, six-platform hash equality, RetroArch E2E capture, and the required aggregate on source SHA `2033137`. The commits after that source revision contain planning documentation only.
| 3 | The committed AccuracyCoin scoreboard passes the six required sound tests. | ✓ VERIFIED | `tests/accuracy/scoreboard.txt` and `scoreboard-main.txt` contain the six exact named passing rows; `accuracycoin.page14` passed and its runner validates result names and RAM locations. |
| 4 | Five reference tones meet the non-harmonic peak threshold below 16 kHz. | ✓ VERIFIED | `runner.spectral` measures pulse periods 100, 40, 12, 8 and triangle period 1 using the documented integer FFT/window rules. It passed; recorded results range from -85.46 dB to -91.83 dB, below the -80 dB limit. |

**Score:** 4/4 roadmap success criteria and 15/15 plan must-have truths verified; 0 behavior-unverified.

### Required Artifacts

| Artifact | Expected | Status | Details |
|---|---|---|---|
| `src/apu.c`, `src/internal.h`, `src/bus.c` | Instance APU state and CPU-cycle register/timer/DMA behavior | ✓ VERIFIED | Substantive APU implementation is invoked from bus progression and register decode; `core.apu` covers channel, frame-counter, IRQ and DMC paths. |
| `src/synth.c`, `src/frame.c` | Integer transition synthesis and caller-owned PCM output | ✓ VERIFIED | Root CMake registers `src/synth.c`; production APU transitions enter the bounded per-instance kernel and frame API. `core.synth`, `core.apu` and `runner.spectral` passed. The earlier mixer-table header was consolidated into `src/synth.c`. |
| `runner/main.c`, `runner/audio_hash.c`, `runner/audio_hash.h` | Canonical dual audio hash CLI | ✓ VERIFIED | Private APU observer and frame PCM feed separate streaming SHA-256 states; known-answer and silence tests passed. |
| `tests/core/test_apu.c`, `tests/core/test_synth.c` | Channel, cycle, transition ordering and synthesis tests | ✓ VERIFIED | Registered in CTest and passed; tests exercise transition ordering and real production paths. |
| `tests/runner/test_audio_hash.c`, `tests/runner/test_spectral.c` | Hash encoding and spectral quality checks | ✓ VERIFIED | Both registered in `tests/CMakeLists.txt`; both passed. |
| AccuracyCoin scoreboards and page-14 test | Six explicit passing sound rows enforced in CI | ✓ VERIFIED | Exact names appear in both snapshots; `accuracycoin.page14` passed. |
| `tests/runner/hashes.txt`, `tests/cmake/write_hashes.cmake` | Three licensed-game frame/audio baseline pairs | ✓ VERIFIED | Writer runs the games and the fixture-required content comparison passed. The 30 existing frame rows are retained alongside six audio rows. |
| `tests/libretro/libretro_host.c` | Adapter stereo samples equal direct-core mono samples | ✓ VERIFIED | The test performs sample-by-sample equality assertions on both stereo channels for two audible frames; passed. |
| `README.md`, `include/nesturbator.h` | Public audio behavior and deterministic output contract | ✓ VERIFIED | Both document mono PCM, cadence, synthesis and reproducibility; current source comments describe the same API behavior. |

### Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| CPU bus cycle/register path | Instance APU state | Calls APU clock/write functions on the same instance | ✓ WIRED | `core.apu` passed channel/frame/DMC timing cases. |
| APU transition producer | Synthesizer | Synchronous call after transition observer | ✓ WIRED | `src/apu.c` calls the observer then `nesturbator__synth_transition`; observer order and synthesis tests passed. |
| Synthesizer | Public frame buffer | Bounded staging ring and frame drain | ✓ WIRED | Frame and synth coverage plus the silent hash test passed. |
| Runner hash callback | APU transition producer | Private per-instance callback registration | ✓ WIRED | Audio-hash known answers, no-cartridge digest and game baselines passed. |
| PCM frame output | Canonical PCM digest | Signed 16-bit samples serialized low byte then high byte | ✓ WIRED | Hash tests pin negative sample encoding; game and silent hashes passed. |
| Core mono PCM | Libretro stereo callback | Adapter duplicates each sample to left and right | ✓ WIRED | `libretro.host` compared every sample over two audible frames. |
| AccuracyCoin page-14 RAM values | Required scoreboard rows | Named test lookup, parse and CTest | ✓ WIRED | `accuracycoin.page14` passed all six required row checks. |
| Test registrations | `ci` workflow | CMake preset and six-platform workflow matrix | ✓ WIRED | All selected CTests are registered. Workflow matrix lists Linux/macOS/Windows x64/arm64; remote job results were not part of this local verification. |

### Data-Flow Trace (Level 4)

| Artifact | Data variable | Source | Produces real data | Status |
|---|---|---|---|---|
| `src/frame.c` | caller PCM | APU mixed levels through `src/synth.c` | Yes; integer synthesizer writes signed samples | ✓ FLOWING |
| `runner/main.c` | transition hash | Production APU callback `(cycle, level)` events | Yes; canonical bytes streamed to SHA-256 | ✓ FLOWING |
| `runner/main.c` | PCM hash | Public frame API audio buffer | Yes; explicit little-endian sample bytes | ✓ FLOWING |
| Libretro adapter | stereo callback samples | Same core frame PCM | Yes; host test observes actual non-silent callback output | ✓ FLOWING |
| `tests/runner/hashes.txt` | expected game hashes | Committed regression baselines | Yes; runner output is compared against these literal expected values | ✓ FLOWING |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|---|---|---|---|
| APU channel, frame, DMC and IRQ behavior | `ctest --test-dir build/ci --output-on-failure -R 'core\.apu|core\.synth|runner\.audio_hash|runner\.hash_audio\.silent|runner\.spectral|runner\.write_hashes\.content|libretro\.host|accuracycoin\.page14'` | Passed | ✓ PASS |
| Licensed game audio baselines | Same focused CTest invocation (`runner.write_hashes` fixture) | Passed; generated output matched committed hashes | ✓ PASS |
| Libretro stereo parity | Same focused CTest invocation (`libretro.host`) | Passed; all samples equal in both channels over two audible frames | ✓ PASS |
| Five-tone spectral gate | Same focused CTest invocation (`runner.spectral`) | Passed; all five peaks below -80 dB | ✓ PASS |

The current `cmake --workflow --preset ci` run passed 355 tests, skipped `retroarch.testframe` and `retroarch.game` because RetroArch was unavailable locally, and had 0 failures; it also generated all three package archives. Hosted run [37994563525](https://github.com/szTheory/nesturbator/actions/runs/37994563525) passed the pinned RetroArch E2E capture and six-platform builds, so no acceptance check remains pending. `libretro.host` independently passed sample-by-sample stereo parity without an audio device.

### Review Gate

The existing Phase 04 review found no findings across its 26-file scope. All eight files changed since that review are covered by the later clean Phase 03 review (sprite timing) and Phase 04.1 review (trainer RAM and bus mapping); both report zero critical, warning, and info findings. No review finding remains open.

### Requirements Coverage

| Requirement | Source Plans | Status | Evidence |
|---|---|---|---|
| SND-01 | 04-01, 04-02, 04-04 | ✓ SATISFIED | Channel sequences, three game baselines and sample-by-sample libretro parity passed; the requirement checklist and traceability row both mark SND-01 complete. |
| SND-02 | 04-04 | ✓ SATISFIED | Canonical serialization tests, silence digest and game-level dual hashes passed; the workflow matrix wires the same checks on all six platforms. |
| SND-03 | 04-02 | ✓ SATISFIED | Six required AccuracyCoin named rows are committed and `accuracycoin.page14` passed. |
| SND-04 | 04-03 | ✓ SATISFIED | Five-tone `runner.spectral` CI gate passed below -80 dB. |

No SND requirement mapped to Phase 04 is omitted from the plan requirements fields or left unchecked in `REQUIREMENTS.md`.

### Decision Coverage

All trackable CONTEXT decisions are honored: 8/8. The GSD decision-coverage check found no unhonored decisions.

### Test Quality Audit

| Test File/Set | Linked Requirement | Active | Disabled | Circular expected-value generation | Assertion level | Verdict |
|---|---|---:|---:|---:|---|---|
| `tests/core/test_apu.c` | SND-01 | Yes | 0 | No | Value and ordered timing assertions | ✓ ADEQUATE |
| `tests/accuracy/scoreboard.txt`, `accuracycoin.page14` | SND-03 | Yes | 0 | No | Exact named RAM-derived results and committed row comparisons | ✓ ADEQUATE |
| `tests/runner/test_audio_hash.c` | SND-02 | Yes | 0 | No | Known SHA-256 answers over canonical bytes | ✓ ADEQUATE |
| `tests/runner/test_spectral.c` | SND-04 | Yes | 0 | No | Measured peak values compared with numeric threshold | ✓ ADEQUATE |
| `tests/runner/hashes.txt`, `runner.write_hashes.content` | SND-01, SND-02 | Yes | 0 | No | Exact regression hash comparison | ✓ ADEQUATE |
| `tests/libretro/libretro_host.c` | SND-01 | Yes | 0 | No | Sample-by-sample behavioral parity | ✓ ADEQUATE |

The hash fixture records a regression baseline, not an independent acoustic oracle; the requirement asks that committed-game output match recorded hashes. The libretro/core parity test independently exercises the adapter and direct-core paths but intentionally does not exercise a physical audio device. No disabled requirement tests or test-only generator that writes its own expected fixture was found. No blocking debt markers, placeholders or incomplete production implementations were found in phase files.

### Probe Execution

Not applicable. The plans discuss edge probes and include test results, but declare no runnable probe scripts; the phase is not a migration/tooling phase.

### Human Verification Required

None. The behavior contract is deterministic core PCM and libretro callback sample parity, and both are covered by automated checks without audio hardware. RetroArch's local launch checks self-skipped on this host; the CTest host-level adapter check exercised audible stereo output. No physical listening or visual judgment is part of the project's acceptance contract.

### Advisory (New Scope, Unevidenced)

None. The prior passed report had no `gaps:` section, and this refresh found no new-scope implementation concern.

### Gaps Summary

The implementation satisfies all four roadmap criteria and all 15 plan truths. The full local workflow passed 355 tests with only the two unavailable local RetroArch launch tests skipped; hosted CI run 37994563525 passed all six platform builds, cross-platform hash equality, pinned RetroArch E2E, sanitizers, no-float, hygiene, and the required aggregate. No behavior-dependent human verification remains.

---

_Verified: 2026-10-09T22:33:58Z_
_Verifier: Codex (inline; verifier-agent dispatch is unavailable under this session's no-subagent policy)_
