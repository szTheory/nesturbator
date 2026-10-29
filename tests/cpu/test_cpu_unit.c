/* cpu.unit: CPU behaviour the committed 65x02 sample cannot show, run
   through the shipped CPU object on the test bus of tests/cpu/vector_bus.c.

   - JSR whose push of PCL overwrites its own ADH operand. JSR reads ADL,
     makes a dummy stack read, pushes PCH and PCL, and only then reads ADH,
     so it reads the pushed byte (64doc, nesdev.org/6502_cpu.txt; D-17). The
     case is SingleStepTests/65x02 issue 18, taken from 6502/v1/20.json (MIT,
     same pin), because no nes6502 test in the sample or the full set has the
     stack overlapping the operand (02-RESEARCH).
   - The jammed loop: after a JAM opcode, every later step is one read of
     0xFFFF with nothing else changing (D-13).
   - A JAM opcode at 0xFFFF leaves PC at 0x0000: PC wraps at 16 bits. */
#include <string.h>

#include "../check.h"
#include "vector_bus.h"

static struct vector_machine m;

/* Starts a test: RAM, registers and the cycle log all zero. */
static void reset_machine(void)
{
    memset(&m, 0, sizeof m);
}

/* Checks logged cycle i against an expected address, value and kind. */
static void check_cycle(uint32_t i, uint16_t addr, uint8_t value, uint8_t kind)
{
    CHECK(i < m.log_count);
    if (i >= m.log_count) {
        return;
    }
    CHECK_EQ_HEX(m.log[i].addr, addr);
    CHECK_EQ_HEX(m.log[i].value, value);
    CHECK_EQ_U64(m.log[i].kind, kind);
}

static void test_jsr_overlap(void)
{
    reset_machine();
    m.nes.cpu.pc = 379u; /* 0x017B */
    m.nes.cpu.s = 125u;  /* 0x7D */
    m.ram[379] = 32u;    /* JSR */
    m.ram[380] = 85u;    /* ADL 0x55 */
    m.ram[381] = 19u;    /* ADH 0x13, overwritten by the PCH push */
    m.ram[341] = 173u;

    nesturbator__cpu_step(&m.nes);

    CHECK_EQ_U64(m.log_count, 6u);
    CHECK_EQ_U64(m.log_overflow, 0u);
    check_cycle(0u, 379u, 32u, N65V_KIND_READ);
    check_cycle(1u, 380u, 85u, N65V_KIND_READ);
    check_cycle(2u, 381u, 19u, N65V_KIND_READ);
    check_cycle(3u, 381u, 1u, N65V_KIND_WRITE);
    check_cycle(4u, 380u, 125u, N65V_KIND_WRITE);
    check_cycle(5u, 381u, 1u, N65V_KIND_READ);
    CHECK_EQ_HEX(m.nes.cpu.pc, 341u); /* 0x0155 */
    CHECK_EQ_HEX(m.nes.cpu.s, 123u);  /* 0x7B */
    CHECK_EQ_HEX(m.ram[379], 32u);
    CHECK_EQ_HEX(m.ram[380], 125u);
    CHECK_EQ_HEX(m.ram[381], 1u);
    CHECK_EQ_HEX(m.ram[341], 173u);
}

static void test_jsr_uses_penultimate_irq_poll(void)
{
    reset_machine();
    m.nes.cpu.pc = 0x8000u;
    m.nes.cpu.s = 0xfdu;
    m.nes.cpu.p = 0x21u;    /* IRQ enabled; BCC $00 below is not taken. */
    m.ram[0x8000u] = 0x20u; /* JSR $8006. */
    m.ram[0x8001u] = 0x06u;
    m.ram[0x8002u] = 0x80u;
    m.ram[0x8006u] = 0x90u; /* BCC $00. */
    m.ram[0x8007u] = 0x00u;
    m.ram[0xfffeu] = 0x34u;
    m.ram[0xffffu] = 0x12u;
    /* IRQ arrives after JSR's C5/PCL push, before its C6 final operand read.
       The poll at C5 has already happened; BCC C1 is the next legal poll. */
    vector_irq_schedule(&m, 5u, 0u, 1u);

    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x8006u);
    CHECK_EQ_U64(m.nes.cpu.irq_captured, 1u);
    CHECK_EQ_U64(m.nes.cpu.poll_latch, 1u);
    CHECK_EQ_U64(m.nes.cpu.irq_accepted, 0u);

    nesturbator__cpu_step(&m.nes); /* BCC executes and commits its C1 acceptance. */
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x8008u);
    CHECK_EQ_U64(m.nes.cpu.irq_accepted, 1u);
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x1234u);
    CHECK_EQ_HEX(m.ram[0x01fdu], 0x80u);
    CHECK_EQ_HEX(m.ram[0x01fau], 0x08u);
    CHECK_EQ_HEX(m.ram[0x01f9u], 0x21u);
    CHECK_EQ_U64(m.log_count, 15u);
    CHECK_EQ_U64(m.log_overflow, 0u);
    printf("CASE guard JSR C5 poll ignores late C6 IRQ; following BCC C1 accepts\n");
}

