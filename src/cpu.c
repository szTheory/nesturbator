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

/* Flag bits of P. Bits 4 (B) and 5 exist only on the stack: PHP and BRK push
   P | 0x30, and PLP and RTI load (v & ~0x10) | 0x20. No other instruction
   writes them (02-RESEARCH Pitfall 1; D-16). */
#define FLAG_C 0x01u
#define FLAG_Z 0x02u
#define FLAG_I 0x04u
#define FLAG_D 0x08u
#define FLAG_B 0x10u
#define FLAG_U 0x20u
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

/* Shifts and rotates for the accumulator and read-modify-write forms. Each
   touches only N, Z and C. */
typedef uint8_t (*nesturbator_rmw_op)(struct nesturbator *nes, uint8_t v);

/* ASL: bit 7 goes to C, a zero comes in at bit 0. */
static uint8_t asl(struct nesturbator *nes, uint8_t v)
{
    uint8_t r = (uint8_t)(v << 1);
    set_flag(nes, FLAG_C, (v & 0x80u) != 0u);
    set_nz(nes, r);
    return r;
}

/* LSR: bit 0 goes to C, a zero comes in at bit 7. */
static uint8_t lsr(struct nesturbator *nes, uint8_t v)
{
    uint8_t r = (uint8_t)(v >> 1);
    set_flag(nes, FLAG_C, (v & 0x01u) != 0u);
    set_nz(nes, r);
    return r;
}

/* ROL: bit 7 goes to C, the old C comes in at bit 0. */
static uint8_t rol(struct nesturbator *nes, uint8_t v)
{
    uint8_t r = (uint8_t)((uint32_t)(v << 1) | (nes->cpu.p & FLAG_C));
    set_flag(nes, FLAG_C, (v & 0x80u) != 0u);
    set_nz(nes, r);
    return r;
}

/* ROR: bit 0 goes to C, the old C comes in at bit 7. */
static uint8_t ror(struct nesturbator *nes, uint8_t v)
{
    uint8_t r = (uint8_t)((uint32_t)(v >> 1) | ((uint32_t)(nes->cpu.p & FLAG_C) << 7));
    set_flag(nes, FLAG_C, (v & 0x01u) != 0u);
    set_nz(nes, r);
    return r;
}

/* INC and DEC in memory: N and Z only. */
static uint8_t inc(struct nesturbator *nes, uint8_t v)
{
    uint8_t r = (uint8_t)(v + 1u);
    set_nz(nes, r);
    return r;
}

static uint8_t dec(struct nesturbator *nes, uint8_t v)
{
    uint8_t r = (uint8_t)(v - 1u);
    set_nz(nes, r);
    return r;
}

/* The combined unofficial read-modify-writes: the shift, rotate, increment
   or decrement writes memory, then its result feeds an ALU operation on A
   (NESdev Wiki "CPU unofficial opcodes", revision 23975). The timing is that
   of rmw. SLO: ASL then ORA. RLA: ROL then AND. SRE: LSR then EOR. RRA: ROR
   then ADC with the C that ROR left. DCP: DEC then CMP. ISC: INC then SBC. */
static uint8_t slo(struct nesturbator *nes, uint8_t v)
{
    uint8_t r = asl(nes, v);
    ora(nes, r);
    return r;
}

static uint8_t rla(struct nesturbator *nes, uint8_t v)
{
    uint8_t r = rol(nes, v);
    and_op(nes, r);
    return r;
}

static uint8_t sre(struct nesturbator *nes, uint8_t v)
{
    uint8_t r = lsr(nes, v);
    eor(nes, r);
    return r;
}

static uint8_t rra(struct nesturbator *nes, uint8_t v)
{
    uint8_t r = ror(nes, v);
    adc(nes, r);
    return r;
}

static uint8_t dcp(struct nesturbator *nes, uint8_t v)
{
    uint8_t r = (uint8_t)(v - 1u);
    cmp_reg(nes, nes->cpu.a, r);
    return r;
}

static uint8_t isc(struct nesturbator *nes, uint8_t v)
{
    uint8_t r = (uint8_t)(v + 1u);
    sbc(nes, r);
    return r;
}

/* LAX: A and X both take the value read; N and Z follow it. */
static void lax(struct nesturbator *nes, uint8_t v)
{
    nes->cpu.x = v;
    assign(nes, &nes->cpu.a, v);
}

