# Quick task 261031-fq6 handoff

The local implementation is complete in commits `00eedce`, `3fd0d9f` and
`fe376da`. It skips direct CI jobs only for generated Release Please pushes to
`main`. Release's reusable CI and publication checks remain enabled, as does
main-push Nightly.

`cmake --preset hygiene` and the focused `hygiene.ci_release_push_policy`,
`hygiene.ci_release_push_policy.selftest`, and `hygiene.ci_required_rollup`
tests passed. The verifier marked all five local must-haves verified and set
`human_needed` only for observing actual hosted execution and publication; see
[the verification report](261031-fq6-VERIFICATION.md).

No full CI, AccuracyCoin campaign, push, dispatch, merge, or release action
was run. The owner authorized one push and hosted CI run, but preflight found
no remote ref or open PR for the current phase branch. CI runs on PRs, pushes
to `main`, manual dispatch, and reusable calls; a non-main branch push alone
does not trigger it. The current branch also contains broad phase history.

Do not push it or create a broad PR. The recommended route is a clean branch
from latest `main` containing only this task's implementation commits, followed
by a draft PR to start one CI run. That run includes AccuracyCoin, does not
authorize merge or release, and does not exercise the release-merge skip event.
Get the owner's explicit approval for that additional branch/PR route before
proceeding.

Next OpenGSD command: `$gsd-quick status 261031-fq6`
It displays the saved quick-task status without starting a hosted run. Context
may be cleared before running it. Read this handoff and STATE.md before
resuming remote work.
