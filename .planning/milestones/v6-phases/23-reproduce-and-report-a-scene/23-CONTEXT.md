# Phase 23: Reproduce and report a scene - Context

**Gathered:** 2026-10-30
**Status:** Ready for planning

<domain>
## Phase Boundary

Deliver REPRO-01 through REPRO-04: a sanitized scene report, bounded replay on
permitted fixtures using existing facilities, and a triage route separating
repeatability from correctness. Preserve daily progress and distinguish native
payloads, adapter output and host containers/presentation. Phase 22 supplies
identity, storage and version policy. This phase does not diagnose or repair the
commercial scene reports, add a host, or qualify cross-build state compatibility.
</domain>

<decisions>
## Implementation Decisions

The owner selected all relevant areas and delegated resolution: use focused
subagents, examine relevant roles and adversarial alternatives, synthesize a
coherent recommendation, and follow it without routine preference questions.
Three specialist reviews and bounded follow-ups covered report/privacy,
replay/controls and triage/progress safety. Existing milestone discovery was
reused; no broad ecosystem sweep or runtime campaign was needed. These are
planning constraints, not new runtime verification.

### One report route with explicit evidence limits

- **D-01:** Prefer one canonical `docs/reproduction.md` with the copyable player
  report first, a completed permitted-fixture example, then developer replay and
  triage. Link directly from README troubleshooting and the relevant runner,
  testing and progress guides. Keep detailed runner/API/progress contracts at
  their existing canonical homes; link rather than duplicate them. Equivalent
  naming is planner discretion. No docs framework or issue-service automation.
- **D-02:** The report records title/region/content revision or digest; actual
  loaded core/build and host/version/launch route; exact scene and verified
  access method; ordered inputs and ports; expected and observed behavior;
  repeatability, start conditions, relevant settings, and untried conditions.
  Distinguish a title-level observation from reaching the exact reported scene.
  An unreached scene or failed access attempt is not scene acceptance evidence.
- **D-03:** Allow explicit unknown, withheld, not tried and unavailable values.
  A useful observation need not already be a deterministic reproduction. Label
  its limits; do not invent identity, assume a known menu version proves the
  loaded module, or make a player supply a hardware diagnosis. Required identity
  checks must succeed before claiming qualified executable checkpoint replay.
- **D-04:** Use digests and sanitized build/release/source identities as public
  metadata; keep personal module paths, device/account/connection identifiers,
  commercial media, saves, checkpoints, raw captures/traces and raw private logs
  local and outside public Git/CI artifacts. The permitted worked example
  includes its content digest. A commercial digest may be shared as metadata;
  when withheld, retain it locally and report the limit explicitly. A digest
  distributes no ROM bytes and does not establish permission to share artifacts.
- **D-05:** Expected behavior needs provenance: source, scene, content revision/
  region and confidence. Record known original quirks when documented. Unknown-
  provenance video or remembered behavior is a lead, not a hardware oracle.
  Retain existing commercial observations/access limits in their dated evidence
  files; this phase does not relabel them as diagnosed or fixed.

### Supported replay demonstrations, not a new replay engine

- **D-06:** Demonstrate cold-boot input-movie replay and supported native
  checkpoint continuation as separate, explicitly described routes. Current
  `--movie` plus `--load-state` is unsupported. Use a neutral-input continuation
  where necessary; do not imply arbitrary recorded non-neutral input resumes
  through that CLI combination. No runtime expansion solely to hide this limit.
- **D-07:** Reuse owned/permitted fixtures and existing generators/controls.
  Inspect `tests/roms/manifest.txt` before choosing assets. The existing played
  movie/boot-to-play controls and owned NMI checkpoint fixture are candidates,
  not a requirement to create another ROM or framework. Generated owned ROMs
  remain disposable build/scratch outputs; commit no new binary asset.
