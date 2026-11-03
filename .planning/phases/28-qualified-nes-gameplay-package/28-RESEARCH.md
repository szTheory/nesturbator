# Phase 28: Qualified NES gameplay package — Research

**Researched:** 2026-11-03
**Domain:** C package distribution, NES gameplay qualification, provider profile
**Confidence:** MEDIUM

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

#### Licensed NROM gameplay route and progress oracle
- **D-01:** Use MIT-licensed Nesteroids, already admitted by `tests/roms/manifest.txt`, as the preferred NROM. The candidate input recipe is neutral boot to the open screen, Start to enter gameplay, then A to fire; prove a game-specific state transition and shot-count change alongside visible output and non-silent gameplay audio. No-input and Start-only controls, input activity, a title screen, audio alone, or frame hashes alone do not establish interactive progress.
- **D-02:** Treat the currently checked-in Nesteroids debug artifacts as conflicting evidence. At the pinned upstream revision, `player_shot_count` is labeled `$0594` in `nesteroids.labels.txt` and `$0595` in `nesteroids.nes.dbg.txt`; source/include order suggests `game_screen` at `$0017`, while those artifacts label it in CODE at `$A962` and `$A64D`. Derive and verify the exact source-to-RAM mapping from the pinned source and its reproducible build/linker outputs before using any address. Do not guess. If the mapping cannot be defended, switch to another manifest-permitted NROM with an independently supported progress predicate before accepting GAME-01.
- **D-03:** Treat boot duration, pulse duration, visible-response checks, and audio thresholds as qualification details to derive from evidence and bounded runtime runs. The previously discussed 60-neutral-frame duration is an unverified candidate, not a locked value. Keep the scenario deterministic and bounded, and record controls that reject false positives.

#### LibFuturo provider handoff
- **D-04:** Produce a Nesturbator-owned, exact-version NTSC profile based on the current public API and accepted package/source. Cover identity, region and metadata policy, load/refusal and active-session postconditions, controller sampling, pixel interpretation, exact video/audio cadence and bounds, lifecycle, and consumer limits. State that an NTSC preference does not override explicit PAL metadata and Dendy remains refused.
- **D-05:** Keep the public C API unchanged unless current consumer evidence demonstrates a missing capability. Do not edit the sibling LibFuturo checkout or claim its installed consumer, native host, or gameplay acceptance. Distinguish battery-backed game memory from opaque machine snapshots; mark capabilities without current consumer proof unqualified. PAL-through-LibFuturo, browser/device behavior, and Playstead remain unqualified.

#### Package and release lineage
- **D-06:** Preserve v0.1.12 as the latest published baseline. The current phase branch's v0.1.11 metadata is not evidence that its newer v8 source is published. Do not force a version or claim local packages are released; let configured Release Please derive the next version from merged commits.
- **D-07:** Freeze and qualify the exact final source/package/profile/scenario identity before publication. Acceptance must identify the release commit/tag/package version and matching ABI, behavior revision, and state format, and verify the published library, runner, and libretro artifacts and checksums. PKG-01 remains pending until the published-package receipt exists. Follow `.planning/VERIFICATION-POLICY.md`; do not transfer historical v8 results to the new candidate.

#### Reviewed Todos
- Defer `2026-10-26-add-reproducible-owned-property-tests.md`: no demonstrated need in this package/profile/gameplay handoff; preserve its existing record.
- Defer `2026-10-26-make-runner-signal-checks-deterministic.md`: no demonstrated need in this phase; preserve its existing record.

### the agent's Discretion
- Derive exact scenario timings, progress checks, and thresholds from pinned-source evidence and bounded runs. Keep all predicates game-specific and independently supported.
- Reuse the existing installed-package consumer and boot-to-play patterns, adding only the checks needed for relocation, active-game preservation on refusal, and the selected game's progress oracle.
- If Nesteroids' RAM mapping cannot be verified, select another already permitted NROM only after establishing a defensible oracle; otherwise report the concrete blocker before accepting GAME-01.
- Add a public API change only if the current consumer profile or test exposes a demonstrated gap. Prefer a small owned helper or justified duplication to another dependency.

