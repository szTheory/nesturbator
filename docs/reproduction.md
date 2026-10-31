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
(
set -e
scratch="$(mktemp -d "${TMPDIR:-/tmp}/nesturbator-reproduction.XXXXXX")" || exit 1
test -n "$scratch" && test -d "$scratch" || exit 1
cmake --preset ci -B "$scratch/build"
cmake --build "$scratch/build" --target nesturbator-run runner.game_movie runner.nmi_fixture runner.boot_to_play runner.save_rom runner.state_mutate
ctest --test-dir "$scratch/build" --output-on-failure --no-tests=error \
  -R '^(runner.reproduction.cold|runner.reproduction.compare_self_test|runner.reproduction.preflight|runner.reproduction.preflight_self_test|runner.boot_to_play|runner.state.nmi_roundtrip|runner.state.nmi_idempotent|runner.state.nmi_wrong_load|docs.links|docs.links.self_test|docs.reproduction|docs.reproduction.self_test|cmake.script_policy)$'
)
```

PowerShell:

```powershell
& {
$ErrorActionPreference = 'Stop'
$scratch = Join-Path $env:TEMP ("nesturbator-reproduction-" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Path $scratch | Out-Null
$build = Join-Path $scratch "build"
cmake --preset ci -B $build
if ($LASTEXITCODE -ne 0) { throw 'Configure failed' }
cmake --build $build --target nesturbator-run runner.game_movie runner.nmi_fixture runner.boot_to_play runner.save_rom runner.state_mutate
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
ctest --test-dir $build --output-on-failure --no-tests=error -R '^(runner.reproduction.cold|runner.reproduction.compare_self_test|runner.reproduction.preflight|runner.reproduction.preflight_self_test|runner.boot_to_play|runner.state.nmi_roundtrip|runner.state.nmi_idempotent|runner.state.nmi_wrong_load|docs.links|docs.links.self_test|docs.reproduction|docs.reproduction.self_test|cmake.script_policy)$'
if ($LASTEXITCODE -ne 0) { throw 'Reproduction checks failed' }
}
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
user checkpoint files. The preflight example creates only an owned native
checkpoint under its disposable build scratch. Transition hashes, PCM hashes
and native frame hashes are separate repeatability measurements; matching
values are not independent
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

## Before checkpoint replay

Before a checkpoint is replayed, the recorded runner, content and state SHA-256
values must each be present, contain 64 hexadecimal characters, and match the
current files. The recorded origin must be `runner-native-same-invocation`.
Missing or changed identities, an unknown origin and a host container stop the
procedure with `reproduction unsupported:` before any replay. A digest is an
identity check for this declared route, not hostile-file authentication or a
container converter. Never infer identity from a menu version, replace a
recorded digest with the digest of a changed file, or silently choose another
checkpoint.
`PAYLOAD_KIND` is a declared origin label; a matching value does not
authenticate how an arbitrary imported state was produced. Only the positive
fixture test establishes this example's same-invocation creation route.

The example-local `MODE=preflight` accepts `STATE`, `PAYLOAD_KIND`, `RUNNER`,
`ROM`, `RECORDED_RUNNER_SHA256`, `RECORDED_ROM_SHA256` and
`RECORDED_STATE_SHA256` as CMake inputs. Record the three digests when the
runner creates the state; the positive fixture test does that in one
invocation. These are CMake wrapper inputs, not runner options. The public
`runner.reproduction.preflight` test demonstrates that creation and check path
with a generated NMI ROM; the self-test sends each bad identity through the
same preflight mode without invoking replay.

Native format version 3 and behavior revision 9 are separate values. The
loader checks the native format, cartridge identity, size, checksum and field
contents, but it does not enforce the exact executing build or behavior
revision. A state may therefore pass native loading and still be outside the
documented exact-build/content support policy. Successful preflight does not
bypass those native loader checks. See the [save-state contract](api.md#save-states)
and [progress policy](progress.md#supported-snapshot-baseline).

## Scratch storage and bounds

Use one new scratch root and build directory per invocation. The POSIX command
uses `mktemp -d`; PowerShell uses a GUID. The CMake cold and preflight wrappers
create a fresh nonce directory for each run beneath the build tree. They keep
generated ROMs, states, movies and captured stdout/stderr there. Existing NMI
tests use `build/tests/state_work/<case>` and the CTest-managed
`runner.save.fixture` setup uses `build/tests/save_fixture`; their full build
directory must be unique to this invocation. They may recreate their own case
paths. Two separate CTest processes sharing one build tree are unsupported.

These examples do not read or write a host config, active save/state/autoload
directory or daily SRAM. Keep the newest independent battery backup
outside every active example path. Follow the [progress guide](progress.md)
for the backup procedure.
No example imports daily battery data. A state load can replace battery RAM,
so the native continuation fixture deliberately has no battery-backed content.

| Recipe or required setup | Frame bound | Child process bound | CTest outer bound |
| --- | --- | --- | --- |
| Cold NMOVIE1 replay | 600 movie frames per run; 2 cold runs | Generator 10 s; each runner 60 s | `runner.reproduction.cold`: 180 s |
| Checkpoint identity preflight | Fixture generator: 0 emulated frames; runner creates state at frame 1 | Generator 10 s; runner 20 s | `runner.reproduction.preflight`: 60 s |
| Preflight metadata controls | 0 emulated frames | Each negative-control CMake child 10 s | `runner.reproduction.preflight_self_test`: 60 s |
| NMI continuation | NMI ROM generation: 0 frames; save K=1; continue through absolute frame N=4 (frames 2–4) | Bounded by each CTest case | `runner.state.nmi_roundtrip`: 60 s |
| NMI idempotence | Final absolute frame 2 | Bounded by each CTest case | `runner.state.nmi_idempotent`: 60 s |
| NMI stale/omitted-load controls | At most absolute frame 3 | Bounded by each CTest case | `runner.state.nmi_wrong_load`: 60 s |
| NMI tests' battery-fixture setup | 0 emulated frames | N/A | `runner.save.fixture`: 10 s |
| Played-input controls | Five fixed 600-frame runs | Each movie writer 10 s | `runner.boot_to_play`: 300 s; each `runner.nesrunner_movie.*`: 10 s |

The CTest bounds are generated properties, not estimates. The contract check
compares them with CTest's actual metadata, including required/setup fixtures.
Every required runner must exit successfully and produce its complete required
output. A timeout, stop line, interruption, missing state/output or partial
frame stream is incomplete, even when a child returned exit 0. Keep interrupted
scratch locally for inspection; do not reuse it as input. Start the next attempt
with a new root, and clean up only the root created by that invocation.

## Procedure result and runner exit status

| Result | Meaning |
| --- | --- |
| `reproduction unsupported: ...` | The example's identity/origin preflight stopped before replay. It is a procedure result, not a runner exit code. |
| Runner exit 0 | The runner completed; the example still requires all expected output before reporting a match. |
| Runner exit 1 | Read, execution or output failure. |
| Runner exit 2 | Usage or unsupported option combination, including `--movie` with `--load-state`. |
| Runner exit 4 | Battery `.sav` size mismatch. |
| Runner exit 5 | Native state rejected by the loader. |
| Runner exits 3 and 77 | Reserved; never returned. |

Host movies, host recording and foreign movie conversion are unsupported. A
runner-native state cannot be renamed into a RetroArch disk state. See the
[runner exit status contract](runner.md#exit-status) and the native
[save-state contract](api.md#save-states).

## Choose the next evidence source

Start with the exact scene, content, build, host route and ordered inputs. Use
the first comparison that can answer the report's question; this is a decision
guide, not a requirement to run every layer or start a host session.

| Comparison | Supported inference | Remaining unknown | Next bounded evidence source |
| --- | --- | --- | --- |
| Scene, access route, content/build identity and inputs | Confirms whether the report describes the same identified setup and whether the exact scene was reached. A missing or failed access route remains a title-level observation, not scene acceptance. | Whether an unverified content/build identity or an unreached scene matches the reported behavior. Expected behavior can also remain unknown when its source or confidence is unknown. | Record the actual loaded core/build, content revision or digest, exact scene, verified access route and ordered port inputs. Use a documented scene source or one already verified access method; keep unavailable facts explicit. |
| Repeated native frame, transition and PCM windows | Matching hashes over the same inputs and frame/audio boundaries establish repeatability for those native outputs on the measured build and platform. The cold route compares frames 1–600; the checkpoint route compares the suffix at frames 2–4. | Whether the repeated output is correct, whether another platform agrees, and whether an adapter or frontend changed what was presented. | Use `runner.reproduction.cold` for two cold 600-frame executions or `runner.state.nmi_roundtrip` for its neutral K=1 through N=4 suffix. Keep `runner.boot_to_play` and the stale/omitted-load guest-marker checks as separate input and continuation controls. |
| Native pixels versus adapter RGB/callback output | `host.convert` covers palette indexing and source/output pitch. `libretro.host` compares selected video and PCM callback results with direct native output in a test host. | Whether a frontend delivered the intended physical input, accepted and presented every callback, or handled device audio and pacing correctly. These tests do not provide a display or audio device. | Route a native-to-RGB difference to `host.convert`; route a callback or PCM difference to `libretro.host` and its direct-core comparison. Keep each result scoped to the representation and callback tested. |
| Frontend input, presentation, screenshot or audio timing | A frame-aligned host capture can compare host RGB with a native/adapter frame when capture timing, conversion, palette, scaling, shader, crop and encoding are known. The existing hosted comparison is limited to its identified module, content and frame. | Input delivery, dropped or doubled presentation, pacing, resampling, synchronization and physical devices remain frontend/host questions. Unaligned screenshots are qualitative and cannot be equated to native hashes. | Use an existing hosted comparison only when it covers the same supported fixture and frame. For a host-specific symptom, retain the exact host/frontend/build, input route, capture path and relevant settings, then take one bounded comparison in a separately authorized host run. Libretro describes the frontend's role in [core callbacks and host details](https://docs.libretro.com/development/cores/developing-cores/); RetroArch documents [video/audio synchronization and pacing](https://docs.libretro.com/guides/optimal-vsync/). |
| Expected original-game behavior or hardware correctness | A claim-specific hardware document, identified hardware measurement, or independently derived owned-fixture expectation with a deliberately wrong behavior control can support a correctness claim for that behavior. A released reference-emulator binary with its version identified is comparative evidence. | Whether a source applies to this exact content revision, scene and setup; a reference emulator alone does not establish hardware authority. If no suitable oracle or reproducible access exists, the result remains unclassified. | Name the independent source and its scene/content/setup limits. Reduce confirmed interactions to an owned fixture with an independent expected result and a deliberately wrong control. Never inspect GPL/LGPL emulator source. |

## Evidence limits

**Repeatability** answers whether the same measured inputs and starting
conditions produce the same outputs. A matching frame, transition or PCM hash
does not show that the output is correct.

**Fixture expectations** establish only the properties their independent
predicate and negative controls check. A guest marker can show that a particular
input or continuation occurred; it is not a general hardware oracle.

**Hardware correctness** needs an independent source suited to the claim:
documented hardware behavior, identified hardware evidence, or an independently
derived owned fixture whose deliberately wrong behavior fails. Record the source,
content revision, exact scene and confidence. A released reference-emulator
version can be compared without inspecting its source, but its agreement is
still comparison evidence rather than proof of hardware behavior.

The dated [playtesting postmortem](../.planning/playtesting/POSTMORTEM.md#what-remains-unresolved)
retains the commercial observations and their limits. Jurassic Park's OCEAN
wobble and Crystalis's moving world seam have timing-sensitive counterparts;
Little Nemo and Mega Man 3 retain separate reported line/seam observations.
Shatterhand's drift, slowdown and music interruption remain inconclusive, and
the Marble Madness timer comparison remains uncertain. These reports are not
diagnoses or fixes. If exact access or an adequate independent oracle is
missing, keep the result unclassified.
