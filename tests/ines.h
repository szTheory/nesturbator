/* Synthetic iNES image builder for tests. Header-only: static inline functions, no link unit.
 *
 * Header layout: NESdev Wiki "INES" (flags 6 and 7) and "NES 2.0" (byte 8 carries the submapper in
 * its high nibble and mapper bits 8-11 in its low nibble). What the loader accepts today is
 * decided by validate_image's per-board profile in src/cartridge.c; this builder encodes any mapper
 * and NES 2.0 CHR-RAM, and tests choose what to feed the loader.
 *
 * Every byte is authored here; no commercial or third-party data is embedded.
 */
#ifndef NESTURBATOR_TESTS_INES_H
#define NESTURBATOR_TESTS_INES_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define INES_TRAINER_SIZE 512u
#define INES_PRG_BANK 16384u
#define INES_CHR_BANK 8192u

struct ines_spec {
    uint8_t prg_16k;
    uint8_t chr_8k; /* 0 means CHR RAM */
    uint8_t mirroring_vertical;
    uint8_t nes2;
    uint16_t mapper;
    uint8_t submapper;
    const uint8_t *trainer;  /* INES_TRAINER_SIZE bytes, or NULL */
    const uint8_t *prg_code; /* copied at PRG offset 0 (CPU $8000) */
    size_t prg_code_len;
    const uint8_t *chr_data;
    size_t chr_data_len;
    uint16_t nmi_vector;
    uint16_t reset_vector;
    uint16_t irq_vector;
};

static inline size_t ines_size(const struct ines_spec *s)
{
    return 16u + (s->trainer != NULL ? INES_TRAINER_SIZE : 0u) +
           (size_t)INES_PRG_BANK * s->prg_16k + (size_t)INES_CHR_BANK * s->chr_8k;
}

/* Returns the image size, or 0 when the spec does not fit or cap is too small. */
static inline size_t ines_build(uint8_t *out, size_t cap, const struct ines_spec *s)
{
    const size_t prg_size = (size_t)INES_PRG_BANK * s->prg_16k;
    const size_t chr_size = (size_t)INES_CHR_BANK * s->chr_8k;
    const size_t total = ines_size(s);
    size_t pos = 16u;
    uint8_t *prg;

    if (s->prg_16k == 0u || s->prg_code_len > prg_size || s->chr_data_len > chr_size ||
        cap < total) {
        return 0u;
    }
    memset(out, 0, total);
    out[0] = 'N';
    out[1] = 'E';
    out[2] = 'S';
    out[3] = 0x1au;
    out[4] = s->prg_16k;
    out[5] = s->chr_8k;
    out[6] = (uint8_t)(((s->mapper & 0x0fu) << 4) | (s->trainer != NULL ? 0x04u : 0x00u) |
                       (s->mirroring_vertical != 0u ? 0x01u : 0x00u));
    out[7] = (uint8_t)((s->mapper & 0xf0u) | (s->nes2 != 0u ? 0x08u : 0x00u));
    if (s->nes2 != 0u) {
        out[8] = (uint8_t)(((unsigned)s->submapper << 4) | ((s->mapper >> 8) & 0x0fu));
        if (s->chr_8k == 0u) {
            out[11] = 0x07u; /* 64 << 7 = 8 KiB of CHR-RAM */
        }
    }
    if (s->trainer != NULL) {
        memcpy(out + pos, s->trainer, INES_TRAINER_SIZE);
        pos += INES_TRAINER_SIZE;
    }
    prg = out + pos;
    if (s->prg_code_len != 0u) {
        memcpy(prg, s->prg_code, s->prg_code_len);
    }
    prg[prg_size - 6u] = (uint8_t)(s->nmi_vector & 0xffu);
    prg[prg_size - 5u] = (uint8_t)(s->nmi_vector >> 8);
    prg[prg_size - 4u] = (uint8_t)(s->reset_vector & 0xffu);
    prg[prg_size - 3u] = (uint8_t)(s->reset_vector >> 8);
    prg[prg_size - 2u] = (uint8_t)(s->irq_vector & 0xffu);
    prg[prg_size - 1u] = (uint8_t)(s->irq_vector >> 8);
    pos += prg_size;
    if (s->chr_data_len != 0u) {
        memcpy(out + pos, s->chr_data, s->chr_data_len);
    }
    return total;
}

#endif
