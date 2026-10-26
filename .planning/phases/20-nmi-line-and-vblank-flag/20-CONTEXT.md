# Phase 20: NMI line and VBlank flag - Context

**Gathered:** 2026-10-26
**Status:** Ready for planning

<domain>
## Phase Boundary

Deliver NMI-01, NMI-02, KEEP-02 and KEEP-03: reads of `$2002` and writes to
`$2000` around NTSC VBlank start/end observe the correct flag and interrupt
behavior; the six named AccuracyCoin rows pass at `6bee919`; new machine
history survives save/load, rewind and run-ahead. Protect the current PASS set.

The six rows are `$2002 flag timing`, `NMI Control`, `NMI Timing`,
`NMI Suppression`, `NMI at VBlank end` and `NMI disabled at VBlank`.
Phase 20 includes the NMI polling needed by NMI Control 8/9. Phase 21 retains
IRQ I-flag latency, the corresponding branch/IRQ cases, and BRK/IRQ vector
hijacking. A necessary shared seam may support both phases without claiming
Phase 21's outcomes here.

</domain>

<decisions>
## Implementation Decisions

The owner selected all four areas and delegated further decisions: consider
relevant stakeholder perspectives, tradeoffs, primary sources and adversarial
cases to diminishing returns, then automatically adopt one coherent set of
recommendations. Three specialist reviews covered race evidence, timing and
alignment, and state/host proof; the orchestrator covered attribution and
reconciled their conclusions. These are adopted constraints for research and
planning, not claims of implemented or measured improvements.

### Race-test evidence

- **D-01:** Use three complementary layers: owned PPU-dot tests for flag
  transitions; owned CPU bus/program tests for CPU-visible reads, writes and
  NMI delivery; pinned AccuracyCoin as integration evidence. No layer alone
  establishes the whole contract. Reuse `ppu_fixture.h`, `ines.h`, `check.h`
  and the existing campaign rather than adding a test framework or dependency.
- **D-02:** Derive expectations from the pinned test's assembly/comments and
  hardware documentation, never from the current implementation's output.
  Record source, coordinate convention, initial state, event order and any
  derivation beside each expected table. AccuracyCoin is primary evidence of
  test intent, not independent proof that every outcome was hardware-measured.
  Curated NESdev explanations corroborate hardware behavior; distinguish
  transistor-analysis observations from general wiki summaries.
- **D-03:** Cover both VBlank-set and pre-render-clear boundaries, including
  points before, on and after each transition and normal-operation controls
  outside the race window. For the start window, distinguish flag suppressed,
  flag observed but NMI suppressed, and flag observed with NMI retained.
  Express offsets relative to a named sample event, not an ambiguous `dot`.
  Exercise ordinary and mirrored register addresses through the real decoder.
- **D-04:** Cover both `$2002 flag timing` subtests: flag clearing and flag
  setting. VBlank bit 7 is sampled at M2 rise; sprite bits 6/5 are sampled at
  M2 fall. The pinned assembly documents this distinction. Preserve status
  low-bit/open-bus behavior, write-toggle reset and register side effects.
  Direct PPU-register helper tests cannot establish the split bus read.
- **D-05:** Cover `$2000` disabled/enabled, unchanged `$80` rewrites, `$00`
  then `$80` re-enabling during VBlank, enabling after `$2002` cleared the
  flag, and enabling/disabling around both VBlank transitions. Include the
  extra-NMI pattern associated with Bases Loaded II without bundling a game
  or making a title-specific exception. NMI Control 8/9 must distinguish the
  handler running after the instruction following `STA $2000` from running
  before that instruction after `INC $2000`; preserve both RMW writes.
- **D-06:** Write defect reproducers before their fixes. Already-correct
  cases remain passing guards, following Phase 19's D-10 amendment; do not
  falsely require every new test to fail first. If an intermediate commit
  needs expected-failure handling, make the expected assertion/outcome
  explicit, distinguish it from a crash or setup failure, and remove that
  handling with the fix. Final `ci` has ordinary passing assertions.
