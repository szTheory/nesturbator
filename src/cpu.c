/* The 2A03's 6502 core: one instruction per call, one bus access per cycle.
   Cycle order follows 64doc (nesdev.org/6502_cpu.txt) and the 65x02 vectors;
   the JAM sequence follows the NESdev Wiki "CPU unofficial opcodes",
   revision 23975.

   The only functions called from here are nesturbator__bus_read and
   nesturbator__bus_write (D-08), each in a statement of its own, so the
   order of bus cycles is the order of statements. */
#include "internal.h"

/* Flag bits of P. */
#define FLAG_Z 0x02u
#define FLAG_N 0x80u

/* Reads the byte at PC and steps PC: one cycle. */
static uint8_t fetch(struct nesturbator *nes)
{
    uint8_t v = nesturbator__bus_read(nes, nes->cpu.pc);
    nes->cpu.pc = (uint16_t)(nes->cpu.pc + 1u);
    return v;
}

/* Sets the bits in mask when cond is nonzero and clears them otherwise.
   Other bits of P stay as they are: bits 4 and 5 change only on a pull. */
static void set_flag(struct nesturbator *nes, uint8_t mask, int cond)
{
    uint8_t p = (uint8_t)(nes->cpu.p & (uint8_t)~mask);
    nes->cpu.p = cond ? (uint8_t)(p | mask) : p;
}

/* N from bit 7 of v, Z when v is zero. */
static void set_nz(struct nesturbator *nes, uint8_t v)
{
    set_flag(nes, FLAG_N, (v & 0x80u) != 0u);
    set_flag(nes, FLAG_Z, v == 0u);
}

void nesturbator__cpu_step(struct nesturbator *nes)
{
    /* A jammed CPU reads 0xFFFF every cycle until reset (D-13). */
    if (nes->cpu.jammed) {
        nesturbator__bus_read(nes, 0xFFFFu);
        return;
    }

    uint8_t opcode = fetch(nes);
    switch (opcode) {
    case 0xA9: /* LDA immediate */
        nes->cpu.a = fetch(nes);
        set_nz(nes, nes->cpu.a);
        break;

    default:
        /* JAM: after the opcode, a read of the next byte that leaves PC
           alone, then nine reads at the top of the address space, eleven
           in all; no write and no register change. An opcode not written
           yet takes this path too, so it stops the CPU where it can be
           seen. */
        nesturbator__bus_read(nes, nes->cpu.pc);
        nesturbator__bus_read(nes, 0xFFFFu);
        nesturbator__bus_read(nes, 0xFFFEu);
        nesturbator__bus_read(nes, 0xFFFEu);
        for (uint32_t i = 0u; i < 6u; i++) {
            nesturbator__bus_read(nes, 0xFFFFu);
        }
        nes->cpu.jammed = 1u;
        break;
    }
}
