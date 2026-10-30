/* INT-01 source path: an APU frame IRQ crosses the production bus sampler
   and reaches guest handler markers. Poll expectations follow the pinned
   AccuracyCoin TEST_IFlagLatency schedule and the cycle crosswalk in
   21-INTERRUPT-TIMING-ANNEX.md; they are not inferred from core output. */
#include <stdint.h>
#include <stdio.h>

#include "../interrupt_fixture.h"

#define CPU_FLAG_I 0x04u
#define IRQ_HANDLER_P 0x01fbu
#define IRQ_HANDLER_PCL 0x01fcu
#define IRQ_HANDLER_PCH 0x01fdu

static const uint8_t idle_program[] = {0xeau, 0xeau, 0xeau}; /* NOP; NOP; NOP */
static const uint8_t dmc_bus_program[] = {0x90u, 0x90u};     /* DMC GET supplies BCC $90. */
static const uint8_t dmc_open_bus_program[] = {0x5au};       /* AccuracyCoin D sample byte. */
static const uint8_t cli_program[] = {0x58u, 0xe8u, 0xeau};  /* CLI; INX; NOP */
static const uint8_t cli_sei_program[] = {0x58u, 0x78u, 0xe8u, 0xeau}; /* CLI; SEI; INX; NOP */
static const uint8_t sei_program[] = {0xeau, 0x78u, 0xe8u, 0xeau};     /* NOP; SEI; INX; NOP */
static const uint8_t plp_program[] = {0x28u, 0xe8u, 0xeau};            /* PLP; INX; NOP */
static const uint8_t nmi_handler[] = {
    0xe6u, INTERRUPT_FIXTURE_NMI_COUNT, /* INC NMI marker */
    0x40u                               /* RTI */
};
static const uint8_t irq_handler[] = {
    0xe6u, INTERRUPT_FIXTURE_IRQ_COUNT, /* INC IRQ marker */
    0xadu, 0x15u,
    0x40u, /* LDA $4015: acknowledge frame IRQ */
    0x40u  /* RTI */
};

static struct nesturbator *build_fixture(const uint8_t *program, size_t program_len, uint8_t status,
                                         uint32_t frame_cycle)
{
    struct nesturbator *nes = interrupt_fixture_build(
        program, program_len, nmi_handler, sizeof nmi_handler, irq_handler, sizeof irq_handler);
    CHECK(nes != NULL);
    if (nes != NULL)
        interrupt_fixture_start(nes, 0x8000u, status, frame_cycle);
    return nes;
}

/* AccuracyCoin IFlagLatency test 1's decisive source-to-handler path: frame
   IRQ rises on the first NOP's first bus action and is retained by the real
   source combiner and close-edge capture before ordinary cpu_step dispatch. */
static void test_real_apu_irq_reaches_handler(void)
{
    struct nesturbator *nes = build_fixture(idle_program, sizeof idle_program, 0x20u, 29827u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    CHECK_EQ_U64(nes->apu.frame_irq, 0u);
    CHECK_EQ_U64(nes->cpu.irq_line, 0u);
    nesturbator__cpu_step(nes);
    CHECK_EQ_U64(nes->apu.frame_irq, 1u);
    CHECK_EQ_U64(nes->cpu.irq_line, 1u);
    CHECK_EQ_U64(nes->cpu.irq_captured, 1u);
    CHECK_EQ_U64(nes->cpu.poll_latch, 1u);
    CHECK_EQ_HEX(nes->cpu.pc, 0x8001u);

    nesturbator__cpu_step(nes);
    CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_IRQ_VECTOR);
    CHECK_EQ_U64(nes->cpu.s, 0xfau);
    CHECK_EQ_HEX(nes->bus.ram[IRQ_HANDLER_PCH], 0x80u);
    CHECK_EQ_HEX(nes->bus.ram[IRQ_HANDLER_PCL], 0x01u);
    CHECK_EQ_HEX(nes->bus.ram[IRQ_HANDLER_P], 0x20u);
    CHECK_EQ_U64(nes->bus.ram[INTERRUPT_FIXTURE_IRQ_COUNT], 0u);

    nesturbator__cpu_step(nes);
    CHECK_EQ_U64(nes->bus.ram[INTERRUPT_FIXTURE_IRQ_COUNT], 1u);
    CHECK_EQ_HEX(nes->cpu.pc, (uint16_t)(INTERRUPT_FIXTURE_IRQ_VECTOR + 2u));
    printf(
        "CASE source frame-IRQ line=1 captured=1 poll=1 entry=$8200 stack=$8001/P=$20 marker=1\n");
    nesturbator_destroy((nesturbator *)nes);
}