### Deferred Ideas (OUT OF SCOPE)
- PAL-through-LibFuturo, consumer host playback, Playstead acceptance, physical pacing, direct hardware qualification, machine snapshots, and broad mapper/title compatibility are not established by this phase.
- Keep the interrupted CI/AccuracyCoin campaign stopped and unknown.

#### Reviewed Todos (not folded)
- `2026-10-26-add-reproducible-owned-property-tests.md` — reviewed and deferred because this phase has no demonstrated property-testing need.
- `2026-10-26-make-runner-signal-checks-deterministic.md` — reviewed and deferred because this phase has no demonstrated need for a broader runner signal change.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| PKG-01 | A consumer can identify and install a published Nesturbator package built from the exact accepted current source, with consistent package/header version, ABI, behavior revision and state-format identity; the release includes the library, runner and libretro artifacts with verified checksums. | Release boundary, Release Please derivation, and three-component artifact identity below. |
| INST-01 | A clean C consumer can configure, build and run against the installed library package after the install prefix is relocated, without resolving headers or libraries from the source or build tree; it can load a valid permitted ROM, advance frames with input, obtain video/audio, and verify that a refused replacement load preserves the active game. | Existing staged consumer and missing relocation/content/refusal assertions below. |
| PROF-01 | A version-bound first-route profile identifies the exact Nesturbator package/source and accurately describes its selected NTSC content and region limits, load/refusal behavior, controller sampling, video representation, audio format/cadence/buffer bounds and lifecycle; it distinguishes battery-backed game memory from machine snapshots and marks every capability without current consumer proof as unqualified. | Header and API contract summary below. |
| GAME-01 | One ROM already permitted by the repository manifest has a bounded, reproducible boot-to-gameplay input scenario that runs through the relocated installed package; a game-specific progress predicate demonstrates an interactive transition with visible response and non-silent audio. A title screen, input activity, audio alone or frame hash alone does not satisfy the requirement. Nesteroids is the candidate until its exact route and predicate are verified. | Nesteroids source-to-RAM investigation and bounded fallback below. |
</phase_requirements>

## Summary

Extend the existing CMake installed-package consumer into the recurring integration check for relocation, content load, frame input/output, and failed replacement preservation [VERIFIED: `tests/CMakeLists.txt:2424-2451`; `.planning/REQUIREMENTS.md:13-14`]. Write the first-route NTSC profile from the public header and bind it only after the final release identity is fixed [VERIFIED: `include/nesturbator.h`; `.planning/REQUIREMENTS.md:18`]. The three deliverables are independently packaged; PKG-01 closes only against a published receipt with all component archives, checksums, tag/source identity, and matching API metadata [VERIFIED: `cmake/packaging.cmake:39-53`; `.planning/ROADMAP.md:35,47`].

Nesteroids remains the best first route, but its proposed RAM oracle is not ready to use. Pinned source explains the open-screen-to-game transition and `player_shot_count` behavior, while its checked labels/debug outputs disagree [CITED: https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/nesteroids.labels.txt and https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/nesteroids.nes.dbg.txt]. `game_screen`'s source allocation suggests `$0017`, but the pinned build is not reproducible in this environment because only Windows assembler/linker binaries are included and no compatible `ca65`/`ld65` is installed [CITED: https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/compile.bat; VERIFIED: environment probe]. Do not sample either disputed RAM address. First generate and verify linker outputs from the pinned source and confirm the resulting ROM digest. If that cannot be done within a bounded task, use permitted DABG only after deriving and mapping its own source-backed predicate; existing DABG movie scripts are input scaffolding, not gameplay proof [VERIFIED: `tests/runner/game_movie.c:1-6,101-115`].

