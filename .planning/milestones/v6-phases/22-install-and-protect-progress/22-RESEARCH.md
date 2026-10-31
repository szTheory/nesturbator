# Phase 22: Install and protect progress - Research

**Researched:** 2026-10-30  
**Domain:** Player documentation, CMake/CTest documentation contracts, libretro package identity, save and state recovery  
**Confidence:** MEDIUM

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

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
  observations, each for what it proves. Callback/logical-port coverage does
  not prove host routing; observed input does not establish every button,
  reconnect or handheld fallback. New owner play or sign-off is not an
  acceptance gate.

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
  No daily saves, other-core state directories or private content are test
  inputs.
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
  only if a bounded scratch host check establishes it; otherwise state the
  limit.

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

### Deferred Ideas (OUT OF SCOPE)

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
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| READY-01 | A player can use a concise README to find current supported content, installation, controls, progress and troubleshooting, with detailed developer material available through working topic-guide links. | README order and guide ownership are locked; the canonical-source relocation and link controls remain the planned gap. [CITED: .planning/phases/22-install-and-protect-progress/22-CONTEXT.md] |
| READY-02 | A player can select and install the appropriate existing core archive for their platform/architecture and identify the loaded core/build, distinguishing published-release capabilities from merged development behavior. | Six target names, local archive checker, macOS extraction checker, and official host install/identity guidance are mapped below. [VERIFIED: cmake/packaging.cmake:5-53; tests/cmake/check_archives.cmake:635-693; tests/cmake/check_install_line.cmake:1-15] |
| READY-03 | A player can follow host-owned control guidance to verify input through the actual frontend launch and avoid accidentally assigning one controller to both active player ports. | Official RetroArch port/device steps plus existing actual-launch evidence support the guidance; evidence limits are explicit. [CITED: https://docs.libretro.com/guides/input-and-controls/] |
| READY-04 | A player can distinguish battery saves, core-specific snapshots, rewind and automatic loading, and locate the host-configured storage that holds their progress. | Header/README define core save/state boundaries; official Directory Configuration says to inspect the user's configured directories. [VERIFIED: README.md:1285-1305,1334-1339; CITED: https://docs.libretro.com/guides/change-directories/] |
| READY-05 | A player can follow an explicit supported-version, backup and rollback procedure that preserves newer progress and describes incompatible-state refusal without promising conversion or cross-core snapshot support. | State format identity, refusal atomicity, inclusion of battery RAM, and existing independent test oracles are verified. [VERIFIED: include/nesturbator.h:82-100,573-590; README.md:1368-1382,1402-1423] |
| READY-06 | A developer can find the relocated API, runner and release contracts, and the existing automated documentation/package checks still reject missing or contradictory claims at their maintained document locations. | Current CMake consumers and archive/release check boundaries are mapped; moved-path failure controls are called out as new work. [VERIFIED: tests/CMakeLists.txt:1624-1643,1719-1730,2330-2362] |
</phase_requirements>

## Summary

Phase 22 is a documentation and CMake contract migration around existing C library, runner and libretro behavior; it does not need a package or a core change. [VERIFIED: AGENTS.md:14-16,25-45; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md, D-21/D-22]

The strongest implementation seam is state.status_table: it reads the public header and README, checks enum/table equality in both directions, and requires the rewind warning. Runner option and exit-table checks also read README directly. Retarget these consumers to the intended canonical guides, then add small negative controls proving a missing or contradictory canonical guide fails even when stale README text remains. [VERIFIED: tests/cmake/status_table.cmake:10-68; tests/cmake/runner_options.cmake:12-45; tests/cmake/runner_exit_table.cmake:59-67,134-157]

For installation and recovery, preserve the current archive layout and reuse local CMake fixtures. Native state checks enforce format and cartridge identity; the behavior revision is recorded but not enforced. Battery RAM is included in a whole-machine snapshot, so restoring an older snapshot can replace newer battery progress. Do not imply native refusal proves RetroArch-container recovery or cross-build battery compatibility. [VERIFIED: include/nesturbator.h:82-100,573-590; README.md:1402-1423; .planning/playtesting/POSTMORTEM.md:62-65]

**Primary recommendation:** Plan a tracer-first API guide migration with one README link and a failing canonical-location control; expand the explicit-source pattern to runner/release/testing guides; then finish the player install/control/progress route and scoped documentation/package checks. Refresh release identity when implementing; current release availability was not observed in research. [ASSUMED]

### Planning decomposition to feed the planner

1. **Tracer-first: API/status end-to-end slice.** Make the README player route link to one canonical API guide; move the detailed status/native-state text there; retarget status_table.cmake; add a focused negative control where the canonical guide is missing/contradictory but the old README copy remains; and check that README target/fragment links resolve. Keep the support summary and rewind warning where their current support check requires them. This first slice proves the full link → maintained guide → source contract → failure control path. [VERIFIED: tests/cmake/status_table.cmake:10-68; tests/CMakeLists.txt:1624-1628,2340-2346; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-03/D-04/D-20]
2. **Expand developer contracts.** Move runner options/stop/exit details, release/package process, and engineering checks into their canonical guides. Retarget runner_options.cmake and runner_exit_table.cmake to the canonical runner guide while preserving --help/header/doc equality and unique stop/exit checks. Keep release marker ownership in release-please-config.json and retain its one-version-per-marked-line self-test. Add relocation controls that prove an old README copy cannot hide a missing guide claim. [VERIFIED: tests/cmake/runner_options.cmake:30-51; tests/cmake/runner_exit_table.cmake:59-67,134-157; tests/CMakeLists.txt:1624-1643,1719-1730; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-03/D-19/D-20]
3. **Complete the player installation/control/progress route.** Rewrite README in the locked player order; keep its release-marked Apple Silicon block and support summary. Document selecting the existing package for the actual host executable architecture, identify the loaded module/build by path+digest/provenance, and cite configured host directories and Device Index controls. Add the scratch-only save/upgrade/refusal/rollback walkthrough and explicit unknown-compatibility recovery limit. Reuse the generated runner battery fixture and existing native refusal/generation evidence; run the local archive check and relevant document/package checks, plus the macOS install-line check only on macOS. No host/device operation or expensive state campaign is part of this phase. [VERIFIED: tests/cmake/check_archives.cmake:1-11,635-693; tests/cmake/check_install_line.cmake:1-15,114-130; tests/CMakeLists.txt:1027-1033,1081-1091,150-183; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-06/D-09/D-13/D-18/D-21; ASSUMED for proposed grouping]

The battery fixture is generated in the build tree and has independent roundtrip/mismatch cases:

> `set(NESTURBATOR_SAVE_FIXTURE ${CMAKE_CURRENT_BINARY_DIR}/save_fixture)`  
> `add_test(NAME runner.save.fixture`  
> `runner_save_case(roundtrip)`  
> `runner_save_case(mismatch)`

Native state refusal and save-generation oracles are separately registered:

> `nesturbator_core_test(core.state_generation core/test_state_generation.c)`  
> `nesturbator_core_test(core.state_reject core/test_state_reject.c ${FORK_ROMS})`

Use the runner fixture only for battery-file roundtrip/refusal, and the native tests as evidence for in-memory format/identity refusal and generation behavior. Do not run the costly native campaigns for this docs-only phase. [VERIFIED: tests/CMakeLists.txt:150-183,1027-1033,1081-1091; tests/cmake/runner_save.cmake:55-95]

These are coarse planning slices, not a proposed file-by-file task list; preserve the dependency that the tracer checks work before the later guides move. [ASSUMED]

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| README and topic guides | Repository/static docs | — | Human-facing guidance lives in versioned repository files; archives remain unchanged. [VERIFIED: .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-02/D-05] |
| Core selection, input/device assignment, autoload and storage paths | Frontend host | Repository docs | RetroArch owns device assignment and save/state paths; docs direct players to verify actual launch configuration. [CITED: https://docs.libretro.com/guides/input-and-controls/; https://docs.libretro.com/guides/change-directories/] |
| Native save-state validation and battery span | Core API | Host integration | The core owns serialization and exposes battery RAM; the host owns files and state buffers. [VERIFIED: README.md:1285-1305,1334-1339; include/nesturbator.h:573-590] |
| Archive naming, inventory, and release provenance | Build/release | Frontend host | CMake creates component ZIPs and checks local archive contents; the workflow checks the six-platform matrix and published assets. [VERIFIED: cmake/packaging.cmake:39-53; .github/workflows/release.yml:87-105,159-175] |
| Documentation/package regression checks | Build/test | Repository docs | CMake/CTest compares authoritative headers/help and documentation, and validates package inventory. [VERIFIED: tests/CMakeLists.txt:1624-1643,2330-2362; tests/cmake/check_archives.cmake:1-11,635-693] |

## Standard Stack

### Core

| Library/tool | Version | Purpose | Why standard |
|--------------|---------|---------|--------------|
| CMake + CTest | Project minimum CMake 3.25; local CMake/CTest 4.4.3 | Keep existing document, archive and install-line checks in the project's test/build system. | Existing tests are CMake scripts registered with CTest; no new test or Markdown framework is needed. [VERIFIED: CMakePresets.json:2-3,157-163; tests/CMakeLists.txt:1624-1643; local cmake/ctest version probes] |
| Existing C library/header, runner and libretro adapter | Current checkout | Source-of-truth contracts and package payloads. | The phase adds guidance and moves document consumers, not emulation behavior. [VERIFIED: AGENTS.md:14-16,25-28; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-21/D-22] |

### Supporting

| Tool | Version | Purpose | When to use |
|------|---------|---------|-------------|
| GitHub CLI | Local 2.101.0 (2026-10-30 probe) | Optional read-only check of current published tag/assets using the release workflow's asset contract. | Refresh release identity in implementation; do not copy the milestone-opening release snapshot as current. [VERIFIED: local gh version probe; .github/workflows/release.yml:159-175] |
| RetroArch documentation | Official docs pages consulted 2026-10-30 | Menu, core extraction, logging and directory instructions. | Cite host-owned steps and remind players that paths/configuration vary. [CITED: https://docs.libretro.com/guides/download-cores/; https://docs.libretro.com/guides/input-and-controls/; https://docs.libretro.com/guides/change-directories/; https://docs.libretro.com/guides/cli-intro/; https://docs.libretro.com/guides/generating-retroarch-logs/] |

### Alternatives Considered

| Instead of | Could use | Tradeoff |
|------------|-----------|----------|
| Existing CMake parsers and small local controls | A Markdown parser/framework or generated documentation corpus | Adds dependency/surface area and could let duplicated stale text satisfy a missing canonical guide; context excludes a docs framework. [VERIFIED: AGENTS.md:60-68; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-19/D-20/D-22] |
| Existing local package checks plus evidence-qualified host instructions | A new installer or live six-platform host matrix | Expands packaging/host scope and is not an acceptance requirement. [VERIFIED: .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-06/D-09] |

**Installation:** No external package installation is needed for this phase. [VERIFIED: .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-05/D-22]

## Project Constraints (from AGENTS.md)

- **Ownership and deliverables:** Emulation and tests are owned here; libretro.h is the only vendored file; the core links only C memory functions. Keep the library, headless runner and libretro adapter as the three deliverables; add no window/audio/input-device code. [VERIFIED: AGENTS.md:25-28]
- **Clean tree and clean room:** Only manifest-listed test ROMs are permitted; keep personal paths, names, email, private captures and other raw artifacts out of Git. Use the GitHub noreply commit identity; scripts/hygiene.sh checks commits and pushes. Do not open or copy GPL/LGPL emulator source. [VERIFIED: AGENTS.md:29-35,89-90]
- **Deterministic core:** Keep integer-only, instance-owned state and platform-identical output hashes; this documentation phase should not make core changes without demonstrated need. [VERIFIED: AGENTS.md:36-37; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-21]
- **Tests and docs together:** If behavior changes, add an owned test, update README and public-header comments in the same change, and run cmake --workflow --preset ci; automated checks must not wait on a person. Documentation-only work uses relevant checks. [VERIFIED: AGENTS.md:38-41; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-21]
- **Handoffs and workflow boundary:** At a GSD stopping point provide one exact next command and what it does; update STATE with the actual outcome and next command, link a concise resume file when needed, and read STATE/resume before resuming. Keep detailed evidence in linked artifacts and continue authorized work inside the current step, stopping at the required boundary. Work on the phase branch, one GSD step per command; PR merge uses a Conventional Commit title. This research assignment itself does not edit STATE or commit. [VERIFIED: AGENTS.md:3-12,42-45; task scope]
- **Dependencies and style:** Prefer removing need, reuse, standard library, platform, then minimum owned code. Prefer a small helper/local duplication over another dependency; keep modules/control flow small and hardware comments sourced. [VERIFIED: AGENTS.md:60-68]
- **Reference and personal-play evidence:** Use .planning/preparation/README.md and its linked subsystem reference before designing/building. Do not make new owner gameplay/sign-off a gate. If recommending play, read scene-first lessons and state scene, known quirks, expected/failure behavior, access route and effort; use existing library entries and persistent saves, with per-game core selection, and preserve other cores' incompatible state. Keep dated title/exact-scene evidence separate. Read automation direction before converting play findings into CI; use owned fixtures with independent oracles. [VERIFIED: AGENTS.md:18-21,70-90; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-10/D-12/D-21/D-22]
- **Milestone upkeep:** Before new/complete-milestone steps, read PRODUCT-ROADMAP with project state, latest audit, seeds and release evidence; reconcile delivered/partial outcomes, preserve debt, refresh all four horizons/triggers, record ordering rationale, and present preferred scope/alternatives. Commit that roadmap with the step artifacts; only approved scope is Immediate, future scope remains proposed, and archive agreed direction at closeout. Do not run another lifecycle command for this upkeep. This phase research is not a milestone boundary. [VERIFIED: AGENTS.md:47-56]

## Architecture Patterns

### System Architecture Diagram

~~~text
Player
  ├── README: supported limits → install/core identity → content → controls → progress → troubleshooting
  │       └── topic links → API / runner / releases / testing / progress guides
  ├── actual frontend launch → selected libretro module + content
  │       ├── host-configured battery-save directory ← core battery RAM span
  │       └── host-configured snapshot directory ← native core state inside host-owned container
  └── recovery flow → clean close → independent backups/digests → identified core/content pair
                  → isolated scratch restore/refusal checks → preserve both sets if compatibility is unknown

Header/help + canonical guide ──→ existing bidirectional CMake contract checks
Local ZIPs ─────────────────────→ archive inventory + .info/support claim checks
README install block + local ZIP → macOS extraction checker (no release download)
Release workflow ───────────────→ 18 ZIPs + checksums/attestation + published asset check
~~~

The diagram reflects ownership and evidence boundaries: native payloads are not host disk containers, and a local build-tree adapter test is not a downloaded-release archive test. [VERIFIED: README.md:1336-1339,1402-1405; tests/cmake/check_install_line.cmake:1-15; tests/CMakeLists.txt:1774-1788; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-09/D-13]

### Recommended Project Structure

~~~text
README.md                 # player-first route, short warnings, marked install block, support summary
docs/api.md               # API, status and native-state contract
docs/runner.md            # runner options, stop and exit contract
docs/releases.md          # package targets, identity and maintainer release process
docs/testing.md           # CMake/CTest and evidence limits
docs/progress.md          # player save/state/backup/upgrade/rollback procedure
tests/cmake/              # existing contract checks plus small relocation/link controls
~~~

The guide names are the selected preferences and equivalent destinations are allowed. Inventory of docs returned no directory in this checkout, so the planner should confirm no equivalent canonical files before creating duplicate homes. [VERIFIED: .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-02; rg --files docs inventory returned “No such file or directory”]

### Pattern 1: Explicit canonical document inputs

**What:** Keep header/help as authoritative sources, pass the expected canonical guide path to each existing CMake checker, and compare contract text bidirectionally. Make the checker fail if the canonical path is missing or contradictory, even if README retains obsolete text. [VERIFIED: tests/cmake/status_table.cmake:10-68; tests/cmake/runner_options.cmake:12-45; tests/cmake/runner_exit_table.cmake:59-67,134-157; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-19/D-20]

**When to use:** Whenever a unique contract moves from README to its topic guide. Retain README's short link/warning while moving the maintained detailed copy. [VERIFIED: .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-02/D-03/D-19/D-20]

The existing registered test is:

~~~cmake
add_test(NAME state.status_table
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/status_table.cmake)
~~~

The new guide argument and relocation self-test are proposed, not existing test values. [VERIFIED: tests/CMakeLists.txt:1624-1628; ASSUMED for proposed control]

### Pattern 2: Local README target/fragment validation

Add a narrow owned CMake check for required README guide links and heading fragments, with a self-test that breaks one target or fragment and expects failure. Do not fetch URLs or introduce a Markdown dependency. Keep scope to local paths and linked fragments, not general Markdown linting. [ASSUMED; constrained by .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-20/D-22]

### Pattern 3: Package identity is a chain of evidence

Explain three distinct artifacts: published release, identified CI artifact, and pinned local build. Identify the actual module selected by the real launch route, record its path and SHA-256, then match checksum/attestation or CI run/artifact/source commit. If any link cannot be established, say “unknown”; UI name/version alone is not exact build identity. [VERIFIED: .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-07/D-08; .github/workflows/release.yml:106-110,159-175; CITED: https://docs.libretro.com/guides/cli-intro/]

The archive contract is three components × three OS names × two architectures. Local package checking expects three archives for the current platform; the release workflow assembles 18 archives and SHA256SUMS. [VERIFIED: cmake/packaging.cmake:5-53; .github/workflows/release.yml:87-105]

> `set(name_re "^nesturbator-([0-9]+\\.[0-9]+\\.[0-9]+)-(library|runner|libretro)-(linux|macos|windows)-(x64|arm64)\\.zip$")`

This exact archive-name rule is the local identity fixture's filename contract. [VERIFIED: tests/cmake/check_archives.cmake:652-653]

> `set(CPACK_COMPONENTS_ALL library runner libretro)`  
> `for c in library runner libretro; do`  
> `for o in linux macos windows; do`  
> `for a in x64 arm64; do`

The quoted component/platform/architecture values are the packaging and release-workflow source values. [VERIFIED: cmake/packaging.cmake:39-53; .github/workflows/release.yml:91-97]

### Pattern 4: Safe progress walkthrough and bounded oracles

Keep battery save files, native snapshots, host disk containers, rewind and autoload separate. Back up the newest battery save, old core, content identity, host/config identity, snapshots and paired battery data outside active host paths; compare digests. Use a permitted fixture and disposable storage for positive roundtrip and native refusal; leave unresolved cross-build battery compatibility as “preserve both; do not overwrite.” [VERIFIED: .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-13/D-15/D-16/D-17; README.md:1285-1305,1402-1423]

For upgrades, rollback, rejected auto-loaded state, and scratch demonstrations, do not use daily save/state directories or claim a native refusal check proves host-container recovery. [VERIFIED: .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-16/D-17/D-18]

## Don't Hand-Roll

| Problem | Don't build | Use instead | Why |
|---------|------------|-------------|-----|
| Markdown link checking | General parser or network crawler | Small CMake checker for README's local guide links/fragments plus a negative fixture | No dependency is selected; target the actual relocation risk. [VERIFIED: .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-20/D-22] |
| API/runner/release contract synchronization | Second manually maintained/generated corpus | Existing header/help parsers pointed at canonical guides, retaining bidirectional and negative checks | A stale README copy must not satisfy a missing canonical claim. [VERIFIED: tests/cmake/status_table.cmake:10-68; tests/cmake/runner_options.cmake:30-51; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-19/D-20] |
| Save-state conversion or compatibility | Converter, cross-core restore, or broad matrix | Exact-build/exact-content support baseline and refusal/backup procedure | Current format rejects older formats; behavior revision is not enforced. [VERIFIED: include/nesturbator.h:82-100; README.md:1419-1423; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-14/D-17] |
| Archive layout verification | New installer or package-layout duplicate | Existing archive checker and macOS install-line checker | These inspect generated local archives and extraction layout; they do not prove live host operation. [VERIFIED: tests/cmake/check_archives.cmake:1-11,635-693; tests/cmake/check_install_line.cmake:1-15,114-130] |
| Host input/storage handling | Core-side mappings or a universal save path | RetroArch device-index and Directory settings guidance | Host owns input/storage and per-install paths vary. [CITED: https://docs.libretro.com/guides/input-and-controls/; https://docs.libretro.com/guides/change-directories/; VERIFIED: .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-10/D-11/D-13] |

**Key insight:** The main risk is moving contract prose while leaving its checker pointed at README; another is treating a host snapshot wrapper, native payload, and battery file as interchangeable. [VERIFIED: tests/cmake/status_table.cmake:26-38; tests/cmake/runner_options.cmake:30-45; README.md:1336-1339,1402-1423; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-13/D-19/D-20]

## Common Pitfalls

### Pitfall 1: README remains an accidental second source of truth

**What goes wrong:** A moved guide is missing or wrong, but an old README section still lets the test pass. **Why:** Current checkers parse README paths directly. **Avoid:** Pass canonical document paths into parsers; add targeted missing/contradiction/stale-copy controls; leave README with linked summaries. **Warning sign:** A checker reports success after the guide's contract has been deleted. [VERIFIED: tests/cmake/status_table.cmake:26-38; tests/cmake/runner_options.cmake:30-45; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-19/D-20]

### Pitfall 2: UI label or version is mistaken for exact module identity

**What goes wrong:** A player attributes merged-source behavior to an older or different binary. **Avoid:** Record loaded module path and digest, actual host launch route, and matching release checksum/provenance or CI artifact/source identity; if path or source cannot be proven, mark it unknown. The CLI guide supports explicit -L selection and verbose logs, but does not establish that every GUI/ES-DE launch logs an exact module path. [VERIFIED: .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-07/D-08; CITED: https://docs.libretro.com/guides/cli-intro/; .github/workflows/release.yml:106-110,159-175]

### Pitfall 3: Older snapshot silently replaces newer battery progress

**What goes wrong:** Loading a snapshot changes battery bytes and the host later persists older progress. **Avoid:** Back up battery save separately, then snapshots and paired battery data; keep the latest copy outside active paths. Refusal is all-or-nothing, but accepted snapshots include PRG RAM. [VERIFIED: README.md:1368-1372,1402-1417; include/nesturbator.h:573-590; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-15/D-17]

### Pitfall 4: Native refusal is described as host-container recovery

**What goes wrong:** A native format/identity check is presented as proof that RetroArch recovers safely from incompatible autoload. **Avoid:** Call it a native-state refusal oracle only; qualify recovery and preserve backups unless a bounded scratch-host check is separately authorized and run. [VERIFIED: .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-16/D-18; tests/CMakeLists.txt:1081-1091; .planning/playtesting/CONTROLS.md:40-44]

### Pitfall 5: A port is confused with a physical device

**What goes wrong:** One controller is assigned to both active ports or a port change is misrepresented as fixing slowdown. **Avoid:** Explain Port 1/Port 2 versus Device Index; assign P1 and leave P2 unassigned for one player, or select distinct devices for two players. Verify through actual launch config. [CITED: https://docs.libretro.com/guides/input-and-controls/; VERIFIED: .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-10/D-12; .planning/playtesting/CONTROLS.md:51-58]

### Pitfall 6: Local package check is mistaken for six-platform execution

**What goes wrong:** A current-host ZIP inventory or build-tree host test is advertised as downloaded-release/live-host validation across every target. **Avoid:** Distinguish local three-component package validation, release workflow's 18 archive matrix, local macOS extraction and live frontend behavior. [VERIFIED: tests/cmake/check_archives.cmake:1-11,646-693; tests/CMakeLists.txt:1774-1788; .github/workflows/release.yml:87-105; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-09]

## Code Examples

### Verify local archive inventory

~~~sh
cmake -DDIR=build/ci -P tests/cmake/check_archives.cmake
~~~

README source quote: `cmake -DDIR=build/ci -P tests/cmake/check_archives.cmake`. It requires archives already produced for the current local platform and checks name/inventory/metadata; it does not download a published release. [VERIFIED: README.md:1490-1495; tests/cmake/check_archives.cmake:1-11,635-693]

### Verify the existing macOS install block against a local package

~~~sh
cmake -DREADME=README.md -DPACKAGES=build/ci/packages -DOUT=build/ci/install-line -P tests/cmake/check_install_line.cmake
~~~

README source quote: `cmake -DREADME=README.md -DPACKAGES=build/ci/packages -DOUT=build/ci/install-line -P tests/cmake/check_install_line.cmake`. The checker requires exactly `cores/nesturbator_libretro.dylib` and `info/nesturbator_libretro.info` in the scratch extraction. [VERIFIED: tests/cmake/check_install_line.cmake:114-130]

Run only on macOS: the script relies on system tar reading ZIP from stdin, extracts from a generated package, and explicitly does not fetch the release URL. [VERIFIED: README.md:2045-2049; tests/cmake/check_install_line.cmake:1-15,114-130]

### Focused existing document/package tests after configure/build

~~~sh
ctest --test-dir build/ci --output-on-failure -R 'state.status_table|runner.options.readme|runner.exit|release.one_version_per_line|libretro.info_claims|libretro.info_support|libretro.info_staged|libretro.info_archives.self_test'
~~~

The test roots are registered in CMake. Their source contains `add_test(NAME state.status_table`, `foreach(mode table help)`, `add_test(NAME runner.exit.${mode}`, `add_test(NAME runner.options.readme`, `add_test(NAME release.one_version_per_line`, `add_test(NAME libretro.info_claims`, `add_test(NAME libretro.info_support` and `add_test(NAME libretro.info_staged)`. [VERIFIED: tests/CMakeLists.txt:1624-1643,1719-1730,2330-2362]

The configured binary directory is derived from this exact preset source: `"binaryDir": "${sourceDir}/build/${presetName}"` and `"name": "ci"`. Add new canonical-guide/link negative-control names after choosing them. [VERIFIED: CMakePresets.json:6-10,17-20]

These are proposed commands and were not executed in research. Nyquist validation is explicitly disabled, so omit the template's Validation Architecture section while still planning scoped document/package checks. Security enforcement is explicitly false for this configuration. [VERIFIED: .planning/config.json:21-25,47-50; exact values: \"nyquist_validation\": false, and \"security_enforcement\": false,]

## State of the Art

| Old/current approach | Recommended current approach | When | Impact |
|----------------------|-----------------------------|------|--------|
| Detailed contracts live in a player README, and CMake checkers parse README directly. | Keep README as player route; move detailed API/runner/release/testing claims to one canonical topic guide and retarget consumers. | Phase 22, selected in context. | Links are the entry point while checks compare claims against header/help and fail on missing canonical text. [VERIFIED: README.md:6-37; tests/cmake/status_table.cmake:26-38; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-01/D-03/D-19/D-20] |
| A state records behavior revision, which could be mistaken for an enforced compatibility gate. | Explain the strict supported baseline as exact core build plus exact content; state separately that format/identity are enforced and behavior revision is not. | Existing native contract; restate in Phase 22. | No converter or loader enforcement is implied. [VERIFIED: include/nesturbator.h:82-100; README.md:1374-1382,1419-1423; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-14] |
| UI name/version and release-opening snapshot can be treated as current exact identity. | Tie claims to loaded module path/digest and current release or CI evidence; refresh release assets at implementation. | Per installation/upgrade. | Do not ascribe current source capabilities to a stale release. [VERIFIED: .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-07/D-08; .planning/REQUIREMENTS.md:25-31; .github/workflows/release.yml:159-175] |

The README begins with a stale “Status: v3” description and older behavior revision prose; its exact claim is “The behaviour revision is 8:”. The public header currently declares the values below. This is a documentation correction called for by D-01, not evidence that a published release has the same features. [VERIFIED: README.md:6-37; include/nesturbator.h:48-53,78-100]

> #define NESTURBATOR_ABI_VERSION 1  
> #define NESTURBATOR_BEHAVIOUR_REVISION 9  
> #define NESTURBATOR_STATE_FORMAT_VERSION 3

The header also states: `A fix that changes only emulated output raises NESTURBATOR_BEHAVIOUR_REVISION instead, which a state records but a load does not enforce.` The quoted values apply to the current checkout only; inspect the release package before making a release claim. [VERIFIED: include/nesturbator.h:53,80,96-100; release match unobserved]

**Release lookup status:** Public release lookups in this bounded research returned no usable current tag/assets observation. The requirements file says, as an opening snapshot, “At opening, release PR #56 is open and v0.1.11 is latest published”; do not copy that as current. The release workflow validates the selected tag, draft flag, URL and assets and expects all 18 ZIPs plus SHA256SUMS. [VERIFIED: research record; .planning/REQUIREMENTS.md:25-31; .github/workflows/release.yml:159-175]

## Assumptions Log

| # | Claim | Section | Risk if wrong |
|---|-------|---------|---------------|
| A1 | Use a three-slice sequence: tracer-first API, runner/release/testing expansion, then player install/controls/progress. | Summary / Architecture Patterns | Re-grouping may be needed if dependencies or task size require it; preserve ordering and requirement coverage. [ASSUMED] |
| A2 | A narrow CMake link/fragment checker with a negative fixture is the smallest adequate README link solution. | Architecture Patterns | It may miss Markdown edge cases; keep supported syntax explicit and tests local. [ASSUMED] |
| A3 | Create guide files under docs because inventory found no docs directory. | Recommended Project Structure | There may be equivalent destinations elsewhere; confirm before creating duplicate homes. [ASSUMED] |
| A4 | Exact release and host details can be refreshed during implementation from the workflow and official RetroArch docs. | Open Questions | If source, asset or menu details cannot be established, say unknown/varies rather than imply proof. [ASSUMED] |

## Open Questions

1. **Which release is currently published, and which assets/checksums/provenance match the target archive?**
   - What we know: the workflow validates a selected tag as non-draft, positive-size assets and the exact 18 ZIP plus checksum list. The requirements' v0.1.11 status is only an opening snapshot. [VERIFIED: .github/workflows/release.yml:159-175; .planning/REQUIREMENTS.md:25-31]
   - What's unclear: the current tag and asset availability were not observed. [VERIFIED: research record; no release lookup performed]
   - Recommendation: refresh in implementation with the project's read-only release check; cite the inspected tag and matching module digest. If not reachable, report no observation. [ASSUMED]
2. **Can the target RetroArch/ES-DE launch expose exact loaded module path and an unassigned P2 choice in its current UI?**
   - What we know: official docs explain core selection, verbose logging, Port 1/2 device-index assignment and configured directories. Existing observations cover one actual route but not every button/reconnect/fallback. [CITED: https://docs.libretro.com/guides/cli-intro/; https://docs.libretro.com/guides/input-and-controls/; https://docs.libretro.com/guides/change-directories/; VERIFIED: .planning/playtesting/LESSONS.md:53-60]
   - What's unclear: precise label/path visibility in every host version/launch frontend; this research did not operate a host. [VERIFIED: research record; no host operation performed]
   - Recommendation: document official settings route, ask players to verify actual launch configuration, and say identity is unknown if exact path/log evidence is absent; do not require new owner play. [ASSUMED]
3. **Does the released/older core safely reuse newer battery data, or does the host recover from incompatible autoload without overwriting it?**
   - What we know: native format/identity refusal is all-or-nothing; snapshots include battery RAM; existing host evidence only proves same-core autoload. [VERIFIED: include/nesturbator.h:573-590; README.md:1402-1423; .planning/playtesting/CONTROLS.md:40-44]
   - What's unclear: cross-build SRAM compatibility and incompatible host-container recovery. [VERIFIED: .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-17/D-18; .planning/playtesting/CONTROLS.md:40-44]
   - Recommendation: leave both backup sets intact and state the limit. No such runtime/host test is authorized or needed for this documentation phase. [VERIFIED: .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-17/D-18/D-21]

## Environment Availability

| Dependency | Required by | Available | Version | Fallback |
|------------|-------------|-----------|---------|----------|
| CMake | Existing configure/build and script checks | ✓ | 4.4.3 [VERIFIED: local cmake --version probe] | — |
| CTest | Scoped registered checks | ✓ | 4.4.3 [VERIFIED: local ctest --version probe] | — |
| GitHub CLI | Optional current-release refresh | ✓ | 2.101.0 [VERIFIED: local gh --version probe] | Official release page; if unavailable, report no observation |
| macOS system tar behavior | Existing install-line checker | Platform-specific [VERIFIED: tests/cmake/check_install_line.cmake:13-15] | Not probed for host behavior [VERIFIED: research record] | Skip outside macOS; run on macOS in implementation if applicable |

No runtime tests or host operations were run; public release lookup attempts produced no usable current tag/assets observation. [VERIFIED: research record] The macOS install checker is platform-specific and local; its script states the URL is not fetched. [VERIFIED: tests/cmake/check_install_line.cmake:13-15]

## Security Domain

The configuration disables security enforcement. The phase's material risk is data loss: keep private saves/paths out of Git, maintain separate external backups, and never overwrite newer battery data on an unknown compatibility path. [VERIFIED: .planning/config.json:49; exact value: \"security_enforcement\": false,; AGENTS.md:29-32,89-90; .planning/phases/22-install-and-protect-progress/22-CONTEXT.md D-15/D-17]

## Sources

### Primary repository sources (HIGH confidence)

- .planning/phases/22-install-and-protect-progress/22-CONTEXT.md — locked decisions, boundaries and evidence gaps.
- .planning/REQUIREMENTS.md — READY requirements and distinction between opening release snapshot and current check.
- AGENTS.md — ownership, clean-room, determinism, documentation/test and playtesting constraints.
- README.md, include/nesturbator.h, src/state.c — current public behavior, battery/state semantics and identity checks.
- cmake/packaging.cmake, .github/workflows/release.yml — package target naming/layout and publication provenance checks.
- tests/CMakeLists.txt and tests/cmake/{status_table,runner_options,runner_exit_table,check_archives,check_install_line,release_markers}.cmake — document and package consumer/check boundaries.
- tests/core/test_state_reject.c, tests/core/test_state_generation.c, tests/cmake/runner_save.cmake, tests/roms/manifest.txt — refusal, generation, scratch save fixture and permitted ROM evidence.
- .planning/playtesting/{CONTROLS,LESSONS,POSTMORTEM}.md — real-launch/autoload findings and limits; private identities/paths not reproduced.

### Official host documentation (CITED)

- [Installing Cores](https://docs.libretro.com/guides/download-cores/) — manual core and matching info-file installation into configured host directories.
- [Input and Controls](https://docs.libretro.com/guides/input-and-controls/) — RetroPad port and Device Index controls.
- [Directory Configuration](https://docs.libretro.com/guides/change-directories/) — inspect/configure Save Files and Save States directories; defaults vary.
- [Starting a Game](https://docs.libretro.com/guides/starting-a-game/) and [CLI guide](https://docs.libretro.com/guides/cli-intro/) — explicit core selection and command-line/log evidence limits.
- [Generating Logs](https://docs.libretro.com/guides/generating-retroarch-logs/) — host logging settings and log location.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — existing CMake/CTest code and local tool versions inspected.
- Architecture: HIGH — locked ownership and source/check wiring read directly.
- Pitfalls: HIGH — supported by existing state, save, archive and parser sources; unknown host/release facts are kept open.

**Research date:** 2026-10-30  
**Valid until:** 2026-11-29 for stable CMake/test architecture; re-check release and RetroArch menu details during implementation.
