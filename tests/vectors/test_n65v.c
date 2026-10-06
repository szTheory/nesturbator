/* The N65V reader (D-05, D-06), in two tests.

   vectors.n65v <65x02-sample.n65v>

   Reads the whole file through n65v_next as 256 chunks of 100 tests with the
   first chunk's opcode 0x00, and checks that it holds 25,600 tests, that
   chunk k has opcode k, that the indexes in each chunk are 0 to 99, and that
   the file ends exactly after the last chunk.

   vectors.n65v            (no argument: vectors.n65v.crafted)

   Encodes a valid one-chunk, two-test buffer byte by byte and requires the
   reader to return -1, with an error naming the byte offset at fault, for
   every truncation of it, bad magic, version 2, kind 2, an opcode mismatch, a
   header count of 3 with 2 tests present, two tests with index 0, and one
   trailing byte. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../check.h"
#include "n65v.h"

#define SAMPLE_CHUNKS 256u
#define SAMPLE_TESTS 100u

static struct n65v_test test;
static struct n65v_reader reader;

/* The committed sample: 256 chunks of 100 tests, opcodes 00 to ff in order. */
static void check_sample(const char *path)
{
    size_t len = 0u;
    uint8_t *buf = n65v_read_file(path, &len);
    if (buf == NULL) {
        fprintf(stderr, "vectors.n65v: %s: cannot read\n", path);
        check_failures++;
        return;
    }
    n65v_init(&reader, buf, len, 0u, SAMPLE_CHUNKS, SAMPLE_TESTS);
    uint32_t n = 0u;
    uint32_t bad = 0u;
    uint8_t opcode = 0u;
    int rc;
    while ((rc = n65v_next(&reader, &test, &opcode)) == 1) {
        uint32_t chunk = n / SAMPLE_TESTS;
        uint32_t index = n % SAMPLE_TESTS;
        if (((uint32_t)opcode != chunk || (uint32_t)test.index != index) && bad++ == 0u) {
            fprintf(stderr,
                    "vectors.n65v: test %lu: opcode 0x%02x index %u, expected 0x%02lx %lu\n",
                    (unsigned long)n, (unsigned)opcode, (unsigned)test.index, (unsigned long)chunk,
                    (unsigned long)index);
        }
        n++;
    }
    if (rc < 0) {
        fprintf(stderr, "vectors.n65v: %s: %s\n", path, reader.error);
    }
    CHECK_EQ_U64(rc, 0);
    CHECK_EQ_U64(bad, 0);
    CHECK_EQ_U64(n, SAMPLE_CHUNKS * SAMPLE_TESTS);
    CHECK_EQ_U64(reader.off, len);
    free(buf);
}

/* --- Crafted buffers --------------------------------------------------- */

/* The valid buffer: header (8 bytes), then two 29-byte tests of LDA
   immediate. Each test is index (2), initial and final (11 each: pc 2, s a x
   y p, ram count 1, one pair of 3) and one cycle (count 1, triple 3). */
#define CRAFTED_OPCODE 0xA9u
#define CRAFTED_LEN 66u
#define OFF_VERSION 4u
#define OFF_OPCODE 5u
#define OFF_COUNT 6u
#define OFF_KIND0 36u  /* kind byte of test 0's only cycle */
#define OFF_INDEX1 37u /* index of test 1 */

static uint8_t crafted[CRAFTED_LEN + 1u];
static size_t crafted_len;
/* field_start[i] is the offset of the field holding byte i: the reader
   reports a field cut short at the field's first byte. */
static size_t field_start[CRAFTED_LEN + 1u];

static void put8(uint32_t v)
{
    field_start[crafted_len] = crafted_len;
    crafted[crafted_len++] = (uint8_t)(v & 0xFFu);
}

static void put16(uint32_t v)
{
    put8(v);
    put8(v >> 8);
    field_start[crafted_len - 1u] = crafted_len - 2u;
}

static void put_state(uint32_t pc, uint32_t a)
{
    put16(pc);
    put8(0xFDu); /* s */
    put8(a);
    put8(0x00u); /* x */
    put8(0x00u); /* y */
    put8(0x24u); /* p */
    put8(1u);    /* ram count */
    put16(pc);
    put8(CRAFTED_OPCODE);
}

static void put_test(uint32_t index, uint32_t pc)
{
    put16(index);
    put_state(pc, 0x00u);
    put_state(pc + 2u, 0x11u);
    put8(1u); /* cycle count */
    put16(pc);
    put8(CRAFTED_OPCODE);
    put8(N65V_KIND_READ);
}

