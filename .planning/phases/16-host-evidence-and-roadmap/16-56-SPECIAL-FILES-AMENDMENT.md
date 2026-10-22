# Plan56: fresh protected capture with special-file metadata

## Current authority and failed identity

This is the substantive planning correction requested after
16-56-BASELINE-CHECKPOINT.md, under `$gsd-plan-phase 16 --gaps`.
It amends the still-unfinalized Plan56; it does not create Plan57 or supersede
56. The previous actual capture stopped before construction, dispatch or
prospective transport finalization. Preserve prospective_transport_finalized=false,
terminal_transport_failure=false, zero construction operations and 0/2 tasks.
The unsupported capture nevertheless ended that attempt. This planning
amendment supplies new prospective capture authority; the failed execution,
the approved construction overlay and an unchanged DAG supply none themselves.

The failed source is the exact UTF8 `failed_capture.request.code` in
16-56-BASELINE-PROVENANCE.json: 3264 bytes, SHA256
`7c230972025d6a35430715d80a3fb137794f0a1068146055a0820f7b4a21a8e9`.
Its request, actual isError:true/UNSUPPORTED_TYPE, partial11134rows,
233384862hashed bytes, 70 declared roots, private archive3807830bytes/SHA256
`bbff8aafd2b7ba4cd29bd7985397dcb34cf76318e78750db6210942ed74f9016`
and prefix3054916bytes/SHA256
`0b4cf8230b75f7c17ca533341e614750ea60baa2804a7a0a892a04bf4b811455`
remain immutable DATA. No source is executed from this archive. No prefix,
earlier budget result, metadata diagnosis, original buffer or old receipt is
accepted as a complete baseline, reception, preservation comparison or gate.

The new logical capture attempt is exactly `p56-special-metadata-01`.
Before its first capture call bind that ID, this checked amendment/PLAN/CHECK,
the approved CONTEXT-AMENDMENT, current HEAD/raw index/config/dirty originals,
actual parent or child identity and exact complete prospective source/request/
receiver/schema/comparator/expected-case bytes. Parent and child have separate
stores and bind their actual exposed identities; they do not share acceptance.
Fresh immutable keys use `p56-special-metadata-01-` plus context and operation
identity. Stage56's literal budget descriptor pair stays as specified in
51AMENDMENT with p50→p56 substitution, but is freshly executed and bound to
this attempt; no earlier ready flag or original is reused. New source must
differ from the failed source in substantive schema/type handling and have its
own measured exact length/SHA and full bytes. Do not invent that future digest.

This is one new attempt, not a retry controller. Any unexpected failed actual
capture, unstable observation, unsupported schema/type, construction operation,
finalized transport or postfreeze matrix stops and preserves exact available
originals/prefix. No second capture, enumeration restart, suffix, repair,
alternative owner/root, re-freeze or relabeling follows in that execution.
Expected isolated synthetic refusals retain their explicit control identity.
Another attempt would require a substantive new planning amendment. Terminal
predecessor identities and their stronger terminal-source rules stay intact.

## Precedence and unchanged construction boundary

16-56-CONTEXT-AMENDMENT remains the sole approved ordering/allowlist authority.
This correction changes the fresh capture schema, integer handling, receiver
and controls only. It does not authorize an earlier durable byte. Before any
construction, the parent must complete the new protected baseline read-only in
memory; it cannot depend on a helper that construction has not yet authorized.
Use the existing inline standard-library Node producer and independent caller
receiver for that stage. Implement the same tagged schema in both eventual
helpers during Task1's existing construction allowance, not in a third module.
No bootstrap, spool, inventory file, metadata journal or diagnostic is early.

All helper APIs/selectors/fixture recipes, including this correction, finish
inside Task1 before the journal is closed and complete final source is frozen.
Bind the preconstruction capture program as an immutable input to that final
closure. No helper edit changes that program retroactively. Child candidate
checks remain read-only, not a new construction owner or accepted import.
Each actual parent/child repeats the complete own-context gates. Task2 only
validates, records accepted evidence and exports identical frozen helper bytes.
The postconstruction full snapshot includes the two helpers and closed journal
as untrusted inputs and compares protected history to the complete new baseline;
only the exact declared construction deltas are allowed. No whole-root exclusion.

