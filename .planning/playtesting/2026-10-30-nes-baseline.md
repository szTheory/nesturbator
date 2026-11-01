# Private NES gameplay baseline — 2026-10-30

User-requested exploratory RetroArch sessions, one game at a time. Header admission is not gameplay acceptance. User observations and agent-observed launch facts are recorded separately. No full-game or library-wide compatibility claim.

Current checkpoint: feedback covers 22 of 26 prepared Deck titles, spanning
all six supported mapper IDs, with both positive play and open findings. The [scene playlist](SCENE-PLAYLIST.md)
prioritizes known hard cases, and the [full queue](DECK-QUEUE.md) tracks remaining
games. Later sections preserve the chronology; earlier setup counts are dated
observations, not the current collection size.

## Session setup

- RetroArch 1.22.2, macOS arm64, copied local development core from the current v5 source; not a downloaded release.
- Private session manifest retains exact module/content identities and per-launch logs; media stays in the user-provided external folder.
- Separate configuration, saves and states; ordinary play with rewind/run-ahead off initially. No changes to the normal RetroArch configuration.
- Keyboard: arrows, X = A, Z = B, Return = Start, Right Shift = Select, Esc = quit. Controller availability remains to be observed.

## Requested titles

| # | Title | Header mapper | Battery | Status / observation |
|---|---|---:|---|---|
| 01 | Super Mario Bros. 3 (USA) | 4 | no | Mac: plays pretty good; mild sluggishness/audio tearing, cause unverified. Deck Rev A: good input timing, visuals and audio, low perceived latency, no observed issues during the reported session. |
| 02 | Kirby's Adventure (USA) | 4 | yes | Deck: user reports plays great; no save/relaunch or named later scene result. |
| 03 | Mega Man 2 (USA) | 1 | no | Deck: looks good overall, with some slowdown; title explicitly corrected to Mega Man 2. |
| 04 | Castlevania III - Dracula's Curse (USA) | 5 | no | Unsupported mapper 5 |
| 05 | Contra (USA) | 2 | no | Deck: user reports everything plays well, with the same positive controls, visuals, audio and perceived latency as Mario 3; no observed issues. |
| 06 | Legend of Zelda, The (USA) | 1 | yes | Deck Rev A: user reports it plays great and prior progress appeared. Running core/shared battery-save path verified; no logged save-state load. |
| 07 | Super Mario Bros. (World) | 0 | no | User: plays pretty well; graphics/audio good; mild sluggishness under substantial concurrent load, cause unverified. |
| 08 | Metroid (USA) | 1 | no | Deck: going well early; some slowdown when bats descend while shooting, cause unclassified. |
| 09 | Ninja Gaiden (USA) | 1 | no | Deck: displayed as Shadow Warriors; user reports it plays great. |
| 10 | Punch-Out!! (USA) | 9 | no | Unsupported mapper 9 |
| 11 | Solar Jetman - Hunt for the Golden Warpship (USA) | 7 | no | Deck: user reports playing well. |
| 12 | Mega Man 3 (USA) | 4 | no | Deck: good audio/input; stage-select line, vertical-scroll seams and severe crowded/damage slowdown reported. Distinct-port retest retains slowdown near 59.97 FPS. |
| 13 | Castlevania (USA) | 2 | no | Not tried |
| 14 | DuckTales (USA) | 2 | no | Deck: good overall; slowdown at first red plant, cause unclassified. |
| 15 | Batman - The Video Game (USA) | 4 | no | Deck: user reports plays great. |
| 16 | Bionic Commando (USA) | 1 | no | Deck: plays great. |
| 17 | Blaster Master (USA) | 1 | no | Deck: level one looks good; top-edge graphics and some slowdown, cause unclassified. |
| 18 | Crystalis (USA) | 4 | yes | Deck: user reports looks, plays and sounds great; thin top-edge strip observed in private screen capture, overscan explanation plausible but exact rendering correctness unverified. |
| 19 | River City Ransom (USA) | 4 | no | Not tried |
| 20 | Little Nemo - The Dream Master (USA) | 4 | no | Deck: good input/audio/general graphics; scrolling-edge pixels, jump-time horizontal disturbance and HUD-border flicker reported separately. |
| 21 | Journey to Silius (USA) | 1 | no | Not tried |
| 22 | Shatterhand (USA) | 4 | no | Deck: controls/audio/smoothness good; intro/flicker/edge observations plus a horizontal line that drifts down and wraps to the top. Presentation comparison prepared, outcome pending. |
| 23 | Ufouria - The Saga (Europe) | 4 | no | PAL title; regional timing not implemented |
| 24 | Gimmick! (Japan) | 69 | no | Unsupported mapper 69 |
| 25 | Balloon Fight (USA) | 0 | no | Deck: user reports plays great and especially smooth; Balloon Trip not explicitly reported. |

