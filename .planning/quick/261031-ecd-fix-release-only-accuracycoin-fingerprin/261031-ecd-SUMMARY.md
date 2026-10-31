---
quick_id: 261031-ecd
status: complete
implementation_commit: 9470359
verification_scope: focused local checks; full CI/campaign remains stopped pending owner decision
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

The full `cmake --workflow --preset ci` and AccuracyCoin campaign remain
unrun. The prior interruption left their result unknown; the project stop
requires a separate owner decision before rerunning either. No push, merge or
publication occurred.

## Commits

- `6c928f9` — fix(ci): keep release version out of AccuracyCoin fingerprint
- `8573742` — perf(ci): skip Nightly lanes for release pull requests
- `9470359` — docs(ci): clarify release fingerprint and Nightly coverage

## Self-Check: PASSED

The summary exists, all three implementation commits are ancestors of the
current branch, and the working-tree diff has no whitespace errors. Existing
unstaged changes in `AGENTS.md`, `.planning/config.json` and
`.planning/continue.md` remain untouched.
