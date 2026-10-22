# Plan 44 — terminal prewrite feasibility stop

Recorded: 2026-10-22T00:45:00Z. Invocation: `gsd-execute-phase 16 --gaps-only`.
Input HEAD: `a63404b2809fe1788dbb932f5804aaba8e9d1ef2`.

Plan44 stops incomplete, with 0/2 tasks accepted, before bootstrap creation
or executor dispatch. This is a measured incompatibility between retained
bootstrap duties and the new decoded-result limit. It is not a failed durable
A/B call, an emulator defect or a passing prospective transport control.

## Measured first-write obstacle

The normative44AMENDMENT requires the full bytes of every proposed write in
the lossless result, caps each decoded result closure at67,108,864bytes, and
requires the actual planned first bootstrap/header A/B shapes to fit before
any first write. Compression changes encoded size, not decoded size.

Read-only DATA inspection of the retained40parent bootstrap creation receipt
found that its frozen-bootstrap publication operation includes the historical
snapshot, alongside prompt, prior request, execution context and dispatch
intent. Its snapshot was121,067,061bytes. No retained request was executed.
44retains that bootstrap/snapshot protocol; no sharded snapshot or append
protocol was defined, proved or authorized by this execution.

A fresh read-only streaming measurement used that retained inventory schema:
sorted os.walk entries; path/mode/size/mtime_ns/inode; symlink target or raw
file SHA256; metadata-only treatment of files without read permission bits;
canonical sorted-key compact UTF8 JSON array plus final LF. Prospective44/45
roots were excluded and separately verified absent. No historical helper was
executed or imported. The command did not retain or write the inventory bytes.

| Measurement | Actual value |
|---|---:|
| Inventory entries | 441632 |
| Raw file hashes | 357221 |
| Metadata-only files | 4 |
| Changed observed metadata during each entry read | 0 |
| Exact serialized inventory length | 121071262 bytes |
| Decoded closure limit | 67108864 bytes |
| Excess from inventory alone | 53962398 bytes |

Inventory SHA256:
`72007a390220ef526fccffbc1bb84d3243ccb9a5a70ec275a029b3fa35dda14c`.
This is a lower bound on the required bootstrap result closure: prompt,
intent, journal and receipt bytes would add to it. It already exceeds the
limit, irrespective of the16384-byte producer and8192-byte caller emission
limits. Those encoded sizes were not computed or claimed to fit.

## Actual observations and their limits

The mandatory historical mode oracle ran and exited0:4positive/6negative
interpretation cases. It grants no fresh transport or write acceptance.

The inventory measurement request used max_output_tokens18000. Initial chunk
474496 returned running session62358; poll eaffbd remained running; terminal
poll43f6e7 returned exit0/original_token_count153 and the complete611-byte JSON
diagnostic, reason TRANSPORT_BOUND_EXCEEDED, authority=false, proposed_writes=[].
This was a successful measurement command reporting a non-fitting bound;
it was NOT the prospective A/B producer returning exit23. No fresh A/B
program, request builder, codec or literal control table was frozen or run.
No prospective exact-source, decoder, refusal or actual diagnostic control
receives acceptance credit. No actual truncation was induced or observed here.

The receiving model checked the delivered diagnostic framing and visible
metadata, then supplied that exact received text to a separate read-only
receiver. It compared the text with the actual retained terminal result and
verified the literal reason, no-authority outcome, arithmetic and end marker.
Receiver chunk2471aa exited0 with complete MEASURED_BOUND_NOT_FIT, explicitly
fresh_AB_program_defined=false and transport_control_acceptance=false.
The receiver's complete result was observed before administrative closeout.

| Observed subject | UTF8 length | SHA256 |
|---|---:|---|
| Measurement command | 1821 | `186dd8c6dc7bf01fbfa073719ccc461496e061e8c409188daf09199004cc75fb` |
| Actual terminal diagnostic output | 611 | `f89ec80bbd791e84aec3e674eb624442c7b8c8365f1841af27cc6a009555c74b` |

The request, initial result, terminal result and receiver result were retained
in caller store; the intervening nonterminal poll is exposed in the transcript,
not separately retained as an immutable stored receipt. No complete A/B
reception proof is claimed. Hidden provider fields remain unavailable. This
Markdown records actual diagnostics and identities, not a bootstrap receipt,
full inventory payload or reconstruction of any unavailable historical result.

## Preservation and stop

Fresh pre-closeout checks matched the input HEAD and raw index SHA256,
verified an empty index, and matched all six original dirty-path hashes with
every path unstaged. All twelve42–45bootstrap/preflight/runtime roots and all
44/45PREPARATION/SUMMARY artifacts remained absent. No file/root/receipt was
created before this administrative terminal closeout.

The streaming inventory is a single fresh measurement, not a complete
before/after historical comparison. No whole-history immutability proof is
claimed. Four metadata-only files were not read for content or chmodded.
26active terminal=false/pending diagnosis, earlier failures, UNKNOWNs and
four flagged assumptions remain unchanged.42's failed request was neither
reissued nor reconstructed.

No executor, helper import/repair/CLI RED/matrix/freeze/export, runtime clock,
boot query, allowance, application test, source/config edit, formal spend,
source commit/review, native CI, consent, publication or released-byte action
occurred. No begin-parent/runtime gate applies.45remains unexecuted0/3;
07/08remain blocked. Counts45physical/14live/10complete; verification stays
gaps_found. The two45scope advisories remain visible and unchanged.

44AMENDMENT requires refusal if the intended initial pair cannot fit and
states: "Any prewrite failure is terminal with zero new durable writes".
This measured feasibility stop occurs before constructing that pair. No
larger limit, omitted snapshot bytes, file-backed substitute, new snapshot
schema, root suffix, retry or dispatch is inferred. Only administrative
planning records are written after the stop; no44bootstrap is authorized.

Next: `$gsd-plan-phase 16 --gaps`. Review the actual inventory size against the
lossless first-write/decoded-closure contract before granting fresh authority.
Do not run another lifecycle command in this invocation or redispatch44from
a DAG-ready label. This record grants no successor or execution authority.