- **D-07:** Use a bounded set of controls that distinguish plausible wrong
  models: move flag set/clear by one dot; sample all three status bits at one
  edge; revoke an already captured NMI on a read/disable/clear; create an
  extra NMI on unchanged `$80`; omit the RMW dummy write; service an NMI one
  instruction early. Each selected control must trip its intended oracle.
  A tolerated AccuracyCoin shift is not a substitute for an absolute-dot
  owned test. No generic mutation-testing project is added.

### CPU/PPU alignment and NMI ordering

- **D-08:** Use one physical NMI-line model plus CPU capture and polling
  history. Distinguish the PPU flag, NMI enable, asserted line, sampled edge,
  pending request and instruction poll result. A shorter pulse can miss
  capture; lowering the line later must not erase an already captured
  request. Clear/consume history only at its documented reset/service event,
  not indiscriminately at `$2002`, `$2000` disable or pre-render clear.
  Document active-low hardware polarity versus any asserted-high boolean.
- **D-09:** Keep the existing integer half-master-clock timeline and eager
  PPU catch-up at the two M2 edges. Make `$2002`'s early VBlank sample and
  later sprite sample explicit. Preserve one access's side effects and the
  original bus cycle count; do not perform a second register read merely to
  obtain the second sample. Keep APU phase, mapper stamps and DMA repeated
  accesses coherent with that timeline.
- **D-10:** Treat PPUSTATUS sampling and CPU NMI-input sampling as separate
  events. The old research alternately prescribes M2 rise, M2 fall or an
  alignment shift without resolving same-edge ordering. Research must map
  CPU detector stages to the existing M2 events from primary analysis and
  pinned test behavior, then probe flag-set/read-clear/sample/poll ordering.
  M2 fall is the source-backed detector candidate from the specialist review;
  same-edge side-effect ordering remains to be demonstrated. Do not mistake
  the proven bit-7 read sample at M2 rise for proof of NMI detector timing.
- **D-11:** First test the existing deterministic alignment. AccuracyCoin's
  relative synchronization and accepted table shifts do not establish a
  unique boot/reset alignment or justify adding a constant. Add/change one
  machine-wide fixed phase only if a minimal reproducer shows a residual
  alignment defect after correct event ordering, and the result satisfies
  every target table and regression guard. Never select alignment per row,
  program counter, title or run; never randomize it.
- **D-12:** A table's documented alignment alternative applies to the whole
  sequence, except entries the pinned source explicitly ignores. Do not
  choose an alternative independently for each byte, loosen the oracle or
  shift VBlank itself to fit a table. Any persistent phase/history needed
  for continuation belongs to the instance and save-state contract.
- **D-13:** Keep changes local to existing PPU, bus and CPU seams. Prefer a
  small owned helper or justified duplication to a new dependency. Do not
  introduce a generalized scheduler, second accuracy mode, public trace API,
  configurable chip profiles or cycle-stepping CPU rewrite for this phase.
  Exact helper/field layout is planner discretion, constrained by evidence.
  Keep the PPU-to-CPU link boundary and the IRQ line's existing single writer.
  Review the NMI poll against instruction classes, including branch exceptions
  where necessary for NMI; do not expand into Phase 21's IRQ fixes.

### Save-state race coverage

- **D-14:** Keep the approved state format **1 to 2** and behavior revision
  **7 to 8**. Phase 19 already took revision 7; older research's Phase 20
  `6 to 7` wording is superseded. Update version pins, README and public-header
  comments with the behavior that changes. Preserve the existing format-error
  precedence and reject format 1 without mutating the instance or save state.
- **D-15:** Serialize every historical value needed to continue: detector
  history, pending/polled NMI and any persistent suppression/read-sample/phase
  state. Reuse the existing state visitor and validation path. A genuinely
  derived current line can be recomputed; an earlier sampled line or edge
  cannot be reconstructed from current PPUCTRL/PPUSTATUS. A local temporary
  fully consumed inside one synchronous bus call need not become state.
  Prefer one source of truth over redundant serialized booleans.
