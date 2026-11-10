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

## D-44 source and receipt controls

The historical AccuracyCoin baseline remains d4802809ff41 and is still
identified as dirty. The D-44 binding carries the pinned 56-path historical
input map and checks current checker hashes separately; this does not
independently authenticate the historical dirty tree or establish hardware
behavior. Compatibility remains ABI 1, behavior revision 9, state format 3
and ordinary-loader NTSC.

The focused input-fingerprint, activation and receipt controls run with
`NESTURBATOR_ACCURACYCOIN_FULL=OFF`; they do not invoke local acceptance or the
actual AccuracyCoin campaign. The ignored route driver is private to its
prepared worktree and is not shipped or available in a fresh checkout. Its
synthetic controls are capped at 180 seconds, saved-evidence inspection at 60
seconds, hosted intent admission at 180 seconds and each other one-shot
operation at 1800 seconds including cleanup and evidence persistence. Local
acceptance also retains its 1500-second workflow and 1700-second observer caps.
A failed one-shot stage remains spent; inspect its saved evidence instead of
rerunning it.
