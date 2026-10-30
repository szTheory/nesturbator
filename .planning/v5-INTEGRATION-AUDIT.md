# v5 Cross-Phase Integration Audit

**Result: PASS — all expected cross-phase paths are wired.** The audit traced the completed Phase 19–21 deliverables at `HEAD e5e2d7d`. It found 8/8 expected connection groups WIRED, 11/11 requirement-aligned flows complete, and no integration BLOCKER. Nine evidence or maintenance warnings remain recorded below. No code or phase records were changed; only this report and focused test logs under `build/v5-milestone-audit/` were written.

## Scores

| Area | Result |
|---|---:|
| Expected cross-phase connection groups | 8 WIRED / 8; 0 BROKEN |
| Missing expected connections | 0 |
| Orphaned relevant phase contracts | 0 |
| Requirement-aligned end-to-end flows | 11 complete / 11; 0 broken |
| Requirement integration paths | 11 WIRED / 11; 0 UNWIRED |
| Focused local runtime/campaign/replay checks | 24 passed / 24 |
| Hosted-validator and inventory controls | 3 passed / 3 |
| Web/API routes and auth-protected pages | Not applicable; this C project ships a library, runner, and libretro adapter |
| Blockers | 0 |
| Warnings retained | 9 |

## Provides and Consumes

The 27 phase summaries use `provides`, `requires`, and `affects` metadata rather than a separate public-export catalog. These are the phase contracts relevant to the milestone:

| Phase | Provides | Consumes |
|---|---|---|
| 19 — JAM reporting and sprite-0 hit | AccuracyCoin guest-JAM classification/reporting; the adapter's frame/timeline-tagged JAM presentation cache; OAMADDR-based sprite evaluation and dot-timed hit; a measured page-19 baseline. See `19-03`, `19-05`, and `19-06-SUMMARY.md`. | The core's `NESTURBATOR_STOP_JAM` result and existing AccuracyCoin runner; prior passing campaign rows. |
| 20 — NMI line and VBlank flag | Split `$2002` status sampling; derived PPU NMI line and captured/polled history; core, runner, and libretro replay evidence; exact six-row NMI gate and collector contract. See `20-03`–`20-06-SUMMARY.md`. | Phase 19's settled sprite status bits and protected campaign baseline. |
| 21 — Interrupt latency and hijack | Legal IRQ acceptance history; shared resumable BRK/IRQ/NMI entry and vector choice; format-3/revision-9 state contract; native and actual-module replay; exact eleven-row gate and source-bound acceptance ledger. See `21-04`, `21-07`, and `21-09-SUMMARY.md`. | Phase 20's NMI capture/poll history and state visitor; Phase 19/20 target and protected-pass rows. |

There are no task-relevant orphaned phase contracts: each provided runtime, replay, and acceptance contract below reaches its consumer and a registered check. This repository has no HTTP endpoints, page routes, login, or session surface, so generic route-consumer and auth-protection checks do not apply.

## Cross-Phase Connection Inventory

