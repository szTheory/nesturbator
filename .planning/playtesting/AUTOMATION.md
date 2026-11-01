# Private scene replay and synthetic regression direction

Updated 2026-10-30. The owner authorized private commercial-game checkpoint,
replay and capture automation to reduce manual play effort, and requested that
confirmed findings feed efficient owned CI tests. No core fix or CI fixture was
implemented during this operational setup.

## Working private loop

The ignored session contains `scene_lab.py`, `replay_scene.py`,
`deck_capture.py`, the recipe/catalog files and an operator README. It uses
Python's standard library, the existing libretro module and installed FFmpeg.
No dependency was added to the emulator or its builds.

`scene_lab.py` loads the adapter through exported libretro callbacks, supplies
bounded per-frame inputs, writes a native-payload checkpoint and exports exact
PNG/PPM frames, PCM WAV and a short audiovisual MP4. Its metadata binds content,
core module and recipe hashes, completed frame and checkpoint identity. It
never reads or writes daily battery-save files. A checkpoint replay applies the
remaining input recipe; a fresh replay starts from content load with zeroed
battery RAM. This is an owned private frontend, not RetroArch's UI/container.

Four named cases now work without the owner playing:

| Case | Verified access and output |
|---|---|
| `jurassic` | Fresh boot into the OCEAN animation; checkpoint after frame 60; short logo clip and native captures |
| `marble` | Scripted player/name/control menus into the first race's intro and timer; checkpoint after frame 900; short clip and native captures |
| `nemo` | Scripted opening into Mushroom Forest, then walk/jump inputs and NPC dialogue; exact moving-camera artifact is not yet reproduced/qualified |
| `mm3` | Portrait/menu animation with Shadow Man visible; checkpoint after frame 450, cursor stays center and interactive navigation is unqualified |

The local convenience command is `python3 private/nes-playtest-2026-10-30/replay_scene.py jurassic`
or the same command with `marble`, `nemo` or `mm3`; `--fresh` runs the route from reset instead
of the checkpoint. Exact media locations remain in the ignored private catalog.
Each run gets a new output directory. Recipe execution is capped at 3600 frames,
capture at 15 emulated seconds, and each helper run at a 15-second wall bound.
These controls are separate from archived collector windows; no collector
entrypoint or stopped campaign was invoked.

All four commercial cases were replayed from their checkpoints with identical
per-frame RGB and PCM hashes to the fresh route. The helper's owned input
fixture also matched the native runner's framebuffer, including held/released
input; output/path/identity/encoder guards were exercised. This establishes
repeatability of current output, not correctness of a suspected defect.

`deck_capture.py` separately captures the current Nesturbator display over SSH
without game inputs, pause/reset or state commands. It checks the active mapped
core/content, validates complete PNG transfers and cleans only its owned remote
temporary directory. Live Mario 3, DuckTales, Little Nemo and Mega Man 3 captures succeeded, including
Mega Man 3's stage-select and Shadow Man waterfall scene. A subsequent
capture refused when no game was active. Capture identity is authoritative: an
attempt named for Blaster Master actually found DuckTales after a game switch,
and is recorded as DuckTales rather than misattributed to the earlier title. Its sampled clips have no audio and
are unsuitable for measuring gameplay frame pacing.

## Current limits

- There is no general autonomous game player or arbitrary scene-teleport API.
  New routes need known input recipes, verified built-in shortcuts or a reached
  checkpoint. Camera movement and battle navigation remain unautomated cases.
- Private headless native checkpoints are not qualified RetroArch disk files.
  Do not copy/rename them into a Deck state slot and claim host compatibility.
  Actual RetroArch auto-load was observed separately in Mario 3.
- The prototype has no SRAM-import option. It does not reproduce an owner's
  existing progress merely by loading the ROM. It does not mutate daily saves.
- Strict module/recipe identities reject stale checkpoints after a change.
  Rerun the input recipe from reset to establish a new checkpoint on a changed
  core, rather than assuming snapshot compatibility.
- Game Genie callbacks remain empty, general system RAM is not exposed, and
  automatic ROM patches remain disabled in the daily launcher. No cheat or
  patch injection was added. Commercial images, states and capture bytes stay
  private and outside CI.

## Host routing finding before more core diagnosis

The active Mega Man 3 append config set player one to controller index 1 while
inheriting player two index 1 from the normal host config. This concrete mapping
error can trigger game-internal second-controller effects, even though external
Game Genie is unsupported. It strongly motivates a targeted recheck of the
reported high jumps and severe slowdown; no causal confirmation is claimed yet.

A backed-up owned-launcher correction now emits an unused player-two index for
the single-controller profile. Shell syntax passed. Metroid initially retained the old active config; a later
Mega Man 3 relaunch was read back with P1 index1 and P2 unused index15. Latest
Mega Man 3 progress was backed up before recommending reset. The owner still
reports major crowded/damage slowdown with the corrected mapping and considers
the higher jumps possibly a level mechanic. Keep cause unclassified; the routing
error was real but did not explain away the slowdown or visual findings.
Two-player games need an explicit distinct second-controller mapping later.

