---
milestone: 1
audited: 2026-10-09T18:22:15Z
status: gaps_found
scores:
  requirements: 18/19
  phases: 5/5
  integration: 18/19
  flows: 11/12
gaps:
  requirements:
    - id: SND-01
      status: unsatisfied
      phase: "04-sound"
      claimed_by_plans: ["04-01-PLAN.md", "04-02-PLAN.md", "04-04-PLAN.md"]
      completed_by_plans: ["04-01-PLAN.md", "04-04-PLAN.md"]
      verification_status: passed
      evidence: >-
        Phase 04 verifies the batch callback path only. libretro/libretro.c stores
        audio_cb but retro_run invokes only audio_batch_cb, so a frontend that
        supplies only the single-sample callback receives no PCM. The host test
        registers both callbacks and asserts zero single-sample calls.
  integration:
    - id: SND-01-single-sample-audio
      status: broken
      severity: BLOCKER
      requirements: ["SND-01"]
      from: "Core stereo PCM frame buffer"
      to: "Libretro single-sample frontend callback"
      evidence: >-
        libretro/libretro.c:206-208 dispatches only audio_batch_cb. audio_cb is
        retained at lines 32, 58-67 but never called. The host test at
        tests/libretro/libretro_host.c:632-636 registers both callbacks and
        expects sample_calls to remain zero.
  flows:
    - name: "Libretro audio on a single-sample-only host"
      status: broken
      requirements: ["SND-01"]
      broken_at: "retro_run does not dispatch PCM through audio_cb when audio_batch_cb is absent."
tech_debt:
  - phase: "01-a-test-frame-in-retroarch"
    items:
      - "Warning: tests/cmake/release_policy.cmake checks for required publish-gate text by substring; a condition that adds || always() can evade the checker, although the current workflow has the intended CI dependency and guard."
  - phase: "03-a-real-game-in-retroarch"
    items:
      - "Warning: include/nesturbator.h:214-218 says the CPU does not run during frames, although loaded-game frames execute CPU instructions."
      - "Deferred behavior: retro_reset() is empty for a loaded game; frontend reset behavior remains outside the v1 requirements."
      - "Informational cleanup: src/ppu.c repeats the vblank timing and odd-frame dot-skip comments."
  - phase: "04.1-close-gap-game-01-initialize-accepted-ines-trainers"
    items:
      - "Warning: trainer bytes are covered through the cartridge CPU-bus regression for iNES and NES 2.0, but no trainer-bearing ROM fixture exercises the full runner/libretro host path. The implementation uses the same public loader in those hosts."
---
# Milestone 1 Audit

**Audited:** 2026-10-09  
**Status:** gaps_found  
**Scope:** Milestone 1 (`v1`), all five roadmap phases: 01, 02, 03, 04, and inserted 04.1.

## Result

All five phases have a present `VERIFICATION.md` with `status: passed`; all 19 v1 requirements are mapped, checked complete in the traceability table, and represented in phase summary frontmatter. The Phase 04.1 verification closes the earlier GAME-01 trainer gap. Cross-phase tracing confirms the trainer data reaches per-instance CPU-visible PRG RAM and is covered by deterministic bus tests.

One integration blocker remains: `SND-01` is incomplete for a libretro frontend that provides only the single-sample audio callback. The adapter retains that callback but sends PCM only through the batch callback. This is an automated host-test gap and requires no listening test or owner UAT.

## Scores

| Area | Score | Basis |
|---|---:|---|
| Requirements | 18/19 | All requirements except the single-sample `SND-01` callback path are satisfied end-to-end. |
| Phase verification | 5/5 | All five in-scope phase verification files exist and report passed. |
| Integration | 18/19 | One requirement edge is broken: PCM to the libretro sample-only callback. |
| End-to-end flows | 11/12 | The sample-only libretro audio flow is broken; the other eleven traced flows are wired. |

## Requirement Coverage

The three-source check compared the `REQUIREMENTS.md` traceability table, each phase verification requirements table, and every in-scope summary's `requirements-completed` frontmatter. No traceability requirement is orphaned. The historical phase checkboxes alone do not prove an integration edge; `SND-01` is therefore downgraded based on the cross-phase source trace below.

