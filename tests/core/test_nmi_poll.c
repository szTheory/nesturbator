/* NMI-02: guest-visible instruction ordering and NMI control patterns.
   AccuracyCoin at 6bee919529289c555803b612b8c9e4f7eaaeeabb, TEST_NMI_Control
   (AccuracyCoin.asm:6521-6640), defines the STA $2000 and INC $2000 poll cases.
   Its README calls out the following-instruction marker for case 8 and the
   poll between the two INC writes for case 9. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../nmi_fixture.h"

#define NMI_COUNT 0x30u
#define NMI_ENTRY_X 0x31u
#define STA_MARKER 0x40u

static const uint8_t nmi_handler[] = {
    0xe6u, NMI_COUNT,       /* INC $30: independently record handler entry */
    0x8eu, NMI_ENTRY_X, 0u, /* STX $0031: record X at handler entry */
    0xe8u,                  /* INX: the pinned NMI Control marker */
    0x40u                   /* RTI */
};

/* Source-shaped Control 8: update a copy, STA $2000, then LDX #$10 and
   compare with $11. STX writes an additional observable marker after LDX. */
static const uint8_t sta_control_program[] = {
    0xa5u, 0x10u,             /* LDA $10 */
    0x09u, 0x80u,             /* ORA #$80 */
    0x85u, 0x10u,             /* STA $10 */
    0x8du, 0x00u,      0x20u, /* STA $2000 */
    0xa2u, 0x10u,             /* LDX #$10: source's following-instruction marker */
    0x8eu, STA_MARKER, 0x00u, /* STX $0040 */
    0xe0u, 0x11u,             /* CPX #$11 */
    0xd0u, 0x05u,             /* BNE $8016: source failure path */
    0xa9u, 0x01u,      0x85u, 0x41u, 0x02u, 0xa9u, 0xffu, 0x85u, 0x41u, 0x02u};

/* Source-shaped Control 9 bus sequence: prepare PPU I/O bus, then execute
   INC $2000. With value $80, the old-value write raises NMI before the final
   $81 write; the instruction's six bus cycles preserve both writes. */
static const uint8_t inc_control_program[] = {
    0xa9u, 0x80u,        /* LDA #$80 */
    0x8du, 0x02u, 0x20u, /* STA $2002: prepare the PPU I/O bus */
    0xeeu, 0x00u, 0x20u, /* INC $2000: both RMW writes are real bus cycles */
    0xa2u, 0x10u,        /* LDX #$10 */
    0xe0u, 0x11u,        /* CPX #$11 */
    0xf0u, 0x05u,        /* BEQ failure marker if NMI arrives after LDX */
    0xa9u, 0x01u, 0x85u, 0x42u, 0x02u, 0xa9u, 0xffu, 0x85u, 0x42u, 0x02u};

static struct nesturbator *poll_nes;

static struct nesturbator *build_poll_fixture(const uint8_t *program, size_t program_size)
{
    struct nesturbator *nes =
        nmi_fixture_build(program, program_size, nmi_handler, sizeof nmi_handler, 0x8200u);
    CHECK(nes != NULL);
    if (nes != NULL) {
        nes->cpu.pc = 0x8000u;
        nes->cpu.s = 0xfdu;
        nes->cpu.p = 0x20u;
        nes->cpu.x = 0u;
        nes->cpu.y = 0u;
        nes->cpu.a = 0u;
    }
    return nes;
}

static void run_sta_control_to_marker(void)
{
    unsigned steps = 0u;
    while (poll_nes->bus.ram[STA_MARKER] == 0u && steps < 40u) {
        nesturbator__cpu_step(poll_nes);
        steps++;
    }
    CHECK(steps < 40u);
    CHECK_EQ_U64(poll_nes->bus.ram[NMI_COUNT], 1u);
}

static void sta_marker_assertion(void)
{
    run_sta_control_to_marker();
    nmi_expected_assertion("control-8-marker-after-following-LDX",
                           poll_nes->bus.ram[STA_MARKER] == 0x11u);
}

static void sta_handler_entry_assertion(void)
{
    run_sta_control_to_marker();
    uint16_t return_pc =
        (uint16_t)((uint16_t)poll_nes->bus.ram[0x01fdu] << 8 | poll_nes->bus.ram[0x01fcu]);
    nmi_expected_assertion("control-8-handler-entry-after-following-LDX",
                           poll_nes->bus.ram[NMI_ENTRY_X] == 0x10u && return_pc == 0x800bu);
}

