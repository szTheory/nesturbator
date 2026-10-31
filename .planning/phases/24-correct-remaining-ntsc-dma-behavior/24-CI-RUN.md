# Authorized local CI result and follow-through

The owner authorized one local full workflow on2026-10-31, with a39-minute ceiling and unchanged individual/page limits. It ran once and completed normally in444.60seconds; CTest time441.98seconds. It was not interrupted or timed out, and no campaign was restarted. The workflow returned8:704/707 tests passed; packaging did not execute after the test step failed. Source identity was05ed2b1, behavior13/format6, with pre-existing local edits recorded as dirty. The archived prior evidence and new full log/result are under ignored build/phase24-acceptance/20261031T223038Z; no private raw paths or bytes are committed here.

The current22-page AccuracyCoin campaign itself completed in4.97seconds, with all146 scored rows terminal and5 DRAW rows retained, no timeout and complete infrastructure provenance. Its raw report hash is aed1823fe23884583b0a25f7a5a827c3b6f51744f6c07fa0906617a34ba2dcc5; build input fingerprint2ae704c0a21b865ca861faf01b5399194c1fe411324906e85b184de3dcbef3d2/count56. This identifies the measured candidate, not later edits. Every119 previously passing current-pin key remained PASS in these observations. This is distinct from fresh protected-main authority and hosted qualification.

| Selected row | Fresh result |
|---|---|
| DMA Landing on Write | PASS01 |
| DMC DMA + OAM DMA | PASS01 |
| DMC DMA Bus Conflicts | FAIL0a |
| Explicit DMA Abort | FAIL0a |
| Implicit DMA Abort | FAIL12 |

Internal Data Bus and Open Bus also changed from FAIL to PASS. No expected status was edited to manufacture these results. Current canonical scoreboard and revision9 provenance remain historical: the candidate still fails three selected rows, so it is not a qualifying replacement baseline.

The full workflow test failures were:

- accuracycoin.input_fingerprint and accuracycoin.activation: correctly rejected stale historical baseline identity. Refresh only with attributable, qualifying complete observations; retain the captured result instead of relabeling it as current qualification.
- ppu.sprites: its second OAM alignment case moved only CPU/PPU timestamps while leaving the actual bus/APU phase unchanged. Advance one real owned bus read instead; its existing515/516-clock expectations and byte assertions then pass. This repairs the fixture history, not the emulator to match bad history.

The full-ROM failures exposed gaps in the bounded fixtures. Bus-conflict offset15 belongs to the sample-valued answer key, although it still acknowledges status/IRQ internally. The earlier owned oracle incorrectly retained internal status on a later deselected read. Explicit/implicit tests need the full16-offset CPU-guest sweeps, including write-deferral and the looping-control timing, not only isolated stop windows. Independent diagnosis is in24-ROM-DIVERGENCE-BUS.md,24-ROM-DIVERGENCE-EXPLICIT.md and24-ROM-DIVERGENCE-IMPLICIT.md.

The agent completed the subsequent owned regressions and causal repairs in behavior14 correction a12a79a, without another confirmation for ordinary work or the approved compatibility policy. See [correction evidence](24-04-CORRECTIONS.md). The single full workflow has been consumed; no additional campaign or hosted/push/PR action follows automatically. Required phase acceptance remains incomplete. This run is useful measured evidence, not a green workflow or proof of all selected rows.
