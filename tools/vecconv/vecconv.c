/* vecconv: converts one file of the SingleStepTests 65x02 vectors
   (nes6502/v1/<xx>.json) to N65V v1 (tests/vectors/n65v.h), then reads the
   output back through the reader the tests use. Host tool only: nothing
   shipped links it and it is not installed (D-01). Integer-only.

     vecconv <in.json> <out.n65v> <opcode-hex> [--first N]

   The tokenizer knows only the vector schema (D-02): any JSON whitespace,
   keys matched by name (name, initial, final, cycles; pc, s, a, x, y, p,
   ram), unsigned decimal integers and the kinds "read" and "write". Each
   file is one JSON array of objects; `name` is dropped. Anything else, a
   value out of range, a list longer than 255, a missing or repeated key, or
   a first cycle that is not a read of the opcode at the initial pc, exits 1
   with "vecconv: <in>: offset <n>: <reason>".

   With --first N it converts the first N objects and ignores what follows,
   so a byte-range prefix of an upstream file converts. Without it the array
   must close and hold exactly 10000 objects, the count the upstream README
   gives. A usage error exits 2. */
#include "n65v.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FULL_COUNT 10000u
#define KEY_MAX 16u

static const char *in_path;

/* --- Output buffer ------------------------------------------------------ */

struct buf {
    uint8_t *data;
    size_t len;
    size_t cap;
};

static void put8(struct buf *b, uint32_t v)
{
    if (b->len == b->cap) {
        size_t cap = b->cap != 0u ? b->cap * 2u : 65536u;
        uint8_t *data = realloc(b->data, cap);
        if (data == NULL) {
            fprintf(stderr, "vecconv: out of memory\n");
            exit(1);
        }
        b->data = data;
        b->cap = cap;
    }
    b->data[b->len++] = (uint8_t)(v & 0xFFu);
}

static void put16(struct buf *b, uint32_t v)
{
    put8(b, v & 0xFFu);
    put8(b, (v >> 8) & 0xFFu);
}

static void put_state(struct buf *b, const struct n65v_state *s)
{
    uint32_t i;
    put16(b, s->pc);
    put8(b, s->s);
    put8(b, s->a);
    put8(b, s->x);
    put8(b, s->y);
    put8(b, s->p);
    put8(b, s->ram_count);
    for (i = 0; i < (uint32_t)s->ram_count; i++) {
        put16(b, s->ram_addr[i]);
        put8(b, s->ram_value[i]);
    }
}

static void put_test(struct buf *b, const struct n65v_test *t)
{
    uint32_t i;
    put16(b, t->index);
    put_state(b, &t->initial);
    put_state(b, &t->final);
    put8(b, t->cycle_count);
    for (i = 0; i < (uint32_t)t->cycle_count; i++) {
        put16(b, t->cycles[i].addr);
        put8(b, t->cycles[i].value);
        put8(b, t->cycles[i].kind);
    }
}

static void put_header(struct buf *b, uint8_t opcode)
{
    put8(b, 'N');
    put8(b, '6');
    put8(b, '5');
    put8(b, 'V');
    put8(b, N65V_VERSION);
    put8(b, opcode);
    put16(b, 0u); /* count, patched at the end */
}

/* --- Files ---------------------------------------------------------------- */

/* Reads the whole file; returns 0 if it cannot. */
static int read_file(const char *path, struct buf *b)
{
    uint8_t chunk[65536];
    size_t n;
    FILE *f = fopen(path, "rb");
    if (f == NULL)
        return 0;
    while ((n = fread(chunk, 1u, sizeof chunk, f)) > 0u) {
        size_t i;
        for (i = 0; i < n; i++)
            put8(b, chunk[i]);
    }
    if (ferror(f)) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

static int write_file(const char *path, const struct buf *b)
{
    FILE *f = fopen(path, "wb");
    int ok;
    if (f == NULL)
        return 0;
    ok = fwrite(b->data, 1u, b->len, f) == b->len;
    if (fclose(f) != 0)
        ok = 0;
    return ok;
}

/* --- Schema tokenizer ----------------------------------------------------- */

struct parser {
    const uint8_t *buf;
    size_t len;
    size_t pos;
};

static int fail(size_t off, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "vecconv: %s: offset %zu: ", in_path, off);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    return 0;
}

/* The next byte, or -1 at the end of the input. */
static int peek(const struct parser *p)
{
    return p->pos < p->len ? (int)p->buf[p->pos] : -1;
}

static void ws(struct parser *p)
{
    int c = peek(p);
    while (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        p->pos++;
        c = peek(p);
    }
}

static int expect(struct parser *p, int ch)
{
    int c;
    ws(p);
    c = peek(p);
    if (c < 0)
        return fail(p->pos, "input ends, expected '%c'", ch);
    if (c != ch)
        return fail(p->pos, "expected '%c'", ch);
    p->pos++;
    return 1;
}

