---
milestone: v7
audited: "2026-11-01T20:07:21Z"
status: tech_debt
scores:
  requirements: 4/4
  phases: 1/1
  integration: 8/9
  flows: 5/5
gaps:
  requirements: []
  integration: []
  flows: []
tech_debt:
  - phase: 24
    items:
      - "README still says protected-main, hosted, cross-platform hash, and performance evidence are outstanding, although the final phase verification records those exact-candidate checks as passed."
      - "Phase verification retains a non-blocking warning about inherited branch-wide whitespace diagnostics; scoped changes and required evidence controls passed."
---

# Milestone v7 Audit — Remaining DMA Accuracy Failures

**Result:** No unsatisfied requirements or broken flows. The milestone has one non-blocking documentation mismatch and a retained historical hygiene warning, so the audit status is `tech_debt`.

## Scope and Definition of Done

The milestone contains Phase 24, **Correct remaining NTSC DMA behavior**. Its goal is for the library, headless runner, and libretro adapter to receive source-supported behavior for the five selected DMA failures while preserving protected passes, deterministic output, performance limits, and native-state continuation.

All five phase success criteria were checked against the final phase verification. That report passes 9/9 observable truths and covers the four approved requirements. The exact-candidate receipt remains bound to candidate `d54e21898fbf7bb3984fd2d33aa563932b464581`; the older interrupted workflow's outcome remains unknown and its restart stop remains in force.

## Requirements Coverage

The three sources are the requirements traceability table, Phase 24's final verification report, and all five plan summary frontmatters.

| Requirement | Traceability | Phase verification | Plan summary frontmatter | Audit result |
|---|---|---|---|---|
| DMA-01 | Phase 24, Complete, checked | Satisfied: all five selected rows pass individually at the unchanged pin | Not listed as completed in plan summaries; manually reconciled against the final evidence | **Satisfied** — the final report records each pinned row and the exact-candidate campaign evidence. |
| DMA-02 | Phase 24, Complete, checked | Satisfied: source-backed causes and owned wrong-model fixtures are verified | Listed completed by Plan 24-05 | **Satisfied** |
| DMA-03 | Phase 24, Complete, checked | Satisfied: protected keys, six-platform hashes, and the unchanged performance allowance pass on exact-candidate evidence | Listed completed by Plan 24-05 | **Satisfied** |
| DMA-04 | Phase 24, Complete, checked | Satisfied: native continuation, refusal controls, format/revision disposition, and documentation are verified | Not listed as completed in plan summaries; manually reconciled against the final evidence | **Satisfied** — the report gives the native continuation and compatibility evidence. |

The plan summaries describe bounded plan outcomes; their omission of DMA-01 and DMA-04 was manually checked against the completed phase report. All four requirements appear in the verification table, none is orphaned, and no requirement is unsatisfied.

## Phase Verification

| Phase | Verification | Result | Notes |
|---|---|---|---|
| 24 — Correct remaining NTSC DMA behavior | [24-VERIFICATION.md](v7-phases/24-correct-remaining-ntsc-dma-behavior/24-VERIFICATION.md) | **Passed, 9/9** | DMA-01 through DMA-04 satisfied; no behavior-unverified items or regressions. |

## Integration and End-to-End Flows

The integration check was a static trace. It found eight connected paths and one partial documentation-status connection, with no broken production connection or orphaned Phase 24 exports.