static void test_jammed_loop(void)
{
    reset_machine();
    m.nes.cpu.pc = 0x0200u;
    m.nes.cpu.a = 0x11u;
    m.nes.cpu.x = 0x22u;
    m.nes.cpu.y = 0x33u;
    m.nes.cpu.s = 0x44u;
    m.nes.cpu.p = 0xE5u;
    m.ram[0x0200] = 0x02u;
    m.ram[0x0201] = 0x5Au;
    m.ram[0xFFFE] = 0x34u;
    m.ram[0xFFFF] = 0x12u;

    /* The JAM itself: eleven reads, PC one past the opcode. */
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_U64(m.log_count, 11u);
    check_cycle(0u, 0x0200u, 0x02u, N65V_KIND_READ);
    check_cycle(1u, 0x0201u, 0x5Au, N65V_KIND_READ);
    check_cycle(2u, 0xFFFFu, 0x12u, N65V_KIND_READ);
    check_cycle(3u, 0xFFFEu, 0x34u, N65V_KIND_READ);
    check_cycle(4u, 0xFFFEu, 0x34u, N65V_KIND_READ);
    for (uint32_t i = 5u; i < 11u; i++) {
        check_cycle(i, 0xFFFFu, 0x12u, N65V_KIND_READ);
    }
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x0201u);
    CHECK_EQ_U64(m.nes.cpu.jammed, 1u);

    /* Each later step: one read of 0xFFFF, nothing else changes. */
    for (uint32_t step = 0u; step < 3u; step++) {
        vector_machine_reset_log(&m);
        nesturbator__cpu_step(&m.nes);
        CHECK_EQ_U64(m.log_count, 1u);
        check_cycle(0u, 0xFFFFu, 0x12u, N65V_KIND_READ);
        CHECK_EQ_HEX(m.nes.cpu.pc, 0x0201u);
        CHECK_EQ_HEX(m.nes.cpu.a, 0x11u);
        CHECK_EQ_HEX(m.nes.cpu.x, 0x22u);
        CHECK_EQ_HEX(m.nes.cpu.y, 0x33u);
        CHECK_EQ_HEX(m.nes.cpu.s, 0x44u);
        CHECK_EQ_HEX(m.nes.cpu.p, 0xE5u);
        CHECK_EQ_U64(m.nes.cpu.jammed, 1u);
    }
}

static void test_jam_pc_wrap(void)
{
    reset_machine();
    m.nes.cpu.pc = 0xFFFFu;
    m.ram[0xFFFF] = 0x02u;

    nesturbator__cpu_step(&m.nes);

    CHECK_EQ_U64(m.log_count, 11u);
    check_cycle(0u, 0xFFFFu, 0x02u, N65V_KIND_READ);
    check_cycle(1u, 0x0000u, 0x00u, N65V_KIND_READ);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x0000u);
    CHECK_EQ_U64(m.nes.cpu.jammed, 1u);
}

enum entry_origin { ENTRY_BRK, ENTRY_IRQ, ENTRY_NMI };

struct expected_bus_access {
    uint16_t addr;
    uint8_t value;
    uint8_t kind;
};

static void test_entry_origin_sequence(enum entry_origin origin, uint8_t initial_s)
{
    const uint8_t initial_p = 0xc9u;
    const uint8_t irq_lo = 0xbcu;
    const uint8_t irq_hi = 0x9au;
    const uint8_t nmi_lo = 0x78u;
    const uint8_t nmi_hi = 0x56u;
    const uint8_t pushed_p =
        origin == ENTRY_BRK ? (uint8_t)(initial_p | 0x30u) : (uint8_t)((initial_p & 0xefu) | 0x20u);
    const uint16_t vector = origin == ENTRY_NMI ? 0xfffau : 0xfffeu;
    const uint16_t target =
        origin == ENTRY_NMI ? 0x5678u : (origin == ENTRY_BRK ? 0x00bcu : 0x9abcu);
    const uint16_t return_pc = origin == ENTRY_BRK ? 0x0001u : 0xffffu;
    const uint8_t signature = 0x5au;
    const uint16_t stack0 = (uint16_t)(0x0100u | initial_s);
    const uint16_t stack1 = (uint16_t)(0x0100u | (uint8_t)(initial_s - 1u));
    const uint16_t stack2 = (uint16_t)(0x0100u | (uint8_t)(initial_s - 2u));
    struct expected_bus_access expected[7];
    uint8_t expected_count = 0u;

    reset_machine();
    m.nes.cpu.pc = 0xffffu;
    m.nes.cpu.s = initial_s;
    m.nes.cpu.p = initial_p;
    m.ram[0xffffu] = irq_hi;
    m.ram[0x0000u] = signature;
    m.ram[0xfffau] = nmi_lo;
    m.ram[0xfffbu] = nmi_hi;
    m.ram[0xfffeu] = irq_lo;
    m.ram[0xffffu] = irq_hi;
    if (origin == ENTRY_BRK) {
        m.ram[0xffffu] = 0x00u;
        m.ram[0x0000u] = signature;
    } else if (origin == ENTRY_IRQ) {
        m.nes.cpu.irq_accepted = 1u;
    } else {
        m.nes.cpu.nmi_pending = 1u;
        m.nes.cpu.nmi_polled = 1u;
    }

    if (origin == ENTRY_BRK) {
        expected[expected_count++] = (struct expected_bus_access){0xffffu, 0x00u, N65V_KIND_READ};
        expected[expected_count++] =
            (struct expected_bus_access){0x0000u, signature, N65V_KIND_READ};
    } else {
        expected[expected_count++] = (struct expected_bus_access){0xffffu, irq_hi, N65V_KIND_READ};
        expected[expected_count++] = (struct expected_bus_access){0xffffu, irq_hi, N65V_KIND_READ};
    }
    expected[expected_count++] =
        (struct expected_bus_access){stack0, (uint8_t)(return_pc >> 8), N65V_KIND_WRITE};
    expected[expected_count++] =
        (struct expected_bus_access){stack1, (uint8_t)return_pc, N65V_KIND_WRITE};
    expected[expected_count++] = (struct expected_bus_access){stack2, pushed_p, N65V_KIND_WRITE};
    expected[expected_count++] =
        (struct expected_bus_access){vector, origin == ENTRY_NMI ? nmi_lo : irq_lo, N65V_KIND_READ};
    expected[expected_count++] = (struct expected_bus_access){
        (uint16_t)(vector + 1u), origin == ENTRY_NMI ? nmi_hi : (origin == ENTRY_BRK ? 0u : irq_hi),
        N65V_KIND_READ};

    nesturbator__cpu_step(&m.nes);

    CHECK_EQ_U64(expected_count, 7u);
    CHECK_EQ_U64(m.log_count, expected_count);
    CHECK_EQ_U64(m.log_overflow, 0u);
    for (uint32_t i = 0u; i < expected_count && i < m.log_count; i++) {
        check_cycle(i, expected[i].addr, expected[i].value, expected[i].kind);
    }
    CHECK_EQ_HEX(m.nes.cpu.pc, target);
    CHECK_EQ_HEX(m.nes.cpu.s, (uint8_t)(initial_s - 3u));
    CHECK_EQ_HEX(m.ram[stack0], (uint8_t)(return_pc >> 8));
    CHECK_EQ_HEX(m.ram[stack1], (uint8_t)return_pc);
    CHECK_EQ_HEX(m.ram[stack2], pushed_p);
    CHECK_EQ_HEX(m.nes.cpu.p, (uint8_t)(initial_p | 0x04u));
    CHECK_EQ_U64(m.nes.cpu.irq_accepted, 0u);
    CHECK_EQ_U64(m.nes.cpu.nmi_polled, 0u);
}

