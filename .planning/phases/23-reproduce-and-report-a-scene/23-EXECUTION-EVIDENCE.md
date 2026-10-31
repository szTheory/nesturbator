# Phase 23 Plan 03 execution evidence

Date: 2026-10-31
Scope: documentation and CMake/CTest contract checks only. No runtime source,
public API, runner status, installed target, host, or device behavior changed.

## Local build and fixture identity

The Plan 03 checks ran on macOS arm64 using the local `ci` build. At the time
the runner was rebuilt, the source checkout was `c865b79`; the runner, core,
header, and libretro runtime sources had no changes from the plan-start base.
The runner reported `nesturbator-run 0.1.11` and had binary SHA-256
`6552fe5f349ed44dcff831c7a1bd7a2ca53a1fdb1b4956c61abfad55f7af15fd`.

The permitted movie fixture was `tests/roms/runner.nes`, SHA-256
`1a67bfabdde2d96eacb04eb7f9d17e8da6c082d4976c318315fe31e2d19b4384`. The
generated `nesrunner-pos` NMOVIE1 movie and all raw runner output stayed in
build scratch; no media, movie, state, screenshot, or raw log was added to Git.

## Fixture route evidence

Cold replay evidence from Plan 23-01 is inherited, not newly measured by Plan
23-03. Two cold runs of the generated 600-frame movie matched on the local
macOS arm64 CI build:

| Output | Window | SHA-256 |
| --- | --- | --- |
| Ordered native frame/tick/hash records | Frames 1–600 | `a90cc60caeea1fc4e4f159e3341b5f1b446022281c018b6d609eebf702175d9b` |
| Audio transitions | Full cold run | `33e4bce709976dca002689e4cc164f694a677d9949fdfa325b7dd6cfa5cc4006` |
| Audio PCM | Full cold run | `255636674903d03dd9cb7ca632246c3375dff8c88300155b052ac5c273d4dcca` |

These hashes establish repeatability for those outputs on that build and
platform; they are not an independent correctness oracle.

The neutral native checkpoint route is separately evidenced by
`runner.state.nmi_roundtrip`: save at absolute frame K=1, continue through end
frame N=4, and compare the same audio suffix at frames 2–4. The route uses no
movie. The existing `runner.state.nmi_idempotent` byte comparison and
`runner.state.nmi_wrong_load` stale/omitted-load guest-marker controls passed.
Plan 23-01 also recorded `runner.boot_to_play` scoring-play evidence and its
no-input, attract-only, and Start-only controls. That test was not part of the
Plan 03 focused selection; its evidence remains in
[23-01-SUMMARY.md](23-01-SUMMARY.md).

`runner.reproduction.preflight` passed its same-invocation runner, ROM, and
state digest check. `runner.reproduction.preflight_self_test` accepted the
valid fixture and rejected seven independent bad-identity or host-container
cases before replay. Those `reproduction unsupported:` results are expected
procedure refusals, not runner exit codes or failed supported runs.

## Helper-gap evidence and preserved controls

Before Plan 23-01, `runner.dump.nesrunner.play` ran the movie once without an
outer timeout, while `runner.write_hashes` ran one movie without a child
process timeout or repeated audio comparison. `runner.state.nmi_roundtrip`
covered a different neutral checkpoint route. The scoped
`runner.reproduction.cold` CMake wrapper now runs the public runner twice,
requires all 600 ordered frame records and both named audio hashes, and checks
successful completion under generator, child, and CTest limits. It adds no
runner option, recorder, replay engine, runtime symbol, or dependency.

The documented fresh-scratch source build and its 18-test result are already
recorded in [23-02-SUMMARY.md](23-02-SUMMARY.md), so Plan 03 did not repeat that
route for bookkeeping. The current Plan 03 focused tests ran against the
existing `build/ci` tree.

## Plan 03 commands and results

All final commands below passed. The first CTest selection included the
`runner.save.fixture` setup case automatically.

```text
cmake --preset ci
PASS — configure and CTest registration generated.

cmake --build --preset ci --target nesturbator-run
PASS — runner target built.

ctest --test-dir build/ci --output-on-failure --no-tests=error -R '^(docs.reproduction|docs.reproduction.self_test|docs.links|docs.links.self_test)$'
PASS — 4/4 focused guide and link tests after Task 1.

ctest --test-dir build/ci --output-on-failure --no-tests=error -R '^(runner.reproduction.cold|runner.reproduction.compare_self_test|runner.reproduction.preflight|runner.reproduction.preflight_self_test|runner.state.nmi_roundtrip|runner.state.nmi_idempotent|runner.state.nmi_wrong_load|docs.reproduction|docs.reproduction.self_test)$'
PASS — 10/10 tests, including the save-fixture setup.

ctest --test-dir build/ci --output-on-failure --no-tests=error -R '^(state.status_table|state.status_table.self_test|runner.options.readme|runner.options.canonical.self_test|runner.exit.table|runner.exit.help|runner.exit.canonical.self_test|docs.links|docs.links.self_test|release.one_version_per_line|release.one_version_per_line.self_test|release.version_format|libretro.info_claims|libretro.info_claims.self_test|libretro.info_support|libretro.info_staged|libretro.info_archives.self_test|cmake.script_policy)$'
PASS — 18/18 inherited document, runner, status, release, package, and script-policy tests.
```

`docs.reproduction.self_test` retains its eight report, command, privacy,
storage, container and loader-policy mutations. It adds five independent
triage mutations: removing the native next-evidence branch, calling matching
hashes hardware correctness, treating screenshots as native hashes, assigning
host pacing to adapter parity, and labeling the unclassified commercial reports
fixed. The positive document check and all 13 mutations passed. Generated CTest
metadata confirms `docs.reproduction` and `docs.reproduction.self_test` each
retain their 60-second timeout.

