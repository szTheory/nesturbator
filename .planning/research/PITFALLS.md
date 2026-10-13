# Pitfalls: v4 Measured NTSC accuracy and robustness

Research/cross-review completed and scope approved 2026-10-13.

| Pitfall | Prevention | Owning phase |
|---|---|---|
| Identical screenshots falsely prove exact advance | Absolute state anchors/global sequence constraints or a validated unique-frame fixture; negative controls include identical-image ambiguity | 16 |
| Blind command retries double progress or toggle pause twice | Confirm supported capabilities; retry only safely idempotent requests with bounded attributable completion; fail/restart unconfirmed actions | 16 |
| Parent directory names select special fixture behavior | Exact basename/content identity and adversarial path controls | 16 |
| New AccuracyCoin name encoding silently changes keys | Tested decoder, inventory validation and reviewed old-to-new protected-key migration with immutable pin/hash/licence | 17 |
| Score improvement is merely a changed denominator | Keep old-pin evidence; separate DRAW and gate passing keys within the same pin | 17 |
| A hang makes unexecuted tests look passed | Every inventory entry has explicit pass/fail/skip/not-run/timeout classification and retained diagnostic evidence | 17 |
| An all-pass objective becomes unbounded hardware work | Baseline is the committed outcome; choose named material fixes only afterward | 17 |
| New fuzz target mostly rejects files or duplicates state fuzz | Generate accepted profiles and measure new program execution reach/throughput before choosing campaigns | 18 |
| Guest loops/JAM are mistaken for native exploits or emulator hangs | Guest loops still produce frames; JAM is a defined stop; test actual library/adapter behavior and classify outcomes | 18 |
| Discovery silently expands into fixing all possible bugs | Finite corpus/campaign budgets; minimize confirmed failures; major scope requires amendment; no universal-safety claim | 18 |
| Cosmetic debt displaces milestone value | Limit tune-up to the three named Phase 13 weaknesses, MMC3 metadata and upkeep procedure | 16 |
| Rolling roadmap becomes speculative execution phases | Separate product ledger, readiness horizons, stable outcome IDs and explicit promotion | 16 / milestone boundaries |
| Shipped seeds are treated as unfinished work | Ledger records delivered/partial/recurring disposition; preserve original seeds during new-milestone | Milestone boundaries |

## Sources

[RetroArch network control](https://docs.libretro.com/development/retroarch/network-control-interface/), [LLVM libFuzzer](https://llvm.org/docs/LibFuzzer.html), [GitHub roadmap](https://github.com/github/roadmap), [Rust roadmaps](https://goals.rust-lang.org/2026/roadmaps.html), all accessed 2026-10-13. Detailed tradeoffs/unknowns and the adversarial pass remain in the three [scope reports](v4-scope/).
