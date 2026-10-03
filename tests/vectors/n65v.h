/* N65V v1, the binary form of the SingleStepTests 65x02 vectors (D-03), and
   its bounds-checked reader (D-05). Test code only: vecconv and the vector
   program use it; nothing in the core includes it.

   Every multi-byte field is little-endian. A file is one or more chunks:

     magic "N65V", version u8 (1), opcode u8, count u16, then count tests.

   Each test:

     index u16          0-based position in the upstream file, strictly
                        increasing within a chunk
     initial, final     pc u16, s a x y p u8, ram count u8, then that many
                        (address u16, value u8) pairs
     cycle count u8     then that many (address u16, value u8, kind u8)
                        triples, kind 0 for a read and 1 for a write

   The file ends exactly after the last chunk. There is no checksum in the
   file; the manifest's SHA-256 covers it. */
#ifndef NESTURBATOR_N65V_H
#define NESTURBATOR_N65V_H

#include <stddef.h>
#include <stdint.h>

#define N65V_VERSION 1u
#define N65V_KIND_READ 0u
#define N65V_KIND_WRITE 1u

/* A CPU state: registers and the RAM pairs in upstream order. */
struct n65v_state {
    uint16_t pc;
    uint8_t s, a, x, y, p;
    uint8_t ram_count;
    uint16_t ram_addr[255];
    uint8_t ram_value[255];
};

/* One bus cycle; kind is N65V_KIND_READ or N65V_KIND_WRITE. */
struct n65v_cycle {
    uint16_t addr;
    uint8_t value;
    uint8_t kind;
};

struct n65v_test {
    uint16_t index;
    struct n65v_state initial, final;
    uint8_t cycle_count;
    struct n65v_cycle cycles[255];
};

/* Reader state. Set up with n65v_init; the fields are private to n65v.c
   except error, which holds the message after n65v_next returns -1. */
struct n65v_reader {
    const uint8_t *buf;
    size_t len;
    size_t off;
    uint8_t first_opcode;
    uint8_t opcode;
    uint32_t chunks;
    uint32_t tests_per_chunk;
    uint32_t chunk;
    uint32_t left;
    uint32_t next_index;
    int failed;
    char error[160];
};

/* Reads buf[0..len). The file must hold exactly `chunks` chunks; chunk k has
   opcode (first_opcode + k) mod 256 and exactly tests_per_chunk tests. */
void n65v_init(struct n65v_reader *r, const uint8_t *buf, size_t len, uint8_t first_opcode,
               uint32_t chunks, uint32_t tests_per_chunk);

/* Decodes the next test into *t and its chunk's opcode into *opcode.
   Returns 1 for a test, 0 at a clean end and -1 on an error. The error is
   kept: every later call returns -1. r->error then starts "offset <n>:",
   n being the byte offset of the field at fault. The reader never reads at
   or past len. */
int n65v_next(struct n65v_reader *r, struct n65v_test *t, uint8_t *opcode);

#endif