## Exhaustive metadata-only tagged variants

Keep the canonical compact sorted-key JSON **array plus one LF**, the exact
ordered retained root set and entry walk, relative path bytes, regular-file
hashing rules and existing symlink-target policy. No root/entry is removed to
avoid a special type. Extend the entry schema locally with the exact special
variants below in both capture and independent receiver. `type` is POSIX
S_IFMT's integer value, distinct from permission `mode`.

| type | kind | metadata_only | content_status |
|---|---|---|---|
| 4096 | fifo | true | not-read-special |
| 49152 | socket | true | not-read-special |
| 8192 | character-device | true | not-read-special |
| 24576 | block-device | true | not-read-special |

For those four kinds, exact required row keys are path, type, kind, mode,
size, mtime_ns, ctime_ns, inode, device, rdev, nlink, metadata_only and
content_status. Map inode/device from lstat ino/dev; mode is mode & 4095;
type is mode & 61440. Retain the actual size and rdev even when nonzero;
size is metadata, never readable or hashed content length. No sha256, target,
payload, content bytes, content digest or inferred major/minor is permitted in
a special row. Exact keys and type/kind/status pairing are independently
validated; unsupported S_IFMT, whiteout, unknown or mixed variant stops with
SNAPSHOT_SPECIAL_TYPE_INVALID, never generic skip or regular-file fallback.
Directory, regular and symlink variants keep their inherited required fields;
their added special-only keys are forbidden. Version this local row union in
the bound source/transport descriptor as `phase16-snapshot-special-metadata-1`;
do not rewrite historical row schemas or historical snapshots into that version.

For the original four mode000 **regular** files, retain their old exact
path/type/mode/size/mtime_ns/inode and metadata_only=true records, no content
read or hash, no special-kind relabeling and no inferred equality of bytes.
Compare the four original records to the checkpoint's exact integers in
addition to ordinary new snapshot equality. They remain the same four
limitations even if the special-file count is nonzero. Do not treat every
permission failure as an authorized metadata-only exception: readable regular
files with unexpected EACCES or any other read failure stop under the retained
contract. Permissions and all historical entries remain untouched.

### Exact integers, canonical parsing and coverage

All integer-bearing metadata are emitted as **unquoted canonical decimal JSON
integer tokens** directly from Node bigint lstat values: zero is 0, no leading
zero, plus sign, exponent, decimal point or negative zero. Preserve exact
timestamp/inode values above 2^53 without Number conversion. All fields except
mtime_ns/ctime_ns are nonnegative; timestamps may be signed. Preserve the
existing platform integer domain; do not impose a newly rounded or narrowed
Number range. Bound each metadata integer token to128ASCIIbytes, including
an optional timestamp minus sign, in the frozen schema before capture; this
generous representation bound preserves platform stat integers and keeps
framing finite. Fail SNAPSHOT_INTEGER_INVALID for an overlong value rather
than truncation. Retain exact value and byte spelling separately from counts.

The independent caller receiver must parse numeric tokens losslessly before
ordinary object validation, using the existing strict integer/JSON facilities
or a small local metadata-number adapter; JSON.parse into Number followed by
BigInt cannot recover rounded integers. Both Python helper consumers use exact
int with strict raw-token/canonical-byte checks, reject bool-as-int, and must
agree with the independent literal expected values. Duplicate keys, unknown
fields, changed canonical serialization or trailing data refuse; metadata
validation never relaxes the existing blob/transport parser. Compare the full
reconstructed canonical bytes including LF, not just parsed object equality.

