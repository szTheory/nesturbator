# Phase 28: Qualified NES gameplay package - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-11-03
**Phase:** 28-qualified-nes-gameplay-package
**Areas discussed:** Licensed NROM gameplay route and progress oracle, LibFuturo provider handoff, package and release lineage, verification and release sequencing

---

## Licensed NROM gameplay route and progress oracle

| Option | Description | Selected |
|--------|-------------|----------|
| Nesteroids with qualification gate | Use the admitted MIT NROM and verify its source-to-RAM mapping and Start/A gameplay route before accepting its oracle. | ✓ |
| Another permitted NROM | Switch only if its progress predicate has a defensible independent basis. | |
| Defer gameplay scenario | Leave GAME-01 unfulfilled. | |

**User's choice:** The user asked for the synthesized recommendations to be followed. Applied Nesteroids as the preferred route with the explicit mapping gate and fallback condition.
**Notes:** The pinned source/debug artifacts conflict on candidate addresses for `player_shot_count` and `game_screen`. Guessing is prohibited; source/build evidence must resolve the mapping.

---

## LibFuturo provider handoff

| Option | Description | Selected |
|--------|-------------|----------|
| Nesturbator-owned version-bound NTSC profile | Document current provider semantics and capabilities using the public API; keep consumer acceptance with LibFuturo. | ✓ |
| Modify LibFuturo or add a new API now | Expand scope without a demonstrated interface gap. | |
| Broaden to PAL/consumer capabilities | Claim behavior without current consumer qualification. | |

**User's choice:** Apply the recommended upstream profile approach.
**Notes:** The sibling checkout was inspected read-only. Its current plan, host, and acceptance evidence remain consumer-owned and are not changed by this phase.

---

## Package and release lineage

| Option | Description | Selected |
|--------|-------------|----------|
| Preserve v0.1.12 baseline and let Release Please derive | Bind acceptance to the exact published source/package and verified artifacts. | ✓ |
| Force a manual next version | Set a version before merged history determines it. | |
| Treat local v0.1.11 as published | Conflate current branch metadata with release evidence. | |

**User's choice:** Apply the recommended release-lineage approach.
**Notes:** The phase branch includes newer unreleased source; publication and checksums remain required before PKG-01 can pass.

---

## Verification and release sequencing

| Option | Description | Selected |
|--------|-------------|----------|
| Preserve `gaps_found` and stop at the GSD ship gate | Keeps all current evidence truthful but leaves the already-approved release unable to start through the `$gsd-ship` wrapper. | |
| Run the complete release inside the gap-closure plan | Could produce the missing hosted and published evidence before final verification, but moves release work into the phase execution step and away from the normal milestone closeout boundary. | |
| Use the authorized v9 closeout release path and verify afterward | Preserves the published-receipt acceptance gate and current verification status; carries separate PR-head, tested-merge, resulting-main, and release identities through final verification. | ✓ |

**User's choice:** The user instructed us to follow the recommendations for all decisions in this discussion. Applied the recommendation to keep `gaps_found` and the PKG-01/PROF-01 requirements pending until actual hosted CI and published receipt evidence exist, while using the project's already-authorized milestone release path to resolve the sequence.
**Notes:** Read-only research across GSD lifecycle, release-engineering, and adversarial lenses found that GSD `$gsd-ship` requires phase verification to be `passed`, while this phase cannot pass before publication. Do not alter acceptance truth or use a status override to escape that loop. The existing project release authority applies; preserve every required CI, merge, artifact, checksum, attestation, and published native-replay gate. GitHub's primary documentation distinguishes the PR head from its default test merge commit and requires checks on the latest relevant commit SHA.

## the agent's Discretion

- Derive scenario timing and thresholds from source evidence and bounded runtime qualification.
- Reuse current consumer/package patterns and add only missing relocation, refusal-preservation, and gameplay checks.
- Select another permitted NROM only if Nesteroids' mapping cannot be verified and another title has a defensible oracle.
- Add API surface only after identifying a concrete consumer gap.

## Deferred Ideas

- PAL-through-LibFuturo, LibFuturo host playback, Playstead acceptance, hardware/device qualification, machine snapshots, and broad mapper/title compatibility.
- Reviewed but deferred `2026-10-26-add-reproducible-owned-property-tests.md` and `2026-10-26-make-runner-signal-checks-deterministic.md`.
