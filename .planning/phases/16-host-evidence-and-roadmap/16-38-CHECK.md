## ISSUES FOUND

**Phase:** 16 — Host evidence and roadmap
**Plans checked:** 14 live-index plans; focused review of new Plans 38/39 and scheduled Plans 07/08, with historical 36/37 and the 34/35 duty lineage checked
**Issues:** 0 blocker(s), 1 warning(s), 0 info

### Coverage Summary

| Requirement | Plans / accepted evidence | Status |
|---|---|---|
| EVID-01 | Accepted 01/02/03/09/15/21/30 evidence; preparation repair in 38; fresh current acceptance in 39; native gate in 07 | Covered |
| EVID-02 | Accepted command controls; durable orchestration receipt repair in 38; fresh current-source, lifecycle, formal and parent gates in 39 | Covered |
| EVID-03 | Accepted exact-fixture work in 04; preserved through 38/39 and current-source suite | Covered |
| EVID-04 | Accepted source/package checks; 39 prerequisite, exact-head publication in 07 and actual released-byte verification in 08 | Covered; release remains pending |
| DIR-01 | Accepted installed roadmap/opening evidence in 06; preservation duties in 38/39 | Covered |
| DIR-02 | Accepted same-step lifecycle contract in 06; preserved one-step execution boundary in 38/39 | Covered |

All 17 locked decisions D-01–D-17 and the 39 source groups (3 goal, 6 requirement, 13 research and 17 context groups) remain represented across the accepted/live graph. The four unclassified assumptions remain explicitly flagged in both new plans. The four edge controls, four metadata-only files, Plan 26 terminal=false/pending diagnosis, historical UNKNOWNs, and the raw 34 transcript's 63-character value versus its separately recorded 64-character digest are preserved. No deferred idea or scope reduction was found. The phase's only relevant project requirements are the six listed above.

### Plan and Schedule Summary

| Plan | Tasks | Declared files | Wave / dependency | Status |
|---|---:|---:|---|---|
| 16-38 | 2 | 6 | 10 / 16-30 | Structurally valid |
| 16-39 | 3 | 13 | 11 / 16-38 | Structurally valid; scope warning below |
| 16-07 | 3 | 1 | 12 / 16-39 | Structurally valid; body retained |
| 16-08 | 2 | 1 | 13 / 16-07 | Structurally valid; body retained |

The live index contains 14 plans. Its recovery/release chain is acyclic: 21 → 30 → 38 → 39 → 07 → 08. Plans 36/37 are marked superseded and are excluded from the live index. Their plan bodies remain byte-identical to HEAD; the same is true for Plans 07/08, whose scheduling frontmatter alone changed. Plan 39 requires genuine 38 preparation success **and** actual administrative parent reconciliation before its separate dispatch; Plan 07 waits for 39 and all actual parent gates; Plan 08 waits for 07's exact-head consent/publication gate.

`verify.plan-structure` returned valid for 38 and 39, with all required task fields present (2 and 3 tasks). The updated 07/08 plans also remain structurally valid. The 38/39 `must_haves` include the six phase requirement IDs, with explicit key links from 38's code-only export to 39's fresh import and from 39's reviewed candidate/parent results to 07.

Plan 38 preserves the failed 36 dispatch as terminal 0/2 and 37 as unexecuted 0/3. Its amendment gives the concrete read-only actual-runtime/store/serialization/scope/quoting proof before any write, then the acyclic A/B receipt sequence: store the actual A request/result before its receipt/completion, retain B's actual returned result, and stop on missing or failed receipts without reconstructing, retrying or accepting file equality as authorship. The genuine unchanged copied-34 CLI RED, coherent prerequisite correction, actual exported-CLI entry, 12 required plus five supplemental intended history refusals, 19/143 history, 55+6 preparation controls, journal controls and retained 18-case integration all run before freeze and replay on unchanged final bytes. Any post-freeze failure is terminal. The export remains code-only with runtime admission false.

