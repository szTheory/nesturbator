# Emulator playtesting lessons

Updated: 2026-10-30. Start here when another emulator project reaches a
milestone and wants a practical player-feedback loop. These are workflow and
host-integration lessons; hardware behavior still needs that core's own sources
and tests. The [dated baseline](2026-10-30-nes-baseline.md) records observations
and their limits.

## Owner direction

Prioritize ordinary daily play, usable controls and persistent progress. A
Steam Deck with **ES-DE (EmulationStation) → RetroArch → the owned libretro
core** can provide useful feedback before deeper Playstead integration. This
complements automated conformance and deterministic tests; a casual successful
session is finite smoke evidence, not universal compatibility or performance
qualification. No recurring owner sign-off is required.

Use the real library. A named collection should reference existing game entries,
with per-game emulator choices that remain switchable. Keep their metadata,
normal battery-save storage and other emulator choices. Avoid duplicate game
files, disposable progress or a second test-only library for normal play.
Core-specific snapshot folders within normal save storage are appropriate:
states encode implementation-specific machine state. Battery RAM can often be
portable, but confirm content, board/layout and byte size before sharing it.

Keep endpoint addresses, account/device identifiers, usernames, personal paths,
raw logs, screenshots, media, saves and states out of Git. Local/private records
may hold the operational details. Share only sanitized mechanisms, outcomes,
public artifact provenance and reusable decisions.

The owner later authorized expanding the queue and transferring needed games.
See the [prepared Deck queue](DECK-QUEUE.md) for the current 23-entry collection,
coverage priorities and visible title aliases. New NTSC variants live in the
normal library alongside preserved PAL variants, with explicit regional names;
the collection selects one supported variant per title. Earlier seven-entry
observations below describe the initial setup, not the current collection size.

## Observed results and limits

| Observation | What it establishes | What remains unknown |
|---|---|---|
| macOS Super Mario Bros.: user reports good play, graphics and audio, with mild sluggishness | Useful ordinary-play smoke signal on the identified development core | Cause of sluggishness; dedicated-machine pacing; full game and persistence |
| macOS Super Mario Bros. 3: user reports good play, mild sluggishness and some audio tearing during concurrent work | A second title/board path supplies useful smoke feedback | Resource contention is suspected, not attributed; sustained performance remains unqualified |
| Steam Deck commercial Mario 3 displayed on the docked TV; after a direct-launch success and collection-launch input failure, the owner relaunched from the collection and confirmed the controller works | The actual Steam/ES-DE → Nesturbator launch now has user-confirmed physical input; a live check found the intended core and corrected player-one configuration | Full button coverage, reconnect/handheld fallback, sustained play and persistence remain separate |
| Steam Deck Mario 3: after the collection input repair, the owner reports good input timing, visuals and audio, low perceived latency and no observed issues | Positive ordinary-play smoke signal through the intended Steam/ES-DE/RetroArch route | No instrumented latency, reported duration, full-game coverage or persistence result; earlier Mac symptoms remain unattributed |
| Steam Deck Contra: owner reports everything plays well with the same positive feedback as Mario 3 | Positive user-reported UxROM ordinary-play smoke signal alongside Mario 3's MMC3 result | No independent launch check for this report; duration, full-game coverage, measured latency and persistence remain unqualified |
| Steam Deck Ninja Gaiden (shown as Shadow Warriors): owner reports it plays great | Positive user-reported MMC1 gameplay smoke signal alongside the MMC3 and UxROM sessions | No separate detailed audio/latency assessment, duration, full-game or persistence result |
| Steam Deck Zelda: owner reports it plays great and prior progress appeared; live check confirmed Nesturbator, shared `.srm` storage and no logged snapshot load | Positive MMC1 gameplay signal and prior-progress reuse consistent with battery-save sharing | Prior writer/core and a newly written save/relaunch cycle are unverified; this does not demonstrate cross-core save-state compatibility |
| Steam Deck Crystalis: owner reports looks, plays and sounds great; private capture shows a thin patterned upper-edge strip | Positive MMC3 gameplay/audio smoke signal and a concrete presentation observation | Exposed overscan is plausible, but exact hardware correctness is unverified; no crop option currently exists in the adapter |
| Reference-only Nesturbator collection registered against seven existing NTSC games; repaired Steam shortcut started ES-DE in Gaming Mode and its log loaded that collection | Installation/configuration/preservation checks passed; actual frontend startup reached its loaded-library state | All seven games and the complete ES-DE → core → exit/progress path remain separate acceptance |
| Owner-authorized expansion to 23 references: seven reused NTSC entries plus 16 verified transfers; ES-DE freshly loaded the collection | Setup now covers candidate titles for all six supported mapper families; original files, metadata and save storage preserved, selected progress backed up | Only five Deck gameplay reports are credited; the new startup preflight does not establish interactive play or every board profile |

