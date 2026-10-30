# Phase 21: Interrupt latency and hijack - Context

**Gathered:** 2026-10-28
**Status:** Ready for research and planning

<domain>
## Phase Boundary

Deliver INT-01, INT-02 and KEEP-01: IRQ acceptance after CLI, SEI, PLP, RTI
and branches matches the console; NMI takes over BRK/IRQ vectors at the
documented arrival boundaries; all eleven v5 AccuracyCoin rows pass at
`6bee919`. Preserve every existing PASS and deterministic state/host replay.

The three remaining target rows are `Interrupt flag latency`, `NMI Overlap
BRK` and `NMI Overlap IRQ`. Phase 20 supplies the NMI line, capture and poll
model. Its PR #59 is now merged as `a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`;
this discussion starts from that merged tree on the Phase 21 branch.

This command captures decisions only. No implementation, phase plan, new
hardware experiment, campaign or product test was run during discussion.

</domain>

<decisions>
## Implementation Decisions

The owner selected all relevant areas and delegated their resolution: fan out
across relevant roles, examine primary sources, alternatives and adversarial
cases, then adopt a coherent recommendation without routine questions.
Three parallel reviews covered polling, vector takeover and state/acceptance;
a bounded cross-review challenged the combined state/entry recommendation.
These are adopted planning constraints, not measured implementation results.

### IRQ acceptance and instruction polls

- **D-01:** Decide IRQ eligibility at the documented instruction poll using
  the I flag applicable to that poll. Service accepted history at the next
  dispatch; do not gate it again with live architectural I. Keep the live
  IRQ level, captured level and accepted decision conceptually distinct;
  exact minimal representation is researcher/planner discretion.
- **D-02:** CLI, SEI and PLP use the pre-change I value at their poll; RTI
  uses its restored I. Test both PLP transitions, both RTI outcomes, pending
  IRQ across SEI, CLI/SEI combinations and immediate RTI retrigger when an
  unacknowledged IRQ remains asserted. Stacked P reflects architectural
  flags, which can differ from the I used to accept the request.
- **D-03:** Apply branch exceptions to IRQ as well as NMI: retain the first
  legal poll for not-taken and taken same-page branches; on page crossing,
  preserve an earlier accepted request as well as any acceptance at the
  second legal poll. No invented poll in the same-page extra cycle. Cover
  all eight branch opcodes and forward/backward crossings. Derive the exact
  numbered-cycle table from the pinned sources before writing expectations.
- **D-04:** Preserve Phase 20's integer timeline, split PPUSTATUS sampling,
  ordinary-loop NMI capture/retention and single global equal-timestamp
  policy. Keep `nesturbator__irq_update` as the sole production IRQ-line
  writer combining APU/DMC/mapper sources. Reopen alignment or sampling only
  for a minimal demonstrated defect; no per-row/title phase, second accuracy
  mode, generic scheduler or randomized alignment.

### Shared entry and vector takeover

- **D-05:** Use one small owned BRK/IRQ/NMI entry mechanism. Keep entry
  **origin** separate from selected **vector**: BRK retains its skipped byte,
  opcode+2 return PC and stacked B=1 under hijack; hardware IRQ/NMI retain
  their interrupted PC and B=0. Preserve U/I semantics, stack wrapping,
  dummy accesses and the seven-cycle sequence. Do not conflate vector
  takeover with changing the entry's original stack semantics.
- **D-06:** Choose a coherent NMI or IRQ/BRK vector pair at a source-qualified
  takeover boundary; do not fix the vector at entry or independently change
  its high byte after the low byte was fetched. Research must explicitly map
  circuit T-states and phi phases, input recognition, external bus actions
  and this core's numbered cycles. Preparation's "after cycle 4" and older
  research's "at vector-low fetch" are not equivalent implementation rules.
- **D-07:** Model entry-phase recognition and consumption from its own
  evidence. Phase 20's six-case ordinary-loop circuit oracle explicitly does
  not establish entry/hijack windows. The primary hijack analysis describes
  recognition inhibition that prevents a mixed vector, a sufficiently late
  sustained NMI following a handler instruction, and a short pulse that can
  be missed. Distinguish raw line, capture, recognition and service; do not
  erase retained history on line release, promise every raw entry pulse is
  retained, or indiscriminately disable every detector stage during entry.
