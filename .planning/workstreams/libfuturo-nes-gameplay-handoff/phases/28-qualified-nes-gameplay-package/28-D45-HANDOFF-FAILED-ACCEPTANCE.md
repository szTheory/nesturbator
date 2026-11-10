# Phase 28 Local Acceptance

This record binds the local package and moved-prefix gameplay evidence to source commit `adbb84b79bc5bb70385db51f6acb0b3a424c0609` (tree `e9a76f2684a920e2cac0823895a0590e8d9211d2`) on branch `agent-phase28-d45-controls`. The implementation SHA-256 manifest is `65646381173d785de235efc66684f27e887a68d58fa5f1802d1dc3b11964f84d`; it includes the receipt checker, all tracked source, package/configuration inputs, tests and admitted ROMs, plus the provider profile and gameplay recipe. Generated logs, package archives, this acceptance record, and the future published-receipt overlay are evidence outputs and are excluded from the implementation fingerprint.

## Local verification

- Bounded workflow: `cmake --workflow --preset ci` — passed (exit `0`, 560s, 2026-11-10T23:34:16Z to 2026-11-10T23:43:36Z; 1500-second timeout). Configure, build, test and package stages passed.
- Build/runtime identity: AppleClang 21.0.0.21000101 (`cc`), AppleClang 21.0.0.21000101 (`c++`), CMake `cmake version 4.4.3`, CTest `ctest version 4.4.3`, host `Darwin/arm64`, target `Darwin/arm64`, Release/Ninja, CTest preset `ci` with 4 jobs, warning-as-error enabled, full AccuracyCoin campaign enabled.
- Required CTest results: accuracycoin.campaign=passed;provider.profile=passed;provider.profile.controls=passed;install.gameplay=passed;provider.receipt.controls=passed. `provider.receipt.controls` took 3.02s in this full workflow, read from its exact CTest log. The full registered selection and fixture metadata are captured in `build/phase28-d45/local/ctest-discovery.json`; raw run output is `build/phase28-d45/local/LastTest.log`. The current full workflow includes its configured AccuracyCoin fixture; no separate older interrupted collector was invoked.
- Installed gameplay report: `build/ci/qualified-consumer-477307e08947/gameplay.json` (`b89868547120284668bc59e867b46addb4d5f543e2e3934fbac55f6ebb7c6962`). The installed runtime profile check and three current package archive inventory checks passed. Archive names, sizes and SHA-256 values are in the identity.
- Provider profile `1de93648482c0dd3fcface6a2982633037faded7194dd55ad94425d2fb0652e1`, gameplay guide `edf72c31bf318600f9e5d4d5a0c3515e19a96b65be8953eebd79aa37d929c436`, admitted ROM `eca79b9d0b546e96c1bc73099e239539fb9c7773ff10a431bb3a9cd6763208ca`; ABI 1, behavior revision 9, state format 3.
- Evidence root: `build/phase28-d45/local`. Input manifest: `build/phase28-d45/local/inputs.sha256`. Working-tree status at freeze is preserved in the identity and `build/phase28-d45/local/working-tree-status.porcelain`; unrelated user-owned planning/coordination changes were excluded from the source fingerprint and left untouched.

## D-45 reset route

- Frozen base: `d8155bee6ea7dfd59aa3ff2ad61e60d1ed26dfa3`; evidence root: `build/phase28-d45`; ledger: `build/phase28-d45/route.json`.
- Local identity: `.planning/workstreams/libfuturo-nes-gameplay-handoff/phases/28-qualified-nes-gameplay-package/28-D45-IDENTITY.json`; acceptance narrative: `.planning/workstreams/libfuturo-nes-gameplay-handoff/phases/28-qualified-nes-gameplay-package/28-D45-ACCEPTANCE.md`.
- Freeze command: `cmake -DSOURCE_DIR=. -DMODE=reset-preflight -DRESET_KIND=d45-controls-v1 -DRESET_BASE=d8155bee6ea7dfd59aa3ff2ad61e60d1ed26dfa3 -DRESET_ROUTE=build/phase28-d45/route.json -DACCEPTANCE=.planning/workstreams/libfuturo-nes-gameplay-handoff/phases/28-qualified-nes-gameplay-package/28-D45-IDENTITY.json -DACCEPTANCE_MARKDOWN=.planning/workstreams/libfuturo-nes-gameplay-handoff/phases/28-qualified-nes-gameplay-package/28-D45-ACCEPTANCE.md -P tests/cmake/provider_receipt.cmake`
- Published replay command on the immutable exact-tag release-source checkout, after transferring the selected identity and release evidence: `cmake -DSOURCE_DIR=. -DMODE=published -DACCEPTANCE=build/phase28-d45/release/identity.json -DRELEASE_EVIDENCE=build/phase28-d45/release/release-evidence.json -DTAG=<observed-tag> -DWORK=build/phase28-d45/published -P tests/cmake/provider_receipt.cmake`

## Separate release gates

- Local acceptance: **passed** for the exact source identity above.
- Exact-candidate hosted CI: **pending**; no hosted result is claimed.
- Resulting main CI: **pending**; no hosted result is claimed.
- Published receipt and PKG-01: **pending**; local archives do not satisfy publication acceptance.

The evidence-only acceptance commit is not part of the source package identity. Under this one-shot package reset, a change to code, profile, scenario, package or qualification inputs after freeze ends the route; no refreeze or repeated acceptance is authorized.
