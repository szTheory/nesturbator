# Approval lessons for Phase 24

## What caused the repeated stops

There were two different things to protect:

- An interrupted full-CI/AccuracyCoin run had an unknown result. Its explicit stop and no-automatic-retry rule were valid and remain valid until that exact run is authorized again.
- Plan language then made ordinary bounded tests, implementation follow-through, and the required full local acceptance look like separate permission requests. Several completed checkpoints also restated that unrelated CI stop, which made settled decisions easy to ask about again.

Phase 24's state-format checkpoints were not literally the same decision repeated: the silence counter, split bus latches, and abort countdown each changed saved-state meaning, bumped the native format, and refused earlier formats. But the phase had already defined how to handle those consequences, and each change had a concrete field diff, native continuation/refusal tests, and documentation. The efficient boundary is one decision for a genuinely new user-facing compatibility consequence when the approved scope does not cover it, not a fresh authorization for each implementation slice. Once decided, record it and do not ask again.

The remaining full acceptance was also not equivalent to hosted CI or release. A local `ci` workflow was a needed phase check; its 39-minute ceiling and no-retry condition were separate explicit limits. A long runtime estimate alone did not create a new risk boundary. The current authorization is still exactly one local `cmake --workflow --preset ci` invocation within 39 minutes, with existing inner limits unchanged and no automatic retry after interruption/timeout. It does not authorize another campaign, hosted work, push/PR, merge, or release. A broader authorization would need to come from a separate, clear user correction.

## Adversarial review

| Lens | What autonomy improves | Boundary to keep |
|---|---|---|
| Security | Required local tests and verification run with the approved change, so incomplete work is not left behind. | Do not infer permission for external services, credentials, broad deletion, or destructive data changes. Preserve an explicit user stop while its cause remains unresolved. |
| Product | Users see compatibility consequences once, with the actual field/version diff and options, instead of approving every small implementation step. | A new incompatibility or user-data impact outside the approved phase still needs a decision. A previously approved format policy and its tested consequences should not trigger repeated prompts. |
| Release | Local evidence can be completed without waiting for a second prompt. | Local CI does not grant hosted CI, push, PR, merge, or release authority. This lesson changes none of the existing release rule. |
| Engineering | A plan and its acceptance criteria become executable work; required tests, fixes, reviews, and bounded retries proceed autonomously. | Respect exact scope, resource ceilings, inner test limits, and any explicit no-retry condition. A materially changed selection or cap is a new decision. |

The main tradeoff is that broad standing authority can hide a change in scope. Keep the authority tied to an already approved task or phase, carry forward its stated bounds, and ask only at an actual boundary. This preserves deliberate stops without turning runtime estimates or each required verification command into approval gates.

## Recommended AGENTS.md insertion

Add this under **Workflow handoffs** or as a short **Standing authority** subsection:

> **Standing authority.** Approval of a task or phase includes the ordinary local engineering needed to complete its approved scope: implementation and documentation, required builds/tests/local CI, review and verification, and bounded retries within the task's stated limits. Proceed without asking for each command or because a runtime estimate is long; report what ran and its result. Treat plan steps and acceptance criteria as work to perform, not separate permission requests. Ask only when evidence reveals a materially broader scope or an unplanned, materially higher cost, risk, compatibility impact, destructive data change, or external action. Keep an explicit user stop active while its cause remains unresolved. A specific approval is limited to its stated scope, resource window, and retry rule; it does not renew an interrupted run, authorize another campaign or hosted work, or imply push, PR, merge, or release authority. Where an explicit no-retry rule applies, preserve partial evidence and stop after interruption or timeout.

This wording lets the user approve the actual compatibility decision once, when one is needed, while allowing the already approved phase to carry that decision through serializer, tests, and docs. It also makes clear that a duration prediction is not itself a request for permission.

## Records to reconcile

- Keep the three completed state-format checkpoints as historical evidence. Do not rewrite their decisions or ask about them again.
- In `.planning/STATE.md`, `24-ACCEPTANCE.md`, and `24-EXECUTION-RESUME.md`, record the actual outcome of the currently authorized single local workflow and the existing 39-minute/no-auto-retry boundary. Do not mark acceptance complete from an incomplete or interrupted run.
- In `24-04-PLAN.md` and its handoff, resolve the blocking-human runtime checkpoint against that one exact authorization. Preserve the no-retry rule and the remaining hosted, protected-main, cross-platform, performance, and whitespace evidence as separate requirements. Do not extend the authorization to cover them.
- Plans 01–03 contain historical statements that the full workflow was not authorized at planning time. Leave those accurate historical statements intact; if future handoffs reuse them as active gates, point to this lesson and the current STATE/acceptance record instead of asking again.

## No further decision in this artifact

The immediate run's scope is already bounded and authorized as one local workflow with a 39-minute ceiling and no automatic retry. This artifact does not request broader CI/campaign authority or a change to release authority. Any later action outside that scope must be assessed at that time against the standing-authority rule and the user's then-current instructions.

## Applied resolution

The repository's concise Standing engineering authority section is committed.
The global agent preferences also record required local verification/retests,
approved compatibility follow-through, and measured efficiency improvements.
These are standing instructions; the user need not restate them next session.
The specifically limited workflow completed normally in444.6seconds with
exit8; its one invocation is consumed. Ordinary owned repairs continue.
See [actual evidence](24-CI-RUN.md) and [efficiency synthesis](24-CI-EFFICIENCY.md).
