---
phase: 28-qualified-nes-gameplay-package
plan: "21"
subsystem: rights-and-release-receipt
status: complete
requires: [28-20]
provides: [Pinned DABG rights and base-API receipt, One-shot reset source/stage controls]
affects: [28-22, 28-23, 28-24]
requirements-completed: []
completed: 2026-11-10
---

# Phase 28 Plan 21: rights and source-bound release controls

The DABG notice matches the pinned upstream bytes, and offline receipt/source-scope controls pass on the base-compatible port. No real reset acceptance stage has started.

## Changes and evidence

- Task 1 `d8d9109` ports the pinned standalone notice and DABG notice-index addition, preserving the base notice text. Rights verification independently downloaded and compared the pinned upstream license: SHA-256 `06fa223afb2949c20e030910d7010af2100a9e42ffa3d3f3c1c0307ec2903fdc`. The source/ROM/notice/profile/guide identity check passed without retry. Its command supplies an owned WORK directory, corrected in the plan before execution because the existing interface requires one.
- The receipt retains live hosted identity, eighteen archives/checksums, source/subject attestation, exact native installation replay and schema-2 rights controls. Published runner verification now uses actual base-supported version and bounded 110-frame frame/audio hashes, checking complete output shape and digest lengths. A local runner probe passed. A first path lookup named the wrong build location and launched no runner; the corrected actual build target path passed.
- Task 2 `e7a4ef0` adds exclusive local/candidate/main reservation and source freeze records. Duplicate, failed and interrupted attempts stay spent. Source/manifest/scope/ancestry and clean input checks precede real admission; hosted records require attempt 1 and the admitted candidate head. Local mode owns the sole workflow. Parent wrappers preserve logs and terminal results. Synthetic control records cannot qualify actual source or publication.
- A follow-through manifest closure includes built tools, release configuration, rulesets/hooks and source-handling/check configuration omitted by the inherited donor selector. Its independent selector fixture and dirty-input classifier were updated consistently before freeze. No inherited fixed count or old hashes are accepted.
- Focused profile/receipt controls passed in under three seconds. The expanded receipt controls passed in about two seconds. They exercise real selector/scope/reservation/finalization helpers using owned synthetic Git fixtures, preserve existing callers' scratch and reject wrong rights/notice/archives/attestations/run/source identities. Source and existing protected fixture bytes stay unchanged.
- Configured `NESTURBATOR_ACCURACYCOIN_FULL=ON`, matching the unchanged hosted-CI test-selection command. The option registers the existing protected campaign; it changes no production/build flags or source. Fresh discovery includes every required package/profile/campaign test. No campaign was launched by discovery/configure.
- Logs are in ignored `build/phase28-d42-orchestrator`: plan21 rights, runner probe, controls, selector closure, required selection and discovery. Full source identity is reserved for plan 22 after code review; the earlier observed 422-path selector count was pre-closure and never frozen.

Donor commit `7a004f2a4f8833a23b820fbb834f295911561636` per-file blobs:

- `THIRD-PARTY-NOTICES.md`: `4274a701610a3a76a07e4b758b85c2f12cd750ce`
- `tests/roms/licenses/DABG-ZLIB-LICENSE.txt`: `f9519385010e6b6c28f9ed5921c6bef51afa39a0`
- `tests/cmake/provider_receipt.cmake`: `cee0e16803a45b887428fd00294dc79d23618692`
- `docs/releases.md`: `7b8eb766ef68d1c206decafb4530e2c15f8a762b`

## Next step and limits

Plan 22 first completes the bounded code review and required fixes, then freezes the only candidate and invokes local mode exactly once. No route/freeze record, full workflow, remote trigger or publication exists yet. Candidate/main allowances remain unused. No capture/APU work, baseline edit, source behavior change or new regional claim occurred. D-39 and historical stopped campaigns remain untouched; publication and downstream host/consumer acceptance remain pending actual proof.