Freeze the complete ordered root declaration freshly: same protected roots
and source scope as inherited capture, including every retained special entry,
plus the current tracked planning corrections as inputs. Record absence of
every declared fresh root; refuse any pre-existing candidate. A historical70
root count or11134row prefix is not today's count. Bind complete root/entry
counts, regular-hashed bytes, separate four-unreadable and per-special-kind
counts, exact canonical total bytes/SHA and complete path-to-root coverage
before READ begins. Root order and per-directory order remain the inherited
case-sensitive deterministic order; fail unsupported path encoding rather than
normalize, skip or follow it. Compare byte-exact complete root membership,
entry membership and order in the independent receiver. Missing/extra/duplicate/
reordered entries or an exhausted prefix fail SNAPSHOT_COVERAGE_INVALID or
the existing index reason, never acceptance of an otherwise valid prefix.

### Stable metadata observation without special content access

Special entries are lstat-only: never open, read, hash, connect, listen,
readlink, realpath, traverse, chmod, unlink, replace or send/receive their
contents. Never wait for FIFO peers. Read bits do not alter that rule.
Bracket each metadata-only row with lstat before/after; compare exact
type/mode/dev/ino/size/mtimeNs/ctimeNs/rdev/nlink, not only size/mtime. Retain
the stability tuples privately with the complete baseline originals. For each
directory bracket readdir/descendant traversal with lstat and the exact ordered
name census; validate ancestor identities before each child observation and
after traversal. Recheck the complete captured metadata/name census once in a
bounded final verification pass before binding the immutable complete capture;
this pass is lstat/readdir comparison, not a new capture or content retry.
Ordinary regular reads/symlink target observations keep their stricter
inherited stable-lstat protections. Type is classified before choosing an
operation; a special variant never enters the regular/symlink/directory path.

Disappearance, inode/dev replacement, regular↔FIFO/socket/device/symlink swap,
ancestor substitution, changed timestamp/mode/rdev/nlink/size, or changed
directory census fails SNAPSHOT_LSTAT_UNSTABLE at the observation boundary;
retain the exact failed observation and stop without restart or fallback.
Instrumented swap controls inject the change between the bracket observations
and require refusal before a content operation on the substituted special
entry. Record the inherited local-session stability assumption honestly:
separate lstat observations are not a universal atomic filesystem snapshot or
proof against an undetectable change-and-restore between all observations.
No ABA/hostile-kernel guarantee is added by metadata equality. Detected drift
always stops; no weaker preservation claim or extra read is accepted instead.

## Deterministic independent control ledger

Before actual new capture, freeze literal coherent stat records, canonical byte
expectations and intended reasons independently from capture output. Exercise
the SAME prospective capture/type classifier and independent receiver. Use
read-only in-memory filesystem doubles: every forbidden content operation on
a special entry increments a tripwire and throws. No fixture FIFO/socket/device
is created on disk, no third helper, privileged mknod or actual socket activity.
Synthetic subjects stay labeled synthetic, authority=false,
runtime_admission=false, proposed_writes=[], logical exit23 for refusals and
zero durable/clock/boot/root effects. Node OS exit remains unavailable.

