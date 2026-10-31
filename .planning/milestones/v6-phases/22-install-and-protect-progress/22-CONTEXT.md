# Phase 22: Install and protect progress - Context

**Gathered:** 2026-10-30
**Status:** Ready for planning

<domain>
## Phase Boundary

Deliver READY-01 through READY-06: a direct player README, accurate installation
and core identity, host-owned controls, and progress-safe backup/upgrade/rollback
guidance, preserving the existing executable developer contracts. Requirements
and success criteria are locked in `.planning/REQUIREMENTS.md` and
`.planning/ROADMAP.md`; there is no separate phase SPEC.

This is documentation and contract maintenance around the existing library,
runner and libretro adapter. Phase 23 owns scene reports/replay/triage. Existing
daily-play and automated evidence is credited; no new owner gameplay, release
publication, device setup or broad research sweep is a prerequisite.
</domain>

<decisions>
## Implementation Decisions

The owner selected all relevant decisions and explicitly delegated resolution:
fan out across relevant roles, examine alternatives and adversarial cases, then
follow a coherent set of recommendations without routine preference questions.
Three specialist reviews covered docs/contracts, install/input, and progress;
bounded follow-ups challenged migration and data-loss traps. These are adopted
planning constraints, not newly verified runtime outcomes.

### Player entry point and topic ownership

- **D-01:** Order the README around current supported content/limits, install
  and identify the core, load content, controls, progress, troubleshooting, then
  developer links. Correct the stale milestone/behavior opening. Judge brevity
  by this direct route, with no arbitrary line quota or general prose rewrite.
- **D-02:** Prefer focused repository guides: `docs/api.md`, `docs/runner.md`,
  `docs/releases.md`, `docs/testing.md`, and a player `docs/progress.md` for the
  full backup/upgrade/refusal/rollback walkthrough. README keeps short player
  warnings and direct links. Equivalent existing destinations may be reused;
  exact filenames/headings are planner discretion if lookup stays explicit.
- **D-03:** Give each detailed contract one maintained home: API/status/native
  states in API; runner options/stop/exit in runner; maintainer packaging/release
  process in releases; engineering checks in testing; player storage/recovery in
  progress. Retain unique claims, caveats and the warning that rewinding past an
  in-game save reverts save RAM. Link summaries to their detailed owner.
- **D-04:** Keep the literal, release-marked Apple Silicon install block and
  machine-checked support summary in README. Retarget API/runner document
  consumers explicitly. Keep release-process prose version-neutral where
  possible; introduce no second manually maintained current-release number.
- **D-05:** Guides are repository documentation. Existing archives contain no
  README/guides; retain their current layout and allowlists. Make source/release
  documentation links explicit, using the matching tag/revision where historical
  behavior matters. Do not imply extracted archives contain relative doc links
  or expand packaging merely to support this reorganization. No docs framework.

### Archive choice and actual loaded identity

- **D-06:** The public installation route is the existing published libretro
  archive. Show Linux/macOS/Windows × x64/arm64, selecting architecture for the
  RetroArch executable actually in use. Explain `cores/` and `info/` extraction
  into the host's configured directories, permissions and platform differences;
  keep the existing tested macOS command. No new installer or buildbot route.
- **D-07:** Clearly separate published release, identified CI development
  artifact, and pinned local build. Recheck published tag/assets during
  implementation rather than copying milestone-opening status as current.
  Package publication is not required. Never assign merged-source capabilities
  to an older release or infer supported games from a conformance percentage.
- **D-08:** Core name/semantic version in the UI is a useful first check, not
  proof of exact source/build. Identify the actual loaded module path plus its
  digest and release checksum/provenance or CI run/artifact/source identity.
  Describe explicit core selection/log inspection where available; if exact
  identity cannot be established, report it unknown. No new build-ID API or
  behavior change is needed just for this documentation requirement.
- **D-09:** Verify archive layout/install extraction with existing permitted
  no-ROM/test fixtures. Distinguish six-target package checks from live host
  execution coverage; a build-tree host test is not a downloaded-package test.
  Do not require live six-platform/device qualification or commercial content.

### Host controls through the real launch

- **D-10:** RetroArch owns device assignment, remaps, hotkeys and storage.
  Describe verification through the same ES-DE/Steam/RetroArch launch route used
  for play; a separately opened frontend may have different configuration.
  Keep personal paths, identifiers and device-specific private artifacts out.
- **D-11:** With one controller, assign P1 and leave P2 unassigned; with two
  players, select distinct intended devices for the active ports. Explain the
  host port/device distinction. Do not add core-side device/mapping code or
  present port assignment as a fix for unclassified slowdown/rendering reports.
- **D-12:** Credit owned two-port tests and the recorded actual-launch input
  observations, each for what it proves. Callback/logical-port coverage does not
  prove host routing; observed input does not establish every button, reconnect
  or handheld fallback. New owner play or sign-off is not an acceptance gate.

### Progress, compatibility and rollback