/* The line is first captured at the end of CLI's last action. The CLI poll
   still sees old I=1, so the following INX is the first eligible poll. */
static void test_cli_old_i_blocks_current_poll(void)
{
    struct nesturbator *nes = build_fixture(cli_program, sizeof cli_program, 0x24u, 29826u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    nesturbator__cpu_step(nes); /* CLI: C1=29827, C2=29828 asserts and captures IRQ. */
    CHECK_EQ_U64(nes->apu.frame_irq, 1u);
    CHECK_EQ_U64(nes->cpu.irq_line, 1u);
    CHECK_EQ_U64(nes->cpu.irq_captured, 1u);
    CHECK_EQ_U64(nes->cpu.poll_latch, 0u);
    CHECK_EQ_HEX(nes->cpu.pc, 0x8001u);
    CHECK((nes->cpu.p & CPU_FLAG_I) == 0u);

    nesturbator__cpu_step(nes); /* The first eligible instruction executes before IRQ. */
    CHECK_EQ_HEX(nes->cpu.pc, 0x8002u);
    CHECK_EQ_U64(nes->cpu.x, 1u);
    CHECK_EQ_U64(nes->cpu.poll_latch, 1u);
    nesturbator__cpu_step(nes);
    CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_IRQ_VECTOR);
    CHECK_EQ_HEX(nes->bus.ram[IRQ_HANDLER_PCH], 0x80u);
    CHECK_EQ_HEX(nes->bus.ram[IRQ_HANDLER_PCL], 0x02u);
    CHECK_EQ_HEX(nes->bus.ram[IRQ_HANDLER_P], 0x20u);
    printf("CASE guard CLI-old-I=1 next-INX=1 return=$8002 pushed-P=$20\n");
    nesturbator_destroy((nesturbator *)nes);
}

/* AccuracyCoin IFlagLatency tests 3-4: a pending IRQ accepted by SEI's poll
   under old I=0 must survive the architectural I=1 update until dispatch. */
static void test_sei_old_i_acceptance(void)
{
    struct nesturbator *nes = build_fixture(sei_program, sizeof sei_program, 0x20u, 29826u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    nesturbator__cpu_step(nes); /* NOP: frame source reaches and is captured by production bus. */
    CHECK_EQ_U64(nes->apu.frame_irq, 1u);
    CHECK_EQ_U64(nes->cpu.irq_line, 1u);
    CHECK_EQ_U64(nes->cpu.irq_captured, 1u);
    CHECK_EQ_U64(nes->cpu.poll_latch, 0u);
    CHECK_EQ_HEX(nes->cpu.pc, 0x8001u);

    nesturbator__cpu_step(nes); /* SEI polls with old I=0, then sets architectural I. */
    CHECK_EQ_HEX(nes->cpu.pc, 0x8002u);
    CHECK((nes->cpu.p & CPU_FLAG_I) != 0u);
    CHECK_EQ_U64(nes->cpu.poll_latch, 1u);
    CHECK_EQ_U64(nes->cpu.irq_captured, 1u);

    nesturbator__cpu_step(nes);
    CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_IRQ_VECTOR);
    CHECK_EQ_U64(nes->cpu.x, 0u);
    CHECK_EQ_HEX(nes->bus.ram[IRQ_HANDLER_PCL], 0x02u);
    CHECK_EQ_HEX(nes->bus.ram[IRQ_HANDLER_P], 0x24u);
    nesturbator__cpu_step(nes);
    CHECK_EQ_U64(nes->bus.ram[INTERRUPT_FIXTURE_IRQ_COUNT], 1u);
    printf("CASE SEI-old-I=0 source-captured=1 accepted-through-dispatch=1\n");
    nesturbator_destroy((nesturbator *)nes);
}

