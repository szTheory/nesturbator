# Phase 15 close-out record

Date: 2026-10-13

Review means confirm and record (D-21). Nothing here was bumped by hand.

## Pins

Every `uses:` line in `.github/workflows/*.yml`, compared with the latest release found with `gh api repos/<owner>/<repo>/releases/latest` on 2026-10-13.

| Action | Pinned version | Pinned SHA | Latest release | Status | Source |
|---|---|---|---|---|---|
| actions/attest | v4.2.2 | 1e69f48acb82 | v4.2.2 (2026-08-04) | latest | https://github.com/actions/attest/releases |
| actions/checkout | v7.0.1 | 3d3c42e5aac5 | v7.0.1 (2026-07-20) | latest | https://github.com/actions/checkout/releases |
| actions/create-github-app-token | v3.2.0 | bcd2ba492189 | v3.3.0 (2026-10-12) | behind | https://github.com/actions/create-github-app-token/releases |
| actions/download-artifact | v8.0.2 | 9000827ccba6 | v8.0.2 (2026-10-07) | latest | https://github.com/actions/download-artifact/releases |
| actions/upload-artifact | v7.0.2 | cf430e030ddb | v7.0.2 (2026-10-07) | latest | https://github.com/actions/upload-artifact/releases |
| googleapis/release-please-action | v5.0.0 | 45996ed1f6d0 | v5.0.0 (2026-04-22) | latest | https://github.com/googleapis/release-please-action/releases |

Behind: `actions/create-github-app-token` only. v3.3.0 was published the day before this review. `gh pr list --author app/dependabot --state open` shows no open Dependabot pull request yet, so there is nothing for Plan 08 to merge now; Dependabot owns the bump and its pull request is merged when it appears. Nothing is hand-bumped.

## Runner labels

| Label | Status | Next retirement | Source |
|---|---|---|---|
| ubuntu-24.04 | current | none announced | https://github.com/actions/runner-images |
| ubuntu-24.04-arm | current | none announced | https://github.com/actions/runner-images |
| macos-15 | current | leaves at the macOS 27 GA | https://github.com/actions/runner-images |
| macos-15-intel | current, the last x86_64 image | available until August 2027 | https://github.com/actions/runner-images/issues/13046 |
| windows-2025 | current, Visual Studio 2026 | none announced | https://github.com/actions/runner-images |
| windows-11-arm | current, now on the Visual Studio 2026 image `windows-11-vs2026-arm64` | none announced | https://github.com/actions/runner-images/issues/14602 |

Evidence for `windows-11-arm`: our own run 38329002566, job 115074198221, printed `Image: windows-11-vs2026-arm64`, `Version: 20261004.176.1` and a `DevEnvDir` under `Visual Studio\18`. Issue 14602 is still open without a closing notice, so the job log is the completion evidence.

## AccuracyCoin

Pinned: `673ef550db296136d52229961e7d39366116882a` (`tests/roms/manifest.txt`). Upstream is 9 commits ahead (`gh api repos/100thCoin/AccuracyCoin/compare/673ef550...main --jq .ahead_by`). Compare: https://github.com/100thCoin/AccuracyCoin/compare/673ef550...main

Kept. A bump re-baselines the ROM sha256, the scoreboard and the hashes, which does not serve closing the milestone. The bump is backlog item Phase 999.2 in ROADMAP.md.

## Timeouts

Filled in by Plan 08.
