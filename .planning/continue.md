# Resume v6 at Phase 22 discussion

Read AGENTS.md, [STATE.md](STATE.md), [PROJECT.md](PROJECT.md) and
[PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md). v6 **Player-ready NES and reproducible
feedback** is initialized. The owner selected the recommended scope with
“follow ur recs” and then approved the concrete requirements and two-phase
roadmap with “yes i approve” on 2026-10-30. Do not repeat scope selection,
requirements approval or the new-milestone lifecycle step.

## Outcome and next actor

The approved [requirements](REQUIREMENTS.md) map all ten items exactly once:
Phase 22 covers installation, controls, progress/version policy and executable
documentation contracts; Phase 23 covers the bounded report/replay/triage route.
The [roadmap](ROADMAP.md), project, product direction and state/handoff are
committed locally. No phase plans or implementation have been created; no push
or release is part of milestone opening.

The owner acts next by invoking:

`$gsd-discuss-phase 22`

This clarifies the approach for **Install and protect progress** before planning.
Context may be cleared first. No additional confirmation is needed to finish the
completed milestone opening; stop here rather than chaining another GSD step.

Reuse the [research](research/milestone-v6-discovery/SYNTHESIS.md),
[postmortem](playtesting/POSTMORTEM.md) and existing 22 title reports across six
supported mapper IDs. No further broad play sweep or generic research fan-out.
SEED-006 contributes measured documentation/contract repairs only; seed files
and previously delivered outcomes stay unchanged. The three pending testing
proposals have no direct requirement match and remain unlinked/deferred.
Static-runtime model/config warnings were observed during opening; no global
install or settings change was made. The roadmap was prepared inline under the
skill's no-explicit-subagent fallback, reusing existing research.

## Boundaries and retained evidence

- Overscan, actual core fixes, DMA, PAL/Dendy, new mappers and actual Playstead
  integration are outside v6. Following accuracy work is conditional RM-015 on
  a confirmed core fault, otherwise the five remaining DMA failures; preserve
  the three already-passing former candidates and all protected PASS results.
- Documentation relocation must retain the independent install, runner-option,
  stop/exit-table, status/state and info/release checks. Update their document
  lookup deliberately without weakening their assertions. Runtime changes, if
  an actual helper gap is demonstrated, require normal tests and README/header
  updates. No new dependency, shared host or generic replay framework.
- Preserve daily battery saves and core-specific snapshots. Native payloads and
  RetroArch disk containers are distinct. State format 3 rejects earlier formats;
  behavior revision 9 alone is not a general state compatibility guarantee.
- Reported seams, slowdown and music interruption remain unclassified. Repeated
  current RGB/PCM is repeatability evidence. The existing controller/VSync
  read-backs do not establish a visual fix. Read [automation](playtesting/AUTOMATION.md),
  [scene cards](playtesting/SCENE-PLAYLIST.md), [controls](playtesting/CONTROLS.md)
  and [lessons](playtesting/LESSONS.md) before any operational work. No live test,
  exhausted allowance, runtime stop or daily-progress operation is authorized by
  this opening. Commercial content/media/states remain private and ignored.
- v5 is archived with all eleven requirements complete. Keep its
  [closeout](milestones/v5-CLOSEOUT.md), [audit](milestones/v5-MILESTONE-AUDIT.md),
  collector/platform/provenance debt and historical source-bound acceptance.
  Release PR #56 was rechecked open; latest published release was v0.1.11 at
  opening. Publication and merged-source capability remain separate claims.
- Preserve unrelated pre-existing unstaged AGENTS.md and config edits. Do not
  install/update global GSD or change model settings as milestone housekeeping.