/* ANC: AND, then C copies N. */
static void anc(struct nesturbator *nes, uint8_t v)
{
    and_op(nes, v);
    set_flag(nes, FLAG_C, (nes->cpu.a & 0x80u) != 0u);
}

/* ARR: t = A AND v, then A = t rotated right through C. N and Z follow A;
   C is bit 6 of A and V is bit 6 XOR bit 5. No other flag changes. */
static void arr(struct nesturbator *nes, uint8_t v)
{
    uint32_t t = (uint32_t)(nes->cpu.a & v);
    uint8_t r = (uint8_t)((t >> 1) | ((uint32_t)(nes->cpu.p & FLAG_C) << 7));
    assign(nes, &nes->cpu.a, r);
    set_flag(nes, FLAG_C, (r & 0x40u) != 0u);
    set_flag(nes, FLAG_V, ((((uint32_t)r >> 6) ^ ((uint32_t)r >> 5)) & 1u) != 0u);
}

/* SBX: X = (A AND X) - v without borrow in; C, N and Z as CMP sets them. */
static void sbx(struct nesturbator *nes, uint8_t v)
{
    uint8_t t = (uint8_t)(nes->cpu.a & nes->cpu.x);
    cmp_reg(nes, t, v);
    nes->cpu.x = (uint8_t)(t - v);
}

/* The accumulator forms of ASL, LSR, ROL and ROR: one dummy cycle. */
static void accumulator(struct nesturbator *nes, nesturbator_rmw_op op)
{
    implied(nes);
    nes->cpu.a = op(nes, nes->cpu.a);
}

/* Read-modify-write at ea: the CPU reads v, writes v back unchanged while it
   computes, then writes the result (64doc, "Read-Modify-Write instructions").
   Both writes are real bus accesses. */
static void rmw(struct nesturbator *nes, uint16_t ea, nesturbator_rmw_op op)
{
    uint8_t v = nesturbator__bus_read(nes, ea);
    nesturbator__bus_write(nes, ea, v);
    uint8_t r = op(nes, v);
    nesturbator__bus_write(nes, ea, r);
}

/* The stack is page 1. A push writes at 0x0100 + S and then decrements S; a
   pull increments S and then reads (64doc, "stack"). S wraps within the
   page. */
static void push(struct nesturbator *nes, uint8_t v)
{
    nesturbator__bus_write(nes, (uint16_t)(0x0100u | nes->cpu.s), v);
    nes->cpu.s = (uint8_t)(nes->cpu.s - 1u);
}

static uint8_t pull(struct nesturbator *nes)
{
    nes->cpu.s = (uint8_t)(nes->cpu.s + 1u);
    return nesturbator__bus_read(nes, (uint16_t)(0x0100u | nes->cpu.s));
}

/* The dummy read of the stack at 0x0100 + S that PLA, PLP, RTS, RTI and JSR
   make before S moves. */
static void stack_dummy(struct nesturbator *nes)
{
    nesturbator__bus_read(nes, (uint16_t)(0x0100u | nes->cpu.s));
}

/* Conditional branches: the offset byte, then, when taken, a dummy read at
   PC while the low byte is added, and a second dummy read at
   (old PCH << 8) | new PCL when the target is in another page (64doc,
   "Relative addressing"). The offset is signed. */
static void branch(struct nesturbator *nes, int cond)
{
    uint8_t off = fetch(nes);
    if (!cond) {
        return;
    }
    implied(nes);
    uint16_t pc = nes->cpu.pc;
    uint32_t delta = (off & 0x80u) != 0u ? (uint32_t)off + 0xFF00u : (uint32_t)off;
    uint16_t target = (uint16_t)(pc + delta);
    if ((target & 0xFF00u) != (pc & 0xFF00u)) {
        nesturbator__bus_read(nes, (uint16_t)((pc & 0xFF00u) | (target & 0x00FFu)));
    }
    nes->cpu.pc = target;
}

/* PLP and RTI: bit 4 clear and bit 5 set whatever the stack held (D-16). */
static void pull_p(struct nesturbator *nes)
{
    uint8_t v = pull(nes);
    nes->cpu.p = (uint8_t)((v & (uint8_t)~FLAG_B) | FLAG_U);
}

