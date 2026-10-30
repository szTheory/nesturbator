# Targeted NES scene playlist

Updated 2026-10-30. This follows the owner's request to maximize useful feedback
per minute and explain classic failure cases before each game. Start with the
first two probes below; both reach their target almost immediately. All games
are in **Collections → Nesturbator**. The [full Deck queue](DECK-QUEUE.md)
tracks the rest of the original shortlist.

## Checkpoint so far

Feedback now covers 22 prepared Deck titles, with broad positive play plus
specific unresolved findings. The owner reports good controls/audio across many
sessions, but now also describes scene-dependent slowdown. All six supported
mapper IDs have ordinary-play signal; neither individual board variants nor
unreported hard scenes are automatically credited. See the dated baseline for
full reports and the [controls](CONTROLS.md) for reset/capture shortcuts.

## Results of the targeted pass

| Target | Current evidence |
|---|---|
| OCEAN logo | Owner sees tearing during wobble; private cold-boot audiovisual capture/checkpoint exists. Cause remains unconfirmed. |
| Marble Madness | Owner completed first race and some second race, generally good; timer glyph holes/blue outline questioned. Native close-up has blue edging and a small opening in 4. Intro text box was not separately accepted. |
| Crystalis | Owner sees a moving seam during vertical camera scrolling; status-bar displacement unclear. Keep separate from the earlier top-edge strip and original HUD/dialogue-line quirks. |
| Balloon Trip | Owner explicitly reports static HUD and no star issues; positive named-scene observation. |
| Mario 3 first power-up | Owner explicitly reports normal vertical emergence with no visual issue noticed; positive named-scene observation. |
| Zelda vertical transitions | Not specifically reported. |
| Battletoads level two | Not specifically reported. |
| Scheherazade battle damage | Not reached/reported; village gameplay looks good, NPC layering matched an unspecified video. User confirmed the title. |
| Little Nemo Mushroom Forest | Scrolling edges, a jump-time horizontal disturbance and flicker immediately above the status border are three separate open observations. |
| Mega Man 3 | Private stage-select capture includes the line's reported region above Shadow Man. Top Man vertical scrolling/falls show a reported seam. Crowding/damage slowdown and high jumps are confounded by confirmed duplicate P1/P2 host mapping; the relaunched corrected-port retest still showed crowd/damage slowdown near 59.97 output FPS. The owner regards high jumps as possibly a level mechanic. |

The owner later confirmed Gradius's positive result; this is confirmation of the
same title, not an additional distinct game result.

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

## Known original behavior and reference cards

Before recommending a scene, explain known original quirks as well as emulation
failure symptoms. Keep uncertainty visible; there is no complete authoritative
catalog of every game's slowdown or artifact. Start with
[NESdev's original-game bugs](https://www.nesdev.org/wiki/Game_bugs), use its
[tricky-game notes](https://www.nesdev.org/wiki/Tricky-to-emulate_games) for hardware
traps, and add exact-scene hardware/firsthand references as needed.

- **Sprites:** the NES evaluates at most eight sprites per scanline, and games
  can rotate their order to share visibility. One character can use several
  sprites. This explains why crowded-scene flicker can be authentic; it does
  not prove a particular flicker is accurate. Sprite omission and game-side
  computational slowdown are different effects. [PPU evaluation](https://www.nesdev.org/wiki/PPU_sprite_evaluation).
- **Blaster Master / Kirby:** firsthand [Blaster Master run notes](https://speeddemosarchive.com/BlasterMaster.html)
  and [Kirby route research](https://tasvideos.org/GameResources/NES/KirbysAdventure)
  discuss lag. That makes game-side slowdown plausible; the owner's exact
  scenes remain unattributed.
- **Mega Man 2 / DuckTales:** [Rockman 2 run notes](https://tasvideos.org/6534S)
  discuss underwater slowdown but use the Japanese revision; this is conditional,
  not proof for an unspecified USA scene. [DuckTales NES route notes](https://tasvideos.org/UserFiles/Game/26)
  discuss lag, but no verified first-red-plant-specific reference was found.
- **Little Nemo:** a [firsthand HUD-line discussion](https://forums.nesdev.org/viewtopic.php?start=705&t=6928)
  describes a similar expanding/contracting line and edge-cropping concern,
  without verification on a Nemo cartridge. Similar output across emulators
  does not establish hardware correctness. Ordinary cropping can hide outer
  edges while leaving an internal world/HUD disturbance visible.
- **Mega Man 3:** built-in player-two effects are documented in the
  [community controller-two reference](https://strategywiki.org/wiki/Mega_Man_3/Tips_and_Tricks).
  The host actually assigned the same controller index to both players. The
  owned launcher now leaves player two unassigned for this one-controller
  setup. A new launch was read back with P2 unassigned, and the owner still reported
  crowded/damage slowdown. The port correction does not classify that slowdown
  or the independent visual seams.

The owner's [Marble Madness video](https://www.youtube.com/watch?v=-_rkCXLEGXY&t=47s)
(at 0:47) and [Crystalis video](https://www.youtube.com/watch?v=Vx39m1u6d-E)
were opened and identified as NintendoComplete playthroughs. A paused Marble
frame shows outlined timer digits. The hardware/emulator, palette and exact
revision provenance were not established. No Scheherazade video URL was actually
included, although the owner reports matching NPC overlap in their comparison.

Additional scene evidence: Mega Man 3's Top Man descent/ladder/camera movement
and Shadow Man's opening red-waterfall fall both showed reported seams. The
Shadow Man waterfall effect while standing still was later qualified by the
owner as possibly a repeating-pattern illusion; retain it as uncertain. Large
bulldozer-enemy flicker is separate and may involve the per-scanline sprite
limit; no exact original-hardware comparison was established. The owner reports
music/sound good and input tight, while crowded/jumping enemies and getting hit
produce pronounced slowdown even after distinct-port read-back. Output FPS
stayed around 59.96–59.98 without correlation to the perceived slowdown.

The owner explicitly wants built-in passwords/Continue codes recorded as scene
access recipes, prompted by Metroid. A valid password can recreate game
progress across compatible game versions without reproducing an exact CPU/PPU
instant; a core snapshot preserves a more exact runtime point. Label both the
access method and revision. Metroid's early bat/shooting slowdown remains
unattributed. Bionic Commando plays great; Blaster Master level one looks good
with top-edge pixels/slowdown; DuckTales is good overall with first-red-plant
slowdown; Mega Man 2 looks good with some slowdown. Kirby also has an occasional
slowdown report. These remain scene-specific observations, not automatically
normal behavior just because lag is documented in the game generally.

Shatterhand's first-stage feedback: tight controls, satisfying/smooth gameplay,
good audio/animations, a possible intro-reticle sprite artifact, crowded-enemy
flicker and bottom-edge pixels during falls. The separately reported horizontal
line began around two-thirds height during left/right movement, then the owner
observed it slowly descending and wrapping from bottom to top. A private clip
captured the actual first-stage fence/platform location. This drifting/wrapping
motion strengthens a presentation-tearing hypothesis; it is not evidence of an
NES counter overflow. Shatterhand has documented raster/parallax use in
[Area B](https://forums.nesdev.org/viewtopic.php?t=14064), which does not classify
this first-stage Area A observation.

The base host config had regular VSync and Adaptive VSync both enabled. A
backed-up owned Nesturbator append override now sets regular VSync on and
Adaptive VSync off for the next launch. The active game was not interrupted;
actual new-launch read-back and visual comparison remain pending. No global
configuration or emulation code changed. Resolve this host factor before
interpreting drifting seams as confirmed core timing failures.

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