- **D-13:** Explain battery saves, core snapshots, host disk containers, rewind
  and automatic loading separately. Find the host's configured save/state
  directories; do not prescribe one universal path. Preserve existing library
  entries and core-specific snapshot storage. Native payloads are not RetroArch
  disk state files and renaming does not convert them.
- **D-14:** Recommend exact-core-build and exact-content pairing for snapshots
  as the documented supported baseline. State current native format/identity
  checks separately: format 3 rejects prior formats; behavior revision is
  recorded but not enforced as a compatibility check. This stricter support
  recommendation adds no loader enforcement, converter, cross-core restore,
  cross-build compatibility matrix or future-format promise.
- **D-15:** Before upgrade or old-state loading, close the host cleanly and
  separately back up newest battery progress, the prior core, content identity,
  host/config identity, snapshots and their paired battery data. Verify copy
  integrity with digests, not mtimes. Snapshots contain battery RAM too: loading
  an older snapshot can replace newer battery progress. Keep the newest backup
  outside the active paths used by the walkthrough.
- **D-16:** The walkthrough uses only disposable scratch configs/storage and
  permitted fixtures. Cover a compatible positive roundtrip, deliberate native
  incompatible-state refusal, unchanged machine/save generation, and rollback
  preparation/recovery. Reuse existing independent native rejection tests for
  the refusal oracle; do not relabel that result as host-container refusal.
  No daily saves, other-core state directories or private content are test inputs.
- **D-17:** For fresh-start rollback, preserve the newest battery copy and
  disable/isolate automatic state loading before launching the retained core.
  Reusing newer battery data with an older core is conditional on identical
  content/board/save geometry and any demonstrated compatibility. Current
  same-build handoff evidence does not establish cross-build SRAM compatibility.
  If this is unknown, preserve both backup sets and report the limit rather than
  overwriting newer data or automatically restoring an old paired snapshot.
- **D-18:** Recovery instructions for an unsupported auto-state must tell the
  player how to keep backups intact, isolate the offending state/disable auto-
  loading and attempt an identified fresh launch. Existing RetroArch evidence
  establishes same-core auto-load and one measured container, not incompatible
  auto-load recovery with newer SRAM preserved. Claim that stronger behavior
  only if a bounded scratch host check establishes it; otherwise state the limit.

### Executable contracts and bounded verification

- **D-19:** Preserve independent install/archive, API/status/state, runner
  options/stop/exit and info/release checks. Update maintained document lookup
  paths and release-marker ownership deliberately. Retain bidirectional
  help/header/doc comparisons, enum/table equality and all unique negative
  controls. No assertion deletion, self-confirming generated snapshot or broad
  merged-corpus search that lets stale copies satisfy a missing canonical claim.
- **D-20:** Cover relocation failure specifically: a missing canonical guide,
  missing/contradictory status or runner contract, or obsolete README-only copy
  must not pass. Existing info/archive self-tests do not prove moved API/runner
  source lookup. Add only small owned controls for those actual gaps; verify
  README guide links and fragments without adding a Markdown tool dependency.
- **D-21:** Documentation-only changes run relevant document/package checks
  and their controls. Reuse existing native state/save and host evidence rather
  than rerunning expensive campaigns. A necessary runtime behavior change must
  first demonstrate its gap and carry an owned regression, same-change README/
  public-header comments and `cmake --workflow --preset ci`. Preserve hashes,
  no-skip, performance gates and unresolved collector/platform/provenance debt.
- **D-22:** Reuse owned C/CMake/CTest and platform facilities; prefer a small
  helper or justified local duplication to a dependency. No GUI/device code,
  shared framework, new ROM asset or copied emulator implementation. Existing
  live-testing stops/allowances remain unchanged. Work on the phase branch;
  this command ends after context/log/state capture. Planning, implementation,
  push, release and merge are separate steps.

### Agent discretion and evidence still to obtain

Exact guide names, section anchors, minimal CMake argument wiring, test grouping,
fixture selection and concise wording remain planner/researcher choices within
the decisions above. Resolve actual release/menu details from primary sources
when implementing. Inspect `tests/roms/manifest.txt` before selecting a fixture;
the existing Holy Mapperel battery fixture is a candidate, not new content.

There are no unresolved owner choices. The release identity refresh, moved-path
controls, scratch procedure and document checks remain implementation evidence
to obtain. No new install, refusal/recovery or gameplay outcome was demonstrated
during discussion.
</decisions>

<canonical_refs>
## Canonical References

Downstream agents must read the scope/evidence authorities and relevant contract
owners below; source references anchor existing behavior, not new requirements.

