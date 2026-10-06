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

int main(void)
{
    test_jsr_overlap();
    test_jammed_loop();
    test_jam_pc_wrap();
    CHECK_DONE();
}