- **D-08:** Public examples must work from the public checkout and documented
  build facilities without local private helpers or media. Existing owned test
  targets may be documented with their build and invocation steps; do not assume
  they ship in release archives. Demonstrate any remaining access/orchestration
  gap before writing a small owned helper. Keep it local to this example and
  reuse C/CMake/CTest and platform facilities; no new core dependency or daemon.
- **D-09:** Repeat identical runs and compare explicitly identified frame and
  audio outputs. Loaded-state frame numbers are absolute: resume after K through
  end frame N, not N additional frames. Compare audio from the same suffix and
  boundaries; a whole cold-boot audio hash cannot equal a shorter resumed window
  by assumption. Native pixels, RGB dumps, transition streams and PCM are
  different representations and need separately named comparisons.
- **D-10:** Credit existing input-sensitive and stale/omitted-load controls
  when they establish the exact invariant; do not duplicate them for a new
  documentation claim. The control must detect an independently chosen wrong
  input or continuation, rather than just compare two equally idle runs. Current
  output hashes show repeatability; guest markers or independent expectations
  establish their own narrower properties, not general hardware correctness.
- **D-11:** Preserve NMOVIE1 validation, two-port masks and current option/error
  semantics. Do not call the runner a recorder. Explain the supported generation
  route and default/neutral inputs. Treat recording from a host, converting
  foreign movies, and checkpoint-plus-movie continuation as unsupported unless
  a distinct existing supported route is actually demonstrated.

### Scratch isolation, identity and finite bounds

- **D-12:** Every executable example states an explicit finite frame cap and
  outer wall-time bound, checks completion/required outputs, and uses a fresh
  scratch root with deliberate paths. Document the actual timeout mechanism;
  the runner has no intrinsic wall-clock timeout. Reuse CMake/CTest timeouts
  where suitable, including for public orchestration. A timed-out, stopped,
  truncated or missing-output run is not a successful replay.
- **D-13:** Keep save/state/config/output paths isolated from daily progress.
  Never point examples at active host directories, auto-load slots, other-core
  storage or private content. The safest fixture route avoids battery writes
  where unnecessary. If battery storage is involved, use disposable copies and
  retain the newest independent backup outside every active example path.
- **D-14:** Carry forward exact-core-build/content snapshot support, separate
  newest-battery backup and host-owned container/config policy from Phase 22.
  Loading a valid older state can replace newer battery RAM. Neither successful
  load nor matching state format establishes cross-build compatibility.
- **D-15:** Before replay, verify required identity and native payload/container
  type. Wrong, unknown or unverifiable pairing gets a clear unsupported/stop
  result; do not rename a RetroArch disk state into a native payload or silently
  substitute a checkpoint. Distinguish this procedure's preflight policy from
  loader enforcement: the native loader does not enforce exact core build or
  behavior revision. Preserve documented runner refusal/read/usage exits; do not
  invent a new runner unsupported exit code.

### Triage by output layer and oracle

- **D-16:** The guide routes evidence through identified scene/access and input,
  native pixels/audio, adapter conversion/callback output, host presentation,
  then independently supported original-game behavior. Each branch states what
  was compared, what it suggests, what remains unknown and the next bounded
  evidence source. This is a decision guide, not a requirement to run every
  layer or require new owner play for each report.
- **D-17:** Native/adapter agreement covers only the measured outputs. It does
  not prove that a frontend delivered intended inputs, accepted/presented each
  callback, avoided dropped/doubled frames or resampled audio correctly. Use
  existing adapter conversion and host progression evidence with their limits;
  add instrumentation only for a demonstrated required gap.
- **D-18:** Screenshots are host RGB evidence, affected by conversion, scaling,
  shaders, crop, capture timing and encoding. Record relevant settings and the
  capture path in sanitized terms. Without aligned frames and known transforms,
  comparisons are qualitative; they cannot be equated to native hashes. Audio
  pacing, synchronization and devices similarly remain host-owned concerns.