### Scope and existing findings
- `AGENTS.md` — ownership, clean-room, dependency, verification and step boundaries.
- `.planning/REQUIREMENTS.md` — READY-01 through READY-06 and acceptance limits.
- `.planning/ROADMAP.md` — Phase 22 success criteria and Phase 23 boundary.
- `.planning/PROJECT.md` — three deliverables and deterministic core constraints.
- `.planning/research/milestone-v6-discovery/SYNTHESIS.md` — measured remaining delta.
- `.planning/research/milestone-v6-discovery/ADVERSARIAL-REVIEW.md` — behavior-revision warning and scope traps.
- `.planning/playtesting/POSTMORTEM.md` — existing title evidence and unclassified symptoms.
- `.planning/playtesting/LESSONS.md` — real-launch, storage and scene-first lessons.
- `.planning/playtesting/CONTROLS.md` — host input observations and limits.
- `.planning/playtesting/AUTOMATION.md` — private artifacts, scratch work and runtime stops.
- `.planning/preparation/README.md` — reference index; use its relevant subsystem source.
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` — host/package/runner reference.
- `.planning/preparation/ENGINEERING.md` — existing build/test/release mechanisms.

### Product and executable contract owners
- `README.md` — existing prose, marked install block and support summary to retain.
- `include/nesturbator.h` — API/status, native state and battery semantics.
- `src/state.c` — current validate-before-apply and identity checks.
- `libretro/libretro.c` — host identity, serialization and battery integration.
- `libretro/nesturbator_libretro.info` — host capabilities and displayed metadata.
- `runner/exit_status.h` — authoritative runner exit values.
- `cmake/packaging.cmake` — six-target archive layout.
- `release-please-config.json` — marked version ownership.
- `.github/workflows/release.yml` — existing publication/checksum/provenance route.
- `tests/CMakeLists.txt` — registered document and behavior checks.
- `tests/cmake/check_install_line.cmake` — exact macOS extraction contract.
- `tests/cmake/check_archives.cmake` — inventories and archive/metadata controls.
- `tests/cmake/status_table.cmake` — API enum/table and state-warning checks.
- `tests/cmake/runner_options.cmake` — runner help/document option equality.
- `tests/cmake/runner_exit_table.cmake` — runner stop/exit contract equality.
- `tests/cmake/info_claims.cmake` — info/header/README support and negative controls.
- `tests/cmake/release_markers.cmake` — marked release-file coverage.
- `tests/release/two_versions.md` — independent release-version negative control.
- `tests/core/test_controller.c` — logical player-port coverage.
- `tests/core/test_state_reject.c` — format/identity refusal atomicity.
- `tests/core/test_state_generation.c` — battery generation on state load/refusal.
- `tests/libretro/libretro_host.c` — actual adapter battery/state behavior.
- `tests/retroarch/run_retroarch.cmake` — current host progression/state evidence and limits.
- `tests/roms/manifest.txt` — permitted committed fixtures.

### External primary documentation
- [RetroArch CLI](https://docs.libretro.com/guides/cli-intro/) — explicit core selection and verbose logging; verify installed-host support before prescribing details.
- [RetroArch input and controls](https://docs.libretro.com/guides/input-and-controls/) — host device/port ownership; menus/config vary with host.
</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- Existing CMake document parsers, archive allowlists and release controls avoid a new docs system.
- Native rejection/generation tests already cover all-or-nothing refusal; adapter and host fixtures cover complementary, narrower paths.
- Existing two-port tests and dated real-launch observations avoid a duplicate host framework.

### Established Patterns
- The public header and code define behavior; independently checked prose explains it.
- Release-please markers own selected version text; generated archives are checked against exact inventories.
- Native states validate completely before apply; host disk wrapping and automatic loading are separate layers.
- Owned fixtures, focused fault controls and CTest provide evidence without a new dependency.

### Integration Points
- No codebase maps were present; scouts used relevant README, packaging, tests, public header and owned adapter/state code.
- API and runner relocation touches their document-path arguments/parsers and registration; support/install checks can retain README ownership.
- Versioned source documentation links do not require adding docs to package inventories.
</code_context>

<specifics>
## Specific Ideas

“Another copy and paste is better than another dep” means reuse first, accept
justified small owned duplication, and keep any demonstrated abstraction local.
Use relevant player, API-consumer, release-engineering, portability, QA, security/
data-loss and maintenance lenses. Unrelated UI stacks, distributed systems and
graphics-engine ecosystems add no phase requirement. Further generic fan-out
has diminishing value after the concrete migration/safety traps are addressed.
</specifics>

<deferred>
## Deferred Ideas

Phase 23 report/replay/triage, actual Playstead integration, accuracy/overscan,
regions/mappers, cross-build state qualification/conversion, new packaging/docs
distribution and recurring collector repair stay outside Phase 22. Preserve
existing debt and safety stops; none is waived by this discussion.

### Reviewed Todos (not folded)
- `.planning/todos/pending/2026-10-26-add-reproducible-owned-property-tests.md` — broad testing proposal, no direct READY requirement gap.
- `.planning/todos/pending/2026-10-26-make-runner-signal-checks-deterministic.md` — separate signal-test redesign; retain existing stop checks here.
- `.planning/todos/pending/2026-10-26-evaluate-scoped-nightly-mutation-testing.md` — separate nightly/tooling proposal; focused owned relocation controls suffice.

The match tool returned broad testing keywords; the approved milestone already
reviewed these proposals. No todo is promoted or marked complete.
</deferred>

---
*Phase: 22-install-and-protect-progress*
*Context gathered: 2026-10-30*
