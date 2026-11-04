# Phase 28 Local Acceptance

This record binds the local package and moved-prefix gameplay evidence to source commit `f36bf88d85f33be70e1fe48f105bb73ff0251856` (tree `21c6324839b53333b3d9950a7d7d73246998a5c9`) on branch `agent-28-04-phase28-release`. The implementation SHA-256 manifest is `7c93d8edea43d0fc6d5f6b61366e9b4a703af0af26f384ea16293f50be5a735d`; it includes the receipt checker, all tracked source, package/configuration inputs, tests and admitted ROMs, plus the provider profile and gameplay recipe. Generated logs, package archives, this acceptance record, and the future published-receipt overlay are evidence outputs and are excluded from the implementation fingerprint.

## Local verification

- Bounded workflow: `cmake --workflow --preset ci` — passed (exit `0`, 450s, 2026-11-03T23:36:27Z to 2026-11-03T23:43:57Z; 1500-second timeout). Configure, build, test and package stages passed.
- Build/runtime identity: AppleClang 21.0.0.21000101 (`cc`), AppleClang 21.0.0.21000101 (`c++`), CMake `cmake version 4.4.3`, CTest `ctest version 4.4.3`, host `Darwin/arm64`, target `Darwin/arm64`, Release/Ninja, CTest preset `ci` with 4 jobs, warning-as-error enabled, full AccuracyCoin campaign enabled.
- Required CTest results: accuracycoin.campaign=passed;install.gameplay=passed;provider.profile.controls=passed;provider.profile=passed;provider.receipt.controls=passed. `provider.receipt.controls` took 1.11s in this full workflow, read from its exact CTest log. The full registered selection and fixture metadata are captured in `build/phase28-local-requal-final-2/ctest-discovery.json`; raw run output is `build/phase28-local-requal-final-2/LastTest.log`. The current full workflow includes its configured AccuracyCoin fixture; no separate older interrupted collector was invoked.
- Installed gameplay report: `build/ci/qualified-consumer-d2eead93216b/gameplay.json` (`75590644017fb3296d6cc74dab8abaf6f09b6e8983963a1e430d8a82795acd6a`). The installed runtime profile check and three current package archive inventory checks passed. Archive names, sizes and SHA-256 values are in the identity.
- Provider profile `2ec6a760bb5781a54bc58b8c342a040a4deb71539287c779c5bcff2ec19228c2`, gameplay guide `294370c1a13b61ca76b5b4ebf12c288567ffa424b5c65562f44d3c6a1f1b96cd`, admitted ROM `eca79b9d0b546e96c1bc73099e239539fb9c7773ff10a431bb3a9cd6763208ca`; ABI 1, behavior revision 17, state format 6.
- Evidence root: `build/phase28-local-requal-final-2`. Input manifest: `build/phase28-local-requal-final-2/inputs.sha256`. Working-tree status at freeze is preserved in the identity and `build/phase28-local-requal-final-2/working-tree-status.porcelain`; unrelated user-owned planning/coordination changes were excluded from the source fingerprint and left untouched.

## Separate release gates

- Local acceptance: **passed** for the exact source identity above.
- Exact-candidate hosted CI: **pending**; no hosted result is claimed.
- Resulting main CI: **pending**; no hosted result is claimed.
- Published receipt and PKG-01: **pending**; local archives do not satisfy publication acceptance.

The evidence-only acceptance commit is not part of the source package identity. If code, profile, scenario, package or qualification inputs change, refreeze the identity and rerun the affected proof on that exact source.

## Superseded local identity

The earlier local acceptance remains preserved in Git at source commit `08a0d1ed2cf738c428e81dead9407b9b695a8608`, tree `51e8787f052cb5658dacff3ab49445b9f0430b33`, with input manifest `669bb6f3c9cfd6b60f11d6e9e08410445f37fe21f37f276f151613baaaad4cf6`. This acceptance supersedes it because the hosted receipt checker now gives the candidate PR, tested merge, resulting implementation-main commit, release PR, and Release/push source separate identities. The updated checker and the README troubleshooting link required by the merged source tests are included in the new manifest above.

The configured AccuracyCoin fixture in this local workflow passed. The previously interrupted separate campaign remains stopped with unknown status; this run did not resume it.
