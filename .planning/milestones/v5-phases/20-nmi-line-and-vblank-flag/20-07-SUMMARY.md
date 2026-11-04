---
phase: 20-nmi-line-and-vblank-flag
plan: "07"
subsystem: testing
tags: [ppu, bus, nmi, vblank, accuracycoin, ctest]
requires:
  - phase: 20-06
    provides: [exact-six AccuracyCoin target gate, measured campaign and protected-pass evidence]
provides:
  - Independent dot-coordinate witness for changing sprite status across the two $2002 sample edges.
  - Ordinary and mirrored production decoder tests plus a bounded wrong-sampler rejection.
  - Evidence-linked WR-01/WR-02 dispositions and refreshed local campaign provenance.
affects: [NMI-01, NMI-02, KEEP-02, KEEP-03, 20-08, 20-09, 20-10]
actuals:
  tokens: 5653
  tasks: 2
  commits: 3
tech-stack:
  added: []
  patterns:
    - Independent PPU coordinate witnesses precede one production bus read per register alias.
    - Local campaign evidence keeps exact target keys and protected-main PASS authority separate.
key-files:
  created: []
  modified:
    - tests/ppu/test_nmi_races.c
    - README.md
    - include/nesturbator.h
    - tests/accuracy/baseline-metadata.txt
    - tests/accuracy/measurements.md
    - .planning/phases/20-nmi-line-and-vblank-flag/20-REVIEW-DISPOSITION.md
    - .planning/phases/20-nmi-line-and-vblank-flag/20-EVIDENCE.md
key-decisions:
  - "Keep the finite-margin split sample explicit: status bit 7 is sampled at M2 rise and sprite bits at M2 fall; this test makes no physical exact-tie claim."
  - "Resolve WR-01 and WR-02 with local code and test evidence while keeping all four Phase 20 requirement IDs pending for independent phase-level re-verification."
  - "Record the campaign as source_dirty=1 and retain hosted exact-head, protected-PASS, cross-platform, and performance work as outstanding."
patterns-established:
  - "A constructed sprite-evaluation state can witness a status transition independently at +9, +12, and +23 ticks before testing the production decoder."
  - "A wrong-sampler control must fail only at the returned-status assertion, not setup, timing guards, or unrelated checks."
requirements-completed: []
requirements-pending: [NMI-01, NMI-02, KEEP-02, KEEP-03]
coverage:
  - id: D1
    description: "The owned PPUSTATUS test proves a sprite-overflow bit changes between M2 rise and fall through ordinary and mirrored decoder addresses."
    requirement: NMI-01
    verification:
      - kind: unit
        ref: "ppu.nmi_races: test_sprite_status_changes_between_m2_samples; 0:130 clear, 0:131 set, $2002 and $3FFA return 0x35"
        status: pass
    human_judgment: true
    rationale: "NMI-01 remains pending for independent phase-level re-verification."
  - id: D2
    description: "Public comments distinguish the live NMI line from retained captured-request history and describe the split $2002 sample."
    requirement: NMI-02
    verification:
      - kind: integration
        ref: "README.md and include/nesturbator.h; cmake --workflow --preset ci passed 683/683"
        status: pass
    human_judgment: true
    rationale: "NMI-02 remains pending for independent phase-level re-verification."
  - id: D3
    description: "The local campaign record names the six passing target keys, protects the original 108 PASS keys, and preserves dirty-source provenance."
    requirement: KEEP-02
    verification:
      - kind: integration
        ref: "AccuracyCoin campaign, exact-six gate, protected observation, 108-key no-lost-pass, and runner.write_hashes.content"
        status: pass
    human_judgment: true
    rationale: "Hosted exact-head and six-platform evidence, protected-main authority, and performance evidence remain pending."
duration: 27min
completed: 2026-10-27
status: complete
plan_head_before: 42e5ba7fd0fa6a453455e096060fb94186902515
plan_head_after: 25044e03f5f9956a4dcba92a47fc7890084ca749
commits: 3
---

# Phase 20 Plan 07: Split Sprite Status Sampling Summary

**A dot-by-dot PPU witness proves `$2002` and `$3FFA` return sprite overflow from M2 fall while preserving the earlier VBlank sample.**

## Performance

- **Duration:** 27 minutes
- **Started:** 2026-10-27T03:18:59Z
- **Completed:** 2026-10-27T03:46:18Z
- **Tasks:** 2
- **Files modified:** 7 plan implementation/evidence files, excluding this summary and sequential state artifacts

## Accomplishments

