/* Owned CPU-executed timing guests, derived from the pinned MIT test
   contract, not emulator source: AccuracyCoin 6bee919, ExplicitDMAAbort /
   ImplicitDMAAbort and their clockslide/sync helpers (HWC.19). Relocation
   preserves relevant opcodes, access order and branch cycle counts.
   https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm
   Include from test_bus.c after its helpers. These guests add no ROM file. */
#ifndef NESTURBATOR_TEST_DMA_ABORT_SWEEP_H
#define NESTURBATOR_TEST_DMA_ABORT_SWEEP_H

enum rom_abort_sweep_kind {
    ROM_ABORT_EXPLICIT = 0,
    ROM_ABORT_IMPLICIT1,
    ROM_ABORT_IMPLICIT2,
    ROM_ABORT_IMPLICIT3,
    ROM_ABORT_SWEEP_COUNT
};

struct rom_abort_sweep_location {
    uint16_t loop_pc;
    uint16_t duration_return_pc;
    uint16_t output_address;
    const char *name;
};

static uint16_t rom_abort_done_pc;
static uint16_t rom_abort_check_return_pc;
static struct rom_abort_sweep_location rom_abort_locations[ROM_ABORT_SWEEP_COUNT];
static uint8_t rom_abort_implicit_late;
static uint8_t rom_abort_first_mismatch[ROM_ABORT_SWEEP_COUNT];

static void rom_abort_emit_compare_loop(uint16_t *pc, uint16_t result, uint16_t key);

static void rom_abort_put(uint16_t address, uint8_t value)
{
    landing_prg[(size_t)(address & 0x3fffu)] = value;
}

static void rom_abort_emit_rel(uint16_t *pc, uint8_t opcode, uint16_t target)
{
    uint16_t operand;
    test9_emit8(pc, opcode);
    operand = *pc;
    test9_emit8(pc, 0u);
    test9_put(operand, (uint8_t)(target - (uint16_t)(operand + 1u)));
}

static void rom_abort_emit_word_slide(uint16_t *pc, uint16_t helper, uint16_t cycles)
{
    test9_emit_jsr(pc, helper);
    test9_emit16(pc, cycles);
}

static void rom_abort_emit_clock_tables(uint16_t cstable_sync)
{
    uint16_t pc = 0xfd00u;
    for (unsigned i = 0u; i < 14u; ++i)
        rom_abort_put(pc++, 0xc9u); /* ClockslideFromWord CSTable bytes. */
    rom_abort_put(pc++, 0xc5u);
    rom_abort_put(pc++, 0xeau);
    rom_abort_put(pc++, 0x4cu);
    rom_abort_put(pc++, (uint8_t)cstable_sync);
    rom_abort_put(pc, (uint8_t)(cstable_sync >> 8));

    pc = 0xff00u;
    for (unsigned i = 0u; i < 35u; ++i)
        rom_abort_put(pc++, 0xc9u); /* Clockslide_50 through Clockslide_16. */
    rom_abort_put(pc++, 0xc5u);     /* Clockslide_15; $EA is its zero-page operand. */
    rom_abort_put(pc++, 0xeau);     /* Clockslide_14. */
    rom_abort_put(pc, 0x60u);       /* Clockslide_12. */
}

static void rom_abort_emit_clockslide500(void)
{
    const uint16_t minus12 = 0x8500u;
    const uint16_t slide500 = 0x8540u;
    uint16_t pc = minus12;
    test9_emit_jsr(&pc, (uint16_t)(0xff00u + (50u - 26u)));
    test9_emit_jsr(&pc, 0xff00u);
    test9_emit8(&pc, 0x60u);

    /* Keep the pinned nested JSR/fallthrough layout. Its stack writes matter
       when an implicit one-cycle halt coincides with a write cycle. */
    pc = slide500;
    uint16_t hundred = (uint16_t)(slide500 + 12u);
    test9_emit_jsr(&pc, hundred); /* Clockslide_500. */
    test9_emit_jsr(&pc, hundred); /* Clockslide_400. */
    test9_emit_jsr(&pc, hundred); /* Clockslide_300. */
    test9_emit_jsr(&pc, hundred); /* Clockslide_200. */
    test9_emit_jsr(&pc, minus12); /* Clockslide_100. */
    test9_emit8(&pc, 0x60u);
}

static void rom_abort_emit_phase_slides(void)
{
    uint16_t pc = 0x8580u;
    test9_emit8(&pc, 0x85u); /* STA $00 */
    test9_emit8(&pc, 0x00u);
    test9_emit8(&pc, 0xa9u); /* LDA #$FF */
    test9_emit8(&pc, 0xffu);
    test9_emit8(&pc, 0x85u); /* STA $01 */
    test9_emit8(&pc, 0x01u);
    test9_emit8(&pc, 0x6cu); /* JMP ($0000): five cycles, matching the ROM. */
    test9_emit16(&pc, 0u);

    pc = 0x85a0u;
    test9_emit8(&pc, 0x85u); /* STA $00 */
    test9_emit8(&pc, 0x00u);
    test9_emit8(&pc, 0xa9u); /* LDA #$FF */
    test9_emit8(&pc, 0xffu);
    test9_emit8(&pc, 0x85u); /* STA $01 */
    test9_emit8(&pc, 0x01u);
    test9_emit8(&pc, 0xa9u); /* LDA #36 */
    test9_emit8(&pc, 36u);
    test9_emit8(&pc, 0x38u); /* SEC */
    test9_emit8(&pc, 0xe5u); /* SBC $00 */
    test9_emit8(&pc, 0x00u);
    test9_emit8(&pc, 0x85u); /* STA $00 */
    test9_emit8(&pc, 0x00u);
    test9_emit8(&pc, 0x6cu); /* JMP ($0000), five cycles. */
    test9_emit16(&pc, 0u);
}

