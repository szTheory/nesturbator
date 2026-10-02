/* host.convert: nesturbator_host_convert indexes the palette by the low 9
   bits of each native pixel and honours both pitches (D-15). */
#include <stdint.h>

#include "../check.h"
#include "convert.h"

#define PITCH 300u
#define OUT_PITCH 260u
#define SENTINEL 0xDEADBEEFu

static uint32_t palette[512];
static uint16_t video[PITCH * 240u];
static uint32_t out[OUT_PITCH * 240u];

int main(void)
{
    /* Every entry distinct and tied to its index. */
    for (uint32_t i = 0; i < 512u; i++) {
        palette[i] = 0x00010000u * i + i;
    }
    for (uint32_t i = 0; i < PITCH * 240u; i++) {
        video[i] = 0x0005u;
    }
    for (uint32_t i = 0; i < OUT_PITCH * 240u; i++) {
        out[i] = SENTINEL;
    }

    /* Row 0: the masking cases. 0xFE3F has bits above 8 set and maps to 0x03F. */
    video[0] = 0x000u;
    video[1] = 0x1FFu;
    video[2] = 0x040u;
    video[3] = 0xFE3Fu;
    /* Row 239 starts at video[239 * PITCH]. */
    video[239u * PITCH] = 0x123u;
    video[239u * PITCH + 255u] = 0x0ABu;
    /* Past column 255 of a source row: never read into the output. */
    video[256] = 0x1FFu;

    nesturbator_host_convert(palette, video, PITCH, out, OUT_PITCH);

    CHECK_EQ_HEX(out[0], palette[0x000]);
    CHECK_EQ_HEX(out[1], palette[0x1FF]);
    CHECK_EQ_HEX(out[2], palette[0x040]);
    CHECK_EQ_HEX(out[3], palette[0x03F]);
    CHECK_EQ_HEX(out[4], palette[0x005]);
    CHECK_EQ_HEX(out[255], palette[0x005]);

    CHECK_EQ_HEX(out[239u * OUT_PITCH], palette[0x123]);
    CHECK_EQ_HEX(out[239u * OUT_PITCH + 255u], palette[0x0AB]);
    CHECK_EQ_HEX(out[1u * OUT_PITCH], palette[0x005]);

    /* Columns 256-259 of every output row keep the sentinel. */
    int untouched = 1;
    for (uint32_t y = 0; y < 240u; y++) {
        for (uint32_t x = 256u; x < OUT_PITCH; x++) {
            if (out[y * OUT_PITCH + x] != SENTINEL) {
                untouched = 0;
            }
        }
    }
    CHECK(untouched);

    CHECK_DONE();
}
