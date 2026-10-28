/* NMI-02: guest-visible instruction ordering and NMI control patterns.
   AccuracyCoin at 6bee919529289c555803b612b8c9e4f7eaaeeabb, TEST_NMI_Control
   (AccuracyCoin.asm:6521-6640), defines the STA $2000 and INC $2000 poll cases.
   Its README calls out the following-instruction marker for case 8 and the
   poll between the two INC writes for case 9. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
    CHECK_EQ_U64(poll_nes->cpu.nmi_pending, 0u);
    CHECK_EQ_U64(poll_nes->cpu.nmi_polled, 0u);
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
    run_sta_control_to_marker();
    CHECK_EQ_U64(poll_nes->bus.ram[STA_MARKER], 0x11u);
    nesturbator_destroy((nesturbator *)poll_nes);
    poll_nes = NULL;
}

static void test_control_8_handler_entry(void)
{
    prepare_sta_control();
    if (poll_nes == NULL)
        return;
    run_sta_control_to_marker();
    uint16_t return_pc =
        (uint16_t)((uint16_t)poll_nes->bus.ram[0x01fdu] << 8 | poll_nes->bus.ram[0x01fcu]);
    CHECK_EQ_U64(poll_nes->bus.ram[NMI_ENTRY_X], 0x10u);
    CHECK_EQ_U64(return_pc, 0x800bu);
    printf("CASE guard nmi-control-8 STA-next=LDX entry-X=$10 return=$800B marker-X=$11\n");
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
    CHECK_EQ_U64(poll_nes->cpu.nmi_polled, 1u);

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
    uint8_t program[0x200] = {0u};
    size_t offset_in_prg = (size_t)(pc - 0x8000u);
    program[offset_in_prg] = 0xd0u; /* BNE */
    program[offset_in_prg + 1u] = offset;
    program[expected_pc - 0x8000u] = 0xeau; /* next instruction after the branch */
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
    uint8_t branch_polled = nes->cpu.nmi_polled;
    uint16_t return_pc = expected_pc;
    if (branch_polled == 0u) {
        nesturbator__cpu_step(nes);
        return_pc = (uint16_t)(expected_pc + 1u);
        CHECK_EQ_HEX(nes->cpu.pc, return_pc);
        CHECK_EQ_U64(nes->cpu.nmi_polled, 1u);
    }
    nesturbator__cpu_step(nes);
    CHECK_EQ_HEX(nes->cpu.pc, 0x8200u);
    CHECK_EQ_U64(nes->cpu.irq_line, 0u);
    uint16_t stacked_pc = (uint16_t)((uint16_t)nes->bus.ram[0x01fdu] << 8 | nes->bus.ram[0x01fcu]);
    CHECK_EQ_HEX(stacked_pc, return_pc);
    printf("CASE guard branch-%s NMI-return=$%04X IRQ=unchanged\n", name, return_pc);
    nesturbator_destroy((nesturbator *)nes);
}