static void rom_abort_emit_clockslide_from_word(uint16_t helper)
{
    uint16_t pc = helper;
    uint16_t cstable_sync;
    uint16_t csx_loop, csx_mid, cszy_loop, cszy_249, cszy_mid, cs94_loop;

    test9_emit8(&pc, 0x85u); /* STA $FF */
    test9_emit8(&pc, 0xffu);
    test9_emit8(&pc, 0x84u); /* STY $FE */
    test9_emit8(&pc, 0xfeu);
    test9_emit8(&pc, 0x86u); /* STX $FD */
    test9_emit8(&pc, 0xfdu);
    test9_emit8(&pc, 0xbau);             /* TSX */
    test9_emit8(&pc, 0x18u);             /* CLC */
    test9_emit_abs(&pc, 0xbdu, 0x0101u); /* LDA $101,X */
    test9_emit8(&pc, 0x85u);             /* STA $00 */
    test9_emit8(&pc, 0x00u);
    test9_emit8(&pc, 0x69u); /* ADC #2 */
    test9_emit8(&pc, 0x02u);
    test9_emit_abs(&pc, 0x9du, 0x0101u); /* STA $101,X */
    test9_emit_abs(&pc, 0xbdu, 0x0102u); /* LDA $102,X */
    test9_emit8(&pc, 0x85u);             /* STA $01 */
    test9_emit8(&pc, 0x01u);
    test9_emit8(&pc, 0x69u); /* ADC #0 */
    test9_emit8(&pc, 0x00u);
    test9_emit_abs(&pc, 0x9du, 0x0102u); /* STA $102,X */
    test9_emit8(&pc, 0xa9u);             /* LDA #1 */
    test9_emit8(&pc, 0x01u);
    test9_emit8(&pc, 0x38u); /* SEC */
    test9_emit8(&pc, 0xe5u); /* SBC $3C (IncorrectReturnAddressOffset). */
    test9_emit8(&pc, 0x3cu);
    test9_emit8(&pc, 0xa8u); /* TAY */
    test9_emit8(&pc, 0xb1u); /* LDA ($00),Y */
    test9_emit8(&pc, 0x00u);
    test9_emit8(&pc, 0x85u); /* STA $20 */
    test9_emit8(&pc, 0x20u);
    test9_emit8(&pc, 0xc8u); /* INY */
    test9_emit8(&pc, 0xb1u); /* LDA ($00),Y */
    test9_emit8(&pc, 0x00u);
    test9_emit8(&pc, 0x85u); /* STA $21 */
    test9_emit8(&pc, 0x21u);
    test9_emit8(&pc, 0xa5u); /* Page-cross compensation, verbatim timing shape. */
    test9_emit8(&pc, 0x00u);
    test9_emit8(&pc, 0xc9u);
    test9_emit8(&pc, 0xffu);
    rom_abort_emit_rel(&pc, 0xd0u, (uint16_t)(pc + 2u));
    rom_abort_emit_rel(&pc, 0xd0u, (uint16_t)(pc + 2u));
    test9_emit8(&pc, 0xc9u);
    test9_emit8(&pc, 0xfeu);
    rom_abort_emit_rel(&pc, 0xd0u, (uint16_t)(pc + 2u));
    test9_emit8(&pc, 0xa5u); /* LDA $20 */
    test9_emit8(&pc, 0x20u);
    test9_emit8(&pc, 0x29u); /* AND #$0F */
    test9_emit8(&pc, 0x0fu);
    test9_emit8(&pc, 0x49u); /* EOR #$0F */
    test9_emit8(&pc, 0x0fu);
    test9_emit8(&pc, 0x85u); /* STA $00 */
    test9_emit8(&pc, 0x00u);
    test9_emit8(&pc, 0xa9u); /* LDA #HIGH(CSTable) */
    test9_emit8(&pc, 0xfdu);
    test9_emit8(&pc, 0x85u); /* STA $01 */
    test9_emit8(&pc, 0x01u);
    test9_emit8(&pc, 0x6cu); /* JMP ($0000) */
    test9_emit16(&pc, 0u);

    cstable_sync = pc;
    rom_abort_emit_clock_tables(cstable_sync);
    test9_emit8(&pc, 0xa5u); /* LDA $20 */
    test9_emit8(&pc, 0x20u);
    test9_emit8(&pc, 0x4au); /* LSR A, four times. */
    test9_emit8(&pc, 0x4au);
    test9_emit8(&pc, 0x4au);
    test9_emit8(&pc, 0x4au);
    test9_emit8(&pc, 0xaau); /* TAX */
    test9_emit8(&pc, 0x4cu); /* JMP CSXMid */
    uint16_t csx_mid_jmp_operand = pc;
    test9_emit16(&pc, 0u);
    csx_loop = pc;
    test9_emit8(&pc, 0x08u); /* PHP */
    test9_emit8(&pc, 0x28u); /* PLP */
    test9_emit8(&pc, 0xeau); /* NOP */
    test9_emit8(&pc, 0xeau); /* NOP */
    csx_mid = pc;
    test9_emit8(&pc, 0xcau);                  /* DEX */
    rom_abort_emit_rel(&pc, 0x10u, csx_loop); /* BPL CSXLoop */

    test9_emit8(&pc, 0xa6u); /* LDX $21 */
    test9_emit8(&pc, 0x21u);
    test9_emit8(&pc, 0x4cu); /* JMP CSZYLoop_Mid */
    uint16_t cszy_mid_jmp_operand = pc;
    test9_emit16(&pc, 0u);
    cszy_loop = pc;
    test9_emit8(&pc, 0xa0u); /* LDY #49 */
    test9_emit8(&pc, 49u);
    cszy_249 = pc;
    test9_emit8(&pc, 0x88u);                  /* DEY */
    rom_abort_emit_rel(&pc, 0xd0u, cszy_249); /* BNE CSWaste249Cy */
    test9_emit8(&pc, 0xa5u);                  /* LDA $00 */
    test9_emit8(&pc, 0x00u);
    cszy_mid = pc;
    test9_emit8(&pc, 0xcau);                   /* DEX */
    test9_emit8(&pc, 0xeau);                   /* NOP */
    rom_abort_emit_rel(&pc, 0xd0u, cszy_loop); /* BNE CSZYLoop */

    test9_emit8(&pc, 0xa2u); /* LDX #18 */
    test9_emit8(&pc, 18u);
    cs94_loop = pc;
    test9_emit8(&pc, 0xcau);                   /* DEX */
    rom_abort_emit_rel(&pc, 0xd0u, cs94_loop); /* BNE CSWaste94Cy */
    test9_emit8(&pc, 0xa5u);                   /* LDA $00 */
    test9_emit8(&pc, 0x00u);
    test9_emit8(&pc, 0xa6u); /* LDX $FD */
    test9_emit8(&pc, 0xfdu);
    test9_emit8(&pc, 0xa4u); /* LDY $FE */
    test9_emit8(&pc, 0xfeu);
    test9_emit8(&pc, 0xa5u); /* LDA $FF */
    test9_emit8(&pc, 0xffu);
    test9_emit8(&pc, 0x60u); /* RTS */

    test9_put((uint16_t)(csx_mid_jmp_operand), (uint8_t)csx_mid);
    test9_put((uint16_t)(csx_mid_jmp_operand + 1u), (uint8_t)(csx_mid >> 8));
    test9_put((uint16_t)(cszy_mid_jmp_operand), (uint8_t)cszy_mid);
    test9_put((uint16_t)(cszy_mid_jmp_operand + 1u), (uint8_t)(cszy_mid >> 8));
}

