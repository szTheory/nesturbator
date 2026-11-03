# Phase 28: Qualified NES gameplay package - Context

**Gathered:** 2026-11-03
**Status:** Ready for planning

<domain>
## Phase Boundary

Deliver a published current Nesturbator package with verified library, runner, and libretro artifacts; prove a clean C consumer can use the library after relocating its install prefix; publish a truthful version-bound first-route NTSC provider profile; and qualify one reproducible, licensed NROM boot-to-gameplay scenario with a game-specific progress predicate, visible response, and non-silent audio. The phase owns upstream package, profile, and scenario evidence. LibFuturo owns its installed consumer, NES native host, and actual gameplay acceptance. Publication remains a final acceptance gate. Keep the older interrupted CI/AccuracyCoin campaign stopped and unknown.

</domain>

<decisions>
## Implementation Decisions

### Licensed NROM gameplay route and progress oracle
- **D-01:** Use MIT-licensed Nesteroids, already admitted by `tests/roms/manifest.txt`, as the preferred NROM. The candidate input recipe is neutral boot to the open screen, Start to enter gameplay, then A to fire; prove a game-specific state transition and shot-count change alongside visible output and non-silent gameplay audio. No-input and Start-only controls, input activity, a title screen, audio alone, or frame hashes alone do not establish interactive progress.
- **D-02:** Treat the currently checked-in Nesteroids debug artifacts as conflicting evidence. At the pinned upstream revision, `player_shot_count` is labeled `$0594` in `nesteroids.labels.txt` and `$0595` in `nesteroids.nes.dbg.txt`; source/include order suggests `game_screen` at `$0017`, while those artifacts label it in CODE at `$A962` and `$A64D`. Derive and verify the exact source-to-RAM mapping from the pinned source and its reproducible build/linker outputs before using any address. Do not guess. If the mapping cannot be defended, switch to another manifest-permitted NROM with an independently supported progress predicate before accepting GAME-01.
- **D-03:** Treat boot duration, pulse duration, visible-response checks, and audio thresholds as qualification details to derive from evidence and bounded runtime runs. The previously discussed 60-neutral-frame duration is an unverified candidate, not a locked value. Keep the scenario deterministic and bounded, and record controls that reject false positives.

### LibFuturo provider handoff
- **D-04:** Produce a Nesturbator-owned, exact-version NTSC profile based on the current public API and accepted package/source. Cover identity, region and metadata policy, load/refusal and active-session postconditions, controller sampling, pixel interpretation, exact video/audio cadence and bounds, lifecycle, and consumer limits. State that an NTSC preference does not override explicit PAL metadata and Dendy remains refused.
- **D-05:** Keep the public C API unchanged unless current consumer evidence demonstrates a missing capability. Do not edit the sibling LibFuturo checkout or claim its installed consumer, native host, or gameplay acceptance. Distinguish battery-backed game memory from opaque machine snapshots; mark capabilities without current consumer proof unqualified. PAL-through-LibFuturo, browser/device behavior, and Playstead remain unqualified.

### Package and release lineage
- **D-06:** Preserve v0.1.12 as the latest published baseline. The current phase branch's v0.1.11 metadata is not evidence that its newer v8 source is published. Do not force a version or claim local packages are released; let configured Release Please derive the next version from merged commits.
- **D-07:** Freeze and qualify the exact final source/package/profile/scenario identity before publication. Acceptance must identify the release commit/tag/package version and matching ABI, behavior revision, and state format, and verify the published library, runner, and libretro artifacts and checksums. PKG-01 remains pending until the published-package receipt exists. Follow `.planning/VERIFICATION-POLICY.md`; do not transfer historical v8 results to the new candidate.

### Verification and release sequencing
- **D-08:** Keep the current verification result truthful. Candidate `08a0d1ed2cf738c428e81dead9407b9b695a8608` passed the bounded local workflow and package/gameplay checks; exact-candidate hosted CI, resulting-main CI, and the actual published receipt remain pending. Do not repeat the local workflow unless a relevant input changes, infer hosted results from local or historical runs, mark PKG-01/PROF-01 complete, or use a milestone override to close the phase.
- **D-09:** Close the remaining evidence through the existing v9 milestone release path, then rerun final phase verification. At release, freshly triage related PRs; bind required CI to the actual PR head and record any tested merge SHA separately; merge only after required checks pass; verify required CI on the resulting main head; use the observed Release Please tag/source/version; and run the published receipt against the actual artifacts before closing PKG-01/PROF-01. The project already authorizes this normal milestone release path in `AGENTS.md` and `.planning/VERIFICATION-POLICY.md`. The GSD `$gsd-ship` preflight currently requires phase verification to be `passed`, while this phase cannot pass until publication. Do not change that evidence standard or force the status to get around the ordering; use the project-authorized closeout release path for this cycle and keep the phase pending until the published receipt passes. Preserve the older interrupted CI/AccuracyCoin stop and all unrelated user work.

