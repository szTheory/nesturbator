# Requirements: nesturbator

**Milestone:** v11 — MMC2 mainline compatibility port
**Defined:** 2026-11-19
**Workstream:** `mmc2`
**Core Value:** Console behavior is reproduced identically across platforms by a small MIT-licensed C core.

## Context

The v10 MMC2 implementation is complete and its source-bound workflow passed on
the Phase 29 candidate. The product files are already present on the local
Phase 28 branch. Live GitHub `main` is a separate baseline at commit
[`8923b66`](https://github.com/szTheory/nesturbator/commit/8923b66cca16739c5b402643798515cf5537ae22)
(release 0.1.13); it does not contain MMC2 and records behavior revision 9 and
state format 3. The port must start from that live baseline, adapt the existing
owned implementation, and must not merge the Phase 29 branch wholesale.

The current LibFuturo report still places its provider route behind the
publisher-owned release receipt. Its public repository describes an older
revision-9/state-format-3 source seam and a create/destroy probe only; that is
not current NES gameplay acceptance. See the dated [integration context](../../../docs/coordination/milestone-context-2026-11-03.md).
This milestone supplies no LibFuturo or Playstead acceptance claim.

## v1 Requirements

### MMC2 behavior on the live mainline baseline

- [ ] **MMC2-07**: Starting from the revalidated live `main` baseline, the core supports the same single NES 2.0 mapper 9/submapper 0 profile selected in v10, with direct owned-marker coverage for PRG banking, mirroring, both CHR latches and trigger/non-trigger PPU reads through the library, runner and libretro adapter.
- [ ] **MMC2-08**: Save/load preserves every mutable MMC2 selector and both latches, rebuilds derived mappings, refuses incompatible prior state formats, and produces deterministic continuation through all three deliverables. Derive compatibility identities from the target baseline: for the reviewed 0.1.13 head this means behavior revision 10 and state format 4, retaining ABI 1; do not copy v10's revision 18 or format 7.
- [ ] **MMC2-09**: Existing supported mapper behavior and protected output evidence remain unchanged. Any state-hash updates are limited to the necessary serialized-format change and are explained; do not re-anchor AccuracyCoin inputs, baselines or campaign evidence to hide a mismatch.
- [ ] **MMC2-10**: README and public-header comments describe the supported mapper 9 profile and the new state compatibility boundary, retain the current public ABI, and make no Punch-Out!!, broad mapper-9, hardware-verification or consumer-integration claim.

### Automated acceptance and delivery

- [ ] **MMC2-11**: A cheap source-bound preflight verifies the pinned base, MMC2-only source/test delta, test registration, version/state identities and protected AccuracyCoin bindings before expensive acceptance. Focused controls pass, then the existing `cmake --workflow --preset ci` passes completely on the same candidate and produces the library, runner and libretro packages within the configured limits.
- [ ] **MMC2-12**: The reviewed port lands through the normal Conventional Commit pull-request path; exact-candidate required checks pass before merge, then resulting-main checks pass before publication. If the milestone produces a release-worthy change, publish through the normal release path and verify package contents, checksums and source identity. This release does not by itself satisfy the separate LibFuturo receipt or consumer gates.

## Future Requirements

- Other mapper families, MMC4/mapper 10, additional MMC2 variants and commercial-title compatibility.
- LibFuturo installed-consumer qualification, a LibFuturo NES host, gameplay acceptance and Playstead integration; these remain consumer-owned except for the separate publisher receipt/profile/scenario work recorded under RM-017.

## Out of Scope

| Feature | Reason |
|---|---|
| Whole-branch cherry-pick or import of Phase 29 recovery scaffolding | The target is a different compatibility baseline; the prior trial conflicted across 11 files. Port only the reviewed MMC2 behavior and necessary compatibility changes. |
| Changing protected AccuracyCoin inputs, evidence, hashes or acceptance rules to make the port pass | Protected evidence is an independent guard. A mismatch must stop and be diagnosed. |
| New public API/ABI, dependency, maintained CI lane, optimization or performance route | No demonstrated need exists for this port. Reconsider only through a separately reviewed decision if implementation exposes a concrete gap. |
| Retrying Phase 29 CI, its terminal attempts, or starting attempt-09 | Those outcomes remain historical and terminal. The new target baseline has its own acceptance scope. |
| Provider receipt, licensed scenario, LibFuturo/Playstead host or gameplay acceptance | These belong to RM-017 and the consumer workstream; MMC2 evidence cannot substitute for them. |
| Broader hardware, silicon or commercial-game claim | v10's adopted mapper behavior and evidence tier remain unchanged. |

## Evidence and verification policy

Before source work, recheck live `main` and create a clean, isolated phase
worktree pinned to that exact commit. The local `origin/main` reference was
stale at this milestone boundary and is not a valid substitute. If `main` has
moved, record the new identity and re-evaluate compatibility before editing.

Use the existing owned mapper fixture and CMake/CTest infrastructure. Run the
smallest source-bound consistency and focused checks first, preserve the full
required workflow and protected evidence, and inspect available CI timing for
safe iteration improvements. Keep useful throwaway checks local; add a
maintained CI check or dependency only when independent recurring value is
demonstrated. Deterministic mapper acceptance needs no human UAT. The project
workflow's 30-minute ceiling and any stricter per-check limits still apply.

The seed review is bounded: SEED-005 supports this one mapper port but adds no
other board; recurring SEED-006 calls for only scope-specific, evidence-backed
documentation or CI improvements. All other seeds remain unchanged and out of
this milestone.

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| MMC2-07 | Phase 30 | Pending |
| MMC2-08 | Phase 30 | Pending |
| MMC2-09 | Phase 30 | Pending |
| MMC2-10 | Phase 30 | Pending |
| MMC2-11 | Phase 30 | Pending |
| MMC2-12 | Phase 30 | Pending |

**Coverage:** Six v1 requirements, six mapped, zero unmapped. Each requirement
has one owning phase; mapping does not claim implementation or acceptance.

---
*Requirements defined: 2026-11-19*
