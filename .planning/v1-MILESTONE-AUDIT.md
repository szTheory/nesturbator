---
milestone: 1
audited: 2026-10-09T17:09:01Z
status: gaps_found
scores:
  requirements: 17/19
  phases: 4/4
  integration: 17/19
  flows: 4/6
gaps:
  requirements:
    - id: GAME-01
      status: unsatisfied
      phase: 03-a-real-game-in-retroarch
      claimed_by_plans: ["03-01-PLAN.md", "03-02-PLAN.md", "03-09-PLAN.md"]
      completed_by_plans: ["03-01-PLAN.md", "03-02-PLAN.md", "03-09-PLAN.md"]
      verification_status: passed
      evidence: >-
        The loader accepts trainer-bearing images and advances the PRG pointer past the
        512-byte trainer, but does not initialize trainer bytes into CPU-visible RAM.
        The CPU bus does not map PRG RAM at $6000-$7FFF. The verification and summary
        cover trainer geometry, not execution semantics.
    - id: SND-01
      status: unsatisfied
      phase: 04-sound
      claimed_by_plans: ["04-01-PLAN.md", "04-02-PLAN.md", "04-04-PLAN.md"]
      completed_by_plans: ["04-01-PLAN.md", "04-04-PLAN.md"]
      verification_status: passed
      evidence: >-
        The libretro host test registers both audio callbacks and verifies the batch
        callback. retro_run only invokes audio_batch_cb, so a host using the supported
        single-sample callback receives no audio.
  integration:
    - id: GAME-01-trainer-memory
      status: broken
      severity: BLOCKER
      requirements: ["GAME-01"]
      from: "Validated iNES/NES 2.0 trainer bytes"
      to: "CPU-visible PRG RAM"
      evidence: >-
        src/cartridge.c:151-167 skips the trainer when selecting PRG and stores no
        reachable trainer region; src/bus.c leaves $6000-$7FFF unmapped.
    - id: SND-01-single-sample-audio
      status: broken
      severity: BLOCKER
      requirements: ["SND-01"]
      from: "Core PCM frame buffer"
      to: "libretro single-sample frontend callback"
      evidence: >-
        libretro/libretro.c:58-66 stores both callback types, while :202-208 only
        invokes audio_batch_cb. tests/libretro/libretro_host.c:632-635 registers
        both callbacks and therefore does not cover the fallback.
  flows:
    - name: "Trainer-bearing NROM load and execution"
      status: broken
      requirements: ["GAME-01"]
      broken_at: "Trainer bytes are not initialized into CPU-visible RAM."
    - name: "Libretro audio on a single-sample-only host"
      status: broken
      requirements: ["SND-01"]
      broken_at: "retro_run emits samples only through the batch callback."
---
# Milestone 1 Audit

**Audited:** 2026-10-09  
**Status:** gaps_found  
**Scope:** Milestone 1, phases 01–04

## Result

The audit found two concrete behavioral gaps that the phase verification reports did not catch. The four phase verification files exist and report passed; all 19 requirements appear in the traceability table, verification tables, and phase summary frontmatter. Cross-phase source inspection downgrades GAME-01 and SND-01 because their accepted input/output paths are incomplete. No requirement is orphaned in the planning records.