### Reviewed Todos
- Defer `2026-10-26-add-reproducible-owned-property-tests.md`: no demonstrated need in this package/profile/gameplay handoff; preserve its existing record.
- Defer `2026-10-26-make-runner-signal-checks-deterministic.md`: no demonstrated need in this phase; preserve its existing record.

### the agent's Discretion
- Derive exact scenario timings, progress checks, and thresholds from pinned-source evidence and bounded runs. Keep all predicates game-specific and independently supported.
- Reuse the existing installed-package consumer and boot-to-play patterns, adding only the checks needed for relocation, active-game preservation on refusal, and the selected game's progress oracle.
- If Nesteroids' RAM mapping cannot be verified, select another already permitted NROM only after establishing a defensible oracle; otherwise report the concrete blocker before accepting GAME-01.
- Add a public API change only if the current consumer profile or test exposes a demonstrated gap. Prefer a small owned helper or justified duplication to another dependency.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Scope and project constraints
- `.planning/ROADMAP.md` §Phase 28 and §Phase 28 evidence and release boundary — phase goal, success criteria, package publication gate, and evidence limits.
- `.planning/REQUIREMENTS.md` — PKG-01, INST-01, PROF-01, and GAME-01 acceptance requirements.
- `.planning/PROJECT.md` — current milestone direction and core constraints.
- `.planning/STATE.md` — current project state, approved v9 scope, and prior evidence limits.
- `AGENTS.md` — clean-room, determinism, dependency, documentation, workflow, and explicit-recommendation preferences.
- `.planning/VERIFICATION-POLICY.md` — required release and verification boundaries.
- `docs/coordination/milestone-context-2026-11-03.md` — current LibFuturo handoff context, release baseline, and ownership boundaries.
- `.planning/phases/28-qualified-nes-gameplay-package/28-VERIFICATION.md` — current 14/17 result and exact remaining hosted-CI and published-receipt truths.
- `.planning/phases/28-qualified-nes-gameplay-package/28-ACCEPTANCE.md` — source-bound local acceptance and the final release/receipt procedure.
- `.planning/phases/28-qualified-nes-gameplay-package/28-03-PLAN.md` and `28-03-SUMMARY.md` — accepted separation of local readiness from hosted and published evidence; treat recorded PR metadata as historical until fresh triage.
- [GitHub Actions workflow events](https://docs.github.com/en/actions/reference/workflows-and-actions/events-that-trigger-workflows) — distinguishes a pull request's head SHA from the test merge commit used by the default `pull_request` checkout.
- [GitHub required status checks](https://docs.github.com/en/pull-requests/how-tos/merge-and-close-pull-requests/troubleshooting-required-status-checks) — required checks must succeed on the latest commit SHA; earlier commit results do not satisfy that requirement.

### Gameplay fixture, license, and evidence
- `.planning/seeds/SEED-261009-zs2-single-game-playability-check.md` — boot-to-gameplay acceptance intent.
- `ASSET_POLICY.md` — permitted ROM and fixture policy.
- `.planning/playtesting/AUTOMATION.md` — converting gameplay findings into independent, owned regression evidence.
- `.planning/preparation/README.md` — index to subsystem reference material; open the relevant source before designing or building.
- `.planning/preparation/DECISIONS.md` — prior project decisions.
- `.planning/preparation/ARCHITECTURE.md` — public architecture and subsystem boundaries.
- `.planning/preparation/ENGINEERING.md` — engineering and verification guidance.
- `.planning/preparation/GSD-HANDOFF.md` — phase branch and handoff conventions.
- `tests/roms/manifest.txt` — permitted ROM identity, license, and hash.
- `tests/runner/test_boot_to_play.c` and `tests/runner/game_movie.c` — existing boot-to-play and game-specific control patterns; the existing runner game uses mapper 1 and is not the Phase 28 NROM.
- `https://github.com/battlelinegames/nesteroids/tree/ae65d78048af68f3e735c49d77a7f37e976a7afe` — pinned Nesteroids source. Relevant files include `src/def/header.asm`, `src/screens/screens.asm`, `src/screens/open_screen.asm`, `src/lib/gamepad.asm`, `src/screens/game_play.asm`, `src/obj/player_shot.asm`, `nesteroids.map.txt`, `nesteroids.labels.txt`, `nesteroids.nes.dbg.txt`, and `LICENSE`. The checked-in symbol artifacts disagree on candidate RAM addresses; resolve from reproducible source/build evidence before use.

### Installed package, public API, and release
- `tests/consumer/CMakeLists.txt` and `tests/consumer/main.c` — current installed-package consumer; currently covers basic creation/frame advancement, not relocation plus gameplay.
- `tests/cmake/check_install_line.cmake` — existing install-line verification pattern.
- `CMakeLists.txt`, `cmake/nesturbatorConfig.cmake.in`, and `cmake/packaging.cmake` — existing package target, config, installation, and archive pattern.
- `docs/api.md` — public C API semantics.
- `docs/releases.md`, `.release-please-manifest.json`, `release-please-config.json`, and `.github/workflows/release.yml` — release identity and automation.
- `docs/reproduction.md` and `docs/testing.md` — reproducibility and test evidence contracts.
- `https://github.com/googleapis/release-please/blob/main/docs/manifest-releaser.md` and `https://github.com/googleapis/release-please` — Release Please manifest/version derivation behavior.
- `https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html` — CMake relocatable package configuration guidance.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `tests/consumer/CMakeLists.txt` already builds against `find_package(nesturbator CONFIG REQUIRED)` and the installed target. Extend or add a separate consumer to prove the moved prefix works without source/build paths.
- `tests/runner/test_boot_to_play.c` and `tests/runner/game_movie.c` provide patterns for scripted input, game-state observation, output checks, and negative controls. Their mapper-1 ROM cannot satisfy the NROM requirement.
- `tests/cmake/check_install_line.cmake`, `cmake/packaging.cmake`, and `cmake/nesturbatorConfig.cmake.in` are existing install/package patterns; preserve the dependency-light package structure.
- `tests/cmake/provider_receipt.cmake` separates local, handoff, and published modes; published mode is the only path that writes the final receipt after validating actual release identity, archives, checksums, attestations, and native replay.

### Established Patterns
- The public C API exposes version, ABI, behavior revision, state format, effective region, and exact rate fractions. Use those facts in the profile rather than inventing an API.
- Existing package verification stages an install and builds a consumer, but does not currently relocate the prefix and exercise a game scenario.
- ROM identity and license are controlled by `tests/roms/manifest.txt`; use the pinned MIT fixture and keep ROM bytes and private captures within project policy.
- Acceptance has distinct layers: local workflow, exact-candidate required CI, resulting main CI, and published artifacts. A local archive is not published-package proof.

### Integration Points
- The relocated installed consumer must cover content load, controller input, frame advancement, video/audio retrieval, and refusal of a replacement load while preserving the active game.
- The selected game scenario should be exercised through the installed package and share its exact source/package identity with the NTSC provider profile.
- The profile describes Nesturbator's side of the provider seam. LibFuturo owns its adapter, host, and game-session acceptance; no sibling worktree edits are part of this phase.
- Final release evidence must identify the PR head, any GitHub test merge SHA, the resulting main head, and the Release Please source/tag separately; do not collapse those identities into one “CI passed” claim.

</code_context>

<specifics>
## Specific Ideas

The recommended route is Nesteroids: enter gameplay with Start, then use A to fire, with a game-specific transition and shot count as candidate state evidence. However, current label/debug files disagree on both the `player_shot_count` location (`$0594` versus `$0595`) and `game_screen` location (`$A962` and `$A64D` versus a source/include-order candidate at `$0017`). Treat the mapping as an explicit qualification gate and resolve it from the exact pinned source and reproducible linker outputs; no guessed RAM address is acceptable.

The recommendation for the consumer handoff is a current version-bound NTSC profile in this repository, while clearly stating that LibFuturo owns its installed consumer and native host. The recommendation for release identity is to keep v0.1.12 as the published baseline and let Release Please calculate the next version after merge.

For the remaining release gaps, keep the current `gaps_found` result and follow the project-authorized v9 closeout path without weakening the evidence gates. The published receipt must exist before final phase verification can pass. The current `$gsd-ship` preflight conflicts with that sequence, so do not invoke it to begin the release or rewrite verification status; use the documented release authority and preserve exact PR/main/release identities through receipt generation.
</specifics>

<deferred>
## Deferred Ideas

- PAL-through-LibFuturo, consumer host playback, Playstead acceptance, physical-device pacing, direct hardware qualification, machine snapshots, and broad mapper/title compatibility are not established by this phase.
- Keep the interrupted CI/AccuracyCoin campaign stopped and unknown.

### Reviewed Todos (not folded)
- `2026-10-26-add-reproducible-owned-property-tests.md` — reviewed and deferred because this phase has no demonstrated property-testing need.
- `2026-10-26-make-runner-signal-checks-deterministic.md` — reviewed and deferred because this phase has no demonstrated need for a broader runner signal change.

</deferred>

---

*Phase: 28-qualified-nes-gameplay-package*
*Context gathered: 2026-11-03*