The owner explicitly considers the two Mac sessions sufficient smoke signal for
now. The rest of the original shortlist remains untested or outside current
board/region support. Better pacing evidence can come later on a quieter device;
it is not a prerequisite for retaining useful feedback now.

## Transferable mechanisms

| Mechanism | Lesson and practical check |
|---|---|
| Separate launcher, host and core | ES-DE manages the library and launch command; RetroArch supplies device/display/audio handling; the core supplies emulation. A successful core launch does not prove the ES-DE/Steam wrapper works, and a host controller failure need not be an emulation bug. |
| Use the right artifact | Use the target OS/architecture module, not a Mac module copied to Linux. The Deck setup used a digest-checked Linux x64 CI artifact from the v5 runtime code. CI/development artifacts and published releases remain distinct. Check actual loaded module identity. |
| Read device discovery before changing code | RetroArch detected the external PowerA controller but lacked a matching SDL2 profile; the Deck controls held player one. A device-specific profile plus a preferred external device with handheld fallback addressed these host configuration seams. |
| Match the driver's button numbering | The first profile incorrectly used raw SDL hardware indices. RetroArch's SDL2 profile expected normalized GameController indices. Correcting to its installed standard Xbox layout made Start work. A correct raw-device mapping is not automatically the correct frontend binding. |
| Resolve the device in the actual launch context | A hardcoded index or physical-name preference can break after reconnecting or switching between Desktop and Gaming Mode. Steam presented virtual gamepads in the real collection launch, so the earlier physical-name preference did not carry across. Resolve the attached external device in the launch context, retain handheld fallback, then verify the actual frontend path with a real input action. The owner confirmed the controller works after relaunching Mario 3 from the collection; fallback and reconnect behavior remain untested. |
| Distinguish extended displays from a failed desktop | Plasma was running with two screens: Steam on the built-in panel and wallpaper on the TV. A screenshot and live output layout explained the apparent blank desktop. A backed-up TV-only docked layout plus showing the desktop resolved visibility; the owner confirmed the TV result. Avoid broad desktop resets. |
| Keep ordinary launch stable | After an ES-DE update, Steam recorded a shell error in the EmuDeck wrapper while the new AppImage's version command passed. The wrapper also regenerated launchers and updated/reset its backend during launch. The repair uses the installed AppImage through a stable wrapper, retaining configured cloud-sync hooks and moving updater work off the play path. Exact causation of the historical shell error is not fully reproduced. |
| Preserve defaults when adding a core | Read the installed ES-DE version's NES configuration, retain its existing command order/options, and append the owned core as an alternative. Apply the new choice to selected supported entries, not every game in a curated system. Regional filenames can contradict legacy header flags; do not silently route PAL titles into an NTSC-only core. |
| Verify crop support at the adapter boundary | A host overscan preference may have no effect if the core always submits the full frame and does not consume that preference. Nesturbator currently sends 256×240 with no crop option; optional presentation cropping is future work. Keep an unexplained edge artifact recorded until reference evidence distinguishes expected edge graphics from an emulation error. |
| Treat region as part of content identity | An existing PAL copy is not a valid NTSC test substitute merely because a legacy header reports zero. With explicit transfer authority, add the needed regional variant to the normal library, label it clearly, preserve the original and select one variant per title in the test collection. Do not silently replace content under an existing filename or assume progress transfers between regional revisions. |
| Collections are references | ES-DE custom collections list existing ROM paths. A small collection is a convenient view over the same entries and progress, not a duplicated ROM directory. Back up metadata and change only the intended per-game emulator field. |
| Test progress separately | Normal save directories and a valid state API do not prove graceful host shutdown, automatic snapshot creation or safe recovery. Verify actual exit/save/reload results before making those claims. Never treat a forced process stop as proof that progress flushed. |

