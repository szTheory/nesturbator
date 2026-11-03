---
phase: 26-pal-dma-and-regional-state-continuation
plan: "02"
requirements-completed: [STATE-01]
---

# Phase 26 Plan 02: Regional state continuation

## Outcome

Native format-6 state identity and validation now follow the loaded effective
region while keeping the 64-byte header, ABI 1, and behavior revision 17.
Same-region PAL continuation preserves fractional audio history and reachable
DMC state; opposite-region and unknown-region loads fail before mutating the
instance. Libretro serialization remains fixed for loaded PAL content.

This is the final acceptance record for the combined Phase 26 candidate. Plan
01's PAL DMA model and its evidence boundaries are in
[26-01-SUMMARY.md](26-01-SUMMARY.md).

## Candidate and scope

- Source commit: `90d71731498d37ddb2db91fe19b6f7afd58e4742`
- Source tree: `1943d6da7d709e7a5b5033392248ba2258625c38`
- The full workflow ran on that exact source candidate. Documentation and phase
  closeout artifacts are added after it.
- Existing unrelated edits to planning configuration/continuity records and
  the untracked `docs/coordination/` directory were preserved and excluded
  from the candidate and phase commits.
- The configured local workflow ran within its 30-minute outer limit and kept
  existing fixture and per-test limits. No hosted workflow or publication was
  started.

## Regional continuation and refusal

The generated owned PAL NROM guest reaches a public frame boundary with
`dma_pending=1`, DMC halt phase 1, no load wait/enable delay/abort delay, and
nonzero `audio_rem=614339` and `sample_phase=624579`. A same-region fork
reproduces full pixels, PCM, tick/frame counters, and canonical state bytes
across the restored frame and three further frames. A history-loss control
that clears the pending request produces a different canonical state.

The loader uses denominator-minus-one remainder caps (NTSC 315000, PAL
709379), scanline caps 261/311, and requires the PPU/header tick gap to be
strictly less than the regional frame duration (714732/1063920). Accepted
regional boundary values were each loaded and advanced through a public frame.
Cross-region loads in both directions and unknown region values fail with the
cartridge identity error before body checks or application; instance, RAM,
save generation, and future output remain unchanged. Prior NTSC format-6 states
remain loadable, with behavior revision informational. The real libretro host
test covers fixed PAL state size through frames, reset, same-region restore,
failed cross-region restore, and preference changes without content reload.

The public boundary cannot save during an active synchronous DMA bus call, so
the fixture preserves and checks reachable in-flight request/phase fields at
the frame boundary; it does not claim mid-call serialization.

## Review and focused checks

The scoped code review reported no blockers and two warnings. Both were fixed:
the public contract now rejects an exact one-frame PPU/header tick gap, and
each accepted regional maximum is exercised by advancing a public frame. The
fixes passed `core.state_reject.regional`; documentation links passed. The
review record and disposition are [26-REVIEW.md](26-REVIEW.md) and
[26-REVIEW-DISPOSITION.md](26-REVIEW-DISPOSITION.md). A supplemental review of
the final CI corrections is recorded there as well.

After diagnosing the first full-workflow failure, focused retests passed all
six affected contracts: hosted evidence revision, AccuracyCoin input
fingerprint, interrupt entry/state observers, regional state rejection, and
documentation links. The final full workflow also passed the PAL fork,
regional refusal/boundary tests, and libretro host coverage.

## Full local workflow receipt

Command: `cmake --workflow --preset ci`

Final result on the exact candidate above: configure, build, all 714 unfiltered
CTest tests, and package creation passed. CTest reported 473.48 seconds; the
whole workflow took about eight minutes, well within the authorized 30-minute
limit. The serial RetroArch session-controls test took 283.33 seconds and
dominates the measured test time. Its acceptance coverage is unique, and no
safe reduction was identified; the configured test cap remains 900 seconds.
The workflow already runs CTest with four jobs. No extra verification run was
needed.

Packages produced by the workflow:

| Archive | SHA256 |
|---|---|
| `nesturbator-0.1.11-library-macos-arm64.zip` | `4f320b3fc97aa839aaf9f9e5e16d40db36e7a6cf9a4a8b0883f934611897925d` |
| `nesturbator-0.1.11-libretro-macos-arm64.zip` | `46444b1c57f735ba22d1ca851ffcb4b41713447cb92c1ec342c379dae3cddb9f` |
| `nesturbator-0.1.11-runner-macos-arm64.zip` | `fc5de619584553541892e4a42ba8cc7022b79ea930e8f1af01c6e8b54c897938` |

## Diagnosed attempts and campaign provenance

The first full workflow stopped at link time because the vector-bus test seam
did not expose the production SYNC-read observer used by CPU interrupt tests.
A test-only wrapper was added and focused CPU vector checks passed. The next
full run completed tests but found six stale or incomplete contracts: two
interrupt observers missed SYNC reads, the regional fixture compared
uninitialized output buffers, hosted evidence expected behavior revision 16,
and the AccuracyCoin build-input fingerprint was stale. Each cause was fixed;
the output buffers are initialized, both observer paths see SYNC reads, the
revision expectation is 17, and only the current build-input hash was updated.
Focused retests and the final workflow passed. These diagnoses and their fixes
are committed before the final candidate; no known failure remains in Phase 26.

The final configured AccuracyCoin campaign completed 22/22 pages and 146/146
scored rows (126 pass, 20 fail, 5 inventory-only draws, no skips or timeouts).
Its report digest is
`f3b5bbf7ddcf4d18deabda6d875b441330dc71c97b8aca5fdcc38bf471f98f6c`; the
canonical scoreboard SHA256 is
`9ec5d50d291d2713f5281851f642270498a4e8b0dab273b8b72cd19b66ff111b` and was
not changed. This is the configured campaign run under standing local-CI
authority, not a continuation or relabeling of the older interrupted campaign;
that historical run remains unknown.

## Remaining boundaries and next action

PAL DMA eligibility is implemented from the cited trace report and its
authors' SYNC interpretation. The generated wrong-model fixture demonstrates
sensitivity to that choice, but it is not direct hardware proof. Later held
no-op-cycle bus addresses/register effects, other unsettled PAL abort/reload
behavior, and direct project-run hardware comparison remain unverified. The
DMA-01 and STATE-01 unresolved spec-less rows and transparency prohibitions
remain flagged; format-6 compatibility does not assert identical historical
behavior or universal cross-region/cross-build qualification.

Phase 26 is ready for goal verification. The verifier must check both plan
truth sets, the required local workflow receipt, and the unresolved boundaries
without starting Phase 27. After verification passes, run the phase-complete
closeout and commit only Phase 26 artifacts and project state.
