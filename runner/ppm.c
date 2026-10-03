#include "ppm.h"

#include <stddef.h>
#include <stdio.h>

#define WIDTH 256u
#define HEIGHT 240u

int nesturbator_run_write_ppm(const char *path, const uint32_t *xrgb, uint32_t pitch)
{
    static const char header[] = "P6\n256 240\n255\n";
    uint8_t row[WIDTH * 3u];
    int failed = 0;
    FILE *f = fopen(path, "wb");
    if (f == NULL) {
        return -1;
    }
    if (fwrite(header, 1, sizeof header - 1u, f) != sizeof header - 1u) {
        failed = 1;
    }
    for (uint32_t y = 0; y < HEIGHT && !failed; y++) {
        const uint32_t *src = xrgb + (size_t)y * pitch;
        for (uint32_t x = 0; x < WIDTH; x++) {
            row[3u * x] = (uint8_t)((src[x] >> 16) & 0xffu);
            row[3u * x + 1u] = (uint8_t)((src[x] >> 8) & 0xffu);
            row[3u * x + 2u] = (uint8_t)(src[x] & 0xffu);
        }
        if (fwrite(row, 1, sizeof row, f) != sizeof row) {
            failed = 1;
        }
    }
    if (fclose(f) != 0) {
        failed = 1;
    }
    return failed ? -1 : 0;
}