/* After a list or object member: ',' continues (returns 1), the closing
   character ends (returns 2), anything else fails (returns 0). */
static int next_member(struct parser *p, int close)
{
    int c;
    ws(p);
    c = peek(p);
    if (c == ',') {
        p->pos++;
        return 1;
    }
    if (c == close) {
        p->pos++;
        return 2;
    }
    if (c < 0)
        return fail(p->pos, "input ends, expected ',' or '%c'", close);
    return fail(p->pos, "expected ',' or '%c'", close);
}

/* An unsigned decimal integer no greater than max. */
static int parse_uint(struct parser *p, uint32_t max, uint32_t *out)
{
    size_t start;
    uint32_t v = 0;
    int c;
    ws(p);
    start = p->pos;
    c = peek(p);
    if (c == '-' || c == '+')
        return fail(start, "signed number");
    if (c < '0' || c > '9')
        return fail(start, c < 0 ? "input ends, expected a number" : "expected a number");
    while (c >= '0' && c <= '9') {
        if (v <= 65536u) /* saturates; anything above 65536 is out of range */
            v = v * 10u + (uint32_t)(c - '0');
        p->pos++;
        c = peek(p);
    }
    if (c == '.')
        return fail(p->pos, "fractional number");
    if (c == 'e' || c == 'E')
        return fail(p->pos, "number with an exponent");
    if (v > max)
        return fail(start, "number above %lu", (unsigned long)max);
    *out = v;
    return 1;
}

/* A string without escapes. Up to KEY_MAX - 1 bytes are copied to out
   (when out is not NULL); *long_out is set if there were more. */
static int parse_string(struct parser *p, char *out, int *long_out)
{
    size_t n = 0;
    int c;
    if (!expect(p, '"'))
        return 0;
    for (;;) {
        c = peek(p);
        if (c < 0)
            return fail(p->pos, "input ends inside a string");
        if (c == '"')
            break;
        if (c == '\\')
            return fail(p->pos, "escape in a string");
        if (c < 0x20)
            return fail(p->pos, "control character in a string");
        if (out != NULL && n + 1u < KEY_MAX)
            out[n] = (char)c;
        else if (long_out != NULL)
            *long_out = 1;
        n++;
        p->pos++;
    }
    p->pos++;
    if (out != NULL)
        out[n < KEY_MAX ? n : KEY_MAX - 1u] = '\0';
    return 1;
}

/* A key and its colon. Returns the key's index in names, or -1 after
   reporting an unknown key, or -2 after another error. *seen tracks
   repeats. */
static int parse_key(struct parser *p, const char *const *names, int count, uint32_t *seen)
{
    char key[KEY_MAX];
    int too_long = 0;
    int i;
    size_t at;
    ws(p);
    at = p->pos;
    if (!parse_string(p, key, &too_long))
        return -2;
    for (i = 0; i < count && (too_long || strcmp(key, names[i]) != 0); i++)
        ;
    if (i == count) {
        fail(at, "unknown key \"%s%s\"", key, too_long ? "..." : "");
        return -1;
    }
    if (*seen & (1u << i)) {
        fail(at, "repeated key \"%s\"", key);
        return -1;
    }
    *seen |= 1u << i;
    if (!expect(p, ':'))
        return -2;
    return i;
}

static int missing_key(size_t at, const char *const *names, int count, uint32_t seen)
{
    int i;
    for (i = 0; i < count; i++) {
        if (!(seen & (1u << i)))
            return fail(at, "missing key \"%s\"", names[i]);
    }
    return 1;
}

/* '[' and, if the list is empty, ']'. Returns 1 for a non-empty list, 2 for
   an empty one, 0 on error. */
static int open_list(struct parser *p)
{
    if (!expect(p, '['))
        return 0;
    ws(p);
    if (peek(p) == ']') {
        p->pos++;
        return 2;
    }
    return 1;
}

static int parse_ram(struct parser *p, struct n65v_state *s)
{
    int r = open_list(p);
    s->ram_count = 0;
    while (r == 1) {
        uint32_t addr, value;
        ws(p);
        if (s->ram_count == 255u)
            return fail(p->pos, "ram list longer than 255");
        if (!expect(p, '[') || !parse_uint(p, 0xFFFFu, &addr) || !expect(p, ',') ||
            !parse_uint(p, 0xFFu, &value) || !expect(p, ']'))
            return 0;
        s->ram_addr[s->ram_count] = (uint16_t)addr;
        s->ram_value[s->ram_count] = (uint8_t)value;
        s->ram_count++;
        r = next_member(p, ']');
    }
    return r != 0;
}

