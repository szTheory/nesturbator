# Steam Deck NES play queue

Prepared 2026-10-30 at the owner's request. Open **Collections → Nesturbator** in
ES-DE. The collection now references 26 normal-library NTSC entries. Feedback now covers 22 titles, including open visual/slowdown observations;
the four entries without a Deck gameplay report are ordered below.
For maximum signal per minute, use the [targeted scene playlist](SCENE-PLAYLIST.md)
first; several of its probes revisit specific scenes in already-played games.
No game was launched automatically for this preparation.

## Where to resume

The Shatterhand presentation comparison was activated and read back: regular
VSync on, Adaptive VSync off. Some falling/left-right disturbances and busy-scene
slowdown remain; the exact original drifting-line comparison is inconclusive.
No fix is credited. Further forced replay/reset is unnecessary; use the private
captures and bounded native recipes before requesting more owner repetition.

The corrected-port MM3 retest still showed crowd/damage slowdown and visual
observations. See [controls](CONTROLS.md), [scene findings](SCENE-PLAYLIST.md)
and [automated replay/CI direction](AUTOMATION.md).

Spend a few enjoyable minutes per game, or keep playing if it is fun. Report
controls/timing, graphics, audio and anything unusual. If something goes wrong,
the scene, action and whether it repeats are more useful than a generic fail.
An approximate duration helps distinguish brief startup from longer play.
Stop or skip freely; no full playthrough or required manual release gate is
introduced. The order is a recommendation, not the collection's alphabetical
display order.

## Remaining games, in recommended order

The displayed names below match the prepared entries. Mapper IDs come from the
actual selected images, rather than the original informal annotations.

| Order | ES-DE entry | Mapper | Fun short-session target and useful signal |
|---|---|---|---|
| 1 | Super Mario Bros. (World) | 0 — NROM | World 1-1, power-ups and a pipe transition. Sprite-0/status split and sprite priority add more signal than the mapper label alone. This repeats the earlier Mac title on the Deck. |
| 2 | Street Gangs | 4 — MMC3 | This is the existing River City Ransom (USA) entry. Try scrolling fights, crowded scenes and a shop/menu if reached. Single-player is sufficient; two-player input remains a separate check. |
| 3 | Castlevania | 2 — UxROM | Existing USA Rev A entry. Try stairs, whipping candles, doors and the first stage's transitions. |
| 4 | Journey to Silius (USA) | 1 — MMC1 | First-stage movement, combat and weapon/menu use. Listen to the music/effects mix. |

Gradius, Battletoads, Jurassic Park, Marble Madness and The Magic of
Scheherazade are additions to the original shortlist. The other 21 entries are
supported-region choices from the owner's list. The three latest additions
supply quickly reachable or built-in-password-accessible documented targets;
the password's battle encounter route remains optional and not time-qualified.

## Already reported on the Deck

| Entry | Owner feedback | Important limit |
|---|---|---|
| Super Mario Bros. 3 | Good input timing, visuals and audio, low perceived latency, no observed issues after host input repair | No instrumented latency or full-game result |
| Contra | Everything plays well, similar positive feedback to Mario 3 | Duration and full-game coverage unreported |
| Shadow Warriors | Ninja Gaiden (USA); plays great | Existing scraped display alias; no detailed per-subsystem report |
| The Legend of Zelda | Plays great; prior progress appeared | Consistent with shared battery-save storage; prior writer/core and a new write/relaunch cycle unverified |
| Crystalis | Looks, plays and sounds great | Thin patterned top-edge strip captured privately; exposed overscan plausible, exact hardware correctness unverified |
| Gradius (USA) | Plays really well, no issues | Specific graphics-bank transitions and duration were not separately reported |
| Solar Jetman (USA) | Playing well | Specific cave transitions and duration were not separately reported |
| Balloon Fight (USA) | Plays great, particularly smooth | Balloon Trip mode and its split have not been explicitly reported |
| Kirby's Adventure (USA) | Plays great | Specific transition and save/relaunch checks remain unqualified |
| Battletoads (USA) | Plays great, no observed issues | Level-two descent was not explicitly reported |
| Batman - The Video Game (USA) | Plays great | Generic session result, no named later scene reported |
| Bionic Commando (USA) | Plays great | No named later scene or full-game claim |
| Jurassic Park (USA) | Tearing reported in the wobbling OCEAN logo | Timing candidate; captured privately and native route replayable, cause unconfirmed |
| Marble Madness (USA) | First level and part of second play/look good; timer font/outline questioned | Native close-up contains blue edging and a small opening in 4; exact host/reference comparison remains open |
| The Magic of Scheherazade (USA) | General village play looks good; NPC layering matched a comparison video | User confirmed title; exact video URL/hardware provenance and RPG-battle damage probe unverified |
| Blaster Master (USA) | Level one looks good; top-edge pixels and some slowdown | Original lag is documented generally; this scene's slowdown remains unattributed |
| DuckTales (USA) | Good overall, smooth controls; slowdown at first red plant | Specific first-red-plant slowdown remains unattributed; private live capture available |
| Little Nemo (USA) | Controls, sound and general graphics good; scrolling edge, jump-time line and HUD-border flicker reported | Three separate observations; original-hardware correctness not established |
| Mega Man 2 (USA) | Looks good overall with some slowdown | Corrected title explicitly; area/cause unverified |
| Mega Man 3 (USA) | Good sound/music and tight input; stage-select line, vertical-scroll seam and severe crowded/damage slowdown reported | P1/P2 duplicate mapping corrected and read back on relaunch; owner reports crowd/damage slowdown persists near 59.97 FPS, with separate stage-select/vertical-scroll observations |
| Metroid (USA) | Going well early; slowdown around bats while shooting | Early bat/shooting slowdown remains unattributed; built-in passwords proposed for efficient scene access |
| Shatterhand (USA) | Tight controls, good audio/animations and smooth play; intro artifact, sprite flicker, bottom edge and drifting horizontal line reported | Captured first-stage scene; line reportedly travels downward and wraps to top, so regular-VSync-on/adaptive-off read-back passed on relaunch, with remaining disturbances and no confirmed fix |