static void encode(void)
{
    crafted_len = 0u;
    put8('N');
    put8('6');
    put8('5');
    put8('V');
    for (size_t i = 0u; i < 4u; i++) {
        field_start[i] = 0u; /* the magic is one field */
    }
    put8(N65V_VERSION);
    put8(CRAFTED_OPCODE);
    put16(2u);
    put_test(0u, 0x0200u);
    put_test(1u, 0x0300u);
}

/* Reads buf[0..len) as one chunk of `tests` tests with opcode `opcode`.
   Returns the last n65v_next result; *count gets the tests decoded. */
static int read_all(size_t len, uint8_t opcode, uint32_t tests, uint32_t *count)
{
    uint8_t op = 0u;
    int rc;
    *count = 0u;
    n65v_init(&reader, crafted, len, opcode, 1u, tests);
    while ((rc = n65v_next(&reader, &test, &op)) == 1) {
        (*count)++;
    }
    return rc;
}

/* The read must fail, with an error at byte offset `at`. */
static void expect_error(const char *what, size_t len, uint8_t opcode, uint32_t tests, size_t at)
{
    char prefix[32];
    uint32_t count = 0u;
    int rc = read_all(len, opcode, tests, &count);
    snprintf(prefix, sizeof prefix, "offset %zu:", at);
    if (rc != -1 || strncmp(reader.error, prefix, strlen(prefix)) != 0) {
        fprintf(stderr, "%s: rc %d, error \"%s\", expected -1 and \"%s ...\"\n", what, rc,
                reader.error, prefix);
        check_failures++;
    }
    /* The error is sticky. */
    uint8_t op = 0u;
    CHECK(n65v_next(&reader, &test, &op) == -1);
}

static void check_crafted(void)
{
    uint32_t count = 0u;
    char what[48];

    encode();
    CHECK_EQ_U64(crafted_len, CRAFTED_LEN);
    /* The valid buffer reads as two tests and a clean end. */
    CHECK_EQ_U64(read_all(crafted_len, CRAFTED_OPCODE, 2u, &count), 0);
    CHECK_EQ_U64(count, 2u);
    CHECK_EQ_HEX(test.final.a, 0x11u);
    CHECK_EQ_U64(test.index, 1u);

    /* Every truncation fails at the start of the field it cuts. */
    for (size_t len = 0u; len < CRAFTED_LEN; len++) {
        snprintf(what, sizeof what, "truncated to %zu", len);
        expect_error(what, len, CRAFTED_OPCODE, 2u, field_start[len]);
    }

    encode();
    crafted[0] = 'X';
    expect_error("bad magic", crafted_len, CRAFTED_OPCODE, 2u, 0u);

    encode();
    crafted[OFF_VERSION] = 2u;
    expect_error("version 2", crafted_len, CRAFTED_OPCODE, 2u, OFF_VERSION);

    encode();
    crafted[OFF_KIND0] = 2u;
    expect_error("kind 2", crafted_len, CRAFTED_OPCODE, 2u, OFF_KIND0);

    encode();
    expect_error("opcode mismatch", crafted_len, (uint8_t)(CRAFTED_OPCODE + 1u), 2u, OFF_OPCODE);

    /* A header that promises 3 tests over 2 fails where the 3rd would start. */
    encode();
    crafted[OFF_COUNT] = 3u;
    expect_error("count 3, 2 tests", crafted_len, CRAFTED_OPCODE, 3u, CRAFTED_LEN);

    /* The same count the caller expects is still checked against the header. */
    encode();
    expect_error("short count", crafted_len, CRAFTED_OPCODE, 3u, OFF_COUNT);

    encode();
    crafted[OFF_INDEX1] = 0u;
    expect_error("index 0 twice", crafted_len, CRAFTED_OPCODE, 2u, OFF_INDEX1);

    encode();
    put8(0x00u);
    expect_error("one trailing byte", crafted_len, CRAFTED_OPCODE, 2u, CRAFTED_LEN);
}

int main(int argc, char **argv)
{
    if (argc == 1) {
        check_crafted();
    } else if (argc == 2) {
        check_sample(argv[1]);
    } else {
        fprintf(stderr, "usage: vectors.n65v [65x02-sample.n65v]\n");
        return 2;
    }
    CHECK_DONE();
}
