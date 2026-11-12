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

## D-51 optimized receipt validation

D-51 preserves the registered `provider.receipt.controls` test and its
15-second timeout. In `pr_validate_structure`, checksum name/digest entries
are validated once and retained as paired values. A nonempty prefix plus the
name's hexadecimal bytes forms a CMake list-safe lookup key, preserving the
original name, including malformed names and delimiters. Archive matching
then uses the paired digest without rereading the full receipt document inside
the archive loop. Asset, checksum and archive collection subtrees are read
after their existing type and count checks. No lookup survives the current
validation call.

The existing validator's explicit reason prefixes and diagnostic context,
validation order and rejection order are preserved. CMake-generated low-level
JSON error text may contain shorter paths because the validator now reads
extracted collection subtrees. Three additional controls require reversed
checksum order with 18 distinct name-bound digests to pass, swapped digests to
fail with `checksum-disagreement`, and a delimiter-plus-duplicate name
mutation to fail with `checksum-missing`. The route selftest checks the exact
D-51 base, branch, four-file source delta, evidence paths and route
discriminator against the shared receipt helpers; it does not launch a hosted
operation.

Implementation-time snapshot: the earlier candidate CI run had timed out this
test at its unchanged 15-second cap, and the optimization's runtime margin had
not yet been measured. That approved preparation scope allowed up to three
focused prefreeze validations and a separate full qualification. Later D-51
evidence records the fresh route and its terminal duplicate-run stop during
release CI; it was not retried and no D-51 release was published. The D-50
source, failed run and spent route remain historical and unchanged.

## D-52 PR title proof

The full CI workflow remains attached to source events. PR title edits run the
separate `PR title` metadata workflow, so a title change cannot cancel or
restart full source validation. The protected `main` branch requires the
`PR title` check.

For candidate and release PRs, the observer reads the live PR title and head,
then asks GitHub for the latest `PR title` check from the Actions app on that
head. It selects exactly one workflow run by the check suite ID, then binds its
job, check run, attempt and raw job log to the live title digest. This ignores
cancelled sibling workflow runs that have no current required check while
rejecting missing, ambiguous, pending, failed or stale current checks. The
observer repeats the current-check lookup after retaining the log. It records
the successful proof before each merge and validates that saved proof and log
afterward without replacing them from later PR metadata.

The ordinary published-package path accepts `-DREQUIRE_TITLE_PROOF=ON` to
require both linked PR proofs and their retained logs in release evidence and
the published receipt. `verify-published` uses the same flag to check that the
receipt preserves those exact records. The flag defaults off for historical
receipt fixtures and older read-only callers; the new normal release driver
passes it explicitly. The title proof adds no test registration or timeout;
its parser and malformed/stale/workflow/head cases run inside the existing
receipt-control registration and its unchanged 15-second cap.

The generic supervisor runs the actual registered `provider.receipt.controls`
CTest selection, checks that its timeout remains 15 seconds and retains its
absolute JUnit output. A full local proof still runs `cmake --workflow --preset
ci` with the configured full AccuracyCoin campaign, installed gameplay/profile
checks and package inventory. Static preparation or a direct helper invocation
cannot substitute for those registered tests.

Each attempt has a fresh owned output and immutable input/log/result records.
An ordinary local attempt honors its explicit acceptance Markdown path; its
handoff records publication and hosted checks as pending and tests the existing
missing-publication refusal. The generic supervisor replaces the old reset
wrapper's source/premerge/process/accounting assertions; using its child-entry
flag without those replacements does not qualify a source. See the bounded
limits and title-check rollout in [the release guide](releases.md#normal-bounded-verification-and-title-rollout).

Implementation-time snapshot: verification was pending when this change was
prepared. The dated phase records identify the actual tested source and outcome;
this description alone establishes no qualification, release or consumer
acceptance.


The registered receipt controls also exercise the complete title-proof function
through closed API fixtures at array indexes zero and one, with separate run,
job and check identities. They reject newer failed or pending runs, stale suites
or logs, a changed final live title and a replacement run arriving during proof.
The fixture transport is installed only inside the selftest control and restored
before it returns; ordinary title and hosted verification retain the live API
transport. Title-job output is captured directly to a file; file hashing and
retention preserve its bytes, while text reads serve only parsing. Complete proof
fixtures check retained LF and CRLF logs and literal template-like title text
against independent expected digests. Existing checkout-log capture retains its
separate stripped-text contract. These structural fixtures do not claim current
GitHub qualification.