| # | Expected connection | Trace proving the consumer call | Status | Requirements |
|---:|---|---|---|---|
| 1 | Phase 19 sprite identity/hit → Phase 20 split `$2002` status sampling | `src/ppu.c` starts evaluation from OAMADDR at dot 65 and tags the sprite selected on dot 66; the collision detector sets status bit 6. `nesturbator__ppu_status_read_finish` composes `$2002` from the earlier VBlank sample and the current sprite status bits. `ppu.sprites` and `ppu.nmi_races` are registered and pass. | WIRED | SPR-01, SPR-02, NMI-01 |
| 2 | Phase 20 PPU NMI output → production bus capture | `nesturbator__ppu_nmi_line` derives the signal from status/control; `src/bus.c:sample_interrupt_lines` captures rising NMI history and IRQ level at the bus close seam. `$2000` writes and VBlank set/clear alter the same derived line. Race and NMI edge tests pass. | WIRED | NMI-01, NMI-02, INT-02 |
| 3 | Captured NMI/IRQ history → instruction-specific poll and accepted request | `src/bus.c:cycle` transfers prior close samples to poll fields; `src/cpu.c:branch` preserves branch poll samples and `accept_irq_sample` applies I-flag rules; `cpu_step` consumes accepted history. `core.nmi_poll`, `core.irq_poll`, and `cpu.unit` are registered. | WIRED | NMI-02, INT-01 |
| 4 | Phase 20 NMI history + Phase 21 accepted IRQ → shared BRK/IRQ/NMI entry and vector hijack | `src/cpu.c:cpu_step` enters the shared staged path; `cpu_entry_action` uses production bus operations and chooses the BRK/IRQ vector pair after C4 when an NMI was recognized. `core.interrupt_entry` exercises the production bus object and the overlap tables. | WIRED | INT-01, INT-02 |
| 5 | PPU/CPU histories and staged entry → state visitor, validation, and public state API | `include/nesturbator.h` pins format 3/revision 9. `src/state.c` visits the PPU and CPU history fields, validates before APPLY, and rejects formats 1/2 with `NESTURBATOR_ERR_STATE_FORMAT`. `state.format_pin`, `core.state_reject`, and replay checks pass. | WIRED | KEEP-02, KEEP-03 |
| 6 | Native state API → runner save/load/replay | `runner/main.c` calls `nesturbator_load_state` and `nesturbator_save_state`; runner-state checks exercise NMI round-trip, idempotency, wrong-game load, and legacy-format rejection. Current focused tests pass. | WIRED | KEEP-03 |
| 7 | Native state API → actual libretro unserialize/runahead, including Phase 19 JAM cache | `libretro/libretro.c` delegates serialization to the public state API. `retro_run` returns the tagged cached image on a same-timeline JAM and exits before audio submission; `retro_unserialize` restores state and compares CRC/timeline. `tests/libretro/libretro_host.c` dynamically loads the built module, resolves and calls `retro_unserialize`, and compares speculative/committed output. `libretro.serialize` and `libretro.runahead` pass. | WIRED | JAM-02, KEEP-03 |
| 8 | Campaign output → exact eleven targets/protected PASS authority → shared hosted collector and six-platform evidence | `tests/cmake/v5_targets.cmake` rejects missing, duplicate, malformed, or non-pass target rows. `accuracy.scoreboard.no_lost_pass` consumes fresh protected observations. The Phase 21 wrapper includes the shared Phase 20 collector; local controls exercise both validators, while the phase ledgers bind the accepted artifacts, source identities, hash inventories, performance and Nightly content. | WIRED | All v5 requirements; especially KEEP-01, KEEP-02 |

The exact eleven targets are `Interrupt flag latency`, `NMI Overlap BRK`, `NMI Overlap IRQ`, `$2002 flag timing`, `NMI Control`, `NMI Timing`, `NMI Suppression`, `NMI at VBlank end`, `NMI disabled at VBlank`, `Sprite 0 Hit behavior`, and `Arbitrary Sprite zero`. The gate checks each full key and pass status, not an aggregate score.

## End-to-End Flow Results

Every flow below was traced through producer, consumer, final observable, and a registered check. The focused results are in `build/v5-milestone-audit/focused-ctest.log` and `focused-evidence-ctest.log`.

| Requirement | End-to-end path | Result |
|---|---|---|
| JAM-01 | Core frame returns `STOP_JAM` → AccuracyCoin observer attributes the active `$03` row → report/metadata and runner diagnostics say `guest-jam`; boot and strict-mode legs retain their distinct exit/report contracts. | WIRED; `accuracycoin.jam` passed. |
| JAM-02 | Native JAM → `retro_run` emits the current frozen frame and no audio → state restore tracks the cache timeline → actual libretro host compares run-ahead against the non-runahead oracle. | WIRED; `libretro.runahead` passed. |
| SPR-01 | PPU pattern collision → bit 6 at its dot → guest status read and `Sprite 0 Hit behavior` row → exact-target gate. | WIRED; `ppu.sprites`, current AccuracyCoin campaign fixture, and `accuracycoin.v5_targets` passed. |
| SPR-02 | Dot-65 OAMADDR start and dot-66 first evaluated identity → sprite-zero eligibility/hit status → `Arbitrary Sprite zero` campaign row → exact-target gate. | WIRED; the same sprite, campaign, and exact-target checks passed. |
| NMI-01 | VBlank flag set/suppression and split `$2002` samples → production bus read result → race oracle and `$2002 flag timing` target. | WIRED; `ppu.nmi_races`, campaign/protected fixtures, and exact-target gate passed. |
| NMI-02 | `$2000` enable/disable and VBlank edges → derived line/captured edge → legal CPU poll and guest handler marker → five remaining named NMI rows. | WIRED; `core.nmi_edge`, `core.nmi_poll`, campaign fixture, and exact-target gate passed. |
| INT-01 | APU IRQ level → bus close capture → CLI/SEI/PLP/RTI/branch poll semantics → accepted IRQ → instruction entry and AccuracyCoin latency row. | WIRED; `core.irq_poll`, `cpu.unit` in existing phase verification, and exact-target gate passed. |
| INT-02 | NMI during BRK/IRQ → captured history → shared entry's vector-pair choice and bus vector reads → guest marker, resumable state, and both overlap rows. | WIRED; `core.interrupt_entry`, `core.interrupt_state`, `core.state_fork.entry`, and `libretro.runahead` passed. |
| KEEP-01 | Protected starting PASS keys plus exact v5 target keys → current CI regression and source-bound six-lane acceptance → vectors/MMC3/Holy Mapperel/boot and actual-host checks. | WIRED; current no-lost-pass fixture passed; retained hosted evidence records all six lanes and those conformance gates for its accepted source. |
| KEEP-02 | Frame/audio/state pins → full-key sorted inventory and equality gates → moved state-format pins and six-platform hashes. | WIRED; `hash.inventory.self_test`, `state.format_pin`, `accuracy.scoreboard.no_lost_pass`, and source-bound six-platform inventories passed. |
| KEEP-03 | Format-3 state visitor/validator → core, runner, and real adapter restore → continuation equality and refusal of format 1/2. | WIRED; `core.nmi_state`, `core.interrupt_state`, `core.state_reject`, runner NMI/format tests, `libretro.serialize`, and `libretro.runahead` passed. |

