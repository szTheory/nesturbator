---
id: SEED-261009-zs2
status: dormant
planted: 2026-10-09
planted_during: Phase 04.2 ship, v1 milestone closeout
trigger_when: before a future phase or release claim that a supported game is playable end to end, or when a milestone includes game/frontend integration
scope: medium
audit_acknowledged:
  milestone: v1
  at: 2026-10-09
  status: dormant
---

# SEED-261009-zs2: Prove one supported NROM game from boot through interactive play

## Why This Matters

Nesturbator's Phase 03 evidence covers real-game frames, deterministic input movies, and a hosted RetroArch capture; Phases 04 and 04.2 cover audio and libretro callback behavior. These checks establish important parts, but no recorded run combines a meaningful start, responsive gameplay input, scrolling or scene movement, and audible output in one supported game. A single integrated path is strong evidence for a user-facing playability claim. This is a future improvement, not a retrospective change to the audited v1 acceptance contract.

## When to Surface

**Trigger:** Before a later phase or release claims that a supported game is playable end to end, and when a milestone changes the real-game/frontend path.

Review this seed at `$gsd-new-milestone` when gameplay or frontend integration is in scope.

## Scope Estimate

**Medium** — choose one suitable mapper-0 game, define a defensible boot-to-play predicate and scripted input, then extend the existing RetroArch smoke path to observe game-state progress, visible response/scrolling, and non-silent audio.

## Breadcrumbs

- `.planning/phases/03-a-real-game-in-retroarch/03-CONTEXT.md` — selected licensed mapper-0 games and the original RetroArch frame/input scope.
- `.planning/phases/03-a-real-game-in-retroarch/03-VERIFICATION.md` — exact-SHA frame capture, movie replay, and separate integration evidence.
- `.planning/phases/04-sound/04-VERIFICATION.md` and `.planning/phases/04.2-close-gap-snd-01-deliver-single-sample-libretro-audio/04.2-VERIFICATION.md` — deterministic audio and libretro callback coverage.
- `tests/roms/manifest.txt` — permitted committed test ROMs and their provenance.
- `.planning/preparation/GSD-HANDOFF.md` — bounded guest diagnostics, gap closure, and evidence rules.

## Notes

- Prefer a redistributable game already admitted by `tests/roms/manifest.txt`; verify the exact artifact and license before adding any fixture.
- Aim to automate recurring boot, control, rendering, and audio checks in CI through the existing host/RetroArch paths when the game fixture can legally be used there.
- Keep private game ROMs, input movies, captures, and derived traces local and untracked. If acceptable CI input is unavailable, record the remaining acceptance evidence as blocked while continuing independent work with verified prerequisites.
- A title screen, execution time, input activity, or nonzero device counters alone is not proof of gameplay. Define the observable game-state transition before running the smoke.
- The proposal came from a sibling emulator's operational experience; NES hardware behavior must still be established from Nesturbator's own cited references and tests.