static void test_interrupt_entry_origins_wrap_pc_and_stack(void)
{
    for (enum entry_origin origin = ENTRY_BRK; origin <= ENTRY_NMI; origin++) {
        test_entry_origin_sequence(origin, 0x00u);
        test_entry_origin_sequence(origin, 0x01u);
    }
    printf("CASE guard cpu_step BRK/IRQ/NMI seven accesses PC=$FFFF stack wraps S=$00/$01\n");
}

static void test_interrupt_entry_action_is_one_bus_cycle(void)
{
    static const uint16_t addresses[7] = {0x8001u, 0x8001u, 0x01fdu, 0x01fcu,
                                          0x01fbu, 0xfffeu, 0xffffu};
    reset_machine();
    m.nes.cpu.pc = 0x8001u;
    m.nes.cpu.s = 0xfdu;
    m.nes.cpu.p = 0x20u;
    m.ram[0xfffeu] = 0x34u;
    m.ram[0xffffu] = 0x12u;
    nesturbator__cpu_entry_begin(&m.nes, NESTURBATOR_ENTRY_IRQ);

    for (uint8_t action = 0u; action < 7u; action++) {
        uint32_t accesses_before = m.log_count;
        CHECK_EQ_U64(nesturbator__cpu_entry_action(&m.nes), 1u);
        CHECK_EQ_U64(m.log_count, accesses_before + 1u);
        CHECK_EQ_HEX(m.log[accesses_before].addr, addresses[action]);
    }
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x1234u);
    CHECK_EQ_U64(m.nes.cpu.entry_origin, NESTURBATOR_ENTRY_NONE);
    CHECK_EQ_U64(m.nes.cpu.entry_next_action, 0u);
    CHECK_EQ_U64(m.log_count, 7u);
    printf("CASE guard staged IRQ helper completes exactly one bus cycle per action\n");
}

static void test_reset_aborts_partial_interrupt_entry(void)
{
    reset_machine();
    m.nes.cpu.pc = 0x8001u;
    m.nes.cpu.s = 0xfdu;
    m.ram[0xfffcu] = 0x78u;
    m.ram[0xfffdu] = 0x56u;
    nesturbator__cpu_entry_begin(&m.nes, NESTURBATOR_ENTRY_IRQ);
    CHECK_EQ_U64(nesturbator__cpu_entry_action(&m.nes), 1u);
    CHECK_EQ_U64(m.nes.cpu.entry_origin, NESTURBATOR_ENTRY_IRQ);
    CHECK_EQ_U64(m.nes.cpu.entry_next_action, 2u);

    nesturbator__cpu_reset(&m.nes);
    CHECK_EQ_U64(m.nes.cpu.entry_origin, NESTURBATOR_ENTRY_NONE);
    CHECK_EQ_U64(m.nes.cpu.entry_next_action, 0u);
    CHECK_EQ_U64(m.nes.cpu.entry_vector, NESTURBATOR_ENTRY_VECTOR_NONE);
    CHECK_EQ_U64(m.nes.cpu.entry_vector_low, 0u);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x5678u);
    CHECK_EQ_HEX(m.nes.cpu.s, 0xfau);
    CHECK_EQ_U64(m.log_count, 8u); /* One old C1 action, then reset's seven reads. */
    printf("CASE guard reset aborts a partial interrupt entry before the reset vector\n");
}

struct branch_case {
    uint8_t opcode;
    uint8_t taken_p;
    uint8_t not_taken_p;
    const char *name;
};

struct branch_shape {
    uint16_t pc;
    uint8_t offset;
    uint16_t target;
    uint16_t dummy;
    uint16_t fixup;
    uint8_t accesses;
    uint8_t taken;
    const char *name;
};

static const struct branch_case branch_cases[] = {
    {0x10u, 0x20u, 0xa0u, "BPL"}, {0x30u, 0xa0u, 0x20u, "BMI"}, {0x50u, 0x20u, 0x60u, "BVC"},
    {0x70u, 0x60u, 0x20u, "BVS"}, {0x90u, 0x20u, 0x21u, "BCC"}, {0xb0u, 0x21u, 0x20u, "BCS"},
    {0xd0u, 0x20u, 0x22u, "BNE"}, {0xf0u, 0x22u, 0x20u, "BEQ"}};

static const struct branch_shape branch_shapes[] = {
    {0x2000u, 0x12u, 0x2002u, 0x0000u, 0x0000u, 2u, 0u, "not-taken"},
    {0x2000u, 0x02u, 0x2004u, 0x2002u, 0x0000u, 3u, 1u, "same-page"},
    {0x20fdu, 0x02u, 0x2101u, 0x20ffu, 0x2001u, 4u, 1u, "forward-cross"},
    {0x2101u, 0xfcu, 0x20ffu, 0x2103u, 0x21ffu, 4u, 1u, "backward-cross"}};