/* AccuracyCoin test 5's first acceptance seam: CLI polls under old I=1,
   then SEI polls the captured real frame IRQ under old I=0 and sets I. */
static void test_cli_sei_preserves_prior_acceptance(void)
{
    struct nesturbator *nes = build_fixture(cli_sei_program, sizeof cli_sei_program, 0x24u, 29826u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    nesturbator__cpu_step(nes); /* CLI: source asserts on C2, old I rejects it. */
    CHECK_EQ_U64(nes->apu.frame_irq, 1u);
    CHECK_EQ_U64(nes->cpu.irq_line, 1u);
    CHECK_EQ_U64(nes->cpu.irq_captured, 1u);
    CHECK_EQ_U64(nes->cpu.poll_latch, 0u);
    CHECK_EQ_HEX(nes->cpu.pc, 0x8001u);
    CHECK((nes->cpu.p & CPU_FLAG_I) == 0u);

    nesturbator__cpu_step(nes); /* SEI uses old I=0 for eligibility. */
    CHECK_EQ_HEX(nes->cpu.pc, 0x8002u);
    CHECK((nes->cpu.p & CPU_FLAG_I) != 0u);
    CHECK_EQ_U64(nes->cpu.poll_latch, 1u);

    nesturbator__cpu_step(nes);
    CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_IRQ_VECTOR);
    CHECK_EQ_U64(nes->cpu.x, 0u);
    CHECK_EQ_HEX(nes->bus.ram[IRQ_HANDLER_PCH], 0x80u);
    CHECK_EQ_HEX(nes->bus.ram[IRQ_HANDLER_PCL], 0x02u);
    CHECK_EQ_HEX(nes->bus.ram[IRQ_HANDLER_P], 0x24u);
    printf("CASE source CLI-SEI accepted-before-SEI pushed-P=$24\n");
    if (nes->cpu.pc == INTERRUPT_FIXTURE_IRQ_VECTOR) {
        nesturbator__cpu_step(nes);
        CHECK_EQ_U64(nes->bus.ram[INTERRUPT_FIXTURE_IRQ_COUNT], 1u);
    }
    nesturbator_destroy((nesturbator *)nes);
}

/* Reconcile AccuracyCoin test E's DMC-provided BCC $90 and C3 $4015 read
   through the production bus. DMA parks the actual C1 fetch; its GET supplies
   the pinned byte and captures frame IRQ before that CPU fetch resumes. */
