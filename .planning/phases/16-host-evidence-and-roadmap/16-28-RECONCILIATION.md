# Plan 16-28 pre-clock reconciliation

This record preserves the immutable Plan26 incomplete boundary, the Plan27 pre-clock dispatch failure, and the accepted Plan28 dispatch/preflight evidence. It does not claim a Plan28 runtime root, clock, source correction, formal allocation, application acceptance, review, or parent-gate result.

## Dispatch binding

- Actual task and parent: `/root/execute_16_28` from runtime NEW_TASK metadata; Sender `/root`.
- Opaque invocation ID, actual model, and actual effort remain null because those values were not exposed. Requested model/effort remain separate intent fields and are not used as observations.
- Dispatch intent SHA256: `51fe2986e3731c5d81c0964b4ba9bbe3be071977b71910ab275044d878a8add4`; exact prompt SHA256: `fca0d906b5bb7a240a4e05878de67a7f012db08f5daf8a2219a8eacf646280d4`; actual result SHA256: `281fe8b6c8018f1f91d327dc3c099c2afabb70a9193e6efd2e23a1143e55a8b4`.
- Foreground violation observed: `false`; source branch/HEAD/index: `gsd/phase-16-host-evidence-and-roadmap` / `c41b72491f7149111ddeafcdd9ee4ba96096ac80` / `d86edc90659c699cb31d5e192b5aebc775c6feeeb9fc9dc7caaf12f3a1a0da0d`.

## Preserved historical boundaries

Plan26 remains active and `terminal=false` at its original start `69651537405833`, deadline `76851537405833`, and boot UUID `2493cd26-ed27-49e3-ba91-bc1afadea884`. Its operation ledger has 143 rows and 19 completed action result records. The pending diagnosis remains `d06ef81eb0afeb806dd69bbe2bcd1f0b0cc9eccf0c41723be1223a0a6ca6a31e` with the exact result and stream digests in the retained budget/result records. The new formal-history attestation inventories those action results and stream hashes without opening the failed stderr for acceptance. Plan26 is administratively `abandoned-incomplete`; its runtime metadata, deadline and pending diagnosis are unchanged. The historical raw snapshot still contains 1212 entries: {'changed': 0, 'matched': 1212, 'missing': 0}.

Plan27 remains a pre-clock failure. Its preserved checkpoint SHA256 is `d0134a904442e8f1f342b6bae3a24b1d592742888947db06731a06cb0d849249`; its original plan, continuation and checker hashes are `{'16-27-PLAN.md': 'c6c9e7e7d48c43dc91f8230060dc115fedbef34077d819335de16456e547905c', '16-27-CONTINUATION.md': '80b55b4324a6442c20be4ad45b6f8254c58a35bb871ffcce9c88daf1b42b191b', '16-27-CHECK.md': '1e4f07965823a3786243811567da99f8f0ee49e04e6b2c52f52fc2d24133edc8', '16-27-PRE-CLOCK-CHECKPOINT.md': 'd0134a904442e8f1f342b6bae3a24b1d592742888947db06731a06cb0d849249'}`. There is no `build/phase16-validation27` runtime root, initialized clock, source correction, formal consumption, or application acceptance. Its 47/47 scratch inventory and one stdout stream remain partial observations, not Plan28 acceptance.

## Accepted Plan28 pre-clock proof

- Successor launcher SHA256: `795beaf84d0ae2a6d535976b4b84cacea60e9029f63d40e1722ce4f3cecafc45`; lineage retains the Plan26 helper `e04a6738a9df55edc2ae641f4464c0019ccc0b3f4c18cb05c1e9acd0ae9e234d` and Plan27 helper `3f1dcd469bc81f2f74cc4e4de74da1161c2efbe6d0bc8c3c5389cbc3ad9c15b2`. The exact Plan26-to-Plan28 comparison patch is `b1423319c656c43b1aba975d59bf3ae600c7cd81d271fc30d56c8ae7d852d084` (85089 bytes).
- Scratch preflight: all 55 cases passed; no application tests or boot query ran inside the scratch matrix.
- Actual elevated boot access: Darwin `sysctlbyname:kern.bootsessionuuid`, UUID `2493cd26-ed27-49e3-ba91-bc1afadea884`, size/value query return codes `0` / `0`.
- Actual caller-memory matrix: 6 ordinary positive reads, 12 no-byte refusals, 9 binary/empty/boundary reads reconstructing 8448 bytes, and 2 source-mutating exit17 reads. Receipt SHA256: `53f9d94a6925915c8381cdc9b4a8faaaea41ff8482dbc2f868f91d920b3ccc82`.
- Current source-manifest identity: `5a52b2be59606d09511add59f96829f59515b81d479a41a1d11b859fa598a99b`; immutable source-manifest file SHA256: `1b8a8ff912392c78993372a7dba74fa29268dcdef27cca8aa19212127537818f`. All ten source inputs and the protected configuration still match the dispatch-frozen hashes.
- Six formal identities were transferred once to `build/phase16-validation28/formal`; every row remains `allocated-once-unused`. No Plan26 or Plan27 allowance, correction, pair, application test, or formal consumption is reused.

The sole initializer has not run. The next authorized runtime action remains the exact Plan28 `--initialize` call, followed immediately by its first elevated `--observe` on the new development root. All parent gates and Plan07/08 native, consent, and release evidence remain pending and outside this plan.