static void test_branch_opcode_access_table(void)
{
    for (uint32_t op = 0u; op < sizeof branch_cases / sizeof branch_cases[0]; op++) {
        for (uint32_t shape = 0u; shape < sizeof branch_shapes / sizeof branch_shapes[0]; shape++) {
            const struct branch_case *branch = &branch_cases[op];
            const struct branch_shape *form = &branch_shapes[shape];
            reset_machine();
            m.nes.cpu.pc = form->pc;
            m.nes.cpu.p = form->taken != 0u ? branch->taken_p : branch->not_taken_p;
            m.ram[form->pc] = branch->opcode;
            m.ram[(uint16_t)(form->pc + 1u)] = form->offset;
            if (form->taken != 0u) {
                m.ram[form->dummy] = 0x5au;
                if (form->accesses == 4u)
                    m.ram[form->fixup] = 0x6bu;
            }

            nesturbator__cpu_step(&m.nes);

            CHECK_EQ_U64(m.log_count, form->accesses);
            CHECK_EQ_U64(m.log_overflow, 0u);
            check_cycle(0u, form->pc, branch->opcode, N65V_KIND_READ);
            check_cycle(1u, (uint16_t)(form->pc + 1u), form->offset, N65V_KIND_READ);
            if (form->taken != 0u) {
                check_cycle(2u, form->dummy, 0x5au, N65V_KIND_READ);
                if (form->accesses == 4u)
                    check_cycle(3u, form->fixup, 0x6bu, N65V_KIND_READ);
            }
            CHECK_EQ_HEX(m.nes.cpu.pc, form->target);
        }
    }

    /* Signed branch arithmetic wraps through the 16-bit address space. */
    reset_machine();
    m.nes.cpu.pc = 0xfffdu;
    m.nes.cpu.p = 0x20u;
    m.ram[0xfffdu] = 0x10u;
    m.ram[0xfffeu] = 0x04u;
    m.ram[0xffffu] = 0x5au;
    m.ram[0xff03u] = 0x6bu;
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_U64(m.log_count, 4u);
    check_cycle(0u, 0xfffdu, 0x10u, N65V_KIND_READ);
    check_cycle(1u, 0xfffeu, 0x04u, N65V_KIND_READ);
    check_cycle(2u, 0xffffu, 0x5au, N65V_KIND_READ);
    check_cycle(3u, 0xff03u, 0x6bu, N65V_KIND_READ);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x0003u);
}

static void test_branch_late_same_page_irq_is_not_a_second_poll(void)
{
    reset_machine();
    m.nes.cpu.pc = 0x2000u;
    m.nes.cpu.s = 0xfdu;
    m.nes.cpu.p = 0x20u;
    m.ram[0x2000u] = 0x10u; /* BPL +2 */
    m.ram[0x2001u] = 0x02u;
    m.ram[0x2002u] = 0x5au;
    m.ram[0x2004u] = 0xe8u; /* INX confirms the target instruction ran. */
    m.ram[0xfffeu] = 0x00u;
    m.ram[0xffffu] = 0x90u;
    m.irq_schedule_enabled = 1u;
    vector_irq_schedule(&m, 1u, 1u, 1u); /* after C1 poll, before its close */

    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x2004u);
    CHECK_EQ_HEX(m.nes.cpu.s, 0xfdu);
    CHECK_EQ_U64(m.poll_sample[0], 0u);
    CHECK_EQ_U64(m.line_after_access[0], 1u);
    CHECK_EQ_U64(m.captured_after_access[0], 1u);
    /* The next instruction must run before a later poll can accept the line. */
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x2005u);
    CHECK_EQ_U64(m.nes.cpu.x, 1u);
    printf("CASE branch-same-page-late-IRQ first-poll=0 second-poll=none target-INX=1\n");
    if (m.nes.cpu.pc == 0x2005u && m.nes.cpu.x == 1u) {
        nesturbator__cpu_step(&m.nes);
        CHECK_EQ_HEX(m.nes.cpu.pc, 0x9000u);
        CHECK_EQ_HEX(m.ram[0x01fdu], 0x20u);
        CHECK_EQ_HEX(m.ram[0x01fcu], 0x05u);
        CHECK_EQ_HEX(m.ram[0x01fbu], 0x20u);
    }
}

static void test_branch_page_cross_second_poll_and_first_acceptance(void)
{
    /* A source rise after the first poll is accepted at the cross-page poll. */
    reset_machine();
    m.nes.cpu.pc = 0x20fdu;
    m.nes.cpu.s = 0xfdu;
    m.nes.cpu.p = 0x20u;
    m.ram[0x20fdu] = 0x10u;
    m.ram[0x20feu] = 0x02u;
    m.ram[0x20ffu] = 0x5au;
    m.ram[0x2001u] = 0x6bu;
    m.ram[0x2101u] = 0xeau;
    m.ram[0xfffeu] = 0x00u;
    m.ram[0xffffu] = 0x90u;
    vector_irq_schedule(&m, 2u, 1u, 1u); /* after C2 poll, before C2 close */
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x2101u);
    CHECK_EQ_U64(m.poll_sample[0], 0u);
    CHECK_EQ_U64(m.poll_sample[1], 0u);
    CHECK_EQ_U64(m.poll_sample[2], 1u);
    CHECK_EQ_U64(m.poll_sample[3], 1u);
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x9000u);
    CHECK_EQ_HEX(m.ram[0x01fdu], 0x21u);
    CHECK_EQ_HEX(m.ram[0x01fcu], 0x01u);
    CHECK_EQ_HEX(m.ram[0x01fbu], 0x20u);
    CHECK_EQ_U64(m.log_count, 11u);
    CHECK_EQ_U64(m.irq_event_overflow, 0u);

    /* The first poll has already accepted IRQ. C3 reads the test bus's
       $4015 acknowledgement address, and the second poll must not erase it. */
    reset_machine();
    m.nes.cpu.pc = 0x4013u;
    m.nes.cpu.s = 0xfdu;
    m.nes.cpu.p = 0x20u;
    m.nes.cpu.irq_line = 1u;
    m.nes.cpu.irq_captured = 1u;
    m.ram[0x4013u] = 0x10u; /* BPL -32: target $3FF5, C3 reads $4015 */
    m.ram[0x4014u] = 0xe0u;
    m.ram[0x4015u] = 0x40u;
    m.ram[0x3ff5u] = 0xeau;
    m.ram[0x40f5u] = 0x6bu;
    m.ram[0xfffeu] = 0x00u;
    m.ram[0xffffu] = 0x90u;
    vector_irq_ack_on_read(&m, 0x4015u);

    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x3ff5u);
    CHECK_EQ_U64(m.poll_sample[0], 1u);
    CHECK_EQ_U64(m.captured_at_poll[0], 1u);
    CHECK_EQ_U64(m.line_at_poll[0], 1u);
    CHECK_EQ_HEX(m.log[2].addr, 0x4015u);
    CHECK_EQ_U64(m.irq_ack_count, 1u);
    CHECK_EQ_U64(m.line_after_access[2], 0u);
    CHECK_EQ_U64(m.captured_after_access[2], 0u);
    CHECK_EQ_U64(m.poll_sample[3], 0u);
    CHECK_EQ_U64(m.captured_at_poll[3], 0u);
    CHECK_EQ_U64(m.line_at_poll[3], 0u);
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x9000u);
    CHECK_EQ_HEX(m.ram[0x01fdu], 0x3fu);
    CHECK_EQ_HEX(m.ram[0x01fcu], 0xf5u);
    CHECK_EQ_HEX(m.ram[0x01fbu], 0x20u);
    printf("CASE branch-first-acceptance-survives-4015 captured-then-cleared=1\n");
}

