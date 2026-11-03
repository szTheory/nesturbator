---
milestone: v8
audited: "2026-11-03T10:18:04Z"
status: tech_debt
scores:
  requirements: "7/7 satisfied"
  phases: "4/4 passed"
  integration: "7/7 wired"
  flows: "5/5 complete"
gaps:
  requirements: []
  phases: []
  integration: []
  flows: []
tech_debt:
  - phase: "25"
    items:
      - "Two plan-level transparency prohibitions remain flagged: source-derived PAL timing is not hardware qualification, and the older interrupted campaign remains stopped with unknown outcome."
      - "PAL $4017 delay and reset-inhibit duration remain disclosed model assumptions."
  - phase: "26"
    items:
      - "Direct 2A07 proof, later held no-op-cycle effects, and PAL abort/reload behavior remain unverified."
      - "The DMA-01 and STATE-01 evidence prohibitions remain flagged; no universal historical or cross-build compatibility claim is made."
      - "There is no PAL-specific runner-process state round-trip fixture; shared state API coverage and the real libretro adapter cover the implementation."
  - phase: "27"
    items:
      - "WR-01 remains open: tracked baseline metadata points to an earlier source revision and capture path. Current candidate campaign evidence was separately verified and is not invalidated."
      - "The three plan-level prohibitions remain flagged: physical host pacing/device behavior, silicon color calibration, and the stopped historical campaign are not claimed."
  - phase: "project"
    items:
      - "The older interrupted CI/AccuracyCoin campaign remains stopped and unknown; this audit did not restart it."
nyquist:
  overall: not_applicable
  reason: "No active validate-phase verify:post hook is configured."
---

# v8 Milestone Audit — PAL region support

**Result: tech_debt.** All seven requirements are satisfied, all four phases have passing verification, and the integration review found no broken wiring or end-to-end flow. The open WR-01 provenance-maintenance warning and the explicitly retained evidence boundaries remain visible for closeout.

## Scope and definition of done

The approved v8 scope covers region selection, PAL timing and DMC behavior, regional state continuation, modeled PAL presentation, host rate reporting, and NTSC preservation across Phases 25, 26, 27, and 27.1. The canonical milestone manager reports all four phases implemented, roadmap-complete, and verified. The manager has no remaining phase actions.

This audit reuses the current phase verification reports and the current-head local workflow receipt recorded by Phase 26: 722 tests passed and the library, runner, and libretro packages were produced. The current HEAD is a documentation-only descendant of that candidate; the audit did not run CI or restart the older stopped campaign. The older campaign remains stopped and unknown.

## Requirements cross-check

The three sources were checked: REQUIREMENTS.md traceability, phase verification tables, and machine-readable requirements-completed metadata extracted from the phase summaries.

| Requirement | Traceability owner | Verification | Summary metadata | Audit result |
|---|---|---|---|---|
| REG-01 | Phase 25, complete | Passed and satisfied | Listed in Phase 25 summaries | **Satisfied** |
| PAL-01 | Phase 25, complete | Passed and satisfied | Listed in Phase 25 summaries | **Satisfied** |
| DMA-01 | Phase 26, complete | Passed and satisfied | Listed in Phase 26 Plan 01 summary | **Satisfied** |
| STATE-01 | Phase 26, complete | Phase 26 and supplemental Phase 27.1 passed | Listed in Phase 26 Plan 02 and Phase 27.1 summaries | **Satisfied** |
| VID-01 | Phase 27, complete | Passed and satisfied | Listed in Phase 27 Plan 04 summary | **Satisfied** |
| AV-01 | Phase 27, complete | Passed and satisfied | Listed in Phase 27 summaries | **Satisfied** |
| NTSC-01 | Phase 27, complete | Passed and satisfied | Listed in Phase 27 summaries | **Satisfied** |

**Score: 7/7 satisfied.** No requirement is orphaned. Phase 26 remains the sole owner of STATE-01; Phase 27.1 adds supplemental validation evidence without changing that ownership.

## Phase verification and readiness

| Phase | Verification | Canonical readiness | Result |
|---|---|---|---|
| 25 — Region selection and PAL timing | Passed, 23/23 | Roadmap complete; implementation complete | Region policy, PAL timing/pitch, host forwarding, and NTSC safeguards verified |
| 26 — PAL DMA and regional state continuation | Passed, 11/11 | Roadmap complete; implementation complete | DMA-01 and STATE-01 verified; summary attribution and verification fingerprint are current |
| 27 — PAL host presentation and NTSC preservation | Passed, 16/16 | Roadmap complete; implementation complete | Requirements pass; WR-01 remains a non-blocking provenance warning |
| 27.1 — Close STATE-01 regional state validation gaps | Passed, 7/7 | Roadmap complete; implementation complete | Supplemental endpoint and regional frame-counter cases verified |