| Requirement | Phase | Verification | Summary frontmatter | Audit result | Evidence |
|---|---:|---|---|---|---|
| FRAME-01 | 01 | passed | listed | SATISFIED | Six-platform build and test workflow, with the hosted jobs recorded in Phase 01 verification. |
| FRAME-02 | 01 | passed | listed | SATISFIED | Installed-package consumer compiles and runs using the public header. |
| FRAME-03 | 01 | passed | listed | SATISFIED | Deterministic no-cartridge frame and runner hash path are verified. |
| FRAME-04 | 01 | passed | listed | SATISFIED | Libretro host frame is compared with runner output. |
| FRAME-05 | 01 | passed | listed | SATISFIED | Unattended RetroArch screenshot comparison is covered by hosted evidence. |
| FRAME-06 | 01 | passed | listed | SATISFIED | Sanitizer, no-float, symbol and hygiene lanes pass. |
| FRAME-07 | 01 | passed | listed | SATISFIED | Platform archives and automatic checksummed release path are verified. |
| CPU-01 | 02 | passed | listed | SATISFIED | Sampled 65x02 vectors compare final state and every bus cycle. |
| CPU-02 | 02 | passed | listed | SATISFIED | Pinned full-vector workflow and exact-run evidence are verified; the scheduled cron event itself is not claimed. |
| GAME-01 | 03, 04.1 | passed | listed | SATISFIED | Phase 04.1 closes the trainer gap: accepted trainers are copied into writable CPU-visible PRG RAM and checked by cartridge bus tests. |
| GAME-02 | 03 | passed | listed | SATISFIED | NROM hashes, runner/libretro parity, cross-platform inventory and hosted RetroArch screenshot are verified. |
| GAME-03 | 03 | passed | listed | SATISFIED | Movie replay and input-driven runner/libretro frame parity are verified. |
| GAME-04 | 03 | passed | listed | SATISFIED | AccuracyCoin output is read from RAM and protected by the scoreboard regression gate. |
| GAME-05 | 03 | passed | listed | SATISFIED | Loader corpus regression and nightly libFuzzer wiring are verified. |
| GAME-06 | 03 | passed | listed | SATISFIED | Hosted six-platform frame-hash inventories match. |
| SND-01 | 04 | passed | listed | **UNSATISFIED** | The batch callback is verified; a host that supplies only `audio_cb` receives no samples. |
| SND-02 | 04 | passed | listed | SATISFIED | Transition and PCM hashes use canonical serialization and cross-platform comparison. |
| SND-03 | 04 | passed | listed | SATISFIED | The six required AccuracyCoin sound rows pass. |
| SND-04 | 04 | passed | listed | SATISFIED | The five-tone spectral gate passes below the -80 dB threshold. |

## Phase Verification

| Phase | Verification | Result |
|---|---|---|
| 01 — A test frame in RetroArch | passed, 69/69 | Complete; hosted six-platform, RetroArch, hygiene and release evidence recorded. |
| 02 — The CPU matches the public vectors | passed, 62/62 | Complete; two scheduled-event-only truths were explicitly owner-approved using exact merge/manual substitute evidence; no pending UAT. |
| 03 — A real game in RetroArch | passed, 13/13 | Complete; prior GAME-01 trainer semantics were not in scope of that report and were closed in Phase 04.1. |
| 04 — Sound | passed, 15/15 | Complete at phase level; its libretro test exercises batch callback only, so cross-phase audit found the sample-only integration omission. |
| 04.1 — Initialize accepted iNES trainers | passed, 8/8 | Complete; trainer bytes are visible and writable through the normal CPU bus for both accepted header formats. |

No phase verification file is missing. The phase reports identify no outstanding owner verification. Local RetroArch skips are backed by hosted screenshot evidence where required.

## Cross-Phase Integration

The integration check traced providers to consumers across the C library, runner, libretro adapter, tests, CTest, hosted CI, nightly jobs and release workflow. HTTP routes and authentication are not applicable to this C emulator.

