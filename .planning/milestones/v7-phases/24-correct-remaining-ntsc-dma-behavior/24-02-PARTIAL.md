# Plan 24-02 partial execution — native bus-latch checkpoint

**Not a completed plan summary.** Task 1 and the collision-specific native part of Task 3 are committed in `8828225`. Task 2's split-latch correction and its native continuation are unimplemented pending the concrete decision in [24-02-CHECKPOINT.md](24-02-CHECKPOINT.md). No complete 24-02-SUMMARY.md exists; Plan 24-03 remains dependent on this incomplete plan.

## Completed correction

DMC halt/dummy/alignment overlap OAM GET/PUT using local synchronous stages. Only actual DMC GET steals an OAM source slot; OAM retries that byte on GET. All256 bytes, fixed per-case +2/+1/+3 duration, collision/resume indices, sample address/data/phase, original CPU-read resumption and normal/observed source equality pass. Compiling both bus copies from pre-OAM source rejects the regression. Format stays4 with unchanged saved fields/meaning; behavior revision11 and all required current-version consumers are committed. The twelve six-board state digests differ only by behavior tag and CRC, proven by normalizing to the exact prior hashes; sizes/ticks/payloads and other hash rows are unchanged.

Native continuation reaches silent frame1, pending-DMC frame17 and active-DMC/OAM-pending frame45 from the real owned guest. Loaded and uninterrupted outputs/transitions/effects/final bytes match. A skipped pending OAM changes continuation. Local guest gap G=6 restores all original 60-frame short straddle/fork checks on NROM/MMC3 without changing limits; the prior timing failed both OAM witnesses. Details are in the amendment and ledger.

## Verification

- Default bus/APU/API/mapper, C/C++ header, state format pin, normalized API policy and mutation checks passed on the final code.
- Existing docs.reproduction and mutation checks passed under unchanged limits, after current-version text was updated.
- core.state_fork --dma and --short --stress with the committed manifest-listed nesteroids prefill passed, preserving both boards' OAM/DMC/IRQ witnesses.
- clang-format18, whitespace and normal git hygiene checks passed.
- --dma-trace conflict-latches intentionally exits1: current status20/20, external20, following controller21; expected00/00, externalA5, controllerA1. This explicit diagnostic is not part of default regression until its correction is implemented.
- No full CI/AccuracyCoin ROM/campaign, push or PR ran. Required fresh pin/protected/hash/performance evidence remains absent.

The OAM [review](24-02-REVIEW.md) found no production blocker. Its timing-oracle warning is resolved with fixed source-shaped expectations; its stale format quote was withdrawn after inspecting the already-correct current guide. The evidence ledger retains separate baseline and corrected observations.

## Resume precisely

The intentional production-commit-without-complete-SUMMARY state is this designed dynamic checkpoint, not an orphan/duplicate execution. Reconcile this partial record, the reachable OAM commit and STATE first. After the owner decision, continue inline at Task2; do not rerun/recreate Task1 or mark Plan24-02 complete prematurely. Complete both-latch native continuation before a coherent split-bus correction commit; retain all existing guards. Only after all plan tasks pass may a complete SUMMARY be written.

Current actual native contract: format4, behavior11, ABI1. Proposed next contract: new serialized cpu_data byte, explicitly external open_bus meaning, format5 and behavior12. No new field, meaning change or format5 loader is implemented. Approval of this proposal would not authorize new abort fields or clear the campaign stop.
