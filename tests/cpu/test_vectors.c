/* cpu.vectors: runs N65V tests through the CPU object and compares each
   test's final registers, RAM and every bus cycle (D-08, D-11).

   cpu.vectors <file.n65v> <opcode-hex> <chunks> <tests-per-chunk>

   With chunks 256 the file is a sample whose chunks are opcodes 00 to ff and
   only chunk <opcode> runs; with chunks 1 the file is that opcode's chunk
   alone. Every test in the file is read, so the whole file is validated.
   Standard output carries one line, "65x02/<xx>: <failed> of <run> vectors
   failed"; the first mismatch of each failing test and any reader error go
   to standard error. Exit status: 0 when nothing failed, 1 on a failed test
   or a bad file, 2 on a usage error. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "n65v.h"
#include "vector_bus.h"

static struct vector_machine machine;
static struct n65v_test test;
static struct n65v_reader reader;

static int usage(void)
{
    fprintf(stderr, "usage: cpu.vectors <file.n65v> <opcode-hex> <1|256> <tests-per-chunk>\n");
    return 2;
}

static int hex_digit(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

/* Parses decimal 1 to max; returns 0 on anything else. */
static uint32_t parse_count(const char *s, uint32_t max)
{
    uint32_t v = 0u;
    if (*s == '\0') {
        return 0u;
    }
    for (; *s != '\0'; s++) {
        if (*s < '0' || *s > '9') {
            return 0u;
        }
        v = v * 10u + (uint32_t)(*s - '0');
        if (v > max) {
            return 0u;
        }
    }
    return v;
}

/* Reads the whole file into a malloc'd buffer; NULL on failure. */
static uint8_t *read_file(const char *path, size_t *len)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        return NULL;
    }
    size_t cap = 65536u;
    size_t n = 0u;
    uint8_t *buf = (uint8_t *)malloc(cap);
    while (buf != NULL) {
        n += fread(buf + n, 1u, cap - n, f);
        if (n < cap) {
            break;
        }
        uint8_t *bigger = (uint8_t *)realloc(buf, cap * 2u);
        if (bigger == NULL) {
            free(buf);
            buf = NULL;
            break;
        }
        buf = bigger;
        cap *= 2u;
    }
    if (buf != NULL && ferror(f)) {
        free(buf);
        buf = NULL;
    }
    fclose(f);
    *len = n;
    return buf;
}

static void mismatch(uint8_t opcode, const char *field, unsigned expected, unsigned got)
{
    fprintf(stderr, "%02x.json[%u]: %s expected 0x%02x got 0x%02x\n", (unsigned)opcode,
            (unsigned)test.index, field, expected, got);
}

/* Runs the current test; returns 1 if it matched. Reports only the first
   mismatch. */
