# Phase 18: Execution and lifecycle robustness - Research

**Researched:** 2026-10-24
**Domain:** deterministic C execution fuzzing, lifecycle contracts and libretro host behavior
**Confidence:** MEDIUM

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

DATA_91a7e0c4_START
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
DATA_91a7e0c4_END

### the agent's Discretion

DATA_b3f85d12_START
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
DATA_b3f85d12_END

### Deferred Ideas (OUT OF SCOPE)

DATA_60c9a1fe_START
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
DATA_60c9a1fe_END
</user_constraints>


<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|-----------------|
| ROB-01 | A small owned fuzz/replay target executes bounded accepted profiles for all six supported board families with generated program/CHR bytes and controlled input. | Raw loader fuzz does not execute; state fuzz runs short continuations after accepted state mutations on fixed generated carts. Require measured additional guest/core-path reach. [VERIFIED: tests/fuzz/rom_loader.c:9-20; tests/fuzz/state_loader.c:158-190,206-275] |
| ROB-02 | The target exercises reset, save/load and unload/reload, checking instance independence and deterministic continuation through existing API contracts. | Public lifecycle contract and existing reset/state/fork checks provide reusable assertions. [VERIFIED: include/nesturbator.h:412-430,450-469,477-519; tests/core/test_reset.c:519-536] |
| ROB-03 | Normal CI replays a fixed permitted/generated corpus; finite nightly sanitizer campaigns retain deterministic crash/timeout reproducers with measured time, memory and execution budgets. | CTest replay, registration guard and Linux nightly artifact flow are direct seams; campaign values await measurement. [VERIFIED: tests/CMakeLists.txt:979-1040; tests/cmake/fuzz_registration.cmake:1-15; .github/workflows/nightly.yml:236-297] |
| ROB-04 | Guest loops and JAM have tested progress/stop behavior, including the adapter's observable halt policy; any reproduced library/host contract violation receives a minimized regression and correction or explicit rescoping. | Core frame loop is tick-bounded; current adapter returns before callback dispatch on non-OK. Extend portable host and runner checks. [VERIFIED: src/frame.c:6-72; libretro/libretro.c:182-239; tests/libretro/libretro_host.c:975-1005; tests/runner/test_movie.c:149-176] |
</phase_requirements>

## Summary

Use one owned in-process execution fuzz/replay target with deterministic, bounded recipes, generated accepted carts, controlled input and per-input isolation. Keep loader fuzz and state fuzz as independent baselines: the loader only loads/unloads, while the state target executes a short continuation on fixed generated carts. [VERIFIED: tests/fuzz/rom_loader.c:9-20; tests/fuzz/state_loader.c:158-190,206-275]

Start with D-03's short measured prototype. Match toolchain, instrumentation, resources and compatible corpora; report guest execution reach separately from harness/loader edges, along with accepted inputs, frames, lifecycle operations, throughput, elapsed time, memory and stop causes. No useful-reach delta or numeric budget has been measured. [VERIFIED: 18-CONTEXT.md D-03; ASSUMED: new-target reach and budgets]

**Primary recommendation:** Reuse existing CMake/CTest, libFuzzer, ASan/UBSan and portable replay. Generate from the current loader profile gate, prove non-JAM execution and relevant bank/register paths, then exercise lifecycle behavior. Put input/allocation/frame/operation bounds in target code and choose campaign limits only after measurement. [VERIFIED: tests/CMakeLists.txt:979-1034; src/cartridge.c:77-149; 18-CONTEXT.md D-01–D-16]

## Architectural Responsibility Map