/* PHP and BRK push P with bits 4 and 5 set; P itself is unchanged (D-16). */
static void push_p(struct nesturbator *nes)
{
    push(nes, (uint8_t)(nes->cpu.p | FLAG_B | FLAG_U));
}

/* Reads a little-endian address: low byte at lo_addr, high byte at
   hi_addr. */
static uint16_t read_addr(struct nesturbator *nes, uint16_t lo_addr, uint16_t hi_addr)
{
    uint8_t lo = nesturbator__bus_read(nes, lo_addr);
    uint8_t hi = nesturbator__bus_read(nes, hi_addr);
    return (uint16_t)(((uint32_t)hi << 8) | lo);
}

/* JAM (0x02 0x12 0x22 0x32 0x42 0x52 0x62 0x72 0x92 0xB2 0xD2 0xF2): after
   the opcode, a read of the next byte that leaves PC alone, then reads of
   0xFFFF, 0xFFFE, 0xFFFE and six of 0xFFFF, eleven in all. No write, no
   register or flag change; the CPU stays jammed until reset (D-13; NESdev
   Wiki "CPU unofficial opcodes" revision 23975; 65x02 commit b7ed828, issue
   1). */
static void jam(struct nesturbator *nes)
{
    nesturbator__bus_read(nes, nes->cpu.pc);
    nesturbator__bus_read(nes, 0xFFFFu);
    nesturbator__bus_read(nes, 0xFFFEu);
    nesturbator__bus_read(nes, 0xFFFEu);
    for (uint32_t i = 0u; i < 6u; i++) {
        nesturbator__bus_read(nes, 0xFFFFu);
    }
    nes->cpu.jammed = 1u;
}

/* SHY, SHX, SHA and TAS store reg AND (H + 1), where hi:lo is the base
   address before idx is added. The dummy read at the uncorrected address is
   always made. On a page cross the stored value also replaces the high byte
   of the address (D-15; NESdev Wiki "CPU unofficial opcodes" revision
   23975). When DMC DMA halts the CPU in this read, the AND with H + 1 drops
   out; that RDY case joins here with the DMA (deferred). */
static void store_sh(struct nesturbator *nes, uint8_t reg, uint8_t lo, uint8_t hi, uint8_t idx)
{
    uint32_t sum = (uint32_t)lo + idx;
    uint8_t value = (uint8_t)(reg & (uint8_t)(hi + 1u));
    uint8_t high = sum > 0xFFu ? value : hi;
    nesturbator__bus_read(nes, (uint16_t)(((uint32_t)hi << 8) | (sum & 0xFFu)));
    nesturbator__bus_write(nes, (uint16_t)(((uint32_t)high << 8) | (sum & 0xFFu)), value);
}

/* The abs,X and abs,Y forms of store_sh: the two address bytes, then the
   store. */
