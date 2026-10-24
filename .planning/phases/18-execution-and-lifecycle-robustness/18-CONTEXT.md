# Phase 18: Execution and lifecycle robustness - Context

**Gathered:** 2026-10-24
**Status:** Ready for planning

The user delegated every decision area: fan out across relevant technical and
stakeholder perspectives, research primary sources, challenge alternatives,
synthesize coherent recommendations and follow them automatically. Three
focused advisors reviewed execution reach, lifecycle/halt semantics, and
campaign evidence; two then cross-reviewed the combined recommendations.
These are agent-selected decisions under that delegation. This discussion ran
no execution prototype, fuzz campaign or product-behavior test.

<domain>
## Phase Boundary

Deliver ROB-01 through ROB-04: useful additional accepted-program execution
reach across mapper families 0, 1, 2, 3, 4 and 7; lifecycle and deterministic
continuation checks; reproducible fixed-corpus replay and finite sanitizer
campaigns; tested guest-loop and JAM outcomes at library and host boundaries.

Retain the C library, headless runner and libretro adapter, integer-only
instance-local core, owned code and current supported NTSC profiles. Phase 17
is complete but its PR remains unmerged; preserve its conformance protections.
An all-pass score is neither a prerequisite nor a Phase 18 obligation. This
command captures discussion context and stops before planning or execution.
</domain>

<decisions>
## Implementation Decisions

### Useful accepted-program reach

- **D-01:** Use one small owned execution fuzz/replay target with compact,
  deterministic profile recipes that generate accepted cartridge headers and
  bounded PRG/CHR bytes in memory, plus controlled frame-scheduled input and
  lifecycle operations. Keep raw loader and state-loader fuzzing separately:
  loader fuzz does not execute; state fuzz already executes two frames after
  accepted state mutations on fixed generated carts. The new target must add
  measured runtime reach rather than rename either existing claim.
- **D-02:** Combine a qualified deep-execution seed subset with mutable program,
  CHR and input data. Every family must demonstrate actual non-JAM execution
  and reach the relevant CPU/bus/PPU/mapper paths, followed by lifecycle checks.
  A successful load, harness-only coverage, immediate JAM or a matching blank
  output is insufficient. For banking families, include seeds that actually
  exercise bank/register behavior; NROM has no equivalent register obligation.
  Preserve unrestricted byte variation as well as structured seeds, so a
  narrow instruction grammar does not become the only reachable program space.
  Qualify vectors and executable placement under the selected bank modes; do
  not mistake generator-induced early stops for deep exploration. Cover all
  six families without promising every admitted subprofile or maximum-size
  board on every mutation. Choose representative size/RAM/CHR variants from
  measured value and existing loader acceptance, without adding board support.
- **D-03:** Begin implementation with a short bounded prototype. Compare against
  current loader/state fuzz under matched toolchain, instrumentation and
  resource conditions, using each target's compatible, identified corpus.
  Report per-family accepted inputs, completed frames and lifecycle operations,
  useful core-path coverage, throughput, elapsed time, observed memory and stop
  causes. Separate harness/loader edges from execution reach and account for
  different guest work. Use existing compiler coverage facilities or narrow
  test observations, not new public API or production fuzz shortcuts. Select
  budgets only after this measurement. If useful additional reach is absent,
  revise/reuse the harness or propose an explicit scope amendment; ROB-01
  remains unmet under the original acceptance contract.
- **D-04:** Recipe decoding and generation are owned, bounded C with explicit
  byte order, fixed-width arithmetic and an input-derived integer PRNG where
  useful. Identical recipe bytes mean identical generated content, inputs and
  operation sequence across platforms and across repeated calls in a process.
  Bound input size, generated allocations, frame work and operation count
  independently of libFuzzer flags; standalone replay must enforce its own
  bounds too. Handle empty/malformed inputs safely, release per-input resources
  and prevent previous-input buffers or caches from affecting the oracle.
  Keep recipes reproducible with a documented format/generator identity. No
  assembler, fuzz framework, test framework or serialization dependency.