- **D-08:** Preserve NMI priority and the IRQ source's acknowledgment rules.
  Service consumption happens once at its documented event. A held NMI
  level is not a second edge; a new eligible edge must not be erased by
  completion of an older entry. Entry bus calls must not accidentally turn
  their overwritten poll temporaries into a new ordinary instruction poll.
  Do not introduce an unbounded interrupt queue or extend into reset-pin
  hijacking. Soft reset must coherently terminate any partial entry.

### Source-derived tests and adversarial evidence

- **D-09:** Write owned reproducers before fixes, using three complementary
  layers: CPU test-bus access/stack assertions, production-bus guest programs
  with observable markers, and complete pinned AccuracyCoin tables/campaign.
  `tests/cpu/vector_bus.*` currently logs accesses without production sampling;
  use it for sequence evidence, never as sole proof of detector timing.
  Reuse owned fixtures and CTest; add no framework or dependency.
- **D-10:** Expected tables must state source/pin, initial machine, numbered
  cycles/phase convention, scheduled event and expected PC/P/stack/vector/
  handler order. Derive them from hardware analysis and test assembly, never
  from this core's output. AccuracyCoin is primary evidence of test intent;
  distinguish that from circuit observations and unmeasured physical ties.
  Keep already-correct cases as passing guards; only defect reproducers are
  expected to fail initially. Final CI has ordinary passing assertions.
- **D-11:** Cover IRQ assertion and acknowledgment before/on/after legal
  polls, including acknowledgment between two branch polls. Exercise a real
  IRQ source through production bus sampling, with existing mapper checks
  retained. For BRK and IRQ sweep NMI arrival at every distinct entry cycle,
  and include ordinary NMI entry, selected-vector boundaries, late sustained
  and short-pulse cases, simultaneous priority, new-edge versus held-level
  cases, PC wrap and stack wrap. Observe exact accesses and guest handler
  order, not only the final vector or aggregate campaign score.
- **D-12:** Pin whole-table alignment alternatives exactly as the test
  assembly allows. The overlap tests pair NMI and BRK/IRQ answer tables;
  an allowed shift must apply coherently to both tables, never per byte or
  independently per handler. Diagnose first divergence if latency subtests
  encounter DMA/open-bus prerequisites; a nearby failure is not permission
  to expand into a new DMA cluster or weaken the expected table.
- **D-13:** Use a bounded set of discriminating wrong-model controls: live-I
  recheck, delayed RTI, extra/missing branch poll, lost earlier acceptance,
  wrong priority/B/return PC, early/late takeover, mixed vector, duplicate or
  lost request, and omitted entry-history restore. Each selected control
  must fail its intended assertion or verdict path. Choose controls for
  distinct failures; do not build generic mutation infrastructure.

### Genuine mid-entry save/load with the existing public frame contract

- **D-14:** Introduce **narrowly resumable interrupt entry only**. This is a
  Phase 21 exception to Phase 20's exclusion of mid-instruction state, required
  by Phase 21's stronger mid-entry continuation criterion. Ordinary opcodes
  remain instruction-based; normal `cpu_step` and frame calls still drain
  entry synchronously. No public stepping API, runtime callback, asynchronous
  snapshot or frame-deadline suspension is added; input sampling and crossing
  instruction completion retain their documented meanings.
- **D-15:** The same production entry-stage mechanism must serve normal
  execution and owned internal fixtures. Persist all history necessary to
  resume: origin, next unperformed action, partial vector data/selection and
  consumption history where not derivable. Exact fields are not prescribed.
  Resume an active entry before fetching an opcode, reevaluating I/priority
  or starting another entry. BRK's opcode fetch already counts as cycle 1;
  IRQ/NMI have their two discarded reads. Never repeat/omit a push, dummy
  access, vector read, clock advance or service event after load.
- **D-16:** Prove normal BRK/IRQ/NMI initiation with complete `cpu_step`
  tests. Fixtures may construct and explicitly label the entry origin, then
  execute the production stages to genuine completed-bus-action cutpoints.
  Save with the production visitor, load with the native loader, and resume
  through ordinary `cpu_step`/frame execution. Do not serialize an incomplete
  C stack or fabricate partial pushes and call that executed continuation.
  Sampling halfway through one bus call remains unsupported.