| Capability | Primary tier | Secondary | Rationale |
|------------|--------------|-----------|-----------|
| Admission, execution, reset and native state | API / Backend | Test harness | The library owns validation, device execution and per-instance machine state. [VERIFIED: include/nesturbator.h:261-335,412-430,477-519; src/frame.c:38-72] |
| Recipes, deterministic replay and lifecycle oracle | Test/build | API / Backend | Keep fuzz-only generation in tests and invoke public API contracts. [VERIFIED: tests/fuzz/rom_loader.c:9-20; tests/fuzz/state_loader.c:121-147; 18-CONTEXT.md D-01, D-04] |
| Runner JAM diagnostic/save behavior | Runner | API / Backend | Preserve current process contract and reuse runner tests. [VERIFIED: runner/exit_status.h:13-20; tests/runner/test_movie.c:149-176; tests/CMakeLists.txt:148-184] |
| Video/audio policy after JAM | Libretro adapter | Portable host | Converted-frame presentation and callbacks are adapter policy, not machine state. [VERIFIED: libretro/libretro.c:182-239; 18-CONTEXT.md D-09–D-10] |
| Corpus and campaign evidence | CI/test infrastructure | Local replay | Fixed replay is portable; mutation and sanitizers already run in Linux nightly. [VERIFIED: tests/CMakeLists.txt:979-1040; .github/workflows/nightly.yml:236-297] |

## Project Constraints (from AGENTS.md)

- Own all emulator/test code; libretro.h is the sole vendored file; core links only C memory functions. [VERIFIED: AGENTS.md:14-15]
- Keep library, headless runner and libretro adapter only; no window/device code. [VERIFIED: AGENTS.md:16-17]
- No ROM/BIOS bytes outside permitted manifest entries; avoid personal paths/names/emails; use hygiene. Generate corpus ROMs in memory. [VERIFIED: AGENTS.md:18-21; 18-CONTEXT.md D-11]
- Clean-room only: hardware documents and test behavior; never open or retain GPL/LGPL emulator source. [VERIFIED: AGENTS.md:22-24]
- Integer-only, deterministic, instance-local core. [VERIFIED: AGENTS.md:25-26]
- Behavior changes ship with the required test, README and public-header comment updates. [VERIFIED: AGENTS.md:27-30]
- One GSD step per command, phase branch, PR and Conventional Commit title. [VERIFIED: AGENTS.md:31-34]
- Product-roadmap refresh is for the authorized milestone steps; no extra lifecycle step here. [VERIFIED: AGENTS.md:36-45; 18-CONTEXT.md D-16]
- Small modules, fixed-width types, cited hardware comments, no needless dependency. [VERIFIED: AGENTS.md:49-56]
- No configured project skills were found. [VERIFIED: .claude/CLAUDE.md:62-67; .planning/config.json:71]

## Standard Stack

| Component | Version/availability | Use |
|-----------|----------------------|-----|
| C | C17, extensions off | Owned recipe target and checks; required by project. [VERIFIED: .claude/CLAUDE.md:20-31] |
| CMake/CTest/Ninja | CMake min 3.25; local versions 4.4.3 / 4.4.3 / 1.13.2 | Existing cross-platform build and replay. [VERIFIED: CMakePresets.json:1-10,77-101; local version probe] |
| Clang sanitizers/coverage | Local Apple Clang 21.0.0; compile/link probe passed | Reach evidence and memory/UB diagnostics. [VERIFIED: local compiler probe; .github/workflows/nightly.yml:253-272] |
| LLVM libFuzzer | Existing Linux nightly; local macOS runtime missing | Mutation of recipe bytes; retain the portable replay target. LLVM documents it as in-process and coverage-guided. [CITED: llvm.org/docs/LibFuzzer.html] |

No package install is proposed. Reuse the existing assertion/process helpers and do not add a framework, assembler, serializer or fuzz engine. [VERIFIED: tests/check.h; tests/runner/test_process.h; 18-CONTEXT.md D-04, D-16]

## Architecture Patterns

### Data flow