### Lifecycle, continuation and instance independence

- **D-05:** Test public contracts using fresh instances, uninterrupted versus
  save/load continuation, and interleaved instances. Replay the same content,
  input and operation sequence on independent instances; perturb another
  instance, including a different board, and verify the reference continuation
  is unaffected. Exercise reset, state save/load and unload/reload after useful
  execution, rather than only at creation. Use saved machine bytes plus status
  and defined frame/audio observations; a shared implementation agreeing with
  itself is a determinism/lifecycle oracle, not proof of hardware accuracy.
- **D-06:** Compare only what each API result promises. After successful frames,
  compare frame number/ticks, native video, valid audio sample count/data and
  relevant canonical state. Avoid C-struct padding, host pointers and unwritten
  output bytes in portable comparisons. On STOP_JAM compare the stop/latch and
  serialized machine state; never hash partially written video/audio or treat
  unchanged success metadata as a completed frame. Initialize host buffers and
  use focused boundary guards/sanitizers. Require observations that distinguish
  suppressed execution, a skipped restore, wrong saved origin and shared
  instance state; use a few discriminating controls through the actual verdict
  path rather than a generic mutation framework.
- **D-07:** Preserve existing lifecycle meanings. Soft reset keeps documented
  CPU/cartridge RAM and mapper registers, reads the vector through current
  banks, clears JAM and applies the documented reset timing; it is not a cold
  boot. First alter retained state so retention checks are meaningful.
  Unload/reload restores the documented fresh machine and invalidates old
  memory spans. Save/load occurs at supported boundaries, including the
  existing explicit halted-state contract; refused loads leave machine state
  unchanged. Battery save generation has its separate instance-lifetime
  monotonic/change-count contract: do not demand it rewind or equal another
  instance's counter merely because serialized machine bytes agree. Reuse
  existing state, battery, reset and parity tests rather than duplicating the
  state loader's whole rejection matrix.

### Guest loops and JAM at each boundary

- **D-08:** An ordinary looping NES program is valid execution. It must return
  successful bounded frame calls with advancing frame/tick observations even
  when PC or pixels repeat. Test an owned loop fixture and a controlled JAM
  fixture. A failure to return from CPU/device catch-up is distinct from a
  guest loop; enclose calls/replays with existing process/CTest timeout tools.
  Do not add a product wall clock, arbitrary instruction-kill policy or a new
  run-ticks API to make the target convenient.
- **D-09:** Preserve native JAM semantics: the first JAM may occur during a
  partial frame; STOP_JAM latches, subsequent frame calls do not advance, reset
  clears the latch, a saved JAM state restores it and a valid pre-JAM state
  clears it. Recovery can execute JAM again if the reset vector still selects
  that opcode, so recovery fixtures must distinguish clearing from re-entry.
  Preserve the runner's documented JAM diagnostic/failure exit and battery-save
  behavior; reuse current exit-status conventions and tests.
- **D-10:** Correct the adapter's observable JAM path: for a successfully loaded
  session, each `retro_run` still polls input and sends exactly one video
  callback when registered. On JAM, present the last successfully completed,
  converted XRGB frame; use initialized black if none is valid. Always pass an
  owned valid buffer, avoiding a new dependency on NULL-frame duplication
  capability. Send no partial or stale audio through either callback form and
  do not fabricate native frame/tick progress, auto-reset, unload or terminate
  the frontend. Invalidate the presentation cache on successful content load,
  reset, unload and successful state load; a refused state load preserves it.
  The native state excludes framebuffers, so restoring a JAM state can show
  black until a complete frame exists. This is presentation policy, not a new
  serialized machine field. Test first/repeated JAM, a prior completed frame,
  transitions and recovery in the portable libretro host. Retain normal-frame
  parity and existing no-stale-audio checks. The current no-video-on-JAM path
  conflicts with libretro's documented video cadence. Timed adapter silence is
  deferred unless focused evidence from the supported pinned host demonstrates
  a pacing problem; it would need an explicit, tested scheduling amendment.

