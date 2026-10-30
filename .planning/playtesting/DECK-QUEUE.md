# Steam Deck NES play queue

Prepared 2026-10-30 at the owner's request. Open **Collections → Nesturbator** in
ES-DE. The collection now references 23 normal-library NTSC entries. Five have
positive owner gameplay reports; the 18 remaining choices are ordered below.
No game was launched automatically for this preparation.

## Where to resume

Start with **Gradius (USA)**, then **Solar Jetman (USA)** and **Balloon Fight
(USA)**. Those add CNROM, AxROM and NROM to the Deck's existing MMC1, UxROM and
MMC3 smoke signal. Completion of these three sessions would represent every
currently supported mapper family, not every board variant or timing edge case.

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
| 1 | Gradius (USA) | 3 — CNROM | Play the first stage; collect power-ups and watch graphics changes, scrolling, music and busy sprite scenes. Adds CHR-bank-switching coverage. |
| 2 | Solar Jetman - Hunt for the Golden Warpship (USA) | 7 — AxROM | Launch, rotate/thrust, enter a cave and return toward the ship. Adds single-screen mirroring and 32 KiB banking. Its inertia is part of the controls; unfamiliar handling alone does not establish input lag. |
| 3 | Balloon Fight (USA) | 0 — NROM | A few stages, then Balloon Trip if desired. Watch the stars and the scrolling split; this adds a simple no-bank-switching control case on the Deck. |
| 4 | Kirby's Adventure (USA) | 4 — MMC3 | Let the drawing intro run, then try doors, inhaling/copy abilities and the map. Varied graphics transitions, sprites and battery-backed progress. |
| 5 | Battletoads (USA) | 7 — AxROM | Intro audio and first-stage combat; the second-stage descent is useful if reached naturally. Timing-sensitive scrolling/splits and sprite load. No need to beat the game or reach its hardest stages. |
| 6 | Super Mario Bros. (World) | 0 — NROM | World 1-1, power-ups and a pipe transition. Sprite-0/status split and sprite priority add more signal than the mapper label alone. This repeats the earlier Mac title on the Deck. |
| 7 | Blaster Master (USA) | 1 — MMC1 | Drive, exit/re-enter the vehicle and enter an interior area. Check play-mode, scrolling and room transitions. |
| 8 | Mega Man 2 (USA) | 1 — MMC1 | Stage select, a stage such as Metal Man, ladders and pause/weapon menu. Listen to music mixed with effects. |
| 9 | DuckTales (USA) | 2 — UxROM | Amazon stage, pogo action, climbing and room transitions. A second UxROM game with different graphics and movement. |
| 10 | Street Gangs | 4 — MMC3 | This is the existing River City Ransom (USA) entry. Try scrolling fights, crowded scenes and a shop/menu if reached. Single-player is sufficient; two-player input remains a separate check. |
| 11 | Mega Man 3 (USA) | 4 — MMC3 | Stage select, a stage, ladders, slide and pause menu. Compare general stability with Mega Man 2 without expecting identical game mechanics. |
| 12 | Castlevania | 2 — UxROM | Existing USA Rev A entry. Try stairs, whipping candles, doors and the first stage's transitions. |
| 13 | Batman - The Video Game (USA) | 4 — MMC3 | First-stage movement, wall jumps, combat and pause. Another scrolling/sprite-heavy platformer. |
| 14 | Bionic Commando (USA) | 1 — MMC1 | Enter an action area, grapple and move between screens; check menus and action transitions. Lack of a normal jump is intentional. |
| 15 | Little Nemo - The Dream Master (USA) | 4 — MMC3 | First-stage movement and an animal transformation. Check changes in character graphics, scrolling and audio. |
| 16 | Shatterhand (USA) | 4 — MMC3 | First-stage combat, jumps and a robot companion if reached. Check sprite load and background stability. |
| 17 | Journey to Silius (USA) | 1 — MMC1 | First-stage movement, combat and weapon/menu use. Listen to the music/effects mix. |
| 18 | Metroid (USA) | 1 — MMC1 | Move through several rooms, acquire/use the Morph Ball and try a door transition. USA NES Metroid uses passwords; it is not a battery-save check. |

Gradius and Battletoads are the two additions to the original shortlist.
Gradius fills its missing CNROM family; Battletoads supplies another AxROM game
with well-documented timing demands. The other 21 entries are supported-region
choices from the owner's list, including the five already played.

## Already reported on the Deck

| Entry | Owner feedback | Important limit |
|---|---|---|
| Super Mario Bros. 3 | Good input timing, visuals and audio, low perceived latency, no observed issues after host input repair | No instrumented latency or full-game result |
| Contra | Everything plays well, similar positive feedback to Mario 3 | Duration and full-game coverage unreported |
| Shadow Warriors | Ninja Gaiden (USA); plays great | Existing scraped display alias; no detailed per-subsystem report |
| The Legend of Zelda | Plays great; prior progress appeared | Consistent with shared battery-save storage; prior writer/core and a new write/relaunch cycle unverified |
| Crystalis | Looks, plays and sounds great | Thin patterned top-edge strip captured privately; exposed overscan plausible, exact hardware correctness unverified |

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

- Reused seven existing NTSC entries and transferred 16 missing NTSC variants
  into the normal NES library over SSH. The collection contains references,
  with one selected entry per title. Existing PAL variants remain intact;
  new entries identify their USA/World region to distinguish them in the main
  library. No separate test ROM directory was created.
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
  frontend was opened. No core, display or controller configuration changed.
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
