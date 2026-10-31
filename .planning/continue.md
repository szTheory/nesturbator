# Resume v6 after Phase 22 Plan 22-02

Read `AGENTS.md`, `.planning/STATE.md`, `.planning/REQUIREMENTS.md`,
`.planning/ROADMAP.md`, and
[Phase 22 context](phases/22-install-and-protect-progress/22-CONTEXT.md).

## Outcome and next actor

Plans 22-01 and 22-02 are complete. Plan 22-02 moved the runner contract and
examples into [docs/runner.md](../docs/runner.md), with option, exit, stop,
status-table and link controls. Its scoped checks passed; full CI remains with
the phase executor. Read the [22-02 summary](phases/22-install-and-protect-progress/22-02-SUMMARY.md)
and [22-03 plan](phases/22-install-and-protect-progress/22-03-PLAN.md).

The owner continues the phase by invoking:

`$gsd-execute-phase 22`

This resumes the remaining sequential plans, starting with release and
engineering/testing guide migration. Context may be cleared first. Read STATE,
this resume file and the next plan. Keep READY-01 and READY-06 pending until
Phase 22 is complete; do not start Phase 23 early.

Six fallback edge assumptions and six descriptor-less prohibitions are explicitly
flagged for downstream review. They do not create an owner gameplay/sign-off gate
or qualify cross-build SRAM or incompatible host autoload recovery. Current release
availability and host menu wording remain execution-time observations.

## Constraints to carry forward

- Keep support and literal marked installation in the player README; prefer
  focused API/runner/release/testing guides and a player progress walkthrough.
  Keep current archive layout; guides live in the repository.
- Retain independent doc/package checks and negative controls. Cover moved
  canonical API/runner lookup and links without weakening assertions.
- Recheck published assets during implementation. Host name/version alone is
  insufficient build identity; use actual module path/digest and provenance.
- Host controls use the real frontend launch and distinct active player ports.
  Existing daily input and automated tests retain their separate evidence limits.
- Snapshot support recommendation is exact build/content. Native format/content
  checks differ from recorded but unenforced behavior revision and host containers.
  Old snapshots restore old battery RAM too. Preserve newest battery separately;
  cross-build SRAM and incompatible host-autoload recovery remain unqualified.
- Use scratch permitted fixtures and existing refusal tests. No personal device,
  daily progress operation, commercial sweep or new live-testing window is granted.
- Phase 23 owns report/replay/triage. Accuracy/overscan, regions/mappers, converters,
  actual Playstead, generic testing proposals and collector debt remain outside
  this phase. Existing safety stops and runtime allowances persist.
- Work on `gsd/phase-22-install-and-protect-progress`. Preserve the owner's
  pre-existing unstaged `AGENTS.md` and `.planning/config.json` edits. Do not change
  global GSD/model settings in response to known runtime warnings.

Plan 22-02 changed documentation and test-checker coverage only. It established
no runtime, package-install, host-recovery or gameplay result. Alternatives are
in the human-only discussion log; downstream agents use the context.