static void rom_abort_emit_measurement_helpers(void)
{
    const uint16_t dmasync50 = 0x8400u;
    const uint16_t word_slide = 0x8600u;
    const uint16_t clock500 = 0x8540u;
    const uint16_t duration = 0x8700u;
    const uint16_t cycle_begin = 0x8740u;
    const uint16_t cycle_end = 0x8780u;
    const uint16_t check_dma = 0x87b0u;
    uint16_t pc;

    rom_abort_emit_clockslide500();
    rom_abort_emit_phase_slides();
    rom_abort_emit_clockslide_from_word(word_slide);

    pc = dmasync50;
    test9_emit_jsr(&pc, 0x8200u);
    rom_abort_emit_word_slide(&pc, word_slide, 350u);
    test9_emit8(&pc, 0x60u);

    pc = duration;
    test9_emit8(&pc, 0xa0u); /* LDY #0 */
    test9_emit8(&pc, 0u);
    test9_emit_jsr(&pc, (uint16_t)(0xff00u + 5u)); /* Clockslide_45. */
    uint16_t duration_loop = pc;
    test9_emit_abs(&pc, 0xadu, 0x4000u); /* LDA $4000 */
    rom_abort_emit_rel(&pc, 0xf0u, 0u);  /* Patched to CalculateDMADuration_End. */
    uint16_t duration_end_operand = (uint16_t)(pc - 1u);
    rom_abort_emit_word_slide(&pc, word_slide, 560u);
    test9_emit8(&pc, 0xc8u);                   /* INY */
    test9_emit_abs(&pc, 0x4cu, duration_loop); /* JMP CalculateDMADuration_Loop. */
    uint16_t duration_end = pc;
    test9_emit8(&pc, 0x60u);
    test9_put(duration_end_operand,
              (uint8_t)(duration_end - (uint16_t)(duration_end_operand + 1u)));

    pc = cycle_begin;
    test9_emit_jsr(&pc, dmasync50);
    test9_emit8(&pc, 0xa9u);
    test9_emit8(&pc, 0x4eu);
    test9_emit_abs(&pc, 0x8du, 0x4010u);
    test9_emit_jsr(&pc, (uint16_t)(0xff00u + 6u)); /* Clockslide_44. */
    test9_emit_jsr(&pc, clock500);
    test9_emit_jsr(&pc, 0xff00u);                   /* Clockslide_50. */
    test9_emit_jsr(&pc, (uint16_t)(0xff00u + 34u)); /* Clockslide_16. */
    test9_emit8(&pc, 0x60u);

    pc = cycle_end;
    test9_emit8(&pc, 0x78u); /* SEI */
    test9_emit_jsr(&pc, clock500);
    for (unsigned i = 0u; i < 4u; ++i)
        test9_emit8(&pc, 0xeau); /* Four NOPs in the pinned helper. */
    test9_emit_jsr(&pc, duration);
    test9_emit8(&pc, 0x60u);

    pc = check_dma;
    test9_emit_jsr(&pc, cycle_begin);
    test9_emit8(&pc, 0xeau);
    test9_emit8(&pc, 0xeau);
    test9_emit_jsr(&pc, cycle_end);
    test9_emit8(&pc, 0x60u);
}

static void rom_abort_emit_header(uint16_t main_pc)
{
    uint16_t pc = 0x8000u;
    test9_emit_jsr(&pc, 0x8200u); /* Real open-bus pretest and first DMC sync. */
    test9_emit_abs(&pc, 0x4cu, main_pc);
}

static void rom_abort_emit_case_tail(uint16_t *pc, uint16_t output_address, uint16_t duration_pc,
                                     struct rom_abort_sweep_location *location, const char *name)
{
    location->output_address = output_address;
    location->name = name;
    /* JSR CalculateDMADuration returns directly to this store instruction. */
    test9_emit_jsr(pc, duration_pc);
    location->duration_return_pc = *pc;
    if (output_address < 0x100u) {
        test9_emit8(pc, 0x94u); /* STY zp,X */
        test9_emit8(pc, (uint8_t)output_address);
    } else {
        test9_emit8(pc, 0x98u);                    /* TYA */
        test9_emit_abs(pc, 0x9du, output_address); /* STA abs,X */
    }
}

