# Resume v6 at Phase 22 execution

Read `AGENTS.md`, `.planning/STATE.md`, `.planning/REQUIREMENTS.md`,
`.planning/ROADMAP.md`, and
[Phase 22 context](phases/22-install-and-protect-progress/22-CONTEXT.md).

## Outcome and next actor

Phase 22 planning is complete. The [research](phases/22-install-and-protect-progress/22-RESEARCH.md)
and four plans are ready; [independent verification](phases/22-install-and-protect-progress/22-PLAN-CHECK.md)
passed with no blockers or warnings. Requirements and adopted decisions are fully
mapped in the [source audit](phases/22-install-and-protect-progress/22-SOURCE-AUDIT.md).

The owner acts next by invoking:

`$gsd-execute-phase 22`

This implements and verifies four sequential plans: API relocation/controls,
runner contracts, release/testing guides, then player install/control/progress.
Context may be cleared first. Read each plan and its dependencies; no implementation
or runtime/package/host verification has run. Stop after this planning step;
do not auto-chain execution or start Phase 23.

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

The planning step changed planning artifacts only. It established no new runtime,
package-install, host-recovery or gameplay result. Alternatives are in the
human-only discussion log; downstream agents use the context.