Plan 39 requires both the complete 38 preparation and the administrative parent reconciliation, then a separate actual 39 dispatch. It retains one fresh ordered 12-check closure before runtime authority, including actual caller receipts, elevated Darwin boot access, six unused slots, 7200000 ms runtime authority, a 10-second reserve, at most two corrections and two distinct pairs, source-root/finalizer/cwd/causal/lifecycle/default-all controls, and the 55355 FD9 exclusion. It preserves all prior-history and reason-specific failure limits, requires 649 unique tests/four jobs/no skips/four workflow stages and three fresh ZIPs before the five-path commit, ten-blob equality, separate-context exact-commit review with zero open findings, and all separate parent gates. Plans 07/08 retain exact-head native CI, affirmative owner consent, normal publication and actual verification of 18 archives/checksums/attestations plus six metadata files.

The new orchestration protocol and all retained command/failure pairs are concrete. Supplied failing-direction probe `/tmp/nesturbator38-failure-directions.json` reports 327/327 `ok`, zero blocker/warning and `readError: null`; the supplied verify-path probe reports 327/327 `not_applicable`, zero blocker/warning and `readError: null`. `not_applicable` supplies no positive runtime-path or acceptance evidence. No application, build, CI, test or generated helper was run during this review.

### Other Verification Dimensions

- **Context compliance:** all locked decisions are covered; deferred ideas are absent; no task contradicts the user's constraints.
- **Architectural tier:** the research responsibility map assigns this work to owned test tooling, existing APIs, CMake and Markdown records. The new helper/admission work stays in test tooling and retains strict admission; no tier mismatch was found.
- **Nyquist:** skipped because `.planning/config.json` explicitly disables `workflow.nyquist_validation`; no phase Validation Architecture is supplied.
- **Research resolution:** `16-RESEARCH.md` marks Open Questions `(RESOLVED)`.
- **Patterns/reviews:** no phase PATTERNS.md or 38/39 REVIEWS.md exists.
- **AGENTS.md:** the current branch is `gsd/phase-16-host-evidence-and-roadmap`; behavior changes in 39 retain tests, README/public-header updates, full CI before the paired commit, noreply identity, and the later PR/publication gates. No new dependency or core behavior change is planned.
- **Data contracts/coupling:** 38 exports helper-preparation code only; 39 imports it as fresh input and rebuilds all acceptance evidence. The plans are in ordered waves with declared dependencies; no same-wave mutable-resource coupling or incompatible transform was found.

Estimate checks (`--calibrated`) report:

| Plan | Estimated tokens | Budget | Ratio | Confidence | Calibration | Result |
|---|---:|---:|---:|---|---|---|
| 38 | 74,500 | 100,000 | 0.745 | low | factor 1; sample count 0 | Under budget; no re-slicing recommendation |
| 39 | 99,900 | 100,000 | 0.999 | low | factor 1; sample count 0 | Under budget; no re-slicing recommendation |

These estimates have no completed-phase calibration evidence and should not be treated as precise. STATE/ROADMAP still carry the known historical Plan 36 stop; this review treated that as the supplied administrative lag and verified the new plan graph/frontmatter directly.

### Warning — should fix

**1. [scope_sanity] The declared file footprint should stay within the per-plan warning threshold or be divided into smaller plan slices.**

- Plan: 16-39
- Evidence: `16-39-PLAN.md` declares 13 modified paths (Task 1: 5, Task 2: 5, Task 3: 3). This exceeds the 10-file warning threshold, although no individual task exceeds five files and the task write set is complete.
- Example fix (non-binding): consider whether one independently executable group can move to a later plan while preserving the ordered acceptance/freeze gates, or document why the 13-path aggregate is necessary for this single closure.

### Structured Issues

```yaml
issues:
  - plan: "39"
    dimension: "scope_sanity"
    severity: "warning"
    required_property: "The declared per-plan file footprint stays within the scope warning threshold or is split into independently executable slices."
    description: "16-39-PLAN.md declares 13 modified paths across its three task file lists (5 + 5 + 3), above the 10-file warning threshold. Each task is individually bounded and the declared write set is complete, but the aggregate plan footprint remains above the warning boundary."
    fix_hint: "Consider splitting an independently executable group into a later plan, or document why the ordered 13-path closure should remain one plan."
```

### Recommendation

0 blockers and 1 warning require planner review. No phase requirement is uncovered, no dependency is broken, and the user-selected recovery remains fully represented. Re-check Plan 39 scope before execution; do not execute during this planning review.

### Review Snapshot

HEAD at review: `55ff534d5199ab2bd228a964eb2d0ae12a7f927b`; branch: `gsd/phase-16-host-evidence-and-roadmap`.