### Fixed replay and finite campaign evidence

- **D-11:** Replay a fixed permitted/generated corpus in ordinary CTest on all
  six supported platform lanes, including the normal sanitizer replay path.
  Keep stable seeds for every family, deep execution, lifecycle transitions,
  loop/JAM and any minimized findings. Prefer compact recipe files that
  generate ROM bytes in memory. Existing hygiene/manifest rules still apply to
  every binary or ROM fixture; no corpus exemption or unapproved bytes. Verify
  the required corpus is nonempty, qualified seeds reach their promised path
  and the real replay target is registered. Absence or a skipped expected test
  is a failure, not zero findings. Keep heavy coverage-guided mutation in the
  existing Linux Clang ASan/UBSan nightly pattern. Additional mutation platforms
  need concrete incremental value, not automatic sixfold replication.
- **D-12:** Copy source corpus seeds to a writable build/campaign directory
  before mutation; libFuzzer adds interesting inputs to its corpus. Keep the
  committed corpus unchanged. Set explicit finite campaign iterations and
  duration, per-input timeout and memory thresholds, as well as target-enforced
  input/allocation/work caps. Select values from the prototype's throughput,
  coverage and slow-seed/memory observations, including sanitizer overhead.
  Record fixed-corpus replay cost and a measured replay timeout separately.
  libFuzzer RSS is sampled, not a hard instantaneous allocation quota; `max_len`
  does not constrain standalone replay. An outer job timeout is the final
  enclosure and must leave time for collection/cleanup. Use the repository's
  measured job-timeout policy; do not copy the old loader/state numbers or
  invent thresholds during discussion. No speed improvement or wall-time
  performance gate is added.
- **D-13:** Retain exact crash/timeout inputs and available resource-failure
  evidence with a compact run record: tested source and target/generator
  identity, toolchain/sanitizer/build settings, corpus/input hashes, input
  recipe, command/seed/limits, platform/run identity, reached profile/operation,
  failure class and logs. Original descriptor bytes plus deterministic
  generation are the primary reproducer; generated images can be retained in
  ordinary build artifacts when useful. Upload available evidence on campaign
  failure using the existing artifact path. Verify selected-mode artifact
  collection with deliberate failures rather than assuming flags guarantee it.
  OOM input retention is mode-dependent and must be demonstrated before it is
  claimed; always retain available diagnostics and missing-evidence causes.
  Paths in durable records are relative or sanitized. Keep a small committed
  measured summary and minimized regressions so artifact expiry cannot erase
  the phase's acceptance record; no new evidence service or ledger framework.
- **D-14:** Preserve original inputs before bounded minimization. A minimized
  regression must reproduce the same failure class and, for an execution
  finding, still load an accepted cartridge and reach the intended failing
  operation. Reduction to a rejected header, an unrelated assertion or harness
  allocation failure does not count. Crash minimization and corpus coverage
  reduction are different operations. Repeated recipe execution is
  deterministic; wall-time/RSS thresholds still depend on build and host
  conditions. Distinguish reproducible core nonprogress, a slow seed, expected
  JAM, an oracle defect and infrastructure cancellation. An outer kill or lost
  artifacts produces an incomplete/failed campaign, never a completed clean
  result or an invented guest timeout. Report actual executed work and the
  stopping bound; zero findings means only no findings in that finite campaign.

### Findings, documentation and scope

- **D-15:** Every reproduced library/host contract violation needs a minimized
  regression and correction within the authorized boundary, or a visible
  explicit scope amendment with the unresolved outcome retained. Do not waive
  safety findings, silently absorb a major architecture repair, increase caps
  until green or promise a bug count. Harness/generator/oracle defects need
  correction too and cannot establish a product failure. Newly observed
  hardware accuracy differences become named findings; there is no implicit
  all-pass repair campaign. A changed accepted-profile claim requires explicit
  disposition rather than hiding that family from replay.
