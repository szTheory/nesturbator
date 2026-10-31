---
quick_id: 261031-ecd
status: complete
implementation_commit: 9470359
verification_scope: focused local checks, exact-head required CI, gated release CI, and published release
date: 2026-10-31
---

# Fix release CI and trim redundant Nightly work

The shared AccuracyCoin input fingerprint now normalizes only the numeric
values of the three public release-version macros. All other public-header
bytes and every other fingerprinted input remain exact. A focused CTest
control verifies all three version changes leave the hash stable, an ABI macro
change alters it, and the stored hash/count match the current source tree.
The report digest and scoreboard were not changed.

The non-required Nightly lanes skip only Release Please bot PRs with the
standard release title. Required CI is unchanged; ordinary relevant PRs,
scheduled runs, manual runs and main pushes retain Nightly coverage. A main
push after release merge still runs the full Nightly workflow.

README and testing documentation describe the release identity and workflow
contracts. The local phase branch records `build_inputs_sha256=0534b85785ca6d51afca1747f7ea68ad636284eed7a4a6c918715f595fb2e751` for 56 inputs. That value is not for Release PR #56 because this branch has test-registration changes absent from its base. I applied the code and workflow patches to a clean source snapshot of the exact base `45a078d`, recomputed the fingerprint as `78822adfa35ac2b0e2f562cfb0067ae08db7e76c3e6e4027fa63d7bcca400de2` for 56 inputs, recorded that value in the snapshot's baseline metadata, and reran the focused fingerprint controls successfully. Use the `78822...` value when replaying the fix onto the release base; do not use the local phase-branch value.

## Verification

| Check | Result |
|---|---|
| `cmake --preset ci` | Configure passed; no campaign was run |
| `accuracycoin.input_fingerprint` | Pass; release-version neutrality, non-version sensitivity and metadata identity checked |
| `cmake --preset hygiene` | Configure passed |
| `hygiene.nightly_workflow_policy` and self-test | 2/2 passed; the live workflow guard and a missing-guard mutation were checked |
| `cmake --build --preset hygiene --target fuzz.execution` | Pass |
| `git diff --check` | Pass |

The owner authorized one exact-head full CI/campaign run for Release PR #56.
Run `41211519919` passed on head `6bd7f267345f3bc153fc08f301a9bac166ed5812`:
all 693 tests passed, including `accuracycoin.campaign`,
`accuracycoin.input_fingerprint`, and `accuracycoin.activation`. GitHub
auto-squash-merged the PR after all required checks passed.

The post-merge Release workflow (`41212164789`) passed its reusable full CI,
hash-equality gate, and publication job on merge commit
`2338608ab184f040620a30d17756f5e06a42d4e3`. Its AccuracyCoin campaign passed
693 tests in 337.24 seconds. The ordinary main-push CI (`41212164724`) and
Nightly (`41212164805`) also passed on that same commit; the main-push CI
repeated the campaign (693 tests in 193.51 seconds). The Release workflow
verified all 18 platform archives, generated and verified their attestations,
confirmed a tampered archive fails verification, and published release
[v0.1.12](https://github.com/szTheory/nesturbator/releases/tag/v0.1.12) with
18 archives and `SHA256SUMS`.

## CI Efficiency Follow-up

The Release workflow's reusable CI is necessary for exact-tag validation and
for attaching build artifacts to the release job. The separate `main` push
also started the ordinary CI workflow on the same merge commit and reran the
full AccuracyCoin campaign. This duplicate validation is a concrete
optimization candidate: skip only the ordinary `main` push CI on the generated
Release Please squash commit, while keeping the reusable Release CI and its
publish dependency. The main-push Nightly lanes are separate optional
coverage and should remain enabled absent a separate decision.

The next GSD task is `$gsd-quick --validate Skip the redundant main-push CI run for Release Please merge commits while preserving the reusable release CI and publication gate.`
The previous full-campaign authorization was for this release run; get a fresh
owner decision before pushing a workflow change that triggers another full
campaign. Existing local edits to `AGENTS.md`, `.planning/config.json`, and
`.planning/continue.md` were left untouched.

## Commits

- `6c928f9` — fix(ci): keep release version out of AccuracyCoin fingerprint
- `8573742` — perf(ci): skip Nightly lanes for release pull requests
- `9470359` — docs(ci): clarify release fingerprint and Nightly coverage

## Self-Check: PASSED

The summary exists, all three implementation commits are ancestors of the
current branch, and the working-tree diff has no whitespace errors. Existing
unstaged changes in `AGENTS.md`, `.planning/config.json` and
`.planning/continue.md` remain untouched.
