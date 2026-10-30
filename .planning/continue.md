# Resume after NES smoke tests and Steam Deck setup

Read AGENTS.md, STATE.md, PRODUCT-ROADMAP.md and
[playtesting lessons](playtesting/LESSONS.md). The lessons are the PII-free
cross-project entry point the owner requested. Use the
[dated baseline](playtesting/2026-10-30-nes-baseline.md) for scoped observations.

## Current user-facing outcome

The Deck is in normal Gaming Mode. ES-DE 3.5.0 is open through the repaired
existing Steam shortcut; its fresh startup log loaded the expanded Nesturbator
collection. The owner authorized broader preparation and transfers; that
reference-only collection now contains 26 supported-region entries with per-game
Nesturbator selection and switchable other cores. Seven existing NTSC entries
were reused and 16 missing NTSC variants added initially, followed by three new
scene targets in the normal library alongside
preserved PAL versions, with explicit region names. Original files, metadata,
save storage and unrelated emulator choices were preserved. Normal battery
storage remains; Nesturbator states are namespaced
inside normal state storage so other cores' snapshots survive.

The user saw commercial Mario 3 running on the TV and confirmed Start after
correcting the PowerA-specific SDL2 profile and player-one preference. The first
profile used raw rather than normalized SDL2 indices and failed; preserve that
lesson. The subsequent real collection launch used Nesturbator but Start did not respond under Steam Input. Its virtual-device presentation bypassed the direct run's physical-name preference. The launch script now resolves the attached external device index in context, with handheld fallback. The owner relaunched Mario 3 from the collection and confirmed the controller works. A read-only live check found the intended core and corrected player-one configuration; the game was left running. No immediate user action is required. Reconnect/handheld fallback, the complete ES-DE → game → orderly return/progress roundtrip and broad input/performance coverage remain unqualified. Further user feedback is welcome,
not a mandatory release-signoff task. The later expansion authorizes queue
preparation and transfers, not invented gameplay results or unattended full
playthroughs.

Latest Deck gameplay feedback: Mario 3 has good input timing, visuals and audio,
low perceived latency and no observed issues according to the owner. This is
ordinary-play smoke evidence, with no instrumented latency or full-game claim.
The owner subsequently reported Contra (USA) plays well with the same positive
feedback, adding a UxROM smoke result. The owner then reported Ninja Gaiden
(USA), displayed as **Shadow Warriors**, plays great, adding positive MMC1 smoke
signal. The owner next reported The Legend of Zelda (USA, Rev A) plays great
and prior progress appeared. A live check confirmed Nesturbator and shared
battery-save storage with no logged snapshot load. This is consistent with
battery-progress reuse, not cross-core snapshot support; the prior writer/core
and a newly written save/relaunch cycle remain unverified. No setup action is
pending. Save-state compatibility and longer-session coverage remain open.

Crystalis is the fifth positive Deck gameplay report: the owner says it looks,
plays and sounds great. A requested private Gamescope capture shows a thin
patterned strip at the top. The adapter sends full 256×240 with no crop option
or overscan-hint handling; the host's enabled crop preference does not change
that. Exposed overscan is plausible, but exact hardware correctness is unverified.
No display setting or code was changed. Optional presentation cropping remains
proposed; preserve the observation rather than claiming it proven normal or a
confirmed PPU bug. No immediate user action is required.

Mac Mario 1 and Mario 3 played well enough for initial smoke signal, with mild
sluggishness and some Mario 3 audio tearing under concurrent work. Resource
contention is suspected, not established. The owner explicitly deferred stronger
performance confidence to a quieter/dedicated device.

## Private replay/capture and current findings

Read [automation/CI direction](playtesting/AUTOMATION.md),
[scene findings/reference cards](playtesting/SCENE-PLAYLIST.md) and
[controls](playtesting/CONTROLS.md) before more testing. Feedback now covers 22
of 26 prepared Deck titles. The owner requested private checkpoints/automated
routes, exact known pitfalls and original quirks before play, then small owned
synthetic CI regressions for confirmed hardware interactions. Native fixes and
public helper/CI promotion remain unapproved future milestone scope.

The ignored session contains a verified stdlib/FFmpeg libretro capture prototype,
convenience replay catalog and read-only live Deck capture helper. See its
OPERATOR-NOTES.md and SCENE-LAB-README.md. Jurassic, Marble timer, Nemo opening
and MM3 menu recipes/checkpoint replays have identical per-frame RGB/PCM. Nemo
reaches NPC dialogue rather than proving the reported moving-camera defect;
MM3 menu cursor remains center. Native checkpoints are not qualified RetroArch
files; no daily SRAM input, arbitrary warp or autonomous whole-game player is
implemented. Live captures contain actual game/core identity; one named for an
earlier Blaster request actually captured DuckTales after a game switch.

