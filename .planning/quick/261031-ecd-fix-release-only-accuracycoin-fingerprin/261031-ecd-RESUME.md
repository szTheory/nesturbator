# CI release fix handoff

Read `AGENTS.md`, `.planning/STATE.md`, this file, and the quick task plan and
summary before resuming.

The local quick task is implemented in commits `6c928f9`, `8573742`, and
`9470359`. Targeted fingerprint and Nightly workflow policy checks passed. The
required six-platform CI definition is unchanged. Release Please PRs skip the
optional Nightly lanes; ordinary relevant PRs and scheduled, manual, and main
push runs retain those lanes.

The current phase branch has extra `tests/CMakeLists.txt` registrations, so its
metadata hash is not suitable for Release PR #56. On a clean snapshot of the
PR's exact base `45a078d`, applying the code and workflow patches produced
`build_inputs_sha256=78822adfa35ac2b0e2f562cfb0067ae08db7e76c3e6e4027fa63d7bcca400de2`
with 56 inputs; the fingerprint control passed against that metadata. Use this
value when preparing the fix on `main`.

The owner authorized one full CI run on 2026-10-31. Release PR #56 now points
to `6bd7f267345f3bc153fc08f301a9bac166ed5812`; CI run `41211519919` is in
progress and includes the 22-page AccuracyCoin campaign (CTest timeout 370
seconds). The Nightly PR run skipped its optional lanes as intended. The prior
campaign result remains unknown. Do not rerun the full campaign if this run
fails without another owner decision. Squash merge and release publication
remain authorized after required checks pass on the exact release head and
post-merge release CI succeeds.

Next command: `$gsd-quick resume fix-release-only-accuracycoin-fingerprin`.
It resumes monitoring this run and continues to squash merge and release when
the required checks pass. Context may be cleared before running it.