Add a cheap host-profile invariant at future portable-helper promotion: one
physical controller must not accidentally feed two active ports. Existing
owned two-port input tests validate the core but cannot validate a user's
external host configuration. This host fix needs no speculative core timing
change and must remain separate from the visual findings below.

## Presentation comparison before a core regression

Shatterhand's user-reproduced line slowly drifts downward and wraps to the top.
A private live clip preserves the first-stage location. That pattern strengthens
host-presentation tearing as a competing explanation to fixed NES-row artifacts.
The global config had Adaptive VSync enabled alongside normal VSync. The owned
Nesturbator append profile was backed up and now disables adaptive VSync while
retaining regular VSync on the next launch. Current gameplay/global config were
preserved. A fresh Shatterhand launch was read back with these settings. The owner still
reports falling/left-right disturbances, busy-scene slowdown and a temporary
music interruption; exact original drift comparison remains inconclusive. No
tearing fix is claimed. Treat this as the next highest-value experiment,
before translating every multi-game seam into a synthetic PPU failure.

## Highest-value CI candidates

First capture and localize the failure; then reduce the hardware interaction.
The current findings suggest two small fixture families, not a large commercial
replay suite. Root causes remain unconfirmed.

| Observation | Existing owned coverage to reuse | Candidate independent guard |
|---|---|---|
| OCEAN wobble tearing, Crystalis world seam, and unclassified Nemo/MM3 split observations | `ppu.split_scroll`, `ppu.mmc3`, `core.mapper_mmc3` | An owned mapper-4 program whose CPU IRQ handler changes scroll; assert IRQ acceptance and exact pixel-row boundaries at a few neighboring alignments |
| Marble Madness timer shape/color concern | `ppu.render`, `ppu.fetch`, `core.palette`, `host.convert` and runner/adapter parity | Begin with an original two-bitplane hollow glyph and contrasting outline, then a timed pattern-table selection; assert fetched addresses and native color indices before RGB conversion |
| Balloon Trip fixed HUD/stars reported good | Existing register/read-buffer, rendering PPUDATA and split tests | Add only a missing interaction demonstrated by a failing reduction; retain the positive scene report without duplicating existing checks |

The existing split-scroll test uses hand-derived row origins and an independent
pixel sampler, but primarily schedules direct register writes away from the
hardest dot boundaries. It does not prove the complete CPU → IRQ handler → PPU
write → rendered split path. That integration boundary is the leading candidate
for investigation, not a diagnosed cause of both games.

The automated native-size adapter RGB Marble capture contains dark-blue timer edging and a small
opening in its 4. Preserve the user's differing impression and source video;
compare exact scene/digits and host/native images before declaring a missing-
bitplane or palette defect. A video with unknown capture hardware, emulator,
palette or revision is comparison evidence, not a hardware oracle.

Use the existing [iNES fixture helper](../../tests/ines.h) and
[PPU fixture](../../tests/ppu/ppu_fixture.h). Write original program/pattern bytes
from hardware documentation; do not copy commercial code or graphics into a
synthetic fixture. No assembler dependency is required for these small cases.
Expected values should come from hardware timing equations, fetch addresses
and explicit pixel masks. Verify a deliberately wrong timing/bitplane control
fails the oracle. Freeze a hash only after independent expected behavior is
established, and require the minimized test to fail before its fix and pass
afterward. A hash of current buggy output is not a correctness oracle.

## Keeping the ratchet efficient

Keep small deterministic guards in existing CTest targets and measure their
cost. Add a runner/adapter integration case only when it detects something the
unit-level test cannot. Broad repeated sweeps can move to scheduled lanes if
their marginal signal justifies the cost.

Retain each unique proven regression invariant. Consolidate or retire duplicate
checks only after demonstrating equivalent detection, including their failure
controls; repeated passing alone is not a reason to remove protection. A flaky
infrastructure check needs an attributable quarantine/replacement plan, not a
weaker expected value. Retire obsolete exploration steps and redundant capture
campaigns while preserving the distilled regression.

Existing DMA failures and historical collector stops remain separate debt.
These visual reports do not establish a DMA cause or authorize another campaign.
Public helper promotion, native fixes and CI additions remain future scoped
work, informed by the [scene findings](SCENE-PLAYLIST.md) and
[rolling direction](../PRODUCT-ROADMAP.md).

Additional bounded Shatterhand navigation did not reach the first stage after
three attempts; only intro/title output was captured. It is not a gameplay or
seam checkpoint. Its final frame matched native-runner replay of the same input
recipe, and an owned input callback control delivered Start, so the navigation
failure remains unclassified rather than diagnosed as a helper/core bug.
A separate three-second live output-monitor audio recording succeeded (stereo
48 kHz, no microphone input); it was not synchronized to the display clip and
does not establish that the reported brief music interruption recurred.
