# Phase 26 source coverage and planning assumptions

Planning only. No implementation builds/tests, branch creation/switch, hosted action or campaign restart was run. Existing unrelated working-tree edits are outside these plans.

## Discovery and dependency result

Level 0: the current phase research and direct reads confirm established private CPU/bus arbitration, generated observer/control, canonical field visitor, state fork/refusal and real-module host patterns. No library choice, new dependency, public API or second serializer is needed. The current Phase 26 research supplies source context; Phase 25 summary frontmatter supplies prior timing/sample decisions without broad historical enrichment. Project skills directories yielded no local skill index; configured planner skills are empty. Graph context is disabled. The assumption-delta scan supplied by the orchestrator detected no signals. Security/threat-model, AI/UI specifications, MVP framing and Nyquist capabilities are disabled; tracer-first and explicit task verification remain active.

| Plan/task | Needs | Produces | Dependency / checkpoint |
|---|---|---|---|
| 26-01.1 | Latched effective region; real CPU/APU/bus observer seam | One working pending-read → SYNC halt → GET → resume path and wrong control; private call-only read kind | Wave 1; none |
| 26-01.2 | Revision-17 header and old NTSC cut bytes from task 1 | Public model contract, version-policy update and explained metadata-only state pins | Same behavior change as task 1; none |
| 26-01.3 | Proven tracer and public contract | Read-kind/phase expansion and bounded DMA evidence | Sequential expansion; none |
| 26-02.1 | 26-01 PAL DMC behavior and existing timing fields | Public PAL save/load/output continuation and regional identity/visitor bounds | Wave 2; already-settled persistent contract |
| 26-02.2 | Regional writer/loader | Independent bound/refusal/old-state tests and API state contract | Same regional behavior change; none |
| 26-02.3 | Complete DMA/state slices and fixtures | PAL real-adapter size guard, source review and final local-CI receipt | Final candidate; standing authority satisfies local CI gate |

Wave 2 depends on 26-01 because state cuts must exercise the new PAL halt model and both plans modify the public header and README. Plan 26-01 carries the DMA README/header contract; Plan 26-02 carries the API guide's regional state/version contract with state behavior. No same-wave file overlap exists. Each plan lists nine modification paths; all tasks modify at most five tracked/new files. Each plan has three tasks; calibration factor 1, sample count 0 and confidence low produced estimates of 32000 and 36000 tokens respectively. Tasks 1–2 in each plan land together so behavior, regression and public documentation form one reviewable change, following the recorded Phase 24 precedent. No owner confirmation is needed for the already locked compatibility policy.

Detailed DMA evidence belongs in the required `26-01-SUMMARY.md`: exact guest/request/halt/GET/resume facts, same-timeline wrong control, observed/linked equality, source/claim crosswalk, revision-derived old/new NTSC state-byte comparisons and old-state loading. Detailed final acceptance belongs in the required `26-02-SUMMARY.md`: exact tested source/tree and dirty scope, focused and final unfiltered CI/package results, source-review fixes and scoped retests, timings/provenance/resource limits, unresolved flags and scope boundaries. These required summaries carry the evidence without separate process-artifact files or loss of coverage.

## Multi-source coverage audit

