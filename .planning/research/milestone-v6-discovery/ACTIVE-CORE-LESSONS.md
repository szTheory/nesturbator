# Lessons from other active emulator cores

Researched 2026-10-30, read-only sibling documents/history. No media was searched,
no emulator was run, and no sibling implementation or GPL/LGPL emulator source
was opened. These are observed project records and recommendations, not new
behavioral verification. Source paths below identify project-relative files;
sibling evidence is local and can change independently of this repository.

## Survey and scope

| Project | Recent activity inspected | Reason for depth |
|---|---|---|
| Glueyneo | October 30 player-first research; archived first-playable UAT | Direct answer to the Neo Geo/Metal Slug question and a closely matching recent user request |
| GabbaBoy | October 24–30 lessons, current state, working rules | Concrete costly CI, state-validation and release-tooling failures with recorded remedies |
| Schwaystation | October 29 CI-tail summary, working rules and transfer ledger | Proportional process, public docs and genuine cross-platform enforcement |
| Genewhip | October 24 recovery summary; current operating rules/state | A cautionary recovery-loop case and later simplification lessons; no runtime authority imported |
| Cogmaster | Latest October 20 pause commit only | Paused after Phase 3; other sources already cover the applicable build/process lessons, so deeper mining has diminishing value |

Unrelated applications and older dormant emulators were excluded. The three
principal peers all have recent activity; modification times alone were not
used to infer readiness.

## Neo Geo identity and the Metal Slug observation

**Glueyneo is the Neo Geo project.** Its recent readiness report says accepted
commercial evidence covers **Metal Slug X**, the selected MVS set and Europe MVS
v2 BIOS, first-character Mission 1 opening segment. This includes controls,
scrolling/composition and representative music/firing, not a completed game or
general Neo Geo library. Its normal content route consumes normalized GNFX/GNMV
bundles; arbitrary game ZIPs are not directly playable. Another board/title
cannot safely become supported merely by relaxing the selected profile's checks.

Primary recorded user evidence exists: the archived Phase 7 UAT binds
`verified_head: 25064032` to the report “Both recognizable music and firing
effects.” The same record explicitly says slowdown under heavy concurrent
development load was **not attributed or performance-qualified**. The October 30
readiness research is inspection, not a fresh replay of that scene.

Sources: Glueyneo `.planning/preparation/2026-10-30-commercial-spotcheck-readiness.md:5–17,34–65,69–75`;
`.planning/milestones/ws-first-playable-game-2026-10-24/phases/07-sound-and-scenario-qualification/07-UAT.md:8,18–26`;
`AGENTS.md:3`.

This is valuable context for a **separate Neo Geo session**. NES media, loader
support and compatibility must be assessed in Nesturbator's own contracts. No
Glueyneo import utility, fixed board profile or BIOS requirement belongs here.

## Recommendations and transfer limits

| Decision / lenses | Recommendation for Nesturbator | Tradeoff, counterexample and existing coverage |
|---|---|---|
| Player feedback versus benchmark progress; product/QA | Make the current NES RetroArch path easy to try, accept brief private title-specific feedback, and turn a reproducible defect into a focused owned regression | A good scene does not prove full-game compatibility or console accuracy. Nesturbator already has actual RetroArch progression/state tests; do not rebuild a qualification framework. Glueyneo's GNMV media tooling solves a different problem. |
| README discovery; player/docs/release | Shorten the entry point to current supported boards, truthful status, install/load/controls, save locations, known limits and links to deeper guides. Move harness/maintainer details to topic docs incrementally with relevant work | Nesturbator's README is 2,421 lines, starts with “Status: v3”, and describes revision 8 while a later section says revision 9. This is a demonstrated navigation/consistency problem. Do not imply unreleased implementation is in a downloadable package; preserve the historical evidence in deeper docs. |
| Actual host route; UX/integration/security | Check the loaded core artifact and ordinary controller/audio/save/exit route in an isolated reversible session; retain failures and useful diagnostics locally | Native state equality cannot prove frontend controller mapping or physical sound. Conversely, voluntary listening/play is additional feedback, not a mandatory manual checkpoint for deterministic contracts. Existing NES host tests are an asset, not missing work. |
| Timeout pressure; CI/devops/performance | Inspect duration headroom and subprocess counts when a job approaches its limit; batch repeated Git reads, remove redundant work, and shard only with complete complementary inventories | GabbaBoy needed seven CI attempts before correcting serial/full ASan work; another test passed at 96% of its timeout before tipping over. Raising bounds blindly or retrying unchanged work hides cost. Nesturbator already delivered two October CI optimizations; reprofile before copying an optimization. |
| Negative controls; QA/security/emulator state | For a selected new state or DMA invariant, demonstrate that the intended bad behavior fails for its intended reason; pair hostile-state refusals with reachable-state save/load properties | GabbaBoy's controls all passed while several compiled the identical wrong mutant and a witness always printed the matching line. Merely counting green controls is weak evidence. Nesturbator already tests reachable interrupt cutpoints; inspect a specific missing DMA seam rather than launching blanket mutation infrastructure. |
| Verification churn; workflow/devops | Keep post-merge delivery facts outside verifier-covered historical summaries; prepare all covered records before final verification; rerun only evidence actually invalidated | GabbaBoy appended a boundary SHA to an already covered SUMMARY and paid for an otherwise redundant re-verification. Nesturbator's v5 closeout preserved summaries byte-for-byte, so this is mostly a planning discipline to retain. Genuine later behavior changes still require verification. |
| Archive durability; maintainability/release | Put reusable checkers under tests/scripts or existing tests/cmake, and keep their live inputs independent of movable phase directories | GabbaBoy's archive would have broken nine test consumers. Nesturbator's sampled registered state/hash tests already use tests/cmake, and v5 archived successfully. Check a concrete consumer before proposing a new archive scanner. |
| Portability feedback; build/devops | Exercise supported floor-tool spelling, CRLF/path behavior and actual nested shell selection early through existing CI; push reviewable changes early enough to learn from hosted lanes | GabbaBoy's local CMake 4 syntax parser rejected CMake 3 output after 88 unpushed commits; Windows nested bare bash chose WSL although the parent had usable Git Bash. This is targeted producer-boundary coverage, not a reason to add another platform matrix. |
| Gate value and scope; architecture/product | Every new recurring check should name the fault it prevents and its recurring value. Prefer one small shared helper only after a real second consumer; permit one-off evidence for one-off questions | Schwaystation keeps measured CI-tail evidence without adding a recurring gate. Its arbitrary line ceilings and automatic deferral of tooling findings are project policies, not generally safe NES rules: an evidence collector defect can invalidate acceptance and needs a real disposition. |
| Recovery loops; product/safety | Each slice should deliver a falsifiable product delta, with per-process/time/resource bounds and truthful source-bound evidence. Preserve actual safety stops while avoiding invented lifetime run quotas or accounting ceremonies | Genewhip distinguishes diagnostic value from final acceptance, and its current operating rules reject cumulative interpreter-closure/counter prerequisites. A failing safe development run can teach something without qualifying compatibility. Neither a sibling's old halt nor its newer authority changes Nesturbator's existing stops. |