static uint16_t rom_abort_emit_explicit_program(void)
{
    uint16_t pc = 0x9000u;
    uint16_t loop, branch_operand;

    test9_emit_jsr(&pc, 0x87b0u); /* CheckDMATiming. */
    rom_abort_check_return_pc = pc;
    test9_emit8(&pc, 0xc0u); /* CPY #4 */
    test9_emit8(&pc, 4u);
    test9_emit8(&pc, 0xa2u); /* LDX #0 */
    test9_emit8(&pc, 0u);
    loop = pc;
    rom_abort_locations[ROM_ABORT_EXPLICIT].loop_pc = loop;
    rom_abort_locations[ROM_ABORT_EXPLICIT].output_address = 0x0050u;
    rom_abort_locations[ROM_ABORT_EXPLICIT].name = "explicit";
    test9_emit_jsr(&pc, 0x8400u); /* DMASync_50CyclesRemaining. */
    test9_emit8(&pc, 0xa9u);
    test9_emit8(&pc, 0x4eu);
    test9_emit_abs(&pc, 0x8du, 0x4010u);
    test9_emit_jsr(&pc, 0xff00u); /* Clockslide_50. */
    test9_emit_jsr(&pc, 0x8540u); /* Clockslide_500. */
    test9_emit8(&pc, 0x8au);      /* TXA */
    test9_emit_jsr(&pc, 0x8580u); /* Clockslide64_Minus_A. */
    test9_emit8(&pc, 0xeau);      /* NOP */
    test9_emit8(&pc, 0xa9u);      /* LDA #0 */
    test9_emit8(&pc, 0u);
    test9_emit_abs(&pc, 0x8du, 0x4015u); /* STA $4015 */
    test9_emit8(&pc, 0x8au);             /* TXA */
    test9_emit_jsr(&pc, 0x85a0u);        /* Clockslide37_Plus_A. */
    test9_emit_jsr(&pc, 0x8540u);
    test9_emit_jsr(&pc, (uint16_t)(0xff00u + 27u)); /* Clockslide_23. */
    test9_emit8(&pc, 0xa5u);                        /* LDA $00 */
    test9_emit8(&pc, 0u);
    test9_emit8(&pc, 0xa9u); /* LDA #$10 */
    test9_emit8(&pc, 0x10u);
    test9_emit_abs(&pc, 0x8du, 0x4015u);
    test9_emit_jsr(&pc, 0x8540u);
    test9_emit_jsr(&pc, (uint16_t)(0xff00u + 35u)); /* Clockslide_15. */
    rom_abort_emit_case_tail(&pc, 0x0050u, 0x8700u, &rom_abort_locations[ROM_ABORT_EXPLICIT],
                             "explicit");
    test9_emit8(&pc, 0xe8u); /* INX */
    test9_emit8(&pc, 0xe0u); /* CPX #$10 */
    test9_emit8(&pc, 0x10u);
    test9_emit8(&pc, 0xd0u);
    branch_operand = pc;
    test9_emit8(&pc, 0u);
    test9_put(branch_operand, (uint8_t)(loop - (uint16_t)(branch_operand + 1u)));
    test9_emit8(&pc, 0xe6u); /* INC ErrorCode, as in the ROM. */
    test9_emit8(&pc, 0x11u);
    rom_abort_emit_compare_loop(&pc, 0x0050u, 0xa000u);
    rom_abort_done_pc = pc;
    test9_emit_abs(&pc, 0x4cu, rom_abort_done_pc);
    return 0x9000u;
}

static void rom_abort_emit_compare_loop(uint16_t *pc, uint16_t result, uint16_t key)
{
    uint16_t loop;
    uint16_t branch_operand;
    test9_emit8(pc, 0xa2u); /* LDX #0 */
    test9_emit8(pc, 0u);
    loop = *pc;
    if (result < 0x100u) {
        test9_emit8(pc, 0xb5u); /* LDA zp,X */
        test9_emit8(pc, (uint8_t)result);
    } else
        test9_emit_abs(pc, 0xbdu, result);
    test9_emit_abs(pc, 0xddu, key); /* CMP key,X */
    test9_emit8(pc, 0xeau);         /* Two-cycle stand-in for a passing BNE's not-taken path. */
    test9_emit8(pc, 0xe8u);         /* INX */
    test9_emit8(pc, 0xe0u);         /* CPX #$10 */
    test9_emit8(pc, 0x10u);
    test9_emit8(pc, 0xd0u);
    branch_operand = *pc;
    test9_emit8(pc, 0u);
    test9_put(branch_operand, (uint8_t)(loop - (uint16_t)(branch_operand + 1u)));
}

static const uint8_t rom_abort_explicit_key[16] = {4u, 4u, 4u, 4u, 4u, 4u, 3u, 4u,
                                                   1u, 1u, 0u, 0u, 0u, 0u, 0u, 0u};
static const uint8_t rom_abort_implicit_key1[2][16] = {
    {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 1u, 1u, 0u, 0u, 0u, 0u},
    {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 4u, 4u, 1u, 1u, 0u, 0u, 0u, 0u}};
static const uint8_t rom_abort_implicit_key2[2][16] = {
    {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 1u, 0u, 0u, 0u, 0u, 0u},
    {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 4u, 4u, 1u, 0u, 0u, 0u, 0u, 0u}};
static const uint8_t rom_abort_implicit_key3[2][16] = {
    {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 4u, 4u, 4u, 4u, 4u, 4u},
    {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 4u, 4u, 4u, 4u, 4u, 4u, 4u, 4u}};

static void rom_abort_emit_key_tables(void)
{
    static const uint16_t addresses[7] = {0xa000u, 0xa010u, 0xa020u, 0xa030u,
                                          0xa040u, 0xa050u, 0xa060u};
    for (unsigned i = 0u; i < 16u; ++i) {
        rom_abort_put((uint16_t)(addresses[0] + i), rom_abort_explicit_key[i]);
        rom_abort_put((uint16_t)(addresses[1] + i), rom_abort_implicit_key1[0][i]);
        rom_abort_put((uint16_t)(addresses[2] + i), rom_abort_implicit_key2[0][i]);
        rom_abort_put((uint16_t)(addresses[3] + i), rom_abort_implicit_key3[0][i]);
        rom_abort_put((uint16_t)(addresses[4] + i), rom_abort_implicit_key1[1][i]);
        rom_abort_put((uint16_t)(addresses[5] + i), rom_abort_implicit_key2[1][i]);
        rom_abort_put((uint16_t)(addresses[6] + i), rom_abort_implicit_key3[1][i]);
    }
}