~~~mermaid
flowchart TD
  A[Recipe corpus or mutation] --> B[Bounded deterministic decoder/generator]
  B --> C{Valid recipe and accepted profile?}
  C -- no --> D[Safe bounded return]
  C -- yes --> E[Fresh instance and generated cart]
  E --> F[Bounded frames and controlled input]
  F --> G{Frame status}
  G -- success --> H[Compare defined frame/audio/state observations]
  G -- JAM --> I[Compare latch and serialized state only]
  H --> J[Reset / save-load / second instance / unload-reload]
  I --> J
  J --> K[Portable CTest replay]
  J --> L[Finite Linux sanitizer campaign]
  L --> M[Retained input and run evidence]
  N[Adapter success] --> O[Convert/cache complete XRGB and send audio]
  P[Adapter JAM] --> Q[Cached complete XRGB or initialized black; no audio]
  O --> R[Input poll and one video callback when registered]
  Q --> R
~~~

### Concrete source seams

Extend the current target registration in tests/CMakeLists.txt and tests/cmake/fuzz_registration.cmake. Reuse tests/fuzz/replay_main.h for standalone replay only; its file-size ceiling does not replace target-side work caps. Extend lifecycle checks in tests/core/test_reset.c, test_state_fork.c, test_state_generation.c and tests/libretro/libretro_host.c. [VERIFIED: tests/CMakeLists.txt:979-1040; tests/cmake/fuzz_registration.cmake:1-15; tests/fuzz/replay_main.h:14-48; tests/core/test_reset.c:519-536]

### Accepted profiles

The current supported mapper list is quoted verbatim:

DATA_4fc1a9b7_START
~~~c
The supported mappers are 0, 1, 2, 3, 4 and 7.
~~~
DATA_4fc1a9b7_END
[VERIFIED: include/nesturbator.h:308]

The public header documents each accepted board profile, including submapper, bank-count and board-specific constraints. The loader's board_profile_ok is the actual acceptance gate; generate representative variants from it and call the public loader rather than creating a parallel support registry. [VERIFIED: include/nesturbator.h:282-334; src/cartridge.c:77-149,151-231]

### Recipe and reach pattern

Decode a compact little-endian recipe into an accepted profile, bounded PRG/CHR bytes, frame-scheduled input and lifecycle actions. Keep structured execution seeds plus unrestricted mutation. Each family must show accepted load, non-JAM guest work and relevant CPU/bus/PPU/mapper reach; banked families require a post-register/bank observable. [VERIFIED: 18-CONTEXT.md D-01–D-04]

LibFuzzer supplies a byte-array-and-size callback. Recipe parsing and fuzz-only logic stay in the test target; no public API or production shortcut is needed. [CITED: llvm.org/docs/LibFuzzer.html; VERIFIED: 18-CONTEXT.md D-04]

~~~c
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    struct recipe recipe;
    if (!decode_bounded_recipe(data, size, &recipe))
        return 0;
    run_generated_profile_and_lifecycle(&recipe);
    return 0;
}
~~~

This is a schematic test-side pattern, not a new API.

### Lifecycle and halt oracle

Use fresh instances; compare uninterrupted against save/load continuation; interleave and perturb a second instance; exercise reset and unload/reload after altering retained state. On success compare defined frame number/ticks, native video, valid audio count/data and canonical state. On JAM compare latch/status/serialized state only, never partial buffers or unchanged success metadata. [VERIFIED: include/nesturbator.h:190-252,412-430,450-469,477-519,630-638; src/frame.c:52-72; 18-CONTEXT.md D-05–D-07]

Relevant native status values:

DATA_0e4c2a7f_START
~~~c
NESTURBATOR_OK = 0,
NESTURBATOR_STOP_JAM = 6,
NESTURBATOR_ERR_CARTRIDGE = 7,
~~~
DATA_0e4c2a7f_END
[VERIFIED: include/nesturbator.h:73-92]

A successful soft reset retains documented RAM/mapper state, clears JAM, uses the current bank for the vector and applies reset timing. Unload returns a fresh no-cartridge machine and invalidates old memory spans. Battery generation is a separate instance-lifetime counter. [VERIFIED: include/nesturbator.h:282-340,412-430,450-469; 18-CONTEXT.md D-07]

