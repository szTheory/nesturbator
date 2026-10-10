/* holymapperel-decode FILE.ppm: reads the Holy Mapperel result screen from the
   P6 image nesturbator-run --dump-frame writes. Exit 0: the code is 0000.
   Exit 1: a non-zero code, printed with the meaning of each digit. Exit 2:
   there is no result screen, or the input is not a 256x240 P6 image. Only the
   exit-0 path prints "code 0000 (".
   holymapperel-decode --prg-ram FILE.ppm: reads the PRG RAM text row (y 48,
   from x 16) and prints holymapperel: prg ram "<text>"; exit 0, or 2 when a
   cell is unreadable or the input is not a 256x240 P6 image. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hm_decode.h"

static int fail_read(void)
{
    printf("holymapperel: not a 256x240 P6 image\n");
    return 2;
}

int main(int argc, char **argv)
{
    FILE *f;
    uint8_t *buf;
    size_t cap = 1u << 20;
    size_t size;
    const uint8_t *rgb = NULL;
    uint16_t code = 0;
    int digit[4];
    int status;
    int k;
    int prg_ram = 0;
    const char *path;

    if (argc == 3 && strcmp(argv[1], "--prg-ram") == 0) {
        prg_ram = 1;
        path = argv[2];
    } else if (argc == 2) {
        path = argv[1];
    } else {
        fprintf(stderr, "usage: holymapperel-decode [--prg-ram] FILE.ppm\n");
        return 2;
    }
    f = fopen(path, "rb");
    if (f == NULL) {
        fprintf(stderr, "holymapperel-decode: cannot open %s\n", path);
        return 2;
    }
    buf = (uint8_t *)malloc(cap);
    if (buf == NULL) {
        fclose(f);
        return 2;
    }
    /* A valid file is 184,335 bytes; a longer one fills the buffer and is rejected. */
    size = fread(buf, 1, cap, f);
    fclose(f);
    if (!hm_read_ppm(buf, size, &rgb)) {
        free(buf);
        return fail_read();
    }
    if (prg_ram) {
        char text[HM_TEXT_CELLS + 1];
        int ok = hm_read_text_row(rgb, HM_PRG_RAM_Y, text, sizeof text);
        free(buf);
        printf("holymapperel: prg ram \"%s\"\n", text);
        return ok ? 0 : 2;
    }
    status = hm_decode_digits(rgb, &code, digit);
    free(buf);
    if (status == 2) {
        printf("holymapperel: no result screen (");
        if (digit[0] == -1 && digit[1] == -1 && digit[2] == -1 && digit[3] == -1) {
            printf("anchor missing or no digit readable)\n");
        } else {
            printf("digits ");
            for (k = 0; k < 4; k++) {
                if (digit[k] < 0) {
                    putchar('?');
                } else {
                    putchar("0123456789ABCDEF"[digit[k]]);
                }
            }
            printf(")\n");
        }
        return 2;
    }
    if (status == 0) {
        printf("holymapperel: code 0000 (WRAM 0, PRG 0, IRQ 0, CHR 0)\n");
        return 0;
    }
    if (code == 0xC0DE) {
        printf("holymapperel: code C0DE (the driver never finished)\n");
        return 1;
    }
    printf("holymapperel: code %04X (WRAM %X, PRG window bitmask %X, IRQ %X, CHR %X)\n",
           (unsigned)code, (unsigned)digit[0], (unsigned)digit[1], (unsigned)digit[2],
           (unsigned)digit[3]);
    return 1;
}
