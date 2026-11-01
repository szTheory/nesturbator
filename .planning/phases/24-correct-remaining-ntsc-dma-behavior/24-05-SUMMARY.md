# Plan 24-05 summary — replacement-candidate preparation

## Completed local gap work

- Fixed WR-01: retained DMA trace overflow now fails explicitly, and the
  landing capture retains the actual final `$FFC0` sample GET and resumed
  `$4000` CPU read at cycle 38,166 after the checked 30,012-cycle wait.
- `bus.unit` built and passed; the focused landing trace passed; synthetic
  controls passed for overflow, ZIP-bound artifact bytes, protected-key
  authority, CI-attempt identity, platform hash inventory, and raw performance.
- Corrected README local-versus-hosted acceptance wording and recorded the
  inherited prohibition flags as `human_judgment` / `unresolved`.
- Task 1's original commit `e2e24a6709aab6332267f414101fb7c2c05bb8bc` was
  rewritten as `9680ee8830ce95275d58158248c5f02f1da6b9fc`; pre-existing user
  workspace edits remain unstaged.

## Original request and push attempt

The original request was bound to pre-rewrite candidate
`e2e24a6709aab6332267f414101fb7c2c05bb8bc`, tree
`1f8d0b89a784d29598e072aaad1c36b950717119`, request digest
`c772cfb2a8e0d46dbdbc58fb54623fc06be15b5724060fa2b93da0b03a77fe2e`, and
branch `gsd/phase-24-correct-remaining-ntsc-dma-behavior`. The push command
returned without final status after 30 seconds. Read-only checks afterward
found no target branch or PR, so no CI run exists. Local diagnostics showed
the pre-push history hygiene scan passed but the format gate rejected
pre-rewrite commit `7348a10c68e592b71ec1899121003bf62d6bc813` for unformatted
`src/bus.c` and `tests/cpu/test_bus.c`; its output remains at
`build/phase24-gap/push-format-diagnostic.log`.

## Authorized history repair

On 2026-11-01T11:17:43Z, the owner authorized local history repair and a
replacement candidate/request only. The branch had 120 outgoing commits from
common ancestor `45a078d2de1f08e39caf1c5942fcdb57f493d8af` and no merge
commits. Replaying from that ancestor preserved the final code/data tree. The
unformatted commit was rewritten as
`6503f3551899a5d607d283a41a00451a7090f4de`; clang-format 18 changed only
whitespace in the two source files, verified by matching whitespace-stripped
token streams. Two later source-file replay conflicts were resolved with the
exact original commit snapshots, each already formatted. Twenty-nine commit
IDs changed; the preceding 91 stayed the same. The outgoing format check now
passes.

A local documentation commit refreshed 21 changed commit references across
eight files. No code behavior changed, and no tests, full workflow, or hosted
CI were rerun. The replacement request is prepared at
`build/phase24-gap-replacement/request.md`; it records the final head/tree and
leaves `remote_action_authorized=false`. The original push grant does not
apply to this candidate. The next actor is the owner, who reviews the exact
request and decides whether to authorize its push, new PR, and one existing
configured CI run. If CI runs, review its results read-only for low-effort
improvements before any new code change or rerun. Phase 24 and DMA-03 remain
incomplete.