static void test_branch_irq_after_second_poll_waits_for_next_poll(void)
{
    reset_machine();
    m.nes.cpu.pc = 0x20fdu;
    m.nes.cpu.s = 0xfdu;
    m.nes.cpu.p = 0x20u;
    m.ram[0x20fdu] = 0x10u;
    m.ram[0x20feu] = 0x02u;
    m.ram[0x20ffu] = 0x5au;
    m.ram[0x2001u] = 0x6bu;
    m.ram[0x2101u] = 0xe8u; /* INX must run before the later acceptance. */
    m.ram[0xfffeu] = 0x00u;
    m.ram[0xffffu] = 0x90u;
    vector_irq_schedule(&m, 4u, 1u, 1u); /* after second poll, before C4 closes */

    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x2101u);
    CHECK_EQ_U64(m.poll_sample[2], 0u); /* C3 is not a poll on this branch. */
    CHECK_EQ_U64(m.poll_sample[3], 0u); /* The second poll preceded this rise. */
    CHECK_EQ_U64(m.line_after_access[3], 1u);
    CHECK_EQ_U64(m.captured_after_access[3], 1u);
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x2102u);
    CHECK_EQ_U64(m.nes.cpu.x, 1u);
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x9000u);
    CHECK_EQ_HEX(m.ram[0x01fdu], 0x21u);
    CHECK_EQ_HEX(m.ram[0x01fcu], 0x02u);
    CHECK_EQ_HEX(m.ram[0x01fbu], 0x20u);
    printf("CASE guard branch-cross-IRQ-after-second-poll waits for next poll\n");
}

struct branch_source_window_case {
    const char *name;
    uint8_t initially_asserted;
    uint32_t event_access;
    uint8_t after_open;
    uint8_t event_line;
    uint8_t first_sample;
    uint8_t second_sample;
    uint8_t accepted;
};

static void prepare_crossing_bpl(void)
{
    reset_machine();
    m.nes.cpu.pc = 0x20fdu;
    m.nes.cpu.s = 0xfdu;
    m.nes.cpu.p = 0x20u;
    m.ram[0x20fdu] = 0x10u; /* BPL +2 */
    m.ram[0x20feu] = 0x02u;
    m.ram[0x20ffu] = 0x5au;
    m.ram[0x2001u] = 0x6bu;
    m.ram[0x2101u] = 0xe8u; /* INX distinguishes a missed second poll. */
    m.ram[0xfffeu] = 0x00u;
    m.ram[0xffffu] = 0x90u;
}

/* Source transitions straddle the pinned C1-first and C4-page-cross polls.
   The flat bus records input capture only; it does not model production
   device sampling or sub-cycle detector behavior. */
static void test_branch_source_transition_windows(void)
{
    static const struct branch_source_window_case cases[] = {
        {"before-first", 1u, 0u, 0u, 0u, 1u, 1u, 1u},
        {"after-first-before-second", 0u, 1u, 1u, 1u, 0u, 1u, 1u},
        {"before-second", 0u, 3u, 0u, 1u, 0u, 1u, 1u},
        {"after-second-opening", 0u, 4u, 1u, 1u, 0u, 0u, 0u},
        {"after-second-close", 0u, 4u, 0u, 1u, 0u, 0u, 0u}};

    for (uint32_t i = 0u; i < sizeof cases / sizeof cases[0]; i++) {
        const struct branch_source_window_case *c = &cases[i];
        prepare_crossing_bpl();
        if (c->initially_asserted != 0u) {
            m.nes.cpu.irq_line = 1u;
            m.nes.cpu.irq_captured = 1u;
            m.irq_schedule_enabled = 1u;
        } else {
            vector_irq_schedule(&m, c->event_access, c->after_open, c->event_line);
        }

        nesturbator__cpu_step(&m.nes);
        CHECK_EQ_HEX(m.nes.cpu.pc, 0x2101u);
        CHECK_EQ_U64(m.poll_sample[0], c->first_sample);
        CHECK_EQ_U64(m.poll_sample[3], c->second_sample);
        CHECK_EQ_U64(m.irq_event_overflow, 0u);
        if (c->event_access != 0u) {
            uint32_t event_index = c->event_access - 1u;
            CHECK_EQ_U64(m.line_after_access[event_index], c->event_line);
            CHECK_EQ_U64(m.captured_after_access[event_index], c->event_line);
        }

        if (c->accepted != 0u) {
            nesturbator__cpu_step(&m.nes);
            CHECK_EQ_HEX(m.nes.cpu.pc, 0x9000u);
            CHECK_EQ_HEX(m.ram[0x01fdu], 0x21u);
            CHECK_EQ_HEX(m.ram[0x01fcu], 0x01u);
            CHECK_EQ_HEX(m.ram[0x01fbu], 0x20u);
        } else {
            nesturbator__cpu_step(&m.nes);
            CHECK_EQ_HEX(m.nes.cpu.pc, 0x2102u);
            CHECK_EQ_U64(m.nes.cpu.x, 1u);
            nesturbator__cpu_step(&m.nes);
            CHECK_EQ_HEX(m.nes.cpu.pc, 0x9000u);
            CHECK_EQ_HEX(m.ram[0x01fdu], 0x21u);
            CHECK_EQ_HEX(m.ram[0x01fcu], 0x02u);
            CHECK_EQ_HEX(m.ram[0x01fbu], 0x20u);
        }
        printf("CASE source-window %s first=%u second=%u accepted=%u\n", c->name, c->first_sample,
               c->second_sample, c->accepted);
    }
}

