# Phase 22: Install and protect progress - Discussion Log

> Audit trail only. Downstream planning/research/execution uses `22-CONTEXT.md`.

**Date:** 2026-10-30
**Areas discussed:** README/topic ownership; archive/build identity; actual-launch
controls; progress/version/rollback; executable contracts and adversarial cases.

## Owner direction and method

The owner invoked `$gsd-discuss-phase 22`, then explicitly requested relevant
subagent fan-out, primary sources, pros/cons/tradeoffs, adversarial review and a
coherent recommendation to follow automatically. The owner reaffirmed a preference
for small owned duplication over incidental dependencies. This superseded routine
area-selection/preference questions; selections below were made under that
delegation within the approved scope. Auto-chaining was not requested or enabled.

Three read-only specialist reviews used current scope, existing discovery,
playtesting evidence and focused owned source/test inspection. Installation
research consulted official RetroArch CLI/input documentation. Bounded follow-ups
challenged migration and data-loss traps. No runtime tests, host/device operations,
gameplay or daily-file changes occurred.

## Player README and topic guides

| Option | Benefit | Cost / failure mode | Selected |
|--------|---------|---------------------|----------|
| Focused topic guides with direct README routes | Clear player path and contract ownership | Links and lookup must move coherently | Yes |
| One developer omnibus | Fewer files | Recreates the long-entry-point problem | No |
| All detailed contracts in README | Least parser movement | Leaves the demonstrated navigation/truth problem | No |

**Delegated selection:** Focused API, runner, release and testing guides, plus a
player progress walkthrough. Keep literal tested installation/support in README.

**Adversarial result:** Archives currently contain no README/guides. Treat guides
as repository documentation and retain package layout. Keep release markers in
their current owner; version-neutral release prose avoids drift. API migration
must preserve the save-state section and rewind/save-RAM warning.

## Installation and loaded identity

| Option | Benefit | Cost / failure mode | Selected |
|--------|---------|---------------------|----------|
| Published libretro ZIP | Existing public player route | Release may lag source | Primary |
| Identified CI development artifact | Binds behavior to source/run | Not a release; semantic label can be identical | Alternative |
| Pinned local build | Controlled developer checkout | Toolchain and dirty-source attribution | Developer alternative |
| New build-ID API/installer | Potentially easier display | Unnecessary behavior/scope expansion | No |

**Delegated selection:** Existing six-target packages; match host executable
architecture. Record loaded path/digest and package provenance; name/version
alone does not identify exact code. Recheck published assets during implementation.
No publication or live platform matrix is required.

## Host controls

| Option | Benefit | Cost / failure mode | Selected |
|--------|---------|---------------------|----------|
| Host assignment through actual frontend launch | Matches real configuration/evidence | Must disclose button/reconnect evidence limits | Yes |
| Core mapping/device changes | Could mask host setup issues | Outside core ownership and READY-03 | No |
| Another commercial/device play sweep | More observations | Duplicates evidence; unapproved gate | No |

**Delegated selection:** P1 assigned/P2 unassigned for one controller; distinct
devices for two active players. Logical-port tests and actual-launch observations
are complementary. This does not diagnose slowdown/rendering symptoms.

## Progress and compatibility

| Option | Benefit | Cost / failure mode | Selected |
|--------|---------|---------------------|----------|
| Exact-build/content snapshot recommendation; newest battery separate | Conservative recovery without code changes | Some accepted states remain unqualified across builds | Yes |
| Native acceptance as broad compatibility promise | Mirrors loader checks | Behavior revision unenforced; old states revert SRAM | No |
| Version-pair qualification matrix | Can support selected cross-build restores | Ongoing qualification without current need | Deferred |

**Delegated selection:** Back up prior core/content identity, paired snapshots/
battery data and newest battery progress separately. Verify immutable backup
digests. Use only scratch fixtures/config/storage for the walkthrough.

**Adversarial result:** Fresh rollback with newer SRAM is conditional on same
content/board/save geometry and demonstrated compatibility. Native atomic refusal
does not prove host-container/autoload recovery. Reuse native tests as the native
oracle and disclose host limits. Disable/isolate auto-load before fresh recovery.

## Executable contracts and verification

| Option | Benefit | Cost / failure mode | Selected |
|--------|---------|---------------------|----------|
| Retarget independent checks; add focused moved-path controls | Retains unique defect detection | Small CMake changes must distinguish canonical source | Yes |
| Search a combined Markdown corpus | Simple lookup | Stale copies hide missing canonical claims | No |
| Drop assertions to shorten README | Less work | Weakens approved executable contracts | No |
| Docs framework/general mutation tooling | Broad capabilities | Dependency/maintenance without demonstrated need | No |

**Delegated selection:** Preserve API/status/state, runner options/stop/exit,
install/archive and info/release checks. Info/archive controls do not exercise
moved API/runner lookup; cover that gap and guide links. Runtime changes would
trigger regressions, README/header updates and full CI under the existing rule.

## Agent discretion

Exact filenames/anchors, minimal CMake path wiring, fixture choice and bounded
control organization remain implementation choices. Approved scope, safety stops,
dependency/clean-room rules and phase boundary remain locked. Recommendations
converged; further generic role fan-out would repeat known findings.

## Deferred Ideas

Phase 23 replay/report/triage; actual Playstead; accuracy/overscan/regions/mappers;
converters/cross-build matrices; archive docs expansion; collector repair; the
already-reviewed property/mutation/signal proposals. No todo was promoted and no
implementation requirement was marked complete.
