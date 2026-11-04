# v8 Closeout — PAL region support

**Closed:** 2026-11-03
**Closeout:** Verified; audit status `tech_debt`
**Next scope:** None committed

## Outcome

v8 delivers PAL region support across the six existing mapper families, with NTSC behavior preserved. All seven requirements are satisfied. Four phases and twelve plans are complete; the four phase verifications pass 57/57 truths, integration passes 7/7 paths, and all five end-to-end flows complete. The accepted current-source workflow passed 722 tests and packaged the library, runner and libretro adapter. The planning milestone did not publish a package or create a tag.

The refreshed audit has no requirement, phase-readiness, integration or end-to-end blocker. Its `tech_debt` status retains the WR-01 warning: checked-in baseline metadata points to an earlier source revision and campaign path, while current-candidate campaign evidence was separately verified. The audit also keeps explicit evidence limits for direct 2A07 proof, later held no-op-cycle behavior, PAL reset-edge duration, 2C07 calibration and physical host pacing. These limits are not hardware or host qualification claims.

The older interrupted full-CI/AccuracyCoin campaign remains stopped with unknown outcome. This closeout did not restart it or authorize a hosted workflow, push, release, device session or additional campaign.

## Audit reconciliation

The halted DMA-01 traceability quick item was reconciled using the later Phase 26 verification refresh (11/11), the canonical manager's four completed and passing v8 phases, and the refreshed milestone audit. Phase 26 summary attribution is corrected; Phase 26 remains the sole STATE-01 owner, with Phase 27.1 recorded as supplemental validation. The audit-open query then returned no open items. No new verification overrides were acknowledged; 16 prior acknowledgments remain carried forward (see [STATE.md Deferred Items](../STATE.md)).

The optional quick-task archive was skipped because the five remaining folders do not have established v8 ownership. They remain under `.planning/quick/` for their existing owners and records.

The archive reports 15 tasks from summary metadata, while the twelve plan contracts contain 31 task blocks. The milestone index records the plan-contract total and the parser discrepancy without rewriting phase summaries.

Archived phase plans preserve their original source references. References to the root `.planning/ROADMAP.md` and `.planning/REQUIREMENTS.md` identify the documents used during execution; the complete v8 snapshots are [v8-ROADMAP.md](v8-ROADMAP.md) and [v8-REQUIREMENTS.md](v8-REQUIREMENTS.md).

## Product direction and release boundary

The same-step product roadmap review is archived in [v8-PRODUCT-ROADMAP.md](v8-PRODUCT-ROADMAP.md), direction revision 14. RM-005 is shipped. No next milestone is committed. RM-015 remains the preferred conditional proposal only if bounded native/source triage confirms a core fault; scoped maintenance is the smaller alternative. RM-008 remains consumer-owned. PAL infrastructure is proven, but Dendy RM-006 still needs Dendy-specific timing evidence and demand. Mid and Long triggers are unchanged. SEED-003 remains partial; SEED-006 remains recurring, with no standalone tune-up promoted. Seed files were unchanged.

Release PR #56 merged and published v0.1.12 on 2026-10-31; that remains the latest published package. v8 is a planning milestone only: no v8 package, tag, push or hosted action is claimed. Milestone tags are disabled.

## Next step

The owner is the next actor. Run:

`$gsd-new-milestone`

This starts the next milestone setup and selects fresh scope from the current proposals. Context can be cleared before running it; read [STATE.md](../STATE.md), the [product roadmap](../PRODUCT-ROADMAP.md), and [playtesting lessons](../playtesting/LESSONS.md) first.