| Source | Item | Required outcome / constraint | Plan/task | Status |
|---|---|---|---|---|
| GOAL | Phase 26 goal | PAL model plus matching-region-only state continuation | 26-01 / 26-02 | COVERED |
| GOAL | Success 1 | Owned fixture and distinguishable wrong model | 26-01.1, 26-01.3 | COVERED |
| GOAL | Success 2 | Model contract, later no-op and direct-hardware limits | 26-01.1–3 | COVERED |
| GOAL | Success 3 | Same-region pixels, PCM/count, ticks and canonical bytes including real DMC/remainders | 26-02.1–2 | COVERED |
| GOAL | Success 4 | Cross-region identity refusal before any apply; entire machine and future output unchanged | 26-02.1–2 | COVERED |
| GOAL | Success 5 | Actual format/revision/ABI and compatibility consequences | 26-01.2, 26-02.1–3 | COVERED |
| REQ | DMA-01 | Source-backed modeled halt and sensitive control with unresolved limits documented | 26-01; final combined gate 26-02.3 | COVERED |
| REQ | STATE-01 | Regional identity/continuation, atomic refusal and version contract | 26-02 | COVERED |
| RESEARCH | Responsibility map | Shared C core owns halt/identity/bounds; tests use production paths | 26-01.1, 26-02.1 | COVERED |
| RESEARCH | Small private read-kind seam | Mark opcode dispatch, leave operands/data/dummy generic; consistent IRQ/NMI C1 interpretation | 26-01.1, 26-01.3 | COVERED |
| RESEARCH | Shared eligibility only | Retain NTSC path, existing phase/enable checks, OAM overlap and unrelated abort/reload behavior | 26-01.1, 26-01.3 | COVERED |
| RESEARCH | Generated same-timeline control | Same PAL schedule, actual timer history, compile-time control isolated from production | 26-01.1, 26-01.3 | COVERED |
| RESEARCH | Narrow trace | Request kind, opcode acceptance and needed GET/resume/continuation facts only | 26-01.1, 26-01.3 | COVERED |
| RESEARCH | Existing serializer | Existing byte 23 and visitor order/width/meaning; no duplicate PAL format | 26-02.1 | COVERED |
| RESEARCH | Regional bounds | Separate denominators, prerender lines and strict frame-gap inequalities; unchanged other bounds | 26-02.1–2 | COVERED |
| RESEARCH | Validate before apply | Identity before body and application, snapshot all refusal state | 26-02.1–2 | COVERED |
| RESEARCH | Reachable public DMA cut | Guest-generated pending work at a public frame return; active bus-local transfer cannot be saved publicly mid-call | 26-02.1 | COVERED |
| RESEARCH | Canonical continuation | Pixels, PCM/count, counters, states and sensitive lost-history control | 26-02.1 | COVERED |
| RESEARCH | Same image / opposite profiles | Public preference-eligible image isolates region mismatch from cartridge mismatch | 26-02.2 | COVERED |
| RESEARCH | Version nuance | New revision changes state metadata/CRC hashes; old NTSC format-6 states remain supported | 26-01.1–2, 26-02.2 | COVERED |
| RESEARCH | Existing focused test registrations | Existing targets plus focused PAL fork/refusal entries with real commands and existing caps | 26-01.1–3, 26-02.1–3 | COVERED |
| RESEARCH | Evidence limits / open questions | Both questions explicitly RESOLVED FOR PLANNING / SCOPE ONLY under D-03/D-04; later no-op effects and representative direct hardware proof remain unknown/unverified and unasserted | 26-01.2–3, 26-02.3 | COVERED |
| RESEARCH | No external package | Owned C17, existing CMake/CTest and helpers; no package install | Both plans | COVERED |
| RESEARCH | Required CI / public docs | README/header/API accompany behavior, final exact-candidate unfiltered workflow within standing limits | Both plans; final 26-02.3 | COVERED |
| CONTEXT | D-01 | Opcode/SYNC halt model; NTSC any-read behavior retained | 26-01.1, 26-01.3 | COVERED |
| CONTEXT | D-02 | Generated production-path fixture and identical-PAL-timeline control | 26-01.1, 26-01.3 | COVERED |
| CONTEXT | D-03 | Narrow oracle and explicitly unasserted later no-op/unrelated PAL behavior | 26-01.1–3, 26-02.3 | COVERED |
| CONTEXT | D-04 | Trace observation vs SYNC interpretation and unresolved direct proof | 26-01.1–3 | COVERED |
| CONTEXT | D-05 | Existing 64-byte header; byte 23 values 0/1 | 26-02.1–2 | COVERED |
| CONTEXT | D-06 | Format 6; exact regional bounds and old NTSC compatibility | 26-02.1–2 | COVERED |
| CONTEXT | D-07 | Behavior 17, ABI 1, informational revision, format conflict surfaced before mutation | 26-01.1–2, 26-02.1–3 | COVERED |
| CONTEXT | D-08 | Identity-before-apply; entire instance preserved; continued bytes/output/DMC; fixed libretro size | 26-02.1–3 | COVERED |
| CONTEXT | D-09 | README/header/API contract and all three version consequences | 26-01.1–2, 26-02.1–3 | COVERED |

No required item is missing. General property/mutation/signal proposals, direct hardware capture, later no-op behavior implementation and Phase 27 PAL presentation/colors/final NTSC acceptance are excluded because CONTEXT explicitly defers them. Dendy/new boards/consumer integration/dependencies/unrelated DMA are binding scope boundaries, not planning omissions. Earlier exploratory format-7 guidance is superseded by D-05 through D-07.

## Spec-less edge fallback — explicitly flagged assumptions

The orchestrator supplied these rows; no SPEC exists. They stay unclassified/unresolved. Concrete planned oracles below derive from locked decisions and requirements, rather than relabeling the fallback as resolved.

| Requirement | Category | Status | Verification | Resolution | Reason | Probe |
|---|---|---|---|---|---|---|
| DMA-01 | unclassified | unresolved | null | null | null | unclassified — review manually |
| STATE-01 | unclassified | unresolved | null | null | null | unclassified — review manually |

Coverage remains applicable 2, resolved 0, unresolved 2, unclassified 2, explicit 0, backstop 0.