No flow stops at a route or component that exists without a call. The API/auth route audit is not applicable to these three native deliverables.

## Focused Verification Run

The following two runs used the existing `build/ci` tree. They did not rerun the full CI workflow or contact GitHub.

1. Runtime, campaign, state, runner, and adapter connections: 21 explicitly selected tests plus three CTest fixture prerequisites, **24/24 passed in 28.42 seconds**. This included `accuracycoin.jam`, `accuracycoin.v5_targets`, `accuracy.scoreboard.no_lost_pass`, `ppu.nmi_races`, `ppu.sprites`, `core.nmi_edge`, `core.nmi_poll`, `core.irq_poll`, `core.interrupt_entry`, `core.interrupt_state`, `core.nmi_state`, `core.state_reject`, `core.state_generation`, `core.state_fork.entry`, runner NMI round-trip/idempotency/wrong-load and format-rejection tests, `state.format_pin`, `libretro.serialize`, and `libretro.runahead`. The fixture prerequisites `accuracycoin.campaign`, `accuracycoin.protected_observation`, and `runner.save.fixture` also passed.
2. Evidence acceptance plumbing: `phase20.hosted_evidence.controls`, `phase21.hosted_evidence.controls`, and `hash.inventory.self_test`, **3/3 passed in 38.55 seconds**. These are offline validator controls; they did not fetch hosted evidence.

Phase-verification records also report a freshly passing full workflow for the current work and the PR #61 final CI/Nightly checks. The full suite was not repeated for this audit. Local validators demonstrate acceptance-contract behavior; they do not recertify immutable hosted bundles.

## Requirements Integration Map

| Requirement | Integration path | Status | Issue |
|---|---|---|---|
| JAM-01 | Core `STOP_JAM` → AccuracyCoin runner classifier/active-row attribution → report, metadata, CLI result | WIRED | — |
| JAM-02 | Core frame/JAM → adapter frozen-frame/no-audio branch → actual `retro_unserialize` and run-ahead host comparison | WIRED | W9: hosted bundle identity remains source-bound. |
| SPR-01 | PPU collision timing → status bit 6 → guest row → exact eleven-key gate | WIRED | W6, W9 |
| SPR-02 | OAMADDR/dot-66 identity → sprite-zero eligibility → guest row → exact eleven-key gate | WIRED | W6, W9 |
| NMI-01 | PPU VBlank/suppression → split `$2002` sample → race test and target row | WIRED | W6, W9 |
| NMI-02 | PPUCTRL/VBlank → derived line/captured request → instruction poll/guest marker → target rows | WIRED | W6, W9 |
| KEEP-02 | State and output pins → sorted full-key inventory → protected-PASS and six-platform equality | WIRED | W1–W3, W8, W9 |
| KEEP-03 | Format-3/revision-9 visitor and validation → core/runner/libretro restore and continuation → old-format refusal | WIRED | W5, W8, W9 |
| INT-01 | IRQ source → production capture → legal poll history → accepted entry → latency target | WIRED | W6, W8, W9 |
| INT-02 | NMI capture during BRK/IRQ → shared staged entry vector selection → native/adapter replay and overlap targets | WIRED | W6, W8, W9 |
| KEEP-01 | Protected-main PASS authority + exact target gate → six-lane CI/conformance, hash inventory, actual host | WIRED | W1–W3, W8, W9 |

