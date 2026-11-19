# v11 handoff: MMC2 mainline compatibility port

**Status:** Plans 30-01 and 30-02 are complete. The first two Plan 30-03
full-workflow attempts failed and remain preserved. The owner-authorized third
local attempt passed all 702 CTest entries, the AccuracyCoin campaign and
activation, and package generation on commit `48f6f05`; its one-run
authorization is spent. Exact-head hosted CI then found compiler/format
diagnostics. Reviewed follow-on commit `6e28aa2` fixes those issues; its current
source binding, preflight, 24-test focused set and clang-format check pass. PR
#73 is open on the prior head until the fix is pushed and all exact-head CI and
Nightly checks pass. Merge and release delivery remain.

## Local acceptance passed; delivery remains

The corrected compatibility checks passed the expanded 24-test focused gate.
The final source-bound preflight and independent plan/preflight review passed.
The third workflow ran once at candidate
`48f6f054ed363ea50b5da0bfe96f960230f673b7`, completed in 460.358 seconds under
the 1500-second cap, and generated the library, runner and libretro archives.
Those packages remain bound to that source and are not final provenance for
the follow-on. Logs, fingerprint evidence and package hashes are in the [30-03
checkpoint](phases/30-mmc2-mainline-compatibility-and-delivery/30-03-SUMMARY.md).

Push `6e28aa2` to the existing PR #73 and require all configured CI platform
jobs and triggered Nightly checks to pass on its exact head. Provider PR #68
remains separate, open and unmergeable; do not modify it. Do not start another
local full workflow under the spent one-run authorization. Revalidate live
`main`, then complete resulting-main and release-source evidence before asking
the owner to approve merge or publication.

**Next actor:** Orchestrator. No checkout switch or new clone is needed.

Detailed run and targeted-fix evidence is in the [30-03 checkpoint](phases/30-mmc2-mainline-compatibility-and-delivery/30-03-SUMMARY.md).

## Why this milestone exists

Phase 29 completed the bounded MMC2 mapper 9 implementation on a different
branch. That code is already present on the local Phase 28 branch, but public
GitHub `main` lacks MMC2. A trial cherry-pick conflicted across 11 files because
the target compatibility baseline differs. This milestone therefore ports the
reviewed behavior onto the actual mainline baseline; it does not copy the
Phase 29 branch or repeat its recovery work.

At the 2026-11-19 boundary, live main was
[`8923b66cca16739c5b402643798515cf5537ae22`](https://github.com/szTheory/nesturbator/commit/8923b66cca16739c5b402643798515cf5537ae22),
release v0.1.13, behavior revision 9, state format 3 and ABI 1. Local
`origin/main` was stale. Phase 30 discussion and source work must recheck live
main. If that exact compatibility baseline remains current, the intended
changes are behavior revision 10, state format 4 and ABI 1. Re-derive all
compatibility decisions if upstream moved.

## Phase 30 contract

Deliver the already selected NES 2.0 mapper 9/submapper 0 profile through the C
library, headless runner and libretro adapter. Preserve PRG banking, mirroring,
both CHR latches and trigger/non-trigger read behavior; preserve deterministic
save/load continuation and existing supported mapper behavior. Update only
necessary compatibility fields, state hashes, README and public-header
comments. Keep AccuracyCoin inputs and protected baselines unchanged. The
accepted latch behavior remains secondary-reference evidence; do not claim
silicon validation, broad mapper-9 support, MMC4, or Punch-Out!! compatibility.

Before source edits, use a clean isolated phase worktree pinned to the exact
revalidated main commit and make the selected `mmc2` planning state resolvable
there. Preserve unrelated dirty checkouts. A cheap source-bound preflight must
catch wrong base, out-of-scope file/test changes, stale test registration,
incorrect version/state fields or altered protected evidence before expensive
acceptance. Run useful focused controls, then the standard
`cmake --workflow --preset ci` once on the same candidate within the existing
30-minute workflow ceiling, 1500-second plan gate and stricter per-check caps.
Inspect current CI timing for demonstrated iteration savings; retain throwaway
local probes when useful, and add maintained CI machinery or dependencies only
for recurring value. The project expects automated verification and does not
require human UAT for deterministic mapper behavior.

After implementation, review and goal verification, use the ordinary reviewed
Conventional Commit PR path. Exact-candidate required checks must pass before
merge; resulting-main checks must pass before publication. At milestone
closeout, follow the existing release authority if the change is release-worthy
and all gates pass, verify package contents/checksums/source identity, and keep
project-owned worktrees clean while preserving unrelated user work.

## Preserved boundaries and stops

- Do not retry Phase 29 CI or terminal attempts, create attempt-09, or import
  Phase 29 recovery scaffolding. The new baseline has its own reviewed checks.
- Preserve the five inherited unverified-prohibition flags and v10 WR-01 from
  the [v10 handoff](V10-HANDOFF.md); this scope does not waive
  them or re-anchor AccuracyCoin evidence.
- RM-017 remains separate. Nesturbator owns provider package/profile/scenario
  and publisher evidence; LibFuturo owns the installed consumer, NES host and
  gameplay acceptance. MMC2 does not supply the missing receipt or satisfy
  Playstead acceptance.
- No performance capture, retired APU route, interrupted-campaign restart,
  dependency, public ABI expansion or broad hardware claim is authorized by
  this milestone.
- Keep future work in the selected `mmc2` workstream. Do not edit or take over
  separate OpenGSD downstream implementation work.

## Review basis

Bounded architecture, verification/release and product/consumer reviews
converged on the minimal port from exact live main, with independent state and
protected-evidence checks. The adversarial check was whether the earlier
whole-branch cherry-pick or a larger cleanup would be faster; the 11-file
conflict and changed compatibility baseline make that approach higher risk and
broader than needed. Revalidation, a source-bound preflight and an isolated
worktree address that risk directly.

The current public [LibFuturo seam map](https://github.com/szTheory/libfuturo/blob/main/docs/nesturbator-seams.md)
documents an older revision-9/state-format-3 pin and create/destroy probe. Its
[README](https://github.com/szTheory/libfuturo) describes synthetic-v1 source
proof. These are not current installed-consumer or gameplay acceptance. The
fresh 2026-11-19 owner-supplied report still records the publisher receipt as
missing, so RM-017 remains blocked independently of this mapper port. Details
and ownership are in the [integration context](../../../docs/coordination/milestone-context-2026-11-03.md#v11-opening-consumer-check-2026-11-19).

The milestone roadmap, requirements, product-roadmap review and durable
verification/discussion preferences are already recorded in the selected
workstream and project planning documents. See [v11 requirements](REQUIREMENTS.md)
and [Phase 30 roadmap](ROADMAP.md).