| File | SHA-256 |
|---|---|
| `16-38-PLAN.md` | `fb947cd34ca10a25702eef3aa2fc4b76e484ee9c3391acb5a570ca82119810fd` |
| `16-38-AMENDMENT.md` | `562d65b13be525587c88555756131aaea8cf1cbdb722b3e1408e9f0f773b6ef2` |
| `16-39-PLAN.md` | `3464f5b471c1ba6e0c7ec38a711827d438ec0ca6334a61099d9e947917745bec` |
| `16-39-AMENDMENT.md` | `48eabcf827b0736093b2a62b7b44811eb84cda13924b294a340c76b4798a6327` |
| `16-36-PLAN.md` | HEAD `063e1d594a955983b1b00368a1558ac74a26535753162ba0fa367bb7b794f144`; working `4e61c53cc35f719357e8ed3fc8307d6b6e5b19360fbda7afacb95f9d77bd96fb` |
| `16-37-PLAN.md` | HEAD `4131002488818e431286bcc2de79f31eef03bdd5a823f34b1b7f3b2c17632342`; working `a42e09136c99df0b7da1bf6906a838bc7d1426fa07509900a639b143e345efe2` |
| `16-07-PLAN.md` | HEAD `cb2783549a5aacf4b54a227448976efd42ef758733ec11a49206747bbe511fb2`; working `66ed69b3815b8fb40d6fca4006f09e3de757d92dcea37c678cf00662194805b0` |
| `16-08-PLAN.md` | HEAD `48c6a2eab4b3f8bf8b021e40e63203ebcb7fd84a7fe79b63b58b2efe23266eed`; working `eed75120eb1bd16f5905669718974fe26eee38cbe66a3fa73bb8bc86e427bcd7` |
| Path probe JSON | `23e32bcca6b925212dae63f16b6f01b8ad2eb97494f50a9bab0bad11e58bd9a5` |
| Failing-direction JSON | `48240ac3814c82e2c53d5e39a4b3abfddc47d0ac9d8efeafafc0e7f4a7519534` |

The working bodies of Plans 36/37/07/08 exactly equal their HEAD bodies; only their frontmatter differs. The six pre-existing dirty project files remained the same paths throughout review. Their review-snapshot hashes are:

| File | SHA-256 |
|---|---|
| `.planning/config.json` | `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e` |
| `tests/retroarch/state_session.sh` | `1d9574ff863a0757f5a9cf8818e34c7daf6bda9063fabd78f155bf2b1575d560` |
| `tests/retroarch/test_state_session.sh` | `65e772450f992d16cfdbce6ee86e254b2dbc78f0bf1b84cbce619ba46af86e58` |
| `tests/retroarch/interruption_trace.sh` | `a899d87ed398cbfff75b965b01a5575912cb9b0696b048ffdd55732d278ee165` |
| `README.md` | `57f275d7ee2dfb1b2c6f2c31393c8620406992aaa8d172b87ab531af8eebf97e` |
| `include/nesturbator.h` | `aa2e65d8ab258a102ff43d8c9ffeac6161b33a469337a8e5b2b1ae4ebb0295f0` |

No source, configuration, runtime, build or test file was changed by this review.

## Revision recheck — iteration 2 (2026-10-21)

**Current result: ISSUES FOUND — 0 blockers, 1 warning, 0 info.** I re-read the revised Plan 39 and amendment against the original locked decisions and the prior scope finding. The revision adds a concrete three-task ownership map, an explicit analysis of why the acceptance transaction cannot be split into independently executable plans without duplicating or transferring authority, and model-working-set limits (200 source lines or 64 evidence rows and 16,384 designed stdout bytes per routine request). It also requires machine validation to consume all raw evidence and forbids using bounded summaries in place of caller-memory receipts or truncating tool results. These controls bound the planned inspection process and preserve the required evidence.

They do not resolve the measured scope-threshold issue: Plan 39 still declares 13 modified paths, versus the checker rubric's 10-file warning threshold. The paths are now fully and accurately assigned 5/5/3 across three tasks, with no task above five files; this does not change the per-plan total. The estimate remains 99,900/100,000 (ratio 0.999), confidence low, sample count 0. The calibrated estimate query is under budget and returns no re-slicing recommendation, but the low-confidence estimate is not independent evidence of fit; the rubric says to weight task/file thresholds more heavily when calibration is low. The detailed no-safe-split rationale is accepted as mitigation and makes this a transparent warning rather than a blocker, but it does not make 13 fall within the warning threshold. Keep the original scope finding at **warning**; do not downgrade it to info or mark it resolved.