static void store_sh_abs(struct nesturbator *nes, uint8_t reg, uint8_t idx)
{
    uint8_t lo = fetch(nes);
    uint8_t hi = fetch(nes);
    store_sh(nes, reg, lo, hi, idx);
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

    /* Shifts and rotates. Read-modify-write abs,X always makes the dummy
       read. */
    case 0x0A: /* ASL A */
        accumulator(nes, asl);
        break;
    case 0x06: /* ASL zp */
        rmw(nes, ea_zp(nes), asl);
        break;
    case 0x16: /* ASL zp,X */
        rmw(nes, ea_zpx(nes), asl);
        break;
    case 0x0E: /* ASL abs */
        rmw(nes, ea_abs(nes), asl);
        break;
    case 0x1E: /* ASL abs,X */
        rmw(nes, ea_absi(nes, nes->cpu.x, 1), asl);
        break;
    case 0x4A: /* LSR A */
        accumulator(nes, lsr);
        break;
    case 0x46: /* LSR zp */
        rmw(nes, ea_zp(nes), lsr);
        break;
    case 0x56: /* LSR zp,X */
        rmw(nes, ea_zpx(nes), lsr);
        break;
    case 0x4E: /* LSR abs */
        rmw(nes, ea_abs(nes), lsr);
        break;
    case 0x5E: /* LSR abs,X */
        rmw(nes, ea_absi(nes, nes->cpu.x, 1), lsr);
        break;
    case 0x2A: /* ROL A */
        accumulator(nes, rol);
        break;
    case 0x26: /* ROL zp */
        rmw(nes, ea_zp(nes), rol);
        break;
    case 0x36: /* ROL zp,X */
        rmw(nes, ea_zpx(nes), rol);
        break;
    case 0x2E: /* ROL abs */
        rmw(nes, ea_abs(nes), rol);
        break;
    case 0x3E: /* ROL abs,X */
        rmw(nes, ea_absi(nes, nes->cpu.x, 1), rol);
        break;
    case 0x6A: /* ROR A */
        accumulator(nes, ror);
        break;
    case 0x66: /* ROR zp */
        rmw(nes, ea_zp(nes), ror);
        break;
    case 0x76: /* ROR zp,X */
        rmw(nes, ea_zpx(nes), ror);
        break;
    case 0x6E: /* ROR abs */
        rmw(nes, ea_abs(nes), ror);
        break;
    case 0x7E: /* ROR abs,X */
        rmw(nes, ea_absi(nes, nes->cpu.x, 1), ror);
        break;

    /* INC and DEC in memory. */
    case 0xE6: /* INC zp */
        rmw(nes, ea_zp(nes), inc);
        break;
    case 0xF6: /* INC zp,X */
        rmw(nes, ea_zpx(nes), inc);
        break;
    case 0xEE: /* INC abs */
        rmw(nes, ea_abs(nes), inc);
        break;
    case 0xFE: /* INC abs,X */
        rmw(nes, ea_absi(nes, nes->cpu.x, 1), inc);
        break;
    case 0xC6: /* DEC zp */
        rmw(nes, ea_zp(nes), dec);
        break;
    case 0xD6: /* DEC zp,X */
        rmw(nes, ea_zpx(nes), dec);
        break;
    case 0xCE: /* DEC abs */
        rmw(nes, ea_abs(nes), dec);
        break;
    case 0xDE: /* DEC abs,X */
        rmw(nes, ea_absi(nes, nes->cpu.x, 1), dec);
        break;

    /* Stack. */
    case 0x48: /* PHA */
        implied(nes);
        push(nes, nes->cpu.a);
        break;
    case 0x08: /* PHP */
        implied(nes);
        push_p(nes);
        break;
    case 0x68: /* PLA */
        implied(nes);
        stack_dummy(nes);
        assign(nes, &nes->cpu.a, pull(nes));
        break;
    case 0x28: /* PLP */
        implied(nes);
        stack_dummy(nes);
        pull_p(nes);
        break;

    /* Branches. */
    case 0x10: /* BPL */
        branch(nes, (nes->cpu.p & FLAG_N) == 0u);
        break;
    case 0x30: /* BMI */
        branch(nes, (nes->cpu.p & FLAG_N) != 0u);
        break;
    case 0x50: /* BVC */
        branch(nes, (nes->cpu.p & FLAG_V) == 0u);
        break;
    case 0x70: /* BVS */
        branch(nes, (nes->cpu.p & FLAG_V) != 0u);
        break;
    case 0x90: /* BCC */
        branch(nes, (nes->cpu.p & FLAG_C) == 0u);
        break;
    case 0xB0: /* BCS */
        branch(nes, (nes->cpu.p & FLAG_C) != 0u);
        break;
    case 0xD0: /* BNE */
        branch(nes, (nes->cpu.p & FLAG_Z) == 0u);
        break;
    case 0xF0: /* BEQ */
        branch(nes, (nes->cpu.p & FLAG_Z) != 0u);
        break;

    /* Jumps. */
    case 0x4C: /* JMP abs */
        nes->cpu.pc = ea_abs(nes);
        break;
    case 0x6C: { /* JMP (ind): the pointer's high byte comes from the same
                    page, so a pointer at xxFF reads xx00 (64doc). */
        uint16_t ptr = ea_abs(nes);
        uint16_t next = (uint16_t)((ptr & 0xFF00u) | ((ptr + 1u) & 0x00FFu));
        nes->cpu.pc = read_addr(nes, ptr, next);
        break;
    }

    /* RTS: a dummy read at PC, a dummy stack read, PCL and PCH pulled, a
       dummy read at the pulled address, then PC steps past it. */
    case 0x60: {
        implied(nes);
        stack_dummy(nes);
        uint8_t lo = pull(nes);
        uint8_t hi = pull(nes);
        nes->cpu.pc = (uint16_t)(((uint32_t)hi << 8) | lo);
        fetch(nes);
        break;
    }

    /* RTI: a dummy read at PC, a dummy stack read, then P, PCL and PCH
       pulled (D-16). */
    case 0x40: {
        implied(nes);
        stack_dummy(nes);
        pull_p(nes);
        uint8_t lo = pull(nes);
        uint8_t hi = pull(nes);
        nes->cpu.pc = (uint16_t)(((uint32_t)hi << 8) | lo);
        break;
    }

    /* BRK: the byte after the opcode is read and skipped, the return
       address and P | 0x30 are pushed, I is set and D is left alone (D-16),
       and PC loads from 0xFFFE/0xFFFF. */
    case 0x00:
        fetch(nes);
        push(nes, (uint8_t)(nes->cpu.pc >> 8));
        push(nes, (uint8_t)nes->cpu.pc);
        push_p(nes);
        set_flag(nes, FLAG_I, 1);
        nes->cpu.pc = read_addr(nes, 0xFFFEu, 0xFFFFu);
        break;

    /* JSR: the low address byte, a dummy stack read, the return address
       (the last operand byte) pushed high byte first, then the high address
       byte, read after the pushes so a push over the operand is seen
       (D-17). */
    case 0x20: {
        uint8_t lo = fetch(nes);
        stack_dummy(nes);
        push(nes, (uint8_t)(nes->cpu.pc >> 8));
        push(nes, (uint8_t)nes->cpu.pc);
        uint8_t hi = fetch(nes);
        nes->cpu.pc = (uint16_t)(((uint32_t)hi << 8) | lo);
        break;
    }

    /* Unofficial NOPs (NESdev Wiki "CPU unofficial opcodes", revision
       23975). Each makes the bus accesses of its addressing mode and changes
       nothing; the memory forms read the operand, and abs,X makes the dummy
       read only on a page cross, as a read does. */
    case 0x1A:
    case 0x3A:
    case 0x5A:
    case 0x7A:
    case 0xDA:
    case 0xFA:
        implied(nes);
        break;
    case 0x80:
    case 0x82:
    case 0x89:
    case 0xC2:
    case 0xE2:
        fetch(nes);
        break;
    case 0x04:
    case 0x44:
    case 0x64:
        read_at(nes, ea_zp(nes));
        break;
    case 0x14:
    case 0x34:
    case 0x54:
    case 0x74:
    case 0xD4:
    case 0xF4:
        read_at(nes, ea_zpx(nes));
        break;
    case 0x0C:
        read_at(nes, ea_abs(nes));
        break;
    case 0x1C:
    case 0x3C:
    case 0x5C:
    case 0x7C:
    case 0xDC:
    case 0xFC:
        read_at(nes, ea_absi(nes, nes->cpu.x, 0));
        break;

    /* LAX: load A and X. */
    case 0xA7: /* LAX zp */
        lax(nes, read_at(nes, ea_zp(nes)));
        break;
    case 0xB7: /* LAX zp,Y */
        lax(nes, read_at(nes, ea_zpy(nes)));
        break;
    case 0xAF: /* LAX abs */
        lax(nes, read_at(nes, ea_abs(nes)));
        break;
    case 0xBF: /* LAX abs,Y */
        lax(nes, read_at(nes, ea_absi(nes, nes->cpu.y, 0)));
        break;
    case 0xA3: /* LAX (zp,X) */
        lax(nes, read_at(nes, ea_izx(nes)));
        break;
    case 0xB3: /* LAX (zp),Y */
        lax(nes, read_at(nes, ea_izy(nes, 0)));
        break;

    /* SAX: store A AND X; no flag changes. */
    case 0x87: /* SAX zp */
        nesturbator__bus_write(nes, ea_zp(nes), (uint8_t)(nes->cpu.a & nes->cpu.x));
        break;
    case 0x97: /* SAX zp,Y */
        nesturbator__bus_write(nes, ea_zpy(nes), (uint8_t)(nes->cpu.a & nes->cpu.x));
        break;
    case 0x8F: /* SAX abs */
        nesturbator__bus_write(nes, ea_abs(nes), (uint8_t)(nes->cpu.a & nes->cpu.x));
        break;
    case 0x83: /* SAX (zp,X) */
        nesturbator__bus_write(nes, ea_izx(nes), (uint8_t)(nes->cpu.a & nes->cpu.x));
        break;

    /* Combined read-modify-writes. abs,X, abs,Y and (zp),Y always make the
       dummy read, as a read-modify-write does. */
    case 0x03: /* SLO (zp,X) */
        rmw(nes, ea_izx(nes), slo);
        break;
    case 0x07: /* SLO zp */
        rmw(nes, ea_zp(nes), slo);
        break;
    case 0x0F: /* SLO abs */
        rmw(nes, ea_abs(nes), slo);
        break;
    case 0x13: /* SLO (zp),Y */
        rmw(nes, ea_izy(nes, 1), slo);
        break;
    case 0x17: /* SLO zp,X */
        rmw(nes, ea_zpx(nes), slo);
        break;
    case 0x1B: /* SLO abs,Y */
        rmw(nes, ea_absi(nes, nes->cpu.y, 1), slo);
        break;
    case 0x1F: /* SLO abs,X */
        rmw(nes, ea_absi(nes, nes->cpu.x, 1), slo);
        break;
    case 0x23: /* RLA (zp,X) */
        rmw(nes, ea_izx(nes), rla);
        break;
    case 0x27: /* RLA zp */
        rmw(nes, ea_zp(nes), rla);
        break;
    case 0x2F: /* RLA abs */
        rmw(nes, ea_abs(nes), rla);
        break;
    case 0x33: /* RLA (zp),Y */
        rmw(nes, ea_izy(nes, 1), rla);
        break;
    case 0x37: /* RLA zp,X */
        rmw(nes, ea_zpx(nes), rla);
        break;
    case 0x3B: /* RLA abs,Y */
        rmw(nes, ea_absi(nes, nes->cpu.y, 1), rla);
        break;
    case 0x3F: /* RLA abs,X */
        rmw(nes, ea_absi(nes, nes->cpu.x, 1), rla);
        break;
    case 0x43: /* SRE (zp,X) */
        rmw(nes, ea_izx(nes), sre);
        break;
    case 0x47: /* SRE zp */
        rmw(nes, ea_zp(nes), sre);
        break;
    case 0x4F: /* SRE abs */
        rmw(nes, ea_abs(nes), sre);
        break;
    case 0x53: /* SRE (zp),Y */
        rmw(nes, ea_izy(nes, 1), sre);
        break;
    case 0x57: /* SRE zp,X */
        rmw(nes, ea_zpx(nes), sre);
        break;
    case 0x5B: /* SRE abs,Y */
        rmw(nes, ea_absi(nes, nes->cpu.y, 1), sre);
        break;
    case 0x5F: /* SRE abs,X */
        rmw(nes, ea_absi(nes, nes->cpu.x, 1), sre);
        break;
    case 0x63: /* RRA (zp,X) */
        rmw(nes, ea_izx(nes), rra);
        break;
    case 0x67: /* RRA zp */
        rmw(nes, ea_zp(nes), rra);
        break;
    case 0x6F: /* RRA abs */
        rmw(nes, ea_abs(nes), rra);
        break;
    case 0x73: /* RRA (zp),Y */
        rmw(nes, ea_izy(nes, 1), rra);
        break;
    case 0x77: /* RRA zp,X */
        rmw(nes, ea_zpx(nes), rra);
        break;
    case 0x7B: /* RRA abs,Y */
        rmw(nes, ea_absi(nes, nes->cpu.y, 1), rra);
        break;
    case 0x7F: /* RRA abs,X */
        rmw(nes, ea_absi(nes, nes->cpu.x, 1), rra);
        break;
    case 0xC3: /* DCP (zp,X) */
        rmw(nes, ea_izx(nes), dcp);
        break;
    case 0xC7: /* DCP zp */
        rmw(nes, ea_zp(nes), dcp);
        break;
    case 0xCF: /* DCP abs */
        rmw(nes, ea_abs(nes), dcp);
        break;
    case 0xD3: /* DCP (zp),Y */
        rmw(nes, ea_izy(nes, 1), dcp);
        break;
    case 0xD7: /* DCP zp,X */
        rmw(nes, ea_zpx(nes), dcp);
        break;
    case 0xDB: /* DCP abs,Y */
        rmw(nes, ea_absi(nes, nes->cpu.y, 1), dcp);
        break;
    case 0xDF: /* DCP abs,X */
        rmw(nes, ea_absi(nes, nes->cpu.x, 1), dcp);
        break;
    case 0xE3: /* ISC (zp,X) */
        rmw(nes, ea_izx(nes), isc);
        break;
    case 0xE7: /* ISC zp */
        rmw(nes, ea_zp(nes), isc);
        break;
    case 0xEF: /* ISC abs */
        rmw(nes, ea_abs(nes), isc);
        break;
    case 0xF3: /* ISC (zp),Y */
        rmw(nes, ea_izy(nes, 1), isc);
        break;
    case 0xF7: /* ISC zp,X */
        rmw(nes, ea_zpx(nes), isc);
        break;
    case 0xFB: /* ISC abs,Y */
        rmw(nes, ea_absi(nes, nes->cpu.y, 1), isc);
        break;
    case 0xFF: /* ISC abs,X */
        rmw(nes, ea_absi(nes, nes->cpu.x, 1), isc);
        break;

    /* Unofficial immediate ALU operations. */
    case 0x0B: /* ANC */
    case 0x2B:
        anc(nes, fetch(nes));
        break;
    case 0x4B: /* ALR: AND, then LSR of A */
        nes->cpu.a = lsr(nes, (uint8_t)(nes->cpu.a & fetch(nes)));
        break;
    case 0x6B: /* ARR */
        arr(nes, fetch(nes));
        break;
    case 0xCB: /* SBX */
        sbx(nes, fetch(nes));
        break;
    case 0xEB: /* SBC immediate */
        sbc(nes, fetch(nes));
        break;

    /* ANE and LXA mix A with a chip-dependent constant from the machine
       profile (D-14). */
    case 0x8B: /* ANE */
        assign(nes, &nes->cpu.a,
               (uint8_t)((nes->cpu.a | nes->profile.ane_magic) & nes->cpu.x & fetch(nes)));
        break;
    case 0xAB: /* LXA */
        nes->cpu.x = (uint8_t)((nes->cpu.a | nes->profile.lxa_magic) & fetch(nes));
        assign(nes, &nes->cpu.a, nes->cpu.x);
        break;

    case 0xBB: { /* LAS abs,Y: S, A and X all take the value read AND S. */
        uint8_t r = (uint8_t)(read_at(nes, ea_absi(nes, nes->cpu.y, 0)) & nes->cpu.s);
        nes->cpu.s = r;
        nes->cpu.x = r;
        assign(nes, &nes->cpu.a, r);
        break;
    }

    /* The unstable stores, all through store_sh (D-15). */
    case 0x9C: /* SHY abs,X */
        store_sh_abs(nes, nes->cpu.y, nes->cpu.x);
        break;
    case 0x9E: /* SHX abs,Y */
        store_sh_abs(nes, nes->cpu.x, nes->cpu.y);
        break;
    case 0x9F: /* SHA abs,Y */
        store_sh_abs(nes, (uint8_t)(nes->cpu.a & nes->cpu.x), nes->cpu.y);
        break;
    case 0x9B: /* TAS abs,Y: S = A AND X first */
        nes->cpu.s = (uint8_t)(nes->cpu.a & nes->cpu.x);
        store_sh_abs(nes, nes->cpu.s, nes->cpu.y);
        break;
    case 0x93: { /* SHA (zp),Y: H is the pointer's high byte */
        uint8_t p = fetch(nes);
        uint8_t lo = nesturbator__bus_read(nes, p);
        uint8_t hi = nesturbator__bus_read(nes, (uint8_t)(p + 1u));
        store_sh(nes, (uint8_t)(nes->cpu.a & nes->cpu.x), lo, hi, nes->cpu.y);
        break;
    }

    case 0x02:
    case 0x12:
    case 0x22:
    case 0x32:
    case 0x42:
    case 0x52:
    case 0x62:
    case 0x72:
    case 0x92:
    case 0xB2:
    case 0xD2:
    case 0xF2:
        jam(nes);
        break;
    }
}
