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

## Ready for targeted scenes

The owner next reported Gradius, Solar Jetman, Balloon Fight, Kirby, Battletoads
and Batman play well/great, describing the sessions as smooth without perceived
performance issues. Eleven positive title reports now span all six supported
mapper IDs. Exact scenes, every board variant and measured performance are
separate. The owner explicitly wants concrete known pitfalls and efficient
scene access before each recommendation, as durable cross-emulator learning.
Read the [scene playlist](playtesting/SCENE-PLAYLIST.md) before recommending more
play; the [full queue](playtesting/DECK-QUEUE.md) tracks all prepared entries.

Start Jurassic Park (USA), letting the OCEAN logo run, then Marble Madness (USA)
for its first race-introduction text box. The Magic of Scheherazade is also
installed for an optional 5W-password/party-battle damage probe; no deterministic
immediate encounter route was verified. Three new NTSC games passed admission,
frame-120 startup, archive-hash and per-game choice checks. Original files,
metadata and save storage were preserved, current progress backed up privately.
The collection has 26 entries, 15 with no Deck gameplay report yet.

The owner returned to ES-DE after an active-game guard deferred refresh. Remote
WM/keyboard quit requests did not exit the app; a direct normal close-protocol
message to its verified window did. The normal Steam shortcut then reopened
ES-DE and its fresh log loaded the expanded collection and completed startup.
No game was interrupted/automatically launched, and no force kill, device
restart, display setting or core behavior change was used. No restart/setup
work remains; the owner acts next when ready to play. The agent can continue
recording scene-specific observations and helping with verified routes.

Game Genie is not currently implemented; cheat callbacks are empty and general
system RAM is not exposed. A frontend cheat menu is not proof of support.
Prefer built-in passwords, then snapshots made with this core after reaching a
scene. Preserve daily progress since state loading may replace battery RAM.
Foreign-core states and raw runner payloads are not qualified RetroArch scene
shortcuts. No scene snapshot, replay library, cheat implementation or automated
warp was created. Treat these as future proposed capabilities, not new milestone
scope. Planning still has its separate boundary below.

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
