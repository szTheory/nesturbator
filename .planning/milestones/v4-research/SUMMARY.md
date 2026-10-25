# Project Research Summary

**Project:** nesturbator, milestone v4 Measured NTSC accuracy and robustness
**Researched and approved:** 2026-10-13
**Confidence:** HIGH for directly checked repository/upstream observations; MEDIUM for prioritization and implementation cost pending bounded measurement.
**Research method:** Three user-requested specialist agents and one adversarial cross-review pass; synthesis organized into the standard four dimensions without duplicating the completed research.

Prepared 2026-10-13. Three gsd-project-researcher agents used gpt-6.1-sol, medium effort, followed by one cross-review pass. The configured roadmapper resolves to gpt-6-astra, medium effort. Research stopped after cross-review converged on scope and no new distinct decision justified further fan-out.

## Key Findings

| Decision | Recommendation | Strongest alternative / change trigger |
|---|---|---|
| Immediate milestone | Measured NTSC accuracy and robustness, three focused phases | PAL-first adds visible compatibility; choose it next if the baseline has only narrow revision-sensitive gaps. |
| Accuracy promise | Complete pinned inventory and honest bounded baseline | A headline passing score is premature; choose exact correction requirements only after measurement. |
| Runtime safety | Add accepted-program execution/lifecycle coverage using existing tools | State fuzz already executes continuations; park a new target if a short prototype adds no useful reach. |
| Existing host proof | Repair three named Phase 13 weaknesses with discriminating controls | Screenshots alone cannot prove exact progression; use absolute anchors or an owned unique-frame fixture. |
| Phase count | Three; release/equality evidence remains in each phase | A fourth narrow closeout phase is possible if execution scope actually warrants it; no validation-only phase by default. |
| Roadmap storage | PRODUCT-ROADMAP.md alongside active execution ROADMAP.md | Seeds alone lose ordering; one combined roadmap competes with GSD parsing/rewrites; a hosted board adds synchronization overhead. |
| Horizons | Readiness and milestone distance, decreasing detail outward | Calendar dates lack evidence and create false precision. |
| Refresh cadence | Agent-owned project instruction at every milestone start/close | CI can check structure, not thoughtful priority review. Add a minimal structural guard only if needed. |
| Dependencies | Owned C/CMake/Markdown; reuse current fuzz/sanitizer tooling | No new framework or orchestration library has demonstrated value. |
| Regions | PAL first, Dendy independently scoped | Both affect clocks/audio/state/AV; no frame-rate-only shortcut. |
| Additional hardware | One validated board at a time, demand and fixture readiness first | Old dump-share percentages are neither current demand nor proven playable-title coverage. |

## Evidence and stakeholder reasoning

Players gain more confidence in the machine already supported, but no new region in v4. Host builders gain stronger state/progression proof and explicit stop behavior. C architects keep the existing timeline and per-instance state, rather than introducing a new abstraction. Test engineers gain complete inventory, negative controls and deterministic replay. Security work targets memory/resource/liveness defects without suggesting native execution of NES code. Devops budgets are measured and campaigns finite. Maintainers retain the three-deliverable boundary and a flat dependency tree. Provenance remains exact-pin/licence based; no GPL emulator implementation was read.

The current scoreboard protects 17 results on selected pages, not a measured full-suite score. AccuracyCoin's proposed upstream pin has 146 scored tests and five DRAW entries and changed name encoding. The loader fuzz target does not execute programs, but the existing state fuzzer does execute two-frame continuations; this existing value is preserved. Phase 13 frame-chain, command-completion and full-path substring weaknesses are still present; they weaken evidence rather than proving core state emulation broken. Six-platform hashes and portable host tests already exist.

The main scope risk is unbounded discovery: measuring a broad suite or fuzzing must not silently commit to all accuracy or safety repairs. Explicit inventories, deadlines, fixed campaigns and scope amendments keep the promise falsifiable. Confirmed reproducible safety failures cannot be hidden behind a green finite campaign. The current JAM callback behavior is a discovery/design gate, not a claim of a reproduced host hang or security exploit.

## Implications for Roadmap

The user approved three independently shippable phases: 16, scoped host evidence and roadmap upkeep (EVID-01..04, DIR-01..02); 17, complete pinned conformance baseline (CONF-01..04); 18, accepted-program execution/lifecycle robustness with closeout (ROB-01..04). Continue numbering after v3's Phase 15; no reset and no phase planning/execution in this step.

Phase 16 protects later host claims. The conformance baseline must precede any promise to repair unnamed hardware failures, but Phase 18 does not technically depend on a passing conformance score. Existing core/state/host contracts are its prerequisites. Phase 18 starts with a bounded coverage/throughput prototype, then fixes finite replay and sanitizer budgets. Unexpected major architecture or defect scope requires an explicit amendment.

Every phase uses existing CI/hygiene/perf and relevant parity/state evidence. No dependency is added. Keep PAL/Dendy, new board families and all-pass accuracy outside v4. PRODUCT-ROADMAP is current direction; ROADMAP is execution scope. Historical v3 research was copied byte-for-byte to `milestones/v3-research/` before replacing these active research files.

## Sources

Primary sources accessed 2026-10-13: [AccuracyCoin pinned README](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/README.md), [pin comparison](https://github.com/100thCoin/AccuracyCoin/compare/673ef550db296136d52229961e7d39366116882a...6bee919529289c555803b612b8c9e4f7eaaeeabb), [Holy Mapperel](https://github.com/pinobatch/holy-mapperel), [LLVM libFuzzer](https://llvm.org/docs/LibFuzzer.html), [Clang UBSan](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html), [RetroArch network control](https://docs.libretro.com/development/retroarch/network-control-interface/), [libretro core contract](https://docs.libretro.com/development/cores/developing-cores/), [GitHub public roadmap](https://github.com/github/roadmap), [Rust roadmaps](https://goals.rust-lang.org/2026/roadmaps.html), [Rust goal format](https://goals.rust-lang.org/about/goal-format.html).

Curated community hardware references include the preparation files' pinned sources and [NESdev cycle chart](https://www.nesdev.org/wiki/Cycle_reference_chart). Reference-emulator behavior is comparison evidence, not a substitute for hardware documentation. Roadmap ordering and effort remain judgments, not measured product demand.

Specialist reports: [accuracy](v4-scope/accuracy.md), [reliability](v4-scope/reliability.md), [governance](v4-scope/governance.md). Each includes local path/line evidence, alternatives, uncertainty and an adversarial pass. The user approved this scope on 2026-10-13. The substantive research is now preserved in project-local records; approved project instructions and the rolling direction ledger are installed.