| Control | Exact required observation |
|---|---|
| One positive for EACH of FIFO/socket/character-device/block-device; mixed directory with regular/symlink and four unreadable originals | Exact independently specified complete canonical rows/order/integers; zero special-content tripwires; separate kind/limitation counts |
| Above-2^53 inode and ns, signed timestamp, nonzero device/rdev/size and maximum supported exact integer width for EACH kind | Exact literal bytes and values round-trip in caller and both helper consumers, no Number rounding or content-length interpretation |
| EACH kind's type/kind mismatch, unsupported type and special→regular tag substitution | SNAPSHOT_SPECIAL_TYPE_INVALID |
| EACH kind missing required key, extra sha256/target/payload, wrong metadata_only/content_status or bool/string/float numeric leaf | SNAPSHOT_SPECIAL_SCHEMA_INVALID (numeric lexical/value violations use SNAPSHOT_INTEGER_INVALID first) |
| EACH kind rounded/changed integer with coherent local descriptor/hash | SNAPSHOT_METADATA_MISMATCH against independent exact record; noncanonical/overflow token instead yields SNAPSHOT_INTEGER_INVALID |
| EACH kind capture branch attempts open/read/hash/readlink/connect or traversal | SNAPSHOT_SPECIAL_CONTENT_FORBIDDEN; tripwire proves prevented operation, not a real access |
| EACH kind swapped inode/type/dev/metadata between lstat observations; separate special↔regular/symlink and ancestor/name-census controls | SNAPSHOT_LSTAT_UNSTABLE, before substituted special content operation; no rescan |
| EACH kind missing/extra/duplicate/reordered row, root omission, truncated valid prefix or unknown extra root | Existing SNAPSHOT_COVERAGE_INVALID/SNAPSHOT_INDEX_INVALID as predeclared per case |
| Four old unreadable records replaced by hashed, fabricated empty, changed integer or special variant | Intended SNAPSHOT_METADATA_MISMATCH or SNAPSHOT_SPECIAL_TYPE_INVALID; all four limitations stay explicit |
| Complete coherent record stream delivered under old failed attempt or another context/schema/source binding | SNAPSHOT_IDENTITY_INVALID; no source/attempt/proof transfer |

Bind reason precedence per case before invocation. A generic parse/setup crash,
the old UNSUPPORTED_TYPE, a different earlier error or an unused expected label
fails coverage. The receiver's independent literal expected table is not
generated by the producer classifier. For kinds absent in actual retained
history, synthetic coverage is explicit, never a claim of actual device/socket
observation. Actual retained FIFO .continue/.ready receive full fresh lstat
rows in complete own-context capture; observed counts are measured later.

Implement `--test-special-file-snapshot` inside the existing assembler during
Task1 construction and equivalent production schema validation in the existing
launcher. The selector consumes actual new complete baseline/source/request/
result originals, complete own READ and independent COMPARE reception,
coverage/stability records and all labeled type-specific controls. It never
initiates a capture or grants authority retrospectively. It joins existing
segmented-snapshot, large-payload, acyclic, budget, orchestration and wrapper
selectors and full working/adjacent/content-addressed-exported preparation
matrices. Final preparation verifier requires every original and reason.

## Independent downstream duties and unchanged gates

51 starts only in a later separately invoked step after genuine56PREPARATION/
SUMMARY/code export AND actual56administrative parent reconciliation.52 waits
for genuine51success AND its actual reconciliation, then52/53/54 run in the
same original52native child continuously. Each actual51/52parent and child
binds its own fresh special-schema capture identity/source/receiver and repeats
the **entire** metadata-only/coverage/stability/control/reception gate before
its own first write. Use exact IDs `p51-special-metadata-01` and
`p52-special-metadata-01` in their own context stores. Their first capture is
not a retry of56 and no56acceptance/metadata-only row/digest fills their proof.
They import CODE ONLY after own authorization;56-only early construction is
not extended. Include this selector in their same final-byte rehearsal/freeze/
replay/preparation verifiers, without a13th runtime row. All producer/receiver
adaptation is done in their existing declared prefinalization stage.

Every outer/nested18000, decoded67108864, request4194304, producer16384,
caller8192(normally1024), rawsegment4096, at most32sequential awaited packets,
one outstanding packet and separate ACK stay. Preserve full actual originals,
fresh retained38 reception, real initial-header and EVERY largest empty/nonempty
A/B, complete READ/COMPARE byte reconstruction and finite graph, exact result
slots/cutoffs/readback/seal before nonconstruction writes. Full protected
history, six byte-exact unstaged dirty files,26active terminal=false/pending
budget+ledger and38journal comparison stay; no partial-prefix admission.
07/08full actual parent/native/exact-head affirmative consent/normal publication/
18tag-bound archives+checksums+attestations/sixmetadata/exact-releaseCI stay.
All UNKNOWNs/unavailable originals and all unexecuted gates remain pending.

## Multi-source audit, assumptions, artifacts and estimate