Phase 04 was shipped as [PR #20](https://github.com/szTheory/nesturbator/pull/20), squash-merged at 66845607ed48710821531d9dfd03f4514a551c35. Its six-platform build, hash-equality, RetroArch E2E, sanitizer, no-float, hygiene, title, and required aggregate checks passed in CI run 37952928409. The full-vector and ROM-loader-fuzz jobs passed in nightly run 37952927992.

## Scores

| Area | Score | Basis |
|---|---:|---|
| Requirements | 17/19 | Two requirements have incomplete accepted behavior: GAME-01 trainer execution and SND-01 single-sample libretro audio. |
| Phase verification | 4/4 | All four verification reports exist and say passed: Phase 01 69/69, Phase 02 62/62, Phase 03 13/13, Phase 04 15/15. |
| Integration | 17/19 | The 17 remaining requirement paths are wired; GAME-01 and SND-01 each have a broken edge described below. |
| End-to-end flows | 4/6 | Normal ROM-to-frame, controller/video parity, batch audio parity, and CI/release flows work. Trainer-bearing ROM execution and single-sample-only audio do not. |

## Requirement Coverage

| Requirement | Phase | Status | Evidence |
|---|---:|---|---|
| FRAME-01 | 01 | SATISFIED | Six-platform CI builds and tests the deliverables; latest required CI aggregate passed. |
| FRAME-02 | 01 | SATISFIED | Installed-package public-header consumer is registered and covered by the passing phase verification. |
| FRAME-03 | 01 | SATISFIED | No-cartridge frame, runner image/hash, and cross-platform hash comparison are wired and verified. |
| FRAME-04 | 01 | SATISFIED | Libretro video output is compared against runner output by the host test. |
| FRAME-05 | 01 | SATISFIED | Required hosted RetroArch E2E screenshot comparison passed. |
| FRAME-06 | 01 | SATISFIED | Hosted ASan, no-float, and hygiene checks passed. |
| FRAME-07 | 01 | SATISFIED | PR archives and automatic checksummed release path were verified; Phase 04 PR checks passed. |
| CPU-01 | 02 | SATISFIED | Sampled 65x02 vectors compare final state and every bus cycle. |
| CPU-02 | 02 | SATISFIED | Pinned full-vector nightly job is wired and the latest full-vector job passed. |
| GAME-01 | 03 | UNSATISFIED | Trainer-bearing images are accepted but trainer data is not initialized into CPU-visible RAM. |
| GAME-02 | 03 | SATISFIED | Pinned game frames, libretro parity, six-platform hashes, and hosted RetroArch E2E are verified. |
| GAME-03 | 03 | SATISFIED | Movie replay and input-driven libretro/runner parity are tested. |
| GAME-04 | 03 | SATISFIED | AccuracyCoin results are read from RAM and the protected scoreboard checks pass. |
| GAME-05 | 03 | SATISFIED | Loader corpus regression and nightly fuzz workflow are wired; latest ROM-loader-fuzz job passed. |
| GAME-06 | 03 | SATISFIED | Hosted six-platform frame and movie hash inventories compare equal. |
| SND-01 | 04 | UNSATISFIED | Batch audio works, but a frontend using only the single-sample callback receives no audio. |
| SND-02 | 04 | SATISFIED | Transition and PCM hashes use canonical serialization and are compared across six platforms. |
| SND-03 | 04 | SATISFIED | All six required AccuracyCoin sound rows are present and pass. |
| SND-04 | 04 | SATISFIED | The five-tone spectral test is registered in CI and meets the -80 dB threshold. |

## Phase Verification

| Phase | Verification | Summary coverage | Result |
|---|---|---:|---|
| 01 — A test frame in RetroArch | Passed, 69/69 | 12 plans | Passed; one release-policy checker warning remains. |
| 02 — CPU vectors | Passed, 62/62 | 11 plans | Passed; the reported single-sample callback issue was later attributed to the sound integration and remains open in Phase 03 review disposition. |
| 03 — A real game in RetroArch | Passed, 13/13 | 14 plans | Passed; GAME-01 trainer semantics were not verified, and the local GUI launch limitation is covered by hosted E2E. |
| 04 — Sound | Passed, 15/15 | 4 plans | Passed; only the batch callback path was exercised. |

No phase verification file is missing. Nyquist validation is disabled in project configuration, and there is no active validate-phase post hook; Nyquist scanning was therefore skipped.

## Cross-Phase Integration

The integration checker traced the public C API through the runner and libretro adapter, CPU/bus/PPU execution, APU-to-PCM flow, AccuracyCoin RAM reporting, hash gates, six-platform CI, RetroArch E2E, and release publication. This C project has no HTTP routes; API route-consumer checks do not apply. The public C API has consumers in the runner, libretro adapter, and installed-package/host tests.

| Flow | Result | Evidence |
|---|---|---|
| CLI ROM load → mapper-0 cartridge → CPU/PPU frame → image/hash | WIRED for supported ROMs without trainer-dependent state | runner/main.c loads through the public API; src/frame.c clocks the shared core; runner hashes frame output. |
| Controller input → libretro frame → host/runner parity | WIRED | libretro/libretro.c polls input and emits video; tests/libretro/libretro_host.c compares replayed output. |
| CPU cycles → APU → synthesized PCM → runner audio hashes | WIRED | src/bus.c clocks the APU; src/apu.c feeds src/synth.c; frame PCM feeds runner/audio_hash.c. |
| PCM → libretro batch audio → host parity | WIRED | The host test compares both stereo channels sample by sample. |
| PCM → libretro single-sample callback | BROKEN — BLOCKER, SND-01 | audio_cb is retained but never called; only audio_batch_cb is dispatched. |
| CTest/presets → six-platform CI → hashes/RetroArch E2E → release | WIRED | Required CI and nightly jobs passed; PR #20 is merged. |

Requirements centered on policy or conformance rather than a runtime user flow—FRAME-01, FRAME-06, FRAME-07, CPU-01, CPU-02, and SND-04—are intentionally self-contained, and their checks connect to the built deliverables.

## Gaps to Close

### GAME-01 — Trainer data is accepted but not applied

The loader validates and accepts trainer-bearing iNES/NES 2.0 images and advances the PRG pointer past the 512-byte trainer at src/cartridge.c:151-167. It never seeds the trainer into CPU-visible memory, and the CPU bus does not map $6000-$7FFF for PRG RAM. A game that relies on trainer-initialized RAM therefore starts with different state. The test in tests/core/test_cartridge.c checks trainer geometry and load lifetime only; it does not assert trainer visibility or execution behavior.

Implement the documented accepted trainer behavior, including the needed RAM mapping and initialization, with a CI regression. If this emulator version deliberately excludes trainer-dependent execution, reject that input before allocation and revise the public contract and phase claim so acceptance matches behavior.

### SND-01 — Single-sample libretro callback receives no audio

libretro/libretro.c stores audio_cb but retro_run only invokes audio_batch_cb. The host test at tests/libretro/libretro_host.c:632-635 registers both callbacks, so it verifies only the batch route. Preserve batch dispatch when available and deliver each stereo frame through audio_cb when batch dispatch is unavailable. Add a host test that registers only the single-sample callback and compares its output to the direct core samples.

Both gaps can be closed with automated regression coverage in CI. Neither requires listening tests or human UAT.

## Non-blocking Tech Debt and Deferred Items

| Phase | Item | Classification |
|---|---|---|
| 01 | tests/cmake/release_policy.cmake checks for required publish-gate text by substring. A condition that appends an override such as || always() can still pass, even though the current release workflow has the correct CI dependency and guard. | Warning: strengthen the policy mutation test. |
| 03 | include/nesturbator.h says the CPU does not run during frames, despite loaded games executing CPU instructions. | Warning: stale public API documentation. |
| 03 | retro_reset() is empty for a loaded game. | Deferred behavior outside the listed milestone requirements; decide whether to reset core state. |
| 03 | Two duplicate hardware comments remain in src/ppu.c. | Informational cleanup. |
| 03 | The local macOS RetroArch GUI test abort/skip remains recorded in deferred-items.md. Hosted required RetroArch E2E passed, so acceptance has no human or local-host dependency. | Environment limitation; no milestone blocker. |
| Project | .planning/STATE.md still says PR #20 is open and checks are running, although PR #20 is merged with green required checks. The milestone completion step should refresh this lifecycle bookkeeping. | Planning bookkeeping. |

Phase 02’s deferred sandbox-specific RetroArch failure is marked resolved. No human verification or UAT remains necessary for the current milestone once the two behavioral gaps receive automated regression coverage.

