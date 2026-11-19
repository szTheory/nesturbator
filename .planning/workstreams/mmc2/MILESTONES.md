# Milestones

## v11 — MMC2 mainline compatibility port (Planning complete; Phase 30 discussion pending)

**Scope:** 1 phase; six requirements

**Approved direction:** Carry the verified bounded mapper 9/submapper 0 profile
onto the exact revalidated live Nesturbator `main`, through the C library,
runner and libretro adapter. At this planning boundary the live head was
`8923b66cca16739c5b402643798515cf5537ae22` (v0.1.13; behavior revision 9,
state format 3, ABI 1); local `origin/main` was stale. If that base remains
current, derive behavior revision 10 and state format 4 while retaining ABI 1.
The next phase discussion must recheck main and set up a clean isolated source
worktree. The port excludes whole-branch cherry-picks, Phase 29 recovery
scaffolding, AccuracyCoin re-anchoring, new dependencies, consumer acceptance,
and broad hardware/title claims.

**Requirements and phase:** [v11 requirements](REQUIREMENTS.md),
[roadmap and Phase 30 contract](ROADMAP.md), [fresh-context handoff](V11-HANDOFF.md).
Phase 30 has not started; no v11 implementation, test, CI run, package or
release is claimed.

## v10 — Bounded MMC2 mapper 9 (Planning milestone complete: 2026-11-19)

**Scope:** 1 phase; 10 indexed plans; 11 summarized task records

**Key accomplishments:**
- Phase 29 met all six requirements, passed goal verification 12/12, and traced 8/8 integration handoffs and 6/6 end-to-end flows.
- One source-bound `cmake --workflow --preset ci` run passed 728/728 tests and generated the library, runner, and libretro packages. No new dependency, maintained CI lane, or public ABI change was added.
- Five inherited prohibitions remain unverified with human review recommended; WR-01 remains an open documentation/comment warning. No waiver, public package release, or v10 tag is claimed.
- A later transfer audit found the MMC2 product files already present on the local Phase 28 branch. Current GitHub `main` is a separate, older release baseline and needs a new reviewed port scope; details are in [the v10 handoff](V10-HANDOFF.md). Dirty checkout ownership remains unrelated and all edits are preserved.

## Lessons carried forward

- `--ws mmc2` selects planning state, not a Git checkout. Check the repository root and branch before executing a phase command.
- Run the smallest source-bound preflight and focused controls first, then use the standard full workflow once. Avoid custom recovery wrappers that reinterpret CTest registration metadata or turn an unrelated recorder mismatch into a false acceptance failure.
- A stopped attempt stays terminal. Diagnose the cause, preserve its record, and create a separately reviewed scope only when new runtime work is justified; never quietly retry it under another label.
- Refresh phase verification after editing covered acceptance or summary records. This metadata-only refresh does not require rerunning CI when the code candidate is unchanged.
- The provider receipt ties a published package/source/build to its versioned profile and licensed scenario/use terms. It is not itself a license or an AccuracyCoin test; those gates remain separate from MMC2 acceptance.
- Verify the actual upstream release target as well as the local primary branch before transferring work. A local branch containing the feature does not mean GitHub `main` contains it; do not port across compatibility baselines by wholesale cherry-pick or disturb unrelated dirty work.

---
