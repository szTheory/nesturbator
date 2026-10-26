# Feature Landscape: v4 Measured NTSC accuracy and robustness

Scope approved 2026-10-13. Existing board families, battery saves, states, rewind/run-ahead, signal shutdown and performance gates remain credited.

## Committed outcomes

| Category | User-visible outcome | Requirements |
|---|---|---|
| Host evidence | State/progression checks reject known wrong sequences, command outcomes are attributable, exact fixtures are selected and MMC3 metadata is truthful | EVID-01 through EVID-04 |
| Conformance | The runner inventories the pinned test suite and exposes complete bounded observations with explicit unresolved classifications | CONF-01 through CONF-04 |
| Execution robustness | Accepted programs and lifecycle transitions receive deterministic replay and bounded sanitizer coverage, with documented loop/JAM outcomes | ROB-01 through ROB-04 |
| Direction | Future priorities remain visible and are refreshed within each milestone boundary | DIR-01 and DIR-02 |

## What is not a claim

Seventeen currently protected results are selected observations, not a full score. A complete inventory can include explicit timeout/not-run outcomes; a prerequisite hang must not be presented as downstream execution. A finite fuzz campaign with no findings does not prove universal safety. Existing state fuzz already executes continuations; the new target must demonstrate added program-execution reach.

## Following direction

Named gameplay-relevant NTSC fixes follow measured failures. Otherwise PAL is the preferred compatibility milestone, with Dendy independently scoped. Board additions, host preflight/real-host pilot and speed require concrete demand or measurement. Expansion audio, MMC5, FDS and peripherals remain separate long-horizon outcomes in [PRODUCT-ROADMAP](../PRODUCT-ROADMAP.md).

## Exclusions and seed portions

No all-pass accuracy promise, PAL/Dendy implementation, new board family, expansion audio/FDS/peripheral, new ABI/achievement/netplay, broad real-host OS rollout or cosmetic debt sweep in v4. Select the baseline portion of SEED-003, remaining execution-robustness portion of SEED-004 and recurring bounded SEED-006. Preserve originals; delivered board/state/performance seed outcomes are historical evidence.

## Sources

[AccuracyCoin pinned README](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/README.md), [libretro core contract](https://docs.libretro.com/development/cores/developing-cores/); accessed 2026-10-13. See [accuracy](v4-scope/accuracy.md), [reliability](v4-scope/reliability.md) and [governance](v4-scope/governance.md) for tradeoffs and source tiers.
