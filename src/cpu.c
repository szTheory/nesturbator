/* The 2A03's 6502 core: one instruction per call, one bus access per cycle.
   Cycle order follows 64doc (nesdev.org/6502_cpu.txt) and the 65x02 vectors;
   the JAM sequence follows the NESdev Wiki "CPU unofficial opcodes",
   revision 23975.

   The only functions called from here are nesturbator__bus_read and
   nesturbator__bus_write (D-08). No expression holds two bus calls whose
   order C leaves unspecified: a helper that returns an address finishes its
   cycles before the call it is an argument of starts, so the order of bus
   cycles is the order of the source. */
#include "internal.h"

/* Flag bits of P. Bits 4 and 5 have no name here: no instruction in this
   file writes them (02-RESEARCH Pitfall 1). */
#define FLAG_C 0x01u
#define FLAG_Z 0x02u
#define FLAG_I 0x04u
#define FLAG_D 0x08u
#define FLAG_V 0x40u
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

/* Implied and accumulator modes: the cycle after the opcode reads the next
   byte and discards it; PC does not move. */
static void implied(struct nesturbator *nes)
{
    nesturbator__bus_read(nes, nes->cpu.pc);
}

/* Loads and transfers: the register takes v, N and Z follow it. */
static void assign(struct nesturbator *nes, uint8_t *reg, uint8_t v)
{
    *reg = v;
    set_nz(nes, v);
}

/* LDA, LDX, LDY from memory. */
static void load(struct nesturbator *nes, uint8_t *reg, uint16_t ea)
{
    uint8_t v = nesturbator__bus_read(nes, ea);
    assign(nes, reg, v);
}

/* Reads the operand of a read instruction at ea. */
static uint8_t read_at(struct nesturbator *nes, uint16_t ea)
{
    return nesturbator__bus_read(nes, ea);
}

/* ADC, and SBC as ADC of the operand's complement. The 2A03 has no decimal
   mode: the nes6502 README says "This version of the 6502 disabled the
   binary-coded decimal mode.", so the sum is binary whatever D holds
   (D-16). V is set when both inputs have the same sign and the result's
   sign differs. */
static void adc(struct nesturbator *nes, uint8_t v)
{
    uint32_t a = nes->cpu.a;
    uint32_t sum = a + v + (nes->cpu.p & FLAG_C);
    set_flag(nes, FLAG_C, sum > 0xFFu);
    set_flag(nes, FLAG_V, ((~(a ^ v) & (a ^ sum)) & 0x80u) != 0u);
    assign(nes, &nes->cpu.a, (uint8_t)sum);
}

static void sbc(struct nesturbator *nes, uint8_t v)
{
    adc(nes, (uint8_t)~v);
}

static void ora(struct nesturbator *nes, uint8_t v)
{
    assign(nes, &nes->cpu.a, (uint8_t)(nes->cpu.a | v));
}

static void and_op(struct nesturbator *nes, uint8_t v)
{
    assign(nes, &nes->cpu.a, (uint8_t)(nes->cpu.a & v));
}

static void eor(struct nesturbator *nes, uint8_t v)
{
    assign(nes, &nes->cpu.a, (uint8_t)(nes->cpu.a ^ v));
}

/* CMP, CPX, CPY: C when reg >= v unsigned; N and Z from reg - v. */
static void cmp_reg(struct nesturbator *nes, uint8_t reg, uint8_t v)
{
    set_flag(nes, FLAG_C, reg >= v);
    set_nz(nes, (uint8_t)(reg - v));
}

/* BIT: Z from A AND v; N and V copy bits 7 and 6 of v. */
static void bit_op(struct nesturbator *nes, uint8_t v)
{
    set_flag(nes, FLAG_Z, (nes->cpu.a & v) == 0u);
    set_flag(nes, FLAG_N, (v & 0x80u) != 0u);
    set_flag(nes, FLAG_V, (v & 0x40u) != 0u);
}

/* Register increments and decrements wrap at 8 bits. */
static void step_reg(struct nesturbator *nes, uint8_t *reg, uint8_t delta)
{
    implied(nes);
    assign(nes, reg, (uint8_t)(*reg + delta));
}

