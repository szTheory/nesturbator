# Next milestone discovery: make NES play easy to try

Recorded 2026-10-30 during `$gsd-new-milestone`, following the owner's request
to mine Muhtari and other active cores. This is a researched recommendation,
not approved requirements, execution phases or runtime authority.

The owner confirmed that the Neo Geo details were pasted context from a
parallel milestone discussion and that the requested spot check is NES here.
Adapt the shared research intent; do not import console-specific tasks or
assume the two projects share readiness, content formats or scope.

Subsequent owner-directed playtesting established a faster near-term route:
Steam Deck → ES-DE → RetroArch → the owned core, using the real library and
normal progress storage for daily play. This can precede deeper Playstead
integration. Read the newer [PII-free lessons](../../playtesting/LESSONS.md) and
[dated baseline](../../playtesting/2026-10-30-nes-baseline.md) for that intent,
host repairs and bounded observations; the discovery proposals below do not
claim that consumer integration or a new milestone was approved.

**Later checkpoint:** the [mini postmortem](../../playtesting/POSTMORTEM.md)
supersedes this report's original session setup and broad-trial proposal.
Feedback now covers 22 Deck titles across all six supported mapper IDs; more
ordinary play is optional. Prefer RM-014's remaining docs/progress-safety and
reproducible-report gaps. Newly proposed RM-015 can precede DMA only when
bounded native/reference triage establishes a core fault. Read the current
[product roadmap](../../PRODUCT-ROADMAP.md) before selecting scope; none is
approved here.

## Recommendation

Prefer **v6: Player-ready NES delivery** before the remaining DMA cluster.
Make the existing emulator straightforward to install and try in RetroArch,
collect cheap title-specific feedback, and supply a precise Playstead handoff.
Actual Playstead integration remains a consumer-owned outcome requiring paired
acceptance; writing the handoff does not deliver it. Keep DMA next unless a
reproduced game defect or a concrete Playstead integration opportunity changes
the order. PAL remains the regional alternative.

The owner has supplied a new priority, not a claim that conformance work lacks
value. A small readiness milestone exposes real friction without reopening
the completed timing phases or promising a full commercial game library.

## Where the NES core stands

- v1 delivered the basic NROM/audio core; v2 added common boards and battery
  saves; v3 added states/rewind/run-ahead and the runner contract; v4 added
  conformance and robustness evidence; v5 completed the selected sprite/NMI/
  interrupt timing work. See [milestones](../../MILESTONES.md).
- Current supported boards are 0/1/2/3/4/7 with documented variants, in the
  NTSC cartridge scope. Current source uses ABI 1/state format 3/behavior 9.
  The canonical scoreboard records 119 PASS/27 FAIL, all 146 scored rows
  terminal, with five DRAW entries outside the score. This is finite evidence,
  not a full-library compatibility percentage. See [v5 closeout](../../milestones/v5-CLOSEOUT.md)
  and [public header](../../../include/nesturbator.h).
- Existing real RetroArch progression, state and battery checks plus native/
  actual-adapter tests deserve credit. The new delta is the ordinary player's
  installation/content/input experience and any identified package/consumer
  seam, not another generic host-testing architecture.
- Published 0.1.11 and merged v5 source are different baselines. Release PR #56
  remained open when checked in this conversation on 2026-10-30. Recheck before
  choosing session/package identity; never label a development build a release.
- The README has a demonstrated discovery/truth problem: about 2,400 lines,
  stale v3/revision-8 opening, and install instructions near line 2024 ending
  at the test card. Important runner/API/test prose should be preserved in
  topic docs while the entry point leads to playing a game.

## Transfer decisions

| Area | Best local action | Avoid / practical limit |
|---|---|---|
| Player feedback | Small private NES RetroArch trial: boot/gameplay, controls, picture, sound/pacing, progress and quit; record first bad scene/action | A title screen or one successful scene cannot establish general compatibility. Feedback is voluntary and useful; it is not recurring owner sign-off. |
| Documentation | Short player README: current status/limits → install → load content → controls → saves → troubleshooting → developer links | Preserve README-dependent test/release consumers as prose moves. Do not discard contracts, weaken their checks or add a docs dependency. |
| Actual host input | Reuse existing host tests; add one independent input-dependent visible predicate only if the new claim has a demonstrated gap | Callback equality and two equally idle screenshots can both pass. Verify optional host drivers in the actual binary before depending on them. |
| Save safety | Keep battery save, emulator snapshot, rewind and exact Continue distinct | Nesturbator already implements serialization. Playstead still owns newer-save preservation, recovery and consumer support rows. Recent-play is not restore. |
| Playstead | Release/core/info identity, honest capabilities and a small handoff; consumer proves library → host/core → play → orderly return → fresh-process battery | Its current mGBA-specific adapter is not a generic RetroArch integration. Do not implement a window or shared host inside this core. |
| CI/evidence | Repair actual collector identity/lookup defects before relying on a new collected bundle; reuse the two recent CI optimizations | Do not blindly rerun expensive suites, raise deadlines, add global mutation tooling or duplicate matrices. Existing accepted-run mitigations do not fix recurring collection. |
| Workflow | One state writer, high-signal local lessons with provenance, targeted reviews and tests after relevant changes | No arbitrary lifetime quotas, extra approval rituals or resumption of unchanged safety stops. Existing phase boundaries remain. |
| Reuse | Reuse existing code, C/CTest/CMake and platform facilities; allow a small owned helper or justified duplication | No incidental shared framework/daemon/schema repo; no copied sibling emulation implementation under this core's stricter own-code rule. |

