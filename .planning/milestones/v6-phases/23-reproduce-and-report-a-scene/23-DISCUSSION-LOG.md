# Phase 23: Reproduce and report a scene - Discussion Log

> Audit trail only. Planning/research/execution consume `23-CONTEXT.md`.

**Date:** 2026-10-30
**Areas:** Report/identity/privacy; permitted replay/controls; scratch isolation;
triage layers and independent oracles.

## Owner direction and method

The owner selected all relevant areas, requested subagent fan-out across relevant
roles with adversarial primary-source-informed consideration, and delegated
adoption of coherent recommendations. No routine preference question was needed.
This did not authorize another GSD step or live testing.

Three focused `gsd-advisor-researcher` reviews covered report/privacy,
replay/controls and triage/progress safety. Follow-ups resolved digest handling,
challenged attribution/save-loss traps and checked the apparent movie/checkpoint
requirement tension. No remaining owner choice or scope conflict was found.
Prior milestone discovery was reused. Further generic research had diminishing
value after the specific facility/evidence limits converged.

## Report placement, fields and privacy

| Option | Benefit | Cost or risk | Disposition |
|---|---|---|---|
| Canonical guide: player form, completed permitted example, developer replay/triage | One maintained contract; direct README entry | Clear navigation needed | Selected |
| README prompt plus testing-guide recipe | Visible; existing testing home | Field definitions drift; expands README | Short linked entry only |
| Public report with raw evidence local | Useful observation without publishing private artifacts | Missing identity limits reproducibility | Selected alongside guide |

Minimum fields follow REPRO-01. Unknown/withheld/not tried values are honest
results; executable replay has a stricter identity bar. Distinguish title-level
observations from reaching the exact scene. Expected behavior needs provenance.

Digest follow-up: content SHA-256 is report metadata, not ROM distribution.
Privacy rules prohibit personal identifiers/raw private artifacts, not all
content digests. Publish the permitted fixture digest; commercial reports can
share one or retain it locally with an explicit limit. Keep personal paths,
media, saves, checkpoints, raw logs/captures/traces outside public Git/CI output.

## Cold boot versus checkpoint continuation

| Option | Benefit | Cost or risk | Disposition |
|---|---|---|---|
| Separate cold-boot movie and native checkpoint neutral-input suffix | Current facilities; existing independent controls | Explain unsupported composition/audio windows | Selected |
| Add checkpoint-relative non-neutral movie continuation | More general replay | New indexing/runtime semantics beyond demonstrated need | Deferred |
| Treat host disk state as native payload | Superficially short recipe | Wrong container; unsupported identity and save loss | Rejected |

The runner rejects `--movie` with `--load-state`. The roadmap asks for cold boot
and a supported checkpoint, with wrong input OR wrong continuation detected;
it does not require arbitrary non-neutral movie continuation. Owned fixture
source-build targets are public facilities, not private helpers or tools assumed
to ship in release archives. Implementation must verify clean-checkout access.

`runner.boot_to_play` has independent input-sensitive controls. Existing
`runner.state.nmi_roundtrip` compares frame/state and same-suffix audio against
uninterrupted continuation; stale/omitted-load controls reject wrong continuation.
Credit these exact invariants rather than duplicate them. Resumed audio covers
its suffix, not full cold-boot audio. Every recipe needs finite frames and a
real outer timeout; the runner has no wall-clock timeout option.

## Triage, safety and correctness

| Option | Benefit | Cost or risk | Disposition |
|---|---|---|---|
| Bounded native/adapter/host/original-behavior triage | Narrows next evidence without guessing | Name representations and limits | Selected |
| Host-first comparison | Useful for presentation-only differences | Cannot clear input/callback delivery or prove hardware correctness | Conditional branch |
| Immediate synthetic fixture/fix | Can prove a localized causal fault | Premature before localization/independent oracle | Separate future scope |

Adversarial corrections adopted:

- Native/adapter agreement does not establish host input delivery, callback
  acceptance, displayed progression or audio resampling.
- Screenshots are transformed host RGB evidence; without alignment and known
  settings, compare qualitatively, not as native hashes.
- Expected/original behavior needs source, revision/region and confidence.
  Unknown-provenance video is a lead, not a hardware oracle.
- Exact-build/content pairing is support policy. The loader does not enforce
  exact build or behavior revision; successful load is not qualification.
- Checkpoints include battery RAM. Fresh scratch config/save/state/output paths
  and a separate newest-battery backup are operational requirements.
- Unknown/wrong identity/container combinations stop explicitly. Preserve real
  runner exit meanings; do not invent an unsupported status or silent fallback.

The triage review consulted primary
[Libretro core documentation](https://docs.libretro.com/development/cores/developing-cores/)
and [RetroArch VSync guidance](https://docs.libretro.com/guides/optimal-vsync/).
These support callback/presentation boundaries, not a new tested host invocation.
Local contracts and prior scene evidence remain the main authorities.

## Agent discretion and deferred ideas

Guide anchors, precise fixture budgets, public build commands, orchestration and
focused checks remain planner choices. Prove recipes and helper gaps during
implementation. No runtime/scene outcome was verified in discussion.

The matched property-test todo was reviewed and not folded: broad helper/
shrinking work has no demonstrated REPRO gap. Existing mutation/signal proposals,
accuracy/DMA/overscan, regions/mappers, Playstead, converters, generalized replay,
collector work and live campaigns remain deferred. Stops/budgets are unchanged.
Only discussion artifacts and handoff are produced; planning is separate.