- **D-19:** A hardware correctness claim needs an independent oracle appropriate
  to the specific fault: documented hardware behavior, suitable measured hardware
  evidence or an independently derived fixture expectation. A released reference
  emulator can provide comparison evidence with its identity/limitations, not
  automatic hardware authority. Never inspect GPL/LGPL emulator source. If no
  adequate oracle or reproducible access exists, retain the unclassified result.

### Focused verification and workflow boundaries

- **D-20:** Machine-verifiable public examples and any unique negative controls
  belong in existing CMake/CTest paths. Check actual commands, required fields,
  links and safety/unsupported claims; prose-token presence alone is not replay
  evidence. Reuse existing document-check patterns without broad merged-corpus
  searches, self-confirming expected snapshots or weakening unique assertions.
- **D-21:** Documentation-only changes use relevant executable document/package/
  fixture checks. A necessary runtime behavior change first demonstrates its
  gap, then carries an owned regression, README/public-header comments and
  `cmake --workflow --preset ci`. Preserve pinned hashes, protected PASS keys,
  no-skip and performance gates. No unrelated expensive campaign reruns.
- **D-22:** Preserve all live-testing stops, budgets, collector/platform debt
  and private-artifact boundaries. No live host/device operation, commercial
  replay, recurring collector repair, release, push or PR is authorized by this
  discussion. Stop after context/log/state capture; planning is the next step.

### Agent discretion and evidence still to obtain

The planner chooses concise headings, exact fixture/frame budgets, existing
public generator entry points, scratch orchestration and focused test grouping.
Prove the clean-checkout commands, same-window audio comparison, input/wrong-
continuation controls and identity/container preflight during implementation.
Do not describe these as newly verified by this discussion. There are no
unresolved owner choices; escalate only an actual requirement conflict or new
scope/authority decision, not routine implementation preferences.
</decisions>

<canonical_refs>
## Canonical References

Downstream agents must read the scope and evidence authorities, then the
contract owners relevant to each task. Preparation notes describe their own
dated evidence; current public/source contracts govern executable behavior.

