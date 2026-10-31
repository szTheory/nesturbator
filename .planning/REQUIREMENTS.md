# Requirements: nesturbator v6

**Milestone:** Player-ready NES and reproducible feedback
**Defined:** 2026-10-30
**Status:** All ten approved requirements are implemented. Phase 22 and Phase 23 verification are current and passed; the v6 milestone audit is next. The approved scope is unchanged.
**Core Value:** Games behave as on the console, identically across platforms, from a small MIT C library.

## v6 Requirements

### Install and protect progress

- [x] **READY-01**: A player can use a concise README to find current supported content, installation, controls, progress and troubleshooting, with detailed developer material available through working topic-guide links.
- [x] **READY-02**: A player can select and install the appropriate existing core archive for their platform/architecture and identify the loaded core/build, distinguishing published-release capabilities from merged development behavior.
- [x] **READY-03**: A player can follow host-owned control guidance to verify input through the actual frontend launch and avoid accidentally assigning one controller to both active player ports.
- [x] **READY-04**: A player can distinguish battery saves, core-specific snapshots, rewind and automatic loading, and locate the host-configured storage that holds their progress.
- [x] **READY-05**: A player can follow an explicit supported-version, backup and rollback procedure that preserves newer progress and describes incompatible-state refusal without promising conversion or cross-core snapshot support.
- [x] **READY-06**: A developer can find the relocated API, runner and release contracts, and the existing automated documentation/package checks still reject missing or contradictory claims at their maintained document locations.

### Reproduce and report a scene

- [x] **REPRO-01**: A player can submit a sanitized scene report identifying the content revision/digest, actual core/build and host, scene/access route, inputs, expected/observed behavior, repeatability and untried conditions.
- [x] **REPRO-02**: A developer can follow a bounded existing-runner/host recipe on a permitted fixture to replay recorded inputs from cold boot and a supported checkpoint and compare frame/audio outputs across repeated runs.
- [x] **REPRO-03**: A developer can run the documented reproduction in isolated scratch storage with explicit frame/time limits, keep private artifacts out of public output, and receive a clear stop or unsupported result for identity/container mismatches instead of silently using a wrong checkpoint.
- [x] **REPRO-04**: A developer can follow a documented triage route that separates native output, adapter conversion, host presentation and original-game behavior, and records repeatability separately from independently established correctness.

## Acceptance boundaries

The following qualify the requirements without adding a new host architecture or
an accuracy-fix obligation:

- README length is judged by whether the player path is direct and technical
  detail is linked; no arbitrary line target. Reorganization retains all unique
  contracts and updates their independent validators deliberately. Known consumers
  include install-line/archive checks, runner options, stop/exit/status tables,
  save-state and `.info`/header support checks. Do not make mismatches pass by
  deleting assertions or replacing contracts with self-confirming snapshots.
- Installation uses the existing six-target packaging contract and actual archive
  layout. Recheck the available release at implementation time. At opening,
  release PR #56 is open and v0.1.11 is latest published; development source has
  ABI 1/state format 3/behavior revision 9. No new release publication, libretro
  buildbot submission or live six-platform host matrix is required by v6.
- Controls explain host-owned assignment, actual launch context and a distinct
  second player; this does not add core input-mapping code. Existing daily-play
  routing observations and automated two-port coverage are credited with their
  limits. No new commercial play sweep or owner sign-off is an acceptance gate.
- State policy distinguishes library format from behavior revision, ROM identity,
  host snapshot container and core version. Current format 3 rejects earlier
  formats and behavior revision alone is not enforced compatibility. Preserve
  newer battery data before loading an older state; preserve the prior core,
  content identity and paired backups for rollback. No converter or new future-
  version compatibility promise. Document recovery if the host attempts autoload
  of an unsupported snapshot, without altering daily files during verification.
- REPRO-02 uses existing owned/permitted input, runner movie, frame/audio and
  checkpoint facilities. Demonstrate repeatability and an input-sensitive control
  that detects a wrong/dropped input or wrong continuation; cite existing coverage
  where it already proves the same invariant. A current-output hash establishes
  repeatability, not hardware correctness. Native checkpoints are not assumed to
  be RetroArch disk containers. Public examples must work without private helpers.
- REPRO-03 documents an explicit finite frame cap and outer time bound for each
  example, and tests only scratch fixtures. A missing required facility is reported
  as unsupported; write a small owned helper only after demonstrating the gap.
  No daemon, shared replay framework, third-party test implementation or new core
  dependency. Machine-verifiable examples/controls run automatically through
  existing CMake/CTest paths where they add unique coverage.
