# Proposed control remedy and separate fresh route

Status: PROPOSED ONLY. No source fix, test replay or new route is approved or executed. D44 remains terminally failed at candidate CI; its successful local evidence remains valid for that frozen source.

## Recommendation

Approve the two narrow control fixes in [the unapplied patch](28-CONTROL-REMEDY.patch), prefreeze verification within the existing caps, and a separate fresh qualification route only after those controls are reviewed and verified. Keep the 15-second receipt-control limit, every other test limit and every one-shot stop. Do not retry PR65, edit its head, repair the D44 frozen source, raise a timeout or transplant emulator changes.

## What failed and what is known

| Gate | Observed result | Diagnosis and limit |
|---|---|---|
| Windows x64 and ARM64 required builds | `accuracycoin.input_fingerprint` failed the historical-manifest mutation control; the validator reported changed `CMakeLists.txt` before the intended pinned-aggregate rejection. Production activation passed; each lane's other683 tests passed. | High-confidence inference: the control hashes an LF string before `file(WRITE)`, while Windows text-mode output changes persisted newline bytes. The uploaded artifacts omit the actual fixture directory, so its physical bytes were not directly observed. |
| Intel Mac required build | `provider.receipt.controls` timed out after15.05seconds at the unchanged15second cap; the other695 tests passed. | The aggregate timeout is proven. No retained per-substep timings identify a unique bottleneck. Static source inspection identifies redundant fixture Git operations that can be removed without removing assertions; the resulting runtime is not yet measured. |