- **D-16:** Validate field bounds and actual reachable combinations before
  applying state. Historical latches may legitimately differ from the live
  line, so a validator must not reject the race it is meant to preserve.
  Extend existing rejection tests with CRC-correct malformed fields and
  format-1 rejection; retain whole-instance/continuation-unchanged checks.
  Canonical save/load/save equality supplements behavioral continuation.
- **D-17:** Separate exact race-state proof from public host replay. The
  public API saves a whole machine between frames, and `cpu_step` completes
  an instruction; it cannot pause a caller mid-instruction. Use owned
  internal fixtures to exercise set-dot, clear-dot and edge-before-poll
  state, using the production state visitor/loader wherever a resumable,
  valid machine state can be established. Prove and label whether each
  cutpoint is reached by execution or constructed as a valid fixture; resume
  only where all CPU execution state is represented. No asynchronous save,
  thread sampling, production callback or debugger API is authorized.
- **D-18:** The researcher/planner must resolve exact cutpoint feasibility
  explicitly; do not assume whole CPU cycles make event-dot snapshots
  impossible. With rendering off and origin `(261,0)`, set-dot ordinals are
  `82523 + n*89342`, and clear-dot ordinals are `1 + n*89342`. Some are
  divisible by three: set ordinal 261207 and clear ordinal 89343 can coincide
  with CPU-cycle ends. This is a derived candidate, not a run result; account
  for reset cycles, instruction lengths, PPU phase and synthesizer history
  in the actual fixture. A nearest-frame snapshot does not satisfy an exact-dot claim,
  and a made-up mid-instruction state cannot prove full-machine continuation.
  If a requested cutpoint cannot be represented, record the specific limit
  and the required acceptance clarification before claiming KEEP-03. Preserve
  exact transition/latch tests plus public replay coverage; do not silently
  weaken the roadmap or implement a CPU state machine to hide the mismatch.
- **D-19:** Extend `core.state_fork.*`, `runner.state.*`, core run-ahead and
  `libretro.runahead` using owned NMI-sensitive programs and existing helpers.
  Compare uninterrupted, restored and speculative runs with native state,
  frame/tick/audio results and a guest-observable NMI counter or event marker.
  Host equality alone can share the same core bug; the race tests provide
  independent expectations. Exercise real save/load work, stale/omitted-load
  controls, and at least a targeted omitted-history control. Preserve the
  existing multi-board matrix; detailed race sweeps can use the smallest NROM
  fixture rather than multiplying identical PPU tests across all boards.
  Include two instances with different latch histories to catch shared state.

### Change attribution, CI and delivery

- **D-20:** Use one Phase 20 branch/PR and ordered green changes: reproducers;
  split register sampling and NMI line/capture with their state implications;
  NMI polling; final evidence refresh. Exact subdivision follows dependencies,
  but each causal behavior change carries its tests, README/header comments,
  moved hash explanations and measured campaign diff. Keep state format and
  behavior revision coherent throughout the PR; no release with unsaved latches.
- **D-21:** The actual starting scoreboard is **108 pass / 38 fail**, all 146
  scored rows terminal, with five DRAW entries outside the score. Protect
  Phase 19's newly passing rows, not just v5 opening's 101. Preserve the cold,
  same-invocation protected-main comparison and existing result provenance.
  Six target passes alone imply 114/32 if nothing else changes; this is an
  expectation, never a hard-coded verdict or a reason to chase neighbors.
  Record incidental improvements and investigate every lost PASS.
- **D-22:** Keep the AccuracyCoin pin, schemas, guest caps and current CI
  placement. Use focused tests while iterating; run the full campaign at
  causal behavior milestones and final `cmake --workflow --preset ci`.
  Preserve protected PASS checks on every supported lane, six-platform hash
  equality, CPU vectors, MMC3, Holy Mapperel and boot-to-play. Reuse existing
  sanitizer/performance/host checks; do not add a full campaign to every lane,
  loosen the +1% performance policy or raise a timeout without its required
  measurement. New tests must catch a distinct failure, not mirror code.
- **D-23:** Re-pin only observed frame/audio/state hashes in the causal change,
  with old/new values and a hardware/state reason. Explain audio movement
  through CPU/APU event timing; it is not automatically justified by a video
  fix. Refresh scoreboard, activation revision, fingerprint and metadata from
  the actual campaign, preserving source-dirty status honestly. Existing dirty
  local baseline provenance is not clean hosted evidence; retain that debt
  until an identified clean hosted run supplies the missing confirmation.
