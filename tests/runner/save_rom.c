/* Writes the synthetic cartridges and save files the runner.save.* cases use.
 *
 *   runner.save_rom DIR
 *
 * battery.nes   MMC1, 8 KiB battery RAM; stores "SAVE" at $6000 and copies $6100 to $6004
 * plain.nes     the same code with 8 KiB work RAM and no battery
 * jam.nes       the battery shape; stores 'J' at $6000, then executes the JAM opcode $02
 * seed.sav      8192 bytes, zero except $42 at offset 0x100
 * short.sav     100 bytes of $11
 *
 * Every byte is authored here (CMake strings cannot hold NUL bytes). */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../ines.h"

#define SPAN 8192u

static int write_file(const char *dir, const char *name, const uint8_t *bytes, size_t len)
{
    char path[1024];
    FILE *f;
    int ok;
    if (snprintf(path, sizeof path, "%s/%s", dir, name) >= (int)sizeof path) {
        return 0;
    }
    f = fopen(path, "wb");
    if (f == NULL) {
        return 0;
    }
    ok = fwrite(bytes, 1, len, f) == len;
    return fclose(f) == 0 && ok;
}

static int write_rom(const char *dir, const char *name, int battery, const uint8_t *code,
                     size_t code_len)
{
    static uint8_t image[16u + 2u * INES_PRG_BANK];
    struct ines_spec spec;
    size_t len;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 2u;
    spec.nes2 = 1u;
    spec.mapper = 1u;
    spec.battery = (uint8_t)battery;
    if (battery != 0) {
        spec.prg_nvram_shift = 7u; /* 64 << 7 = 8 KiB */
    } else {
        spec.prg_ram_shift = 7u;
    }
    spec.prg_code = code;
    spec.prg_code_len = code_len;
    spec.nmi_vector = 0x8000u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    len = ines_build(image, sizeof image, &spec);
    return len != 0u && write_file(dir, name, image, len);
}

int main(int argc, char **argv)
{
    static const uint8_t save_code[] = {
        0xA9, 'S',  0x8D, 0x00, 0x60, /* LDA #'S'; STA $6000 */
        0xA9, 'A',  0x8D, 0x01, 0x60, /* LDA #'A'; STA $6001 */
        0xA9, 'V',  0x8D, 0x02, 0x60, /* LDA #'V'; STA $6002 */
        0xA9, 'E',  0x8D, 0x03, 0x60, /* LDA #'E'; STA $6003 */
        0xAD, 0x00, 0x61,             /* LDA $6100 */
        0x8D, 0x04, 0x60,             /* STA $6004 */
        0x4C, 0x1C, 0x80,             /* JMP * (the JMP is at $801C) */
    };
    static const uint8_t jam_code[] = {
        0xA9, 'J', 0x8D, 0x00, 0x60, /* LDA #'J'; STA $6000 */
        0x02,                        /* JAM */
    };
    static uint8_t seed[SPAN];
    static uint8_t shorter[100];
    if (argc != 2) {
        fprintf(stderr, "usage: runner.save_rom DIR\n");
        return 2;
    }
    memset(shorter, 0x11, sizeof shorter);
    seed[0x100] = 0x42u;
    if (!write_rom(argv[1], "battery.nes", 1, save_code, sizeof save_code) ||
        !write_rom(argv[1], "plain.nes", 0, save_code, sizeof save_code) ||
        !write_rom(argv[1], "jam.nes", 1, jam_code, sizeof jam_code) ||
        !write_file(argv[1], "seed.sav", seed, sizeof seed) ||
        !write_file(argv[1], "short.sav", shorter, sizeof shorter)) {
        fprintf(stderr, "runner.save_rom: cannot write into %s\n", argv[1]);
        return 1;
    }
    return 0;
}