static void test_poll_instruction_classes(void)
{
    static const struct {
        const char *name;
        uint8_t code[3];
        size_t len;
        uint16_t return_pc;
        uint8_t stack;
    } cases[] = {
        {"implied", {0xeau, 0u, 0u}, 1u, 0x8001u, 0u},
        {"read", {0xadu, 0x00u, 0x80u}, 3u, 0x8003u, 0u},
        {"store", {0x85u, 0x02u, 0u}, 2u, 0x8002u, 0u},
        {"rmw", {0xe6u, 0x02u, 0u}, 2u, 0x8002u, 0u},
        {"stack", {0x48u, 0u, 0u}, 1u, 0x8001u, 0u},
        {"rts", {0x60u, 0u, 0u}, 1u, 0x1235u, 1u},
        {"rti", {0x40u, 0u, 0u}, 1u, 0x1234u, 2u},
    };
    for (size_t i = 0u; i < sizeof cases / sizeof cases[0]; ++i) {
        struct nesturbator *nes = build_poll_fixture(cases[i].code, cases[i].len);
        CHECK(nes != NULL);
        if (nes == NULL)
            continue;
        nmi_fixture_at(nes, 241u, 2u, 0x80u, 0x80u, 0u);
        nes->cpu.pc = 0x8000u;
        nes->cpu.p = 0x20u;
        nes->cpu.s = cases[i].stack == 1u ? 0xfbu : cases[i].stack == 2u ? 0xfau : 0xfdu;
        nes->cpu.a = 0x5au;
        nes->bus.ram[0x0002u] = 0x10u;
        if (cases[i].stack == 1u) {
            nes->bus.ram[0x01fcu] = 0x34u;
            nes->bus.ram[0x01fdu] = 0x12u;
        } else if (cases[i].stack == 2u) {
            nes->bus.ram[0x01fbu] = 0x20u;
            nes->bus.ram[0x01fcu] = 0x34u;
            nes->bus.ram[0x01fdu] = 0x12u;
        }
        nesturbator__cpu_step(nes);
        CHECK_EQ_HEX(nes->cpu.pc, cases[i].return_pc);
        CHECK_EQ_U64(nes->cpu.nmi_pending, 1u);
        CHECK_EQ_U64(nes->cpu.nmi_polled, 1u);
        uint8_t return_stack = nes->cpu.s;
        nesturbator__cpu_step(nes);
        CHECK_EQ_HEX(nes->cpu.pc, 0x8200u);
        CHECK_EQ_HEX(nes->bus.ram[0x0100u | (uint8_t)(return_stack - 1u)],
                     (uint8_t)cases[i].return_pc);
        CHECK_EQ_HEX(nes->bus.ram[0x0100u | return_stack], (uint8_t)(cases[i].return_pc >> 8));
        printf("CASE guard nmi-poll-class=%s captured=1 polled=1 return=$%04X\n", cases[i].name,
               cases[i].return_pc);
        nesturbator_destroy((nesturbator *)nes);
    }
}

static void test_branch_late_capture_classes(void)
{
    static const struct {
        uint16_t pc;
        uint8_t offset;
        uint8_t flags;
        uint16_t expected_pc;
        uint16_t start_dot;
        uint8_t expected_polled;
        const char *name;
    } cases[] = {
        {0x8000u, 0x02u, 0x22u, 0x8002u, 337u, 0u, "not-taken"},
        {0x8000u, 0x02u, 0x20u, 0x8004u, 334u, 0u, "same-page"},
        {0x80fcu, 0x02u, 0x20u, 0x8100u, 334u, 1u, "page-cross"},
    };
    uint8_t program[0x100] = {0u};
    for (size_t i = 0u; i < sizeof cases / sizeof cases[0]; ++i) {
        memset(program, 0u, sizeof program);
        size_t offset = (size_t)(cases[i].pc - 0x8000u);
        program[offset] = 0xd0u;
        program[offset + 1u] = cases[i].offset;
        if (cases[i].expected_polled == 0u)
            program[cases[i].expected_pc - 0x8000u] = 0xeau;
        struct nesturbator *nes =
            nmi_fixture_build(program, sizeof program, nmi_handler, sizeof nmi_handler, 0x8200u);
        CHECK(nes != NULL);
        if (nes == NULL)
            continue;
        nmi_fixture_at(nes, 240u, cases[i].start_dot, 0u, 0x80u, 0u);
        nes->cpu.pc = cases[i].pc;
        nes->cpu.p = cases[i].flags;
        nes->cpu.s = 0xfdu;
        nesturbator__cpu_step(nes);
        CHECK_EQ_HEX(nes->cpu.pc, cases[i].expected_pc);
        CHECK_EQ_U64(nes->cpu.nmi_pending, 1u);
        CHECK_EQ_U64(nes->cpu.nmi_polled, cases[i].expected_polled);
        if (cases[i].expected_polled == 0u) {
            nesturbator__cpu_step(nes);
            CHECK_EQ_HEX(nes->cpu.pc, (uint16_t)(cases[i].expected_pc + 1u));
            CHECK_EQ_U64(nes->cpu.nmi_polled, 1u);
            nesturbator__cpu_step(nes);
        } else {
            nesturbator__cpu_step(nes);
        }
        CHECK_EQ_HEX(nes->cpu.pc, 0x8200u);
        printf("CASE guard nmi-branch-late-%s pending=1 polled=%u\n", cases[i].name,
               cases[i].expected_polled);
        nesturbator_destroy((nesturbator *)nes);
    }
}

