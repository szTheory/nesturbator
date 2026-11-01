# NES to Playstead: current boundary and recommended handoff

Researched: 2026-10-30. Read-only consumer inspection; no app/emulator run,
download, installation or sibling edit. Recommendations are proposed scope.

## What is established

- Nesturbator already ships library, headless runner and libretro adapter, with
  battery RAM, validated states, rewind/run-ahead and runner stop/exit contracts.
  Reimplementing those because Muhtari is adding them would duplicate delivered
  work. Authorities: [v5 closeout](../../milestones/v5-CLOSEOUT.md),
  [public contract](../../../include/nesturbator.h) (state identity at lines 53–100,
  battery/dirty tracking at 510–540), and [README](../../../README.md).
- Playstead's checked-in ordinary adapter is standalone mGBA, not RetroArch:
  [AdapterPin.json](../../../../playstead/playstead-mac/Playstead/Adapter/AdapterPin.json)
  names its app, argument template and save/exit behavior;
  [AdapterHost.swift](../../../../playstead/playstead-mac/Playstead/Adapter/AdapterHost.swift)
  uses `Process` arguments (around 599–601) and documents mGBA-specific input
  limits (around 379). A new system label cannot configure another frontend.
- Playstead's existing exact-state support is a narrow managed mGBA Game Boy
  MBC1/8 KiB SRAM/macOS arm64 row. It establishes no NES state support:
  [STATE-SUPPORT.md](../../../../playstead/playstead-mac/docs/STATE-SUPPORT.md),
  [StateSupportMatrix.swift](../../../../playstead/playstead-mac/Playstead/Adapter/StateSupportMatrix.swift).
- Playstead has already captured Muhtari as an early integration priority, with
  external RetroArch first and a later native-host decision. This is posture,
  not delivered integration; preserve that priority rather than silently
  replacing it with NES:
  [SEED-040](../../../../playstead/.planning/seeds/SEED-040-atari-7800-muhtari-play-and-progress.md)
  (lines 13–27, 51–110 and 121–155).

Inspected Nesturbator source HEAD `6e9ce44`, Playstead HEAD `36830ad`; both
working trees may contain additional edits. These short identities are
navigation context, not immutable attribution of dirty file bytes. Re-read the
consumer before implementation.

## Recommendation and tradeoffs

| Decision | Preferred choice | Benefit / tradeoff / refusal case |
|---|---|---|
| Interactive route | Existing libretro core in separately verified RetroArch | Uses shipped host seam and keeps device code out of core. External window/focus/config friction remains; qualify it in Playstead. Runner success is not an interactive player. |
| Integration ownership | NES supplies a compact released-contract handoff; Playstead owns adapter/install/session/UI | Prevents two repositories implementing incompatible launcher policy. Actual consumer acceptance remains open until its production composition runs. |
| Artifact identity | Bind host executable, loaded core module, `.info`, architecture and configuration; distinguish released from development build | A checked archive, current source tree or displayed version alone cannot establish the module actually loaded. Reuse existing checksum/manifest facilities; no new installer framework. |
| Process launch | An argument vector, bounded readiness/exit and per-game save/config locations | Paths containing spaces remain data. Avoid shell command interpolation, global RetroArch config changes and broad process cleanup. |
| Progress | Ordinary launch + battery reload first; explicit states as separately qualified operations | A fresh launch is not exact Continue. Preserve newer durable battery progress before restoring an older state; state support does not prove custody/recovery. |
| Native hosting | Revisit after measured external-host friction or missing state-control capability | Could improve one-app UX, but adds A/V/input/pacing, crash isolation and maintenance work. Do not build an ecosystem host inside this core. |
| Shared family infrastructure | Reuse conventions and small understood helpers first | A shared SDK/daemon/schema repository would create version and ownership coupling before a second actual consumer proves the need. |

## Small upstream handoff

The proposed NES handoff should state: released package and exact core/info
assets; admitted NTSC board/input/content types and explicit exclusions; no
proprietary BIOS requirement for this supported cartridge scope; launch shape;
per-game battery file placement and orderly shutdown; native format and
ROM/mapper/submapper/region refusal rules; recorded behavior revision (which
the core does not enforce on load); controller/keyboard responsibility;
known limits; and the legal fixture plus acceptance steps for the real consumer.
Keep fixture acquisition and commercial content outside the repository. A
Playstead support row may bind behavior/executable identity more strictly than
the native loader; document that as consumer policy, not existing core refusal.

Use one consumer-owned fixture session to prove library selection → verified
RetroArch/core → responsive play → orderly return/focus → fresh-process battery
reload. A later state row must prove capture, restore, mismatch refusal and
recovery of newer durable progress. Existing Nesturbator hosted RetroArch and
portable adapter tests establish valuable upstream seams, not this consumer
chain. Do not add a new global ABI or promise cross-version state migration.

## Primary external checks

The official [RetroArch CLI guide](https://docs.libretro.com/guides/cli-intro/)
documents explicit core/content loading and custom config paths, supporting
the external route. It does not establish Playstead integration.
The official [run-ahead guide](https://docs.libretro.com/guides/runahead/)
describes dependence on state quality and additional CPU work; use this to
sequence a normal-play baseline before enabling optional latency features,
not as proof that a particular host/core pair is correct. Checked 2026-10-30.

## What would change this recommendation

A consumer-owned RetroArch adapter with current acceptance could reduce the
handoff to configuration and a NES qualification row. Conversely, measured
focus/input/state-control friction could justify a focused Playstead-owned host
spike. Neither condition is established by the files inspected here.
