# Plan 46 — terminal parent prewrite reception stop

Recorded: 2026-10-22T02:31:28Z. Invocation: `gsd-execute-phase 16 --gaps-only`.
Input HEAD: `acef92a247773a6197409499330294b6716be981`.

Plan46 stops incomplete, with 0/2 tasks accepted, before bootstrap creation
or executor dispatch. The parent omitted the required explicit outer
`max_output_tokens: 18000` setting on its new snapshot calls. This is an
orchestration contract violation, not an emulator defect, actual truncation,
failed durable A/B or successful prospective refusal.

## Stop condition and actual provenance

The normative46AMENDMENT opening requires "both nested exec_command and outer
functions.exec max_output_tokens exactly18000" and says "Any failure before
first durable write leaves zero new durable bytes and stops." Its reception
contract also makes missing actual results terminal. No unchanged restart,
new root, repaired-and-hidden result or transfer of partial acceptance is
authorized.

The actual outer functions.exec requests that initialized capture, partition
and READ omitted the pragma that explicitly sets18000. The tool declaration
documents10000 as its default. The effective hidden provider/tokenizer budget
is unavailable; no claim is made that it was observed, that bytes were clipped,
or that a result exceeded18000tokens. Node requests expose no token-budget
parameter, as already documented in46AMENDMENT. This failure is the missing
required outer request setting, not that Node capability limitation.

On discovering the omission, the parent terminated running functions cell157.
The actual wait result was `Script terminated`. No second READ/COMPARE run
or bootstrap operation followed. Administrative calls then used an explicit
outer18000pragma; this does not repair the failed reception attempt.

The last retained complete receiving-store position is21READ packets,
covering112segments (indices0..111), offsets[0,458752). Their actual Node
requests/results and independently decoded bytes were retained in caller
memory. COMPARE position is0packets/0segments/0bytes. No full snapshot
reconstruction, whole-stream canonical parse or exact second-stream comparison
completed. These partial checks supply no contract/task acceptance.

A separate actual read-only administrative Node result reported producer
position READ49/COMPARE0. Thus the producer advanced beyond the last retained
receiving-store position. Complete actual envelopes for packets21..48 and any
in-flight result at termination are unavailable to this closeout; they are
not recovered from producer memory, expected packets, a digest or a rebuilt
request. Producer cursor49 is an observation of producer state, not proof of
49independently received results. The missing interval is preserved explicitly.
Source text for the terminated100-packet loop is visible in the transcript,
but its caller-store source key is unavailable after termination. No durable
receipt or archive exists for this attempt.

An administrative report-formatting call subsequently raised a TypeError when
hashing that unavailable loop-source key. Its Node readback had already been
retained. A later administrative call inspected that existing retained result
and reported the unavailable key honestly; it did not repeat READ/COMPARE or
recover missing envelopes. This additional formatting failure grants no proof.

## Fresh read-only observations, without acceptance

A persistent standard-library Node producer captured the current historical
build inventory once in memory, without a spool or file. It used the retained
sorted walk and canonical sorted-key compact UTF8 JSON array plus final LF;
integer inode/mtime_ns values were serialized exactly from bigint fields.
Readable files were lstat-checked before/after hashing; files without read bits
remained metadata-only. Fresh46/47roots were separately found absent.

| Observed measurement | Value |
|---|---:|
| Inventory entries | 441632 |
| Raw file hashes | 357221 |
| Metadata-only files | 4 |
| Canonical inventory bytes | 121071262 |
| Maximum raw segment bytes | 4096 |
| Bound segment count | 29559 |
| Bound packets per READ or COMPARE stream | 4207 |
| Planned actual READ+COMPARE results | 8414 |
| Largest precomputed producer packet, including LF | 14997 bytes |
| Last retained received READ bytes | 458752 |
| Independently compared snapshot bytes | 0 |

The captured inventory SHA256 is
`72007a390220ef526fccffbc1bb84d3243ccb9a5a70ec275a029b3fa35dda14c`.
It happens to equal44's historical measurement; this was a fresh capture,
not adoption of that historical measurement as a new snapshot.
The finite segment descriptor table SHA256 is
`68c135b9df18d117911096fac1acfbee712c68d40fbeb1c9bdd9e99d605397bd`.
These identities supplement observations; neither is full reception evidence.