- DMA-01 concrete checks: a request pending at a real ordinary PAL read; no halt until an eligible opcode; actual accepted opcode/GET/resume; control on identical pre-divergence PAL history fails that halt-edge predicate; observed/linked equality and retained NTSC/OAM checks.
- STATE-01 concrete checks: positive same-region forks, sensitive lost-history control, exact denominator/scanline/gap boundaries, complete bidirectional atomic refusal and unchanged future output, older revision-16 NTSC format-6 payload loading, informational revision and real-module fixed size.

These are model and software-contract facts. They do not resolve every chip variant, later no-op bus effects, historical interrupted outcomes or universal cross-build continuation.

## Two-stage prohibition recall

Protocol read from the installed `prohibition-probe.md`; there is no compiled recall engine or existing SPEC prohibition section. Recall was performed separately for each requirement before the precision pass.

| Requirement | Stage 1 raw candidates | Stage 2 disposition |
|---|---|---|
| DMA-01 | Treat fixture success as physical proof; imply all chip revisions are settled; imply later no-op effects are known; use NTSC clocks for the control; mark operands as SYNC; skip pending read observation; let observer differ from production; lose a pending request; allow trace overflow; widen to unrelated abort behavior | Keep the first three transparency concerns as one bounded-claims prohibition. Clock/read-kind/pending/observer/overflow issues are routine correctness and belong in concrete tests. Unrelated abort scope is already an explicit binding boundary. |
| STATE-01 | Advertise old-format loading as identical historical behavior; imply all regions are interchangeable; imply arbitrary cross-build qualification; apply fields before refusal; lose remainders; weaken NTSC bounds; change state size; make revision a gate; serialize padding; corrupt pointed-to RAM | Keep the first three compatibility-transparency concerns as one prohibition. Remaining items are routine state correctness/hygiene covered by explicit tests and the existing canonical format contract. |

The precision pass does not dismiss either kept prohibition; both remain descriptor-less and flagged-unverified:

| Requirement | Category | Status | Verification | Resolution | Reason | Statement |
|---|---|---|---|---|---|---|
| DMA-01 | transparency | unresolved | null | null | null | MUST NOT present the selected trace-derived DMA model or passing owned fixture as direct hardware proof or a settled later no-op-cycle bus contract. |
| STATE-01 | transparency | unresolved | null | null | null | MUST NOT describe native format-6 load compatibility as identical historical emulation behavior or universal cross-region or cross-build qualification. |

Prohibition coverage: applicable 2, resolved 0, unresolved 2, test-tier 0, judgment-tier 0. Neither item has a `check_*` descriptor or attested fail-first claim. Explicit bounded agent source/documentation review is planned and identified as such; it does not auto-green these rows. Clean-room/owned fixture and retained authorization limits remain binding independently of this lifecycle.

## Verification and planning evidence

- Relevant prior commands are reused verbatim for `bus.unit`, public API/header/policy/docs, real `libretro.host`, and the full `cmake --workflow --preset ci` gate.
- New focused native commands use `build/dev`, grounded by tracked `CMakePresets.json`, existing executable targets and explicit registrations produced by these tasks. Every runnable automated block has a sibling `fails_when`. No source-text count/match gate is authored, so no speculative grep baseline is needed.
- Existing modification paths were checked with `git ls-files`. Detailed implementation evidence is recorded in the required plan summaries in this phase directory; generated ROM/state scratch data stay ignored and no ROM bytes are committed.
- The source/plan validators run during planning. Implementation CMake/CTest checks run only during execution. Existing stricter fixture/test caps stay in force; the final full workflow has an outer 1800-second limit under standing authority.
- Preconditions and reversibility were considered: no external setup, env secret or unavailable prior artifact is needed. The persistent format decision is already settled; new private call-only read kind is reversible. No checkpoint for automatic checks or already-approved local CI is warranted.
- Pre-mortem risks are covered: accidental operand-as-SYNC classification → production trace/control and expanded read-kind checks; self-consistent but weakened regional bounds → independent literal edge expectations and retained NTSC cases; meaningless continuation cut → bounded guest-generated pending/nonzero/old-bound-exceeding history and lost-history control; misleading pin refresh → old/new state-byte comparison and unchanged output inventory.

Planning validation: both frontmatter schemas and plan structures passed. A static contract pass confirmed nine modification paths and three tasks per plan, tracer first, at most five modification paths per task, tracked existing source paths, both requirements/all nine decisions, all five goal criteria and 33 coverage rows, intact closing tags and immediately adjacent failure conditions for every runnable automated command. Both research questions explicitly resolve planning scope while retaining the unknown hardware facts. Plan 26-02's retained one-way format contract produces the validator's generic missing-checkpoint warning; D-05 through D-07 explicitly settle that exact decision, so the documented already-settled exception applies and no new checkpoint is inserted. No implementation test was run.