- **D-16:** Behavior changes carry their project-required test run by
  `cmake --workflow --preset ci`, README/public-header comment updates, and
  relevant deterministic/state/parity/perf checks in the same change. Preserve
  Phase 17's protected passes and all three deliverables. Update superseded
  preparation descriptions only where this phase changes them. Reuse owned
  helpers or justified local duplication; no new dependency is approved.
  Stay on a Phase 18 branch and use normal PR delivery. No global GSD/config
  edits, administrative evidence loop, automatic merge or release is selected.
  Milestone direction upkeep remains inside a later explicitly invoked
  complete-milestone step; do not perform that lifecycle step here.

### Agent discretion and measurements still required

Exact target/file names, recipe encoding, helper boundaries, representative
profile sizes, corpus membership, coverage presentation, operation schedule,
numeric budgets, minimization commands and finite artifact retention remain
implementation choices within these decisions. Research must verify current
accepted profiles and selected tool behavior; implementation must measure
useful added execution reach, replay/campaign cost and artifact collection.
No throughput, coverage increase, runtime budget, new failure or actual host
pacing result was measured in this discussion.

No Phase 18 directory, SPEC, prior context, plans, checkpoint or blocking
continuation existed at initialization. No pending todos matched. Project
constraints and the latest three prior phase contexts were carried forward;
retired Phase 16 administrative gates are historical, not prerequisites.
</decisions>

<canonical_refs>
## Canonical References

Downstream agents must read relevant sections before planning or implementing:

- `AGENTS.md`, `.planning/PROJECT.md`, `.planning/REQUIREMENTS.md`,
  `.planning/ROADMAP.md` — ownership, deliverables, ROB-01–04 and approved scope.
- `.planning/preparation/README.md` — reference routing and source tiers.
- `.planning/preparation/ARCHITECTURE.md` §§1–8 — instance/timing/mapper design;
  historical proposed APIs do not require adding functions absent today.
- `.planning/preparation/ENGINEERING.md` §§1–5, §7 — C, CTest, sanitizers,
  six-platform replay, measured CI budgets and hygiene.
