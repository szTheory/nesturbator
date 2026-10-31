# Resume v6 after Phase 22 Plan 22-03

Read `AGENTS.md`, `.planning/STATE.md`, `.planning/REQUIREMENTS.md`,
`.planning/ROADMAP.md`, and
[Phase 22 context](phases/22-install-and-protect-progress/22-CONTEXT.md).
Also read the [22-03 summary](phases/22-install-and-protect-progress/22-03-SUMMARY.md)
and the next plan.

## Outcome and next actor

Plans 22-01, 22-02 and 22-03 are complete. Plan 22-03 added canonical
[release](../docs/releases.md) and [testing](../docs/testing.md) guides, and
reduced README to the player route and focused developer links. Its six Task 1
checks passed before and after the tracer commit; the final Task 2 selection
passed 16/16. The marked install block and checked MMC3 clause remain unchanged.
Full CI remains with the serial phase executor.

The owner continues the phase by invoking:

`$gsd-execute-phase 22`

This continues with Plan 22-04, the player installation, controls and progress
route. Context may be cleared first. Read STATE, this resume file and Plan
22-04. Keep READY requirements pending until Phase 22 is complete; do not start
Phase 23 early.

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

Plan 22-03 changed documentation only and reused existing native test source
and evidence. It did not run full CI, a conformance campaign, package creation
or extraction, a live host session, private scene replay, or player-progress
operations. Preserve the existing stops and user-owned unstaged
`AGENTS.md` and `.planning/config.json` edits.