## Proposed bounded slices, not registered phases

1. **Install and play.** A concise README and topic-doc migration preserve
   executable documentation/release contracts. A user can load actual supported
   content, understand controls/save locations and identify limitations. The
   tested core/host artifact is identifiable, including whether it is a source
   build, candidate package or downloaded release. Select existing tests and
   any fault-specific controls against the actual changed boundaries.
2. **Try games and prepare the consumer.** Use a small private session for
   subjective feedback and existing permitted fixtures for repeatable upstream
   evidence. Retain failures locally, convert reproducible faults into small
   owned regressions, and produce the NES-to-Playstead contract and qualification
   checklist. This milestone may finish its upstream handoff while actual
   Playstead integration remains explicitly unqualified.

If the owner instead selects **actual Playstead play as the milestone outcome**,
co-scope a separate consumer task before registering requirements: installation,
readiness, focus/input, clean exit and progress across launches must be exercised
through Playstead's production path. Decide cross-repository ownership and
consumer timing explicitly. Do not quietly swap that outcome for a document.

## Original session proposal (before content/setup)

The following records the initial proposal, not the current operational state.
At that point the owner had confirmed **Nesturbator (NES) here**, no game folder
had been supplied and the research had launched nothing. Subsequent setup and
observations are in the linked baseline/postmortem. Glueyneo is the
Neo Geo project; its real Metal Slug evidence is specifically **Metal Slug X's
Mission 1 opening scene**, with normalized content, not arbitrary commercial
ZIP compatibility. Keep that work in its own session and repository.

For NES, choose a few supported board/region cases rather than a library sweep.
Use private content/saves and an isolated reversible session with an identified
host/core. The agent gathers technical identity/logs; the owner can report:

> Title — got into gameplay? controls? picture? sound/speed? saves if tried?
> First problem/scene, or “felt good”; anything untried stays unknown.

Start ordinary play before optional rewind/run-ahead/shaders, then try requested
features separately. Preserve newer save progress before an older-state test.
No live retry, exhausted allowance, safety stop or other project's execution
budget is renewed by this proposal. Raw private media/derived states/movies/
captures/traces stay outside Git and public CI artifacts. A sanitized result
can describe the limited observation without distributing content.

## Four horizons and scope alternatives

| Horizon | Proposed direction and trigger |
|---|---|
| Immediate | Empty until the milestone scope/requirements/roadmap are approved. v5 remains archived. |
| Near | Prefer player-ready delivery (new RM-014), with RM-008's Playstead consumer integration separately qualified; then the five remaining DMA failures, protecting all current passes. An observed title blocker can change this order. PAL is the main regional alternative; Dendy remains independently conditional on regional infrastructure and its own clock/VBlank evidence. |
| Mid | Additional board, target-device performance, and native-host/managed Continue work only on named consumer demand or measured friction/budget. |
| Long | Expansion audio, MMC5, FDS and peripherals remain independent exploratory scopes with legal fixtures and hardware-specific acceptance. No new evidence promotes them. |

**Alternative A: DMA first.** Strong measurable accuracy outcome: five failing
rows plus current-pass protection, owned bus-cycle/state tests and unchanged
performance budget. Leaves player onboarding/consumer friction less visible.
**Alternative B: actual Playstead integration first.** Directly serves the
launcher goal, but requires coordinated consumer ownership and acceptance rather
than NES-only changes. **Alternative C: maintenance only.** Smallest bounded
collector/docs repair; it cannot claim playable onboarding or integration.

The ordering changed because the owner now explicitly wants to try games,
improve public usability and plan launcher integration. No new finding makes
PAL, hardware expansion or a large rewrite a prerequisite. Existing collector,
macOS nofp, historical attribution and physical/provenance limits remain in
the [debt authority](../../milestones/v5-MILESTONE-AUDIT.md); closed ledger
representation problems must not be reintroduced as open bugs.

## Research trail and stopping rule

- [Muhtari playtesting](MUHTARI-PLAYTESTING.md): requested experience,
  unaccepted actual-input work, artifact identity and short session feedback.
- [Muhtari direction](MUHTARI-DIRECTION.md): README, priorities, ownership,
  reuse and workflow transfer.
- [Other active cores](ACTIVE-CORE-LESSONS.md): Glueyneo, GabbaBoy,
  Schwaystation and targeted Genewhip; paused/dormant sources were not deeply
  mined once results converged.
- [Current Playstead boundary](PLAYSTEAD-BOUNDARY.md): actual consumer
  adapter/support rows and official RetroArch documentation.
- [Adversarial review](ADVERSARIAL-REVIEW.md): scope/claim review and corrected
  native-versus-consumer state compatibility wording.

Independent passes converge on actual consumer experience, truthful short docs,
fault-specific evidence and proportional CI. The next useful evidence is a
scope decision or actual permitted session, not further generic role fan-out.
