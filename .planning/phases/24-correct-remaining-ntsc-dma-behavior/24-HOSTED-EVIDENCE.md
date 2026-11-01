# Phase 24 hosted evidence status

## Current result

No hosted run, push, pull request, merge, or release has been performed for
this gap closure. Synthetic local controls prove rejection behavior only.
The previous full local campaign remains historical evidence; it used the
checkout `scoreboard-main` fallback and does not establish cold protected-main
authority.

The next step is to prepare a read-only request bound to the committed
Plan24-05 candidate and inspect current PR metadata. A prepared request does
not grant remote authority. The blocking decision and any later permission
must be recorded in `24-ACCEPTANCE.md`.

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

No such hosted receipt exists yet. Do not create an empty delivery record or
claim a verdict from local/synthetic controls.