## Exact supporting records

- **Player-first existing capability:** Nesturbator `README.md:2063–2086` already
  documents real RetroArch and battery roundtrips. `tests/CMakeLists.txt:696–707`
  registers state-format and hash-inventory controls; `tests/core/test_state_reject.c:498–558,631–735`
  covers reachable histories and roundtrips. These are inspection observations,
  not new test runs.
- **Docs drift:** Nesturbator `README.md:6–35,1341`; Schwaystation
  `AGENTS.md:32–41` explicitly separates a short user README from deeper docs.
- **CI cost:** GabbaBoy `.planning/context/LESSONS.md:894–900` (retries,
  complementary sharding and enclosing waiter bounds), `:947–953` (1,398 Git
  processes, batch remedy and preserved checks). Nesturbator `.planning/STATE.md:70–75`
  records the already delivered optimization work.
- **Controls/state:** GabbaBoy `.planning/context/LESSONS.md:956–963` records
  identical mutant sources, invariant false positives and reachable-state
  rejection/OOB defects, then the discriminating repairs.
- **Metadata/archive:** GabbaBoy `.planning/context/LESSONS.md:903–918`;
  Nesturbator `.planning/milestones/v5-CLOSEOUT.md:160–170`.
- **Portable producer boundary:** GabbaBoy `.planning/context/LESSONS.md:965–990`.
  Its October 30 shell repair is implementation evidence pending actual renewed
  target CI according to that record; do not promote it to a platform pass.
- **Proportional checks:** Schwaystation `AGENTS.md:42–54,72–76` and
  `.planning/quick/261029-ldd-complete-the-phase-1-ci-tail/261029-ldd-SUMMARY.md:34–53`.
  Independent lanes compare eight hash files; the measured-timeout repair added
  no recurring gate. NES already has six-platform parity; extra architectures
  require demand rather than copying PS1's matrix.
- **Recovery lesson:** Genewhip `.planning/context/OPERATING.md:65–73` and
  `.planning/quick/261024-oh9-diagnose-phase6-recovery-loop-synthesize/261024-oh9-SUMMARY.md:12–36`.
  The October 24 proposal remained unadopted and runtime authority unchanged.

## Suggested priority

1. Carry the user-facing NES trial/docs/Playstead-readiness candidate into the
   milestone discussion, alongside the existing DMA alternative. A short actual
   title/host check can reveal whether player friction or an accuracy defect is
   the next valuable repair; research alone does not select the milestone.
2. Repair existing collector enforcement before relying on a new accepted
   collection, as the current audit already requires. Apply the negative-control
   lessons directly to those repairs rather than creating generic policy gates.
3. Retain state, determinism, privacy and dependency ownership already here.
   No new dependency, replacement host, benchmark framework, universal
   compatibility claim, sibling edit or runtime window is selected by this note.

The high-value findings converge after these sources: actual consumer evidence,
useful public docs, fault-specific controls, proportional CI and honest scope.
Further dormant-project mining would mainly duplicate them.