An actual duplicate P1/P2 controller mapping was corrected in the owned launcher
for single-controller play, with backup and new-launch read-back. Current MM3
P1 index1/P2 unused15 are distinct. Latest MM3 state files were backed up before
reset advice. Severe crowd/damage slowdown persists after correction; the owner
believes higher jumps may be level mechanics. Do not call those solved or
cheat-caused. User-reported output FPS stays about 59.96–59.98 and does not
correlate with action slowdown. Core timing is NTSC60.0988, host sync configured
60Hz. Game-side lag is plausible, not hardware correctness established.

Priority visual observations: Jurassic OCEAN tearing, Crystalis world seam,
Marble timer shape/color concern (native outline/hole actually present), Nemo
edge/jump/HUD-border disturbances, MM3 selection line and vertical-scroll seams.
Shadow Man's opening red waterfalls are a precise reported scene; stationary
pattern later qualified as possibly illusory. Keep original-game quirks and
unknown-provenance video matches separate. Balloon Trip HUD/stars and Mario3
first power-up have explicit positive results. The owner provided two
NintendoComplete video links, stored canonically in the scene/baseline docs.

A later Shatterhand report identified a horizontal line that slowly moves
downward and wraps to the top. The actual scene was privately captured. A
backed-up owned append-profile override now disables adaptive VSync while
keeping regular VSync enabled on next launch. Global config/current gameplay
were preserved. The owner can exit normally, relaunch Shatterhand and repeat
the movement; activation/read-back and visual result are pending. This is the
next high-value host-presentation comparison, not a claimed core fix. The agent can
continue native/reference analysis from private artifacts; another native
implementation or CI phase needs its own scope decision. No core behavior,
emulation rate, dependency or collector window changed here; the prepared
per-core adaptive-VSync presentation override is the explicit later exception
to the earlier unchanged-display observation.
Historical stops and planning boundaries remain intact.

## Host work and operational privacy

A two-screen extended desktop put Steam on the built-in panel and wallpaper on
the TV. A backed-up TV-only docked layout resolved visibility; user confirmed it.
An ES-DE post-update Steam failure logged an EmuDeck wrapper shell error. A
backed-up stable wrapper preserves configured cloud-save hooks, invokes the
installed AppImage and avoids launcher regeneration/backend updates on launch.
Its version route and actual Steam/Gaming Mode startup passed. Do not infer the
historical shell error's exact cause beyond the observed evidence.

The installed Linux x64 module came from a digest-checked v5 CI artifact, with
valid package/ELF/dependency checks. It is a development artifact, distinct from
published package 0.1.11. No native emulation code, ABI, state format or behavior
changed. Temporary transferred ROM duplicates were removed after verification;
no separate test game system was registered.

Private operational manifests and provenance are under ignored
`private/nes-playtest-2026-10-30/`. Use those for connection/installation/backup
locations, never copy their private contents or identifiers into Git. The source
checkouts and original media remain intact. The direct diagnostic Mario service
is stopped; only the normal ES-DE session is intentionally left running.

## Planning continuation

`$gsd-new-milestone` resumes the pending scope decision. Context may be cleared
first. v5 stays archived with all eleven requirements satisfied, canonical
119 PASS/27 FAIL over 146 scored rows plus five unscored DRAW, and ABI 1/state
format 3/behavior 9. Read its closeout/audit for preserved debt and evidence limits.
The earlier live check found release PR #56 still open; recheck before claiming
new publication. Existing historical runtime stops remain unchanged.

The preferred proposal remains player-ready NES delivery, now informed by the
faster Deck/ES-DE daily-feedback loop before deeper Playstead work. Actual
Playstead integration needs its consumer-owned implementation and acceptance.
DMA follows readiness unless concrete title evidence changes priority; PAL is
the regional alternative. No new requirements register, execution phases or
milestone switch was approved. Reuse the completed sibling research rather than
repeating broad fan-out. Preserve the pre-existing config/instruction edits.

Latest owner concern: protect daily-use save states across future core updates. Current format 3 refuses earlier formats; recorded behavior revision is not enforced. A future supported-version/migration/rollback policy must be explicit, even before package 1.0. See lessons; no compatibility guarantee was added.

Latest presentation follow-up: a relaunched Shatterhand profile was read back
with regular VSync on/adaptive VSync off and corrected input ports. Some falling/
left-right disturbances and busy helpers/fireballs slowdown remain; music
briefly stopped after a hit by owner report. No fix is claimed. Three-second
output-only audio capture succeeded separately from video. A numbered slot0
load failed because no slot file existed; a distinct auto-state is available
and backed up, exact scene uninspected. Reset was unnecessary for this
presentation change; no further mandatory owner replay is needed. Native
Shatterhand navigation failed to reach gameplay within three bounded attempts;
its final output matches native-runner replay, so neither helper nor core cause
is diagnosed. Preserve that limit and MM3 menu-interactivity uncertainty.
