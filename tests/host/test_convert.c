/* host.convert: nesturbator_host_convert indexes the palette by the low 9
   bits of each native pixel and honours both pitches (D-15). */
#include <stdint.h>
#include <string.h>

#include "../check.h"
#include "../ines.h"
#include "convert.h"
#include "nesturbator.h"

#define PITCH 300u
#define OUT_PITCH 260u
#define WIDTH 256u
#define HEIGHT 240u
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

    /* Repeat the pitched conversion with the table returned for a real PAL image. */
    {
        uint8_t image[16u + INES_PRG_BANK + INES_CHR_BANK];
        struct ines_spec spec;
        nesturbator_config cfg;
        nesturbator *inst = NULL;
        uint32_t pal[512];
        memset(&spec, 0, sizeof spec);
        spec.prg_16k = 1u;
        spec.chr_8k = 1u;
        spec.nes2 = 1u;
        spec.reset_vector = 0x8000u;
        const size_t image_size = ines_build(image, sizeof image, &spec);
        CHECK_EQ_U64(image_size, sizeof image);
        if (image_size == sizeof image) {
            image[12] = 1u;
            memset(&cfg, 0, sizeof cfg);
            cfg.size = (uint32_t)sizeof cfg;
            cfg.abi = NESTURBATOR_ABI_VERSION;
            CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
            CHECK(inst != NULL);
            if (inst != NULL) {
                CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, image_size), NESTURBATOR_OK);
                CHECK_EQ_U64(nesturbator_get_region(inst), NESTURBATOR_REGION_PAL);
                nesturbator_get_palette(inst, pal, 512u);
                CHECK_EQ_HEX(pal[0x056], 0x8E1914u);
                CHECK_EQ_HEX(pal[0x116], 0x75172Eu);

                for (uint32_t y = 0u; y < HEIGHT; ++y) {
                    for (uint32_t x = 0u; x < PITCH; ++x) {
                        video[y * PITCH + x] = (uint16_t)(0xfe00u | ((x * 13u + y * 7u) & 0x01ffu));
                    }
                }
                video[0] = 0xfe56u;
                video[1] = 0xfe16u;
                video[2] = 0xff16u;
                for (uint32_t i = 0u; i < OUT_PITCH * HEIGHT; ++i)
                    out[i] = SENTINEL;
                nesturbator_host_convert(pal, video, PITCH, out, OUT_PITCH);
                for (uint32_t y = 0u; y < HEIGHT; ++y) {
                    for (uint32_t x = 0u; x < WIDTH; ++x)
                        CHECK_EQ_HEX(out[y * OUT_PITCH + x], pal[video[y * PITCH + x] & 0x01ffu]);
                    for (uint32_t x = WIDTH; x < OUT_PITCH; ++x)
                        CHECK_EQ_HEX(out[y * OUT_PITCH + x], SENTINEL);
                }
                CHECK_EQ_HEX(out[0], pal[0x056]);
                CHECK_EQ_HEX(out[1], pal[0x016]);
                CHECK_EQ_HEX(out[2], pal[0x116]);
                nesturbator_destroy(inst);
            }
        }
    }

    CHECK_DONE();
}