The first authoring runs exposed three checker expectation problems, not guide
or runtime failures: one assertion crossed a source line break, one checked the
clean-room statement in the wrong guide section, and one expected a diagnostic
long enough for CMake to wrap. The assertions were scoped to their owning
sections and shortened; the final focused selections above passed.

## Requirements and decision mapping

| Requirement or decision | Implementation and evidence |
| --- | --- |
| REPRO-01 | Report fields, actual identity, exact-scene access and uncertainty are retained in the canonical form; README entry and document checks pass. |
| REPRO-02 | The cold movie and neutral checkpoint routes, same-window comparisons and independent input/continuation controls are separately identified in the guide and prior fixture evidence. |
| REPRO-03 | Scratch isolation, explicit limits, complete-output handling and identity/container preflight remain explicit and passed their focused checks. |
| REPRO-04 | The decision table and evidence limits separate native, adapter, host and original-game claims; the focused mutation controls pass. |
| D-01 | One maintained guide is linked from README, runner, testing and progress; link assertions pass. |
| D-02 | The form identifies content/build, exact scene/access, ordered inputs, expectation source, observation and conditions not tried. |
| D-03 | Unknown/unavailable values remain valid; identity mismatches stop at procedure-level preflight before replay. |
| D-04 | Public evidence uses allowed identities and digests; raw logs, saves, states, captures and personal paths remain excluded. |
| D-05 | Expected behavior includes source/confidence; dated commercial outcomes remain linked and unclassified. |
| D-06 | Cold movie and neutral native checkpoint demonstrations remain separate; movie-plus-load-state is unsupported. |
| D-07 | The permitted `runner.nes` fixture and owned generated NMI fixture are reused without committing generated binaries. |
| D-08 | Public source-build targets and documented CMake/CTest commands are used; the added helper stays within existing CMake facilities. |
| D-09 | Cold frames 1–600 and checkpoint audio frames 2–4 retain their matching windows and named output representations. |
| D-10 | `runner.boot_to_play` and NMI guest-marker controls remain separate from hash repeatability. |
| D-11 | NMOVIE1 validation, two-port masks and existing runner option/status semantics remain unchanged. |
| D-12 | Frame, child-process and CTest bounds plus complete-output rules remain documented and checked. |
| D-13 | Fresh scratch paths stay separate from daily battery and host directories. |
| D-14 | Exact-build/content support remains stricter than native loader validation; behavior revision is recorded, not enforced. |
| D-15 | Missing/mismatched identity and host-container inputs are refused by the example preflight without changing runner exit codes. |
| D-16 | The new table selects a next bounded evidence source at each measured layer. |
| D-17 | Adapter callback results remain limited to the test host; physical input and host presentation are separate. |
| D-18 | Screenshot alignment/transforms, pacing, resampling, synchronization and devices remain host-owned unknowns. |
| D-19 | Correctness claims require an independent, claim-specific oracle; released reference binaries remain comparative evidence. |
| D-20 | Existing CMake/CTest controls assert guide links, evidence limits, generated routes and false-claim mutations. |
| D-21 | This is documentation/test orchestration only; focused checks passed and no full workflow/runtime change was needed. |
| D-22 | No live host/device, commercial replay, collector campaign, private-content access, release, push or PR occurred. Existing stops and debt remain unchanged. |

## Evidence status and remaining limits

| Status | Result |
| --- | --- |
| Actual Plan 03 pass | Local macOS arm64 configure, runner build, 4-test Task 1 selection, final 10-test replay/document selection, and final 18-test inherited-contract selection passed. |
| Expected unsupported refusal | Preflight negative controls rejected missing, changed, unknown and host-container identities before replay; this does not expand runner exit semantics. |
| Unavailable platforms | Linux and Windows were not available in this local run and were not tested here. No cross-platform result is inferred. |
| Untried host/device route | No live RetroArch session, physical display/audio/input device, commercial replay, or private capture was run. Existing `host.convert`, `libretro.host` and hosted RetroArch coverage retain their documented scopes; Plan 03 did not rerun them. |

The native/adapter, screenshot and hardware distinctions are source-reviewed and
mechanically guarded by independent document mutations. The absence of a live
host/device run, private artifacts, and prohibited emulator-source inspection
is a session/source-review finding; CTest cannot prove those actions were not
performed elsewhere. No owner acceptance is inferred.

The five fallback edges remain unresolved: REPRO-01 idempotency and
concurrency, plus unclassified REPRO-02, REPRO-03 and REPRO-04. All eight
descriptor-less prohibition dispositions across Plans 23-01 through 23-03
remain flagged-unverified because their verification mode is judgment:

1. Do not publish commercial media, saves, checkpoints, raw private logs or personal identifiers.
2. Do not claim an exact scene or identity when access or identity is unavailable.
3. Do not claim movie-plus-state, host recording or foreign movie conversion as runner capabilities.
4. Do not treat repeated hashes as independent hardware correctness.
5. Do not touch daily progress or silently substitute an unknown, mismatched or host-container checkpoint.
6. Do not call timeout, interruption, truncation or missing output a successful replay.
7. Do not relabel commercial reports as diagnosed/fixed or equate adapter parity with host presentation.
8. Do not launch live hosts/devices, commercial replay, campaigns, publication or emulator-source inspection under this phase.

No issue in this list is converted into an automatic pass by the documentation
or mutation tests.
