# Targeted NES scene playlist

Updated 2026-10-30. This follows the owner's request to maximize useful feedback
per minute and explain classic failure cases before each game. Start with the
first two probes below; both reach their target almost immediately. All games
are in **Collections → Nesturbator**. The [full Deck queue](DECK-QUEUE.md)
tracks the rest of the original shortlist.

## Checkpoint so far

Eleven games have positive user-reported Deck play: Mario 3, Contra, Ninja
Gaiden/Shadow Warriors, Zelda, Crystalis, Gradius, Solar Jetman, Balloon Fight,
Kirby's Adventure, Battletoads and Batman. The owner reports smooth gameplay
and no perceived performance problems across these sessions. These are dated,
build-associated ordinary-play observations, complementary to CI. There are no
instrumented latency/frame-time measurements or full-game claims.

The reports span all six supported mapper IDs: 0, 1, 2, 3, 4 and 7. Individual
board variants and the named scenes below still need their own evidence.
Generic success in Battletoads or Balloon Fight does not establish that level
two or Balloon Trip was reached. Keep scene results separate from title reports.

## Highest-value pass

| Order | Game and exact target | What to watch | How to reach it |
|---|---|---|---|
| 1 | **Jurassic Park (USA): OCEAN logo** | Intentional wobble should remain coherent; report displaced horizontal lines, tearing or broken logo pieces. | Launch and let the opening play. Do not skip it. |
| 2 | **Marble Madness (USA): race-introduction text box** | Legible, stable text; report flickering, wrong tiles or a moving boundary across the box. | Start the first race and watch its introductory text. |
| 3 | **Crystalis: outdoor vertical scrolling, then dialogue** | Report a seam travelling through the playfield or a displaced split/palette boundary as the camera moves. | Use existing progress; walk north/south repeatedly outdoors, then open a conversation. Keep the already recorded top-edge strip separate. |
| 4 | **Balloon Fight (USA): Balloon Trip** | Watch the fixed HUD against the scrolling sky and the background stars; report unexplained split jumps or corruption. | Select Balloon Trip directly at the title menu. Ordinary stages are a different probe. |
| 5 | **Super Mario Bros. 3: item emerging from a block** | The emerging item should layer correctly against its block/background. | World 1-1: hit a power-up block and watch the emergence. |
| 6 | **The Legend of Zelda: vertical screen transitions** | Report persistent bad rows or displaced scrolling after several transitions. Brief quirks need reference comparison. | Use the existing save; cross several north/south screen boundaries. |
| 7 | **Battletoads (USA): level two / Wookie Hole descent** | Report a hang, blank playfield with continuing music, or unstable HUD/playfield alignment. | Beat stage one normally. The target is the descent itself, not another opening-level success. |
| 8 | **The Magic of Scheherazade (USA): damage during a menu-based party battle** | Check that the game continues when an enemy damages a party member. Overworld action damage is a different path. | Choose CONTINUE, enter **5W**, then move between outdoor screens until an RPG battle occurs; use ordinary commands until hit. Encounter timing is uncertain, so keep this optional/lower priority. |

The technical motivations come from [NESdev's tricky-game notes](https://www.nesdev.org/wiki/Tricky-to-emulate_games),
with firsthand [Marble Madness/Battletoads timing reports](https://forums.nesdev.org/viewtopic.php?start=15&t=12470).
Jurassic Park probes MMC3 IRQ timing; Marble Madness's text probes mid-frame
pattern selection; the selected USA image is AxROM, with no CHR bank register.
Crystalis probes its moving split and palette changes; Balloon Trip probes its
split and PPUDATA use. The Mario 3 target exercises sprite/background priority.
Scheherazade's target concerns a bank-boundary CPU execution path.

[NESdev's game-bug list](https://www.nesdev.org/wiki/Game_bugs) documents original
Crystalis/Zelda quirks too. A reported oddity is evidence to investigate, not an
automatic emulator failure or automatic proof of correct emulation. The private
Crystalis screenshot alone still does not classify its top-of-picture strip.

## Reaching later scenes without wasting play time

The **5W** password is a built-in chapter-five start with equipment, independently
documented by [a firsthand player guide](https://flyingomelette.com/mos/oddities.html).
It does not require Game Genie. That start has unusual but expected party/item
state. A [chapter-five walkthrough](https://flyingomelette.com/mos/guide_part5.html)
offers an outdoor route, but no guaranteed immediate RPG encounter was verified.
Do not spend a long session hunting a random fight solely for this checklist.

Game Genie is **not implemented in the current adapter**: `retro_cheat_set()` and
`retro_cheat_reset()` are empty in [libretro.c](../../libretro/libretro.c).
Only battery-save memory is exposed; frontend system-RAM cheat search is not a
general workaround. Do not promise that importing a RetroArch cheat file will
enable a level warp. The current launcher also disables automatic ROM patches.

The original Galoob [Game Genie codebook](https://octotube.com/nes-game-genie-full.pdf)
(page 13) publishes **ZAXAALAA** for Battletoads level two. This is a sourced
candidate for future cheat support, not a working option in this setup. It has
not been tested against the selected image here. No cheat support, altered ROM
or additional dependency was implemented during this preparation.

Once a useful scene is reached, a **snapshot made with this same core** in an
unused slot can preserve it for repeat checks. Existing selected progress was
backed up privately before this pass. Snapshot loading can restore battery RAM
and later affect its saved progress, so preserve daily-play data and identify
the slot. A useful private checkpoint record includes game/content identity,
core build and state format, scene, slot, access method and whether assistance
remained active. No scene snapshot has been created or qualified yet.

Foreign-core states are not portable checkpoints. The runner's native state
payload and RetroArch's disk container also require separate host qualification;
do not rename a raw runner state into a RetroArch slot and claim it works.
Native `NMOVIE1` input replay and state-at-frame exist, but no commercial scene
movie library or automatic scene navigator is currently prepared. A future
owned input recipe could regenerate checkpoints when a state format changes.

## Minimal feedback to record

For each probe, report **scene reached**, **looks/plays/sounds normal or what
changed**, and an approximate duration when convenient. If a failure repeats,
note the action that triggers it. Capture a screenshot/video privately when
useful. A successful generic game report stays credited even if the deeper
target has not been reached. If a shortcut modifies gameplay, record it and
recheck suspicious behavior through an unmodified path when practical.

## Preparation evidence

Jurassic Park (mapper 4), Marble Madness (mapper 7) and The Magic of Scheherazade
(mapper 1) were found in the owner's local library, admitted by the current
native runner, and each reached frame 120 without input. They were transferred
into the normal Deck library, with archive hashes checked. No prior entry with
those content names was present. This is startup preparation, not gameplay
acceptance of the scenes.

ES-DE was refreshed during the owner's stated gap, after confirming no game
was active. Remote shortcut/WM requests did not close it; a direct normal
`WM_DELETE_WINDOW` event to its verified window did. No forced termination or
machine restart was used. Original files, metadata and save storage passed
preservation checks. The fresh frontend log loaded the 26-entry collection and
completed startup; all references and per-game core choices passed read-back.
Current selected-game progress was backed up privately. Raw metadata, saves,
images and connection details remain outside Git.
