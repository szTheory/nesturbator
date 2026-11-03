---
phase: 24-correct-remaining-ntsc-dma-behavior
plan: "05"
subsystem: testing
tags: [c, cmake, accuracycoin, ntsc-dma, hosted-evidence]
requirements-completed: [DMA-02, DMA-03]
requirements-pending: []
completed: 2026-11-01
status: complete
---

# Plan 24-05 summary — replacement-candidate preparation

## Final execution outcome — 2026-11-01T18:51:21Z

Plan 24-05 is complete. The current PR #63 candidate `d54e21898fbf7bb3984fd2d33aa563932b464581` passed the full
local CI workflow (707 tests and all three packages), hosted CI run
[41336227793](https://github.com/szTheory/nesturbator/actions/runs/41336227793),
and Nightly run
[41336227847](https://github.com/szTheory/nesturbator/actions/runs/41336227847).
Receipt v16 at `build/phase24-gap-v16/` passed offline validation for all four
DMA-03 evidence classes. See [hosted evidence](24-HOSTED-EVIDENCE.md).

The old 8/9 verification report has not been edited. Formal phase verification
is the next separate step; no phase or requirement completion is claimed yet.
PR #63 remains open, with no merge or release.


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

## Historical original request and push attempt

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

## Historical authorized history repair

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
eight files. The first replacement request at
`build/phase24-gap-replacement/` failed `MODE=request` validation because its
proposed PR command used the old default body path; the failed root is
preserved. The committed generator fix
`f76d4492c45048a8541e14a4fda6fefe669ea82a` uses the selected request root's
body path. Preparation at `build/phase24-gap-replacement-v2/` stopped before
generating request files because the sandbox denied creation of Git's temporary
index lock; that root is preserved. The request at
`build/phase24-gap-replacement-v3/` passed local `MODE=request` validation for
candidate `f76d4492c45048a8541e14a4fda6fefe669ea82a` and tree
`7a039910a2c0fe696da2fd0d2807eeeb7d63d5ef`, with remote authorization false.
Final state/evidence bookkeeping changed after that validation, so the final
request is at `build/phase24-gap-replacement-v4/`. Its `request.sha256` records
the final digest; its JSON keeps `remote_action_authorized=false`. No behavior change, tests, workflow, or
hosted CI run occurred as part of this tooling repair. The original push grant
does not apply to this candidate. The next actor is the owner, who reviews the
exact request and decides whether to authorize its push, new PR, and one
existing configured CI run. If CI runs, review its results read-only for
low-effort improvements before any new code change or rerun. Phase 24 and
DMA-03 remain incomplete.

## Historical review follow-up

The first standard-depth code review found three evidence/test-diagnostic
issues: symlink traversal in the evidence `ROOT`, a nullable CPU-read trace
dereference, and an optional CPU-PC trace compile failure. Commit `dfd0bfc`
fixed all three and added synthetic controls for the symlink boundary and
missing-read diagnostic. `bus.unit` passed with `ROM_ABORT_TRACE_CPU_PC`
enabled, the evidence controls passed, the changed C file passed clang-format
18, and the final standard-depth re-review is clean.

This commit changed the candidate after request v4, so v4 is stale. The next
candidate-bound request must be prepared after the final review and state docs
are committed. Remote authorization remains false; no push, PR, workflow, or
CI run is authorized for that new candidate.

## Goal re-verification status at plan execution

The previous report remains `gaps_found` at 8/9 and is preserved as historical
state. Receipt v16 now supplies the missing evidence for formal re-verification.
At the time this plan summary was written, formal phase verification was still
the next step. The later verification is recorded in
[24-VERIFICATION.md](24-VERIFICATION.md): it passes 9/9 truths, closes DMA-03
with the exact-candidate protected-main, six-platform hash and performance
evidence, and verifies all four phase requirements. The exact candidate and
hosted runs remain those named above; no new campaign, hosted run, merge or
release was started by phase re-verification.
