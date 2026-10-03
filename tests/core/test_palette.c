/* D-14: nesturbator_get_palette's copy rules and the palette invariants. */
#include <stdint.h>
#include <string.h>

#include "nesturbator.h"
#include "../check.h"

#define SENTINEL 0xDEADBEEFu

static uint32_t red(uint32_t c)
{
    return (c >> 16) & 0xFFu;
}
static uint32_t green(uint32_t c)
{
    return (c >> 8) & 0xFFu;
}
static uint32_t blue(uint32_t c)
{
    return c & 0xFFu;
}
static uint32_t luma(uint32_t c)
{
    return 299u * red(c) + 587u * green(c) + 114u * blue(c);
}

int main(void)
{
    nesturbator_config cfg;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    nesturbator *inst = NULL;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);

    /* Copy rules: count is clipped to 512, and nothing beyond it is written. */
    uint32_t pal[513];
    uint32_t buf[513];
    for (uint32_t i = 0; i < 513u; i++) {
        pal[i] = SENTINEL;
    }
    nesturbator_get_palette(inst, pal, 600u);
    CHECK_EQ_HEX(pal[512], SENTINEL);
    for (uint32_t i = 0; i < 512u; i++) {
        CHECK(pal[i] <= 0xFFFFFFu);
    }

    for (uint32_t i = 0; i < 513u; i++) {
        buf[i] = SENTINEL;
    }
    nesturbator_get_palette(inst, buf, 3u);
    CHECK_EQ_HEX(buf[0], pal[0]);
    CHECK_EQ_HEX(buf[2], pal[2]);
    CHECK_EQ_HEX(buf[3], SENTINEL);

    nesturbator_get_palette(inst, buf, 512u);
    for (uint32_t i = 0; i < 512u; i++) {
        CHECK_EQ_HEX(buf[i], pal[i]);
    }

    buf[0] = SENTINEL;
    nesturbator_get_palette(inst, buf, 0u);
    CHECK_EQ_HEX(buf[0], SENTINEL);
    nesturbator_get_palette(inst, NULL, 512u); /* must not crash */

    /* $20 and $30 are white. */
    CHECK_EQ_HEX(pal[0x20], 0xFFFFFFu);
    CHECK_EQ_HEX(pal[0x30], 0xFFFFFFu);

    /* $xE/$xF are black under every emphasis. */
    for (uint32_t e = 0; e < 8u; e++) {
        for (uint32_t r = 0; r < 4u; r++) {
            CHECK_EQ_HEX(pal[(e << 6) | (r << 4) | 0xEu], 0u);
            CHECK_EQ_HEX(pal[(e << 6) | (r << 4) | 0xFu], 0u);
        }
    }

    /* An emphasis bit never raises a channel other than its own: R may rise
       only with native bit 6, G never, B only with native bit 8. */
    for (uint32_t e = 1; e < 8u; e++) {
        for (uint32_t i = 0; i < 64u; i++) {
            uint32_t base = pal[i];
            uint32_t emph = pal[(e << 6) | i];
            if ((e & 1u) == 0u) {
                CHECK(red(emph) <= red(base));
            }
            CHECK(green(emph) <= green(base));
            if ((e & 4u) == 0u) {
                CHECK(blue(emph) <= blue(base));
            }
        }
    }

    /* Integer luma never falls down a column ($0x, $1x, $2x, $3x). */
    for (uint32_t e = 0; e < 8u; e++) {
        for (uint32_t h = 0; h < 16u; h++) {
            for (uint32_t r = 1; r < 4u; r++) {
                uint32_t above = pal[(e << 6) | ((r - 1u) << 4) | h];
                uint32_t here = pal[(e << 6) | (r << 4) | h];
                CHECK(luma(here) >= luma(above));
            }
        }
    }

    nesturbator_destroy(inst);
    CHECK_DONE();
}
