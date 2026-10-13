# Phase 13: Rewind and run-ahead in RetroArch - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-10-12
**Phase:** 13-rewind-and-run-ahead-in-retroarch
**Areas discussed:** RetroArch e2e oracle, Run-ahead host coverage, Six-platform proof order, State fuzz target shape

The owner selected all four areas. Instead of answering each one, they asked for a research
fan-out per area: every stakeholder lens, primary sources, an adversarial pass and one coherent
recommendation per area, followed automatically. They added a preference for copying code over
adding a dependency. Each area was researched by an advisor agent (minimal_decisive tier) and
the recommendations were reconciled in CONTEXT.md.

---

## RetroArch e2e oracle

| Option | Description | Selected |
|--------|-------------|----------|
| Identify-by-match | Pause and step over network commands. Identify each screenshot's frame index by matching it against the runner's continuous frames, and assert relative indices, state-file facts and log lines. | ✓ |
| Pinned absolute frame | Sleep, pause, then compare against runner frame N. Flaky, and can't pin rewind or run-ahead. | |

**User's choice:** the research recommendation (fan-out brief).
**Notes:** A live probe of RetroArch 1.22.2 found that `GET_STATUS` segfaults and that only
`VERSION`, `GET_CONFIG_PARAM` and `READ_CORE_*` reply. Logs follow the machine locale unless
`user_language = "0"` is set. `nc -u` is the sender, with no owned UDP tool.

---

## Run-ahead host coverage

| Option | Description | Selected |
|--------|-------------|----------|
| Single-instance host through the adapter, plus a library-level two-instance test and anti-vacuity controls | Tests the shipped entry points cheaply on six runners. | ✓ |
| Also dlopen two copies of the adapter | Mirrors RetroArch's second-instance mode, but needs a host refactor and is fragile on Windows. | |

**User's choice:** the research recommendation.
**Notes:** Orchestrator change during synthesis: the gating host feeds the speculative frames the
script's real future inputs, so "visible frames equal an uninterrupted run" holds exactly at
every tick. RetroArch-style repeated input is checked against the two-instance model.

---

## Six-platform proof order

| Option | Description | Selected |
|--------|-------------|----------|
| Claim guard in the suite, plus an early draft PR | A configure-time guard requires the proving tests for each claimed level, and an `.info` claims test checks the exact key set. Pushing mid-phase is an early warning. | ✓ |
| Separate PR per claim | Forces an intermediate release with only `serialized`, and breaks one PR per phase. | |

**User's choice:** the research recommendation.
**Notes:** The roadmap order (`serialized` first, `deterministic` in the last commit) is kept as
commit discipline, with the six `build` jobs green before the `serialized` commit.

---

## State fuzz target shape

| Option | Description | Selected |
|--------|-------------|----------|
| Synthetic in-memory carts, a patch over a baseline state, and the CRC fixed up in the harness | No ROM bytes, the same on six platforms, and the shipped code path. | ✓ |
| Real test ROMs from disk, with raw state files | Realistic, but tied to file paths and needs provenance bookkeeping. | |

**User's choice:** the research recommendation.

---

## Claude's Discretion

- File split, helper names and frame counts within the budgets.
- Whether the state-session RetroArch keys are global or go in a second cfg template.

## Deferred Ideas

- Two dlopened copies of the adapter for second-instance run-ahead.
- RetroArch second-instance e2e variant.
- Real-ROM fuzz seeds, a longer fuzz frame mode and a cross-platform fuzz digest.
- Run-ahead cost row in the speed report (Phase 15).