/* Flag instructions: one dummy cycle, then one bit of P. */
static void flag_op(struct nesturbator *nes, uint8_t mask, int cond)
{
    implied(nes);
    set_flag(nes, mask, cond);
}

/* Transfers between registers: one dummy cycle. */
static void transfer(struct nesturbator *nes, uint8_t *dst, uint8_t v)
{
    implied(nes);
    assign(nes, dst, v);
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
    /* Loads. */
    case 0xA9: /* LDA immediate */
        assign(nes, &nes->cpu.a, fetch(nes));
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
    case 0xA2: /* LDX immediate */
        assign(nes, &nes->cpu.x, fetch(nes));
        break;
    case 0xA6: /* LDX zp */
        load(nes, &nes->cpu.x, ea_zp(nes));
        break;
    case 0xB6: /* LDX zp,Y */
        load(nes, &nes->cpu.x, ea_zpy(nes));
        break;
    case 0xAE: /* LDX abs */
        load(nes, &nes->cpu.x, ea_abs(nes));
        break;
    case 0xBE: /* LDX abs,Y */
        load(nes, &nes->cpu.x, ea_absi(nes, nes->cpu.y, 0));
        break;
    case 0xA0: /* LDY immediate */
        assign(nes, &nes->cpu.y, fetch(nes));
        break;
    case 0xA4: /* LDY zp */
        load(nes, &nes->cpu.y, ea_zp(nes));
        break;
    case 0xB4: /* LDY zp,X */
        load(nes, &nes->cpu.y, ea_zpx(nes));
        break;
    case 0xAC: /* LDY abs */
        load(nes, &nes->cpu.y, ea_abs(nes));
        break;
    case 0xBC: /* LDY abs,X */
        load(nes, &nes->cpu.y, ea_absi(nes, nes->cpu.x, 0));
        break;

    /* Stores. abs,X, abs,Y and (zp),Y always make the dummy read. */
    case 0x85: /* STA zp */
        nesturbator__bus_write(nes, ea_zp(nes), nes->cpu.a);
        break;
    case 0x95: /* STA zp,X */
        nesturbator__bus_write(nes, ea_zpx(nes), nes->cpu.a);
        break;
    case 0x8D: /* STA abs */
        nesturbator__bus_write(nes, ea_abs(nes), nes->cpu.a);
        break;
    case 0x9D: /* STA abs,X */
        nesturbator__bus_write(nes, ea_absi(nes, nes->cpu.x, 1), nes->cpu.a);
        break;
    case 0x99: /* STA abs,Y */
        nesturbator__bus_write(nes, ea_absi(nes, nes->cpu.y, 1), nes->cpu.a);
        break;
    case 0x81: /* STA (zp,X) */
        nesturbator__bus_write(nes, ea_izx(nes), nes->cpu.a);
        break;
    case 0x91: /* STA (zp),Y */
        nesturbator__bus_write(nes, ea_izy(nes, 1), nes->cpu.a);
        break;
    case 0x86: /* STX zp */
        nesturbator__bus_write(nes, ea_zp(nes), nes->cpu.x);
        break;
    case 0x96: /* STX zp,Y */
        nesturbator__bus_write(nes, ea_zpy(nes), nes->cpu.x);
        break;
    case 0x8E: /* STX abs */
        nesturbator__bus_write(nes, ea_abs(nes), nes->cpu.x);
        break;
    case 0x84: /* STY zp */
        nesturbator__bus_write(nes, ea_zp(nes), nes->cpu.y);
        break;
    case 0x94: /* STY zp,X */
        nesturbator__bus_write(nes, ea_zpx(nes), nes->cpu.y);
        break;
    case 0x8C: /* STY abs */
        nesturbator__bus_write(nes, ea_abs(nes), nes->cpu.y);
        break;

    /* Transfers. TXS sets no flag. */
    case 0xAA: /* TAX */
        transfer(nes, &nes->cpu.x, nes->cpu.a);
        break;
    case 0xA8: /* TAY */
        transfer(nes, &nes->cpu.y, nes->cpu.a);
        break;
    case 0x8A: /* TXA */
        transfer(nes, &nes->cpu.a, nes->cpu.x);
        break;
    case 0x98: /* TYA */
        transfer(nes, &nes->cpu.a, nes->cpu.y);
        break;
    case 0xBA: /* TSX */
        transfer(nes, &nes->cpu.x, nes->cpu.s);
        break;
    case 0x9A: /* TXS */
        implied(nes);
        nes->cpu.s = nes->cpu.x;
        break;

    /* ORA */
    case 0x09:
        ora(nes, fetch(nes));
        break;
    case 0x05:
        ora(nes, read_at(nes, ea_zp(nes)));
        break;
    case 0x15:
        ora(nes, read_at(nes, ea_zpx(nes)));
        break;
    case 0x0D:
        ora(nes, read_at(nes, ea_abs(nes)));
        break;
    case 0x1D:
        ora(nes, read_at(nes, ea_absi(nes, nes->cpu.x, 0)));
        break;
    case 0x19:
        ora(nes, read_at(nes, ea_absi(nes, nes->cpu.y, 0)));
        break;
    case 0x01:
        ora(nes, read_at(nes, ea_izx(nes)));
        break;
    case 0x11:
        ora(nes, read_at(nes, ea_izy(nes, 0)));
        break;

    /* AND */
    case 0x29:
        and_op(nes, fetch(nes));
        break;
    case 0x25:
        and_op(nes, read_at(nes, ea_zp(nes)));
        break;
    case 0x35:
        and_op(nes, read_at(nes, ea_zpx(nes)));
        break;
    case 0x2D:
        and_op(nes, read_at(nes, ea_abs(nes)));
        break;
    case 0x3D:
        and_op(nes, read_at(nes, ea_absi(nes, nes->cpu.x, 0)));
        break;
    case 0x39:
        and_op(nes, read_at(nes, ea_absi(nes, nes->cpu.y, 0)));
        break;
    case 0x21:
        and_op(nes, read_at(nes, ea_izx(nes)));
        break;
    case 0x31:
        and_op(nes, read_at(nes, ea_izy(nes, 0)));
        break;

    /* EOR */
    case 0x49:
        eor(nes, fetch(nes));
        break;
    case 0x45:
        eor(nes, read_at(nes, ea_zp(nes)));
        break;
    case 0x55:
        eor(nes, read_at(nes, ea_zpx(nes)));
        break;
    case 0x4D:
        eor(nes, read_at(nes, ea_abs(nes)));
        break;
    case 0x5D:
        eor(nes, read_at(nes, ea_absi(nes, nes->cpu.x, 0)));
        break;
    case 0x59:
        eor(nes, read_at(nes, ea_absi(nes, nes->cpu.y, 0)));
        break;
    case 0x41:
        eor(nes, read_at(nes, ea_izx(nes)));
        break;
    case 0x51:
        eor(nes, read_at(nes, ea_izy(nes, 0)));
        break;

    /* ADC */
    case 0x69:
        adc(nes, fetch(nes));
        break;
    case 0x65:
        adc(nes, read_at(nes, ea_zp(nes)));
        break;
    case 0x75:
        adc(nes, read_at(nes, ea_zpx(nes)));
        break;
    case 0x6D:
        adc(nes, read_at(nes, ea_abs(nes)));
        break;
    case 0x7D:
        adc(nes, read_at(nes, ea_absi(nes, nes->cpu.x, 0)));
        break;
    case 0x79:
        adc(nes, read_at(nes, ea_absi(nes, nes->cpu.y, 0)));
        break;
    case 0x61:
        adc(nes, read_at(nes, ea_izx(nes)));
        break;
    case 0x71:
        adc(nes, read_at(nes, ea_izy(nes, 0)));
        break;

    /* SBC */
    case 0xE9:
        sbc(nes, fetch(nes));
        break;
    case 0xE5:
        sbc(nes, read_at(nes, ea_zp(nes)));
        break;
    case 0xF5:
        sbc(nes, read_at(nes, ea_zpx(nes)));
        break;
    case 0xED:
        sbc(nes, read_at(nes, ea_abs(nes)));
        break;
    case 0xFD:
        sbc(nes, read_at(nes, ea_absi(nes, nes->cpu.x, 0)));
        break;
    case 0xF9:
        sbc(nes, read_at(nes, ea_absi(nes, nes->cpu.y, 0)));
        break;
    case 0xE1:
        sbc(nes, read_at(nes, ea_izx(nes)));
        break;
    case 0xF1:
        sbc(nes, read_at(nes, ea_izy(nes, 0)));
        break;

    /* CMP */
    case 0xC9:
        cmp_reg(nes, nes->cpu.a, fetch(nes));
        break;
    case 0xC5:
        cmp_reg(nes, nes->cpu.a, read_at(nes, ea_zp(nes)));
        break;
    case 0xD5:
        cmp_reg(nes, nes->cpu.a, read_at(nes, ea_zpx(nes)));
        break;
    case 0xCD:
        cmp_reg(nes, nes->cpu.a, read_at(nes, ea_abs(nes)));
        break;
    case 0xDD:
        cmp_reg(nes, nes->cpu.a, read_at(nes, ea_absi(nes, nes->cpu.x, 0)));
        break;
    case 0xD9:
        cmp_reg(nes, nes->cpu.a, read_at(nes, ea_absi(nes, nes->cpu.y, 0)));
        break;
    case 0xC1:
        cmp_reg(nes, nes->cpu.a, read_at(nes, ea_izx(nes)));
        break;
    case 0xD1:
        cmp_reg(nes, nes->cpu.a, read_at(nes, ea_izy(nes, 0)));
        break;

    /* CPX and CPY */
    case 0xE0:
        cmp_reg(nes, nes->cpu.x, fetch(nes));
        break;
    case 0xE4:
        cmp_reg(nes, nes->cpu.x, read_at(nes, ea_zp(nes)));
        break;
    case 0xEC:
        cmp_reg(nes, nes->cpu.x, read_at(nes, ea_abs(nes)));
        break;
    case 0xC0:
        cmp_reg(nes, nes->cpu.y, fetch(nes));
        break;
    case 0xC4:
        cmp_reg(nes, nes->cpu.y, read_at(nes, ea_zp(nes)));
        break;
    case 0xCC:
        cmp_reg(nes, nes->cpu.y, read_at(nes, ea_abs(nes)));
        break;

    /* BIT */
    case 0x24:
        bit_op(nes, read_at(nes, ea_zp(nes)));
        break;
    case 0x2C:
        bit_op(nes, read_at(nes, ea_abs(nes)));
        break;

    /* Flags. SED and CLD only store D; nothing reads it (D-16). */
    case 0x18: /* CLC */
        flag_op(nes, FLAG_C, 0);
        break;
    case 0x38: /* SEC */
        flag_op(nes, FLAG_C, 1);
        break;
    case 0x58: /* CLI */
        flag_op(nes, FLAG_I, 0);
        break;
    case 0x78: /* SEI */
        flag_op(nes, FLAG_I, 1);
        break;
    case 0xB8: /* CLV */
        flag_op(nes, FLAG_V, 0);
        break;
    case 0xD8: /* CLD */
        flag_op(nes, FLAG_D, 0);
        break;
    case 0xF8: /* SED */
        flag_op(nes, FLAG_D, 1);
        break;

    /* Register increments and decrements. */
    case 0xE8: /* INX */
        step_reg(nes, &nes->cpu.x, 0x01u);
        break;
    case 0xC8: /* INY */
        step_reg(nes, &nes->cpu.y, 0x01u);
        break;
    case 0xCA: /* DEX */
        step_reg(nes, &nes->cpu.x, 0xFFu);
        break;
    case 0x88: /* DEY */
        step_reg(nes, &nes->cpu.y, 0xFFu);
        break;

    case 0xEA: /* NOP */
        implied(nes);
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
