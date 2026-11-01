# Phase 24 resume — Plan 24-05 replacement-candidate checkpoint

Read `.planning/STATE.md` and [24-ACCEPTANCE.md](24-ACCEPTANCE.md) first.
Plans 24-01 through 24-04 are complete. Plan 24-05 Task 1 fixed WR-01 and
passed its focused build, `bus.unit`, landing trace, and offline rejection
controls. The 2026-11-01 full local campaign remains historical evidence and
is not cold protected-main authority.

## Original remote attempt

The original prepared request was bound to candidate
`e2e24a6709aab6332267f414101fb7c2c05bb8bc`, tree
`1f8d0b89a784d29598e072aaad1c36b950717119`, request digest
`c772cfb2a8e0d46dbdbc58fb54623fc06be15b5724060fa2b93da0b03a77fe2e`, and
branch `gsd/phase-24-correct-remaining-ntsc-dma-behavior`. Its authorized push
returned without final status after 30 seconds. The target branch and PR were
absent afterward, and no CI run exists. Local hook diagnostics showed the
history hygiene scan passed but the format scan rejected unformatted blobs in
pre-rewrite commit `7348a10c68e592b71ec1899121003bf62d6bc813` for `src/bus.c`
and `tests/cpu/test_bus.c`. The original diagnostic is retained at
`build/phase24-gap/push-format-diagnostic.log`.

## Authorized local repair and replacement request

On 2026-11-01T11:17:43Z, the owner authorized local history repair and
preparation of a replacement candidate/request only. The outgoing branch had
120 commits from common ancestor `45a078d2de1f08e39caf1c5942fcdb57f493d8af`
and no merge commits. Replaying from that ancestor preserved the final
candidate code/data tree. The unformatted commit was rewritten as
`6503f3551899a5d607d283a41a00451a7090f4de`; clang-format 18 changed only
whitespace in the two source files, verified by matching whitespace-stripped
token streams. Two later replay conflicts were resolved using the exact
original commit snapshots, which already passed formatting. Twenty-nine
commit IDs changed; the preceding 91 stayed the same. A documentation commit
updated 21 references across eight committed files. The outgoing history
format check now passes.

The first replacement request under `build/phase24-gap-replacement/` failed
`MODE=request` validation because its proposed PR command used the old default
body path; its root is preserved. Commit
`f76d4492c45048a8541e14a4fda6fefe669ea82a` fixes the generator to use the
selected request root's body path. Preparation at
`build/phase24-gap-replacement-v2/` stopped before generating request files
because the sandbox denied Git's temporary `.git/index.lock`; that root is
preserved. The request under `build/phase24-gap-replacement-v3/` passed local
`MODE=request` validation for the current candidate and kept
`remote_action_authorized=false`. Final state/evidence bookkeeping changed
after that validation, so the final request is under
`build/phase24-gap-replacement-v4/`; `request.sha256` records the digest. On
2026-11-01 at 12:06 UTC, the final v4 request passed `MODE=request` validation
against its candidate/index/worktree snapshot and current read-only GitHub
main/branch/PR metadata. No remote action, test suite, workflow, or hosted CI run has occurred
after the rewrite. The prior push authorization is bound to the old candidate
and does not apply to the replacement.

## Next action

The next actor is the owner: review the exact candidate/request and decide
whether to authorize its normal push, new PR, and one existing configured CI
run under the previously prepared limits. If CI runs, inspect its results
read-only for low-effort improvements. Do not merge, release, manually
dispatch, or retry a failed/interrupted run without new authorization.

The phase initializer reports zero incomplete plans because Plan 24-05 already
has a summary. Do not expect another executor dispatch from the same command.
The first standard-depth code review found three issues. Commit `dfd0bfc`
fixed the symlinked ROOT escape, missing CPU-read dereference, and optional
CPU-PC compile failure; `bus.unit`, the evidence controls, the optional
CPU-PC build, and a clean re-review passed. The refreshed goal verification
remains `gaps_found` at 8/9; the remaining DMA-03 evidence is detailed in
`24-VERIFICATION.md`. This invalidated request v4.

The next action is to prepare and validate a new candidate-bound request after
the review and state artifacts are committed. It must keep
`remote_action_authorized=false`. The owner then reviews that exact request and
decides whether to authorize its push, new PR, and one configured CI run under
the recorded limits.

Next command after v5 passes local validation: `$gsd-progress`. It refreshes
GSD routing so the owner can review the concrete request. Context may be
cleared at this handoff; read STATE, this resume file, and the acceptance
record before resuming.