One actual read-only fixed-deflate codec vector carrying19000rawUTF8bytes
of multibyte text passed independent decoding/byte comparison. The first
21READ packets passed local framing/identity/hash/ordered-coverage checks.
The producer packet precomputation covered READ/COMPARE encoding sizes only.
It did NOT prove the complete initial metadata/prompt/dispatch/header A+B,
receipt/archive/seal graph, all maximum-shape classes or negative controls.
No prospective operation program, journal encoder, expected-outcome table or
whole source/dispatch closure received full prewrite acceptance.

Actual source strings retained in caller memory have these identities:

| Source | UTF8 bytes | SHA256 |
|---|---:|---|
| Initial receiver | 514 | `71ed37f5ae637bf3e4d745962c7548f63a0429e269e5607cf2c7f78c766dfc0e` |
| Capture producer | 3807 | `b4dd58a58d9afd1b36269670b4541325cb3c633335e5eb11d91cfe84f141f6cb` |
| Independent byte/codec utilities | 6063 | `736bf5b317d8b602f077052457ec7613f08503c6a2aaf6cc20e42e3dfff813ca` |
| Segment producer/partition | 2801 | `ff3020b51efcac695953a3282b00a428428072fed17df8fb743c452258299013` |
| Independent segment receiver | 2634 | `ebc3834f9e26f5972cd34d002fd8e415a30b1d42342aca8331d7e1789d840384` |

Node tool results exposed TextContent and isError, not OS exits. No Node
logical result is described as an exec_command OS exit23. Hidden fields and
full outer wrapper sizes remain unavailable. Capture/source/actual-result
buffers are volatile caller/Node memory, not durable bootstrap artifacts;
this Markdown does not substitute for their full bytes or claim archival.

## Preservation, remaining gates and route

Before administrative closeout, read-only checks matched all six original
dirty-file SHA256 values and verified that every one remained unstaged.
Input HEAD matched exactly; the index was empty with raw index SHA256
`94a9add880778812259052e96f19288dca55d59441c1723f4c3d433d5ef808b9`.
All18execution roots42..47, each runtime/bootstrap/preflight variant, and
all12PREPARATION/SUMMARY artifacts42..47 were absent. The administrative
Node readback independently reported all six prospective46/47roots absent.
No file/root/receipt was created before the administrative records below.

The one inventory capture plus partial reception is not a complete
before/after historical byte comparison. No whole-history immutability proof
is claimed. Four unreadable files were not read for content or chmodded.
Historical failure bodies,26active terminal=false/pending diagnosis, every
UNKNOWN and four flagged assumptions remain unchanged.

The mandatory historical mode oracle, prospective mode/A-B/refusal/store/name/
quote/serialization/blob/segmented end-to-end controls, durable diagnostics,
complete bootstrap/archive/seal and prior dispatch binding did not complete.
No executor, header/A/B, import, helper repair, genuine copied34CLI RED,
matrix/rehearsal/freeze/export, PREPARATION/SUMMARY or task credit occurred.
No runtime clock/boot/allowance/application test, source/config correction,
formal spend, source commit/review, native CI, owner consent, publication or
released-byte action occurred. Only administrative planning records are
written after this terminal stop. Full CMake CI was not run for these records.

Plan47 remains unexecuted0/3;07/08 remain blocked on all existing acceptance,
parent/native/exact-head consent/publication/released-byte gates. Counts remain
47physical/14live/10complete and verification stays gaps_found. The two
current47scope advisories remain visible with no new approval or fit claim.
An index reporting46DAG-ready is not authority to resume this terminal attempt.

Next separate command: `$gsd-plan-phase 16 --gaps`. Planning must reconcile the
missing explicit outer request setting, interrupted reception and actual
missing-result interval before fresh authority. This record grants no new
limit, retry, successor, root, dispatch or acceptance. Stop this invoked step.