- **D-17:** Exercise every distinct resumable entry stage for all three
  origins, particularly after partial pushes, on both sides of vector
  selection, between vector bytes and around consumption. Compare against
  uninterrupted entry from the same origin: exact machine bytes, accesses,
  ticks and guest markers, then frame/audio continuation. Extend
  `core.state_fork.*`; retain multi-board coverage while using a small NROM
  fixture for the detailed entry sweep. Include nonzero synthesis history
  and interleaved instances with different entry/latch histories.
- **D-18:** Load those compatible native cutpoint states into an actual
  libretro adapter instance via its real unserialize path; extend
  `libretro.runahead` across existing depths with committed/speculative state,
  ticks, video, PCM and guest-marker comparisons. Preserve ordinary
  boundary-origin rewind/run-ahead too. Include stale/omitted-load and
  omitted-history controls. Label the proof **internally reached cutpoints
  loaded through public APIs**: ordinary public frame calls do not expose
  exact mid-entry stops. This satisfies saved-state replay, not a public
  cycle-stepping capability. A boundary-only substitute is insufficient.
- **D-19:** Move state format **2 to 3** and behavior revision **8 to 9** in
  the causal implementation; leave ABI unchanged. New entry history and
  changed poll semantics need explicit compatibility, even if an existing
  byte could be reused. Use the existing SIZE/WRITE/VALIDATE/APPLY/RECORD
  visitor and validate bounds and source-established reachable combinations
  before any mutation. Preserve legitimate historical latches differing
  from live inputs, revisiting `nmi_polled => nmi_pending` if consumption
  changes its reachable meaning. Reject formats 1/2 with existing version
  error precedence and unchanged instance/save generation. Extend CRC-correct
  rejection and save/load/save tests; refresh pins and owned corpus states,
  README and public-header explanations together.

### Regression, attribution and delivery

- **D-20:** The starting campaign is **114 PASS / 32 FAIL**, all 146 scored
  rows terminal, plus five DRAW entries outside the denominator. Protect all
  114 starting PASS keys, including Phase 19/20 gains, and freshly authenticate
  protected-main authority in the same CI invocation. Phase 20's old hosted
  comparison protected 108 keys; that historical count is not Phase 21's
  protection ceiling. No three-target-only or aggregate-score substitute.
- **D-21:** All eleven v5 rows must pass: the three Phase 21 targets; `$2002
  flag timing`, `NMI Control`, `NMI Timing`, `NMI Suppression`, `NMI at VBlank
  end`, `NMI disabled at VBlank`; `Sprite 0 Hit behavior`, `Arbitrary Sprite
  zero`. Three additional passes alone would yield 117/29, an arithmetic
  expectation rather than a hard-coded verdict. Record incidental changes
  and investigate every lost PASS.
- **D-22:** Retain pin `6bee919`, the 1800-frame cap, schemas and current
  CI placement. Focus tests during iteration; run full campaign comparisons
  at causal milestones and final `cmake --workflow --preset ci`. Preserve
  six-platform hash equality, CPU vectors, nightly full vectors/MMC3, Holy
  Mapperel, boot-to-play, sanitizer and host checks. Keep the existing +1%
  performance gate and measured-timeout policy. No new full campaign per
  lane, speculative cap increase or unrelated CI redesign.
- **D-23:** Keep polling and shared-entry behavior attributable in ordered
  green changes, with their tests, README/header comments, measured campaign
  diff and observed moved hashes in the same causal change. Re-pin only
  actual changed frame/audio/state values with reasons; state-byte movement
  is not proof of a timing improvement, and audio changes need a CPU/APU
  explanation. Keep format and behavior version coherent throughout the PR.
- **D-24:** New hosted acceptance must bind candidate, tested merge, protected
  base, effective inputs/modes and run/attempt/artifact identities. Phase 20's
  418-input accepted bundle is prerequisite history, not proof of changed
  Phase 21 source. Reuse existing evidence machinery with documented limits
  and run-specific mitigations. Preserve collector CR-01/WR-03/WR-04 debt;
  do not silently expand into collector maintenance or claim an unmeasured
  paired-chip physical-tie winner.
- **D-25:** Keep all runtime state instance-owned, integer-only and
  allocation-free during execution; the core links only C memory functions.
  All emulation/test code remains owned; `libretro.h` remains the only vendor
  file. No new runtime/test dependency, ROM asset, device code, copied
  emulator implementation or global mutable latch. Read documentation and
  permitted tests; reference emulators only as released binaries.
