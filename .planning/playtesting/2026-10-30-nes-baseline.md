# Private NES gameplay baseline — 2026-10-30

User-requested exploratory RetroArch sessions, one game at a time. Header admission is not gameplay acceptance. User observations and agent-observed launch facts are recorded separately. No full-game or library-wide compatibility claim.

## Session setup

- RetroArch 1.22.2, macOS arm64, copied local development core from the current v5 source; not a downloaded release.
- Private session manifest retains exact module/content identities and per-launch logs; media stays in the user-provided external folder.
- Separate configuration, saves and states; ordinary play with rewind/run-ahead off initially. No changes to the normal RetroArch configuration.
- Keyboard: arrows, X = A, Z = B, Return = Start, Right Shift = Select, Esc = quit. Controller availability remains to be observed.

## Requested titles

| # | Title | Header mapper | Battery | Status / observation |
|---|---|---:|---|---|
| 01 | Super Mario Bros. 3 (USA) | 4 | no | User: plays pretty good; mild sluggishness and some audio tearing, cause unverified. |
| 02 | Kirby's Adventure (USA) | 4 | yes | Not tried |
| 03 | Mega Man 2 (USA) | 1 | no | Not tried |
| 04 | Castlevania III - Dracula's Curse (USA) | 5 | no | Unsupported mapper 5 |
| 05 | Contra (USA) | 2 | no | Not tried |
| 06 | Legend of Zelda, The (USA) | 1 | yes | Not tried |
| 07 | Super Mario Bros. (World) | 0 | no | User: plays pretty well; graphics/audio good; mild sluggishness under substantial concurrent load, cause unverified. |
| 08 | Metroid (USA) | 1 | no | Not tried |
| 09 | Ninja Gaiden (USA) | 1 | no | Not tried |
| 10 | Punch-Out!! (USA) | 9 | no | Unsupported mapper 9 |
| 11 | Solar Jetman - Hunt for the Golden Warpship (USA) | 7 | no | Not tried |
| 12 | Mega Man 3 (USA) | 4 | no | Not tried |
| 13 | Castlevania (USA) | 2 | no | Not tried |
| 14 | DuckTales (USA) | 2 | no | Not tried |
| 15 | Batman - The Video Game (USA) | 4 | no | Not tried |
| 16 | Bionic Commando (USA) | 1 | no | Not tried |
| 17 | Blaster Master (USA) | 1 | no | Not tried |
| 18 | Crystalis (USA) | 4 | yes | Not tried |
| 19 | River City Ransom (USA) | 4 | no | Not tried |
| 20 | Little Nemo - The Dream Master (USA) | 4 | no | Not tried |
| 21 | Journey to Silius (USA) | 1 | no | Not tried |
| 22 | Shatterhand (USA) | 4 | no | Not tried |
| 23 | Ufouria - The Saga (Europe) | 4 | no | PAL title; regional timing not implemented |
| 24 | Gimmick! (Japan) | 69 | no | Unsupported mapper 69 |
| 25 | Balloon Fight (USA) | 0 | no | Not tried |

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
- Ninja Gaiden (USA)
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
fallback; physical confirmation through the collection remains pending.
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
