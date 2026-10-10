---
milestone: 1
audited: 2026-10-10T00:54:54Z
previous_audit: 2026-10-10T00:25:47Z
status: tech_debt
scores:
  requirements: 19/19
  phases: 6/6
  integration: 19/19
  flows: 12/12
gaps:
  requirements: []
  integration: []
  flows: []
tech_debt:
  - phase: 01-a-test-frame-in-retroarch
    items:
      - "Warning: tests/cmake/release_policy.cmake checks for the required publish-gate text by substring. The live release workflow has the correct CI dependency and guard, but a mutation that appends `|| always()` could preserve the required substring and evade this self-test."
  - phase: 03-a-real-game-in-retroarch
    items:
      - "Optional integration coverage: no trainer-bearing fixture currently traverses the complete runner/libretro host path. The shared loader's trainer initialization is verified through the CPU bus for iNES and NES 2.0."
      - "Local coverage: on the owner's Apple Silicon Mac, `retroarch.testframe` and `retroarch.game` self-skip because the installed RetroArch does not start from the test session (03 deferred-items.md). The hosted `retroarch-e2e` job is the only live screenshot evidence for FRAME-05 and GAME-02."
  - phase: planning
    items:
      - "Warning: GSD milestone initialization reports a null version and 1/6 completed phases, while ROADMAP.md and STATE.md report all six phases and all 44 plans complete. roadmap.analyze reports 100% plan progress but marks five legacy phases `executed` and flags checkbox conflicts. Reconcile this metadata during milestone completion."
---

# Milestone 1 Audit

**Audited:** 2026-10-10
**Status:** tech debt; no critical gaps
**Scope:** Milestone 1 (`v1`), phases 01, 02, 03, 04, 04.1, and 04.2.

## Result

All 19 v1 requirements are checked complete in `REQUIREMENTS.md`, represented in phase verification reports, and listed in at least one completed summary's `requirements-completed` frontmatter. All six phase verification reports pass. Cross-phase review found 19/19 requirements wired and 12/12 traced end-to-end flows complete. The prior audit's SND-01 single-sample callback blocker is closed by Phase 04.2 and its deterministic host regression.

No behavior verification or owner UAT remains. Phase 02's two schedule-only checks no longer rest on the owner override alone: scheduled nightly runs 37572239134, 37728282108 and 37884748203 (2026-10-07 to 2026-10-09) passed `vectors-full` and the `report` job; provenance review was recorded in Phase 01 UAT. Nyquist validation is disabled in `.planning/config.json`, and `verify:post` has no active validation hook, so Nyquist scanning is skipped.

The audit status is `tech_debt` because a release-policy test weakness, optional trainer host-path coverage, and a GSD phase-status reporting mismatch remain to review. These do not block Milestone 1 behavior or requirement coverage.

## Scores

| Area | Score | Basis |
|---|---:|---|
| Requirements | 19/19 | Every v1 requirement is verified, represented in summary frontmatter, and checked complete in the traceability table. |
| Phase verification | 6/6 | All six in-scope `VERIFICATION.md` reports exist and have `status: passed`. |
| Integration | 19/19 | The integration checker found every requirement connected to its implementation and automated evidence. |
| End-to-end flows | 12/12 | All traced library, runner, libretro, CI, release, and host flows are wired. |

## Requirement Coverage

The cross-reference compares `REQUIREMENTS.md`, phase `VERIFICATION.md` requirements tables, and every phase summary's `requirements-completed` frontmatter.

