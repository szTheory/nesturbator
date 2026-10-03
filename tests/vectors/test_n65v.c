/* vectors.n65v: the N65V reader (D-05) on the committed sample.

   vectors.n65v <65x02-sample.n65v>

   Reads the whole file through n65v_next as 256 chunks of 100 tests with the
   first chunk's opcode 0x00, and checks that it holds 25,600 tests, that
   chunk k has opcode k, that the indexes in each chunk are 0 to 99, and that
   the file ends exactly after the last chunk. */
#include <stdio.h>
#include <stdlib.h>

#include "../check.h"
#include "n65v.h"

#define SAMPLE_CHUNKS 256u
#define SAMPLE_TESTS 100u

static struct n65v_test test;
static struct n65v_reader reader;

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

/* The committed sample: 256 chunks of 100 tests, opcodes 00 to ff in order. */
static void check_sample(const char *path)
{
    size_t len = 0u;
    uint8_t *buf = read_file(path, &len);
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

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: vectors.n65v <65x02-sample.n65v>\n");
        return 2;
    }
    check_sample(argv[1]);
    CHECK_DONE();
}