static void prepare_sta_control(void)
{
    poll_nes = build_poll_fixture(sta_control_program, sizeof sta_control_program);
    if (poll_nes == NULL)
        return;
    nmi_fixture_at(poll_nes, 241u, 2u, 0x80u, 0u, 0u);
    poll_nes->cpu.pc = 0x8000u;
    poll_nes->cpu.p = 0x20u;
    poll_nes->cpu.s = 0xfdu;
    poll_nes->bus.ram[0x10u] = 0u;
    poll_nes->bus.ram[STA_MARKER] = 0u;
    poll_nes->bus.ram[NMI_COUNT] = 0u;
    poll_nes->bus.ram[NMI_ENTRY_X] = 0xffu;
}

static void test_control_8_marker(void)
{
    prepare_sta_control();
    if (poll_nes == NULL)
        return;
    nmi_expect_red("nmi-control-8-following-instruction-marker",
                   "control-8-marker-after-following-LDX", sta_marker_assertion);
    nesturbator_destroy((nesturbator *)poll_nes);
    poll_nes = NULL;
}

static void test_control_8_handler_entry(void)
{
    prepare_sta_control();
    if (poll_nes == NULL)
        return;
    nmi_expect_red("nmi-control-8-handler-entry-order",
                   "control-8-handler-entry-after-following-LDX", sta_handler_entry_assertion);
    nesturbator_destroy((nesturbator *)poll_nes);
    poll_nes = NULL;
}

static void test_control_9_inc_poll_and_both_writes(void)
{
    poll_nes = build_poll_fixture(inc_control_program, sizeof inc_control_program);
    CHECK(poll_nes != NULL);
    if (poll_nes == NULL)
        return;
    nmi_fixture_at(poll_nes, 241u, 2u, 0x80u, 0u, 0u);
    poll_nes->cpu.pc = 0x8000u;
    poll_nes->cpu.p = 0x20u;
    poll_nes->cpu.s = 0xfdu;

    nesturbator__cpu_step(poll_nes); /* LDA #$80 */
    nesturbator__cpu_step(poll_nes); /* STA $2002 */
    uint64_t before_inc = poll_nes->cpu_cycle;
    nesturbator__cpu_step(poll_nes); /* INC $2000 */
    CHECK_EQ_U64(poll_nes->cpu_cycle - before_inc, 6u);
    CHECK_EQ_HEX(poll_nes->ppu.control, 0x81u);
    CHECK_EQ_HEX(poll_nes->ppu.io_bus, 0x81u);
    CHECK_EQ_U64(poll_nes->cpu.nmi_pending, 1u);

    nesturbator__cpu_step(poll_nes); /* NMI entry must precede LDX in Control 9. */
    CHECK_EQ_HEX(poll_nes->cpu.pc, 0x8200u);
    CHECK_EQ_U64(poll_nes->bus.ram[0x01fcu], 0x08u);
    CHECK_EQ_U64(poll_nes->bus.ram[0x01fdu], 0x80u);
    nesturbator__cpu_step(poll_nes); /* INC $30 in the guest NMI handler */
    nesturbator__cpu_step(poll_nes); /* STX $31 */
    CHECK_EQ_U64(poll_nes->bus.ram[NMI_ENTRY_X], 0u);
    CHECK_EQ_U64(poll_nes->bus.ram[NMI_COUNT], 1u);
    printf(
        "CASE guard nmi-control-9 INC-writes=2 cycles=6 entry-return=$8008 handler-before-LDX\n");
    nesturbator_destroy((nesturbator *)poll_nes);
    poll_nes = NULL;
}

static void test_unchanged_enable_does_not_add_nmi(void)
{
    static const uint8_t program[] = {
        0xa9u, 0x80u, 0x8du, 0x00u, 0x20u, /* LDA #$80; STA $2000 */
        0x8du, 0x00u, 0x20u,               /* STA $2000 unchanged */
        0x4cu, 0x05u, 0x80u                /* JMP to unchanged write */
    };
    struct nesturbator *nes = build_poll_fixture(program, sizeof program);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;
    nmi_fixture_at(nes, 241u, 2u, 0x80u, 0u, 0u);
    nes->cpu.pc = 0x8000u;
    for (unsigned i = 0u; i < 24u; i++)
        nesturbator__cpu_step(nes);
    CHECK_EQ_U64(nes->bus.ram[NMI_COUNT], 1u);
    printf("CASE guard unchanged-$80 guest-NMI-count=1\n");
    nesturbator_destroy((nesturbator *)nes);
}