| Requirement | Phase | Verification | Summary frontmatter | Traceability | Audit result | Evidence |
|---|---:|---|---|---|---|---|
| FRAME-01 | 01 | passed | listed | `[x]` | SATISFIED | Six-platform `ci`/`ci-msvc` build and test workflow. |
| FRAME-02 | 01 | passed | listed | `[x]` | SATISFIED | Installed-package consumer uses only the public header. |
| FRAME-03 | 01 | passed | listed | `[x]` | SATISFIED | Deterministic no-cartridge frame, runner image, and hash equality. |
| FRAME-04 | 01 | passed | listed | `[x]` | SATISFIED | Libretro host frame equals runner output. |
| FRAME-05 | 01 | passed | listed | `[x]` | SATISFIED | RetroArch screenshot comparison has hosted evidence; local absence self-skips. |
| FRAME-06 | 01 | passed | listed | `[x]` | SATISFIED | Sanitizer, no-float, symbol, and hygiene checks pass. |
| FRAME-07 | 01 | passed | listed | `[x]` | SATISFIED | Platform archives and automatic checksummed release path are verified. |
| CPU-01 | 02 | passed | listed | `[x]` | SATISFIED | All 256 opcode cases compare final state and every bus cycle. |
| CPU-02 | 02 | passed | listed | `[x]` | SATISFIED | Pinned full-vector workflow and exact-commit evidence pass; three scheduled nightly runs (2026-10-07 to 2026-10-09) passed `vectors-full` and `report`. |
| GAME-01 | 03, 04.1 | passed | listed | `[x]` | SATISFIED | Loader rejection and trainer initialization are covered, including byte-exact CPU-bus tests for both formats. |
| GAME-02 | 03 | passed | listed | `[x]` | SATISFIED | NROM hashes, runner/libretro parity, sprite behavior, and hosted RetroArch capture. |
| GAME-03 | 03 | passed | listed | `[x]` | SATISFIED | Movie replay and same-input runner/libretro frame parity. |
| GAME-04 | 03 | passed | listed | `[x]` | SATISFIED | AccuracyCoin RAM results and protected scoreboard regression. |
| GAME-05 | 03 | passed | listed | `[x]` | SATISFIED | Loader corpus regression and nightly libFuzzer workflow. |
| GAME-06 | 03 | passed | listed | `[x]` | SATISFIED | Six-platform frame-hash inventories compare byte-for-byte. |
| SND-01 | 04, 04.2 | passed | listed | `[x]` | SATISFIED | APU and runner audio evidence from Phase 04 plus sample-only and batch-priority host parity from Phase 04.2. |
| SND-02 | 04 | passed | listed | `[x]` | SATISFIED | Canonical transition and PCM hashes match across supported platforms. |
| SND-03 | 04 | passed | listed | `[x]` | SATISFIED | All six required AccuracyCoin sound rows pass. |
| SND-04 | 04 | passed | listed | `[x]` | SATISFIED | Five-tone spectral test remains below the -80 dB limit. |

No traceability requirement is orphaned or unsatisfied.

## Phase Verification

| Phase | Verification | Result |
|---|---|---|
| 01 — A test frame in RetroArch | passed, 69/69 | Six-platform build/release evidence, RetroArch screenshot, and all FRAME requirements covered. |
| 02 — The CPU matches the public vectors | passed, 62/62 | CPU-01/02 covered; the two schedule-only overrides are now backed by three passing scheduled runs. |
| 03 — A real game in RetroArch | passed, 31/31 | All GAME requirements and four roadmap success criteria covered. |
| 04 — Sound | passed, 15/15; 4/4 roadmap criteria | APU, synthesis, hashes, scoreboard, spectrum, and batch audio parity covered. |
| 04.1 — Initialize accepted iNES trainers | passed, 8/8 | Trainer bytes reach per-instance PRG RAM and are verified through the CPU bus. |
| 04.2 — Deliver single-sample libretro audio | passed, 5/5 | Sample-only callback parity, batch exclusivity, and failed-frame silence are in CI. |

## Cross-Phase Integration and End-to-End Flows

| Flow | Result | Evidence |
|---|---|---|
| Public C API and installed package → runner and libretro adapter | WIRED | Installed consumer and host tests exercise the same exported core API. |
| No-cartridge instance → deterministic test frame → runner hash and libretro video | WIRED | Runner and adapter consume the core frame; host compares pixels to runner output. |
| Mapper-0 ROM → loader, CPU and PPU → runner frame/hash and libretro video | WIRED | Shared public loader/frame APIs serve both hosts; pinned game and host parity tests cover output. |
| Trainer bytes → per-instance PRG RAM → CPU bus | WIRED | `src/cartridge.c` copies validated trainer bytes; `core.cartridge` reads every byte through the bus. |
| Controller/movie input → deterministic frame stream → runner/libretro parity | WIRED | Movie masks feed the public frame API; replay hashes and host input tests cover the path. |
| AccuracyCoin RAM results → protected scoreboard check | WIRED | Runner reads emulated RAM and CI prevents committed passing rows from disappearing. |
| CPU → sampled and full vector harnesses → CI/nightly | WIRED | Ordered bus-cycle comparisons and pinned full-set workflow are connected. |
| ROM loader → regression corpus and fuzzer → CI/nightly | WIRED | Corpus replay is in CI; bounded libFuzzer runs nightly. |
| APU → PCM → runner audio hashes and six-platform inventory | WIRED | Per-instance synthesis feeds canonical transition and sample hashes. |
| PCM → libretro batch callback → host parity | WIRED | Batch output compares both stereo channels with direct core PCM. |
| PCM → libretro single-sample callback → host parity | WIRED | Sample-only host test compares every ordered stereo pair and count; batch remains exclusive when registered. |
| Build/test matrix and hosted RetroArch → release artifacts | WIRED | CI runs platform, hash, and screenshot gates before checksummed release publication. |

