# Scene reproduction

## Report a scene

Use this form for an uncertain report too. `unknown`, `withheld`, `not tried`
and `unavailable` are valid values; say what could not be established instead
of guessing. Keep the report revisable and local until its contents are safe to
share. A title-level observation does not establish that the reported scene was
reached.

```text
Report revision/date:
Title and region:
Content revision or digest:
Loaded core and exact build identity:
Host and host version:
Launch route and relevant settings:
Exact scene or point reached:
Access method and whether it was verified:
Start conditions (cold boot, checkpoint, save, or other):
Inputs in order, including port for each input:
Expected behavior and its source/confidence:
Observed behavior:
Repeatability (attempts and outcomes):
Conditions not tried:
Evidence limits or unknowns:
Attachments withheld from public report:
```

Record the actual loaded core/build and host route. A package label alone may
not identify a development build. State whether the expected behavior comes
from an original-system observation, a documented game behavior, a prior build,
or an assumption, and how confident that source is. If access failed or the
scene was not reached, report that result; do not turn it into scene acceptance
evidence. Keep title-level observations separate from exact-scene results.

Do not publish commercial game media, saves, native checkpoints, raw private
logs or captures, personal paths, account names or other personal identifiers.
Share only the sanitized fields needed to understand the report. Do not attach
an artifact merely because a report form asks whether one was withheld.

## Permitted fixture example

This is an owned source-test fixture example, not a report about a commercial
game or a claim about host presentation. The movie is generated locally from
the repository's `runner.game_movie` test target; no movie or save bytes are
committed.

```text
Report revision/date: 1 / 2026-10-31
Title and region: nes-runner permitted fixture / region not established
Content revision or digest: tests/roms/runner.nes, SHA-256 1a67bfabdde2d96eacb04eb7f9d17e8da6c082d4976c318315fe31e2d19b4384
Loaded core and exact build identity: native nesturbator-run 0.1.11; source checkout d574549 at measurement time
Host and host version: native command-line runner / not applicable
Launch route and relevant settings: cold boot with the generated two-port NMOVIE1 movie; --hash-audio
Exact scene or point reached: scoring play, as judged by the existing runner.boot_to_play test
Access method and whether it was verified: runner.game_movie scenario nesrunner-pos; verified by runner.boot_to_play
Start conditions (cold boot, checkpoint, save, or other): fresh process, no load-state and no save directory
Inputs in order, including port for each input: port 0 Start at zero-based movie indexes 60–63; port 0 A/jump spans are generator-owned; port 1 is neutral
Expected behavior and its source/confidence: the owned test expects a start transition, sustained scoring play and changing native video/audio; high confidence for this test predicate only
Observed behavior: two 600-frame cold executions matched; ordered frame-record SHA-256 a90cc60caeea1fc4e4f159e3341b5f1b446022281c018b6d609eebf702175d9b; transition SHA-256 33e4bce709976dca002689e4cc164f694a677d9949fdfa325b7dd6cfa5cc4006; PCM SHA-256 255636674903d03dd9cb7ca632246c3375dff8c88300155b052ac5c273d4dcca
Repeatability (attempts and outcomes): two cold executions; CTest passed on 2026-10-31 using the local macOS arm64 CI build
Conditions not tried: another region, host adapter/presentation, hardware behavior
Evidence limits or unknowns: repeated hashes establish repeatability on this local build only, not hardware correctness
Attachments withheld from public report: none; raw runner logs remain in build scratch
```

The Start and jump input constants are documented beside the owned generator
in [`tests/runner/game_movie.c`](../tests/runner/game_movie.c). The existing
`runner.boot_to_play` check independently retains the no-input, attract-only
and Start-only controls. Its predicates are not replaced by hash comparison.

## Cold movie replay

Build from a source checkout with the CI preset. The movie writer is a test
target available in that source build; it is not an installed runner feature.
Use a fresh scratch build so generated files stay outside the source tree.

POSIX shell:

```sh
scratch="$(mktemp -d "${TMPDIR:-/tmp}/nesturbator-reproduction.XXXXXX")"
cmake --preset ci -B "$scratch/build"
cmake --build "$scratch/build" --target nesturbator-run runner.game_movie runner.boot_to_play
ctest --test-dir "$scratch/build" --output-on-failure --no-tests=error \
  -R '^(runner.reproduction.cold|runner.reproduction.compare_self_test|runner.boot_to_play|docs.links|docs.links.self_test|docs.reproduction|docs.reproduction.self_test|cmake.script_policy)$'
```

