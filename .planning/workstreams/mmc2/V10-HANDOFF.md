# v10 Outcome and Mainline Port

## Completed

- Phase 29 passed all six requirements and 12/12 goal truths. Integration review found 8/8 handoffs and 6/6 end-to-end flows.
- The source-bound `cmake --workflow --preset ci` run passed 728/728 tests and generated the C library, runner, and libretro packages on gate HEAD `23ebd976d2ac5386160f8ecce3d2d93365c566f7`.
- The provider receipt is package provenance and usage evidence: it binds a published source/build/package identity to a versioned profile and licensed scenario terms. It is not a software license, an AccuracyCoin score, or a dependency.
- No v10 package release or milestone tag was created. Attempts 01–08 remain terminal; do not retry them or start attempt-09.

## Transfer audit — 2026-11-19

The MMC2 implementation, tests and public documentation from Plans 29-01 through 29-03 are already present in the local Phase 28 branch at `0ea67fc`. The relevant implementation/test/docs files are byte-identical to the verified Phase 29 tree. A cherry-pick into that branch would duplicate work, so no transfer there is needed.

GitHub's default `main` is a different target. At the time of this audit it was at `8923b66` (release v0.1.13) and did not contain `src/mapper_mmc2.c`. The Phase 29 product commits were based on a later compatibility baseline: behavior revision 18 and state format 7, while current `main` is at revision 9 and format 3. A disposable local cherry-pick exposed conflicts in 11 files, including the public header, runner, provider profile, and saved-state hash pins; `docs/runner.md` has also been removed from current `main`. The trial was discarded without changing any project checkout or remote branch.

The primary Phase 28 checkout had 780 dirty entries in the read-only inventory. They are planning, coordination, configuration and documentation changes, with no dirty source, tests or public API files. Several Phase 28 candidate worktrees also have dirty planning/evidence files. These are preserved. They are separate from the MMC2 product changes, but the repository's clean-worktree rule still blocks a release while project-owned worktrees remain dirty.

## Recommendation and boundary

Do not merge the Phase 29 branch wholesale or cherry-pick its commits unchanged to GitHub `main`. A mainline port needs its own reviewed plan against the current release baseline. It must choose the current compatibility version bumps, update provider/header documentation and saved-state hashes, and resolve the protected AccuracyCoin/source-fingerprint gate without re-anchoring its historical baseline or importing the excluded 29-04 through 29-12 recovery changes. Then run the accepted candidate workflow once on that new source.

The open provider-route PRs (#64, #65 and #68) remain separate and have no MMC2 changes. No MMC2 PR was opened. The v10 milestone is complete; the next step, if the owner wants MMC2 on GitHub `main`, is a new milestone and plan rather than more Phase 29 recovery.

## Preserved limits

- Five inherited descriptor-less prohibitions remain unverified, with human review recommended; none is cleared or waived.
- WR-01 remains open for stale future-work wording in `docs/runner.md` and `src/mapper_mmc2.c` on the Phase 28 branch.
- The adopted MMC2 latch rows remain secondary-reference evidence, not hardware or broad game-compatibility proof.
- Do not rerun the completed Phase 29 CI or any terminal recovery attempt.

## Lessons carried forward

- `--ws` selects planning state, not the Git checkout. Confirm repository root and branch before execution.
- Compare the actual release target, not only the local primary worktree; matching code on a feature branch does not mean it is on GitHub `main`.
- Before transferring a feature across release baselines, compare compatibility fields, public docs, tests, golden outputs and protected evidence. A branch cherry-pick is only a starting point.
- Use a disposable checkout to inspect conflicts; never resolve them by taking whole files from the older branch.
- Run bounded source-bound preflight and focused controls before expensive workflows. Keep terminal attempts immutable and do not add a dependency or recurring CI lane without demonstrated need.
