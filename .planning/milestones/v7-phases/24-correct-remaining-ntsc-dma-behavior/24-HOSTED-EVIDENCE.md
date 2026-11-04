# Phase 24 hosted evidence status

## Current candidate outcome — 2026-11-01T18:51:21Z

PR [#63](https://github.com/szTheory/nesturbator/pull/63) is open at candidate
`d54e21898fbf7bb3984fd2d33aa563932b464581`, based on `b25146c6efaac41c58f60555f59f0fe79a90ca68`.
Hosted CI run
[41336227793](https://github.com/szTheory/nesturbator/actions/runs/41336227793)
and Nightly run
[41336227847](https://github.com/szTheory/nesturbator/actions/runs/41336227847)
passed on that exact head. The push automatically triggered both configured
workflows; no separate dispatch or retry was made for this head.

The offline receipt `build/phase24-gap-v16/` passed validation. It contains
16 immutable artifacts and 2,005 archive-member records. The verdict is
`build/phase24-gap-v16/phase24-verdict.txt` (SHA-256
`ecf99e0feae27c6e8320ea66a3e62e276747b11c751d6d6b2d42d670cd5e2312`), bound
to candidate `d54e21898fbf7bb3984fd2d33aa563932b464581`, tested merge
`372cf8a33e439713d980d00d8352fdc0e8e2cdff`, base
`b25146c6efaac41c58f60555f59f0fe79a90ca68`, and CI run `41336227793` attempt 1.
The receipt validates all four required evidence classes below.

Local `cmake --workflow --preset ci` also passed all707 tests on the final
candidate and generated the library, runner and libretro packages. The old
8/9 goal report remains untouched; `$gsd-verify-work 24` is the next step.
PR #63 remains open. No merge or release occurred.


## Earlier one-time campaign on cec7b75

The owner authorized one attempt-1 Nightly campaign on the older `cec7b75`
head; run [41330431562](https://github.com/szTheory/nesturbator/actions/runs/41330431562)
passed. That evidence was superseded by the later d54e218 push and its exact-head
CI/Nightly runs documented above. No further manual campaign was dispatched.

## Historical cec7b75 run update — 2026-11-01T16:52Z

Candidate `cec7b759d1c71646c2dbc5b87307376535f3df0f` is the exact PR #63
head. CI run
[`41326931769`](https://github.com/szTheory/nesturbator/actions/runs/41326931769)
completed with all jobs passing, including macOS session-controls, RetroArch
E2E, and macOS arm64 and Intel builds. Nightly run
[`41326931762`](https://github.com/szTheory/nesturbator/actions/runs/41326931762)
completed with `session-controls` failed on macOS Intel. Its uploaded
`LastTest.log` reports `driver-trace-reported-write-error` in the ordinary
interruption-recorder case even though the three expected trace records are
present. The failing test files have no diff from `main`, and the same job
passed on the two preceding candidate runs. This is likely timing-related,
but its uploaded artifact does not include the recorder error file needed to
identify the exact write. The receipt collector requires Nightly attempt 1, so
a GitHub rerun of this failed run (attempt 2) would not validate. A new
workflow-dispatch campaign on the same candidate could supply the required
attempt-1 run, but needs separate authorization. Receipt v14 remains
preserved; do not collect a current-head receipt until the required
successful hosted evidence is available.

## Historical candidate attempts — 2026-11-01

Open PR [#63](https://github.com/szTheory/nesturbator/pull/63) targets main at
`b25146c6efaac41c58f60555f59f0fe79a90ca68`. Candidate
`50fa63997a532b364b5c4c2c42aea89ab35a4e6d` passed CI run
[`41317331942`](https://github.com/szTheory/nesturbator/actions/runs/41317331942)
and Nightly run
[`41317331847`](https://github.com/szTheory/nesturbator/actions/runs/41317331847),
but its Phase 24 receipt was not validated: the offline adapter had an
undefined SHA helper, assumed the run-scoped artifact API exposed an attempt
field, and did not reconcile derived workflow logs with exact archive-member
validation. No acceptance verdict was written for that candidate.

Candidate `ab5088a240f477e78ea53ffbe55522fbc83689ed` passed configured CI run
[`41319041243`](https://github.com/szTheory/nesturbator/actions/runs/41319041243)
and Nightly run
[`41319041228`](https://github.com/szTheory/nesturbator/actions/runs/41319041228).
Candidate `64951ca2cd9016fba28f9bc6392d4c3c5b150ca7` then passed CI
[`41321523233`](https://github.com/szTheory/nesturbator/actions/runs/41321523233)
and Nightly
[`41321523217`](https://github.com/szTheory/nesturbator/actions/runs/41321523217).
Its receipt v13 contained 16 immutable artifacts and 2,007 archive files,
but validation found that the source contract matcher treated the earlier
hygiene command as candidate CTest. Commit
`0690fa4ef9fc701b5e90a943e5e22d0d6d1c128e` fixed the matcher to select the
candidate build matrix command; `MODE=controls`, all 15 configured hygiene
checks, and request v14 passed. CI
[`41324273313`](https://github.com/szTheory/nesturbator/actions/runs/41324273313)
and Nightly
[`41324273322`](https://github.com/szTheory/nesturbator/actions/runs/41324273322)
both passed. Its receipt v14 contained 16 immutable artifacts and 2,005
archive files, but validation found that delimiter splitting broke the main
commit JSON at semicolons in its message. Commit
`cec7b759d1c71646c2dbc5b87307376535f3df0f` reads commit/tree identity objects
directly from their raw provenance files and adds a semicolon regression;
controls, hygiene, and request v15 passed. Its fast-forward push updated PR
#63. CI
[`41326931769`](https://github.com/szTheory/nesturbator/actions/runs/41326931769)
and Nightly
[`41326931762`](https://github.com/szTheory/nesturbator/actions/runs/41326931762)
were started on that exact head. CI completed successfully; Nightly failed its
macOS Intel session-controls job as documented above. Receipt v14 is
preserved; request v15 is ready, but current-head receipt collection awaits
run `41330431562`. One new campaign was manually dispatched after the owner
authorized it. No retry, merge, or release occurred.

The original candidates `841db4c` and `87ed3a7` failed only in
`docs.reproduction.self_test`: first at its existing 60-second limit, then at
the existing 20-second nested mutation limit. The final reproduction fix
caches generated test names and relevant fixture properties while retaining
full CTest JSON for exact target validation. Focused normal/ASan tests, full
local CI, and tree hygiene passed; no timeout was increased.

## Historical attempts

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

## Evidence validated for formal verification

1. **Protected-main authority:** The receipt binds the independently fetched cold-main scoreboard/tree and fresh passing observations for each required protected PASS key and the DMA neighbor controls.
2. **Hosted current-candidate evidence:** CI and the Nightly campaign passed on exact PR head d54e218, including the configured full campaign and required jobs.
3. **Cross-platform equality:** All six Linux/macOS/Windows x64/arm64 hash inventories are complete and byte-identical for frame, PCM, transition and native outputs.
4. **Performance:** Raw base_a/base_b/head Cachegrind results cover all existing ROMs at 60 and 360 frames, with zero base spread and the unchanged +1% gate passing.

These are available to `$gsd-verify-work 24`; they do not replace that formal step.

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

The validated current receipt is `build/phase24-gap-v16`; earlier failed/stale roots remain preserved separately. Preserve the raw API JSON and raw
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

Candidates `841db4c` and `87ed3a7` were superseded by `50fa639`. Their CI and
Nightly runs are terminal failures caused by the reproduction self-test
timeout/child timeout. Preserve those receipts as historical evidence; do not
rerun them. Do not create an empty delivery record or claim a hosted verdict
from local/synthetic controls. Retrieve and validate artifacts from current
CI run `41317331942` after it completes.

The first standard-depth review found three evidence/test-diagnostic issues;
commit `dfd0bfc` fixed them, and the final re-review is clean. `bus.unit`, the
synthetic evidence controls (including symlink rejection), and the optional
`ROM_ABORT_TRACE_CPU_PC` build passed. This commit invalidated request v4.
Prepare and validate a new candidate-bound request after the review and state
artifacts are committed. It grants no remote authority.

At the time this evidence log was prepared, the previous 2026-11-01 goal
verification remained `gaps_found` at 8/9; receipt v16 was then available for
formal re-verification.

## Formal phase verification — 2026-11-01

The phase report was re-verified against this exact-candidate receipt and the
current source/document tree. PR #63 remains open at `d54e21898fbf7bb3984fd2d33aa563932b464581`;
the evidence is not attributed to a later commit. A fresh read-only download
and independent check of the retained CI artifacts confirmed:

- All 119 protected PASS keys from protected main revision
  `b25146c6efaac41c58f60555f59f0fe79a90ca68` pass in each of the six platform
  lanes and remain passing in the candidate report.
- The six Linux/macOS/Windows x64/arm64 frame, PCM, transition and native hash
  inventories are byte-identical. Their shared SHA-256 is
  `cd90c2d7227ad306559c30caaaa934b3a6290709741bcf77919767a32c7266fe`.
- Eighteen raw performance captures cover three ROMs, base_a/base_b/head, and
  60/360 frames. All three comparisons pass within the unchanged +1% limit,
  measuring +0.18% to +0.25%.
- Exact-head CI run 41336227793 and Nightly run 41336227847 passed. The receipt
  remains bound to candidate `d54e218`, tested merge
  `372cf8a33e439713d980d00d8352fdc0e8e2cdff`, and base
  `b25146c6efaac41c58f60555f59f0fe79a90ca68`.

The updated [verification report](24-VERIFICATION.md) passes 9/9 truths and
DMA-01 through DMA-04. Fresh local follow-through was limited to named focused
state, bus, documentation and receipt-control checks. No full local workflow,
new hosted run, retry, or campaign was started. The older interrupted run's
unknown outcome and its restart stop are unchanged. The automation disposition
records why no manual UAT pass is claimed or needed for this infrastructure
phase.
