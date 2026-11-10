# Package qualification checks

`install.consumer` exercises the installed public C target. `install.gameplay`
builds a fresh consumer, moves its library prefix into a path with spaces,
discards the initial build, and checks include/library resolution and actual
compile/link provenance from the moved prefix. It runs the pinned
[DABG scenario](nrom-gameplay.md), completed no-input/Start-only controls,
refused-load twin continuation and buffer/frame-input checks.

`provider.profile` compares the machine-readable profile with actual public
identity, source arithmetic and source-mapped scenario/ROM digests.
`provider.profile.controls` rejects altered identity, unsupported region
fields, JSON type/array/recipe errors and runtime-report mismatches. Synthetic
reports test rejection only; the actual relocated consumer supplies runtime
evidence. An explicit contract invocation can read that fresh report.

The existing ci workflow includes these registrations without changing
production flags, campaign inputs or comparator thresholds. The package-only
reset has one full local workflow, one exact-candidate required CI and one
resulting-main CI; a failed acceptance gate is terminal without retry.
Publication requires separate live release and downloaded-artifact proof.
No current success restarts the older stopped acceptance campaign.