**Score: 4/4 phases passed and complete.** The previous audit's two documentary findings are resolved: the Phase 26 summary metadata now attributes DMA-01 and STATE-01 to their implementing plans, and the canonical manager recognizes Phase 27.1 as roadmap-complete.

## Cross-phase integration

The integration checker verified production paths through the library, runner, and libretro adapter. It found seven connected requirement paths, no orphaned scoped exports, no missing connections, no integration warnings, and no blockers. HTTP routes, web UI, database, and authentication checks do not apply to this C emulator.

| Requirement | Integration path | Status |
|---|---|---|
| REG-01 | NES 2.0/iNES metadata and eligible host preference → shared loader → effective region → timing and host reports | **WIRED** |
| PAL-01 | Effective region → CPU/PPU/APU/frame/sample timing → independent six-mapper frame and pitch checks | **WIRED** |
| DMA-01 | APU request → CPU SYNC classification → PAL bus arbitration → DMC halt/GET/resume and wrong-model control | **WIRED** |
| STATE-01 | Loaded region → identity and regional bounds checked before apply → native, runner, and libretro restore paths; Phase 27.1 adds endpoint/frame-counter validation | **WIRED** |
| VID-01 | Effective region → modeled palette/emphasis lookup → shared RGB conversion → runner PPM and libretro video output | **WIRED** |
| AV-01 | Shared region/info APIs → runner profile output and libretro region/frame/audio-rate reporting | **WIRED** |
| NTSC-01 | NTSC behavior retained through region branches → six-mapper regression matrix and preserved baselines | **WIRED** |

## End-to-end flows

| Flow | Result | Evidence |
|---|---|---|
| Region selection and reporting | **Complete** | Metadata/preference resolution, timing profile, reload behavior, Dendy refusal, and runner/libretro reports are connected |
| PAL frame and audio behavior | **Complete** | Regional CPU/PPU/APU cadence and six-mapper timing/pitch checks |
| PAL DMC arbitration | **Complete within the documented model** | Production request-to-SYNC-to-bus path plus same-history wrong-model control |
| State continuation and refusal | **Complete** | Same-region continuation; cross-region and invalid-state refusal before mutation; Phase 27.1 endpoint/frame-counter cases |
| Regional presentation and NTSC preservation | **Complete** | Modeled palette conversion reaches runner/libretro output; six-mapper PAL/NTSC acceptance preserves NTSC baselines |

**Score: 5/5 flows complete.** Integration review also traced reload and refusal paths: explicit single-region metadata takes precedence over a conflicting preference, invalid and Dendy loads do not replace active content, option changes apply on reload, no-content reporting falls back to NTSC, and cross-region state loads are refused without mutation.

## Deferred evidence and technical debt

- Phase 25 retains two transparency prohibitions. PAL timing and reset-edge expectations are source-derived model evidence, not direct hardware qualification; the $4017 delay and reset-inhibit duration remain disclosed assumptions.
- Phase 26 retains the direct 2A07, later no-op-cycle, and PAL abort/reload evidence limits. The owned wrong-model control demonstrates sensitivity to the chosen rule, not physical hardware behavior. The PAL-specific runner-process state round-trip is not separately fixture-tested; shared state API tests and the real libretro adapter cover the behavior.
- Phase 27's WR-01 warning is open: checked-in baseline metadata still points to an earlier source revision and capture path. The current candidate's campaign metadata and report were separately checked, so this warning does not invalidate the current acceptance evidence.
- Phase 27 keeps its three plan-level transparency prohibitions unresolved. The modeled 2C07 palette is not universal calibration, software AV reports do not establish physical host pacing, and the old campaign remains stopped and unknown.
- The stopped campaign was not resumed, and no hardware, hosted CI, push, merge, publication, or device action was part of this audit.

These are tracked evidence limits and maintenance debt, not unsatisfied software requirements or broken flows. They do not authorize stronger hardware/host claims or restarting the older campaign.

## Verification disposition

No new tests or CI were run for the audit. All four passing phase verification reports were read and cross-checked against their summary metadata and REQUIREMENTS.md. The configured verify:post hook list contains no validate-phase step hook, so Nyquist discovery is not active and was skipped.

## Closeout

No requirement closure phase is needed. The remaining owner choice is whether to complete v8 while carrying WR-01 and the documented evidence limits, or to do a scoped metadata cleanup before archiving.

Recommended next command, if accepting the disclosed debt:

$gsd-complete-milestone v8

This archives v8 and records the agreed product direction; the command does not restart the stopped campaign or add hardware claims.