All39 exact source rows in16-56-AMENDMENT remain COVERED with unchanged owners:
3 GOAL,6 REQ,13 RESEARCH,17 CONTEXT. In particular D-07–D-12's reason-specific,
attributable, bounded one-shot discipline is implemented by the new identity,
complete metadata schema/coverage and deterministic controls in56Task1 and
validated unchanged in56Task2, independently repeated by51/52Task1 and final
verification. D-01–D-06,D-13–D-17 and EVID-01–EVID-04/DIR-01/DIR-02 retain their
full existing owned implementation/later gates; no scope or credit is removed.

| Correction source | Concrete obligation | Owner | Status |
|---|---|---|---|
| BASELINE-CHECKPOINT/PROVENANCE | Immutable failed source/prefix, exact ceiling, newly authorized capture identity |56Task1;51/52own identities | COVERED |
| GOAL/REQ EVID-02; CONTEXT D-07–D-12 | FIFO and all admitted special kinds, exact integers, independent schema and reason-specific controls |56Task1/Task2;51/52own gates | COVERED |
| RESEARCH intended-reason actual-driver faults and provenance/retention | Complete coverage/stability/originals, no content access or partial acceptance |56Task1/Task2;51/52own gates | COVERED |
| CONTEXT-AMENDMENT approved ordering | Complete baseline before construction; all code then closed journal/final-source freeze; Task2validation/export only |56Task1/Task2 | COVERED |

The freshly supplied fallback edge JSON retains8items, all unresolved at probe
level and4unclassified. EVID-02idempotency/concurrency and EVID-03empty/encoding
remain mapped to the exact explicit truths and controls in the current PLAN;
EVID-01/EVID-04/DIR-01/DIR-02stay flagged unresolved assumptions. No taxonomy,
future compliance, transport capacity or release completion is inferred.
Assumption-delta contribution consumed: primary noun remains exact
candidate/executor/source/authority; decision=no-change. Special kinds are
tagged observations inside that identity, not another owner/source of truth.
Prohibition recall/refilter retains canonical AGENTS/D-01–D-17/test/provenance/
privacy/07consent referrals, prohibitions:[] and no silent drops. Deferred
ideas remain excluded. Research skipped by --gaps; existing local architecture,
no external library/install. Security/Nyquist/pattern mapper stay disabled.

Planning artifacts: new16-56-SPECIAL-FILES-AMENDMENT, scoped56PLAN/AMENDMENT
and51PLAN/AMENDMENT/52PLAN references only. Parent owns STATE/ROADMAP/checks/
planning commit. Historical checkpoint/provenance/check/recheck and approved
CONTEXT-AMENDMENT bodies remain immutable. Execution later creates only the
already-declared twohelpers+journal, then accepted existing evidence/export
targets and genuine success-only PREPARATION/SUMMARY. No new root/module/file
is authorized by the selector; ignored build paths are future owned outputs.

Fresh calibration factor1/sample_count0/confidence low:56raw/calibrated104000
= retained96000+8000 for new source identity, full special-schema/receiver,
stability and four-kind controls. Effective51/52each75000=71000+4000 independent
own-context repetition;53/54unchanged43000/31000. Aggregate328000;
continuous52–54runtime149000. Two tasks/plan, same wave10→11→12→13→14→15→16,
17live/56physical/10complete. These are realistic additive planning projections,
not measured execution, provider/context-fit or throughput claims. No scope
reduction, new split approval or extra owner is inferred from sizing.

## Reviewed advisory overruns and mandatory pre-dispatch context-fit rule

The two scope warnings in16-56-SPECIAL-FILES-CHECK have been reviewed as
planning advisories:56 is104000/100000 (1.04x); the ONE continuous52/53/54
native-child invocation is75000+43000+31000=149000/100000 (1.49x).
Factor1/sample_count0/confidence low establishes neither actual capacity nor
fit. Keep these estimates and the100000 smart-zone budget unchanged. The
328000 aggregate spans separate invocations and cannot establish group fit.
This review accepts no execution risk on the user's behalf and gives no
dispatch admission. Actual capacity/headroom/fit remain UNKNOWN at planning.