static void test_irq_acknowledgment_poll_windows(void)
{
    /* Before the branch's first legal poll: an older, masked source is
       acknowledged by a real-shaped $4015 read, then CLI enables polling. */
    reset_machine();
    m.nes.cpu.pc = 0x20f8u;
    m.nes.cpu.s = 0xfdu;
    m.nes.cpu.p = 0x24u;
    m.nes.cpu.irq_line = 1u;
    m.nes.cpu.irq_captured = 1u;
    m.nes.cpu.poll_latch = 1u;
    m.ram[0x20f8u] = 0xadu; /* LDA $4015 */
    m.ram[0x20f9u] = 0x15u;
    m.ram[0x20fau] = 0x40u;
    m.ram[0x20fbu] = 0x58u; /* CLI */
    m.ram[0x20fcu] = 0x10u; /* BPL -32 crosses back to $20DE. */
    m.ram[0x20fdu] = 0xe0u;
    m.ram[0x20deu] = 0xeau;
    vector_irq_ack_on_read(&m, 0x4015u);
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_U64(m.irq_ack_count, 1u);
    CHECK_EQ_U64(m.line_after_access[3], 0u);
    CHECK_EQ_U64(m.captured_after_access[3], 0u);
    CHECK((m.nes.cpu.p & 0x04u) != 0u);
    nesturbator__cpu_step(&m.nes); /* CLI: masked dispatch, old I=1. */
    CHECK((m.nes.cpu.p & 0x04u) == 0u);
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x20deu);
    CHECK_EQ_U64(m.poll_sample[6], 0u); /* Branch C1 sees the acknowledged level. */
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x20dfu);
    CHECK_EQ_HEX(m.nes.cpu.s, 0xfdu);

    /* Acknowledge on the C1 opening's bus read. The source was sampled high
       at the legal poll, then the read lowers both live and captured state. */
    reset_machine();
    m.nes.cpu.pc = 0x4015u;
    m.nes.cpu.s = 0xfdu;
    m.nes.cpu.p = 0x20u;
    m.nes.cpu.irq_line = 1u;
    m.nes.cpu.irq_captured = 1u;
    m.ram[0x4015u] = 0x90u; /* BCC -112 -> $3FA7 */
    m.ram[0x4016u] = 0x90u;
    m.ram[0x4017u] = 0x5au;
    m.ram[0x40a7u] = 0x6bu;
    m.ram[0x3fa7u] = 0xeau;
    m.ram[0xfffeu] = 0x00u;
    m.ram[0xffffu] = 0x90u;
    vector_irq_ack_on_read(&m, 0x4015u);
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x3fa7u);
    CHECK_EQ_U64(m.poll_sample[0], 1u);
    CHECK_EQ_U64(m.line_after_access[0], 0u);
    CHECK_EQ_U64(m.captured_after_access[0], 0u);
    CHECK_EQ_U64(m.poll_sample[3], 0u);
    CHECK_EQ_U64(m.irq_ack_count, 1u);
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x9000u);
    CHECK_EQ_HEX(m.nes.cpu.s, 0xfau);
    CHECK_EQ_HEX(m.ram[0x01fdu], 0x3fu);
    CHECK_EQ_HEX(m.ram[0x01fcu], 0xa7u);
    CHECK_EQ_HEX(m.ram[0x01fbu], 0x20u);
    printf("CASE branch-first-poll-ack captured-high-then-$4015-low\n");

    /* Acknowledge after the C4 opening sample. It cannot erase the captured
       second-poll result; assertions preserve P versus eligibility-I. */
    reset_machine();
    m.nes.cpu.pc = 0x40e3u;
    m.nes.cpu.s = 0xfdu;
    m.nes.cpu.p = 0x20u;
    m.nes.cpu.irq_line = 1u;
    m.nes.cpu.irq_captured = 1u;
    m.ram[0x40e3u] = 0x10u; /* BPL +$30 -> $4115; C4 reads $4015. */
    m.ram[0x40e4u] = 0x30u;
    m.ram[0x40e5u] = 0x5au;
    m.ram[0x4015u] = 0x40u;
    m.ram[0x4115u] = 0xeau;
    m.ram[0xfffeu] = 0x00u;
    m.ram[0xffffu] = 0x90u;
    vector_irq_ack_on_read(&m, 0x4015u);
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x4115u);
    CHECK_EQ_HEX(m.log[3].addr, 0x4015u);
    CHECK_EQ_U64(m.poll_sample[3], 1u);
    CHECK_EQ_U64(m.line_after_access[3], 0u);
    CHECK_EQ_U64(m.captured_after_access[3], 0u);
    CHECK_EQ_U64(m.irq_ack_count, 1u);
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x9000u);
    CHECK_EQ_HEX(m.ram[0x01fdu], 0x41u);
    CHECK_EQ_HEX(m.ram[0x01fcu], 0x15u);
    CHECK_EQ_HEX(m.ram[0x01fbu], 0x20u);

    /* A read on the next instruction is after the second poll. I=1 keeps
       the captured level from entering, then the guest $4015 read clears it. */
    reset_machine();
    m.nes.cpu.pc = 0x40fdu;
    m.nes.cpu.s = 0xfdu;
    m.nes.cpu.p = 0x24u;
    m.nes.cpu.irq_line = 1u;
    m.nes.cpu.irq_captured = 1u;
    m.ram[0x40fdu] = 0x10u; /* BPL +2 -> $4101, C4 reads $4001. */
    m.ram[0x40feu] = 0x02u;
    m.ram[0x40ffu] = 0x5au;
    m.ram[0x4001u] = 0x6bu;
    m.ram[0x4101u] = 0xadu; /* LDA $4015 */
    m.ram[0x4102u] = 0x15u;
    m.ram[0x4103u] = 0x40u;
    m.ram[0x4015u] = 0x40u;
    vector_irq_ack_on_read(&m, 0x4015u);
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x4101u);
    CHECK_EQ_U64(m.poll_sample[3], 1u);
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x4104u);
    CHECK_EQ_U64(m.irq_ack_count, 1u);
    CHECK_EQ_U64(m.nes.cpu.irq_line, 0u);
    CHECK_EQ_U64(m.nes.cpu.irq_captured, 0u);
    printf("CASE guard ack-after-second-poll delayed while I=1; $4015 clears before next poll\n");
}

