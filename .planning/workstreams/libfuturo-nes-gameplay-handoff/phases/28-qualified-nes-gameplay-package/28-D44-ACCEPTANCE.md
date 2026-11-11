# Phase 28 Local Acceptance

This record binds the local package and moved-prefix gameplay evidence to source commit `ce80b9e20b55d2eea65b2ed82952416b9619296e` (tree `78155bf26d7360526c93138918494303237c783f`) on branch `agent-phase28-d43-evidence`. The implementation SHA-256 manifest is `83cafa1e8b12736bbba5ab5da728766fccf011296c98b51f4e7235e2f524127e`; it includes the receipt checker, all tracked source, package/configuration inputs, tests and admitted ROMs, plus the provider profile and gameplay recipe. Generated logs, package archives, this acceptance record, and the future published-receipt overlay are evidence outputs and are excluded from the implementation fingerprint.

## Local verification

- Bounded workflow: `cmake --workflow --preset ci` — passed (exit `0`, 434s, 2026-11-10T20:46:59Z to 2026-11-10T20:54:13Z; 1500-second timeout). Configure, build, test and package stages passed.
- Build/runtime identity: AppleClang 21.0.0.21000101 (`cc`), AppleClang 21.0.0.21000101 (`c++`), CMake `cmake version 4.4.3`, CTest `ctest version 4.4.3`, host `Darwin/arm64`, target `Darwin/arm64`, Release/Ninja, CTest preset `ci` with 4 jobs, warning-as-error enabled, full AccuracyCoin campaign enabled.
- Required CTest results: accuracycoin.campaign=passed;provider.profile=passed;provider.profile.controls=passed;install.gameplay=passed;provider.receipt.controls=passed. `provider.receipt.controls` took 2.85s in this full workflow, read from its exact CTest log. The full registered selection and fixture metadata are captured in `build/phase28-d43/local/ctest-discovery.json`; raw run output is `build/phase28-d43/local/LastTest.log`. The current full workflow includes its configured AccuracyCoin fixture; no separate older interrupted collector was invoked.
- Installed gameplay report: `build/ci/qualified-consumer-ac039cb66e99/gameplay.json` (`b89868547120284668bc59e867b46addb4d5f543e2e3934fbac55f6ebb7c6962`). The installed runtime profile check and three current package archive inventory checks passed. Archive names, sizes and SHA-256 values are in the identity.
- Provider profile `1de93648482c0dd3fcface6a2982633037faded7194dd55ad94425d2fb0652e1`, gameplay guide `edf72c31bf318600f9e5d4d5a0c3515e19a96b65be8953eebd79aa37d929c436`, admitted ROM `eca79b9d0b546e96c1bc73099e239539fb9c7773ff10a431bb3a9cd6763208ca`; ABI 1, behavior revision 9, state format 3.
- Evidence root: `build/phase28-d43/local`. Input manifest: `build/phase28-d43/local/inputs.sha256`. Working-tree status at freeze is preserved in the identity and `build/phase28-d43/local/working-tree-status.porcelain`; unrelated user-owned planning/coordination changes were excluded from the source fingerprint and left untouched.

## D-44 reset route

- Frozen base: `2e0f2057304f60bab9a06a085a3e9477f1da022b`; evidence root: `build/phase28-d43`; ledger: `build/phase28-d43/route.json`.
- Local identity: `.planning/workstreams/libfuturo-nes-gameplay-handoff/phases/28-qualified-nes-gameplay-package/28-D44-IDENTITY.json`; acceptance narrative: `.planning/workstreams/libfuturo-nes-gameplay-handoff/phases/28-qualified-nes-gameplay-package/28-D44-ACCEPTANCE.md`.
- Freeze command: `cmake -DSOURCE_DIR=. -DMODE=reset-preflight -DRESET_KIND=d44-evidence-v1 -DRESET_BASE=2e0f2057304f60bab9a06a085a3e9477f1da022b -DRESET_ROUTE=build/phase28-d43/route.json -P tests/cmake/provider_receipt.cmake`
- Published replay command on the immutable exact-tag release-source checkout, after transferring the D-44 identity and release evidence: `cmake -DSOURCE_DIR=. -DMODE=published -DACCEPTANCE=build/phase28-d43/release/identity.json -DRELEASE_EVIDENCE=build/phase28-d43/release/release-evidence.json -DTAG=<observed-tag> -DWORK=build/phase28-d43/published -P tests/cmake/provider_receipt.cmake`

## Separate release gates

- Local acceptance: **passed** for the exact source identity above.
- Exact-candidate hosted CI: **pending**; no hosted result is claimed.
- Resulting main CI: **pending**; no hosted result is claimed.
- Published receipt and PKG-01: **pending**; local archives do not satisfy publication acceptance.

The evidence-only acceptance commit is not part of the source package identity. Under this one-shot package reset, a change to code, profile, scenario, package or qualification inputs after freeze ends the route; no refreeze or repeated acceptance is authorized.

## Remaining release checklist

- publication: **pending**
- exact-candidate CI: **pending**
- main CI: **pending**
- PKG-01 remains pending. Local archives establish local qualification only.

The next authorized work is to triage related PRs, preserve the ownership of the unrelated MMC2/migration worktree, and submit the unchanged product candidate once for required CI. Successful resulting-main CI and the normal Release Please source/release gates remain required. Publication still needs checksum and attestation verification for all target archives, plus the one exact-tag downloaded native-package and rights replay. No historical interrupted campaign, failed route or capture is reopened. Downstream host/device acceptance remains consumer-owned.

Independent freeze/current/historical/driver bindings are retained in `28-D44-FREEZE.json`; independent saved-result inspection is `build/phase28-d43/plan26-local-independent.json`. All700 test results passed, including38 valid required-regex pass results; no actual test or gate was repeated to inspect that evidence.