| Flow | Result | Evidence |
|---|---|---|
| Public C API → installed consumer, runner and libretro adapter | WIRED | `include/nesturbator.h` is exported, linked into both hosts, and exercised by installed consumer and host tests. |
| No-cartridge instance → deterministic test frame/silence → host output | WIRED | Shared frame API drives the runner and libretro output; host tests compare video and zero-sample behavior. |
| ROM → validated mapper-0 cartridge → CPU/PPU → runner frame/hash | WIRED | Runner uses the public loader and frame API; hosted hashes cover committed games. |
| Trainer bytes → per-instance PRG RAM → CPU bus → execution | WIRED | `src/cartridge.c:175-179` copies trainer bytes at offset `$1000`; `src/bus.c` maps `$6000-$7FFF`; `core.cartridge` checks both formats and lifecycle. |
| Controller/movie → serialized bus input → deterministic frames and parity | WIRED | Runner movie playback and libretro host parity use the same controller masks. |
| AccuracyCoin → RAM results → committed/protected scoreboard | WIRED | Runner reads RAM results; CI checks the committed rows and prevents loss of protected passes. |
| CPU → sampled and pinned full-vector harnesses → CI/nightly | WIRED | The vector bus seam checks ordered cycles; pinned full-vector workflow and result evidence are present. |
| ROM loader → regression corpus/fuzzer → CI/nightly | WIRED | Corpus regression is in CI and libFuzzer is configured in nightly. |
| APU → PCM → runner hashes and six-platform inventory | WIRED | Per-instance synthesis feeds runner PCM and transition hashes with canonical cross-platform serialization. |
| PCM → libretro batch callback → host sample comparison | WIRED | Host test compares stereo output for audible frames. |
| PCM → libretro single-sample-only callback | **BROKEN — BLOCKER (SND-01)** | `retro_run` dispatches only `audio_batch_cb`; single-sample-only hosts get no PCM. |
| CTest/build matrix → cross-platform evidence/RetroArch → release artifacts | WIRED | Required hosted jobs, hash inventories, screenshot evidence and checksummed release workflow are connected. |

### Integration Findings

#### BLOCKER — `SND-01`: the single-sample callback is never called

`libretro/libretro.c` stores `audio_cb`, but `retro_run` at lines 206–208 only invokes `audio_batch_cb`. A host registering `retro_set_audio_sample` without `retro_set_audio_sample_batch` receives no samples. The current host test registers both callbacks and expects `sample_calls == 0`, so it cannot catch this compatibility path.

Add a fallback that sends each stereo pair through `audio_cb` only when the batch callback is absent, then extend `tests/libretro/libretro_host.c` with a sample-only registration and compare callback values to the direct core PCM. This is deterministic and belongs in CI; human listening/UAT is unnecessary.

#### WARNING — trainer app-host E2E fixture is absent

The Phase 04.1 implementation is wired and its regression checks trainer visibility, bounds, writability, instance isolation, reload and rejection through the CPU bus. The runner and libretro adapter use the same public loader, but no trainer-bearing ROM fixture currently traverses those host boundaries. This is additional coverage, not a broken implementation path or a v1 blocker.

## Non-Blocking Tech Debt and Deferred Items

| Phase | Item | Classification |
|---|---|---|
| 01 | `tests/cmake/release_policy.cmake` can accept an added `|| always()` because it checks the required publish text by substring; the live release workflow currently has the intended CI dependency and guard. | Warning: strengthen the policy checker to validate the complete condition. |
| 03 | `include/nesturbator.h:214-218` says the CPU does not run during frames, despite loaded-game execution. | Warning: correct the public API comment. |
| 03 | `retro_reset()` is empty for loaded games. | Deferred behavior outside current v1 requirements. |
| 03 | Two hardware comments are duplicated in `src/ppu.c`. | Informational cleanup. |
| 04.1 | No trainer-bearing fixture exercises the entire runner/libretro path. | Optional integration coverage; current loader-to-bus behavior is directly regression-tested. |

The v1 milestone requires no human verification or UAT. Nyquist validation is disabled in `.planning/config.json`, and `verify:post` has no active validation hook, so Nyquist scanning was skipped.

## Next Step

Close the remaining `SND-01` gap with the phase workflow:

```text
$gsd-phase --insert 4.2 "Close gap: SND-01 — deliver single-sample libretro audio"
```

Then run `$gsd-discuss-phase 4.2`, `$gsd-plan-phase 4.2`, and `$gsd-execute-phase 4.2`. Refresh this audit after the closure phase before completing Milestone 1.
