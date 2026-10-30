# Phase 21 interrupt timing annex

**Purpose:** Source-qualified poll, vector-takeover and replay expectations for the planner.
**Evidence class:** AccuracyCoin at 6bee919 is a pinned test oracle for programmed expectations. Visual6502 pages are circuit analysis based on the die network and simulator. The mapping into this repository is a reasoned crosswalk, not a physical console measurement. No analog exact-tie winner is claimed.

## 1. Time vocabulary and boundary

| Term | Meaning | Source |
|------|---------|--------|
| Input sample | IRQ/NMI input observed during internal phi2; captured history is distinct from the later operation poll. | NESdev says inputs are sampled during phi2 [CITED: https://www.nesdev.org/wiki/CPU_interrupts]. Phase 20 circuit notes distinguish external M2 from internal phi2 [VERIFIED: .planning/phases/20-nmi-line-and-vblank-flag/20-NMI-ORACLE-RESEARCH.md:40-61]. |
| NMI stage 0 | Phase-qualified synchronizer; assertion and clearing are phase-qualified. | Visual6502 recognition stages [CITED: https://www.nesdev.org/wiki/Visual6502wiki/6502_Interrupt_Recognition_Stages_and_Tolerances]. |
| NMI stage 1 | Recognized NMI state; also controls which vector BRK selects. | Visual6502 recognition stages [CITED: https://www.nesdev.org/wiki/Visual6502wiki/6502_Interrupt_Recognition_Stages_and_Tolerances]. |
| Instruction poll | Operation-specific eligibility decision using sampled history and the I value applicable at that point. | AccuracyCoin IFlagLatency tests and NESdev interrupt page [CITED: https://raw.githubusercontent.com/100thCoin/AccuracyCoin/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm] [CITED: https://www.nesdev.org/wiki/CPU_interrupts]. |
| Core bus action Cn | nth external read/write of the current instruction or entry. It is not a circuit Tn label. Current BRK source orders fetch, PCH, PCL, P, then vector reads [VERIFIED: src/cpu.c:1059-1069]. |
| Core timing phase | Current bus cycle advances 24 half-master ticks. It advances to M2 rise, then internal phi2 opening at +12, copies prior captured inputs, and later captures current lines at close [VERIFIED: src/bus.c:1-14,51-83]. |

Interpretation: do not call an external Cn access the same thing as a Visual6502 Tn state. State access count and phase event together. Current source says “Both interrupt inputs are sampled at internal phi2 close” and “Instruction polls read this captured history at phi2 opening” [VERIFIED: src/bus.c:28-38].

## 2. IRQ acceptance and poll schedule

### 2.1 Source-qualified poll matrix

| Operation/case | External actions | Legal poll point(s) | I value at poll | Expected acceptance |
|----------------|------------------|---------------------|-----------------|---------------------|
| Ordinary instruction | Accesses C1 through Cn. The input is sampled during phi2 and the poll uses the preceding cycle's captured history. | At the operation's documented poll, ordinarily its last cycle. Do not treat every bus action as a separate accepted decision. | I applicable at that poll. | If captured IRQ is asserted and I=0, preserve acceptance until following dispatch even if source or I changes. |
| CLI | Opcode fetch and implied dummy access. | Last CLI cycle, before CLI changes I. | Old I. | If old I=1, CLI does not accept; following instruction polls with I=0 and may be followed by IRQ service. |
| SEI | Opcode fetch and implied dummy access. | Before SEI sets I. | Old I. | Previously accepted IRQ remains eligible after SEI; pushed P reflects architectural I=1. |
| PLP | Four actions: opcode fetch, implied access, stack dummy, P pull. | Poll during the final P-pull bus action before the pulled status is applied. | Old I for eligibility; restored P remains architectural state. | Cover old I=1→new I=0 and old I=0→new I=1. Preserve a result accepted under old I even when new I is set. |
| RTI | Six actions: opcode, PC dummy, stack dummy, P, PCL, PCH. | Poll after P restoration. | Restored I. | I=1 inhibits an unaccepted IRQ; I=0 allows immediate service/retrigger if source remains asserted. |
| Branch not taken | C1 opcode, C2 offset. | First poll associated with C1, before C2. | I at first poll. | No later branch poll; a source rising after the first poll waits for a later instruction poll. |
| Branch taken, same page | C1 opcode, C2 offset, C3 move-PC dummy/action. | Same first poll associated with C1; no second poll in C3. | I at first poll. | Retain first acceptance over C2/C3. |
| Branch taken, page cross | C1 opcode, C2 offset, C3 move-PC/old-page dummy, C4 PCH fix-up. | First poll associated with C1, and second after C3/before C4. | I at each poll. | Preserve acceptance across both; second poll can add acceptance but cannot erase first. |

The two branch exceptions follow AccuracyCoin's explicit schedule: same-page case A states C1 opcode/poll, C2 operand, IRQ appears during DMA, C3 move PC, no second poll; page-cross case B places the second poll after C3 and before C4 PCH correction. NESdev poll summary agrees with this schedule [CITED: https://raw.githubusercontent.com/100thCoin/AccuracyCoin/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm] [CITED: https://www.nesdev.org/wiki/CPU_interrupts].

Current source copies captured IRQ into poll_latch at internal phi2 opening and captures the new line at close. Therefore test the phase interpretation with finite-margin schedules before choosing exact field/statement placement [VERIFIED: src/bus.c:28-38,65-83].

### 2.2 AccuracyCoin IFlagLatency coverage

| Test | Pinned expectation | Owned assertion |
|------|--------------------|-----------------|
| 1 | DMC IRQ occurs and handler runs. | Handler marker proves source reaches service. |
| 2 | IRQ pending through CLI; following instruction increments X to 2 before handler. | Handler-recorded X=2. |
| 3–4 | CLI then SEI; already accepted IRQ still runs, and pushed P has I set. | Handler count and exact pushed P. |
| 5 | IRQ remains asserted through SEI/CLI and RTI; it fires twice, second before second INX. | Two handler entries and marker order. |
| 6 | RTI restores I=1 before poll; no IRQ. | No handler marker, sentinel unchanged. |
| 7 | PLP restores I=0; handler observes one INX before IRQ. | Handler-recorded X=1; add old-I/new-I counterpart. |
| 8 | Calibrates exact DMC/IRQ cycle before branch cases. | Preserve production APU source/DMC readiness; separate source timing from poll semantics. |
| 9 | Not-taken branch poll before C2. | Handler marker after branch. |
| A | Taken same-page branch has no second poll in C3. | X reaches 1 before handler. |
| B | Taken page-cross branch polls after C3 and before C4; an IRQ arriving during C2 is accepted there. | Handler after branch; source places code at a page boundary. |
| C/D | PPU open-bus and DMC data-bus prerequisites. | Report them; do not expand into DMA/open-bus implementation. |
| E | First branch poll sees IRQ; later $4015 dummy read acknowledges source before second poll; first acceptance still triggers IRQ. | Return PC=$3FA5, pushed P=$20 and guest marker $5A per assembly. C/D prerequisite checks must pass. |

Pinned source locations: overview/tests 1–A, AccuracyCoin.asm lines 9403-9648; B, lines 18803-18823; C/D/E, lines 9657-9757 [CITED: https://raw.githubusercontent.com/100thCoin/AccuracyCoin/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm]. The current target row is exactly “accuracycoin/Interrupt flag latency	fail	0x0a	151	-” [VERIFIED: tests/accuracy/scoreboard.txt:110]. Do not infer that every subtest ran only from the aggregate code; owned tests should name each behavior.

## 3. Circuit to core BRK/IRQ/NMI crosswalk

Visual6502 timing convention says [T1] fetch reads an opcode before it occupies IR; active control starts in T2. Its timing table labels BRK T5[V0] as its fourth active cycle and T6 as fifth [CITED: https://www.nesdev.org/wiki/Visual6502wiki/6502_Timing_States]. Hijack analysis says selection is not fixed at BRK start, NMI stage 1 may appear as late as T5 phase 1, and T5/T6 internal vector commands reach external pins one cycle later [CITED: https://www.nesdev.org/w/index.php?title=Visual6502wiki/6502_Interrupt_Hijacking&oldid=19268].

| Core action | BRK-origin bus action | IRQ/NMI-origin bus action | Circuit state | Takeover significance |
|-------------|------------------------|----------------------------|---------------|-----------------------|
| C1 | Fetch opcode $00; PC increments. | Discarded/current-PC read. | External [T1] fetch. Hardware recognition substitutes BRK into IR after fetch stage; software BRK is fetched from memory. | Too early to choose vector. Cause is relevant for stack semantics. |
| C2 | Read and skip signature/padding byte; return PC becomes opcode+2. | Second discarded read at interrupted PC. | T2 active execution. | NMI can be recognized in entry, but vector has not been chosen. |
| C3 | Push return PCH. | Push interrupted PCH. | T3. | No vector choice. |
| C4 | Push return PCL. | Push interrupted PCL. | T4. | Full-hijack arrival window: NMI must be low no later than end T4 / before T5 phase 1. |
| C5 | Push P with B/U set for BRK origin. | Push P with B clear and U set for hardware origin. | T5[V0], fourth active BRK cycle. | At T5 phase 1, stage-1 NMI can select NMI vector. In core action count this is after C4 PCL push and before C5 P push. |
| C6 | Read selected vector low. | Read selected vector low. | External consequence of internal T5 low-vector command, delayed one cycle. | Must use coherent prior decision. |
| C7 | Read selected vector high; set PC. | Read selected vector high; set PC. | External consequence of internal T6 high-vector command, delayed one cycle. | Must use same decision as low byte. |

**Reconciled wording:** preparation's “chosen after cycle 4 of 7” means after C4 PCL and before C5 P, matching the T5 phase-1 internal decision. “At vector-low fetch” is too late if it means waiting for external C6. C6 is the delayed external consequence of T5, not the decision point. The current core's BRK order is source-quoted as fetch(nes), then PCH/PCL/P pushes, then IRQ/BRK vector read [VERIFIED: src/cpu.c:1059-1069]. This mapping is a source-derived inference, not a measured physical tie.

### 3.1 Arrival/hold expectations

| NMI timing | Vector expectation | Handler/stack expectation |
|------------|--------------------|---------------------------|
| Eligible by T5 phase 1; line low no later than end T4 | Full takeover; both bytes use $FFFA/$FFFB. | Retain origin semantics: BRK still skips signature and pushes opcode+2 with B=1; IRQ origin pushes interrupted PC with B=0. |
| After T5 choice, during T5/T6/T0 inhibition | Cannot change selected vector; both reads remain coherent original pair. | If held through next T1 phase 1, first lower-priority handler instruction runs, then NMI entry. |
| Pulse in inhibition released before T1 phase 1 | May be missed under documented circuit behavior. | Do not promise every raw pulse is retained or queued. |
| NMI remains held low or represents already-consumed edge | Not a new falling edge. | Do not service it twice solely because level remains low. |
| New eligible edge during older entry | Preserve independently if entry stages recognize it; do not clear with older entry consumption. | New service follows its own recognition/service path. |

These are finite-margin circuit findings; exact zero-margin CPU/PPU tie winner is outside evidence [CITED: https://www.nesdev.org/w/index.php?title=Visual6502wiki/6502_Interrupt_Hijacking&oldid=19268] [CITED: https://www.nesdev.org/wiki/Visual6502wiki/6502_Interrupt_Recognition_Stages_and_Tolerances].

## 4. Paired AccuracyCoin overlap table expectations

Both overlap tests record status bytes pushed by NMI and lower-priority BRK/IRQ handlers. Exact alignment is checked first. If it fails, the source checks both paired tables shifted by +1 index; it does not allow per-byte or independent-handler shifts [CITED: https://raw.githubusercontent.com/100thCoin/AccuracyCoin/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm].

The shifted comparator reads one byte beyond each 32/64-byte answer table. Those sentinels are the next bytes in source order, not a repetition of the last answer: BRK's NMI table is followed by BRK's first answer (`0x36`); BRK's lower-priority table is followed by `LDA #$09` at `TEST_NmiAndIrq` (`0xA9`); IRQ's NMI table is followed by its lower-priority first answer (`0x26`); and IRQ's lower-priority table is followed by `SEI` at `TEST_APU_Prep` (`0x78`). The four sentinel values below therefore follow the pinned assembly's table and code order exactly [CITED: https://raw.githubusercontent.com/100thCoin/AccuracyCoin/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm].

### 4.1 NMI Overlap BRK: 32 samples

The source arrays are TEST_NmiAndBrk_AnswerNMI and TEST_NmiAndBrk_AnswerBRK (AccuracyCoin.asm lines 9935-9938). Shifted comparison uses source indices 1–32, including sentinel.

| Sample indices | NMI pushed-P | BRK pushed-P |
|----------------|-------------|--------------|
| 0–9 | 0xA4 | 0x36 |
| 10–24 | 0x36 | 0x00 |
| 25–31 | 0x26 | 0x36 |
| Sentinel 32, only for shifted compare | 0x36 | 0xA9 |

Allowed result: both tables at exact indices 0–31, or both shifted to indices 1–32. No mixed exact/shifted result. Comparison logic is at lines 9890-9929 [CITED: https://raw.githubusercontent.com/100thCoin/AccuracyCoin/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm].

### 4.2 NMI Overlap IRQ: 64 samples

The source arrays are TEST_NmiAndIrq_AnswerNMI and TEST_NmiAndIrq_AnswerIRQ (AccuracyCoin.asm lines 10063-10069). Shifted comparison uses indices 1–64, including sentinel.

| Sample indices | NMI pushed-P | IRQ pushed-P |
|----------------|-------------|--------------|
| 0–9 | 0xA4 | 0x26 |
| 10–35 | 0x26 | 0x00 |
| 36–63 | 0x26 | 0x26 |
| Sentinel 64, only for shifted compare | 0x26 | 0x78 |

Allowed result: both tables at exact indices 0–63, or both shifted to indices 1–64. Guest IRQ handler does not acknowledge its source during sweep, so IRQ line remains live for retrigger [CITED: https://raw.githubusercontent.com/100thCoin/AccuracyCoin/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm].

Current rows are exactly “accuracycoin/NMI Overlap BRK	fail	0x0a	151	-” and “accuracycoin/NMI Overlap IRQ	fail	0x06	151	-” [VERIFIED: tests/accuracy/scoreboard.txt:118-119]. These are baseline observations, not post-change expectations.

## 5. Fixture and state continuation seams

| Layer | Existing seam | Establishes | Limitation / planned use |
|-------|---------------|-------------|--------------------------|
| CPU access/stack | vector_bus.c replaces bus read/write with flat RAM and logs each access [VERIFIED: tests/cpu/vector_bus.c:24-37]. | Access order/address, stack bytes, PC/P origin semantics, wrapping. | Cannot prove production APU/PPU sampling. Use only for sequence and stack assertions. |
| Production bus | tests/nmi_fixture.h builds owned NROM and loads a real instance [VERIFIED: tests/nmi_fixture.h:100-139]; test_nmi_poll drives full cpu_step and observes guest markers [VERIFIED: tests/core/test_nmi_poll.c:49-77,121-151]. | Real bus order, PPU NMI line/capture, APU clock/capture, guest handler order. | Add a frame-IRQ transition through production APU/bus clock path. Existing test_apu sets frame_cycle near the IRQ edge then clocks APU [VERIFIED: tests/core/test_apu.c:330-346]. Schedule PPU-derived NMI edges at entry boundaries; retain MMC3 checks. |
| Multi-board state replay | test_state_fork matrix; state_test.h synthetic NROM/MMC3 program puts frame IRQ, DMC IRQ, NMI and mapper IRQ in flight [VERIFIED: tests/state_test.h:299-319,350-397]. | Current cross-board, state and hash controls. | Keep matrix; detailed stage sweep belongs in small NROM fixture. |
| Native cutpoint | test_nmi_state distinguishes constructed origin from executed complete-cpu_step endpoint and checks synthesis history [VERIFIED: tests/core/test_nmi_state.c:158-205]. | Pattern for origin labels, executed endpoint, nonzero audio history and public replay comparisons. | Drive new cuts through production entry stages at completed bus actions. Do not snapshot suspended C stack or fabricate prior pushes. |
| Rejection | state.c validates before APPLY and rejects format mismatch before later checks [VERIFIED: src/state.c:460-500]. | CRC-correct invalid entry-state rejection and all-or-nothing instance controls. | Extend visitor ranges from reachable stage/origin/vector combinations; preserve existing error order. |
| Actual libretro | libretro_host dynamically loads module [VERIFIED: tests/libretro/libretro_host.c:39-75,79-104]; retro_unserialize calls native loader [VERIFIED: libretro/libretro.c:383-399]. | Actual adapter state restore followed by future host-facing frame/video/PCM outputs. | Load state into same synthetic NROM identity, compare run-ahead committed/speculative outputs. Label evidence “internally reached cutpoints loaded through public APIs.” |

### 5.1 Suggested state cutpoint comparison

For each origin (software BRK, IRQ, NMI), drive production entry actions to each distinct completed-bus-action cutpoint. Save with nesturbator_save_state; load into a clean instance with identical cartridge bytes; resume with ordinary cpu_step/frame calls. Compare canonical bytes, subsequent bus trace, ticks, stack/PC/P, guest marker order, then public frame/audio and actual adapter output. Include before/after vector choice, between vector reads and around request consumption; interleave instances with different histories. Keep nonzero synthesis history. Header says “A state carries no ... staged PCM ... APU sample phase and synthesizer/filter history are saved machine state” [VERIFIED: include/nesturbator.h:695-700].

## 6. Acceptance and regression checks

Context locks all eleven v5 rows and protection of all 114 starting PASS keys. Baseline metadata records attempted_scored_rows=146, terminal_scored_rows=146, pass=114, fail=32, draw=5 and execution_frame_budget=1800 [VERIFIED: tests/accuracy/baseline-metadata.txt:14-28].

| Roadmap criterion | Plan evidence |
|-------------------|---------------|
| The latency and both overlap AccuracyCoin rows pass; owned CPU tests cover CLI, SEI, PLP, RTI, branches and NMI at each BRK/IRQ cycle [VERIFIED: .planning/ROADMAP.md:169]. | CPU sequence tests, production bus sampling tests and complete pinned rows. |
| All eleven v5 rows pass and no pre-v5 passing key is lost on any CI lane [VERIFIED: .planning/ROADMAP.md:170]. | Fresh 114-key protected-main authentication in same CI run and all eleven named rows. |
| cpu.vectors, nightly blargg MMC3, Holy Mapperel and runner.boot_to_play remain green [VERIFIED: .planning/ROADMAP.md:171]. | Preserve CI/nightly placement and attribute first divergence. |
| Mid-entry state restores in core.state_fork and libretro.runahead; moved hashes match across six platforms [VERIFIED: .planning/ROADMAP.md:172]. | Internally executed cutpoints, public native save/load, actual adapter unserialize and future output/hash comparison. |

Baseline metadata quotes pin 6bee919529289c555803b612b8c9e4f7eaaeeabb and execution_frame_budget=1800 [VERIFIED: tests/accuracy/baseline-metadata.txt:3,14]. Existing campaign registration retains EXECUTION_CAP=1800 [VERIFIED: tests/CMakeLists.txt:858-880]. No tests were run during research.

## 7. Evidence limits

- AccuracyCoin assembly supports programmed test intent and accepted table alignment, not a console measurement of every pin phase.
- Visual6502 sources explain circuit/simulator behavior; this pass did not rerun the simulator, build a phase oracle or record a physical console trace.
- Exact equal-timestamp CPU/PPU tie precedence remains unresolved and is not a plan prerequisite.
- The production entry-stage test driver, reachable serialized-state matrix and adapter cutpoint round-trip are future work; they do not yet exist.
- IFlagLatency C/D/E depend on DMC/open-bus prerequisites. Isolate first divergence before changing interrupt scope.