static void test_disable_reenable_adds_one_guest_nmi(void)
{
    static const uint8_t program[] = {
        0xa9u, 0x80u, 0x8du, 0x00u, 0x20u, /* enable */
        0xa9u, 0x00u, 0x8du, 0x00u, 0x20u, /* disable */
        0xa9u, 0x80u, 0x8du, 0x00u, 0x20u, /* re-enable */
        0x4cu, 0x0fu, 0x80u                /* JMP to the loop */
    };
    struct nesturbator *nes = build_poll_fixture(program, sizeof program);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;
    nmi_fixture_at(nes, 241u, 2u, 0x80u, 0u, 0u);
    nes->cpu.pc = 0x8000u;
    for (unsigned i = 0u; i < 40u; i++)
        nesturbator__cpu_step(nes);
    CHECK_EQ_U64(nes->bus.ram[NMI_COUNT], 2u);
    printf("CASE guard disable-reenable guest-NMI-count=2\n");
    nesturbator_destroy((nesturbator *)nes);
}

static void test_enable_after_status_clear_does_not_request_nmi(void)
{
    struct nesturbator *nes = build_poll_fixture(NULL, 0u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;
    nmi_fixture_at(nes, 241u, 2u, 0x80u, 0u, 0x15u);
    CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x2002u) & 0x80u, 0x80u);
    CHECK_EQ_U64(nes->ppu.status & 0x80u, 0u);
    nesturbator__bus_write(nes, 0x3ff8u, 0x80u);
    CHECK_EQ_U64(nes->ppu.control, 0x80u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 0u);
    printf("CASE guard enable-after-status-clear mirror=$3FF8 nmi=0\n");
    nesturbator_destroy((nesturbator *)nes);
}

static void test_branch_class_nmi_return(uint16_t pc, uint8_t offset, uint8_t flags,
                                         uint16_t expected_pc, const char *name)
{
    uint8_t program[0x100] = {0u};
    size_t offset_in_prg = (size_t)(pc - 0x8000u);
    program[offset_in_prg] = 0xd0u; /* BNE */
    program[offset_in_prg + 1u] = offset;
    struct nesturbator *nes =
        nmi_fixture_build(program, sizeof program, nmi_handler, sizeof nmi_handler, 0x8200u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;
    nmi_fixture_at(nes, 240u, 338u, 0u, 0x80u, 0u);
    nes->cpu.pc = pc;
    nes->cpu.p = flags;
    nes->cpu.s = 0xfdu;
    nesturbator__cpu_step(nes); /* VBlank set occurs during the branch's bus cycles. */
    CHECK_EQ_HEX(nes->cpu.pc, expected_pc);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 1u);
    CHECK_EQ_U64(nes->cpu.irq_line, 0u);
    nesturbator__cpu_step(nes);
    CHECK_EQ_HEX(nes->cpu.pc, 0x8200u);
    CHECK_EQ_U64(nes->cpu.irq_line, 0u);
    uint16_t return_pc = (uint16_t)((uint16_t)nes->bus.ram[0x01fdu] << 8 | nes->bus.ram[0x01fcu]);
    CHECK_EQ_HEX(return_pc, expected_pc);
    printf("CASE guard branch-%s NMI-return=$%04X IRQ=unchanged\n", name, return_pc);
    nesturbator_destroy((nesturbator *)nes);
}

int main(void)
{
    test_control_8_marker();
    test_control_8_handler_entry();
    test_control_9_inc_poll_and_both_writes();
    test_unchanged_enable_does_not_add_nmi();
    test_disable_reenable_adds_one_guest_nmi();
    test_enable_after_status_clear_does_not_request_nmi();
    test_branch_class_nmi_return(0x8000u, 0x02u, 0x22u, 0x8002u, "not-taken");
    test_branch_class_nmi_return(0x8000u, 0x02u, 0x20u, 0x8004u, "taken-same-page");
    test_branch_class_nmi_return(0x80fcu, 0x02u, 0x20u, 0x8100u, "taken-page-cross");
    CHECK_DONE();
}