**Requirements with no cross-phase wiring:** none. All 11 requirements participate in shared PPU/CPU/state/campaign/host paths. The summary metadata issue W8 does not change the requirements register or the independent verification tables.

## Warnings (No Functional Blocker Found)

| ID | Severity | Finding | Requirement impact |
|---|---|---|---|
| W1 | WARNING | Phase 20 CR-01 / WR-05 and Phase 21 WR-01 remain: the committed source-identity helper does not itself reject worktree-only executable-mode changes. Independent all-input mode/blob comparisons mitigate the historical accepted runs (Phase 21 records 428/428 inputs), but do not repair recurring enforcement. | KEEP-01, KEEP-02 |
| W2 | WARNING | Phase 20 WR-03 remains: a recursive ignored-root scoreboard lookup can become ambiguous when build output contains another matching path; the recorded absolute/clean-source workaround succeeds. | KEEP-01, KEEP-02 |
| W3 | WARNING | Phase 20 WR-04 remains: the Nightly validator has a duplicate identical definition. Current controls pass; cleanup is deferred. | KEEP-01, KEEP-02 |
| W4 | WARNING | Phase 21 WR-02 remains: the historical measurement narrative associates one capture with different source/input/timing metadata than the canonical baseline records. The verification distinguishes the captures; this does not alter runtime results. | KEEP-01, KEEP-02 |
| W5 | WARNING | README overview still says behavior revision 8; the current header and detailed save-state section say format 3/revision 9. | KEEP-03 |
| W6 | WARNING | Physical exact-timestamp CPU/PPU tie precedence remains unmeasured. The implementation and tests support the documented deterministic policy and finite-margin cases, not a physical zero-margin winner. | NMI-01, NMI-02, INT-01, INT-02 |
| W7 | WARNING | Judgment/provenance annotations remain evidence limits; they are not converted into manual testing or a human sign-off gate. | NMI-01, NMI-02, INT-01, INT-02, KEEP-01 |
| W8 | WARNING | Summary metadata is incomplete after independent verification: Phase 19's final summary lists all four requirements; Phase 20 implementation summaries record NMI-01/02 as complete while KEEP-02/03 remain empty/pending, and its final plan summaries have empty completion metadata; all nine Phase 21 summaries have empty `requirements-completed` (some retain pending IDs). The current phase verification reports and `REQUIREMENTS.md` resolve the implementation coverage, but the summaries were not rewritten in this audit. | KEEP-02, KEEP-03, INT-01, INT-02, KEEP-01 |
| W9 | WARNING | Immutable hosted acceptance remains bound to historical source identities: Phase 20 candidate `7848cc8` / tested merge `716e8d4`; Phase 21 candidate `54c7db2` / tested merge `faf1b91`. Those records contain the accepted six-platform and host evidence for their sources. PR #61's final green CI/Nightly and current local checks do not rewrite those bundle identities or constitute a new hosted evidence collection for `e5e2d7d`. | All v5 requirements; most directly KEEP-01 and KEEP-02 |

These are warnings about maintenance, documentation, metadata, or the scope of evidence. None breaks the current runtime call paths, state continuation, target gate, or tested acceptance collector. The Phase 20/21 review dispositions and source-bound evidence ledgers remain authoritative for the deferred work.

## Evidence References

- Phase summaries and independent verification: [Phase 19](phases/19-jam-reporting-and-sprite-0-hit/19-VERIFICATION.md), [Phase 20](phases/20-nmi-line-and-vblank-flag/20-VERIFICATION.md), [Phase 21](phases/21-interrupt-latency-and-hijack/21-VERIFICATION.md).
- Deferred reviews: [Phase 20 disposition](phases/20-nmi-line-and-vblank-flag/20-REVIEW-DISPOSITION.md), [Phase 21 disposition](phases/21-interrupt-latency-and-hijack/21-REVIEW-DISPOSITION.md).
- Hosted ledgers: [Phase 20 evidence](phases/20-nmi-line-and-vblank-flag/20-EVIDENCE.md), [Phase 21 evidence](phases/21-interrupt-latency-and-hijack/21-EVIDENCE.md).
- Focused logs: `../build/v5-milestone-audit/focused-ctest.log` and `../build/v5-milestone-audit/focused-evidence-ctest.log`.