## Header differences from the proposed list

Observed headers identify Mega Man 2 and Journey to Silius as mapper 1; Solar Jetman as mapper 7; DuckTales as mapper 2; Ufouria as mapper 4. The European Ufouria remains outside regional support despite its legacy header not reliably declaring PAL. Zelda is present with the catalog name “Legend of Zelda, The (USA)”.

## Live observations

First title launched: Super Mario Bros. (World), mapper 0. Agent observed World 1-1 in the actual RetroArch window; the private log confirms the selected module, cocoa/HID input drivers and CoreAudio output initialization. The first user feedback is recorded below; save behavior remains unassessed. No gameplay pass is inferred from rendering or audio initialization alone.

### Super Mario Bros. — first user report

The user reports that Mario plays pretty well, graphics look good and audio sounds good. It feels a little sluggish while the computer is running substantial concurrent work. Resource contention is a hypothesis, not an attributed cause or a measured core-performance result. Battery/save-state behavior was not assessed. No scheduling setting had been changed at the time of this report.

Scheduling follow-up: the module was built in Release mode with `-O3 -DNDEBUG`. By the time the process check ran, the Mario process had exited and the log recorded game/core unload. A later system-load snapshot does not establish load during the reported sluggishness. No priority, CPU-affinity or other application settings were changed. Apple recommends QoS-based scheduling for interactive work; priority is not a reserved core ([Apple scheduling guidance](https://developer.apple.com/videos/play/tech-talks/110147/)).

### Super Mario Bros. 3 — next session

Launched directly after the Mario report, with the same copied core and interactive configuration. Agent observed the World 1 map. User subsequently reported that it played pretty good, with similar mild sluggishness and a little audio tearing. Cause remains unverified; no priority change was applied.

## Smoke-test disposition and later devices

The owner considers the two Mario sessions sufficient basic smoke signal for now and deferred the remaining game list and higher-confidence pacing/performance qualification to a quieter or dedicated machine. Neither game is claimed perfect, fully completed or performance-qualified. The remaining titles are untested, not failures. A second Mac or the owner's Steam Deck with EmulationStation is a possible later test device. The owner suggested SSH as a convenient Deck setup route; device/host/core compatibility and actual gameplay are not yet established. No recurring manual verification requirement is introduced.

## Steam Deck continuation

The owner then selected the Steam Deck as a practical daily-play feedback route
through ES-DE and RetroArch, potentially before deeper Playstead integration.
The updated device runs SteamOS x86-64, RetroArch 1.22.2 and ES-DE 3.5.0. The
installed core is a digest-checked Linux x64 CI development artifact from the v5
runtime code, not a newly published release. Actual module/content/connection
identities, raw logs, captures and backup locations remain private.

### Existing library and progress

The owner corrected the initial disposable-test-folder approach: reuse the real
library, avoid duplicate ROMs, keep progress for daily play and use a small
reference-only view to find the selected games. The two temporary transferred
copies were removed after byte verification; original media and all existing
main-library files were preserved. No separate test game system was registered.

The **Nesturbator** custom collection references these existing NTSC entries:

- Castlevania (USA, Rev A)
- Contra (USA)
- Crystalis (USA)
- The Legend of Zelda (USA, Rev A)
- Ninja Gaiden (USA), displayed in ES-DE as **Shadow Warriors**
- River City Ransom (USA)
- Super Mario Bros. 3 (USA, Rev A)

Only those entries select Nesturbator; existing alternative emulators and
unrelated metadata were preserved. This uses the Deck's existing revisions,
rather than publishing duplicate copies to force the Mac's exact revisions.
PAL/unsupported-board entries retain their previous choices. Normal RetroArch
battery storage remains in use; Nesturbator states use the regular state tree's
core-specific folder to preserve other cores' incompatible snapshots. Save/
reload and recovery have not been qualified by this session.

### Observed host issues and repairs

- **TV visibility:** Plasma was running in an extended layout with Steam on the
  built-in display and wallpaper on the TV. After backing up the layout, a
  TV-only docked layout and showing the desktop restored visibility. The owner
  confirmed the display result. No broad desktop reset was used.
- **ES-DE update/launch:** The updated AppImage answered its version command,
  while Steam's failed launch recorded a shell error in the EmuDeck wrapper.
  That wrapper also regenerated launchers and updated/reset its backend while
  launching. A backed-up stable entry retains configured cloud-sync hooks and
  runs the installed AppImage without that updater path. The historical shell
  failure's exact causation was not fully reproduced. The actual existing Steam
  shortcut subsequently started ES-DE in Gaming Mode; its log loaded the custom
  collection, retained the prior game count and reached normal startup.
- **Commercial core/input:** The owner saw commercial Mario 3 running on the TV.
  RetroArch detected the external PowerA controller without a matching profile
  and initially assigned player one to Deck controls. The first added profile
  incorrectly used raw SDL indices; correcting it to RetroArch's normalized
  SDL2 layout, with external-device preference and handheld fallback, led to
  the owner's explicit report: **Start worked**. This is real physical-input
  evidence, not full button coverage or a general compatibility verdict.

The Deck is now in Gaming Mode with ES-DE open through its repaired Steam
shortcut. The owner launched Mario 3 from the collection; its log confirmed the
Nesturbator command and module, but the controller's Start did not work in that
context. Steam's virtual-device presentation differs from the earlier direct
desktop run, so the physical-name preference did not establish player-one
mapping there. A launch-context device-index resolver was added with handheld
fallback. The owner then relaunched Mario 3 from the collection and explicitly
reported: **controller works now**. A read-only live check found Nesturbator
running with the per-launch input configuration selecting the external device
as player one. This closes the reported collection-launch input failure for
this session; full button coverage, reconnect behavior and handheld fallback
remain untested.
Clean return and progress roundtrip remain unqualified. No current game is
claimed perfect, and no core behavior, ABI, state format or emulator
implementation changed here.

The owner also raised state-format stability as daily use begins. Record an
explicit future upgrade/migration/rollback policy before claiming compatibility:
the existing format version refuses incompatible layouts, while recorded
behavior revision alone is not enforced. See the daily-use compatibility note
in the linked lessons; no converter or package-1.0 guarantee was introduced.

Reusable mechanisms and the owner's intended workflow are indexed in
[LESSONS.md](LESSONS.md). Other emulator projects may use that sanitized record;
device/account identifiers, personal paths, addresses and private artifacts
must not be committed.

### Mario 3 — Steam Deck gameplay feedback

After relaunching Super Mario Bros. 3 (USA, Rev A) from the Nesturbator
collection, the owner played and reported good input timing, visuals and audio,
low lag/latency, and no observed issues. This is a positive user-reported
ordinary-play smoke result through Steam Gaming Mode → ES-DE → RetroArch →
Nesturbator. Latency was perceived, not instrumented; session duration and game
completion were not reported. Save/load, reconnect and handheld fallback remain
untested. The earlier Mac sluggishness/audio tearing remains unattributed; this
Deck report does not establish its cause.

The next suggested title was Contra (USA), covering mapper 2 (UxROM). Its
subsequent feedback is recorded below. No device configuration was changed
for these feedback updates.

### Contra — Steam Deck gameplay feedback

Following the Contra recommendation, the owner reported that everything plays
well with feedback similar to Mario 3: good input timing, visuals and audio,
low perceived latency and no observed issues. Record this as a positive
user-reported Contra (USA) smoke result on the existing Deck collection route.
No independent launch/log check was performed for this report. Duration,
full-game coverage, save/load and measured latency remain unqualified.

Ninja Gaiden (USA) was recommended next to add mapper 1 (MMC1). Its subsequent
feedback is recorded below.

### Collection display-name clarification

The owner could not find the recommended Ninja Gaiden entry. A read-only check
found the existing Ninja Gaiden (USA) file present in the seven-entry collection,
with Nesturbator selected and no hidden flag. Its existing scraped display name
is **Shadow Warriors**. No restart, ROM duplication or metadata change is needed;
select Shadow Warriors to play the intended entry. This is an identity/display
check, not a gameplay result.

### Ninja Gaiden / Shadow Warriors — Steam Deck gameplay feedback

After the display-name clarification, the owner reported: **plays great**.
Record a positive user-reported gameplay smoke result for Ninja Gaiden (USA),
displayed as Shadow Warriors in the collection. No separate detailed audio,
latency, duration, full-game or persistence assessment was supplied for this
report. Together with Mario 3 and Contra, this provides ordinary-play smoke
signal across MMC1, MMC3 and UxROM; it does not qualify every game on those boards.

If the owner continues, The Legend of Zelda (USA, Rev A), already in the
collection, is a useful next voluntary choice with battery-backed progress.
Its gameplay and save/reload behavior remain untested; no configuration change
or automatic game launch was performed for this report.

### The Legend of Zelda — Steam Deck gameplay and prior progress

The owner reports Zelda plays great and picked up prior progress, initially
calling it a previous save state. A read-only live check confirmed Nesturbator
running the existing USA Rev A entry. RetroArch's log identifies the normal
shared `.srm` battery-save path and a separate Nesturbator snapshot path, with
no save-state load reported in that log. This is consistent with reused
in-game battery-backed progress, not evidence of cross-core snapshot support.
The owner accepted that distinction. No save bytes were read, altered or added
to the repository for this check.

Record positive user-reported gameplay and observed prior-progress reuse.
The prior writer/core was not established; a newly written battery-save exit/
relaunch cycle, save-state compatibility and full-game coverage remain
unverified. This report does not upgrade the earlier save-state guarantees.

### Crystalis — Steam Deck gameplay and top-edge observation

The owner reports Crystalis looks, plays and sounds great, and asks about a bar
at the very top of the picture. A read-only process check confirmed the intended
Nesturbator module and Crystalis (USA). At the owner's request a current
Gamescope screenshot was captured into ignored private storage and inspected:
a thin white-and-blue patterned strip is visible at the upper edge of the game
picture, above the playable scene. The capture is not committed.

The adapter currently submits the entire 256×240 frame and has no crop core
option or GET_OVERSCAN handling; see [adapter](../../libretro/libretro.c).
The running host's base configuration already has `video_crop_overscan = true`,
with no corresponding launch override. The legacy hint does not crop our
adapter's output. Its API declaration is deprecated in favor of core options;
see the owned vendored [API header](../../libretro/libretro.h).

[NESdev's overscan reference](https://www.nesdev.org/wiki/Overscan) explains the
full NES picture and edge visibility on televisions. The visible strip is
consistent with exposed overscan/edge graphics, but neither this screenshot nor
positive play feedback proves its exact rendering matches hardware. Preserve
it as an unclassified edge observation, not a confirmed PPU defect or a proven
normal Crystalis artifact. No hardware/reference comparison was performed.
Optional overscan cropping is a proposed presentation improvement, not approved
implementation scope; it must not be used to dismiss a real rendering defect.
No game, TV, host display setting or core code was changed. Save/load and full-
game coverage remain unverified.

## Owner-authorized expanded queue preparation

The owner then requested broader mapper/edge-case smoke testing, authorized SSH
transfer of needed local games and asked for preparation while away. This
supersedes the earlier deferral of the rest of the shortlist for preparation;
only reported gameplay receives credit. The [Deck queue](DECK-QUEUE.md) gives
the recommended order, actual visible names, suggested scenes and boundaries.

The collection now references 23 supported-region normal-library entries:
21 from the original list plus Gradius (CNROM coverage) and Battletoads (another
timing-sensitive AxROM title). Seven existing NTSC entries were reused and 16
missing NTSC variants were transferred with archive-hash verification. Existing
PAL variants were preserved; added USA/World entries distinguish their regions.
There is one selected variant per title in this collection and no separate test
ROM directory. Original library files, original metadata and original save
storage passed preservation checks against the pre-transfer snapshot. Existing
selected-game progress was backed up privately before further testing.

All selected images passed admission and 120 no-input frames in the current
local native runner. This is startup preflight, not a gameplay or performance
pass. On the Deck, every collection reference resolves and selects Nesturbator.
The normal Steam shortcut then started ES-DE, whose fresh log loaded the
expanded collection and completed startup. A private screen capture confirmed
the frontend at its normal system carousel. No game was running or interrupted,
and no new game was automatically launched. Display, controller and core
settings were preserved. The owner can return to Collections → Nesturbator and
start **Gradius (USA)**; no frontend restart is currently needed.

The five reported Deck sessions remain the credited gameplay observations.
The remaining 18 titles are prepared and untested on the Deck. MMC1, UxROM and
MMC3 have directional play signal; CNROM, AxROM and NROM remain Deck gameplay
gaps until the suggested next sessions. Unsupported requested boards/regions
remain outside this queue and were not routed to Nesturbator.

## Broad play checkpoint and scene-focused follow-up

The owner subsequently reported Gradius plays really well with no issues,
Solar Jetman is playing well, Balloon Fight plays great and feels especially
smooth, Kirby's Adventure plays great, and Battletoads plays great with no
observed issues. The owner describes the overall sessions as smooth with no
perceived performance problems and explicitly treats this as a useful baseline
checkpoint alongside CI. These are subjective gameplay observations, not
instrumented performance measurements. No independent launch check was performed
for each of these reports, and no specific later scenes or full-game coverage
are inferred. Existing identities/configuration checks remain separate evidence.

The owner then reported Batman plays great while research continued and stated
they had returned to ES-DE. This makes eleven positive reported Deck games,
spanning mapper IDs 0, 1, 2, 3, 4 and 7. It does not certify every board variant
or every scene. Prior Crystalis edge and save/persistence limits remain visible.

The owner explicitly requested specific known failure cases before each future
play recommendation, practical shortcuts to deeper scenes, and reusable
cross-emulator lessons. The [targeted scene playlist](SCENE-PLAYLIST.md) records
sourced scene targets, symptoms, access costs, shortcut limitations and exact
versus generic coverage. Empty cheat callbacks and unavailable system RAM mean
Game Genie/frontend RAM cheats are not currently usable scene-access methods.
Built-in game passwords and same-core checkpoints are the supported direction;
no scene snapshot or automatic warp service was created here.

Jurassic Park (USA), Marble Madness (USA) and The Magic of Scheherazade (USA)
were added from the authorized local library after admission/frame-120 checks.
Archive hashes, all collection references and core choices were verified on the
Deck. Existing library files, metadata and save storage passed preservation
checks. An active-game guard deferred the first refresh attempt; after the
owner returned to ES-DE, a normal close request delivered directly to the
identified frontend window exited it. No game was interrupted and no force
termination was used. The normal Steam shortcut reopened ES-DE and its fresh
log loaded the 26-entry collection. Current selected progress was backed up
privately. Fifteen games remain untested on the Deck, and named scene coverage
is tracked separately even for the eleven positive title reports.

## Detailed findings, private automation and controller-port recheck

Bionic Commando plays great. Jurassic Park's wobbling OCEAN logo shows reported
tearing. Marble Madness's first race and part of the second look/play good;
the owner questions filled timer glyphs/missing blue outline against a supplied
video. A native-size adapter RGB close-up does contain dark-blue edging and a
small opening in 4, so do not diagnose a missing bitplane from the impression
alone. Crystalis has a reported moving seam during north/south camera movement,
with status-bar displacement unclear. This is distinct from its earlier top
strip and documented original HUD/dialogue-line quirks.

The owner confirmed The Magic of Scheherazade as the game with NPC layering
that matched a comparison video. General village play looks good; exact hardware
correctness and the menu-based battle-damage probe remain unqualified. The
owner explicitly reports Balloon Trip's HUD staying fixed and no star issues,
and Mario 3's first power-up emerging normally without a noticed visual issue.
Gradius's good-play result was reiterated, not counted as a new distinct title.

The supplied Marble Madness comparison is the NintendoComplete video at 0:47:
[Marble Madness reference](https://www.youtube.com/watch?v=-_rkCXLEGXY&t=47s).
The supplied [Crystalis reference](https://www.youtube.com/watch?v=Vx39m1u6d-E)
is also from NintendoComplete. Both pages were
opened and identified; no exact hardware/emulator, palette or revision provenance
was established. A paused Marble reference frame shows outlined timer digits.
No actual Scheherazade URL was included. These references do not certify real-
console behavior by themselves; raw page captures remain private.

Blaster Master (owner called it Master Blaster) looks good in level one, with
top-edge pixels and some slowdown. Kirby also has occasional slowdown. DuckTales
is good overall with slowdown near the first red plant. Mega Man 2 looks good
with some slowdown, with the title explicitly corrected by the owner. Metroid
is going well early with slowdown when bats descend while shooting. General
lag references exist, but none establishes the exact reported event as correct.
Built-in passwords/Continue codes are explicitly requested as efficient access
recipes; a game password and an exact emulator snapshot preserve different
amounts of runtime state.

Little Nemo has positive controls/audio/general graphics plus three open visual
observations: scrolling-edge updates, a horizontal disturbance during jumps and
a flickering line above the Mushroom Forest status border. A private live clip
captured the actual title. A firsthand discussion describes a similar HUD line
without verification on a Nemo cartridge; keep the cause unclassified. Optional
cropping is requested presentation work, not implemented here and not a remedy
for an unexplained internal split.

Mega Man 3 has a reported flickering line above Shadow Man in the selection
screen, and moving seams during Top Man ladders/large falls and Shadow Man's
opening red-waterfall descent. The standing-still waterfall appearance was later
qualified by the owner as possibly a repeating-pattern illusion. Preserve that
correction. Large bulldozer-enemy flicker is separate. The owner reports good
music/sound and tight input, but pronounced slowdown with roughly four/five
characters, multiple jumping enemies and damage. Sprite omission/flicker and
game-side computational lag are different effects; exact hardware correctness
was not established by a general NES sprite-limit explanation.

A live configuration check found a concrete host error: player one selected the
Xbox index while inherited player two selected the same index. Built-in MM3
player-two effects made this a valuable confounder to remove. The owned launcher
was backed up and corrected to leave P2 unassigned for single-controller play;
syntax passed. A new MM3 launch was read back with P1 index1 and unused P2
index15, and its latest state files were privately backed up before recommending
reset. The owner still reports severe crowded/damage slowdown after the fix,
considers higher jumps possibly a level mechanic, and still sees visual seams.
Do not credit the port fix as resolving those findings or declare the jumps a
cheat-induced bug. Two-player mappings remain a separate setup task.

The owner enabled the FPS display and reports roughly 59.96–59.98 with no
correlation between its small fluctuations and gameplay slowdown. The current
adapter advertises 60.0988138974 Hz; the host uses 60 Hz configuration with VSync
and audio sync. Stable output with slower action is consistent with game-side
lag, not proof of original-hardware correctness. No synchronization setting
was changed. Further casual diagnostics need only FPS/audio/scene context;
frame-time benchmarks and sampled display clips are different evidence.

A live Mario 3 log explicitly recorded successful automatic loading of a
Nesturbator `.state.auto`. This is actual same-core load evidence, distinct from
Zelda's earlier shared battery reuse. A controlled manual host save/exit/relaunch
cycle and cross-core/version guarantees remain separate. See the
[controller cheat sheet](CONTROLS.md).

The owner then authorized more private automation. A small stdlib libretro
frontend now runs bounded input recipes, checkpoints and native-size captures,
PCM WAV and MP4 exports. Jurassic Park, Marble Madness, Little Nemo and MM3 menu
recipes/checkpoint replays match per-frame RGB and PCM exactly. Nemo reaches
Mushroom Forest and NPC dialogue, not yet a qualified camera-seam reproduction;
MM3's checkpoint is inside its menu with Shadow Man visible, cursor center.
A convenience replay command and identity checks shorten repeat runs. No daily
saves are accessed, no native core behavior changed, and raw checkpoints are
not qualified RetroArch containers. The owner was not required to play those
routes. Private live Deck screenshot/clip capture also succeeded without input
or state commands; an attempt stopped correctly when no game was active.

The [automation/CI plan](AUTOMATION.md) maps the findings to existing owned
PPU/MMC3/fetch/render/conversion tests, proposes small independent-oracle
synthetic guards, and preserves unique regression protection while allowing
redundant exploration to retire. No commercial bytes/state/capture entered Git,
no collector campaign was invoked and no native fix or new CI fixture was
implemented. Scene outcomes, original-behavior references and uncertainties
remain in the [scene playlist](SCENE-PLAYLIST.md).

### Shatterhand drifting-line presentation probe

The owner reports Shatterhand's controls tight, gameplay satisfying and smooth,
animations/general graphics good and audio great. A possible reticle-zoom sprite
artifact, crowded-enemy flicker and bottom-edge pixels during falls are kept
separate from the horizontal seam. Repeated left/right movement at the beginning
of stage one showed a line initially around two-thirds height; further observation
found it slowly travelling down and restarting at the top. A private sampled
clip captured the actual Shatterhand fence/platform scene. This motion supports
a host-presentation hypothesis, not a confirmed core fault or counter overflow.

A read-only compositor convar query returned no value, so no compositor tearing
state is claimed. The actual base RetroArch config did show normal VSync and
Adaptive VSync both enabled. A backed-up owned append-profile override now keeps
normal VSync enabled and disables adaptive VSync on the next launch. No current
game, global setting or emulation code was changed. Activation/read-back and
visual comparison await a fresh Shatterhand launch. No improvement is claimed
until that result. See the scene/automation notes for references and next step.

### Presentation override activation and remaining symptoms

The owner quit/relaunched Shatterhand and also reset, losing their immediate
position. Active launch read-back confirmed regular VSync on, adaptive VSync
off and the distinct P1/unassigned-P2 routing. The owner still sees some falling
and left/right disturbance plus busy-scene slowdown, especially with helpers/
fireballs, and reports that music briefly stopped after a hit then resumed.
The exact earlier fence-location drifting-line comparison is inconclusive;
no improvement or cause is claimed. A private after-relaunch display clip and
separate three-second stereo48k output-monitor audio recording were captured.
The audio source was game/system output, not microphone input; it was not
synchronized to the video and does not prove the music interruption recurred.

The host log recorded a core reset and a failed load of numbered slot 0.
Filesystem checks found that numbered file absent, while a separate autosave
existed and was privately backed up. The exact auto-state scene was not
independently inspected. This load error and the unmeasured death/restart delay
are separate observations. For presentation-only comparisons, relaunch is
sufficient; avoid needless resets and preserve/identify the saved point first.

Three bounded native Shatterhand navigation attempts stayed on intro/title;
no native first-stage seam or gameplay checkpoint was reached. The same input
recipe's final native-runner frame matched the private adapter host, while an
owned callback control delivered Start. Preserve the navigation limitation;
neither a helper failure nor a guest/core fault was diagnosed. MM3 portrait/menu
animation replay remains useful for frame comparison, but interactive cursor
navigation is also unqualified. This is bounded automation, not an autonomous
whole-game compatibility result.