static int parse_cycles(struct parser *p, struct n65v_test *t)
{
    int r = open_list(p);
    t->cycle_count = 0;
    while (r == 1) {
        uint32_t addr, value;
        char kind[KEY_MAX];
        int too_long = 0;
        size_t at;
        ws(p);
        if (t->cycle_count == 255u)
            return fail(p->pos, "cycle list longer than 255");
        if (!expect(p, '[') || !parse_uint(p, 0xFFFFu, &addr) || !expect(p, ',') ||
            !parse_uint(p, 0xFFu, &value) || !expect(p, ','))
            return 0;
        ws(p);
        at = p->pos;
        if (!parse_string(p, kind, &too_long))
            return 0;
        if (!too_long && strcmp(kind, "read") == 0)
            t->cycles[t->cycle_count].kind = N65V_KIND_READ;
        else if (!too_long && strcmp(kind, "write") == 0)
            t->cycles[t->cycle_count].kind = N65V_KIND_WRITE;
        else
            return fail(at, "unknown cycle kind");
        if (!expect(p, ']'))
            return 0;
        t->cycles[t->cycle_count].addr = (uint16_t)addr;
        t->cycles[t->cycle_count].value = (uint8_t)value;
        t->cycle_count++;
        r = next_member(p, ']');
    }
    return r != 0;
}

static const char *const STATE_KEYS[] = {"pc", "s", "a", "x", "y", "p", "ram"};
#define STATE_KEY_COUNT 7

static int parse_state(struct parser *p, struct n65v_state *s)
{
    uint32_t seen = 0, v = 0;
    size_t start;
    int r;
    ws(p);
    start = p->pos;
    if (!expect(p, '{'))
        return 0;
    ws(p);
    r = 1;
    if (peek(p) == '}') {
        p->pos++;
        r = 2;
    }
    while (r == 1) {
        int k = parse_key(p, STATE_KEYS, STATE_KEY_COUNT, &seen);
        int ok;
        if (k < 0)
            return 0;
        switch (k) {
        case 0:
            ok = parse_uint(p, 0xFFFFu, &v);
            s->pc = (uint16_t)v;
            break;
        case 6:
            ok = parse_ram(p, s);
            break;
        default:
            ok = parse_uint(p, 0xFFu, &v);
            if (k == 1)
                s->s = (uint8_t)v;
            else if (k == 2)
                s->a = (uint8_t)v;
            else if (k == 3)
                s->x = (uint8_t)v;
            else if (k == 4)
                s->y = (uint8_t)v;
            else
                s->p = (uint8_t)v;
            break;
        }
        if (!ok)
            return 0;
        r = next_member(p, '}');
    }
    return r != 0 && missing_key(start, STATE_KEYS, STATE_KEY_COUNT, seen);
}

static const char *const TEST_KEYS[] = {"name", "initial", "final", "cycles"};
#define TEST_KEY_COUNT 4

static int parse_test(struct parser *p, struct n65v_test *t, uint8_t opcode)
{
    uint32_t seen = 0;
    size_t start, cycles_at = 0;
    int r;
    ws(p);
    start = p->pos;
    if (!expect(p, '{'))
        return 0;
    ws(p);
    r = 1;
    if (peek(p) == '}') {
        p->pos++;
        r = 2;
    }
    while (r == 1) {
        int k = parse_key(p, TEST_KEYS, TEST_KEY_COUNT, &seen);
        int ok;
        if (k < 0)
            return 0;
        if (k == 0) {
            ok = parse_string(p, NULL, NULL);
        } else if (k == 1) {
            ok = parse_state(p, &t->initial);
        } else if (k == 2) {
            ok = parse_state(p, &t->final);
        } else {
            ws(p);
            cycles_at = p->pos;
            ok = parse_cycles(p, t);
        }
        if (!ok)
            return 0;
        r = next_member(p, '}');
    }
    if (r == 0 || !missing_key(start, TEST_KEYS, TEST_KEY_COUNT, seen))
        return 0;
    /* Every test starts by fetching its opcode from the initial pc. */
    if (t->cycle_count == 0u || t->cycles[0].kind != N65V_KIND_READ ||
        t->cycles[0].addr != t->initial.pc || t->cycles[0].value != opcode)
        return fail(cycles_at, "first cycle is not a read of 0x%02x at the initial pc",
                    (unsigned)opcode);
    return 1;
}

/* Parses the array into out (header included). *count gets the number of
   tests. first is 0 for a whole file. */
