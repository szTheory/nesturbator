/* Makes a corrupt copy of a save state for the runner.state.* cases.
 *
 *   runner.state_mutate <in> <out> <mode>
 *
 * truncate  drop the last byte
 * flip      XOR 0xFF into the last byte (a body byte, so the CRC no longer matches)
 * version   format version + 1 (offset 8), CRC recomputed
 * invalid   reserved byte at offset 48 set to 1, CRC recomputed
 *
 * CMake cannot compute a CRC-32, so this tool recomputes the state CRC (header
 * offset 28, over every byte except those four) the way the library does. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"

#define CRC_OFFSET 28u

static void recompute_crc(uint8_t *b, size_t n)
{
    uint32_t crc = nesturbator__crc32(0u, b, CRC_OFFSET);
    crc = nesturbator__crc32(crc, b + CRC_OFFSET + 4u, n - (CRC_OFFSET + 4u));
    for (unsigned k = 0; k < 4u; k++)
        b[CRC_OFFSET + k] = (uint8_t)(crc >> (8u * k));
}

int main(int argc, char **argv)
{
    static uint8_t buf[1u << 20];
    FILE *f;
    size_t n;
    if (argc != 4) {
        fprintf(stderr, "usage: runner.state_mutate <in> <out> <mode>\n");
        return 1;
    }
    f = fopen(argv[1], "rb");
    if (f == NULL) {
        fprintf(stderr, "runner.state_mutate: cannot read %s\n", argv[1]);
        return 1;
    }
    n = fread(buf, 1, sizeof buf, f);
    fclose(f);
    if (n < 64u || n == sizeof buf) {
        fprintf(stderr, "runner.state_mutate: %s is not a usable state\n", argv[1]);
        return 1;
    }
    if (strcmp(argv[3], "truncate") == 0) {
        n--;
    } else if (strcmp(argv[3], "flip") == 0) {
        buf[n - 1u] ^= 0xffu;
    } else if (strcmp(argv[3], "version") == 0) {
        buf[8]++;
        recompute_crc(buf, n);
    } else if (strcmp(argv[3], "invalid") == 0) {
        buf[48] = 1u;
        recompute_crc(buf, n);
    } else {
        fprintf(stderr, "runner.state_mutate: unknown mode %s\n", argv[3]);
        return 1;
    }
    f = fopen(argv[2], "wb");
    if (f == NULL || fwrite(buf, 1, n, f) != n || fclose(f) != 0) {
        fprintf(stderr, "runner.state_mutate: cannot write %s\n", argv[2]);
        return 1;
    }
    return 0;
}