### Integration Findings

The old SND-01 blocker is closed. `libretro/libretro.c:195-213` returns before callbacks on a failed frame, prioritizes `audio_batch_cb`, and otherwise sends ordered left/right pairs through `audio_cb`. `tests/libretro/libretro_host.c:411-497` compares sample-only and batch paths against independently stepped core PCM and verifies silence after a JAM frame. `tests/CMakeLists.txt:412-424` registers this host test with the adapter and runner dependencies. Phase 04.2 records the full CI workflow passing all 357 tests; the two unavailable local RetroArch checks were environment-gated skips.

The integration checker found no blocker, orphan, or missing expected connection. The trainer feature is verified through the shared loader and CPU bus, though no trainer-bearing fixture currently traverses the entire runner/libretro host path; that is optional extra coverage.

## Non-Blocking Tech Debt

| Area | Item | Classification |
|---|---|---|
| Phase 01 release-policy checker | `tests/cmake/release_policy.cmake` checks required publish text by substring. The current workflow is correct, but appending `|| always()` could evade the self-test. | Warning; strengthen the predicate/mutation test in a future cleanup. |
| Phase 03/04.1 host-path coverage | No trainer-bearing fixture traverses the complete runner/libretro path. The public-loader-to-CPU-bus behavior is directly tested. | Optional integration coverage; no requirement blocker. |
| GSD planning metadata | The live analyzer returns `completed_phases: 1` and null milestone version although `ROADMAP.md` and `STATE.md` show 6/6 phases and 44/44 plans complete. It marks phases 01–04.1 as `executed`, flags their checked roadmap boxes, and reports 100% plan progress. | Bookkeeping warning; reconcile during milestone completion so future GSD routing reads the same completed state. |

The previous audit's stale `include/nesturbator.h` CPU-comment warning and duplicate PPU-comment item do not appear in current source and are removed from this debt list. `retro_reset()` remains a no-op for loaded games, which is outside the v1 requirements and remains out of scope.

## Nyquist Coverage

Skipped: `.planning/config.json` sets `workflow.nyquist_validation` to false and `gsd_run loop render-hooks verify:post` returns no active hooks.

## Refresh (2026-10-10T00:54:54Z)

This refresh re-ran the audit after PR #22 merged. Between the first audit commit and `main` (`5c082c1`), only release metadata and planning notes changed: `.release-please-manifest.json`, `CHANGELOG.md`, the version line in `README.md`, `include/nesturbator.h`, `libretro/nesturbator_libretro.info` and `version.txt`. No emulation, adapter or test code changed, so the integration results above (19/19 wired, 12/12 flows) still describe `main`, and the integration checker was not run again.

- Phase 04.2, with Phase 03 Plan 15 and Phase 04.1, merged to `main` as PR #22 (`58ca4be`). CI on `main` passed at `5c082c1`, and the release-please PR #23 for v0.1.6 merged. The v0.1.6 Release run was still in progress at audit time; v0.1.5 is live with 19 assets.
- Scheduled nightly evidence now exists for CPU-02, as recorded above.
- New tech-debt item: the local RetroArch tests self-skip on the owner's Mac because RetroArch does not start there.
- `retro_reset()` is still a no-op for loaded games. This is outside the v1 requirements and is tracked here only.

## Next Step

No implementation or human-verification gap remains, and every phase is merged to `main`. Complete Milestone 1 and record the non-blocking items above as deferred:

```text
/gsd-complete-milestone 1
```