static void rom_abort_emit_implicit_sweep(uint16_t *pc, enum rom_abort_sweep_kind kind)
{
    uint16_t loop;
    uint16_t branch_operand;
    uint16_t out = kind == ROM_ABORT_IMPLICIT1   ? 0x0500u
                   : kind == ROM_ABORT_IMPLICIT2 ? 0x0520u
                                                 : 0x0540u;
    struct rom_abort_sweep_location *location = &rom_abort_locations[kind];

    test9_emit8(pc, 0xa2u); /* LDX #0 */
    test9_emit8(pc, 0u);
    loop = *pc;
    location->loop_pc = loop;
    location->name = kind == ROM_ABORT_IMPLICIT1   ? "implicit1"
                     : kind == ROM_ABORT_IMPLICIT2 ? "implicit2"
                                                   : "implicit3";
    location->output_address = out;

    test9_emit_jsr(pc, 0x8400u); /* DMASync_50CyclesRemaining. */
    test9_emit8(pc, 0xa9u);      /* LDA #0; STA $4015 */
    test9_emit8(pc, 0u);
    test9_emit_abs(pc, 0x8du, 0x4015u);
    test9_emit8(pc, 0xa9u); /* The first two loops stop looping; loop 3 stays active. */
    test9_emit8(pc, kind == ROM_ABORT_IMPLICIT3 ? 0x4eu : 0x0eu);
    test9_emit_abs(pc, 0x8du, 0x4010u);
    test9_emit_jsr(pc, (uint16_t)(0xff00u + 6u)); /* Clockslide_44. */
    test9_emit8(pc, 0x8au);                       /* TXA */
    test9_emit_jsr(pc, 0x8580u);                  /* Clockslide64_Minus_A. */
    test9_emit_jsr(pc, 0x8540u);                  /* Clockslide_500. */

    if (kind == ROM_ABORT_IMPLICIT2) {
        test9_emit8(pc, 0xeau); /* NOP */
        test9_emit8(pc, 0xa9u); /* LDA #$10 */
        test9_emit8(pc, 0x10u);
        test9_emit_abs(pc, 0x8du, 0x4015u); /* A one-cycle halt on a write is canceled. */
        test9_emit8(pc, 0x8au);             /* TXA */
        test9_emit_jsr(pc, 0x85a0u);        /* Clockslide37_Plus_A. */
        test9_emit_jsr(pc, 0x8540u);
        test9_emit_jsr(pc, (uint16_t)(0xff00u + 32u)); /* Clockslide_18. */
        test9_emit8(pc, 0xa5u);                        /* LDA $00 */
        test9_emit8(pc, 0u);
        test9_emit8(pc, 0xa9u); /* LDA #$4E */
        test9_emit8(pc, 0x4eu);
        test9_emit_abs(pc, 0x8du, 0x4010u);
        test9_emit8(pc, 0xa9u); /* LDA #$10 */
        test9_emit8(pc, 0x10u);
        test9_emit_abs(pc, 0x8du, 0x4015u);
        test9_emit_jsr(pc, 0x8540u);
        test9_emit_jsr(pc, (uint16_t)(0xff00u + 35u)); /* Clockslide_15. */
    } else {
        test9_emit8(pc, 0xeau); /* NOP */
        test9_emit8(pc, 0xa9u); /* LDA #$10 */
        test9_emit8(pc, 0x10u);
        test9_emit_abs(pc, 0x8du, 0x4015u);
        if (kind == ROM_ABORT_IMPLICIT1) {
            test9_emit8(pc, 0xeau); /* Three NOPs before the re-alignment. */
            test9_emit8(pc, 0xeau);
            test9_emit8(pc, 0xeau);
            test9_emit8(pc, 0x8au); /* TXA */
            test9_emit_jsr(pc, 0x8540u);
            test9_emit_jsr(pc, 0x85a0u);                   /* Clockslide37_Plus_A. */
            test9_emit_jsr(pc, (uint16_t)(0xff00u + 27u)); /* Clockslide_23. */
            test9_emit8(pc, 0xa5u);                        /* LDA $00 */
            test9_emit8(pc, 0u);
            test9_emit8(pc, 0xa9u); /* LDA #$4E */
            test9_emit8(pc, 0x4eu);
            test9_emit_abs(pc, 0x8du, 0x4010u);
            test9_emit8(pc, 0xa9u); /* LDA #$10 */
            test9_emit8(pc, 0x10u);
            test9_emit_abs(pc, 0x8du, 0x4015u);
            test9_emit_jsr(pc, 0x8540u);
            test9_emit8(pc, 0xeau); /* Two NOPs before CalculateDMADuration. */
            test9_emit8(pc, 0xeau);
        } else {
            test9_emit8(pc, 0xeau); /* Same three NOPs and alignment as loop1. */
            test9_emit8(pc, 0xeau);
            test9_emit8(pc, 0xeau);
            test9_emit8(pc, 0x8au); /* TXA */
            test9_emit_jsr(pc, 0x8540u);
            test9_emit_jsr(pc, 0x85a0u);                   /* Clockslide37_Plus_A. */
            test9_emit_jsr(pc, (uint16_t)(0xff00u + 27u)); /* Clockslide_23. */
            test9_emit8(pc, 0xa5u);                        /* LDA $00 */
            test9_emit8(pc, 0u);
            test9_emit8(pc, 0xa9u); /* LDA #$4E */
            test9_emit8(pc, 0x4eu);
            test9_emit_abs(pc, 0x8du, 0x4010u);
            test9_emit8(pc, 0xa9u); /* LDA #$10 */
            test9_emit8(pc, 0x10u);
            test9_emit_abs(pc, 0x8du, 0x4015u);
            test9_emit_jsr(pc, 0x8540u);
            test9_emit8(pc, 0xa5u); /* LDA $00 */
            test9_emit8(pc, 0u);
        }
    }

    rom_abort_emit_case_tail(pc, out, 0x8700u, location, location->name);
    test9_emit8(pc, 0xe8u); /* INX */
    test9_emit8(pc, 0xe0u); /* CPX #$10 */
    test9_emit8(pc, 0x10u);
    test9_emit8(pc, 0xd0u); /* BNE loop */
    branch_operand = *pc;
    test9_emit8(pc, 0u);
    test9_put(branch_operand, (uint8_t)(loop - (uint16_t)(branch_operand + 1u)));
    test9_emit8(pc, 0xe6u); /* INC ErrorCode. */
    test9_emit8(pc, 0x11u);
}

