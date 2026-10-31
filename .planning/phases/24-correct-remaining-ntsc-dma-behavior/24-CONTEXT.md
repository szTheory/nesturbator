# Phase 24: Correct remaining NTSC DMA behavior - Context

**Gathered:** 2026-10-31
**Status:** Ready for planning

<domain>
## Phase Boundary

Correct the five selected NTSC DMA failures at AccuracyCoin pin `6bee919529289c555803b612b8c9e4f7eaaeeabb` for the declared RP2A03G target. Preserve the three passing DMA rows, every other protected PASS key, deterministic output, performance limits, and native state continuation where affected. The five rows are `DMA Landing on Write`, `DMC DMA + OAM DMA`, `DMC DMA Bus Conflicts`, `Explicit DMA Abort`, and `Implicit DMA Abort`.

Keep PAL/Dendy, unrelated accuracy failures, broad game playtesting, pin migration, collector/CI cleanup, new dependencies, a scheduler rewrite, public exact-cycle APIs, and unlisted ROM bytes out of scope. The interrupted full-CI/AccuracyCoin campaign result remains unknown and its stop remains active.

</domain>

<decisions>
## Implementation Decisions

### Causal evidence and regression design

- **D-01:** Localize each selected row independently and record its first causal divergence before grouping fixes. Share a fix only when traces establish a common cause; similar aggregate failure codes do not establish one. Phase 21's DMC retry/open-bus repair is a prerequisite, not completion of these remaining failures.
- **D-02:** Add small owned C fixtures at the existing bus/core seam for each distinct corrected behavior. Assert the relevant access kind/address, GET/PUT phase, halt and retry, actual sample read, side effects, and continuation; include a named wrong-model control. Reuse existing test and CMake patterns. A shared test helper is justified only if the traces demonstrate a shared need.
- **D-03:** Keep hardware references, pinned AccuracyCoin expectations, and observed core behavior as separate evidence. AccuracyCoin is the compatibility gate, not proof of silicon correctness. The pinned README identifies RP2A03G as its design target and accepts both pre-mid-1990 and mid-1990-or-later outcomes for `Implicit DMA Abort`; document any narrower revision assumption without claiming one universal RP2A03G behavior. See the [pinned AccuracyCoin README](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/README.md).
- **D-04:** Keep tests and emulation code owned, integer-only, instance-local, and dependency-free. Prefer an existing seam, a small helper, or justified local duplication. Do not add a public exact-cycle API, general scheduler, or test framework.

### State compatibility, documentation, and verification

- **D-05:** Preserve the existing state format when the serialized layout and meaning stay unchanged. Change the format version if a serialized field is added, removed, resized, reordered, or changes meaning; change the behavior revision when emulated output changes. The loader has no old-format converter, and the behavior revision is recorded but not enforced by the loader.
- **D-06:** If a correction changes in-flight DMA state, save and load at production-reachable affected boundaries and compare resumed behavior and frame/audio output with uninterrupted execution. Decide version changes from the actual fields and semantics touched; do not add a converter or make a compatibility promise beyond the existing contract.
- **D-07:** Every behavior change lands with its owned test and README/public-header comments. Preserve the pin, protected PASS keys, deterministic hashes, and existing performance limits.
- **D-08:** Do not run full CI or an AccuracyCoin campaign as discovery, automatic verification, review repair, or a push/PR side effect while the stop is active. Planning may inspect test selection and configuration without executing the workflow. If required project CI would schedule `accuracycoin.campaign`, stop before that command and record the exact command, selected tests, missing evidence, and separate authorization needed. No campaign or tests were run during this discussion.

### The agent's Discretion

The researcher and planner choose the trace order, minimal fixture/helper organization, exact test file placement, and implementation task grouping. They must not presume shared causes or make a wider hardware claim than the evidence supports.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Scope, requirements, and evidence boundary