## Progress and host checks, when useful

Ordinary play comes first. Separately, a useful next progress check is a normal
in-game save in Zelda or Crystalis, or normal automatic progress saving after
clearing a stage in Kirby, followed by an orderly exit and relaunch of that same
entry. Existing selected-game progress was backed up
privately before queue preparation. Preserve valuable progress and avoid
overwriting another core's snapshots. Battery saves and emulator save states
are different compatibility contracts.

A same-core snapshot save → advance → restore check can follow when desired.
It would not establish compatibility with other cores or future versions.
Controller unplug/reconnect, handheld fallback and a second player are also
separate checks; none is credited from the single-controller reports.

Expected NES flicker, game slowdown and edge graphics need reference evidence
before classification as defects. Preserve unexplained observations, including
the Crystalis strip, rather than hiding them behind a crop setting and declaring
them correct. A brief game session complements the synthetic timing/mapper
tests; it does not replace them.

## Original requests outside current support

| Title | Why it is outside this Nesturbator queue |
|---|---|
| Castlevania III - Dracula's Curse (USA) | Mapper 5 / MMC5 is unsupported |
| Punch-Out!! (USA) | Mapper 9 / MMC2 is unsupported |
| Gimmick! (Japan) | Mapper 69 / Sunsoft-5B is unsupported |
| Ufouria - The Saga (Europe) | The supplied image is mapper 4, but PAL timing is unsupported; its legacy header does not reliably identify that regional requirement |

These are not gameplay failures. They retain other emulator choices and were
not added to the Nesturbator collection. Future mapper/region work requires its
own scope; this preparation does not implement it.

## Setup evidence and preservation

- Initial expansion reused seven existing NTSC entries and transferred 16 missing NTSC variants
  into the normal NES library over SSH. The collection contains references,
  with one selected entry per title. Existing PAL variants remain intact;
  new entries identify their USA/World region to distinguish them in the main
  library. No separate test ROM directory was created.
- The scene-focused follow-up added three new NTSC titles, bringing the collection
  to 26. All reference/core/hash checks and fresh ES-DE startup passed; the
  selected current progress was backed up privately.
- Archive hashes and per-game Nesturbator choices were checked on the Deck.
  Original library files, original metadata and save storage were checked
  against the pre-transfer snapshot and preserved. Existing descriptive/media
  metadata was reused for new regional entries where available; existing
  entries and their play history were not rewritten.
- All selected images passed admission and 120 no-input frames in the current
  local native runner. This is a short startup preflight, not an interactive
  game pass, measured performance result or proof of Linux gameplay.
- The normal Steam shortcut started ES-DE; its fresh log loaded the expanded
  Nesturbator collection and completed startup. No game was running when the
  frontend was opened. The later refresh followed the owner's return to ES-DE
  and used a normal application-close event, with no force termination. No core, display or controller configuration changed.
- Raw manifests, hashes, backups, saves, images and connection details remain
  in private operational storage. See the [baseline](2026-10-30-nes-baseline.md)
  for individual reports and [lessons](LESSONS.md) for reusable mechanisms.

## Sources and interpretation

The current [public cartridge contract](../../include/nesturbator.h) specifies
mappers 0, 1, 2, 3, 4 and 7 with bounded supported board profiles. Commercial
selection cannot exercise every submapper, RAM layout or MMC3 chip revision.

[NESdev's tricky-game notes](https://www.nesdev.org/wiki/Tricky-to-emulate_games)
document Balloon Fight's PPUDATA use and Balloon Trip split, Battletoads'
timing-sensitive single-screen scrolling, Super Mario Bros.' sprite-0
dependencies, and Crystalis' MMC3 split. These motivate the observations above;
the chosen scenes are practical smoke-test suggestions, not exhaustive
reproductions of every documented edge case.

[NesCartDB's Gradius cartridge record](https://nescartdb.com/profile/view/786/gradius)
provides a physical CNROM reference, alongside the actual selected header and
successful admission. [NESdev's game-bug notes](https://www.nesdev.org/wiki/Game_bugs)
also document a Crystalis scanline artifact near its status bar/text boxes;
that does not identify the separately observed top-of-picture strip.