struct irq_entry_observation {
    uint16_t pc;
    uint8_t s;
    uint8_t pch;
    uint8_t pcl;
    uint8_t p;
};

struct irq_instruction_observation {
    uint16_t pc;
    uint8_t s;
    uint8_t x;
};

static unsigned check_expected_irq_entry(const struct irq_entry_observation *o, uint16_t vector,
                                         uint8_t return_pch, uint8_t return_pcl, uint8_t pushed_p)
{
    int before = check_failures;
    CHECK_EQ_HEX(o->pc, vector);
    CHECK_EQ_HEX(o->s, 0xfau);
    CHECK_EQ_HEX(o->pch, return_pch);
    CHECK_EQ_HEX(o->pcl, return_pcl);
    CHECK_EQ_HEX(o->p, pushed_p);
    return (unsigned)(check_failures - before);
}

static void reject_wrong_irq_entry_model(const char *name,
                                         const struct irq_entry_observation *wrong, uint16_t vector,
                                         uint8_t return_pch, uint8_t return_pcl, uint8_t pushed_p)
{
    int before = check_failures;
    unsigned rejected = check_expected_irq_entry(wrong, vector, return_pch, return_pcl, pushed_p);
    check_failures = before;
    CHECK_EQ_U64(rejected, 5u);
    printf("CONTROL rejected wrong-model %s with %u intended entry assertions\n", name, rejected);
}

static void test_wrong_poll_model_control_assertions(void)
{
    const struct irq_entry_observation accepted_sei = {0x9000u, 0xfau, 0x20u, 0x01u, 0x24u};
    const struct irq_entry_observation live_i_recheck = {0x2002u, 0xfdu, 0u, 0u, 0u};
    const struct irq_entry_observation immediate_rti = {0x9000u, 0xfau, 0x40u, 0x00u, 0x20u};
    const struct irq_entry_observation delayed_rti = {0x4001u, 0xfdu, 0u, 0x01u, 0u};
    const struct irq_entry_observation missing_cross_poll = {0x2102u, 0xfdu, 0u, 0u, 0u};
    const struct irq_entry_observation cross_poll_entry = {0x9000u, 0xfau, 0x21u, 0x01u, 0x20u};
    const struct irq_entry_observation lost_first_acceptance = {0x3ff6u, 0xfdu, 0u, 0u, 0u};
    const struct irq_entry_observation first_poll_entry = {0x9000u, 0xfau, 0x3fu, 0xf5u, 0x20u};
    const struct irq_instruction_observation clean_same_page = {0x2005u, 0xfdu, 1u};

    CHECK_EQ_U64(check_expected_irq_entry(&accepted_sei, 0x9000u, 0x20u, 0x01u, 0x24u), 0u);
    reject_wrong_irq_entry_model("live-I-recheck-after-SEI", &live_i_recheck, 0x9000u, 0x20u, 0x01u,
                                 0x24u);
    CHECK_EQ_U64(check_expected_irq_entry(&immediate_rti, 0x9000u, 0x40u, 0x00u, 0x20u), 0u);
    reject_wrong_irq_entry_model("delayed-RTI-poll", &delayed_rti, 0x9000u, 0x40u, 0x00u, 0x20u);
    reject_wrong_irq_entry_model("missing-page-cross-second-poll", &missing_cross_poll, 0x9000u,
                                 0x21u, 0x01u, 0x20u);
    CHECK_EQ_U64(check_expected_irq_entry(&cross_poll_entry, 0x9000u, 0x21u, 0x01u, 0x20u), 0u);
    reject_wrong_irq_entry_model("lost-first-acceptance-after-$4015", &lost_first_acceptance,
                                 0x9000u, 0x3fu, 0xf5u, 0x20u);
    CHECK_EQ_U64(check_expected_irq_entry(&first_poll_entry, 0x9000u, 0x3fu, 0xf5u, 0x20u), 0u);

    /* Clean sibling: without an illegal C3 poll the branch target's INX runs.
       The counterfactual wrong-model result below must reject extra polling. */
    CHECK_EQ_HEX(clean_same_page.pc, 0x2005u);
    CHECK_EQ_HEX(clean_same_page.s, 0xfdu);
    CHECK_EQ_U64(clean_same_page.x, 1u);

    /* Extra same-page poll would enter instead of letting this INX run. */
    int before = check_failures;
    CHECK_EQ_HEX(0x9000u, 0x2005u);
    CHECK_EQ_U64(0u, 1u);
    CHECK_EQ_HEX(0xfau, 0xfdu);
    unsigned rejected_extra_poll = (unsigned)(check_failures - before);
    check_failures = before;
    CHECK_EQ_U64(rejected_extra_poll, 3u);
    printf("CONTROL rejected wrong-model extra-same-page-poll with %u intended assertions\n",
           rejected_extra_poll);
}