**Primary recommendation:** Plan a gate-first slice: resolve the Nesteroids source/build/RAM mapping and scenario oracle; then extend the installed consumer and profile from the accepted candidate; freeze identity; run the approved local workflow and release gates. Keep any LibFuturo host or direct-hardware conclusions out of acceptance.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Installed library lookup and relocated use | API / Backend (C library package) | Build/package | Consumer resolves installed exports without source/build paths [VERIFIED: INST-01, `tests/consumer/CMakeLists.txt:10-13`]. |
| NTSC profile contract | API / Backend | Documentation | Public header owns region, input, output, status, and lifecycle semantics [VERIFIED: `include/nesturbator.h:181-192,231-251,265-377,382-482`]. |
| Gameplay scenario and state predicate | API / Backend test harness | Permitted game fixture | Exercise the library API and inspect guest RAM through its conformance peek [VERIFIED: `include/nesturbator.h:489-525`; `GAME-01`]. |
| Host UI/audio devices and actual LibFuturo gameplay | Consumer host | — | This phase provides provider evidence; LibFuturo owns its installed consumer, native host, and game acceptance [VERIFIED: `28-CONTEXT.md`, D-05; `.planning/ROADMAP.md:45`]. |
| Published package provenance | Release workflow / static artifacts | GitHub release | Tag, archives, checksums, attestations, and source commit describe one release candidate [VERIFIED: `docs/releases.md:92-119,163-197`]. |

## Standard Stack