## Feedback loop for the next core

1. Re-read current local instructions and relevant known host lessons. Transfer
   a mechanism as a hypothesis, not another console's implementation or timings.
2. Identify supported existing games and the exact target module/host. Keep
   source media intact and make small reversible configuration changes.
3. Start one commercial title the owner wants to play through the real intended
   path. Use existing library entries and normal save policy. Let the owner
   report a few words about controls, picture, sound, pacing and the first
   problem; the agent collects technical identity privately.
4. Diagnose the first broken seam. Bind a controller profile to the actual
   driver/device, check the active display and inspect the specific failed
   launch. Retain a failed attempt rather than inferring success from a retry.
5. Record observed behavior, context, uncertainty and untested items separately.
   Turn reproducible emulator defects into small owned regressions. Continue
   daily feedback without turning a smoke session into a qualification campaign.
6. Promote integration/performance work only when a concrete defect, consumer
   need or measured budget calls for it. Playstead remains valuable for its own
   library/progress UX, with acceptance through its actual production path.

No dependency, shared host framework, broad platform matrix, mandatory device
setup, new runtime window or safety waiver follows from adopting this loop.
Existing stopped campaigns retain their original boundaries.

## Save-state compatibility as daily use begins

The owner's daily-play goal makes upgrade safety relevant before package 1.0.
The current [public contract](../../include/nesturbator.h) already versions the
state format separately from package/API versions: format 3 refuses formats 1
and 2 before mutation, with no converter. Behavior revision is recorded but not
enforced on native load; a same-format older snapshot can load yet diverge after
an emulation fix. A stable C ABI alone does not promise snapshot compatibility.

Before future core updates, explicitly decide the supported snapshot versions
and any migration policy, preserve original states and a matching old core for
rollback, and test accepted/refused histories with owned fixtures. Avoid silently
overwriting an incompatible automatic snapshot during an upgrade. Battery-save
compatibility is a separate contract. This records the design concern and
acceptance direction; no backward-compatibility or converter promise is made.

## Primary reference trail

- [Dated NES baseline](2026-10-30-nes-baseline.md): owner reports and local
  observation summaries; raw identities and captures remain private.
- [Cross-project discovery](../research/milestone-v6-discovery/SYNTHESIS.md):
  Muhtari, other active cores and the current Playstead boundary.
- [ES-DE custom systems](https://gitlab.com/es-de/emulationstation-de/-/blob/master/INSTALL.md#es_systemsxml)
  and [collections/alternative emulators](https://gitlab.com/es-de/emulationstation-de/-/blob/master/USERGUIDE.md):
  extension points and reference-based collections.
- [RetroArch controller autoconfiguration](https://docs.libretro.com/guides/controller-autoconfiguration/)
  and [input controls](https://docs.libretro.com/guides/input-and-controls/):
  device profiles and player/device assignment.
- [Valve Desktop Mode guidance](https://help.steampowered.com/en/faqs/view/671A-4453-E8D2-323C):
  the distinction between desktop setup and the ordinary Steam experience.

These sources support host mechanisms, not NES timing. Recheck installation
paths, current versions and support before reusing an operational recipe.
