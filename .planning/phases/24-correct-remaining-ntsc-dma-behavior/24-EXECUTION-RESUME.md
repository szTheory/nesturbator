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

The replacement request is prepared under `build/phase24-gap-replacement/`.
Its request JSON carries the exact new head/tree and keeps
`remote_action_authorized=false`. No remote action, test suite, workflow, or
hosted CI run has occurred after the rewrite. The prior push authorization is
bound to the old candidate and does not apply to this one.

## Next action

The next actor is the owner: review the exact candidate/request and decide
whether to authorize its normal push, new PR, and one existing configured CI
run under the previously prepared limits. If CI runs, inspect its results
read-only for low-effort improvements. Do not merge, release, manually
dispatch, or retry a failed/interrupted run without new authorization.

Next command: `$gsd-execute-phase 24 --gaps-only`. Context may be cleared at
this handoff; read STATE, this resume file, and the acceptance record before
resuming.