- `.planning/ROADMAP.md` — Phase 24 goal, success criteria, protected passes, evidence boundaries, and verification authorization gate.
- `.planning/REQUIREMENTS.md` — DMA-01 through DMA-04, target, regression, and compatibility requirements.
- `.planning/STATE.md` — current session state and the active stop on full-CI/campaign reruns.
- `.planning/research/milestone-dma-discovery/SCOPE.md` — row-specific failure behavior and evidence needs, implementation boundaries, and source limits.
- `.planning/preparation/README.md` — index for the project's hardware, conformance, architecture, and engineering references.
- `.planning/preparation/CONFORMANCE.md` — test-suite contracts and limits on interpreting their results.
- `.planning/preparation/NES-HARDWARE-CPU-APU.md` §2.2 — DMA timing model, difficult cases, and cited hardware/test sources.
- `.planning/preparation/ARCHITECTURE.md` §§2, 7 — CPU-driven bus timing and the public state/API model.
- `.planning/preparation/DECISIONS.md` — DEC.03, DEC.05, DEC.09, DEC.16, and DEC.17 cover ownership, target chip, bus seam, and versioned state contracts.
- `.planning/milestones/v5-phases/21-interrupt-latency-and-hijack/21-DMC-PHASE-AUDIT.md` — prior DMC retry/open-bus repair and its stated limits; this is a prerequisite, not evidence that the Phase 24 failure cluster is fixed.

### Existing implementation and tests

- `src/bus.c` — per-cycle bus path, DMC halt/retry and sample access, and OAM transfer.
- `src/apu.c` — DMC request scheduling, reload, and stop behavior.
- `src/state.c` and `include/nesturbator.h` — serialized DMA fields and the state-format/behavior-revision contract.
- `tests/accuracy/scoreboard.txt` — current pinned row status and starting failure codes.
- `tests/core/test_apu.c` and `tests/cpu/test_bus.c` — existing owned DMC and production-bus fixture patterns.
- `tests/core/test_state_fork.c` — native continuation and frame/audio comparison pattern.
- `tests/roms/manifest.txt` — permitted committed ROM inventory; inspect before selecting any ROM fixture.
- `README.md` — public behavior documentation destination.
- `AGENTS.md` — clean-room, dependency, testing/documentation, branch, and one-step workflow rules.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets

- `src/bus.c`: `bus_read_cycle` owns the production read seam; DMC halt, dummy/alignment cycles, sample get, and resumed read occur there. `oam_dma` performs source reads through the same bus decoder. `nesturbator__bus_write` records a failed halt and retries on a later cycle.
- `src/apu.c`: the DMC unit already tracks pending requests, halt phase, delayed load, remaining bytes, and sample-buffer state.
- `tests/core/test_apu.c`: existing cases cover GET/PUT starts, enable delay, and DMC retries across CPU writes.
- `tests/cpu/test_bus.c`: existing production-bus access tests can host focused bus assertions.
- `src/state.c`: one field visitor serializes, validates, and applies BUS/APU state. Existing saved fields include OAM request/page, DMC pending/halt/load-wait state, and APU GET/PUT phase.
- `tests/core/test_state_fork.c`: existing native fork route compares uninterrupted and resumed machine/output continuation.

### Established Patterns

- CPU bus calls advance one cycle; DMA is implemented inside bus reads and preserves repeated read side effects.
- Hardware claims cite their sources. Tests distinguish a test's expected contract from the independent hardware explanation and observed core trace.
- State format and behavior revision are separate. The state loader validates before applying and accepts only its current exact format.
- New regression coverage belongs in the existing CMake/CTest entrypoints and keeps wrong-model controls independently meaningful.

### Integration Points

- DMA behavior connects `src/bus.c`, `src/apu.c`, serialized BUS/APU fields, and the current conformance scoreboard.
- Tests and behavior documentation ship with each correction. Required CI remains the existing workflow, subject to the active campaign authorization gate above.

</code_context>

<specifics>
## Specific Ideas

The owner asks for broad role-lens review adapted to the domain: hardware and architecture, test/oracle quality, API and save compatibility, maintenance, clean-room/security, CI, performance, and product scope. Use reputable primary sources where useful, then stop fan-out when recommendations converge. Avoid unrelated UI, graphics-engine, distributed-systems, or language-framework exploration. Prefer a small owned helper or justified copy/paste over a new dependency.

</specifics>

<deferred>
## Deferred Ideas

No new ideas were raised.

### Reviewed Todos (not folded)

- `2026-10-26-add-reproducible-owned-property-tests.md` — no confident Phase 24 requirement match; keep unlinked.
- `2026-10-26-evaluate-scoped-nightly-mutation-testing.md` — no confident Phase 24 requirement match; keep unlinked.
- `2026-10-26-make-runner-signal-checks-deterministic.md` — no confident Phase 24 requirement match; keep unlinked.

</deferred>

---

*Phase: 24-correct-remaining-ntsc-dma-behavior*
*Context gathered: 2026-10-31*