- **D-26:** Work on `gsd/phase-21-interrupt-latency-and-hijack`, based on
  merged Phase 20. Keep the owner's existing config edit out of commits.
  One GSD step per command remains in force: stop after context/log/session
  capture, with planning and implementation as separate invocations. No
  auto-chain setting, PR, push or merge is part of this discussion step.

### Agent discretion and evidence still required

- Helper/field names, minimal continuation encoding, fixture organization and
  bounded control selection remain implementation choices. Prefer a small
  owned helper or justified local duplication over another dependency.
- Research must derive the exact poll/recognition/cutoff crosswalk and test
  expected tables, prove stage sufficiency and host fixture compatibility,
  and reconcile stale pre-Phase-20 research. Do not turn an unresolved analog
  tie into a prerequisite to independent work. Contradictory finite-margin
  evidence or failed target/protected rows still blocks acceptance.
- No product-behavior or performance improvement is established by this
  discussion. The exact production cutpoint fixtures still need construction
  and verification; no existing state format currently resumes partial entry.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents must read the relevant sections before planning or implementing.**

### Scope and settled contracts
- `.planning/ROADMAP.md` — Phase 21 goal and all four success criteria.
- `.planning/REQUIREMENTS.md` — INT-01, INT-02, KEEP-01 and v5 exclusions.
- `.planning/PROJECT.md` — current product constraints and Phase 20 outcome.
- `.planning/preparation/README.md` — reference index and source tiers.
- `.planning/preparation/DECISIONS.md` — owned code, clean room, timing and API.
- `.planning/preparation/GSD-HANDOFF.md` — one-step operation and automated checks.

### Hardware and architecture
- `.planning/preparation/NES-HARDWARE-CPU-APU.md` §2.3 and sources HWC.02/HWC.17 — polling and hijack source map; reconcile cycle terminology.
- `.planning/preparation/ARCHITECTURE.md` §§2, 3, 7 — existing timeline, instruction boundaries and state/API contracts; D-14 is the limited Phase 21 extension.
- `.planning/research/ARCHITECTURE.md` — shared-entry proposal; current source descriptions, version numbers and alignment recommendations predate Phase 20.
- `.planning/research/FEATURES.md` G3/G4 — remaining interrupt behavior.
- `.planning/research/PITFALLS.md` #4, #5, #6, #11 — pitfalls; exact vector-cutoff prose needs source reconciliation.
- `.planning/research/SUMMARY.md` steps 5, 6, 8 — milestone integration intent, not current measurements.
- `.planning/phases/20-nmi-line-and-vblank-flag/20-CONTEXT.md` — carried decisions and amended finite-margin/tie distinction.
- `.planning/phases/20-nmi-line-and-vblank-flag/20-NMI-ORACLE-RESEARCH.md` — ordinary-loop circuit experiment and explicit entry-window exclusion.

### Testing, state and evidence
- `.planning/preparation/CONFORMANCE.md` — pinned suites, licenses and scoreboard rules.
- `.planning/preparation/ENGINEERING.md` — C/build/CI, hygiene and performance policy.
- `.planning/phases/20-nmi-line-and-vblank-flag/20-VERIFICATION.md` — accepted Phase 20 truths and evidence limits.
- `.planning/phases/20-nmi-line-and-vblank-flag/20-REVERIFICATION-EVIDENCE.md` — current 418-input acceptance, historical source/run identities and 114/32 baseline.
- `.planning/phases/20-nmi-line-and-vblank-flag/20-REVIEW-DISPOSITION.md` — retained collector debt and mitigations.
- `tests/accuracy/scoreboard.txt`, `tests/accuracy/baseline-metadata.txt`, `tests/accuracy/measurements.md` — actual starting outcomes and causal hash evidence.
- `include/nesturbator.h` — format/revision policy, synchronous frame/input and native-state guarantees.