- **D-24:** Keep the core integer-only, allocation-free while running, and
  independent per instance. No new runtime/test dependency, bundled ROM,
  window/device code, special-case title fix or global mutable latch. Preserve
  clean-room boundaries: documentation and permitted test behavior only;
  reference emulators, if needed, are released binaries, never source.
  Keep existing model-config edits out of the discussion commits. This
  command ends after context capture; planning, implementation, PR and merge
  remain separate steps.

### Agent discretion and evidence still to obtain

- Helper names, minimal latch representation, exact fixture layout, diagnostic
  formatting and bounded sweep size are implementation choices.
- Research must establish same-edge clear/capture order, verify the default
  alignment against the complete pinned tables, and demonstrate the selected
  reachable serialization cutpoints (including audio state). No runtime probes or implementation tests were
  run during this discussion; no target row is newly claimed passing.
- Older research's global alignment-shift recommendation, conflated sample
  phases, stale behavior revision and Phase 19 sprite hypotheses are not locked
  choices. The pinned NMI-at-end source also does not support the earlier
  `PLA` versus `NOP` description; use its actual two sequences when deriving
  the reproducer. Reconcile these in phase research rather than propagating them.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read the relevant sections before planning or implementing.**

### Scope and settled decisions
- `.planning/ROADMAP.md` — Phase 20 criteria and Phase 21 boundary.
- `.planning/REQUIREMENTS.md` — NMI-01/02 and KEEP-01/02/03.
- `.planning/PROJECT.md` — deterministic, owned core and three deliverables.
- `.planning/phases/19-jam-reporting-and-sprite-0-hit/19-CONTEXT.md` — D-10
  amendment, revision 7, settled sprite behavior and no-dependency preference.
- `.planning/preparation/GSD-HANDOFF.md` — one step, automated verification.

### Hardware and implementation research
- `.planning/preparation/ARCHITECTURE.md` — sections 2/3/7: time, CPU and state.
- `.planning/preparation/DECISIONS.md` — established architecture and ownership.
- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md` — VBlank/status sources.
- `.planning/preparation/NES-HARDWARE-CPU-APU.md` — sections 2.1/2.3 and source map.
- `.planning/preparation/CONFORMANCE.md` — pin, licenses and bounded campaign.
- `.planning/preparation/ENGINEERING.md` — CI, performance and hygiene.
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` — public host/state contracts.
- `.planning/research/ARCHITECTURE.md` — NMI/state integration; speculative
  ordering/alignment recommendations are subject to D-09 through D-12.
- `.planning/research/FEATURES.md` — G1/G2 and pinned test intent; verify tables
  in the assembly and apply the corrections above.
- `.planning/research/PITFALLS.md` — #1/2/3/6/7/10/12 and source-derived oracles.
- `.planning/research/SUMMARY.md` — steps 3/4; stale revision/sprite claims are
  superseded by Phase 19 evidence and this context.

### Existing evidence and production interfaces
- `include/nesturbator.h` — format/revision, frame-boundary state contract.
- `tests/accuracy/scoreboard.txt` — current 108-row PASS set and target failures.
- `tests/accuracy/baseline-metadata.txt` — campaign identity and dirty provenance.
- `tests/accuracy/measurements.md` — Phase 19 re-record and accepted bounds.
- `tests/runner/hashes.txt` — frame/audio/state pins and state format row.

### External sources (read on 2026-10-26)
- [Pinned AccuracyCoin assembly](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm)
  — primary test-intent source: `TEST_2002FlagTiming`, `TEST_NMI_Control`,
  timing/suppression/end/disable sequences and their answer tables.
- [Pinned AccuracyCoin README](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/README.md)
  — primary error-code descriptions, especially Control 8/9 and both flag tests.