static uint16_t rom_abort_emit_implicit_program(void)
{
    uint16_t pc = 0x9000u;
    uint16_t alternate_branch_operand;
    uint16_t skip_alternate_operand;
    uint16_t alternate_pc;
    uint16_t done_pc;

    test9_emit_jsr(&pc, 0x87b0u); /* CheckDMATiming. */
    rom_abort_check_return_pc = pc;
    rom_abort_emit_implicit_sweep(&pc, ROM_ABORT_IMPLICIT1);
    rom_abort_emit_implicit_sweep(&pc, ROM_ABORT_IMPLICIT2);
    rom_abort_emit_implicit_sweep(&pc, ROM_ABORT_IMPLICIT3);

    test9_emit_abs(&pc, 0xadu, 0x0508u); /* LDA $508 */
    test9_emit8(&pc, 0xc9u);             /* CMP #4 */
    test9_emit8(&pc, 4u);
    test9_emit8(&pc, 0xf0u); /* BEQ alternate accepted profile. */
    alternate_branch_operand = pc;
    test9_emit8(&pc, 0u);

    rom_abort_emit_compare_loop(&pc, 0x0500u, 0xa010u);
    rom_abort_emit_compare_loop(&pc, 0x0520u, 0xa020u);
    rom_abort_emit_compare_loop(&pc, 0x0540u, 0xa030u);
    test9_emit_abs(&pc, 0x4cu, 0u); /* Skip the alternate key path. */
    skip_alternate_operand = (uint16_t)(pc - 2u);

    alternate_pc = pc;
    rom_abort_emit_compare_loop(&pc, 0x0500u, 0xa040u);
    rom_abort_emit_compare_loop(&pc, 0x0520u, 0xa050u);
    rom_abort_emit_compare_loop(&pc, 0x0540u, 0xa060u);
    done_pc = pc;
    test9_emit_abs(&pc, 0x4cu, done_pc);

    test9_put(alternate_branch_operand,
              (uint8_t)(alternate_pc - (uint16_t)(alternate_branch_operand + 1u)));
    test9_put(skip_alternate_operand, (uint8_t)done_pc);
    test9_put((uint16_t)(skip_alternate_operand + 1u), (uint8_t)(done_pc >> 8));
    rom_abort_done_pc = done_pc;
    return 0x9000u;
}

static void rom_abort_emit_test_program(void)
{
    memset(landing_prg, 0xeau, sizeof landing_prg);
    test9_emit_dmasync();
    rom_abort_emit_measurement_helpers();
    rom_abort_emit_key_tables();
    rom_abort_emit_header(0x9000u);
    rom_abort_put(0x3fc0u, 0u); /* The real source reads $FFC0 from DMASync. */
}

static uint8_t rom_abort_expected(enum rom_abort_sweep_kind kind, uint8_t index)
{
    if (kind == ROM_ABORT_EXPLICIT)
        return rom_abort_explicit_key[index];
    unsigned profile = rom_abort_implicit_late != 0u ? 1u : 0u;
    if (kind == ROM_ABORT_IMPLICIT1)
        return rom_abort_implicit_key1[profile][index];
    if (kind == ROM_ABORT_IMPLICIT2)
        return rom_abort_implicit_key2[profile][index];
    return rom_abort_implicit_key3[profile][index];
}

static void rom_abort_print_case_trace(enum rom_abort_sweep_kind kind, uint8_t index,
                                       uint8_t actual, uint8_t expected, uint64_t start_cycle)
{
    unsigned printed = 0u;
    unsigned open_bus_reads = 0u;
    printf("  first mismatch %s[%u]: actual=%u expected=%u start-cycle=%llu events=%lu%s\n",
           rom_abort_locations[kind].name, (unsigned)index, (unsigned)actual, (unsigned)expected,
           (unsigned long long)start_cycle, (unsigned long)dma_trace_count,
           dma_trace_count == DMA_TRACE_CAPACITY ? " (trace capped)" : "");
    for (size_t i = 0u; i < dma_trace_count && printed < 96u; ++i) {
        const struct dma_event *event = &dma_trace[i];
        uint8_t request =
            (uint8_t)(event->kind == NESTURBATOR_DMA_APU_AFTER && event->pending != 0u && i != 0u &&
                      dma_trace[i - 1u].kind == NESTURBATOR_DMA_APU_BEFORE &&
                      dma_trace[i - 1u].pending == 0u);
        uint8_t apu_write = (uint8_t)(event->kind == NESTURBATOR_DMA_CPU_WRITE_COMPLETE &&
                                      (event->address == 0x4010u || event->address == 0x4015u));
        uint8_t duration_read = (uint8_t)(event->kind == NESTURBATOR_DMA_CPU_READ_COMPLETE &&
                                          event->address == 0x4000u && open_bus_reads < 12u
#if defined(ROM_ABORT_TRACE_CPU_PC)
                                          && event->cpu_pc >= 0x8700u && event->cpu_pc < 0x8780u
#endif
        );
        uint8_t dma_event = (uint8_t)(event->kind == NESTURBATOR_DMA_WRITE_RETRY ||
                                      event->kind == NESTURBATOR_DMA_HALT_ACCEPTED ||
                                      event->kind == NESTURBATOR_DMA_PARKED_HALT ||
                                      event->kind == NESTURBATOR_DMA_PARKED_DUMMY ||
                                      event->kind == NESTURBATOR_DMA_PARKED_ALIGN ||
                                      event->kind == NESTURBATOR_DMA_SAMPLE_GET ||
                                      event->kind == NESTURBATOR_DMA_RESUMED_READ);
        if (request != 0u) {
            printf("    cycle=%llu request phase=%s target=%s remaining=%u bits=%u "
                   "empty=%u counter=%u\n",
                   (unsigned long long)event->cpu_cycle, event->phase != 0u ? "GET" : "PUT",
                   event->halt_phase != 0u ? "GET" : "PUT", (unsigned)event->remaining,
                   (unsigned)event->bits, (unsigned)event->buffer_empty,
                   (unsigned)event->dmc_counter);
            printed++;
        } else if (apu_write != 0u) {
            printf("    cycle=%llu write %04x=%02x phase=%s pending=%u target=%s "
                   "remaining=%u abort=%u load-wait=%u empty=%u\n",
                   (unsigned long long)event->cpu_cycle, (unsigned)event->address,
                   (unsigned)event->data, event->phase != 0u ? "GET" : "PUT",
                   (unsigned)event->pending, event->halt_phase != 0u ? "GET" : "PUT",
                   (unsigned)event->remaining, (unsigned)event->abort_delay,
                   (unsigned)event->load_waiting, (unsigned)event->buffer_empty);
            printed++;
        } else if (dma_event != 0u &&
                   (event->kind != NESTURBATOR_DMA_HALT_ATTEMPT || event->pending != 0u)) {
            printf("    cycle=%llu %-15s addr=%04x data=%02x phase=%s target=%s "
                   "pending=%u remaining=%u abort=%u\n",
                   (unsigned long long)event->cpu_cycle,
                   dma_event_name((enum dma_event_kind)event->kind), (unsigned)event->address,
                   (unsigned)event->data, event->phase != 0u ? "GET" : "PUT",
                   event->halt_phase != 0u ? "GET" : "PUT", (unsigned)event->pending,
                   (unsigned)event->remaining, (unsigned)event->abort_delay);
            printed++;
        } else if (duration_read != 0u) {
            printf("    cycle=%llu CalculateDMADuration read $4000=%02x phase=%s pending=%u "
                   "remaining=%u",
                   (unsigned long long)event->cpu_cycle, (unsigned)event->data,
                   event->phase != 0u ? "GET" : "PUT", (unsigned)event->pending,
                   (unsigned)event->remaining);
#if defined(ROM_ABORT_TRACE_CPU_PC)
            printf(" pc=%04x", (unsigned)event->cpu_pc);
#endif
            putchar('\n');
            open_bus_reads++;
        }
    }
    if (printed == 96u)
        puts("    trace event display capped at 96 relevant events");
#if defined(ROM_ABORT_TRACE_CPU_PC)
    size_t duration_reads = 0u;
    for (size_t i = 0u; i < dma_trace_count; ++i) {
        if (dma_trace[i].kind == NESTURBATOR_DMA_CPU_READ_COMPLETE &&
            dma_trace[i].address == 0x4000u && dma_trace[i].cpu_pc == 0x8708u)
            duration_reads++;
    }
    size_t skip = duration_reads > 12u ? duration_reads - 12u : 0u;
    size_t seen = 0u;
    printf("    CalculateDMADuration recorded $4000 reads=%lu; last reads:",
           (unsigned long)duration_reads);
    for (size_t i = 0u; i < dma_trace_count; ++i) {
        const struct dma_event *event = &dma_trace[i];
        if (event->kind != NESTURBATOR_DMA_CPU_READ_COMPLETE || event->address != 0x4000u ||
            event->cpu_pc != 0x8708u)
            continue;
        if (seen++ < skip)
            continue;
        printf(" %llu:%02x", (unsigned long long)event->cpu_cycle, (unsigned)event->data);
    }
    putchar('\n');
#endif
}