### External primary material verified during discussion
- [AccuracyCoin assembly at 6bee919529289c555803b612b8c9e4f7eaaeeabb](https://raw.githubusercontent.com/100thCoin/AccuracyCoin/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm) — `TEST_IFlagLatency`, page-boundary and NMI overlap tests; test intent and complete-table alternatives.
- [Visual6502 interrupt hijacking, revision 19268](https://www.nesdev.org/w/index.php?title=Visual6502wiki/6502_Interrupt_Hijacking&oldid=19268) — primary circuit-analysis text, vector coherence and entry recognition inhibition. The maintained page was also available through web search; direct fetch access varied.
- [Visual6502 interrupt recognition stages and tolerances](https://www.nesdev.org/wiki/Visual6502wiki/6502_Interrupt_Recognition_Stages_and_Tolerances) — follow-up source for research, already indexed as HWC.17/revision 19267; no new circuit experiment was performed here.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `tests/check.h`, `tests/ines.h`, `tests/nmi_fixture.h` — assertions and owned synthetic guests.
- `tests/cpu/vector_bus.c`, `tests/cpu/vector_bus.h`, `tests/cpu/test_cpu_unit.c` — cycle/address logs and CPU-only fixtures; sampling is not production-equivalent.
- `tests/core/test_nmi_poll.c` — guest markers, stack-return checks and fixed branch expectation tables; prefer those independent tables over observed-value branching.
- `tests/core/test_nmi_state.c`, `tests/core/test_state_fork.c`, `tests/state_test.h` — production-state replay, exact-cutpoint labeling and omitted-history controls.
- `tests/core/test_state_reject.c`, `tests/core/test_state_runahead.c`, `tests/runahead_model.h`, `tests/libretro/libretro_host.c` — rejection, speculative replay and actual adapter evidence.

### Established Patterns
- One source-cited hardware model, bus call per CPU cycle, fixed-width integer state and no mutable globals.
- One state visitor validates before applying; canonical bytes and behavior, not C padding, establish replay.
- Protected-main authority and complete row keys prevent a candidate from lowering its own regression bar.
- README and public-header comments accompany every behavior change and its owned CI test.

### Integration Points
- `src/bus.c` — close-edge capture and poll copies; preserve clock/device ordering and production decode.
- `src/cpu.c` — live-I IRQ acceptance, NMI-only branch history, three fixed-vector entry paths and reset.
- `src/internal.h` — instance-owned CPU history; currently no partial-entry continuation.
- `src/state.c`, `src/state.h` — CPU visitor, bounds/reachability and native format handling.
- `src/frame.c` — complete `cpu_step` calls and public frame/input semantics to preserve.
- `src/apu.c` — single IRQ-line update combining source state.
- `libretro/libretro.c` — real native serialize/unserialize path; no second state format.
- `tests/CMakeLists.txt`, `tests/cmake/nmi_targets.cmake`, `tests/cmake/state_format_pin.cmake`, `tests/runner/hashes.txt` — existing gates and version/hash pins to extend coherently.

</code_context>

<specifics>
## Specific Ideas

- The owner's delegated method applies hardware, C architecture, testing,
  determinism, API/host compatibility, security, CI and performance lenses.
  UI, distributed systems and unrelated language ecosystems add no useful
  scope to this phase.
- "Another copy and paste is better than another dep": prefer bounded local
  code over introducing a framework; the shared entry is justified by shared
  hardware behavior and replay, not deduplication alone.
- Retain provenance labels: inspected source, source-derived expectation,
  constructed origin, executed cutpoint and actual hosted result describe
  different evidence. Do not collapse them into a claim of console measurement.

</specifics>

<deferred>
## Deferred Ideas

- DMA/OAM/open-bus clusters, PAL, reset-pin hijacking, chip profiles, public
  exact-cycle/debug APIs and whole-CPU sequencing.
- New fetch-only `cpu_interrupts_v2`/`ppu_vbl_nmi` nightly scope; existing
  licensed/pinned test policy and MMC3 lane remain intact.
- Collector CR-01/WR-03/WR-04 and earlier sprite/JAM-cache advisories remain
  recorded debt; repair only through separately scoped work.

### Reviewed Todos (not folded)
- `.planning/todos/pending/2026-10-26-add-reproducible-owned-property-tests.md`
  — general helper/shrinking proposal stays deferred; reuse deterministic
  state equality and existing rejection helpers within this phase.
- `.planning/todos/pending/2026-10-26-evaluate-scoped-nightly-mutation-testing.md`
  — a new mutation tool/lane is outside interrupt scope; bounded owned
  wrong-model controls are sufficient here.
- `.planning/todos/pending/2026-10-26-make-runner-signal-checks-deterministic.md`
  — process/signal redesign is unrelated; preserve its current tests.

The matcher gave all three 0.6 on generic test keywords. Scope review takes
precedence over keyword overlap; none is promoted or marked completed.

</deferred>

---

*Phase: 21-interrupt-latency-and-hijack*
*Context gathered: 2026-10-28*