Before dispatching56, and separately before the later sole52group dispatch,
the existing orchestration owner must review a complete context-cost ledger
against the actual target runtime's exposed effective context limit and used
context, including its system/tool overhead. Use only available documented
runtime settings and actual telemetry; a model name, default window, smart-zone
budget, token estimate or proposed configuration is not capacity evidence.
Bind the ledger to the exact checked plan/amendment/dispatch source closure
and target identity. Include all mandatory full source/input/history bytes,
independent actual reception originals and retained volatile stores, expected
control/output and reviewer/handoff traffic, construction/edit history,
generation and final closure/diagnosis headroom. Account for simultaneous
producer/receiver/decoded copies where required; do not count an archive,
digest, reference or summary as removal of a required original. Keep explicit
assumptions and reserves; document how the complete projected demand plus
unallocated uncertainty reserve fits the exposed available headroom. The
104000/149000 estimates are the retained planning projections to review, not
certified bounds; source-size accounting alone cannot prove total fit.

If effective capacity, current usage, an adequate whole-duty projection or
reserve cannot be established, or their comparison does not support the
complete duty set, STOP BEFORE dispatch/capture/construction/initialization.
Missing telemetry is a concrete stop condition, not implied capacity. Report
the exact missing or insufficient quantity for a concrete planning amendment;
do not dispatch a child just to probe capacity. Even after a supported parent
review, the actual dispatched child must check its own exposed usage/headroom
before its first capture or write and refuse if that comparison is unavailable
or inadequate. This administrative review supplies no transport, capture,
admission, test or task credit and creates no new helper/root/evidence file.
Retain its actual inputs/disposition in existing caller memory until the
existing gates authorize any declared evidence/handoff target.

56's context strategy uses ONLY the approved ordering: complete the fresh
inline protected baseline, journal all construction of the same two helpers,
close the journal and freeze final source, then perform every independent
actual transport/reception/finite-graph gate and Task2 final-byte matrix/export.
Finish all repairs before freeze to avoid another invalidated proof cycle;
avoid unrelated exploration and redundant narration, while retaining every
required exact source, original, journal revision and control. This changes
neither the estimate nor any input, output, gate, owner or acceptance duty.
It is not another plan/task split, dispatch or permission to compact.

The later52group review covers ALL52/53/54 duties together, not three separate
headroom grants. Freeze and provide the complete exact52/53/54 source/dispatch
closure before its ONE original native-child invocation. Preserve full source,
independently received actual originals and volatile store continuously in
that same child through all12admission rows, immediate initialize→observe,
development, all formal assignments, separate-context independent reviewer
traffic, all summaries and checked completion-start/final cursor. Preparation
in56/51 supplies CODE ONLY and no acceptance or capacity credit. A subordinate
52→53 or53→54checkpoint supplies no reclaimed context or new dispatch allowance.
General compaction, summaries replacing complete source/original reception,
new root/owner/child, reset/resume as a context remedy or any new split remain
unapproved. Existing same-target stage protocol continues only with unchanged
originals, clocks, counters, slots and complete duties; it is no context remedy.

Reconcile actual context growth with the remaining full-duty projection before
each next bounded transport/construction operation, final freeze/dispatch,
stage handoff, initialization and formal/reviewer handoff, using existing
orchestration observations rather than an extra runtime command. Never insert
a check between required immediate initialize→observe or alter timed classes.
If headroom becomes unknown or insufficient, stop with the exact available
prefix under the existing failure/diagnosis/terminal rules; no repair/retry,
counter/clock reset, evidence reconstruction, omitted gate or success label.
Do not begin an operation whose full required reception/return cannot fit.
All transport limits, full metadata controls, independent51/52identities,
six exact unstaged dirty files,26active terminal=false/pending and downstream
parent/native/exact-head consent/publication/released-byte gates stay binding.

<output>Parent checks and commits planning only. Later execute56alone through genuine preparation and actual reconciliation, then STOP. No capture or task acceptance occurs in this planning step.</output>
