# Architecture Guidance: v4 Measured NTSC accuracy and robustness

Approved scope, 2026-10-13. Preserve the existing CPU-driven timeline, mapper seam, per-instance state visitor and three-deliverable API. Phase research refines implementation within these boundaries.

## Host evidence

Strengthen `tests/retroarch/` sequence evidence through absolute saved-frame anchors and global constraints, or an owned unique-frame fixture if identical pictures remain ambiguous. Preserve ordinary game screenshots as compatibility evidence without claiming exact progress they cannot distinguish. Requests need bounded attributable completion; delayed artifacts cannot be assigned to a later step. Fixture basename identity replaces full-path substring matching. Correct `.info` metadata without changing supported board admission.

## Conformance

Extend the runner's existing AccuracyCoin result parsing and driving rather than adding a framework. Decode compressed test names with malformed/duplicate/missing-table controls. Keep immutable pin, exact licence/hash and key migration in one reviewed change. Preserve old-pin comparisons, report every inventory entry and keep DRAW separate. Bound controller driving and per-test/page completion; retain raw result bytes and classified diagnostics for prerequisite failure/hang.

## Execution robustness

Reuse accepted board geometry and test helpers to derive small PRG/CHR images from fuzz bytes. Select a fixed frame/input budget and lifecycle operations; compare fresh instances and save/load continuations through existing API contracts. Preserve loader and state targets. A short prototype measures accepted execution paths and throughput before replay/nightly budgets are selected. Test normal guest loops and JAM against library and adapter contracts; do not infer a security exploit or host hang from a static callback observation.

## State and performance

Harness work does not require state-format changes. If a reproduced core correction needs new instance fields, state visitor/bounds/fork coverage and documented behavior/hash changes accompany it. Existing perf exceptions remain the reviewed path for a justified correction; do not hide a slowdown or skip rendering to satisfy a gate.

## Product direction integration

PRODUCT-ROADMAP lives beside GSD's execution ROADMAP. AGENTS and the preparation handoff require milestone-boundary reconciliation; current requirements/phases link committed product IDs. Preserve historical research under the outgoing milestone. No global GSD edit, schema service or duplicated status authority.

## Sources

Existing preparation ARCHITECTURE, CONFORMANCE, ENGINEERING and LIBRETRO-AND-RUNNER; the current source paths and primary documentation are cited in [accuracy](v4-scope/accuracy.md), [reliability](v4-scope/reliability.md) and [governance](v4-scope/governance.md). Findings were checked 2026-10-13.