Required [CI42905550344](https://github.com/szTheory/nesturbator/actions/runs/42905550344) attempt1 failed in13m18s. [PR65](https://github.com/szTheory/nesturbator/pull/65) remains the exact admitted head; no main admission, merge or publication occurred. Non-required Nightly42905550111 finished successfully in16m55s before cancellation was needed; it does not qualify the failed required gate. No cancellation or rerun was issued.

The local full workflow passed all700 results in434seconds, including the fresh full AccuracyCoin report and three package components. Those facts are preserved; they do not replace the failed platform checks. See [the failure identity](28-D44-CI-FAILURE.json), [local summary](28-26-SUMMARY.md), and [review](28-D44-REVIEW.md).

The newline inference is grounded in the project's control source and primary sources: CMake's [write implementation](https://raw.githubusercontent.com/Kitware/CMake/v4.2.0/Source/cmFileCommand.cxx) opens a text stream, and Microsoft's [text-mode translation documentation](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fopen-wfopen?view=msvc-170) describes LF/CRLF conversion. CMake3.25 already supports explicit [file(CONFIGURE) newline styles](https://cmake.org/cmake/help/v3.25/command/file.html#writing).

## Concrete draft changes

1. In `accuracycoin_input_fingerprint_controls.cmake`, write explicit LF and CRLF fixture variants, hash each persisted file with `file(SHA256)`, then put that actual hash into the deliberately corrupted historical map. Both cases must reach the same existing pinned56-input aggregate rejection. Restore the original fixture file afterward. No production validator, hash normalization, baseline pin or expected rejection is weakened.
2. In `provider_receipt.cmake`, keep every protected-path mutation commit and actual scope rejection. Stage each restored prior file with the next case's existing add/commit, then make one final restoration commit before the registration controls. This replaces seven restoration add/commit pairs with one:12 fewer Git launches and6 fewer synthetic commits. There is no reset, cache, concurrency, dependency, removed rejection or timeout change. This is a candidate efficiency remedy, not a demonstrated Intel speedup.

The patch is draft-only, applicable by `git apply --check`, and unexecuted. Its SHA256 is `adb6b20ece1ce8851d23bd5cef70e9e0a50cddce7156e69476b0840b2fbc304d`. The actual frozen product files remain unchanged. Before any new freeze, validate both newline forms through the actual checker and retain useful before/after control timing and operation counts. If the bounded cleanup cannot establish credible headroom within15seconds, stop before spending another acceptance attempt; do not silently enlarge a cap or add hosted probes.

## Exact proposed authority

A new grant would permit only these two control fixes, minimal explicit new-route bookkeeping in the existing receipt/ignored driver, necessary README/testing/release documentation, and selected workstream evidence. It would not authorize emulator/public-header/API/build-flag/preset/workflow/test-registration/ROM/collector/baseline/result/threshold/provenance changes or a new dependency. The historical/current fingerprint algorithm and binding record remain unchanged; a fresh complete provider manifest must identify the changed control bytes.

- Base: exact failed candidate product head `d8155bee6ea7dfd59aa3ff2ad61e60d1ed26dfa3` with its current product bytes; preserve later evidence-only disposition separately.
- Exclusively create `build/phase28-d45-candidate`, branch `agent-phase28-d45-controls`, with its own ignored `build/phase28-d45` and new D45 identity/acceptance/release evidence paths. Use the installed recorded adapter. Existing/symlink/busy paths require ownership resolution, never deletion, overwrite or substitution.
- A transport-only sender at absent `build/phase28-d45-transport` may push the identical admitted commit/tree after verifying all inputs and running unchanged hooks on every actual outgoing commit. It is never a second candidate or build/test/ledger root. Retain the D44 sender and all other worktrees.
- Prefreeze focused/synthetic checks must exclude actual campaigns and use the existing fixture closure and test caps. No tool installation, standalone campaign, ACT/Cachegrind capture or new CI lane.
- After reviewed prefreeze success: one new freeze, one full local workflow including its configured fresh AccuracyCoin campaign, one new exact-head candidate required CI attempt, one resulting implementation-main required CI attempt, then only the normal successful release-PR/source/publication and downloaded native/rights gates. Preserve actual automatic workflow identities separately.
- Four local workers; workflow child1500seconds, observer1700seconds, outer1800seconds. Every hosted/full workflow also has the30minute wall ceiling, with all stricter fixture/test/download/native limits retained.
- Any actual identity/comparator/test/acceptance/CI gate failure, interruption or timeout ends that new route without repair, retry, second head/candidate, refreeze, merge or publication. D44 and every earlier failed/interrupted route remain stopped. MMC2, APU/capture and the older unknown campaign are not reopened.

This is additional source and acceptance authority, not an acknowledgment to keep D44 stopped. No work under this proposed grant starts before an explicit decision. If approved, autonomy should carry the new selected route through its normal steps without per-step acknowledgments.

## Direction check

Read current product direction, the recurring CI-maintenance seed and independent MMC2 state. The provider handoff remains committed and is now blocked by narrow test controls, so this maintenance is the smallest supported next scope. MMC2 is independently implemented but remains stopped at its own full-CI gate; this remedy does not authorize or prove its retry. Larger mapper/hardware or core-speed work remains a separately proposed outcome. LibFuturo's host/device and installed-consumer acceptance remain consumer-owned.

## Next decision

The owner acts next by answering the concrete approval question in chat. Bounded follow-up review is complete; no manual discussion/planning/execution commands are needed. After approval, the autonomous assistant records the exact new scope and drives its steps and gates. Fresh-context entry is `$gsd-autonomous --ws libfuturo-nes-gameplay-handoff --from 28`; selected STATE links the current resume and saved decision. Context can be cleared. Source and acceptance work remain stopped until an explicit answer.

## Independent conceptual review

The bounded read-only reviewer found both hunks conceptually sound: LF/CRLF cases hash actual persisted bytes and preserve the exact aggregate rejection; the Git-fixture change preserves each per-path scope assertion and restores the clean fixture before registration controls. It is a justified low-risk timeout candidate, not runtime proof. The reviewer corrected the initial static operation count: seven protected paths mean28 explicit add/commit launches and14 synthetic commits before,16 launches and8 commits after; the net reduction is12 launches and6 commits. The patch remains unapplied and untested, and D44 remains terminal.