| Expected connection | Requirements | Status | Evidence |
|---|---|---|---|
| Phase 21 bus phase/write-retry seam into Phase 24 APU/DMA logic | DMA-01, DMA-02, DMA-04 | WIRED | [bus.c](../../src/bus.c#L88), [bus.c](../../src/bus.c#L356), [apu.c](../../src/apu.c#L251) |
| CPU accesses through DMA bus work to resumed access | DMA-01, DMA-02 | WIRED | [cpu.c](../../src/cpu.c#L6), [bus.c](../../src/bus.c#L241) |
| Core implementation into the shared library | DMA-01–04 | WIRED | [CMakeLists.txt](../../CMakeLists.txt#L61) |
| Shared library into the headless runner | DMA-01, DMA-03 | WIRED | [runner/CMakeLists.txt](../../runner/CMakeLists.txt#L18), [runner/main.c](../../runner/main.c#L1231) |
| Shared library into the libretro adapter | DMA-01, DMA-03, DMA-04 | WIRED | [libretro/CMakeLists.txt](../../libretro/CMakeLists.txt#L5), [libretro.c](../../libretro/libretro.c#L282) |
| Changed DMA state through serialization, validation, and public save/load | DMA-04 | WIRED | [state.c](../../src/state.c#L255), [state.c](../../src/state.c#L496), [test_state_fork.c](../../tests/core/test_state_fork.c#L317) |
| Production behavior into registered regression tests and CI preset | DMA-02, DMA-03, DMA-04 | WIRED | [test_bus.c](../../tests/cpu/test_bus.c#L135), [tests/CMakeLists.txt](../../tests/CMakeLists.txt#L2049), [CMakePresets.json](../../CMakePresets.json#L200) |
| Phase 23 reproduction contract into the retained documentation checks | DMA-02 | WIRED | The contract remains registered in [tests/CMakeLists.txt](../../tests/CMakeLists.txt#L1683). |
| Behavior and acceptance status in the README and public header | DMA-02, DMA-04 | PARTIAL | Behavior and compatibility are documented. README acceptance-status paragraphs still describe exact-candidate evidence as outstanding; see [README.md](../../README.md#L28) and the verification result below. |

**Requirements integration map:** DMA-01, DMA-02, DMA-03, and DMA-04 are each WIRED to their relevant implementation, host, state, and regression paths. None requires an absent cross-phase connection. The README discrepancy is a stale status claim, not a broken runtime or test path.

| End-to-end flow | Result |
|---|---|
| CPU bus access → DMC/OAM service → resumed access | Complete by static trace |
| ROM execution through the headless runner and shared library | Complete by static trace |
| ROM execution through libretro and the shared library | Complete by static trace |
| Native state save/load continuation through host wrappers | Complete by static trace |
| Production-linked DMA regressions through CTest and the CI preset | Complete by static trace |

There are no HTTP API routes or authentication flows in this C core. The receipt-validation script checks evidence integrity; it does not substitute for production-linked behavior tests.

## Tech Debt and Boundaries

- **README acceptance status is stale.** [README.md](../../README.md#L28) and [README.md](../../README.md#L47) say cold protected-main, hosted, six-platform equality, and Cachegrind evidence remain outstanding. The [final phase report](v7-phases/24-correct-remaining-ntsc-dma-behavior/24-VERIFICATION.md#L117) records the protected, platform-equality, and performance evidence as passed; its [candidate-bound receipt](v7-phases/24-correct-remaining-ntsc-dma-behavior/24-VERIFICATION.md#L111) records hosted CI and Nightly success. This is non-blocking for the verified implementation and integration paths, but should be reconciled before or during closeout.
- **Historical hygiene warning.** The phase report retains inherited branch-wide whitespace diagnostics in acceptance history. It explicitly says scoped changes and required evidence controls pass and that the warning does not invalidate acceptance.
- **Operational stop preserved.** The older interrupted full-CI/AccuracyCoin run remains unknown. The later exact-candidate evidence does not relabel that run or authorize restarting it. No hosted action, campaign restart, push, merge, or release was part of this audit.

No active Nyquist validation hook is configured, so Nyquist status was not applicable to this audit.

## Conclusion

All four requirements and the single phase pass; all five traced end-to-end flows complete. There are no critical requirement, integration, or flow gaps. The audit is `tech_debt` because of the stale README acceptance status and the phase report's retained historical hygiene warning.

## Closeout disposition — 2026-11-01

The owner chose to complete v7 while carrying both non-blocking audit items
forward. No requirement or verification status was changed, no new audit item
was acknowledged, and 16 items acknowledged at earlier milestone closes remain
carried in STATE.md. The README wording and historical hygiene warning remain
visible debt. The older interrupted full-CI/AccuracyCoin run remains unknown;
its restart stop is unchanged.