static void test_plp_uses_old_i_for_eligibility(void)
{
    const uint8_t old_i[] = {0x24u, 0x20u};
    const uint8_t restored_i[] = {0x20u, 0x24u};
    for (uint32_t i = 0u; i < 2u; i++) {
        reset_machine();
        m.nes.cpu.pc = 0x2000u;
        m.nes.cpu.s = 0xfdu;
        m.nes.cpu.p = old_i[i];
        m.nes.cpu.irq_line = 1u;
        m.nes.cpu.irq_captured = 1u;
        m.ram[0x2000u] = 0x28u; /* PLP */
        m.ram[0x2001u] = 0xe8u; /* INX if old I blocks */
        m.ram[0x01feu] = restored_i[i];
        m.ram[0xfffeu] = 0x00u;
        m.ram[0xffffu] = 0x90u;
        m.irq_schedule_enabled = 1u;
        nesturbator__cpu_step(&m.nes);
        CHECK_EQ_HEX(m.nes.cpu.pc, 0x2001u);
        CHECK_EQ_HEX(m.nes.cpu.p, restored_i[i]);
        CHECK_EQ_U64(m.poll_sample[3], 1u); /* Poll at the final pull action. */

        nesturbator__cpu_step(&m.nes);
        if (i == 0u) {
            CHECK_EQ_HEX(m.nes.cpu.pc, 0x2002u);
            CHECK_EQ_HEX(m.nes.cpu.x, 1u);
            nesturbator__cpu_step(&m.nes);
            CHECK_EQ_HEX(m.nes.cpu.pc, 0x9000u);
            CHECK_EQ_U64(m.nes.cpu.s, 0xfbu);
            CHECK_EQ_HEX(m.ram[0x01feu], 0x20u);
            CHECK_EQ_HEX(m.ram[0x01fdu], 0x02u);
            CHECK_EQ_HEX(m.ram[0x01fcu], 0x20u);
        } else {
            CHECK_EQ_HEX(m.nes.cpu.pc, 0x9000u);
            CHECK_EQ_HEX(m.nes.cpu.x, 0u);
            CHECK_EQ_U64(m.nes.cpu.s, 0xfbu);
            CHECK_EQ_HEX(m.ram[0x01feu], 0x20u);
            CHECK_EQ_HEX(m.ram[0x01fdu], 0x01u);
            CHECK_EQ_HEX(m.ram[0x01fcu], 0x24u);
        }
        printf("CASE PLP-old-I=%u-restored-I=%u captured=1\n", (old_i[i] & 4u) != 0u,
               (restored_i[i] & 4u) != 0u);
    }
}

static void prepare_rti(struct vector_machine *machine, uint8_t restored_p)
{
    reset_machine();
    machine->nes.cpu.pc = 0x3000u;
    machine->nes.cpu.s = 0xfau;
    machine->nes.cpu.p = 0x24u;
    machine->nes.cpu.irq_line = 1u;
    machine->nes.cpu.irq_captured = 1u;
    machine->ram[0x3000u] = 0x40u; /* RTI */
    machine->ram[0x4000u] = 0xeau; /* NOP after restored PC */
    machine->ram[0x01fbu] = restored_p;
    machine->ram[0x01fcu] = 0x00u;
    machine->ram[0x01fdu] = 0x40u;
    machine->ram[0xfffeu] = 0x00u;
    machine->ram[0xffffu] = 0x90u;
    machine->ram[0x9000u] = 0x40u; /* Unacknowledged handler RTI */
    machine->irq_schedule_enabled = 1u;
}

static void test_rti_restored_i_and_unacknowledged_retrigger(void)
{
    /* Restored I=1 inhibits the captured level after RTI. */
    prepare_rti(&m, 0x24u);
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x4000u);
    CHECK_EQ_HEX(m.nes.cpu.p, 0x24u);
    CHECK_EQ_HEX(m.nes.cpu.s, 0xfdu);
    CHECK_EQ_U64(m.nes.cpu.poll_latch, 1u);
    CHECK_EQ_U64(m.poll_sample[5], 1u); /* RTI's final PCH pull sample. */
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x4001u);
    CHECK_EQ_U64(m.nes.cpu.s, 0xfdu);

    /* Restored I=0 accepts the held level immediately; the handler leaves
       it unacknowledged, so its RTI is followed by another IRQ entry. */
    prepare_rti(&m, 0x20u);
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x4000u);
    CHECK_EQ_HEX(m.nes.cpu.p, 0x20u);
    CHECK_EQ_HEX(m.nes.cpu.s, 0xfdu);
    nesturbator__cpu_step(&m.nes);
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x9000u);
    CHECK_EQ_HEX(m.ram[0x01fdu], 0x40u);
    CHECK_EQ_HEX(m.ram[0x01fcu], 0x00u);
    CHECK_EQ_HEX(m.ram[0x01fbu], 0x20u);
    CHECK_EQ_HEX(m.nes.cpu.s, 0xfau);
    nesturbator__cpu_step(&m.nes); /* Handler RTI restores I=0 and PC=$4000. */
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x4000u);
    CHECK_EQ_HEX(m.nes.cpu.p, 0x20u);
    CHECK_EQ_HEX(m.nes.cpu.s, 0xfdu);
    nesturbator__cpu_step(&m.nes); /* The still-asserted IRQ retriggers now. */
    CHECK_EQ_HEX(m.nes.cpu.pc, 0x9000u);
    CHECK_EQ_HEX(m.ram[0x01fdu], 0x40u);
    CHECK_EQ_HEX(m.ram[0x01fcu], 0x00u);
    CHECK_EQ_HEX(m.ram[0x01fbu], 0x20u);
    CHECK_EQ_HEX(m.nes.cpu.s, 0xfau);
    CHECK_EQ_U64(m.irq_event_overflow, 0u);
    printf("CASE guard RTI-restored-I=1 blocks; restored-I=0 retriggers-after-unacked-RTI\n");
}

int main(void)
{
    test_jsr_overlap();
    test_jsr_uses_penultimate_irq_poll();
    test_jammed_loop();
    test_jam_pc_wrap();
    test_interrupt_entry_origins_wrap_pc_and_stack();
    test_interrupt_entry_action_is_one_bus_cycle();
    test_reset_aborts_partial_interrupt_entry();
    test_branch_opcode_access_table();
    test_branch_late_same_page_irq_is_not_a_second_poll();
    test_branch_page_cross_second_poll_and_first_acceptance();
    test_branch_irq_after_second_poll_waits_for_next_poll();
    test_branch_source_transition_windows();
    test_irq_acknowledgment_poll_windows();
    test_wrong_poll_model_control_assertions();
    test_plp_uses_old_i_for_eligibility();
    test_rti_restored_i_and_unacknowledged_retrigger();
    CHECK_DONE();
}
