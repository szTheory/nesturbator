# Resume v6 Phase 23 after Plan 23-01

Read AGENTS.md and STATE.md, then the Phase 23 CONTEXT, RESEARCH, PLAN-CHECK and
plans under `phases/23-reproduce-and-report-a-scene/` relative to `.planning/`.
Plan 23-01 is complete; its [summary](phases/23-reproduce-and-report-a-scene/23-01-SUMMARY.md)
records the report, bounded cold replay, measured local hashes and test evidence.
Plans 23-02 and 23-03 remain pending. The user already authorized the full
Phase 23 execution; no routine decision or confirmation is needed.

Next actor: continue the phase with:

`$gsd-execute-phase 23`

This resumes at plan 23-02 (native checkpoint preflight and scratch safety), then
plan 23-03 (layered triage and guide verification). Context may be cleared first
if the next session reads AGENTS.md, STATE.md and this file. Stop at the phase
boundary; do not implicitly start milestone closeout.

Plan 23-01 verified two cold 600-frame movie runs and matching ordered native
frame/tick/hash records plus transition and PCM hashes on the local macOS arm64
CI build. This is repeatability evidence only. The permitted fixture is not a
commercial-scene report. The five heuristic fallback edges and judgment-based
prohibitions remain flagged; the plan summary preserves them.

Continue with separate cold-boot movie replay and neutral-input native
checkpoint continuation: movie plus load-state is unsupported. Compare the same
audio suffix, preserve absolute frame numbering, reuse the input/stale/omitted
load controls, and keep exact-build/content support stricter than native loader
checks. Native/adapter agreement does not establish host presentation or
hardware correctness. Keep scratch isolation, explicit time/frame bounds and
private-artifact exclusion mandatory.

No live host/device/private-content/daily-progress work, wider runtime window,
collector repair, accuracy fix, push, PR or release is authorized. Phase 22
remains complete with its historical workflow and evidence limits unchanged.
Work is on `gsd/phase-23-reproduce-and-report-a-scene`; preserve the owner's
unstaged AGENTS.md and `.planning/config.json` changes.