static void test_real_apu_dmc_branch_acknowledges_source_before_c4(void)
{
    struct nesturbator *nes = build_fixture(dmc_bus_program, sizeof dmc_bus_program, 0x20u, 29824u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    /* PUT halt, GET dummy, PUT alignment, GET data, resumed C1 on PUT.
       C2 is GET, C3 acknowledges on PUT and C4 clears on GET. */
    nes->bus.open_bus = 0x40u;
    nes->bus.apu_get_put_phase = 1u;
    nes->apu.pulse_clock_phase = 1u;
    nes->apu.dmc.reg[0] = 0x0fu;
    nes->apu.dmc.address = 0x8000u;
    nes->apu.dmc.remaining = 1u;
    nes->apu.dmc.buffer_empty = 1u;
    nes->apu.dmc.bits = 8u;
    nes->apu.dmc.counter = 100u;
    nes->apu.dmc.dma_pending = 1u;
    nes->apu.dmc.dma_halt_phase = 0u;
    nes->apu.dmc.enable_delay = 0u;
    nes->cpu.pc = 0x4013u;
    nesturbator__cpu_step(nes);
    CHECK_EQ_HEX(nes->apu.dmc.sample_buffer, 0x90u);
    CHECK_EQ_U64(nes->apu.dmc.remaining, 0u);
    CHECK_EQ_HEX(nes->cpu.pc, 0x3fa5u);
    CHECK_EQ_U64(nes->apu.frame_irq, 0u);
    CHECK_EQ_U64(nes->cpu.irq_line, 0u);
    CHECK_EQ_U64(nes->cpu.irq_captured, 0u);
    /* The C4 opening sample is the previous close-edge capture. It remains
       visible as poll history after the APU source and current capture fall. */
    CHECK_EQ_U64(nes->cpu.poll_latch, 1u);

    nesturbator__cpu_step(nes);
    CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_IRQ_VECTOR);
    CHECK_EQ_HEX(nes->bus.ram[IRQ_HANDLER_PCH], 0x3fu);
    CHECK_EQ_HEX(nes->bus.ram[IRQ_HANDLER_PCL], 0xa5u);
    CHECK_EQ_HEX(nes->bus.ram[IRQ_HANDLER_P], 0x20u);
    nesturbator__cpu_step(nes);
    CHECK_EQ_U64(nes->bus.ram[INTERRUPT_FIXTURE_IRQ_COUNT], 1u);
    CHECK_EQ_U64(nes->cpu.irq_line, 0u);
    printf("CASE source APU+DMC BCC=$90 C1-captured=1 C3-real-$4015=ack C4-prior-capture=1 line=0 "
           "entry=$8200 return=$3FA5/P=$20 marker=1\n");
    nesturbator_destroy((nesturbator *)nes);
}