static int run_test(uint8_t opcode)
{
    const struct n65v_state *in = &test.initial;
    const struct n65v_state *out = &test.final;
    struct nesturbator__cpu *cpu = &machine.nes.cpu;
    char field[48];

    /* The vectors were recorded with ANE and LXA constants of 0xEE (D-14);
       the harness sets them itself rather than relying on create. */
    machine.nes.profile.ane_magic = 0xEEu;
    machine.nes.profile.lxa_magic = 0xEEu;
    memset(cpu, 0, sizeof *cpu);
    cpu->pc = in->pc;
    cpu->s = in->s;
    cpu->a = in->a;
    cpu->x = in->x;
    cpu->y = in->y;
    cpu->p = in->p;
    for (uint32_t i = 0u; i < in->ram_count; i++) {
        machine.ram[in->ram_addr[i]] = in->ram_value[i];
    }
    vector_machine_reset_log(&machine);

    nesturbator__cpu_step(&machine.nes);

    if (cpu->pc != out->pc) {
        mismatch(opcode, "pc", out->pc, cpu->pc);
        return 0;
    }
    const struct {
        const char *name;
        uint8_t expected, got;
    } regs[] = {{"s", out->s, cpu->s},
                {"a", out->a, cpu->a},
                {"x", out->x, cpu->x},
                {"y", out->y, cpu->y},
                {"p", out->p, cpu->p}};
    for (size_t i = 0u; i < sizeof regs / sizeof regs[0]; i++) {
        if (regs[i].expected != regs[i].got) {
            mismatch(opcode, regs[i].name, regs[i].expected, regs[i].got);
            return 0;
        }
    }
    for (uint32_t i = 0u; i < out->ram_count; i++) {
        uint8_t got = machine.ram[out->ram_addr[i]];
        if (got != out->ram_value[i]) {
            snprintf(field, sizeof field, "ram[0x%04x]", (unsigned)out->ram_addr[i]);
            mismatch(opcode, field, out->ram_value[i], got);
            return 0;
        }
    }
    if (machine.log_overflow) {
        fprintf(stderr, "%02x.json[%u]: more than %u cycles\n", (unsigned)opcode,
                (unsigned)test.index, VECTOR_LOG_CAPACITY);
        return 0;
    }
    if (machine.log_count != test.cycle_count) {
        mismatch(opcode, "cycle count", test.cycle_count, (unsigned)machine.log_count);
        return 0;
    }
    for (uint32_t i = 0u; i < test.cycle_count; i++) {
        const struct n65v_cycle *e = &test.cycles[i];
        const struct n65v_cycle *g = &machine.log[i];
        if (e->addr != g->addr) {
            snprintf(field, sizeof field, "cycle %u addr", (unsigned)i);
            mismatch(opcode, field, e->addr, g->addr);
            return 0;
        }
        if (e->value != g->value) {
            snprintf(field, sizeof field, "cycle %u value", (unsigned)i);
            mismatch(opcode, field, e->value, g->value);
            return 0;
        }
        if (e->kind != g->kind) {
            snprintf(field, sizeof field, "cycle %u kind", (unsigned)i);
            mismatch(opcode, field, e->kind, g->kind);
            return 0;
        }
    }
    return 1;
}

/* Zeroes RAM after a test, so the next test starts from zeroed RAM. After a
   pass the cycle compare shows that only the named addresses can have been
   written, so only those are zeroed. A failed test may have written any
   address, so all of RAM is zeroed. */
static void clear_ram(int passed)
{
    if (!passed) {
        memset(machine.ram, 0, sizeof machine.ram);
        return;
    }
    for (uint32_t i = 0u; i < test.initial.ram_count; i++) {
        machine.ram[test.initial.ram_addr[i]] = 0u;
    }
    for (uint32_t i = 0u; i < test.final.ram_count; i++) {
        machine.ram[test.final.ram_addr[i]] = 0u;
    }
}

int main(int argc, char **argv)
{
    if (argc != 5) {
        return usage();
    }
    int hi = hex_digit(argv[2][0]);
    int lo = hi < 0 ? -1 : hex_digit(argv[2][1]);
    if (lo < 0 || argv[2][2] != '\0') {
        return usage();
    }
    uint8_t want = (uint8_t)(hi * 16 + lo);
    uint32_t chunks = parse_count(argv[3], 256u);
    uint32_t per_chunk = parse_count(argv[4], 10000u);
    if ((chunks != 1u && chunks != 256u) || per_chunk == 0u) {
        return usage();
    }

    size_t len = 0u;
    uint8_t *buf = read_file(argv[1], &len);
    if (buf == NULL) {
        fprintf(stderr, "cpu.vectors: %s: cannot read\n", argv[1]);
        return 1;
    }

    n65v_init(&reader, buf, len, chunks == 1u ? want : 0u, chunks, per_chunk);
    uint32_t run = 0u;
    uint32_t failed = 0u;
    uint8_t opcode = 0u;
    int rc;
    while ((rc = n65v_next(&reader, &test, &opcode)) == 1) {
        if (opcode != want) {
            continue;
        }
        run++;
        int passed = run_test(opcode);
        if (!passed) {
            failed++;
        }
        clear_ram(passed);
    }
    free(buf);
    if (rc < 0) {
        fprintf(stderr, "cpu.vectors: %s: %s\n", argv[1], reader.error);
    }
    printf("65x02/%02x: %u of %u vectors failed\n", (unsigned)want, (unsigned)failed,
           (unsigned)run);
    return (failed != 0u || rc < 0) ? 1 : 0;
}