- `.planning/preparation/CONFORMANCE.md` Oracles and CI tiers — independent
  accuracy evidence, fixture rights and limits of same-implementation agreement.
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` §§1, 4–5 — host callbacks and
  runner boundaries; current header/exit table supersede old proposals.
- `.planning/research/v4-scope/reliability.md` — existing reach, discovery gate,
  generated execution proposal and JAM risk; preliminary four-phase milestone
  alternatives are historical and do not expand the approved three phases.
- `.planning/phases/17-complete-pinned-conformance-baseline/17-CONTEXT.md` —
  protected conformance evidence and finite-observation distinctions.
- `.planning/phases/16-host-evidence-and-roadmap/16-CONTEXT.md` — exact fixture
  identity, automated host evidence and scope. Its retired mechanics remain
  superseded, as recorded in `.planning/phases/16-host-evidence-and-roadmap/16-RECOVERY.md`.
- `.planning/milestones/v3-phases/15-speed-and-close-out/15-CONTEXT.md` —
  retained performance gate and measured workflow-budget policy.
- `include/nesturbator.h`, `README.md`, `runner/exit_status.h` — current public
  lifecycle, status/output, reset, state, memory span and save-generation contracts.

Primary external references checked during discussion (2026-10-24):

- [LLVM libFuzzer](https://llvm.org/docs/LibFuzzer.html) — deterministic target
  expectations, corpus mutation, separate limits and crash minimization.
- [Clang SanitizerCoverage](https://clang.llvm.org/docs/SanitizerCoverage.html)
  — available execution coverage instrumentation; coverage is reach evidence.
- [Libretro core development](https://docs.libretro.com/development/cores/developing-cores/#retro_run)
  and `libretro/libretro.h` — input/video callback contract. No GPL/LGPL
  reference-emulator implementation source was consulted.
</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets

- `tests/fuzz/rom_loader.c` — raw public load/unload target to retain.
- `tests/fuzz/state_loader.c` — twelve generated profiles, state continuation,
  owned seed/check helpers and abort-on-oracle-failure pattern. Reuse ideas
  selectively: its two-frame fixed-program execution is the comparison
  baseline, and hashing buffers after JAM is not a valid new output oracle.
- `tests/fuzz/replay_main.h`, `tests/check.h`, `tests/state_test.h` — standalone
  replay, assertions and state/frame helpers; the shared reader's 64 MiB
  ceiling is not an appropriate new target budget by itself.
- `tests/core/test_state.c`, `tests/core/test_state_fork.c`,
  `tests/core/test_state_generation.c`, `tests/core/test_reset.c` — existing
  state, independence, battery generation and soft-reset contracts.
- `tests/libretro/libretro_host.c` — portable host callbacks, state and JAM
  no-stale-audio assertions; extend the observable halt case here.
- `tests/runner/test_movie.c`, `tests/runner/save_rom.c` — owned JAM/runner
  behavior fixtures; use existing output and save evidence.

### Established Patterns and Integration Points

- `src/cartridge.c`, `src/mapper.h`, `src/mapper_nrom.c`,
  `src/mapper_mmc1.c`, `src/mapper_uxrom.c`, `src/mapper_cnrom.c`,
  `src/mapper_mmc3.c`, `src/mapper_axrom.c` — current accepted profiles and
  actual mapper paths; no parallel global support registry.
- `src/frame.c` — tick-bounded frame loop and JAM latch/partial-output boundary.
- `libretro/libretro.c` — successful-frame conversion/callbacks and current
  early return on non-OK; presentation cache changes stay adapter-local.
- `tests/CMakeLists.txt`, `tests/cmake/fuzz_registration.cmake`,
  `tests/cmake/nightly_workflow_policy.cmake` — target/corpus registration and
  real workflow-policy integration, following existing plain C/CMake patterns.
- `.github/workflows/ci.yml`, `.github/workflows/nightly.yml`,
  `CMakePresets.json` — ordinary replay, Linux sanitizer mutation and outcomes.
- `scripts/hygiene.sh`, `tests/roms/manifest.txt` — content/provenance admission;
  generated campaign products remain in build artifacts.
</code_context>

<specifics>
## Specific Ideas

The user's preference is a small, flat dependency tree: a little justified
owned duplication is preferable to an unnecessary dependency or abstraction.
Relevant lenses were emulator/mapper execution, C/API portability, memory
safety and liveness, host presentation, determinism, test-oracle quality,
CI/resource cost, reproducibility, provenance and maintenance. Unrelated UI
framework, distributed-system and language comparisons add no value here.

The three advisors converged, and adversarial review sharpened status-defined
comparisons, retained reset state, battery-generation semantics, presentation
cache invalidation, standalone replay limits, sampled RSS and mode-dependent
artifact collection. Further documentation-only fan-out had diminishing
returns; remaining uncertainty requires the bounded prototype.
</specifics>

<deferred>
## Deferred Ideas

- PAL/Dendy, new boards/subprofiles, expansion audio, FDS, peripherals, GUI or
  device code, achievements/netplay and a new public host/tick API.
- Broad accuracy repair, guaranteed defect yield, an all-pass score, a new
  fuzz engine/dependency, a generic evidence service or test framework.
- Additional real-RetroArch OS coverage or sanitizer mutation platforms absent
  a concrete platform-specific finding; timed JAM silence absent pacing evidence.
- Existing nightly vectors-full failure remains separate unresolved debt.
- Phase 17 delivery and normal milestone closeout/release/direction refresh
  remain their respective authorized lifecycle work, not this discussion step.
</deferred>

---
*Phase: 18-execution-and-lifecycle-robustness*
*Context gathered: 2026-10-24*