The supplied probes were consumed without rerunning them: verify-path probe is 327/327 `not_applicable` with no read error (no runtime-path proof); failing-direction probe is 327/327 `ok`, zero blocker/warning and no read error. The six pre-existing dirty project files retain the hashes listed above. No application, build, CI, test or generated helper was run; only this CHECK document was updated.

### Revision snapshot

HEAD remains `55ff534d5199ab2bd228a964eb2d0ae12a7f927b` on `gsd/phase-16-host-evidence-and-roadmap`.

| File | Current SHA-256 |
|---|---|
| `16-38-PLAN.md` | `fb947cd34ca10a25702eef3aa2fc4b76e484ee9c3391acb5a570ca82119810fd` |
| `16-38-AMENDMENT.md` | `562d65b13be525587c88555756131aaea8cf1cbdb722b3e1408e9f0f773b6ef2` |
| `16-39-PLAN.md` | `33320a475c3fd2dec2938797497314abf98b548dc6640a749ac8b820160db975` |
| `16-39-AMENDMENT.md` | `cea7b4d2f57d1bbe38f53e8041b84fc20c14e51bdb80ac74b4ecd6ea1fd673d7` |
| `/tmp/nesturbator38-verify-paths.json` | `23e32bcca6b925212dae63f16b6f01b8ad2eb97494f50a9bab0bad11e58bd9a5` |
| `/tmp/nesturbator38-failure-directions.json` | `48240ac3814c82e2c53d5e39a4b3abfddc47d0ac9d8efeafafc0e7f4a7519534` |

### Current issue payload

```yaml
issues:
  - plan: "39"
    dimension: "scope_sanity"
    severity: "warning"
    required_property: "The declared per-plan file footprint stays within the scope warning threshold or is divided into independently executable slices."
    description: "The revised plan documents why its acceptance transaction cannot be divided without duplicating or transferring authority and bounds model inspection per task, but it still declares 13 modified paths (5 + 5 + 3), above the 10-file warning threshold. Its 99,900-token estimate is 0.999 of the 100,000 smart-zone budget with low confidence and zero calibration samples, so the explicit file-count warning remains material."
    fix_hint: "Any revision that reduces the per-plan file count to the warning boundary or lower while preserving the locked evidence and authority contract would satisfy the threshold; otherwise retain this as a visible scope warning."
```

## Parent disposition — user-approved planning override

After the revision recheck retained0blockers/1warning, the user asked for the recommendation and instructed proceeding with it. The parent recommended proceeding with the scope warning recorded because the13-path ordered acceptance transaction cannot be split without frozen-authority transfer. The user thereby authorized the proceed-anyway path of the planning workflow's stalled-revision gate. The finding remains a warning:13paths and99900/100000low-confidence/sample0tokens. No claim of resolved scope, runtime fit or execution acceptance is made. STATE/ROADMAP record the override; plans are ready with override after final planning gates/commit. Only38may execute next;39requires genuine38success and actual administrative parent reconciliation followed by separate invocation. No execution is authorized by this planning disposition.

## Final planning serialization

Parent removed trailing whitespace and extra EOF blank lines only after review. No plan wording, task, field, command, scope or acceptance requirement changed. Reviewed semantic content remains intact; final commit-ready plan bytes are:

| File | SHA-256 |
|---|---|
| `16-38-PLAN.md` | `0fb0c70e4678b80d3e9f41b5134ab09e3a4e9703bb58feb31d8ff2c6a53679b9` |
| `16-38-AMENDMENT.md` | `642d742a922827b0ea37bc51c44438e70031e5b3e3e4667f6f377177cdbb47e9` |
| `16-39-PLAN.md` | `fdc2caa597ff7274762111ccdd6787326cc72006977d52ba633408974ee05f85` |
| `16-39-AMENDMENT.md` | `1ee461f3188bd65617bebd1c7e2443500c332abb339b6e05055b2321abb24f35` |