### Scope, prior decisions and preserved evidence
- `AGENTS.md` — ownership, clean room, dependency, step and safety boundaries.
- `.planning/REQUIREMENTS.md` — REPRO-01 through REPRO-04 and acceptance limits.
- `.planning/ROADMAP.md` — Phase 23 goal/success criteria and Phase 22 dependency.
- `.planning/PROJECT.md` — deterministic three-deliverable core and constraints.
- `.planning/phases/22-install-and-protect-progress/22-CONTEXT.md` — adopted identity/storage/state-policy decisions.
- `.planning/phases/22-install-and-protect-progress/22-VERIFICATION.md` — delivered evidence and limits; avoid duplicating it.
- `.planning/research/milestone-v6-discovery/SYNTHESIS.md` — measured milestone delta and stopping rule.
- `.planning/research/milestone-v6-discovery/ADVERSARIAL-REVIEW.md` — scope, compatibility and attribution traps.
- `.planning/playtesting/LESSONS.md` — scene-first access, original quirks, exact-scene and title evidence.
- `.planning/playtesting/AUTOMATION.md` — safe private loop, regression direction and runtime limits.
- `.planning/playtesting/POSTMORTEM.md` — current commercial observations remain unclassified.
- `.planning/preparation/README.md` — source index and evidence tiers.
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` — native/host boundaries and dated runner references.
- `.planning/preparation/ENGINEERING.md` — existing owned build/test mechanisms.

### Public contracts and existing owned coverage
- `README.md` — direct player troubleshooting/guide entry point.
- `docs/runner.md` — NMOVIE1, hashes, absolute-frame state resume and exit semantics.
- `docs/api.md` — native output, state payload and refusal contract.
- `docs/progress.md` — backup, identity, container and rollback policy.
- `docs/testing.md` — input-sensitive and checkpoint suffix controls, host evidence limits.
- `include/nesturbator.h` — public input/output and native-state contracts.
- `runner/main.c` — current mode combinations, ranges and hash/run boundaries.
- `runner/movie.c` — owned movie validation.
- `runner/state_file.c` — native checkpoint I/O.
- `runner/exit_status.h` — authoritative existing exit values.
- `tests/roms/manifest.txt` — permitted committed media.
- `tests/CMakeLists.txt` — existing runner fixtures, document checks and timeouts.
- `tests/runner/game_movie.c` — owned movie generator; a build target, not a shipped recorder.
- `tests/runner/nmi_fixture.c` — owned generated checkpoint fixture and suffix oracle.
- `tests/cmake/runner_state.cmake` — resumed, stale and omitted-load controls.
- `tests/runner/test_boot_to_play.c` — existing played-input controls; inspect actual registration before reuse.
- `tests/core/test_state_reject.c` — independent refusal/unchanged-instance checks.
- `tests/core/test_state_generation.c` — state-load battery generation behavior.
- `tests/host/test_convert.c` — adapter output representation checks.
- `tests/libretro/libretro_host.c` — actual adapter input/audio/state evidence.
- `tests/retroarch/run_retroarch.cmake` — existing host checks and their bounds; no new live run required.

### External primary documentation
- [Libretro core development](https://docs.libretro.com/development/cores/developing-cores/) — frontend callback and synchronization boundary.
- [RetroArch VSync guidance](https://docs.libretro.com/guides/optimal-vsync/) — presentation and audio synchronization concerns.

External host docs explain boundaries, not a newly tested installed-host menu or
CLI recipe. Verify exact capabilities before prescribing any host invocation.
</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- Existing NMOVIE1 runner, public native-state options and owned fixture generators.
- Played-input controls and NMI resumed/stale/omitted-load markers plus suffix audio comparisons.
- Independent native refusal/generation, adapter conversion and host progression tests.
- CMake executable-document checks and per-test/per-process timeout facilities.

### Established Patterns
- One canonical guide per detailed contract, with independently checked links/claims.
- State load validates native payload semantics; supported exact-build pairing is stricter policy.
- Native pixels, RGB conversion, PCM and host capture are distinct evidence surfaces.
- Owned fixtures and independent causal controls precede broader testing or dependencies.

### Integration Points
- No codebase maps were present; scouts inspected runner/API/testing contracts,
  relevant owned source and CMake registrations.
- README troubleshooting and topic-guide links connect the report/replay/triage route.
- Existing fixture targets are available from source builds; archive contents do
  not imply their presence. A missing clean-checkout path is a gap to demonstrate.
</code_context>

<specifics>
## Specific Ideas

Apply relevant player, maintainer, API-consumer, QA, portability, rendering/audio,
security/privacy and data-loss lenses. Unrelated UI stacks, distributed systems
and graphics engines add no requirements. Reuse prior findings and stop fan-out
once concrete choices and adversarial traps converge. Prefer a small owned helper
or justified local duplication to a dependency: another copy and paste can be
better than a dependency; add one only for a demonstrated unmet need.
</specifics>

<deferred>
## Deferred Ideas

Accuracy repairs/scene-fix acceptance, DMA work, overscan, regions/mappers,
Playstead integration, generic recording or replay engines, movie/state conversion,
checkpoint-relative non-neutral movie support, cross-build compatibility promises,
collector repair and new live campaigns remain separate scopes.

### Reviewed Todos (not folded)
- `.planning/todos/pending/2026-10-26-add-reproducible-owned-property-tests.md` — keyword match only; the broad helper/shrinking proposal adds no demonstrated REPRO gap. Reuse existing controls. It remains proposed, unlinked and incomplete.

Other previously deferred mutation/signal proposals remain untouched. This
discussion adds no requirement, campaign budget or risk acceptance for them.
</deferred>

---
*Phase: 23-reproduce-and-report-a-scene*
*Context gathered: 2026-10-30*
