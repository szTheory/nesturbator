# Package qualification checks

`install.consumer` exercises the installed public C target. `install.gameplay`
builds a fresh consumer, moves its library prefix into a path with spaces,
discards the initial build, and checks include/library resolution and actual
compile/link provenance from the moved prefix. It runs the pinned
[DABG scenario](nrom-gameplay.md), completed no-input/Start-only controls,
refused-load twin continuation and buffer/frame-input checks.

`provider.profile` compares the machine-readable profile with actual public
identity, source arithmetic and source-mapped scenario/ROM digests.
`provider.profile.controls` rejects altered identity, unsupported region
fields, JSON type/array/recipe errors and runtime-report mismatches. Synthetic
reports test rejection only; the actual relocated consumer supplies runtime
evidence. An explicit contract invocation can read that fresh report.

The existing ci workflow includes these registrations without changing
production flags, campaign inputs or comparator thresholds. The package-only
reset has one full local workflow, one exact-candidate required CI and one
resulting-main CI; a failed acceptance gate is terminal without retry.
Publication requires separate live release and downloaded-artifact proof.
No current success restarts the older stopped acceptance campaign.

## D-44 source and receipt controls

The historical AccuracyCoin baseline remains d4802809ff41 and is still
identified as dirty. The D-44 binding carries the pinned 56-path historical
input map and checks current checker hashes separately; this does not
independently authenticate the historical dirty tree or establish hardware
behavior. Compatibility remains ABI 1, behavior revision 9, state format 3
and ordinary-loader NTSC.

The focused input-fingerprint, activation and receipt controls run with
`NESTURBATOR_ACCURACYCOIN_FULL=OFF`; they do not invoke local acceptance or the
actual AccuracyCoin campaign. The ignored route driver is private to its
prepared worktree and is not shipped or available in a fresh checkout. Its
synthetic controls are capped at 180 seconds, saved-evidence inspection at 60
seconds, hosted intent admission at 180 seconds and each other one-shot
operation at 1800 seconds including cleanup and evidence persistence. Local
acceptance also retains its 1500-second workflow and 1700-second observer caps.
A failed one-shot stage remains spent; inspect its saved evidence instead of
rerunning it.

## Persisted-byte and receipt control checks

Historical-manifest mutation controls write explicit LF and CRLF fixtures, hash
the bytes on disk with `file(SHA256)`, and require the same pinned historical
aggregate rejection for each. They do not normalize the production fingerprint
or change the binding, baseline, result or registration bytes.

Receipt fixtures keep all seven protected-path rejection commits. Restored
files are staged with the next mutation, with one final restoration commit
before the registration checks. This removes 12 Git add/commit launches and
six synthetic commits without removing an assertion or raising the 15-second
receipt-control cap. A local macOS arm64 prefreeze comparison measured receipt
controls at 4.50 seconds before and 3.12 seconds after this fix. These local
measurements do not establish hosted Intel Mac performance.

D-45 uses a separate prepared checkout and ignored driver. Prefreeze controls
keep the full AccuracyCoin campaign off. Its single later local qualification
includes the configured campaign; saved-evidence inspection never reruns it.
The driver enforces the 15-second receipt-control subprocess cap, 180-second
synthetic cap, 60-second inspection cap and the existing qualification limits.
No ignored driver is shipped or available from a fresh clone.

## Offline harness work and diagnostics

`ci.session_inventory` asks real CTest discovery for the existing session-name
prefix, then checks the exact full/short registration and real label exclusion.
It retains the prefix filter for automatically included fixture tests and
handles an empty selection. Unrelated test names need not be parsed.

Each offline Phase20/Phase21 control invocation reads its unchanged runner/ROM
inventory once. Every rejection case still rebuilds its fixture, rereads the
scoreboard and recomputes source/evidence digests. Inventory is not reused
across invocations or contracts. Changing the runner or ROM within an
invocation would require a fresh inventory; validator results are never cached.

Receipt selftests print timestamped section markers to locate selector, reset,
scratch, ancestry, receipt/rights, hosted-control and final-usage costs in saved
verbose output. These are wall timestamps, affected by `SOURCE_DATE_EPOCH`,
not timeout enforcement. Test caps, live receipt modes and rejection predicates
remain unchanged. Focused offline results do not qualify a release or restart
a stopped acceptance/campaign run.

## Provider observer preflight

The provider receipt observer queries workflow runs with `ci.yml` or
`release.yml`. Returned records must still match the exact
`.github/workflows/ci.yml` or `.github/workflows/release.yml` path, expected
head, event, workflow name, attempt and required jobs. Selecting by filename
does not relax those identity checks.

The existing receipt selftest checks synthetic record validation; it does not
exercise the live query constructor. The local repair uses a disposable probe
that loads the exact source function definitions and calls the real helper
through the owned fake GitHub executable. A filename-only route table and an
isolated command path prevent real GitHub access and reject unknown requests.
This one-time probe adds no production loading mode, target or registration.

Before any new acceptance route, check every reader and writer against its
concrete draft evidence. Release-evidence JSON and the final published receipt
are separate artifacts. Initialize all intermediate objects with the right
types, including `candidate`, `main`, `release` and `linked_ci` for hosted
evidence, and check setters on disposable copies without changing unrelated
values. Mark drafts as synthetic and keep every real acceptance gate pending.
Check the complete case-sensitive narrative and exact route-command strings
before calling a handoff gate; a static preflight must not call the gate itself.

Offline checks neither qualify a release nor establish hosted runtime margin.
A repaired source needs its own separately approved qualification identity and
route; historical hardcoded routes and spent ledgers cannot be reused.

## Separate repaired-source qualification route

The D46 qualification route starts at the repaired observer source and uses
`d46-observer-v1` with its own exact base, branch, four-file metadata/documentation
delta and `build/phase28-d46` evidence. D44/D45 retain their original tuples and
terminal history. Registration, baseline, profile, scenario and public behavior
remain unchanged. The ignored local supervisor is not shipped.

Prefreeze uses one bounded offline session, full campaign OFF, the existing
receipt control once under its unchanged15second cap and disposable route
controls. A separate single source freeze/full local qualification includes its
fresh configured campaign. Exact candidate/main checks and normal new-head
release lineage precede actual published-byte/native/rights proof; no result is
claimed until those gates pass. Each actual failure, interruption or timeout
stops this route without retry or refreeze. Historical campaigns and ledgers
are never restarted. Local preparation establishes no hosted runtime margin or
consumer/host acceptance.
