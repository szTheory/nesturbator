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

/* Effective addresses. Each helper makes exactly the bus accesses of its
   addressing mode up to, not including, the final access at the address it
   returns (64doc, "addressing modes"). Indexed low-byte sums and zero-page
   pointers wrap within their page. */

/* zp: the operand is the address. */
static uint16_t ea_zp(struct nesturbator *nes)
{
    return fetch(nes);
}

/* zp,X and zp,Y: a dummy read at the unindexed address while the index is
   added, and the sum wraps within zero page. */
static uint16_t ea_zpi(struct nesturbator *nes, uint8_t idx)
{
    uint8_t b = fetch(nes);
    nesturbator__bus_read(nes, b);
    return (uint8_t)(b + idx);
}

static uint16_t ea_zpx(struct nesturbator *nes)
{
    return ea_zpi(nes, nes->cpu.x);
}

static uint16_t ea_zpy(struct nesturbator *nes)
{
    return ea_zpi(nes, nes->cpu.y);
}

/* abs: low byte, then high byte. */
static uint16_t ea_abs(struct nesturbator *nes)
{
    uint8_t lo = fetch(nes);
    uint8_t hi = fetch(nes);
    return (uint16_t)(((uint32_t)hi << 8) | lo);
}

/* Adds idx to the base hi:lo. The CPU first reads at hi with the low byte
   already summed; that read is a dummy when the sum carries into the high
   byte. Reads skip it when there is no carry; writes and read-modify-writes
   always make it (always_dummy). */
static uint16_t index_base(struct nesturbator *nes, uint8_t lo, uint8_t hi, uint8_t idx,
                           int always_dummy)
{
    uint32_t sum = (uint32_t)lo + idx;
    if (sum > 0xFFu || always_dummy) {
        nesturbator__bus_read(nes, (uint16_t)(((uint32_t)hi << 8) | (sum & 0xFFu)));
    }
    return (uint16_t)(((uint32_t)hi << 8) + sum);
}

/* abs,X and abs,Y. */
static uint16_t ea_absi(struct nesturbator *nes, uint8_t idx, int always_dummy)
{
    uint8_t lo = fetch(nes);
    uint8_t hi = fetch(nes);
    return index_base(nes, lo, hi, idx, always_dummy);
}

/* (zp,X): a dummy read at the operand while X is added, then the pointer
   bytes at (b + X) and (b + X + 1), both wrapping within zero page. */
static uint16_t ea_izx(struct nesturbator *nes)
{
    uint8_t b = fetch(nes);
    nesturbator__bus_read(nes, b);
    uint8_t ptr = (uint8_t)(b + nes->cpu.x);
    uint8_t lo = nesturbator__bus_read(nes, ptr);
    uint8_t hi = nesturbator__bus_read(nes, (uint8_t)(ptr + 1u));
    return (uint16_t)(((uint32_t)hi << 8) | lo);
}

/* (zp),Y: the pointer bytes at p and (p + 1) within zero page, then Y is
   added as for abs,Y. */
static uint16_t ea_izy(struct nesturbator *nes, int always_dummy)
{
    uint8_t p = fetch(nes);
    uint8_t lo = nesturbator__bus_read(nes, p);
    uint8_t hi = nesturbator__bus_read(nes, (uint8_t)(p + 1u));
    return index_base(nes, lo, hi, nes->cpu.y, always_dummy);
}

/* LDA, LDX, LDY: load the register, set N and Z. */
static void load(struct nesturbator *nes, uint8_t *reg, uint16_t ea)
{
    *reg = nesturbator__bus_read(nes, ea);
    set_nz(nes, *reg);
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
    case 0xA5: /* LDA zp */
        load(nes, &nes->cpu.a, ea_zp(nes));
        break;
    case 0xB5: /* LDA zp,X */
        load(nes, &nes->cpu.a, ea_zpx(nes));
        break;
    case 0xAD: /* LDA abs */
        load(nes, &nes->cpu.a, ea_abs(nes));
        break;
    case 0xBD: /* LDA abs,X */
        load(nes, &nes->cpu.a, ea_absi(nes, nes->cpu.x, 0));
        break;
    case 0xB9: /* LDA abs,Y */
        load(nes, &nes->cpu.a, ea_absi(nes, nes->cpu.y, 0));
        break;
    case 0xA1: /* LDA (zp,X) */
        load(nes, &nes->cpu.a, ea_izx(nes));
        break;
    case 0xB1: /* LDA (zp),Y */
        load(nes, &nes->cpu.a, ea_izy(nes, 0));
        break;
    case 0xB6: /* LDX zp,Y */
        load(nes, &nes->cpu.x, ea_zpy(nes));
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