An ordinary guest loop should return bounded successful frame calls with advancing frame/tick observations; core nonprogress is separate and belongs under a process/CTest deadline. [VERIFIED: src/frame.c:38-59; 18-CONTEXT.md D-08]

For the adapter, libretro docs require one video callback per retro_run and at least one input poll. Current code returns before callback dispatch on JAM. Keep completed-frame cache in the adapter, invalidate it on the locked transitions, preserve it on refused state load, and emit cached image or initialized black with no audio on JAM. Extend the portable host test. [CITED: docs.libretro.com/development/cores/developing-cores/#retro_run; VERIFIED: libretro/libretro.c:182-239,285-332; tests/libretro/libretro_host.c:975-1005; 18-CONTEXT.md D-09–D-10]

### Pitfalls

- Successful load, harness coverage, early JAM or blank output is not useful execution reach. [VERIFIED: 18-CONTEXT.md D-02]
- Wrong vector/bank placement can stop exploration before the target path. [VERIFIED: 18-CONTEXT.md D-02; include/nesturbator.h:282-323]
- Fuzzer flags do not bound standalone replay or generated work. [VERIFIED: 18-CONTEXT.md D-04, D-12]
- Battery-generation counters are not serialized machine state; do not require rewind/equality across instances. [VERIFIED: include/nesturbator.h:460-469; 18-CONTEXT.md D-07]
- Reset is soft, and the vector may re-enter JAM after the latch clears. [VERIFIED: include/nesturbator.h:412-421; 18-CONTEXT.md D-07, D-09]
- Treat outer cancellation, missing corpus/tests or missing artifacts as incomplete; minimize only if failure class and intended operation persist. [VERIFIED: 18-CONTEXT.md D-11–D-15]
- LLVM RSS is sampled, so pair campaign flags with target-side caps and a measured outer job timeout. [CITED: llvm.org/docs/LibFuzzer.html; VERIFIED: 18-CONTEXT.md D-12]

## Runnable Validation Commands

Required full project workflow:

DATA_a827d530_START
~~~bash
cmake --workflow --preset ci
~~~
DATA_a827d530_END
[VERIFIED: AGENTS.md:27-30; CMakePresets.json:17-23]

Existing portable fuzz replay after configure/build:

DATA_6f2d19c8_START
~~~bash
ctest --test-dir build/ci -R 'fuzz[.]regress|fuzz[.]state[.]regress' --output-on-failure
~~~
DATA_6f2d19c8_END

Current replay registrations:

DATA_79c05b4a_START
~~~cmake
foreach(required fuzz.regress fuzz.state fuzz.state.regress)
  string(REPLACE "." "\\." pattern "${required}")
  if(NOT inventory MATCHES "\"name\"[ \t]*:[ \t]*\"${pattern}\"")
    message(FATAL_ERROR "CTest does not register the required ${required} test")
  endif()
~~~
DATA_79c05b4a_END
[VERIFIED: tests/cmake/fuzz_registration.cmake:10-14]

The CTest build tree is preset-derived:

DATA_51b4d260_START
~~~json
"name": "base",
      "hidden": true,
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/${presetName}"
    },
~~~
DATA_51b4d260_END
[VERIFIED: CMakePresets.json:6-10]

For mutation, extend the Linux nightly target and artifact collection, copy seeds to writable campaign storage, and set new numeric limits only after the prototype. [VERIFIED: .github/workflows/nightly.yml:236-297; 18-CONTEXT.md D-03, D-11–D-14]

## Package Legitimacy Audit

Not applicable: no external package is recommended or approved. [VERIFIED: 18-CONTEXT.md D-04, D-16]

## Security Domain

Project security enforcement is explicitly false; that disables the ASVS gate, not the memory-safety, malformed-input, resource-exhaustion or liveness obligations of this phase. [VERIFIED: .planning/config.json:49; 18-CONTEXT.md D-04, D-08, D-12–D-15]