PowerShell:

```powershell
$scratch = Join-Path $env:TEMP ("nesturbator-reproduction-" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Path $scratch | Out-Null
$build = Join-Path $scratch "build"
cmake --preset ci -B $build
cmake --build $build --target nesturbator-run runner.game_movie runner.boot_to_play
ctest --test-dir $build --output-on-failure --no-tests=error -R '^(runner.reproduction.cold|runner.reproduction.compare_self_test|runner.boot_to_play|docs.links|docs.links.self_test|docs.reproduction|docs.reproduction.self_test|cmake.script_policy)$'
```

The cold test makes a unique directory below the build tree, generates the
600-frame `nesrunner-pos` NMOVIE1 movie, and invokes the public runner twice
from a cold boot. The generator is bounded to 10 seconds, each runner process
to 60 seconds, and the enclosing CTest case to 180 seconds. It requires all
600 ordered `frame N ticks T sha256 H` records and both `audio transitions`
and `audio pcm` SHA-256 lines from each run. The fixture example above records
the SHA-256 of the ordered frame-record stream and the two named audio hashes
measured from the passing run. These values describe repeatability for the
local build and platform recorded there.

The runner validates the two-port NMOVIE1 format. `--movie` supplies its own
length, so do not combine it with `--frames`; movie replay with `--load-state`
is unsupported. Host recording and conversion of foreign movie formats are
also unsupported. The report/replay route does not write daily saves or create
checkpoint files. Transition hashes, PCM hashes and native frame hashes are
separate repeatability measurements; matching values are not independent
hardware-correctness evidence. See the [runner guide](runner.md),
[testing guide](testing.md) and [progress guide](progress.md) for their
canonical contracts.

## Native checkpoint continuation

The supported checkpoint demonstration is separate from cold movie replay.
`runner.reproduction.preflight` creates a fresh owned NMI fixture, asks the
exact `nesturbator-run` target to save after absolute frame 1, and records the
runner executable, fixture ROM and state SHA-256 values in that invocation.
It calls the same `reproduction_preflight` function exercised by
`runner.reproduction.preflight_self_test`. The state is created by the runner
being identified; this same-invocation route is the only origin that this
example can qualify.

The existing `runner.state.nmi_roundtrip` test separately demonstrates the
neutral-input native continuation. It uses no movie, saves at absolute frame
K=1 and resumes through absolute end frame N=4. The continuation runs frames
2–4; it is not four additional frames. Its independent reference starts audio
collection at frame 2 and compares the same transition and PCM suffix through
frame 4. The guest NMI event sequence is checked at the saved and final states,
and the resumed native frame and serialized state are compared with the
straight run. `runner.state.nmi_idempotent` retains the repeated-state-byte
control. `runner.state.nmi_wrong_load` retains independent stale-load and
omitted-load guest-marker failures. These guest markers and causal controls
support their named invariants; matching output hashes alone show
repeatability.

Before a checkpoint is replayed, the recorded runner, content and state SHA-256
values must each be present, contain 64 hexadecimal characters, and match the
current files. The recorded origin must be `runner-native-same-invocation`.
Missing or changed identities, an unknown origin and a host container stop the
procedure with `reproduction unsupported:` before any replay. A digest is an
identity check for this declared route, not hostile-file authentication or a
container converter. Never infer identity from a menu version, replace a
recorded digest with the digest of a changed file, or silently choose another
checkpoint.

Native format version 3 and behavior revision 9 are separate values. The
loader checks the native format, cartridge identity, size, checksum and field
contents, but it does not enforce the exact executing build or behavior
revision. A state may therefore pass native loading and still be outside the
documented exact-build/content support policy. Successful preflight does not
bypass those native loader checks. See the [save-state contract](api.md#save-states)
and [progress policy](progress.md#supported-snapshot-baseline).

`--movie` with `--load-state` remains a usage refusal. Host movies, host
recording and foreign movie conversion are unsupported; a runner-native state
cannot be renamed into a RetroArch disk state. The procedure's unsupported
result is not a new runner exit code: runner exits remain 0, 1, 2, 4 and 5, with
3 and 77 reserved, as listed in the [runner exit status contract](runner.md#exit-status).