static int convert(struct parser *p, uint8_t opcode, uint32_t first, struct buf *out,
                   uint32_t *count)
{
    static struct n65v_test t;
    uint32_t n = 0;
    uint32_t limit = first != 0u ? first : FULL_COUNT;
    int r;

    put_header(out, opcode);
    r = open_list(p);
    while (r == 1) {
        ws(p);
        if (n == limit)
            return fail(p->pos, "more than %lu tests", (unsigned long)limit);
        memset(&t, 0, sizeof t);
        if (!parse_test(p, &t, opcode))
            return 0;
        t.index = (uint16_t)n;
        put_test(out, &t);
        n++;
        if (n == first)
            break; /* a prefix: whatever follows is not read */
        r = next_member(p, ']');
    }
    if (r == 0)
        return 0;
    if (n != limit)
        return fail(p->pos, "the array holds %lu tests, expected %lu", (unsigned long)n,
                    (unsigned long)limit);
    if (first == 0u) {
        ws(p);
        if (p->pos != p->len)
            return fail(p->pos, "data after the array");
    }
    out->data[6] = (uint8_t)(n & 0xFFu);
    out->data[7] = (uint8_t)((n >> 8) & 0xFFu);
    *count = n;
    return 1;
}

/* --- Re-read -------------------------------------------------------------- */

/* Reads path back through the shared reader, re-encodes every test and
   checks the result equals the file byte for byte. */
static int reread(const char *path, uint8_t opcode, uint32_t count)
{
    static struct n65v_test t;
    struct buf file = {NULL, 0, 0}, again = {NULL, 0, 0};
    struct n65v_reader r;
    uint32_t n = 0;
    uint8_t op = 0;
    int rc, ok = 1;

    if (!read_file(path, &file)) {
        fprintf(stderr, "vecconv: %s: cannot read back\n", path);
        free(file.data);
        return 0;
    }
    n65v_init(&r, file.data, file.len, opcode, 1u, count);
    put_header(&again, opcode);
    while ((rc = n65v_next(&r, &t, &op)) == 1) {
        if (op != opcode || (uint32_t)t.index != n) {
            fprintf(stderr, "vecconv: %s: re-read: test %lu has opcode 0x%02x, index %u\n", path,
                    (unsigned long)n, (unsigned)op, (unsigned)t.index);
            ok = 0;
            break;
        }
        put_test(&again, &t);
        n++;
    }
    if (ok && rc < 0) {
        fprintf(stderr, "vecconv: %s: re-read: %s\n", path, r.error);
        ok = 0;
    }
    if (ok) {
        again.data[6] = (uint8_t)(n & 0xFFu);
        again.data[7] = (uint8_t)((n >> 8) & 0xFFu);
        if (n != count || again.len != file.len || memcmp(again.data, file.data, file.len) != 0) {
            fprintf(stderr, "vecconv: %s: re-read: %lu tests do not match what was written\n", path,
                    (unsigned long)n);
            ok = 0;
        }
    }
    free(file.data);
    free(again.data);
    return ok;
}

/* --- Command line --------------------------------------------------------- */

static int hex_digit(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

static int usage(void)
{
    fprintf(stderr, "usage: vecconv <in.json> <out.n65v> <opcode-hex> [--first N]\n");
    return 2;
}

int main(int argc, char **argv)
{
    struct buf in = {NULL, 0, 0}, out = {NULL, 0, 0};
    struct parser p;
    uint32_t first = 0, count = 0;
    uint8_t opcode;
    int hi, lo, ok;

    if (argc != 4 && argc != 6)
        return usage();
    hi = strlen(argv[3]) == 2u ? hex_digit(argv[3][0]) : -1;
    lo = strlen(argv[3]) == 2u ? hex_digit(argv[3][1]) : -1;
    if (hi < 0 || lo < 0)
        return usage();
    opcode = (uint8_t)((hi << 4) | lo);
    if (argc == 6) {
        const char *s = argv[5];
        if (strcmp(argv[4], "--first") != 0 || *s == '\0' || strlen(s) > 5u)
            return usage();
        for (; *s != '\0'; s++) {
            if (*s < '0' || *s > '9')
                return usage();
            first = first * 10u + (uint32_t)(*s - '0');
        }
        if (first == 0u || first > FULL_COUNT)
            return usage();
    }

    in_path = argv[1];
    if (!read_file(in_path, &in)) {
        fprintf(stderr, "vecconv: %s: cannot read\n", in_path);
        return 1;
    }
    p.buf = in.data;
    p.len = in.len;
    p.pos = 0;
    ok = convert(&p, opcode, first, &out, &count);
    if (ok && !write_file(argv[2], &out)) {
        fprintf(stderr, "vecconv: %s: cannot write\n", argv[2]);
        ok = 0;
    }
    if (ok)
        ok = reread(argv[2], opcode, count);
    free(in.data);
    free(out.data);
    if (!ok)
        return 1;
    printf("vecconv: %02x: %lu tests written and re-read\n", (unsigned)opcode,
           (unsigned long)count);
    return 0;
}
