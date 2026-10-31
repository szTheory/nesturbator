# Protecting emulator progress

This guide covers RetroArch's host-managed files and the current core's
native save behavior. It does not change a save format or promise that an
older core or another frontend can load a newer file. Keep your existing
library entries and core-specific state storage.

## Keep the data types separate

- **In-game battery save:** The game stores persistent progress in cartridge
  battery RAM. For supported cartridges the core exposes that RAM to the host;
  RetroArch normally stores it as an `.srm` file. The core does not choose a
  universal directory or perform file I/O.
- **Native core snapshot:** A nesturbator state contains the emulated machine,
  including its battery RAM at the time of the snapshot. Loading an older
  snapshot can therefore replace newer in-game save data. Rewinding past an
  in-game save can do the same.
- **RetroArch disk state:** RetroArch may wrap the serialized core payload in
  a host-specific state file. The host's container and automatic save/load
  settings are separate from the native payload. Renaming a native payload
  does not turn it into a RetroArch state file; other frontends can use other
  containers.
- **Automatic loading:** The frontend can load its state when content starts.
  A same-core automatic load does not establish recovery behavior for an
  incompatible state or for another core build.

Find the active frontend's configured **Save Files** and **Save States**
locations in its directory settings. Current RetroArch documentation shows
`Settings` → `Directory`; defaults depend on the installation and configuration.
Use the same ES-DE/Steam/RetroArch launch route that starts the game, because a
separately opened frontend can use a different configuration. Keep states in
the existing core-specific folders rather than moving them between cores.
See Libretro's [directory configuration](https://docs.libretro.com/guides/change-directories/)
and [Quick Menu guide](https://docs.libretro.com/guides/quick-menu/), which
documents the host's Load/Save States controls and `Close Content` action.

## Supported snapshot baseline

For reliable snapshots, pair the exact core build with the exact content image.
The native loader also checks format, cartridge identity, size, checksum and
field bounds. The current source tree uses state format 6 and refuses other
format versions, including formats 1, 2, 3, 4 and 5. It records the behavior revision
but does not enforce it as a compatibility check; a same-format state from
before an emulation fix can load and later diverge. These native checks do not
describe the frontend's disk container or recovery policy. Check the guide
from the source tag that matches the binary you run; an older published core
may have different behavior.

See the [native save-state contract](api.md#save-states) for the canonical API
rules and the distinction between supported build pairing and loader checks.

For isolated scene reports, cold movie replay and neutral native-checkpoint
examples, see the [scene reproduction guide](reproduction.md). Those examples
use disposable build-tree paths and never import daily SRAM. Keep the newest
independent battery backup outside every active reproduction path.

Battery-save reuse across core builds is separate from snapshot support. Reuse
an existing battery file with a different build only when the content, board,
save geometry and compatibility have been established. Otherwise keep both
copies and report compatibility as unknown. Never overwrite the newest
battery backup to make an older core or state load.

## Before upgrading or loading an older state

1. Close the content and frontend cleanly so the host can flush its current
   save. Do not use a forced process stop as proof that a save completed.
2. Record the core module path and digest, content identity, frontend version,
   active launch route and relevant host configuration identity. Keep private
   paths and raw logs out of Git.
3. Copy the newest battery save to a backup location outside every active save,
   state and walkthrough scratch directory. Separately preserve the previous
   core build, content identity, all relevant states, and the battery data
   paired with each state. Do not combine or replace these backup sets.
4. Hash each source file and its copy, then compare the digests. Use
   `shasum -a 256` on macOS, `sha256sum` on Linux, or
   `Get-FileHash -Algorithm SHA256` in PowerShell. Modification times alone do
   not establish that a copy is complete.
5. Keep the newest battery copy outside the paths RetroArch can load or save.
   Load an older snapshot only when you are willing to restore the battery RAM
   captured in that snapshot.

## Bounded disposable walkthrough

The repository's generated battery cartridge and manifest-listed `dabg.nes`
fixture are test inputs, not personal game data. `runner.save.roundtrip`
checks that a seeded battery save is read before the first frame and written
back; `runner.state.roundtrip` checks a compatible native state roundtrip;
`runner.state.reject_format` deliberately changes a generated state version
and expects the runner to refuse it. The native runner refusal is not evidence
about a RetroArch disk container.

The [disposable progress fixture instructions](testing.md#disposable-progress-fixtures)
describe the owned helpers and the scope of the reused native and host evidence.

From the repository root, use a fresh scratch/build directory for each
simultaneous invocation. The registered tests create case-specific scratch
subdirectories below that build and write only generated/manifest-listed test
fixtures there:

```sh
(
set -e
scratch="$(mktemp -d "${TMPDIR:-/tmp}/nesturbator-progress.XXXXXX")" || exit 1
test -n "$scratch" && test -d "$scratch" || exit 1
backup="${scratch}.backup"
config="$scratch/config"
mkdir -p "$config"
cmake --preset ci -B "$scratch/build"
cmake --build "$scratch/build" --target nesturbator-run runner.save_rom runner.state_mutate
ctest --test-dir "$scratch/build" --output-on-failure --no-tests=error -R '^runner.save.roundtrip$'
mkdir -p "$backup"
cp "$scratch/build/tests/save_work/roundtrip/battery.sav" "$backup/newest-battery.sav"
shasum -a 256 "$scratch/build/tests/save_work/roundtrip/battery.sav" "$backup/newest-battery.sav"
ctest --test-dir "$scratch/build" --output-on-failure --no-tests=error -R '^(runner.save.mismatch|runner.state.roundtrip|runner.state.reject_format)$'
shasum -a 256 "$scratch/build/tests/save_work/roundtrip/battery.sav" "$backup/newest-battery.sav"
)
```

The command block assumes a POSIX shell and stops on scratch creation or
command failure before continuing to backup or replay. In PowerShell, create a
unique root
with `Join-Path $env:TEMP ("nesturbator-progress-" + [guid]::NewGuid().ToString("N"))`,
create its `config` and `build` directories, and use `Copy-Item` plus
`Get-FileHash -Algorithm SHA256` for the backup and digest steps.

The two digest lines must match. `sha256sum` or PowerShell's `Get-FileHash`
can be used instead on those platforms. The second CTest selection does not
touch the copied battery file. If a command is interrupted, the isolated
scratch may remain; inspect that unique directory before removing it, and do
not point a test at an existing save directory. Start each concurrent run with
a different `scratch` value and build tree; its `config` directory is isolated
and unused by these runner-only checks. The sibling `backup` directory is
separate from the active test tree and should be kept until digests match.

The independent `core.state_reject` test checks the whole machine and battery
generation after refusals; `core.state_generation` checks when accepted or
refused loads change that counter. Their sources are
[`tests/core/test_state_reject.c`](../tests/core/test_state_reject.c) and
[`tests/core/test_state_generation.c`](../tests/core/test_state_generation.c).
The runner refusal above does not prove those in-memory properties. Existing
host checks establish only their named, same-core scenarios; this walkthrough
does not test a live host, cross-build battery reuse or incompatible host
autoload recovery.

## Rollback without replacing newer progress

Keep both backup sets intact: the newest battery save on its own, and the
previous core plus the snapshots and battery data paired with them. To recover
from a state that will not load, isolate that state outside the active state
folder, disable or isolate automatic loading, select the identified retained
core through the usual frontend route, and start the content fresh. Do not
automatically restore an older snapshot over the newest battery save. Reuse
that battery file with the retained core only when content identity, board,
save geometry and compatibility are established; otherwise preserve it
unchanged and treat reuse as unknown. An incompatible RetroArch container may
need host-specific handling, which native refusal tests do not qualify.

The operational menu labels vary by host/version. RetroArch's [starting a game
guide](https://docs.libretro.com/guides/starting-a-game/) describes loading a
core and content; check the installed frontend's current settings before
changing its paths or automatic-state options.
