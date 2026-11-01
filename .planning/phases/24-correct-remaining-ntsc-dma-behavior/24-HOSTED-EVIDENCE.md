# Phase 24 hosted evidence status

## Current result

The original push command returned without a final status after 30 seconds.
After its process ended, read-only checks found no requested branch, PR, or
hosted run. The original request was bound to pre-rewrite candidate
`e2e24a6709aab6332267f414101fb7c2c05bb8bc` and digest
`c772cfb2a8e0d46dbdbc58fb54623fc06be15b5724060fa2b93da0b03a77fe2e`. Its
pre-push history hygiene check passed; the format check failed on the
unformatted blobs in pre-rewrite commit
`7348a10c68e592b71ec1899121003bf62d6bc813`. Full output is retained in ignored
`build/phase24-gap/push-format-diagnostic.log`.

On 2026-11-01T11:17:43Z, the owner authorized local history repair and
replacement request preparation only. The format-only rewrite produced commit
`6503f3551899a5d607d283a41a00451a7090f4de`; the final code/data tree before
documentation-reference updates matched the original candidate tree. The
repository's outgoing format check now passes. The first replacement request
in ignored `build/phase24-gap-replacement/` failed local `MODE=request`
validation because its proposed PR command used the old default body path;
that root is preserved. The generator fix is committed as
`f76d4492c45048a8541e14a4fda6fefe669ea82a`. Preparation at
`build/phase24-gap-replacement-v2/` stopped before generating request files
because the sandbox denied Git's temporary `.git/index.lock`; that root is
preserved. The request at `build/phase24-gap-replacement-v3/` passed
`MODE=request` validation for candidate
`f76d4492c45048a8541e14a4fda6fefe669ea82a` and tree
`7a039910a2c0fe696da2fd0d2807eeeb7d63d5ef`; its JSON kept
`remote_action_authorized=false`. Final state/evidence bookkeeping changed
after that validation, so the final request is under
`build/phase24-gap-replacement-v4/`. The prior push authorization does not
apply to the replacement candidate. No remote branch, PR, hosted run, or CI
result exists.

The previous full local campaign remains historical evidence; it used the
checkout `scoreboard-main` fallback and does not establish cold protected-main
authority. Synthetic local controls prove rejection behavior only. There are
no CI results yet to inspect for low-effort improvements. The replacement
candidate requires a separate owner decision before any push or PR; no merge
or release is included.

## Evidence still required

1. An independently fetched cold protected-main scoreboard and fresh passing
   candidate observations for every initial protected PASS key.
2. A successful current-candidate CI pull-request run, including the configured
   Linux x64 full AccuracyCoin campaign.
3. Identical complete frame, PCM, transition, and native hash inventories from
   Linux, macOS, and Windows on x64 and arm64.
4. Real base_a/base_b/head Cachegrind measurements for each existing
   performance ROM at 60 and 360 frames, passing the unchanged `+1%` gate.

## Offline receipt contract

After an explicitly authorized PR run, `MODE=validate` consumes a local
receipt directory created from read-only GitHub API responses and the
immutable Actions artifact downloads. It requires run, job, artifact, PR,
candidate, tested-merge, base, and independent protected-main commit/tree
records; raw build job logs; artifact IDs and SHA-256 digests; and the actual
downloaded evidence files. The prepared request, source workflow, base
scoreboard blob, artifact expiry, all six platform inventories, full campaign,
and raw performance measurements are checked offline before a verdict is
written. Missing, stale, duplicate, expired, or mismatched evidence fails
closed.

The receipt root is `build/phase24-gap`. Preserve the raw API JSON and raw
per-job logs under `provenance/` and `downloads/`; do not hand-author their
contents. `provenance/artifact-map.tsv` has one row per selected immutable
artifact, in this order: `name|id|sha256:<api-digest>|created_at|expires_at`.
Store each exact downloaded ZIP as
`provenance/artifact-zips/<id>.zip`. `provenance/artifact-files.tsv` has one
row per ZIP member, in this order: `id|archive-member-path|normalized-path|sha256`.
The validator extracts every ZIP into a fresh ignored `archive-check/`
directory, checks that the member map covers the exact archive contents, and
checks each member against both its digest and its normalized copy under
`artifacts/`. It removes that scratch directory after a successful validation.
Preserve a failed receipt root and use a new root for any later authorized
attempt.

Normalize artifact files under these prefixes so each file is attributed to
the immutable artifact that owns it: `hashes-<platform>/` for each of the six
hash artifacts; `build-<platform>/` for each corresponding
`accuracycoin-evidence-<os>-<arch>-<run>-<attempt>` artifact;
`perf/` for the selected performance artifact; `retroarch/` for marker
progression; `retroarch-e2e-frames/` for the frame artifact; and
`vector-inventory/` for the registration inventory. Preserve every file from
each ZIP exactly once and record its original ZIP member path in the file map.
The prepared candidate's `tests/accuracy/scoreboard.txt` blob is separately
saved as `provenance/candidate-scoreboard.txt` from the independently
retrieved candidate Git tree; it is checked against the current candidate
checkout and is not represented as a workflow artifact. The protected-main
scoreboard is saved as `provenance/protected-main-scoreboard.txt` from the
independently retrieved base tree. Keep raw downloaded files and metadata
separate from those normalized evidence files.

No hosted receipt exists yet. Do not create an empty delivery record or claim
a verdict from local/synthetic controls.

The first standard-depth review found three evidence/test-diagnostic issues;
commit `dfd0bfc` fixed them, and the final re-review is clean. `bus.unit`, the
synthetic evidence controls (including symlink rejection), and the optional
`ROM_ABORT_TRACE_CPU_PC` build passed. This commit invalidated request v4.
Prepare and validate a new candidate-bound request after the review and state
artifacts are committed. It grants no remote authority.

The 2026-11-01 post-execution goal verification remains `gaps_found` at 8/9.
The named `bus.unit` check and local rejection controls passed, but no new
hosted or full-campaign evidence was produced. Independent cold-main authority,
current-candidate hosted CI/campaign results, six-platform hash equality, and
raw Cachegrind base/head evidence remain absent. See
[`24-VERIFICATION.md`](24-VERIFICATION.md); no hosted verdict is inferred.