static void rom_abort_print_array(enum rom_abort_sweep_kind kind, const uint8_t *values,
                                  unsigned profile)
{
    printf("%s actual:", rom_abort_locations[kind].name);
    for (unsigned i = 0u; i < 16u; ++i)
        printf(" %u", (unsigned)values[i]);
    if (kind == ROM_ABORT_EXPLICIT) {
        printf("\n%s key:   ", rom_abort_locations[kind].name);
        for (unsigned i = 0u; i < 16u; ++i)
            printf(" %u", (unsigned)rom_abort_explicit_key[i]);
    } else {
        printf("\n%s key(%s):", rom_abort_locations[kind].name,
               profile != 0u ? "later accepted" : "early accepted");
        for (unsigned i = 0u; i < 16u; ++i)
            printf(" %u", (unsigned)rom_abort_expected(kind, (uint8_t)i));
    }
    putchar('\n');
}

static void rom_abort_run_guest(enum rom_abort_sweep_kind requested_kind, uint16_t entry_pc,
                                uint8_t wrong_immediate)
{
    nesturbator *powered = make_test9_powered_instance();
    if (powered == NULL)
        return;
    struct nesturbator normal = *powered;
    struct nesturbator observed = normal;
    uint64_t case_start_cycle = 0u;
    size_t case_start_step = 0u;
    size_t steps = 0u;
    uint8_t active = 0u;
    uint8_t active_index = 0u;
    uint8_t check_seen = 0u;
    unsigned completed[ROM_ABORT_SWEEP_COUNT] = {0u, 0u, 0u, 0u};

    CHECK_EQ_HEX(normal.cpu.pc, 0x8000u);
    CHECK_EQ_HEX(entry_pc, 0x9000u);
    dma_trace_count = 0u;
    while (normal.cpu.pc != rom_abort_done_pc && steps < 12000000u) {
        for (unsigned i = 0u; i < ROM_ABORT_SWEEP_COUNT; ++i) {
            if (rom_abort_locations[i].name != NULL &&
                normal.cpu.pc == rom_abort_locations[i].loop_pc) {
                active = 1u;
                active_index = normal.cpu.x;
                case_start_cycle = normal.cpu_cycle;
                case_start_step = steps;
                dma_trace_count = 0u;
                break;
            }
        }
        if (normal.cpu.pc == rom_abort_check_return_pc) {
            check_seen = 1u;
            printf("%s CheckDMATiming result=%u (expected 4)\n",
                   requested_kind == ROM_ABORT_EXPLICIT ? "explicit" : "implicit",
                   (unsigned)normal.cpu.y);
            printf("DMASync open-bus pretest result=%u (1 means aligned open-bus path)\n",
                   (unsigned)normal.bus.ram[0x12u]);
            CHECK_EQ_U64(normal.cpu.y, 4u);
        }
        if (wrong_immediate == 0u)
            nesturbator__cpu_step(&normal);
        test_observed_cpu_step(&observed);
        if (wrong_immediate != 0u)
            normal = observed;
        CHECK_EQ_HEX(observed.cpu.pc, normal.cpu.pc);
        CHECK_EQ_U64(observed.cpu_cycle, normal.cpu_cycle);
        steps++;
        if (normal.cpu.pc == 0u || normal.cpu.jammed != 0u) {
            fprintf(stderr, "abort sweep guest stopped at PC=%04x step=%lu cycle=%llu\n",
                    (unsigned)normal.cpu.pc, (unsigned long)steps,
                    (unsigned long long)normal.cpu_cycle);
            break;
        }
        if (active != 0u && steps - case_start_step > 1000000u) {
            fprintf(stderr, "abort sweep case exceeded one million CPU steps\n");
            CHECK(0);
            break;
        }
        if (active != 0u) {
            for (unsigned i = 0u; i < ROM_ABORT_SWEEP_COUNT; ++i) {
                if (rom_abort_locations[i].name == NULL ||
                    normal.cpu.pc != rom_abort_locations[i].duration_return_pc)
                    continue;
                uint8_t actual = normal.cpu.y;
                uint8_t expected;
                completed[i]++;
                if (i == ROM_ABORT_IMPLICIT1 && active_index == 8u)
                    rom_abort_implicit_late = (uint8_t)(actual == 4u);
                expected = rom_abort_expected((enum rom_abort_sweep_kind)i, active_index);
                if (actual != expected && rom_abort_first_mismatch[i] == 0xffu) {
                    rom_abort_first_mismatch[i] = active_index;
                    if (wrong_immediate == 0u)
                        rom_abort_print_case_trace((enum rom_abort_sweep_kind)i, active_index,
                                                   actual, expected, case_start_cycle);
                }
                active = 0u;
                break;
            }
        }
    }
    CHECK(steps < 12000000u);
    CHECK_EQ_HEX(normal.cpu.pc, rom_abort_done_pc);
    CHECK(check_seen != 0u);
    CHECK(memcmp(&normal, &observed, sizeof normal) == 0);
    if (wrong_immediate != 0u) {
        /* This wrong model consumes a fresh byte at the immediate output
           edge. The early-profile keys must reject its extra X8/X9 reloads. */
        CHECK_EQ_HEX(normal.bus.ram[0x508u], 0u);
        CHECK_EQ_HEX(normal.bus.ram[0x548u], 4u);
        CHECK_EQ_HEX(normal.bus.ram[0x549u], 4u);
        CHECK(rom_abort_first_mismatch[ROM_ABORT_IMPLICIT3] == 8u);
        puts("immediate-sample wrong model rejected at looping offsets8/9");
    } else if (requested_kind == ROM_ABORT_EXPLICIT) {
        CHECK_EQ_U64(completed[ROM_ABORT_EXPLICIT], 16u);
        uint8_t values[16];
        for (unsigned i = 0u; i < 16u; ++i)
            values[i] = normal.bus.ram[0x50u + i];
        rom_abort_print_array(ROM_ABORT_EXPLICIT, values, 0u);
        for (unsigned i = 0u; i < 16u; ++i)
            CHECK_EQ_HEX(values[i], rom_abort_explicit_key[i]);
    } else {
        CHECK_EQ_U64(completed[ROM_ABORT_IMPLICIT1], 16u);
        CHECK_EQ_U64(completed[ROM_ABORT_IMPLICIT2], 16u);
        CHECK_EQ_U64(completed[ROM_ABORT_IMPLICIT3], 16u);
        rom_abort_implicit_late = (uint8_t)(normal.bus.ram[0x508u] == 4u);
        CHECK_EQ_U64(rom_abort_implicit_late, 0u); /* Declared early-G model. */
        uint8_t values1[16], values2[16], values3[16];
        for (unsigned i = 0u; i < 16u; ++i) {
            values1[i] = normal.bus.ram[0x500u + i];
            values2[i] = normal.bus.ram[0x520u + i];
            values3[i] = normal.bus.ram[0x540u + i];
        }
        rom_abort_print_array(ROM_ABORT_IMPLICIT1, values1, rom_abort_implicit_late);
        rom_abort_print_array(ROM_ABORT_IMPLICIT2, values2, rom_abort_implicit_late);
        rom_abort_print_array(ROM_ABORT_IMPLICIT3, values3, rom_abort_implicit_late);
        for (unsigned i = 0u; i < 16u; ++i) {
            CHECK_EQ_HEX(values1[i], rom_abort_implicit_key1[rom_abort_implicit_late][i]);
            CHECK_EQ_HEX(values2[i], rom_abort_implicit_key2[rom_abort_implicit_late][i]);
            CHECK_EQ_HEX(values3[i], rom_abort_implicit_key3[rom_abort_implicit_late][i]);
        }
        printf("implicit accepted branch: %s\n",
               rom_abort_implicit_late != 0u ? "later RP2A03G/H table" : "early table");
    }
    printf("%s guest steps=%lu cpu-cycles=%llu final-PC=%04x\n",
           requested_kind == ROM_ABORT_EXPLICIT ? "explicit" : "implicit", (unsigned long)steps,
           (unsigned long long)normal.cpu_cycle, (unsigned)normal.cpu.pc);
    nesturbator_destroy(powered);
}

static void test_dma_abort_rom_sweeps(void)
{
    rom_abort_implicit_late = 0u;
    memset(rom_abort_first_mismatch, 0xff, sizeof rom_abort_first_mismatch);
    memset(rom_abort_locations, 0, sizeof rom_abort_locations);

    rom_abort_emit_test_program();
    uint16_t explicit_entry = rom_abort_emit_explicit_program();
    rom_abort_run_guest(ROM_ABORT_EXPLICIT, explicit_entry, 0u);

    memset(rom_abort_first_mismatch, 0xff, sizeof rom_abort_first_mismatch);
    memset(rom_abort_locations, 0, sizeof rom_abort_locations);
    rom_abort_emit_test_program();
    uint16_t implicit_entry = rom_abort_emit_implicit_program();
    rom_abort_run_guest(ROM_ABORT_IMPLICIT1, implicit_entry, 0u);

    memset(rom_abort_first_mismatch, 0xff, sizeof rom_abort_first_mismatch);
    dma_wrong_immediate_sample = 1u;
    rom_abort_run_guest(ROM_ABORT_IMPLICIT1, implicit_entry, 1u);
    dma_wrong_immediate_sample = 0u;
}

#endif