- REPRO-04 provides decision points and independent-oracle guidance; it does not
  require diagnosis or repair of a reported commercial scene. Commercial media,
  checkpoints and private content stay ignored and outside public CI. Retain the
  known scene observations and access limits in their existing evidence files.
- Any runtime behavior change discovered necessary for this contract carries
  an owned regression, README/public-header updates and `cmake --workflow --preset ci`.
  Documentation-only work uses the relevant executable document/package checks
  without rerunning unrelated expensive campaigns. Preserve existing hashes,
  protected PASS keys and performance gates; do not relax them for this milestone.
- Recurring collector source-mode/lookup debt remains deferred. If a phase needs
  a new qualified collected bundle, repair the specific recurring collector
  prerequisites first under an explicit scoped adjustment; do not rely on old
  manual mitigations or restart stopped campaigns as routine documentation work.

## Future Requirements

- **Scene accuracy (RM-015):** Promote a separate fix/regression outcome only after bounded source/native/reference triage establishes a concrete core fault. Otherwise proceed to the five remaining DMA failures, retaining the three already-passing former candidates.
- **Playstead integration (RM-008):** Consumer-owned production launch, readiness/input, exit and progress acceptance; a core document or RetroArch test does not establish this outcome.
- **Regions (RM-005/RM-006):** PAL is the regional alternative; Dendy requires its own timing/evidence contract.
- **Mapper expansion:** Favor MMC2/Punch-Out!! when expansion is selected. MMC5 and Sunsoft-5B/expansion audio stay independently scoped.
- **Optional presentation:** Overscan may be selected later with full-frame diagnostic output preserved.
- **Testing proposals:** Property helpers, mutation testing and runner signal-test redesign remain unselected until their added defect detection or measured need is demonstrated.

## Out of Scope

| Feature | Reason |
|---|---|
| Fixing the reported seams, wobble, slowdown or music interruption | Causes are unclassified; v6 delivers a report/reproduction path, not speculative hardware changes. |
| Remaining DMA cluster or an all-pass AccuracyCoin target | Separately measurable accuracy outcomes requiring their own hardware/regression scope. |
| Overscan/cropping options | Optional presentation feature was not selected. |
| PAL/Dendy, new mappers, expansion audio, FDS or peripherals | Independent hardware and compatibility outcomes. |
| Actual Playstead integration, a new GUI or shared host | Consumer-owned work; library, runner and libretro remain the three deliverables. |
| Another commercial sweep, mandatory device play or owner UAT | Existing reports are sufficient; automated permitted-fixture evidence closes machine-verifiable behavior. |
| General autonomous play, cheat injection or commercial-derived fixtures | Existing facilities and original/permitted bytes suffice; private content remains private. |
| Broad collector, platform, historical-audit or CI redesign | Preserve debt; only demonstrated prerequisites need a separately bounded adjustment. |
| State converters or universal cross-version/cross-core snapshot compatibility | Document and protect the current refusal and backup policy instead. |

## Traceability

Approved mapping; each requirement belongs to exactly one phase. All mapped requirements are implemented.

| Requirement | Phase | Status |
|---|---|---|
| READY-01 | Phase 22 | Complete |
| READY-02 | Phase 22 | Complete |
| READY-03 | Phase 22 | Complete |
| READY-04 | Phase 22 | Complete |
| READY-05 | Phase 22 | Complete |
| READY-06 | Phase 22 | Complete |
| REPRO-01 | Phase 23 | Complete |
| REPRO-02 | Phase 23 | Complete |
| REPRO-03 | Phase 23 | Complete |
| REPRO-04 | Phase 23 | Complete |

**Coverage:** 10 approved v6 requirements; 10 mapped exactly once; 0 unmapped.

## Inputs and preserved evidence

Owner selected the recommended v6 scope with “follow ur recs” during
`$gsd-new-milestone`. This contract reuses the completed
[discovery synthesis](research/milestone-v6-discovery/SYNTHESIS.md),
[postmortem](playtesting/POSTMORTEM.md), [automation direction](playtesting/AUTOMATION.md)
and [lessons](playtesting/LESSONS.md). No generic research fan-out was repeated.
SEED-006 supplies only the measured documentation/contract repairs; seed files
remain unchanged and the delivered playability/state/board work stays credited.
The [product roadmap](PRODUCT-ROADMAP.md) retains all four horizons and triggers;
the [v5 audit](milestones/v5-MILESTONE-AUDIT.md) and
[closeout](milestones/v5-CLOSEOUT.md) retain debt and evidence limits. No existing
runtime stop, test budget or live-operation authority is changed by this document.

*Last updated: 2026-10-30 after Phase 22 execution and verification.*