static void test_saved_late_capture_continues_after_instruction_poll(void)
{
    static const uint8_t program[] = {0xeau, 0xe8u, 0xeau}; /* NOP; INX; NOP */
    struct nesturbator *a = build_poll_fixture(program, sizeof program);
    struct nesturbator *b = build_poll_fixture(program, sizeof program);
    CHECK(a != NULL && b != NULL);
    if (a == NULL || b == NULL) {
        if (a != NULL)
            nesturbator_destroy((nesturbator *)a);
        if (b != NULL)
            nesturbator_destroy((nesturbator *)b);
        return;
    }
    nmi_fixture_at(a, 240u, 337u, 0u, 0x80u, 0u);
    a->cpu.pc = 0x8000u;
    a->cpu.p = 0x20u;
    a->cpu.s = 0xfdu;
    a->bus.ram[NMI_COUNT] = 0u;
    a->bus.ram[NMI_ENTRY_X] = 0xffu;
    nesturbator__cpu_step(a);
    CHECK_EQ_HEX(a->cpu.pc, 0x8001u);
    CHECK_EQ_U64(a->cpu.nmi_pending, 1u);
    CHECK_EQ_U64(a->cpu.nmi_polled, 0u);

    size_t size = nesturbator_state_size((nesturbator *)a);
    uint8_t *state = (uint8_t *)malloc(size);
    uint8_t *roundtrip = (uint8_t *)malloc(size);
    CHECK(state != NULL && roundtrip != NULL);
    if (state != NULL && roundtrip != NULL) {
        CHECK_EQ_U64(nesturbator_save_state((nesturbator *)a, state, size), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_load_state((nesturbator *)b, state, size), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_save_state((nesturbator *)b, roundtrip, size), NESTURBATOR_OK);
        CHECK(memcmp(state, roundtrip, size) == 0);
        nesturbator__cpu_step(a);
        nesturbator__cpu_step(b);
        CHECK_EQ_HEX(a->cpu.pc, 0x8002u);
        CHECK_EQ_U64(a->cpu.x, 1u);
        CHECK_EQ_U64(a->cpu.nmi_polled, 1u);
        nesturbator__cpu_step(a);
        nesturbator__cpu_step(b);
        CHECK_EQ_HEX(a->cpu.pc, 0x8200u);
        CHECK_EQ_HEX(b->cpu.pc, 0x8200u);
        nesturbator__cpu_step(a);
        nesturbator__cpu_step(b);
        nesturbator__cpu_step(a);
        nesturbator__cpu_step(b);
        CHECK_EQ_U64(a->bus.ram[NMI_ENTRY_X], 1u);
        CHECK_EQ_U64(b->bus.ram[NMI_ENTRY_X], 1u);
        CHECK_EQ_U64(a->bus.ram[NMI_COUNT], 1u);
        CHECK_EQ_U64(b->bus.ram[NMI_COUNT], 1u);
        CHECK_EQ_U64(nesturbator_save_state((nesturbator *)a, state, size), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_save_state((nesturbator *)b, roundtrip, size), NESTURBATOR_OK);
        CHECK(memcmp(state, roundtrip, size) == 0);
    }
    free(state);
    free(roundtrip);
    nesturbator_destroy((nesturbator *)a);
    nesturbator_destroy((nesturbator *)b);
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
    test_poll_instruction_classes();
    test_branch_late_capture_classes();
    test_saved_late_capture_continues_after_instruction_poll();
    CHECK_DONE();
}