/* AccuracyCoin C's open-bus prerequisite through the real PPU and bus. */
static void test_accuracycoin_c_ppu_open_bus_prerequisite(void)
{
    struct nesturbator *nes = build_fixture(idle_program, sizeof idle_program, 0x20u, 0u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    nesturbator__bus_write(nes, 0x2002u, 0x44u);
    uint8_t value = nesturbator__bus_read(nes, 0x3fa5u);
    CHECK_EQ_HEX(value, 0x44u);
    CHECK_EQ_HEX(nes->ppu.io_bus, 0x44u);
    printf("CASE prerequisite AccuracyCoin-C PPU-open-bus expected=$44 actual=$%02X\n", value);
    nesturbator_destroy((nesturbator *)nes);
}

/* AccuracyCoin D checks that DMC DMA reaches the CPU data bus. This drives
   the existing production DMA path on a parked $4000 read and keeps its
   sample in the owned NROM; it does not change or broaden DMA timing. */
static void test_accuracycoin_d_dmc_open_bus_prerequisite(void)
{
    struct nesturbator *nes =
        build_fixture(dmc_open_bus_program, sizeof dmc_open_bus_program, 0x20u, 0u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    nesturbator__bus_write(nes, 0x2002u, 0x5au);
    CHECK_EQ_HEX(nes->ppu.io_bus, 0x5au);
    nes->bus.open_bus = 0x40u;
    /* Reload eligibility is reached after the opening phase advances. */
    nes->bus.apu_get_put_phase = 1u;
    nes->apu.dmc.reg[0] = 0x0fu;
    nes->apu.dmc.address = 0x8000u;
    nes->apu.dmc.remaining = 1u;
    nes->apu.dmc.buffer_empty = 1u;
    nes->apu.dmc.bits = 8u;
    nes->apu.dmc.counter = 100u;
    nes->apu.dmc.dma_pending = 1u;
    nes->apu.dmc.dma_halt_phase = 0u;
    nes->apu.dmc.enable_delay = 0u;

    uint8_t value = nesturbator__bus_read(nes, 0x4000u);
    CHECK_EQ_HEX(value, 0x5au);
    CHECK_EQ_HEX(nes->apu.dmc.sample_buffer, 0x5au);
    CHECK_EQ_U64(nes->apu.dmc.remaining, 0u);
    CHECK_EQ_U64(nes->apu.dmc.dma_pending, 0u);
    printf("CASE prerequisite AccuracyCoin-D DMC-open-bus expected=$5A actual=$%02X sample=$%02X\n",
           value, nes->apu.dmc.sample_buffer);
    nesturbator_destroy((nesturbator *)nes);
}

/* The frame IRQ is sampled at C3 and the PLP poll opens at C4 while P still
   contains old I. The real $4015 source stays asserted for the following
   instruction so both old/new I transitions have an observable outcome. */
static void test_real_apu_plp_old_i_transitions(void)
{
    const uint8_t old_status[] = {0x24u, 0x20u};
    const uint8_t pulled_status[] = {0x20u, 0x24u};
    for (uint32_t i = 0u; i < 2u; i++) {
        struct nesturbator *nes =
            build_fixture(plp_program, sizeof plp_program, old_status[i], 29825u);
        CHECK(nes != NULL);
        if (nes == NULL)
            continue;
        nes->bus.ram[0x01feu] = pulled_status[i];

        nesturbator__cpu_step(nes); /* PLP C4 polls before installing pulled P. */
        CHECK_EQ_U64(nes->apu.frame_irq, 1u);
        CHECK_EQ_U64(nes->cpu.irq_line, 1u);
        CHECK_EQ_U64(nes->cpu.irq_captured, 1u);
        CHECK_EQ_U64(nes->cpu.poll_latch, 1u);
        CHECK_EQ_HEX(nes->cpu.pc, 0x8001u);
        CHECK_EQ_HEX(nes->cpu.p, pulled_status[i]);

        nesturbator__cpu_step(nes);
        if (i == 0u) {
            /* Old I=1 rejects PLP's poll; the first following INX runs and
               its legal poll accepts the still asserted source. */
            CHECK_EQ_HEX(nes->cpu.pc, 0x8002u);
            CHECK_EQ_U64(nes->cpu.x, 1u);
            nesturbator__cpu_step(nes);
            CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_IRQ_VECTOR);
            CHECK_EQ_U64(nes->cpu.s, 0xfbu);
            CHECK_EQ_HEX(nes->bus.ram[0x01feu], 0x80u);
            CHECK_EQ_HEX(nes->bus.ram[0x01fdu], 0x02u);
            CHECK_EQ_HEX(nes->bus.ram[0x01fcu], 0x20u);
        } else {
            /* Old I=0 accepts at PLP even though the pulled P sets I. */
            CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_IRQ_VECTOR);
            CHECK_EQ_U64(nes->cpu.s, 0xfbu);
            CHECK_EQ_HEX(nes->bus.ram[0x01feu], 0x80u);
            CHECK_EQ_HEX(nes->bus.ram[0x01fdu], 0x01u);
            CHECK_EQ_HEX(nes->bus.ram[0x01fcu], 0x24u);
        }
        printf("CASE source PLP-old-I=%u-new-I=%u\n", (old_status[i] & CPU_FLAG_I) != 0u,
               (pulled_status[i] & CPU_FLAG_I) != 0u);
        nesturbator_destroy((nesturbator *)nes);
    }
}

int main(void)
{
    test_real_apu_irq_reaches_handler();
    test_cli_old_i_blocks_current_poll();
    test_sei_old_i_acceptance();
    test_cli_sei_preserves_prior_acceptance();
    test_real_apu_dmc_branch_acknowledges_source_before_c4();
    test_accuracycoin_c_ppu_open_bus_prerequisite();
    test_accuracycoin_d_dmc_open_bus_prerequisite();
    test_real_apu_plp_old_i_transitions();
    CHECK_DONE();
}
