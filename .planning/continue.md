# Resume after the NES playtesting checkpoint

Read AGENTS.md, [STATE.md](STATE.md), [PRODUCT-ROADMAP.md](PRODUCT-ROADMAP.md)
and the [mini postmortem](playtesting/POSTMORTEM.md). The latest owner request
was an updated game list, a diminishing-returns assessment and future milestone
recommendations with reusable lessons. Those are recorded; no further play is
required before selecting scope.

## Outcome and next actor

Feedback covers 22 of 26 prepared Deck titles and all six supported mapper IDs.
Many sessions have good controls, audio and general visuals; named seams and
scene-specific slowdown remain unclassified. This is ordinary-play smoke
evidence alongside CI, not full-library or hardware-accuracy qualification.

The owner acts next by selecting the next milestone. Prefer **v6 candidate:
Player-ready NES and reproducible feedback (RM-014)**: short truthful README,
identified install/core, controls and progress guidance, explicit state-version
backup/rollback policy and a bounded scene-report/reproduction recipe using
existing facilities. Optional overscan presentation remains a choice, not an
approved feature. No new frontend/framework or broad manual sweep is needed.

Following scope: RM-015 scene-driven accuracy only when bounded native/reference
triage establishes a core fault; otherwise resume the five remaining DMA
failures, preserving the three former candidates already passing. Keep actual
Playstead integration consumer-owned, PAL as the regional alternative, and new
mappers independently scoped. The [mapper scope note](research/milestone-v6-discovery/MAPPER-SCOPE.md)
records the six supported IDs, historical prevalence caveats and the three
additional IDs on the shortlist (9/5/69), with PAL separate. Favor MMC2/Punch-Out!!
when mapper expansion is chosen; it is not committed. All four horizons and
audit debt remain visible.

`$gsd-new-milestone`

This continues scope/version/seed selection and approval, then creates fresh
requirements/roadmap. Context may be cleared first. No next version, requirements
or phases have been approved; v5 stays archived. Do not run a second lifecycle
step to perform roadmap upkeep or infer runtime authority from these proposals.

## Operational evidence to preserve

- The normal Gaming Mode → ES-DE → RetroArch route uses the owner's real
  collection with switchable per-game cores, normal battery storage and
  core-specific snapshots. Commercial Mario 3 input was confirmed after actual
  launch-context repair; direct-launch success had not covered Steam Input.
- The Xbox accidentally fed both P1/P2. The single-controller launcher now
  leaves P2 unassigned; new-launch read-back passed. MM3 slowdown persists;
  high jumps were qualified by the owner as possibly level design, not proved
  cheat effects. Multiple physical players need separately qualified routing.
- Shatterhand regular VSync on/adaptive off was read back after relaunch. Some
  disturbances remain; the exact drifting-line comparison is inconclusive.
  No fix is claimed. Avoid another mandatory reset; preserve reached scenes.
- Four private recipes/checkpoints replay current RGB/PCM identically: Jurassic
  OCEAN, Marble timer, Nemo opening and MM3 menu. Nemo's moving-camera fault and
  MM3 menu navigation are not qualified; Shatterhand automation never reached
  gameplay within three bounded attempts. Native payloads are not qualified
  RetroArch containers. No general autonomous player or Game Genie support.
- Same-core Mario 3 automatic-state-load success was observed, distinct from
  Zelda battery reuse. A Shatterhand numbered-slot load failed because that slot
  did not exist; its separate auto-state was backed up. Manual host roundtrip
  and future-version compatibility remain separate claims.
- Private live captures are sampled and have no audio, so cannot measure frame
  pacing. A separate three-second output-monitor audio sample was captured;
  it does not prove the reported brief music interruption recurred.

Read [AUTOMATION.md](playtesting/AUTOMATION.md), [scene cards](playtesting/SCENE-PLAYLIST.md),
[controls](playtesting/CONTROLS.md), [baseline](playtesting/2026-10-30-nes-baseline.md)
and cross-project [lessons](playtesting/LESSONS.md) as needed. The ignored
private session's operator notes hold exact commands, connection details and
capture/state identities. Never copy those into Git. Existing helpers suffice
for the bounded loop; no new dependency, core fix or public CI fixture landed.

## Existing state and boundaries

v5 is complete: all eleven requirements, canonical 119 PASS/27 FAIL over 146
scored rows plus five unscored DRAW; ABI 1/state format 3/behavior 9. Read the
[closeout](milestones/v5-CLOSEOUT.md) and [audit](milestones/v5-MILESTONE-AUDIT.md)
for debt and evidence limits. Publication was pending when previously checked;
this documentation checkpoint did not recheck or publish a release.

Reuse completed [sibling research](research/milestone-v6-discovery/SYNTHESIS.md).
Do not repeat generic fan-out. Preserve unrelated unstaged config/AGENTS.md
changes, normal daily progress and historical safety/runtime stops. A new
milestone needs its own approved scope; none of those stops is renewed here.
