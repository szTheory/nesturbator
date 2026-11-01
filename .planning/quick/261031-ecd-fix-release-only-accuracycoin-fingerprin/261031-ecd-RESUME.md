# CI release fix handoff

The 261031-ecd implementation and release are complete. Read `AGENTS.md`,
`.planning/STATE.md`, the quick task plan and summary before continuing.

Release PR #56 passed required CI at head
`6bd7f267345f3bc153fc08f301a9bac166ed5812` and was auto-squash-merged as
`2338608ab184f040620a30d17756f5e06a42d4e3`. The PR required-CI run
`41211519919`, post-merge Release workflow `41212164789`, ordinary main-push CI
`41212164724`, and Nightly `41212164805` all succeeded. All AccuracyCoin
campaigns passed. The Release workflow verified all 18 platform archives and
their attestations, rejected a modified archive, and published
[v0.1.12](https://github.com/szTheory/nesturbator/releases/tag/v0.1.12) with
18 archives and `SHA256SUMS`.

CI efficiency finding: Release workflow reusable CI and the ordinary `main`
push CI both ran on the same merge commit, duplicating the full AccuracyCoin
campaign. The reusable Release CI is needed for release validation and build
artifacts. The ordinary main-push CI is the low-hanging optimization target;
preserve the Release workflow and its publish gate. Main-push Nightly lanes are
separate optional coverage and were left enabled.

Next command: `$gsd-quick --validate Skip the redundant main-push CI run for Release Please merge commits while preserving the reusable release CI and publication gate.`
It plans, implements, and locally validates that workflow change. Context may
be cleared before running it. The prior full-campaign authorization covered the
release run; obtain a fresh owner decision before a push starts another
campaign. The agent continues with local planning and implementation next.

Existing local edits to `AGENTS.md`, `.planning/config.json`, and
`.planning/continue.md` were not modified.
