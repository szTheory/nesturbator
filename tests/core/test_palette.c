/* D-14: nesturbator_get_palette's copy rules and the palette invariants. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"
#include "../../runner/sha256.h"

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

static void check_rgb_digest(const uint32_t colors[512], const char *expected)
{
    uint8_t rgb[512u * 3u];
    for (uint32_t i = 0; i < 512u; ++i) {
        rgb[3u * i] = (uint8_t)(colors[i] >> 16);
        rgb[3u * i + 1u] = (uint8_t)(colors[i] >> 8);
        rgb[3u * i + 2u] = (uint8_t)colors[i];
    }

    nesturbator_run_sha256 hash;
    uint8_t digest[32];
    char hex[65];
    nesturbator_run_sha256_init(&hash);
    nesturbator_run_sha256_update(&hash, rgb, sizeof rgb);
    nesturbator_run_sha256_final(&hash, digest);
    nesturbator_run_sha256_hex(digest, hex);
    CHECK(strcmp(hex, expected) == 0);
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

    /* Fixed NTSC-to-sRGB calibration samples. These pin both transfer and
       primary conversion at black, neutral, hue, emphasis, and table edges. */
    CHECK_EQ_HEX(pal[0x00], 0x5A5A5Au);
    CHECK_EQ_HEX(pal[0x0D], 0x000000u);
    CHECK_EQ_HEX(pal[0x16], 0xBA3100u);
    CHECK_EQ_HEX(pal[0x56], 0xBC2700u);
    CHECK_EQ_HEX(pal[0x100], 0x463E7Au);
    CHECK_EQ_HEX(pal[0x1FF], 0x000000u);

    /* $xE/$xF are black under every emphasis. */
    for (uint32_t e = 0; e < 8u; e++) {
        for (uint32_t r = 0; r < 4u; r++) {
            CHECK_EQ_HEX(pal[(e << 6) | (r << 4) | 0xEu], 0u);
            CHECK_EQ_HEX(pal[(e << 6) | (r << 4) | 0xFu], 0u);
        }
    }

    /* Pin the calibrated red- and blue-emphasis outputs directly. */
    CHECK_EQ_HEX(pal[0x56], 0xBC2700u);
    CHECK_EQ_HEX(pal[0x116], 0x991900u);

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

    /* A real NES 2.0 PAL load selects the independently sampled 2C07 model. */
    uint8_t pal_image[16u + INES_PRG_BANK];
    struct ines_spec pal_spec;
    memset(&pal_spec, 0, sizeof pal_spec);
    pal_spec.prg_16k = 1u;
    pal_spec.nes2 = 1u;
    const size_t pal_image_size = ines_build(pal_image, sizeof pal_image, &pal_spec);
    CHECK_EQ_U64(pal_image_size, sizeof pal_image);
    pal_image[12] = 1u; /* NES 2.0 timing: explicit PAL. */

    nesturbator *pal_inst = NULL;
    CHECK_EQ_U64(nesturbator_create(&cfg, &pal_inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge(pal_inst, pal_image, pal_image_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_get_region(pal_inst), NESTURBATOR_REGION_PAL);

    uint32_t pal_table[512];
    nesturbator_get_palette(pal_inst, pal_table, 512u);
    CHECK_EQ_HEX(pal_table[0x00], 0x626262u);
    CHECK_EQ_HEX(pal_table[0x01], 0x002365u);
    CHECK_EQ_HEX(pal_table[0x06], 0x540501u);
    CHECK_EQ_HEX(pal_table[0x0f], 0x000000u);
    CHECK_EQ_HEX(pal_table[0x16], 0x932824u);
    CHECK_EQ_HEX(pal_table[0x21], 0x61A1FBu);
    CHECK_EQ_HEX(pal_table[0x26], 0xE47974u);
    CHECK_EQ_HEX(pal_table[0x31], 0xC4DDFFu);
    CHECK_EQ_HEX(pal_table[0x3e], 0x000000u);
    CHECK(pal_table[0x01] != pal[0x01]);

    /* The top three index bits retain raw PPUMASK emphasis order: bit 5 is
       green on PAL, bit 6 red, and bit 7 blue. These $16 variants are from
       the same independent published RGB output as the full-table digest. */
    CHECK_EQ_HEX(pal_table[0x056], 0x8E1914u); /* PPUMASK bit 5: green */
    CHECK_EQ_HEX(pal_table[0x096], 0x6F2304u); /* PPUMASK bit 6: red */
    CHECK_EQ_HEX(pal_table[0x116], 0x75172Eu); /* PPUMASK bit 7: blue */
    check_rgb_digest(pal_table, "cb052e4ef8c738c19c2304804e3851d5afd2ba8bf68ed810cabd69de0fe9e9e5");
    uint32_t pal_reference[512];
    memcpy(pal_reference, pal_table, sizeof pal_reference);

    /* NULL and unloaded instances keep the original NTSC table. */
    uint32_t null_table[512];
    nesturbator_get_palette(NULL, null_table, 512u);
    CHECK(memcmp(null_table, pal, sizeof null_table) == 0);

    /* PAL copy bounds match the NTSC contract. */
    uint32_t pal_buf[513];
    for (uint32_t i = 0u; i < 513u; ++i) {
        pal_buf[i] = SENTINEL;
    }
    nesturbator_get_palette(pal_inst, pal_buf, 0u);
    CHECK_EQ_HEX(pal_buf[0], SENTINEL);
    nesturbator_get_palette(pal_inst, pal_buf, 3u);
    CHECK_EQ_HEX(pal_buf[0], pal_table[0]);
    CHECK_EQ_HEX(pal_buf[2], pal_table[2]);
    CHECK_EQ_HEX(pal_buf[3], SENTINEL);
    nesturbator_get_palette(pal_inst, pal_buf, 600u);
    CHECK_EQ_HEX(pal_buf[511], pal_table[511]);
    CHECK_EQ_HEX(pal_buf[512], SENTINEL);
    nesturbator_get_palette(pal_inst, NULL, 512u);

    /* Alternate two loaded regions without changing tables or native state. */
    uint8_t ntsc_image[16u + INES_PRG_BANK];
    const size_t ntsc_image_size = ines_build(ntsc_image, sizeof ntsc_image, &pal_spec);
    CHECK_EQ_U64(ntsc_image_size, sizeof ntsc_image);
    ntsc_image[12] = 0u; /* NES 2.0 timing: explicit NTSC. */
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, ntsc_image, ntsc_image_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_get_region(inst), NESTURBATOR_REGION_NTSC);

    uint32_t ntsc_table[512];
    nesturbator_get_palette(inst, ntsc_table, 512u);
    CHECK(memcmp(ntsc_table, pal, sizeof ntsc_table) == 0);
    const size_t pal_state_size = nesturbator_state_size(pal_inst);
    const size_t ntsc_state_size = nesturbator_state_size(inst);
    uint8_t *pal_state_before = malloc(pal_state_size);
    uint8_t *pal_state_after = malloc(pal_state_size);
    uint8_t *ntsc_state_before = malloc(ntsc_state_size);
    uint8_t *ntsc_state_after = malloc(ntsc_state_size);
    CHECK(pal_state_before != NULL && pal_state_after != NULL && ntsc_state_before != NULL &&
          ntsc_state_after != NULL);
    if (pal_state_before != NULL && pal_state_after != NULL && ntsc_state_before != NULL &&
        ntsc_state_after != NULL) {
        const nesturbator_status pal_before_status =
            nesturbator_save_state(pal_inst, pal_state_before, pal_state_size);
        const nesturbator_status ntsc_before_status =
            nesturbator_save_state(inst, ntsc_state_before, ntsc_state_size);
        CHECK_EQ_U64(pal_before_status, NESTURBATOR_OK);
        CHECK_EQ_U64(ntsc_before_status, NESTURBATOR_OK);
        if (pal_before_status == NESTURBATOR_OK && ntsc_before_status == NESTURBATOR_OK) {
            for (unsigned repeat = 0u; repeat < 4u; ++repeat) {
                nesturbator_get_palette(pal_inst, pal_table, 512u);
                nesturbator_get_palette(inst, ntsc_table, 512u);
                CHECK(memcmp(pal_table, pal_reference, sizeof pal_reference) == 0);
                CHECK(memcmp(ntsc_table, pal, sizeof ntsc_table) == 0);
                CHECK_EQ_U64(nesturbator_get_region(pal_inst), NESTURBATOR_REGION_PAL);
                CHECK_EQ_U64(nesturbator_get_region(inst), NESTURBATOR_REGION_NTSC);
            }
            const nesturbator_status pal_after_status =
                nesturbator_save_state(pal_inst, pal_state_after, pal_state_size);
            const nesturbator_status ntsc_after_status =
                nesturbator_save_state(inst, ntsc_state_after, ntsc_state_size);
            CHECK_EQ_U64(pal_after_status, NESTURBATOR_OK);
            CHECK_EQ_U64(ntsc_after_status, NESTURBATOR_OK);
            if (pal_after_status == NESTURBATOR_OK && ntsc_after_status == NESTURBATOR_OK) {
                CHECK(memcmp(pal_state_before, pal_state_after, pal_state_size) == 0);
                CHECK(memcmp(ntsc_state_before, ntsc_state_after, ntsc_state_size) == 0);
            }
        }
    }
    free(pal_state_before);
    free(pal_state_after);
    free(ntsc_state_before);
    free(ntsc_state_after);

    /* Reload and unload reset the selected display table with the effective region. */
    CHECK_EQ_U64(nesturbator_load_cartridge(pal_inst, ntsc_image, ntsc_image_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_get_region(pal_inst), NESTURBATOR_REGION_NTSC);
    nesturbator_get_palette(pal_inst, pal_table, 512u);
    CHECK(memcmp(pal_table, pal, sizeof pal_table) == 0);
    nesturbator_unload_cartridge(pal_inst);
    CHECK_EQ_U64(nesturbator_get_region(pal_inst), NESTURBATOR_REGION_NTSC);
    nesturbator_get_palette(pal_inst, pal_table, 512u);
    CHECK(memcmp(pal_table, pal, sizeof pal_table) == 0);

    nesturbator_destroy(pal_inst);

    nesturbator_destroy(inst);
    CHECK_DONE();
}
