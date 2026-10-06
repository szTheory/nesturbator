/* The N65V v1 reader (D-05). Fields are decoded byte by byte, never copied
   into a struct, and each read first checks n > len - off, which cannot
   overflow because off never exceeds len. The file and hex-digit helpers at
   the end are shared by vecconv and the vector tests. */
#include "n65v.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void fail(struct n65v_reader *r, size_t off, const char *fmt, ...)
{
    char msg[120];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);
    snprintf(r->error, sizeof r->error, "offset %zu: %s", off, msg);
    r->failed = 1;
}

static int get_u8(struct n65v_reader *r, const char *what, const char *field, uint8_t *v)
{
    if (1u > r->len - r->off) {
        fail(r, r->off, "truncated %s%s", what, field);
        return 0;
    }
    *v = r->buf[r->off];
    r->off += 1u;
    return 1;
}

static int get_u16(struct n65v_reader *r, const char *what, const char *field, uint16_t *v)
{
    if (2u > r->len - r->off) {
        fail(r, r->off, "truncated %s%s", what, field);
        return 0;
    }
    *v = (uint16_t)((uint32_t)r->buf[r->off] | ((uint32_t)r->buf[r->off + 1u] << 8));
    r->off += 2u;
    return 1;
}

static int get_state(struct n65v_reader *r, const char *what, struct n65v_state *s)
{
    uint32_t i;
    if (!get_u16(r, what, " pc", &s->pc) || !get_u8(r, what, " s", &s->s) ||
        !get_u8(r, what, " a", &s->a) || !get_u8(r, what, " x", &s->x) ||
        !get_u8(r, what, " y", &s->y) || !get_u8(r, what, " p", &s->p) ||
        !get_u8(r, what, " ram count", &s->ram_count))
        return 0;
    for (i = 0; i < (uint32_t)s->ram_count; i++) {
        if (!get_u16(r, what, " ram address", &s->ram_addr[i]) ||
            !get_u8(r, what, " ram value", &s->ram_value[i]))
            return 0;
    }
    return 1;
}

/* Reads and checks a chunk header at r->off. */
static int get_header(struct n65v_reader *r)
{
    uint8_t version, opcode;
    uint16_t count;
    uint8_t expected = (uint8_t)((r->first_opcode + r->chunk) & 0xFFu);
    size_t at;

    if (4u > r->len - r->off) {
        fail(r, r->off, "truncated magic");
        return 0;
    }
    if (memcmp(r->buf + r->off, "N65V", 4u) != 0) {
        fail(r, r->off, "bad magic");
        return 0;
    }
    r->off += 4u;
    at = r->off;
    if (!get_u8(r, "version", "", &version))
        return 0;
    if (version != N65V_VERSION) {
        fail(r, at, "version %u, expected %u", (unsigned)version, N65V_VERSION);
        return 0;
    }
    at = r->off;
    if (!get_u8(r, "opcode", "", &opcode))
        return 0;
    if (opcode != expected) {
        fail(r, at, "opcode 0x%02x, expected 0x%02x", (unsigned)opcode, (unsigned)expected);
        return 0;
    }
    at = r->off;
    if (!get_u16(r, "count", "", &count))
        return 0;
    if ((uint32_t)count != r->tests_per_chunk) {
        fail(r, at, "count %u, expected %lu", (unsigned)count, (unsigned long)r->tests_per_chunk);
        return 0;
    }
    r->opcode = opcode;
    r->chunk++;
    r->left = count;
    r->next_index = 0;
    return 1;
}

void n65v_init(struct n65v_reader *r, const uint8_t *buf, size_t len, uint8_t first_opcode,
               uint32_t chunks, uint32_t tests_per_chunk)
{
    memset(r, 0, sizeof *r);
    r->buf = buf;
    r->len = buf != NULL ? len : 0u;
    r->first_opcode = first_opcode;
    r->chunks = chunks;
    r->tests_per_chunk = tests_per_chunk;
}

int n65v_next(struct n65v_reader *r, struct n65v_test *t, uint8_t *opcode)
{
    uint32_t i;
    size_t at;

    if (r->failed)
        return -1;
    while (r->left == 0u) {
        if (r->chunk == r->chunks) {
            if (r->off != r->len) {
                fail(r, r->off, "%zu trailing bytes", r->len - r->off);
                return -1;
            }
            return 0;
        }
        if (!get_header(r))
            return -1;
    }

    at = r->off;
    if (!get_u16(r, "index", "", &t->index))
        return -1;
    if ((uint32_t)t->index < r->next_index) {
        fail(r, at, "index %u after %lu", (unsigned)t->index, (unsigned long)(r->next_index - 1u));
        return -1;
    }
    r->next_index = (uint32_t)t->index + 1u;
    if (!get_state(r, "initial", &t->initial) || !get_state(r, "final", &t->final) ||
        !get_u8(r, "cycle count", "", &t->cycle_count))
        return -1;
    for (i = 0; i < (uint32_t)t->cycle_count; i++) {
        struct n65v_cycle *c = &t->cycles[i];
        if (!get_u16(r, "cycle", " address", &c->addr) || !get_u8(r, "cycle", " value", &c->value))
            return -1;
        at = r->off;
        if (!get_u8(r, "cycle", " kind", &c->kind))
            return -1;
        if (c->kind > N65V_KIND_WRITE) {
            fail(r, at, "kind %u", (unsigned)c->kind);
            return -1;
        }
    }
    r->left--;
    *opcode = r->opcode;
    return 1;
}

uint8_t *n65v_read_file(const char *path, size_t *len)
{
    size_t cap = 65536u;
    size_t n = 0u;
    uint8_t *buf;
    FILE *f = fopen(path, "rb");
    if (f == NULL)
        return NULL;
    buf = (uint8_t *)malloc(cap);
    while (buf != NULL) {
        uint8_t *bigger;
        n += fread(buf + n, 1u, cap - n, f);
        if (n < cap)
            break;
        bigger = (uint8_t *)realloc(buf, cap * 2u);
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

int n65v_hex_digit(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}
