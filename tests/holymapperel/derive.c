/* Writes a patched copy of a Holy Mapperel ROM's 16-byte iNES header.
 *
 *   holymapperel-derive BASE OUT PATCH...
 *
 * PATCH is set@<offset>=<hh> (replace the byte) or or@<offset>=<hh> (OR into
 * it); the offset is decimal, 0 to 15, the hex is two digits. The output has
 * the length of BASE. Header layout: NESdev Wiki "INES" (flags 6 and 10).
 * Exit 2 on a usage error, a base shorter than 16 bytes or a malformed patch.
 * The derived file is built at build time and never committed. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEADER 16u

static int parse_patch(const char *text, int *is_or, unsigned *offset, unsigned *value)
{
    char *end;
    unsigned long v;
    if (strncmp(text, "set@", 4) == 0) {
        *is_or = 0;
    } else if (strncmp(text, "or@", 3) == 0) {
        *is_or = 1;
    } else {
        return 0;
    }
    text += *is_or ? 3 : 4;
    v = strtoul(text, &end, 10);
    if (end == text || *end != '=' || v >= HEADER) {
        return 0;
    }
    *offset = (unsigned)v;
    text = end + 1;
    if (strlen(text) != 2u || strspn(text, "0123456789abcdefABCDEF") != 2u) {
        return 0;
    }
    *value = (unsigned)strtoul(text, NULL, 16);
    return 1;
}

int main(int argc, char **argv)
{
    static uint8_t image[1u << 20];
    FILE *f;
    size_t len;
    int i;
    if (argc < 4) {
        fprintf(stderr, "usage: holymapperel-derive BASE OUT PATCH...\n");
        return 2;
    }
    f = fopen(argv[1], "rb");
    if (f == NULL) {
        fprintf(stderr, "holymapperel-derive: cannot read %s\n", argv[1]);
        return 2;
    }
    len = fread(image, 1, sizeof image, f);
    fclose(f);
    if (len < HEADER || len == sizeof image) {
        fprintf(stderr, "holymapperel-derive: %s has an unusable size\n", argv[1]);
        return 2;
    }
    for (i = 3; i < argc; i++) {
        int is_or;
        unsigned offset;
        unsigned value;
        if (!parse_patch(argv[i], &is_or, &offset, &value)) {
            fprintf(stderr, "holymapperel-derive: bad patch %s\n", argv[i]);
            return 2;
        }
        image[offset] = (uint8_t)(is_or ? (image[offset] | value) : value);
    }
    f = fopen(argv[2], "wb");
    if (f == NULL || fwrite(image, 1, len, f) != len || fclose(f) != 0) {
        fprintf(stderr, "holymapperel-derive: cannot write %s\n", argv[2]);
        return 2;
    }
    return 0;
}