- Added an independent PPU progression witness from scanline 0 dot 129: bit 5 stays clear at dot 130 after +9 M2-rise and +12 phi2-open advancement, then sets at dot 131 before +24 M2 fall. Production reads through `$2002` and mirror `$3FFA` each return `0x35` in one 24-tick CPU cycle, with open-bus, I/O-bus, address-latch, and age checks. Eighth-slot and out-of-range controls remain clear.
- Temporarily supplied the M2-rise sprite flags while preserving early bit 7, the late read side effects, and the one-cycle timing. The wrong model failed only at `value == 0x35u` for both aliases (`0x15 != 0x35`); production `src/bus.c` was restored before ordinary verification.
- Corrected the README and public-header NMI contract, closed WR-01 and WR-02 with source-linked evidence, and refreshed measured AccuracyCoin metadata without changing the pinned ROM, report, scoreboard, or hash inventory.
- The single full-enabled campaign completed 22/22 pages and 146/146 scored rows (114 pass, 32 fail, no skip/not-run/timeout, 5 DRAW). All six exact target keys passed; protected observation, verdict, the original 108-key no-lost-pass check, and hash-content comparison passed. Metadata records source revision `af268f348619`, `source_dirty=1`, fingerprint `152511afaec8cc129d7259eb49e5a72a11fb0e94adf9ed1757fc9526f0e77d84`, and report SHA-256 `29ea119d0e0b4db5ca47af4eba2e063413641135d070ac1971fc9719075cf7ac`.
- After correcting a metadata reason string rejected by the protected metadata parser, `cmake --workflow --preset ci` passed 683/683 and packaged the library, runner, and libretro adapter. The AccuracyCoin full option is OFF afterward.

## Task Commits

1. **Task 1: Add changing sprite-status decoder witness and correct public comments** — `af268f3` (`test`).
2. **Task 2: Prove wrong sampler rejection and refresh local evidence** — `6933291` (`docs`).
3. **Follow-up: Cite sprite-evaluation hardware source** — `25044e0` (`docs`).

The plan metadata commit follows the sequential state update and is not included in the three commits measured above.

## Files Created/Modified

- `tests/ppu/test_nmi_races.c` — independent phase coordinates, ordinary/mirror reads, single-access invariants, and bounded controls.
- `README.md` and `include/nesturbator.h` — split sample and live-line versus captured-request comments.
- `tests/accuracy/baseline-metadata.txt` and `tests/accuracy/measurements.md` — measured local campaign identity and truthful dirty provenance.
- `20-REVIEW-DISPOSITION.md` and `20-EVIDENCE.md` — WR-01/02 closure evidence, verification history, and remaining limits.

## Decisions Made

- The constructed transition has finite margin and establishes only the modeled sample order; simultaneous physical CPU/PPU tie precedence remains unresolved.
- Keep NMI-01, NMI-02, KEEP-02, and KEEP-03 pending for independent Phase 20 re-verification. This plan does not claim hosted exact-head or cross-platform completion.
- Preserve the local dirty flag and the existing clean-room prohibition flag; neither is replaced with an inferred clean or manual sign-off result.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Use a parser-valid dirty-source explanation**
- **Found during:** Task 2 (wrong-sampler proof and evidence refresh)
- **Issue:** The first complete CI workflow rejected `source_dirty_reason` because its value contained `source_dirty=1`, and the protected metadata reader forbids `=` inside values.
- **Fix:** Rephrased it as “dirty bit 1” while retaining the measured dirty source revision and both preserved planning-file edits.
- **Files modified:** `tests/accuracy/baseline-metadata.txt`
- **Verification:** `accuracycoin.baseline_fetch` passed 1/1; the refreshed AccuracyCoin gate passed 8/8; the rerun `cmake --workflow --preset ci` passed 683/683.
- **Committed in:** `6933291` (Task 2 commit)

---

**Total deviations:** 1 auto-fixed (Rule 1 - Bug)
**Impact on plan:** The metadata parser issue was corrected within the evidence update; task scope and measured campaign outputs did not change.

## Issues Encountered

- The first AccuracyCoin gate invocation ran the newly measured campaign while activation still held the previous input fingerprint. The campaign, target, protected-PASS, and hash checks passed; activation passed after refreshing metadata and rerunning with the full option disabled, avoiding a second campaign.
- The first full CI workflow caught the metadata reason parser restriction described above. The targeted regression and second full workflow passed after the correction.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Plan 20-08 remains next; its equal-tick adjacency witness and Plan 20-09/20-10 work remain outstanding.
- The full phase remains in progress with all four requirements pending, hosted exact-head/cross-platform/performance evidence unresolved, and no physical exact-tie claim.
- Clean-room audit prohibition remains visibly flagged; no unmanifested ROM data was added.

---
*Phase: 20-nmi-line-and-vblank-flag*
*Completed: 2026-10-27*

## Self-Check: PASSED

SUMMARY, all seven implementation/evidence files, and commits `af268f3`, `6933291`, and `25044e0` were found on the current branch.