| Existing facility | Version / contract | Use | Evidence |
|---|---|---|---|
| C17 + CMake/CTest | C17, CMake minimum 3.25; configured `ci` workflow | Add relocation and scenario assertions to existing native checks; no new framework. | [VERIFIED: `.planning/preparation/ENGINEERING.md:13-36`; `tests/consumer/CMakeLists.txt:3-13`] |
| `nesturbator::nesturbator` CMake target | Installed export, package config, static library and public header | Configure the independent consumer against only a moved prefix. | [VERIFIED: `CMakeLists.txt:90-120`; `cmake/nesturbatorConfig.cmake.in:1-6`] |
| Release Please | `simple` package with the checked-in manifest and configured Conventional Commit policy | Let merged commits determine the next version; do not hard-code the next patch. | [VERIFIED: `release-please-config.json:3-24`; `docs/releases.md:153-161`], [CITED: [Release Please manifest releaser](https://github.com/googleapis/release-please/blob/main/docs/manifest-releaser.md)] |
| Permitted NROM fixture | Nesteroids pinned in manifest as MIT; verify on-disk bytes against the listed SHA-256 | Use as the preferred boot-to-play scenario only after its oracle is reproducible. | [VERIFIED: `tests/roms/manifest.txt:21`; local SHA-256 matched manifest] |

No external runtime/library dependency is needed. A compatible cc65 toolchain is only a one-time build tool for resolving the upstream ROM map. The project prefers existing code/platform facilities before adding dependencies [VERIFIED: `AGENTS.md:91-99`].

## Project Constraints (from AGENTS.md)

- Deliver the C library, headless runner, and libretro adapter; do not add a window, audio-device, or input-device layer [VERIFIED: `AGENTS.md:45-48`].
- Keep emulation/tests owned here, with `libretro.h` the sole vendored file; the core links only C memory functions [VERIFIED: `AGENTS.md:45-46`].
- Preserve clean-room rules: use hardware documentation and test behavior; do not open or retain GPL/LGPL emulator source [VERIFIED: `AGENTS.md:53-55`].
- Keep committed ROM/BIOS bytes restricted to entries in `tests/roms/manifest.txt`; retain only sanitized evidence [VERIFIED: `AGENTS.md:49-52,117-121`; `ASSET_POLICY.md:6-31`].
- Keep the core deterministic and integer-only, with state per instance. Any behavior change lands with tests run by `cmake --workflow --preset ci` plus README and public-header comment updates [VERIFIED: `AGENTS.md:56-61`].
- Prefer fixed-width C, small modules, and no new dependency unless existing code, standard library, and platform facilities fail to meet a demonstrated need [VERIFIED: `AGENTS.md:91-99`].
- Preserve the stopped/unknown older CI/AccuracyCoin campaign. Full workflow runs are bounded at 30 minutes and failures are diagnosed before retry [VERIFIED: `AGENTS.md:123-138`; `.planning/VERIFICATION-POLICY.md:34-46,63-88`].
- At release closeout, verify related PRs, candidate CI, resulting main CI, project-owned worktree cleanliness, published artifacts, checksums, and version/source identity while preserving unrelated work [VERIFIED: `.planning/VERIFICATION-POLICY.md:67-88`].

## Implementation Findings

### 1. Nesteroids is semantically promising, but its address map is unresolved

The manifest row is `tests/roms/nesteroids.nes … ae65d78048af68f3e735c49d77a7f37e976a7afe MIT a30dcfb12c1447dc121619c93b4083d2b83c7405a883ed490c3a59e52ce74f36`; the local image's SHA-256 matched that digest [VERIFIED: `tests/roms/manifest.txt:21`; local `shasum -a 256`]. The pinned source declares an NROM header with `PRG size ... $02`, `CHR size ... $01`, and `; NROM mapper 0` [CITED: [header.asm at the pinned commit](https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/src/def/header.asm#L1-L22)].

The game-level transition and shot-slot semantics are source-supported, but a usable RAM oracle is not yet defensible. `open_press_start` reads Start and calls `start_game`, which sets `game_screen` to `PLAY_SCREEN`; gameplay checks A and calls `player_shoot`; the source says `MAX_PLAYER_SHOTS = 5` and updates `player_shot_count` modulo that limit [CITED: [open_screen.asm](https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/src/screens/open_screen.asm#L117-L143), [game_play.asm](https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/src/screens/game_play.asm#L21-L38), [player_shot.asm](https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/src/obj/player_shot.asm#L1-L53)]. This variable is a cycling projectile slot/index, not a cumulative lifetime shot total. A plan should call it a shot-slot counter and ensure the A pulse occurs when the game's cooldown permits; exact frames and audio/visual thresholds need bounded runs, not guessed constants.

The current symbol artifacts cannot establish a RAM oracle: the pinned `nesteroids.labels.txt` says `game_screen = $A962` and `player_shot_count = $0594`, while the `.dbg.txt` says `game_screen = $A64D` and `player_shot_count = $0595` [CITED: [labels.txt](https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/nesteroids.labels.txt#L25), [dbg.txt game_screen](https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/nesteroids.nes.dbg.txt#L9148), [dbg.txt shot slot](https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/nesteroids.nes.dbg.txt#L9434)]. `nesteroids.map.txt` places the zero-page segment at `000000–000036` but does not provide a reliable per-symbol address [CITED: [map.txt](https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/nesteroids.map.txt#L15-L27)].

Source allocation order makes `$0017` a plausible `game_screen` candidate: the source reserves `ft_temp: .res 7`, then one-byte zero-page fields through `temp_y_reg: .res 1`, before `game_screen: .res 1`; that preceding allocation totals 23 bytes from linker base zero [CITED: [zeropage.asm](https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/src/def/zeropage.asm#L1-L30), [screens.asm](https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/src/screens/screens.asm#L1-L8), [nesteroids.cfg](https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/nesteroids.cfg#L1-L15), [nesteroids.asm include order](https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/src/nesteroids.asm#L15-L46)]. This is a source-derived candidate only; the build script's reproducible output is still required before sampling the address. The pinned `compile.bat` runs `ca65` then `ld65` with map, labels, and debug output flags; its bundled assembler/linker are Windows PE binaries. This environment has no `ca65`, `ld65`, or Wine executable [CITED: [compile.bat](https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/compile.bat#L8-L15); VERIFIED: environment probe].

**Planning gate:** Rebuild the pinned source with a compatible, identified cc65 toolchain in a bounded isolated environment; retain the exact tool version/commands, map/label/debug outputs, and require the rebuilt ROM hash to equal the manifest pin. Cross-check the derived game-state and shot-slot RAM addresses against segment allocations and a runtime read through `nesturbator_peek_cpu_ram`. If any step is unavailable, do not use `$0017`, `$0594`, or `$0595` as a guessed oracle.

**Bounded alternative:** DABG is already permitted as pinned Zlib NROM content, and `tests/runner/game_movie.c` already supplies 180-frame two-port movie scaffolding [VERIFIED: `tests/roms/manifest.txt:20`; `tests/runner/game_movie.c:1-6,101-115`]. Its pinned source declares mapper 0/CHR RAM/NTSC and exposes `LevelNumber`, incremented in `NewLevel` [CITED: [dabg.s](https://github.com/NovaSquirrel/DABG/blob/5ecc60b6af3f726851bfeb1c2555388c5953e0c0/dabg.s#L50-L61), [memory.s](https://github.com/NovaSquirrel/DABG/blob/5ecc60b6af3f726851bfeb1c2555388c5953e0c0/memory.s#L65-L75), [newgame.s](https://github.com/NovaSquirrel/DABG/blob/5ecc60b6af3f726851bfeb1c2555388c5953e0c0/newgame.s#L75-L90), [NewLevel increment](https://github.com/NovaSquirrel/DABG/blob/5ecc60b6af3f726851bfeb1c2555388c5953e0c0/newgame.s#L238-L258)]. This is a fallback research route, not already-qualified evidence: establish a repeatable input path to gameplay, independently map the predicate using reproducible linker output, and add positive/no-input/Start-only controls. If that cannot be done within a bounded script, report GAME-01 blocked rather than substituting hash-only evidence.

### 2. Reuse the installed consumer, then make relocation real

`install.stage` installs the library component under the build tree's `stage`; `install.consumer` builds `tests/consumer` with `CMAKE_PREFIX_PATH` pointed there and runs it [VERIFIED: `tests/CMakeLists.txt:2424-2451`]. The separate consumer uses C17, `find_package(nesturbator CONFIG REQUIRED)`, and `nesturbator::nesturbator` [VERIFIED: `tests/consumer/CMakeLists.txt:1-13`]. Today it only creates an instance and advances the no-cartridge test frame, checking `frame_number == 1`; it does not load content, submit input, inspect A/V, reject a replacement, or move the install [VERIFIED: `tests/consumer/main.c:1-43`].

Keep the existing package config: it uses `@PACKAGE_INIT@` and includes the exported target relative to `CMAKE_CURRENT_LIST_DIR` [VERIFIED: `cmake/nesturbatorConfig.cmake.in:1-6`]. CMake documents `configure_package_config_file()` as the relocatable package-config helper; the current install uses it [VERIFIED: `CMakeLists.txt:94-111`], [CITED: [CMakePackageConfigHelpers](https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html)].

Plan the recurring check to stage, move the install prefix outside its original location, discard any previous consumer build directory, configure with only the relocated prefix, then build/run a separate C-only consumer. Exercise one manifest ROM, scripted per-frame input, frame count, visible pixel response, audio count/content, and a malformed replacement; after refusal, continue the original game and prove it remains usable. Keep the C consumer independent of source/build headers and libraries. This is justified recurring detection for a package's advertised relocation contract and fits the existing CTest install fixture; no additional test framework is needed.

### 3. Profile directly from the public API

The phase branch header says `#define NESTURBATOR_VERSION_MAJOR 0`, `#define NESTURBATOR_VERSION_MINOR 1`, `#define NESTURBATOR_VERSION_PATCH 11`, `#define NESTURBATOR_ABI_VERSION 1`, `#define NESTURBATOR_BEHAVIOUR_REVISION 17`, and `#define NESTURBATOR_STATE_FORMAT_VERSION 6` [VERIFIED: `include/nesturbator.h:47-54,118-143`]. Treat these as candidate-source fields, not the final package identity; freeze the profile against the accepted release commit/tag and confirm runtime/header version agreement.

The exact region enum is `NESTURBATOR_REGION_NTSC = 0`, `NESTURBATOR_REGION_PAL = 1`; preference values are `NESTURBATOR_REGION_PREFERENCE_AUTO = 0`, `NESTURBATOR_REGION_PREFERENCE_NTSC = 1`, `NESTURBATOR_REGION_PREFERENCE_PAL = 2` [VERIFIED: `include/nesturbator.h:181-192`]. The API states “Explicit NES 2.0 NTSC/PAL declarations take precedence; a preference selects only multi-region or legacy iNES content. Dendy and invalid preferences are refused without changing the active machine. The selected region stays fixed until unload or a successful later load” [VERIFIED: `include/nesturbator.h:437-445`]. Nesteroids' header source marks a basic iNES mapper-0 image and uses zeroed flags, so document which route selects its NTSC behavior, without generalizing the preference into an override of explicit PAL metadata [CITED: [pinned Nesteroids header](https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/src/def/header.asm#L1-L22)].

The public NTSC values are width `256`, height `240`, frame rate `39375000 / 655171`, sample rate `48000 / 1`, and “A frame yields 798 or 799 samples on NTSC” [VERIFIED: `include/nesturbator.h:265-275,303-308`]. A fixed one-frame audio buffer must therefore hold 799 samples for the NTSC maximum. Video is caller-owned `uint16_t`: palette entry bits 0–5, emphasis bits 6–8, bits 9–15 zero; audio is mono signed `int16_t`; use `audio_count` for the number written [VERIFIED: `include/nesturbator.h:277-308,325-350`]. The button masks are `NESTURBATOR_BUTTON_A = 1u << 0` and `NESTURBATOR_BUTTON_START = 1u << 3`; input is sampled “at the start of nesturbator_run_frame” and setting it “affects the next call” [VERIFIED: `include/nesturbator.h:231-251,372-377`]. The profile should also document `nesturbator_get_region`, `nesturbator_get_info`, version query, load/unload/reset lifecycle, and rejection postconditions from the header [VERIFIED: `include/nesturbator.h:352-357,382-482`].

Storage surfaces are distinct: the battery-save API exposes a cartridge save-RAM span and the core does no file I/O [VERIFIED: `docs/api.md:158-169`]; a save state is an opaque whole-machine snapshot with its own format identity and host-owned storage [VERIFIED: `docs/api.md:208-219`]. Mark save-state interoperability, PAL through LibFuturo, browser/device behavior, and host gameplay unqualified unless a current consumer provides that evidence. Do not add a public API absent a demonstrated gap.

### 4. Keep publication and identity as separate evidence gates

The project records v0.1.12 as the latest published package while the phase branch says `".": "0.1.11"` in `.release-please-manifest.json` and its header version patch is `11` [VERIFIED: `docs/coordination/milestone-context-2026-11-03.md:15-19`; quote from `.release-please-manifest.json:1-3`; `include/nesturbator.h:47-49`]. The configured package uses `"release-type": "simple"`, `"bump-minor-pre-major": true`, and `"bump-patch-for-minor-pre-major": true` [VERIFIED: `release-please-config.json:3-10`]. Release Please uses the manifest as prior version and derives the next release from conventional commit messages [CITED: [Release Please manifest releaser](https://github.com/googleapis/release-please/blob/main/docs/manifest-releaser.md)]. Reconcile against actual release lineage during planning/execution, preserve v0.1.12, and do not set a guessed next version or a one-time `release-as`.

The release defines three independently named ZIP components: library, runner, and libretro [VERIFIED: `cmake/packaging.cmake:39-53`; `docs/releases.md:14-23`]. Release evidence distinguishes a published release (tag, exact archive name, SHA-256 entry in `SHA256SUMS`, and artifact attestation) from a CI artifact and a local build [VERIFIED: `docs/releases.md:92-119`]. The configured publish path expects 18 archives—three components across Linux/macOS/Windows and x64/arm64—plus sums and attestations before publishing [VERIFIED: `docs/releases.md:163-197`].

| Artifact | Identity and checksum boundary |
|---|---|
| Library ZIP | Published tag + exact library archive name + matching `SHA256SUMS` entry + archive attestation; inspect package/header version and ABI/behavior/state-format fields against the accepted release source. |
| Runner ZIP | Same published tag/source and exact runner archive identity; verify its own archive digest and attestation alongside the other components. |
| Libretro ZIP | Same published tag/source and exact libretro archive identity; verify its digest and attestation. If a host-loaded-module result is ever claimed, compare that module to the core member extracted from this archive. |

Each archive is covered by release attestation, which establishes the archive subject; it does not directly hash the extracted libretro module [VERIFIED: `docs/releases.md:100-112`]. Do not claim a LibFuturo host load or gameplay result in this provider phase.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---|---|---|---|
| Relocatable CMake package metadata | Custom absolute-path rewriting or a new package lookup layer | Existing exported target/config and `configure_package_config_file()` | The package config is relative to its own location [VERIFIED: `cmake/nesturbatorConfig.cmake.in:1-6`]; CMake documents the relocatable helper [CITED: https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html]. |
| Provider contract values | A parallel profile with invented rates/pixel/audio semantics | Header/API as source of truth, then version-bind its documented values | Header defines exact rates, representation, and field behavior [VERIFIED: `include/nesturbator.h:265-350`]. |
| Release identity | A local version string or generated archive as proof of release | Published tag, archive name, sums, attestation and exact source commit | Project release docs distinguish published, CI, and local identities [VERIFIED: `docs/releases.md:92-119`]. |
| Game state predicate | Frame hashes, guessed RAM offsets, or input-only progress | Pinned game source plus verified build/linker mapping and conformance RAM peek | Requirement calls for game-specific interactive progress, and Nesteroids artifacts conflict [VERIFIED: `.planning/REQUIREMENTS.md:22`; CITED: https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/nesteroids.labels.txt and https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/nesteroids.nes.dbg.txt]. |

## Common Pitfalls

- **Trusting a stale symbol file:** label/debug addresses differ [CITED: https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/nesteroids.labels.txt and https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/nesteroids.nes.dbg.txt]. Require reproducible map/labels/debug output from the exact pinned source and same rebuilt ROM before adding RAM reads.
- **Calling a modulo slot a total shot count:** `player_shot_count` cycles through slots [CITED: https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/src/obj/player_shot.asm]; name/assert its semantics accurately and account for the source-defined wait before firing.
- **Calling a staged install “relocated”:** the existing consumer points to `<build>/stage` [VERIFIED: `tests/CMakeLists.txt:2424-2451`]. Move it, use a clean consumer build, and resolve only against its new prefix.
- **Using a weak GAME-01 signal:** title output, audio, input activity, and frame hashes alone are explicitly insufficient [VERIFIED: `.planning/REQUIREMENTS.md:22`]. Pair a source-backed state change with response pixels and non-silent gameplay audio; keep no-input and Start-only controls [VERIFIED: `28-CONTEXT.md`, D-01].
- **Promoting local metadata to a release:** branch and published identities differ [VERIFIED: `docs/coordination/milestone-context-2026-11-03.md:15-19`]. Wait for the candidate's published receipt and do not restart the stopped campaign [VERIFIED: `.planning/VERIFICATION-POLICY.md:34-46,83-88`].
- **Overstating the handoff:** LibFuturo installed-consumer, host, and gameplay acceptance are consumer-owned; PAL-through-LibFuturo, physical pacing, and direct hardware evidence are expressly unqualified [VERIFIED: `28-CONTEXT.md`, D-05 and Deferred Ideas; `.planning/ROADMAP.md:45`].

## Code Examples

Use the existing C API as the consumer pattern: zero and size-tag the public structs as required by the header, create the instance, load the manifest-bound content with the selected region preference, set input before each frame, provide caller-owned video/audio buffers, and inspect frame output plus game state through the public API. On a refused replacement, continue the original instance and verify its state/output. Keep the exact buffer counts and buttons sourced from the header values quoted above. Existing API entry points and no-cartridge behavior are documented in [the public header](include/nesturbator.h:352-482); the current consumer is a minimal example at [tests/consumer/main.c](tests/consumer/main.c:11-43).

For the one-time source investigation, reproduce the upstream build recipe `ca65 ... -g -o nesteroids.o` followed by `ld65 ... -m nesteroids.map.txt -Ln nesteroids.labels.txt --dbgfile nesteroids.nes.dbg` from the pinned `compile.bat` [CITED: [pinned compile.bat](https://github.com/battlelinegames/nesteroids/blob/ae65d78048af68f3e735c49d77a7f37e976a7afe/compile.bat#L8-L15)]. Compare generated mapping, segment bounds, and ROM SHA with the manifest before encoding any address in a test.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|---|---|---:|---|---|
| CMake | Existing package workflow and consumer | Yes | 4.4.3 | — |
| C compiler | C17 consumer | Yes | Apple clang 21.0.1 | Use configured toolchain in CI for cross-platform release evidence. |
| cc65 `ca65`/`ld65` | Reproducible Nesteroids linker mapping | No | — | Run the pinned build in a bounded environment with a compatible recorded cc65 toolchain; if unavailable, qualify another permitted NROM with its own defensible map. |

## Security Domain

`.planning/config.json` explicitly sets `"security_enforcement": false` [VERIFIED: `.planning/config.json:47-51`]; ASVS-specific security research is not in scope for this phase.

## Open Questions (RESOLVED)

These planning dispositions resolve how qualification proceeds; they do not assert runtime results or a future release identity.

1. [RESOLVED — planning disposition] Per D-01/D-02, the exact game route is an execution qualification gate: bound the pinned Nesteroids source/toolchain rebuild and source-to-RAM linker-map investigation to ten minutes, require reproducible ROM bytes matching the manifest digest, and independently defend the gameplay predicate before accepting it. If that gate fails, allow one independently bounded ten-minute DABG source/linker-map/ROM-digest qualification attempt with its own supported gameplay predicate and controls; if neither route qualifies, report the explicit GAME-01 blocker. No disputed address is accepted by planning.
2. [RESOLVED — planning disposition] Per D-03, derive the shortest stable boot, Start and A windows and visible/audio thresholds during execution within Plan 01's bounded replay budget: at most twelve exploratory replays, each at most 600 emulated frames and fifteen seconds wall time, followed by a deterministic recipe of at most 600 frames. Use source constraints and no-input/Start-only controls; do not guess thresholds or treat the unverified 60-neutral-frame candidate as established. Failure within these bounds remains a concrete qualification blocker.
3. [RESOLVED — planning disposition] Per D-06/D-07, configured Release Please deliberately determines the next version after merge; shipping obtains the exact release commit/tag/version from the actual published receipt and verifies the matching artifacts and shared identity. Preserve v0.1.12 as the observed published baseline, invent no future version, and keep PKG-01 pending until the live published-package receipt passes.

## Sources

### Primary project sources
- Phase decisions and scope: `.planning/phases/28-qualified-nes-gameplay-package/28-CONTEXT.md`; requirements: `.planning/REQUIREMENTS.md:13-22`; roadmap/release boundary: `.planning/ROADMAP.md:29-49`; state/next-step context: `.planning/STATE.md:37-45,159-170`.
- Project constraints: `AGENTS.md:43-67,91-99,117-138`; verification policy: `.planning/VERIFICATION-POLICY.md:6-25,34-46,67-88`.
- Subsystem preparation read: `.planning/preparation/README.md`, `DECISIONS.md`, `ARCHITECTURE.md`, `ENGINEERING.md`, `LIBRETRO-AND-RUNNER.md`, and `GSD-HANDOFF.md`.
- Package/profile source of truth: `CMakeLists.txt:90-120`, `tests/CMakeLists.txt:2424-2451`, `tests/consumer/*`, `include/nesturbator.h`, `docs/api.md`, `cmake/packaging.cmake`, `docs/releases.md`, `.github/workflows/release.yml`.
- Fixture identity and policy: `tests/roms/manifest.txt:20-21`, `ASSET_POLICY.md:23-49`, `tests/runner/game_movie.c`.
- Upstream game source at exact commits: [Nesteroids](https://github.com/battlelinegames/nesteroids/tree/ae65d78048af68f3e735c49d77a7f37e976a7afe) and [DABG](https://github.com/NovaSquirrel/DABG/tree/5ecc60b6af3f726851bfeb1c2555388c5953e0c0).

### Official documentation
- [CMakePackageConfigHelpers](https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html) — relocatable package config helper.
- [Release Please manifest releaser](https://github.com/googleapis/release-please/blob/main/docs/manifest-releaser.md) — manifest version and commit-based next-version derivation.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|---|---|---|
| — | None. Decisions still needing runtime evidence are listed as qualification gates/open questions, not asserted facts. | — | — |

## Metadata

**Confidence breakdown:**
- Package/profile contracts: HIGH — read from project package files and public API sources.
- Nesteroids source behavior: MEDIUM — pinned source supports the behavior, but local compatible assembler/linker output is unavailable and checked symbol artifacts conflict.
- Release mechanics: MEDIUM — local workflow/configuration was inspected and checked against official Release Please documentation; the future published receipt does not yet exist.

**Research date:** 2026-11-03
**Valid until:** Recheck at release candidate; version and release identity are fast-moving.