## Environment Availability

| Dependency | Available | Version / observation | Fallback |
|------------|-----------|-----------------------|----------|
| C17 compiler | Yes | Apple Clang 21.0.0 | Portable CI replay |
| CMake / CTest / Ninja | Yes | 4.4.3 / 4.4.3 / 1.13.2 | — |
| ASan/UBSan + SanitizerCoverage | Yes | Compile/link probe passed; no product target executed | Linux nightly |
| libFuzzer runtime | No locally | Apple Clang linker reports missing macOS fuzzer runtime | Existing Linux nightly |
| llvm-cov / llvm-profdata | Yes via xcrun | LLVM 21.0.0 | Narrow test observations |

The probes were tool availability checks only; no execution prototype or product test was run. [VERIFIED: local compiler/link probes; 18-CONTEXT.md D-03]

## Assumptions Log

| # | Claim | Risk if wrong |
|---|-------|---------------|
| A1 | [ASSUMED] Recipe format and exact target/test names remain discretion. | Extra encoding work or registration changes. |
| A2 | [ASSUMED] Corpus members and representative profile variants await reach measurements. | Seeds may load but not exercise useful paths. |
| A3 | [ASSUMED] Target caps and finite replay/campaign budgets are unknown until the prototype. | Arbitrary bounds could hide reach or destabilize CI. |
| A4 | [ASSUMED] OOM/timeout artifact retention needs deliberate failure verification. | A campaign could lack a valid reproducer. |

## Open Questions

1. Does the new harness add useful per-family runtime reach beyond current loader/state fuzz? Measure it before claiming ROB-01. [VERIFIED: 18-CONTEXT.md D-03]
2. Which fixed recipes prove vector placement and bank/register effects under current accepted profiles? [VERIFIED: src/cartridge.c:77-149; 18-CONTEXT.md D-02]
3. What finite budget and artifact mode are reliable after sanitizer overhead and slow-seed observations? [VERIFIED: 18-CONTEXT.md D-12–D-14]

**Nyquist validation:** omitted because the config explicitly sets workflow.nyquist_validation false. [VERIFIED: .planning/config.json:25]

## Sources

### Primary project sources

- AGENTS.md and .claude/CLAUDE.md; .planning/REQUIREMENTS.md, STATE.md, ROADMAP.md, config.json and phase context.
- include/nesturbator.h; src/cartridge.c; src/frame.c; libretro/libretro.c; runner/exit_status.h.
- tests/fuzz/rom_loader.c, state_loader.c, replay_main.h; tests/CMakeLists.txt; tests/cmake/fuzz_registration.cmake; existing reset/state/libretro/runner tests.
- CMakePresets.json and .github/workflows/nightly.yml; preparation README and ARCHITECTURE, ENGINEERING, CONFORMANCE, LIBRETRO-AND-RUNNER references; v4-scope reliability research.
- No GPL/LGPL emulator implementation source was opened. [VERIFIED: research activity]

### Official external documentation

- [LLVM libFuzzer](https://llvm.org/docs/LibFuzzer.html) — callback/corpus replay, coverage, bounds and artifacts; accessed 2026-10-24.
- [Clang SanitizerCoverage](https://clang.llvm.org/docs/SanitizerCoverage.html) — coverage modes and instrumentation; accessed 2026-10-24.
- [Libretro core development](https://docs.libretro.com/development/cores/developing-cores/) — retro_run video callback and input polling contract; accessed 2026-10-24.

Context7 and the ctx7 CLI were unavailable, so official primary pages were browsed directly. [VERIFIED: tool discovery and command probe]

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — existing patterns and local tool versions inspected; no dependency proposed.
- Architecture: MEDIUM — contracts are source-verified; useful reach awaits the required prototype.
- Pitfalls: MEDIUM — grounded in code/decisions; numeric budgets and artifact behavior remain unmeasured.

**Research date:** 2026-10-24  
**Valid until:** 2026-11-23