- [NESdev CPU interrupts](https://www.nesdev.org/wiki/CPU_interrupts),
  [PPU frame timing](https://www.nesdev.org/wiki/PPU_frame_timing),
  [PPU registers](https://www.nesdev.org/wiki/PPU_registers) — curated hardware
  explanations; distinguish their general rules from pinned test tolerance.
- [Visual circuit tutorial](https://www.nesdev.org/wiki/Visual_circuit_tutorial)
  — primary Visual6502-derived transistor analysis for the detector phase,
  not a direct measurement of the RP2A03G.
- [CPU_ALL](https://www.nesdev.org/wiki/CPU_ALL) — curated explanation of
  RP2A03G M2 versus CPU clock edges; verify the target-revision applicability.
- [Official Libretro run-ahead guidance](https://docs.libretro.com/guides/runahead/)
  — frame-based speculative execution and save/load order.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `tests/ppu/ppu_fixture.h`, `tests/ines.h`, `tests/check.h`: owned loaded-NROM
  fixtures, guest snippets and assertions without a new dependency.
- `tests/core/test_nmi_edge.c`: end-to-end unchanged-enable rewrite guard.
- `tests/ppu/test_registers.c`: direct register guards; update assumptions that
  PPU helpers directly clear/set the CPU's pending request.
- `tests/state_test.h`, `tests/core/test_state_fork.c`,
  `tests/core/test_state_reject.c`, `tests/core/test_state_runahead.c`:
  serialized state, rejection, fork and deterministic-continuation helpers.
- `tests/libretro/libretro_host.c`, `tests/runahead_model.h`: public host oracle
  and wrong-host controls; extend the existing matrix.

### Established Patterns
- `src/state.c` owns serialization/validation through field visitors; current
  `nmi_prev`, `nmi_pending` and `vblank_suppress` are already serialized.
- `tests/CMakeLists.txt` registers the shared CI contract, protected campaign,
  state format pin, host parity and performance checks.
- Hardware comments cite documents; implementation and test code remain owned.

### Integration Points
- `src/ppu.c`: VBlank set/clear, register reads/writes and reset.
- `src/bus.c`: two M2 catch-ups, read/write side effects, DMA, cycle counting.
- `src/cpu.c`, `src/internal.h`: edge/poll history and instruction entry.
- `src/frame.c`: instruction-completing frame loop; no mid-call save/yield API.
- `src/state.c`, `src/state.h`: field lists, validation and format boundaries.
- `tests/cmake/accuracycoin_activation.cmake`: revision and campaign provenance.
- `tests/cmake/state_format_pin.cmake`, `tests/runner/hashes.txt`, README and
  `include/nesturbator.h`: synchronized version/hash/documentation surfaces.

</code_context>

<specifics>
## Specific Ideas

"Another copy and paste is better than another dep": reuse small owned
helpers, accept justified local duplication, and add no dependency for these
bounded deterministic tests. Relevant perspectives were hardware/CPU/PPU,
test independence, host/API stewardship, validation/security, performance,
determinism, CI/release attribution and adversarial failure detection. UI,
3D graphics and unrelated language ecosystems add no independent requirement
to this C core phase.

</specifics>

<deferred>
## Deferred Ideas

- Phase 21: I-flag IRQ latency and shared interrupt entry/vector hijack.
- DMA/OAM/open-bus clusters, PAL, additional hardware profiles and any new
  public arbitrary-cycle stepping/debug API remain outside Phase 20.
- Fetch-only nightly blargg expansion remains future scope; no unlicensed
  blargg ROM is committed.

### Reviewed Todos (not folded)
- `.planning/todos/pending/2026-10-26-add-reproducible-owned-property-tests.md`
  — generic seeded helper/shrinking has not demonstrated additional value
  for this phase; reuse existing deterministic state tests.
- `.planning/todos/pending/2026-10-26-make-runner-signal-checks-deterministic.md`
  — unrelated process/signal contract; preserve separate proposed scope.
- `.planning/todos/pending/2026-10-26-evaluate-scoped-nightly-mutation-testing.md`
  — tool/nightly pilot is separate; scoped owned negative controls suffice here.

</deferred>

---

*Phase: 20-nmi-line-and-vblank-flag*
*Context gathered: 2026-10-26*
