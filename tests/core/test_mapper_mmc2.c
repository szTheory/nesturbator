/* Owned mapper-9 profile, PRG, and mirroring tests. The fixture is generated in memory;
   expected PRG/CHR markers below are literal and independent of its builder arithmetic. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../mmc2_fixture.h"
#include "../state_test.h"

static const uint8_t prg_markers[16][8] = {
    {0x10u, 0x11u, 0x12u, 0x13u, 0x14u, 0x15u, 0x16u, 0x17u},
    {0x18u, 0x19u, 0x1au, 0x1bu, 0x1cu, 0x1du, 0x1eu, 0x1fu},
    {0x20u, 0x21u, 0x22u, 0x23u, 0x24u, 0x25u, 0x26u, 0x27u},
    {0x28u, 0x29u, 0x2au, 0x2bu, 0x2cu, 0x2du, 0x2eu, 0x2fu},
    {0x30u, 0x31u, 0x32u, 0x33u, 0x34u, 0x35u, 0x36u, 0x37u},
    {0x38u, 0x39u, 0x3au, 0x3bu, 0x3cu, 0x3du, 0x3eu, 0x3fu},
    {0x40u, 0x41u, 0x42u, 0x43u, 0x44u, 0x45u, 0x46u, 0x47u},
    {0x48u, 0x49u, 0x4au, 0x4bu, 0x4cu, 0x4du, 0x4eu, 0x4fu},
    {0x50u, 0x51u, 0x52u, 0x53u, 0x54u, 0x55u, 0x56u, 0x57u},
    {0x58u, 0x59u, 0x5au, 0x5bu, 0x5cu, 0x5du, 0x5eu, 0x5fu},
    {0x60u, 0x61u, 0x62u, 0x63u, 0x64u, 0x65u, 0x66u, 0x67u},
    {0x68u, 0x69u, 0x6au, 0x6bu, 0x6cu, 0x6du, 0x6eu, 0x6fu},
    {0x70u, 0x71u, 0x72u, 0x73u, 0x74u, 0x75u, 0x76u, 0x77u},
    {0x78u, 0x79u, 0x7au, 0x7bu, 0x7cu, 0x7du, 0x7eu, 0x7fu},
    {0x80u, 0x81u, 0x82u, 0x83u, 0x84u, 0x85u, 0x86u, 0x87u},
    {0x88u, 0x89u, 0x8au, 0x8bu, 0x8cu, 0x8du, 0x8eu, 0x8fu},
};

static const uint8_t chr_markers[32] = {
    0xa0u, 0xa1u, 0xa2u, 0xa3u, 0xa4u, 0xa5u, 0xa6u, 0xa7u, 0xa8u, 0xa9u, 0xaau,
    0xabu, 0xacu, 0xadu, 0xaeu, 0xafu, 0xb0u, 0xb1u, 0xb2u, 0xb3u, 0xb4u, 0xb5u,
    0xb6u, 0xb7u, 0xb8u, 0xb9u, 0xbau, 0xbbu, 0xbcu, 0xbdu, 0xbeu, 0xbfu,
};

struct mmc2_allocator_counts {
    unsigned allocations;
    unsigned frees;
};

enum invalid_profile {
    INVALID_INES,
    INVALID_ARCHAIC,
    INVALID_SUBMAPPER,
    INVALID_SUBMAPPER_MAX,
    INVALID_MAPPER_10,
    INVALID_PRG_SMALL,
    INVALID_PRG_LARGE,
    INVALID_CHR_SMALL,
    INVALID_CHR_LARGE,
    INVALID_PRG_EXPONENT,
    INVALID_CHR_EXPONENT,
    INVALID_PRG_RAM,
    INVALID_PRG_NVRAM,
    INVALID_CHR_RAM,
    INVALID_CHR_NVRAM,
    INVALID_CHR_ABSENT,
    INVALID_CHR_RAM_BOARD,
    INVALID_TRAINER,
    INVALID_BATTERY,
    INVALID_FOUR_SCREEN,
    INVALID_CONSOLE,
    INVALID_RESERVED_TIMING,
    INVALID_MISC_HEADER,
    INVALID_MISC_HEADER_14,
    INVALID_MISC_HEADER_15,
    INVALID_DENDY_REGION,
    INVALID_SHORT_FILE,
    INVALID_TRAILING_FILE,
};

static void *mmc2_alloc(void *user, size_t size)
{
    struct mmc2_allocator_counts *counts = user;
    counts->allocations++;
    return malloc(size);
}

static void mmc2_free(void *user, void *ptr, size_t size)
{
    struct mmc2_allocator_counts *counts = user;
    (void)size;
    counts->frees++;
    free(ptr);
}

static nesturbator *make_instance(struct mmc2_allocator_counts *counts)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    cfg.allocator.alloc = mmc2_alloc;
    cfg.allocator.free = mmc2_free;
    cfg.allocator.user = counts;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    return inst;
}

static void run_frame(nesturbator *inst)
{
    static uint16_t video[256u * 240u];
    static int16_t audio[2048];
    nesturbator_frame io;
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = video;
    io.video_pitch = 256u;
    io.audio = audio;
    io.audio_capacity = 2048u;
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
}

static uint8_t cpu_peek(struct nesturbator *nes, uint16_t addr)
{
    return nesturbator__map_cpu_read(nes, addr);
}

static void check_rejected_without_change(struct nesturbator *nes, const uint8_t *image,
                                          size_t size, struct mmc2_allocator_counts *counts)
{
    uint8_t snapshot[sizeof *nes];
    memcpy(snapshot, nes, sizeof *nes);
    const unsigned allocations = counts->allocations;
    const unsigned frees = counts->frees;
    CHECK_EQ_U64(nesturbator_load_cartridge((nesturbator *)nes, image, size),
                 NESTURBATOR_ERR_CARTRIDGE);
    CHECK(memcmp(snapshot, nes, sizeof *nes) == 0);
    CHECK_EQ_U64(counts->allocations, allocations);
    CHECK_EQ_U64(counts->frees, frees);
}

static size_t build_invalid(enum invalid_profile which, const uint8_t *valid, size_t valid_size,
                            uint8_t *out, size_t cap)
{
    if (cap < valid_size + INES_TRAINER_SIZE + 1u)
        return 0u;
    memcpy(out, valid, valid_size);
    switch (which) {
    case INVALID_INES:
        out[7] &= (uint8_t)~0x0cu;
        break;
    case INVALID_ARCHAIC:
        out[7] = (uint8_t)((out[7] & (uint8_t)~0x0cu) | 0x04u);
        break;
    case INVALID_SUBMAPPER:
        out[8] = 0x10u;
        break;
    case INVALID_SUBMAPPER_MAX:
        out[8] = 0xf0u;
        break;
    case INVALID_MAPPER_10:
        out[6] = (uint8_t)((out[6] & 0x0fu) | 0xa0u);
        break;
    case INVALID_PRG_SMALL:
        out[4] = 7u;
        break;
    case INVALID_PRG_LARGE:
        out[4] = 9u;
        break;
    case INVALID_CHR_SMALL:
        out[5] = 15u;
        break;
    case INVALID_CHR_LARGE:
        out[5] = 17u;
        break;
    case INVALID_PRG_EXPONENT:
        out[4] = 0xfcu;
        out[9] = (uint8_t)((out[9] & 0xf0u) | 0x0fu);
        break;
    case INVALID_CHR_EXPONENT:
        out[5] = 0xfcu;
        out[9] = (uint8_t)((out[9] & 0x0fu) | 0xf0u);
        break;
    case INVALID_PRG_RAM:
        out[10] |= 0x01u;
        break;
    case INVALID_PRG_NVRAM:
        out[10] |= 0x10u;
        out[6] |= 0x02u;
        break;
    case INVALID_CHR_RAM:
        out[11] |= 0x07u;
        break;
    case INVALID_CHR_NVRAM:
        out[11] |= 0x70u;
        out[6] |= 0x02u;
        break;
    case INVALID_CHR_ABSENT:
        out[5] = 0u;
        out[9] &= 0x0fu;
        break;
    case INVALID_CHR_RAM_BOARD:
        out[5] = 0u;
        out[9] &= 0x0fu;
        out[11] = (uint8_t)((out[11] & 0xf0u) | 0x07u);
        break;
    case INVALID_TRAINER:
        memmove(out + 16u + INES_TRAINER_SIZE, out + 16u, valid_size - 16u);
        memset(out + 16u, 0x5au, INES_TRAINER_SIZE);
        out[6] |= 0x04u;
        return valid_size + INES_TRAINER_SIZE;
    case INVALID_BATTERY:
        out[6] |= 0x02u;
        break;
    case INVALID_FOUR_SCREEN:
        out[6] |= 0x08u;
        break;
    case INVALID_CONSOLE:
        out[7] |= 0x01u;
        break;
    case INVALID_RESERVED_TIMING:
        out[12] = 0x04u;
        break;
    case INVALID_MISC_HEADER:
        out[13] = 1u;
        break;
    case INVALID_MISC_HEADER_14:
        out[14] = 1u;
        break;
    case INVALID_MISC_HEADER_15:
        out[15] = 1u;
        break;
    case INVALID_DENDY_REGION:
        out[12] = 3u;
        break;
    case INVALID_SHORT_FILE:
        return valid_size - 1u;
    case INVALID_TRAILING_FILE:
        out[valid_size] = 0u;
        return valid_size + 1u;
    }
    return valid_size;
}

static void test_profile_refusals_are_transactional(void)
{
    static uint8_t image[MMC2_FIXTURE_CAP];
    static uint8_t invalid[MMC2_IMAGE_CAP];
    static const enum invalid_profile cases[] = {
        INVALID_INES,          INVALID_ARCHAIC,        INVALID_SUBMAPPER,
        INVALID_SUBMAPPER_MAX, INVALID_MAPPER_10,      INVALID_PRG_SMALL,
        INVALID_CHR_SMALL,     INVALID_CHR_LARGE,      INVALID_PRG_EXPONENT,
        INVALID_CHR_EXPONENT,  INVALID_PRG_RAM,        INVALID_PRG_NVRAM,
        INVALID_CHR_RAM,       INVALID_CHR_NVRAM,      INVALID_CHR_ABSENT,
        INVALID_CHR_RAM_BOARD, INVALID_TRAINER,        INVALID_BATTERY,
        INVALID_FOUR_SCREEN,   INVALID_CONSOLE,        INVALID_RESERVED_TIMING,
        INVALID_MISC_HEADER,   INVALID_MISC_HEADER_14, INVALID_MISC_HEADER_15,
        INVALID_DENDY_REGION,  INVALID_SHORT_FILE,     INVALID_TRAILING_FILE,
    };
    struct mmc2_allocator_counts counts = {0u, 0u};
    const size_t size = mmc2_fixture_build(image, sizeof image, 1u);
    CHECK_EQ_U64(size, MMC2_FIXTURE_CAP);
    nesturbator *inst = make_instance(&counts);
    nesturbator *control = make_instance(&counts);
    CHECK(inst != NULL && control != NULL);
    if (inst == NULL || control == NULL)
        goto done;
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge(control, image, size), NESTURBATOR_OK);
    struct nesturbator *nes = (struct nesturbator *)inst;
    struct nesturbator *peer = (struct nesturbator *)control;
    nesturbator__bus_write(nes, 0xa000u, 5u);
    nesturbator__bus_write(peer, 0xa000u, 5u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.prg, 5u);

    for (size_t i = 0u; i < sizeof cases / sizeof cases[0]; ++i) {
        const size_t invalid_size = build_invalid(cases[i], image, size, invalid, sizeof invalid);
        CHECK(invalid_size != 0u);
        check_rejected_without_change(nes, invalid, invalid_size, &counts);
    }

    /* Rejections did not change the current cartridge or its continuation. */
    run_frame(inst);
    run_frame(control);
    for (uint16_t addr = 0u; addr < 4u; ++addr) {
        uint8_t actual = 0u;
        uint8_t expected = 0u;
        CHECK_EQ_U64(nesturbator_peek_cpu_ram(inst, addr, &actual), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_peek_cpu_ram(control, addr, &expected), NESTURBATOR_OK);
        CHECK_EQ_HEX(actual, expected);
    }

done:
    nesturbator_destroy(inst);
    nesturbator_destroy(control);
}

static void check_ciram_aliases(struct nesturbator *nes, uint8_t vertical)
{
    static const uint16_t addresses[] = {0x2000u, 0x2400u, 0x2800u, 0x2c00u};
    static const uint8_t vertical_expected[] = {0x33u, 0x44u, 0x33u, 0x44u};
    static const uint8_t horizontal_expected[] = {0x22u, 0x22u, 0x44u, 0x44u};
    memset(nes->ppu.nametable, 0, sizeof nes->ppu.nametable);
    for (unsigned i = 0u; i < 4u; ++i)
        nesturbator__ppu_write(nes, addresses[i], (uint8_t)(0x11u * (i + 1u)));
    for (unsigned i = 0u; i < 4u; ++i)
        CHECK_EQ_HEX(nesturbator__ppu_read(nes, addresses[i]),
                     vertical != 0u ? vertical_expected[i] : horizontal_expected[i]);
}

static void test_prg_banks_registers_and_mirroring(void)
{
    static uint8_t vertical_image[MMC2_FIXTURE_CAP];
    static uint8_t horizontal_image[MMC2_FIXTURE_CAP];
    const size_t vertical_size = mmc2_fixture_build(vertical_image, sizeof vertical_image, 1u);
    const size_t horizontal_size =
        mmc2_fixture_build(horizontal_image, sizeof horizontal_image, 0u);
    struct mmc2_allocator_counts counts = {0u, 0u};
    nesturbator *inst = make_instance(&counts);
    nesturbator *peer = make_instance(&counts);
    CHECK(inst != NULL && peer != NULL);
    if (inst == NULL || peer == NULL)
        goto done;
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, vertical_image, vertical_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge(peer, horizontal_image, horizontal_size),
                 NESTURBATOR_OK);
    struct nesturbator *nes = (struct nesturbator *)inst;
    struct nesturbator *other = (struct nesturbator *)peer;

    CHECK_EQ_U64(nes->mapper.reg.mmc2.prg, 0u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.chr[0], 0u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.chr[1], 0u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.chr[2], 0u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.chr[3], 0u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[0], 1u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[1], 1u);
    static const uint8_t vertical_map[] = {0u, 1u, 0u, 1u};
    static const uint8_t horizontal_map[] = {0u, 0u, 1u, 1u};
    CHECK(memcmp(nes->map.nt, vertical_map, sizeof vertical_map) == 0);
    CHECK(memcmp(other->map.nt, horizontal_map, sizeof horizontal_map) == 0);
    check_ciram_aliases(nes, 1u);
    check_ciram_aliases(other, 0u);

    /* Every selector reads all eight independently marked 1 KiB pages in bank 0..15. */
    for (unsigned bank = 0u; bank < 16u; ++bank) {
        nesturbator__bus_write(nes, 0xa000u, (uint8_t)(0xf0u | bank));
        CHECK_EQ_U64(nes->mapper.reg.mmc2.prg, bank);
        for (unsigned page = 0u; page < 8u; ++page) {
            const uint16_t base = (uint16_t)(0x8000u + page * MMC2_PAGE_1K);
            CHECK_EQ_HEX(cpu_peek(nes, base), prg_markers[bank][page]);
            const uint16_t last_offset = (bank == 15u && page == 7u) ? 0x3f9u : 0x3ffu;
            CHECK_EQ_HEX(cpu_peek(nes, (uint16_t)(base + last_offset)), prg_markers[bank][page]);
        }
    }

    /* Banks 13, 14 and 15 stay fixed in their windows for every selector. */
    static const uint8_t fixed_banks[] = {13u, 14u, 15u};
    static const uint16_t fixed_bases[] = {0xa000u, 0xc000u, 0xe000u};
    for (unsigned window = 0u; window < 3u; ++window) {
        for (unsigned page = 0u; page < 8u; ++page) {
            const uint16_t base = (uint16_t)(fixed_bases[window] + page * MMC2_PAGE_1K);
            CHECK_EQ_HEX(cpu_peek(nes, base), prg_markers[fixed_banks[window]][page]);
            const uint16_t last_offset = (window == 2u && page == 7u) ? 0x3f0u : 0x3ffu;
            CHECK_EQ_HEX(cpu_peek(nes, (uint16_t)(base + last_offset)),
                         prg_markers[fixed_banks[window]][page]);
        }
    }
    CHECK_EQ_HEX(cpu_peek(nes, 0xffffu), 0xfeu); /* reset-vector high byte */

    /* Register decode boundaries, data masks, aliases, and repeated writes. */
    nesturbator__bus_write(nes, 0xa000u, 5u);
    nesturbator__bus_write(nes, 0x9fffu, 9u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.prg, 5u);
    nesturbator__bus_write(nes, 0xafffu, 0xfcu);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.prg, 12u);
    CHECK_EQ_HEX(cpu_peek(nes, 0x8000u), prg_markers[12][0]);
    nesturbator__bus_write(nes, 0xb000u, 0xe7u);
    nesturbator__bus_write(nes, 0xc000u, 0xe5u);
    nesturbator__bus_write(nes, 0xd000u, 0xe9u);
    nesturbator__bus_write(nes, 0xe000u, 0xe7u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.chr[0], 7u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.chr[1], 5u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.chr[2], 9u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.chr[3], 7u);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x0000u), chr_markers[5]);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x1000u), chr_markers[7]);
    const uint8_t chr_before_write = nesturbator__ppu_read(nes, 0x0000u);
    nesturbator__ppu_write(nes, 0x0000u, 0xffu);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x0000u), chr_before_write);
    CHECK(nes->map.chr_w[0] == NULL);

    nesturbator__bus_write(nes, 0xefffu, 0x00u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.chr[3], 0u);
    nesturbator__bus_write(nes, 0xf000u, 0xfeu);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.mirror, 0u);
    CHECK(memcmp(nes->map.nt, vertical_map, sizeof vertical_map) == 0);
    check_ciram_aliases(nes, 1u);
    nesturbator__bus_write(nes, 0xf000u, 0xffu);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.mirror, 1u);
    CHECK(memcmp(nes->map.nt, horizontal_map, sizeof horizontal_map) == 0);
    check_ciram_aliases(nes, 0u);
    nesturbator__bus_write(nes, 0xf000u, 0xffu);
    CHECK(memcmp(nes->map.nt, horizontal_map, sizeof horizontal_map) == 0);

    /* A second instance has independent PRG selectors and mirror state. */
    nesturbator__bus_write(nes, 0xa000u, 1u);
    nesturbator__bus_write(other, 0xa000u, 9u);
    CHECK_EQ_HEX(cpu_peek(nes, 0x8000u), prg_markers[1][0]);
    CHECK_EQ_HEX(cpu_peek(other, 0x8000u), prg_markers[9][0]);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.prg, 1u);
    CHECK_EQ_U64(other->mapper.reg.mmc2.prg, 9u);

    nesturbator__bus_write(nes, 0xf000u, 1u);
    const struct nesturbator__mapper saved = nes->mapper;
    CHECK_EQ_U64(nesturbator_reset(inst), NESTURBATOR_OK);
    CHECK(memcmp(&saved, &nes->mapper, sizeof saved) == 0);
    CHECK(memcmp(nes->map.nt, horizontal_map, sizeof horizontal_map) == 0);
    CHECK_EQ_HEX(cpu_peek(nes, 0x8000u), prg_markers[1][0]);

done:
    nesturbator_destroy(inst);
    nesturbator_destroy(peer);
}

static void test_exponent_profile_encoding_and_ntsc_refusals(void)
{
    static uint8_t image[MMC2_FIXTURE_CAP];
    const size_t size = mmc2_fixture_build(image, sizeof image, 1u);
    struct mmc2_allocator_counts counts = {0u, 0u};
    nesturbator *inst = make_instance(&counts);
    CHECK(inst != NULL);
    if (inst == NULL)
        return;

    /* E=17, M=0 encodes 2^17 bytes for both ROMs through the existing parser. */
    image[4] = 0x44u;
    image[5] = 0x44u;
    image[9] = 0xffu;
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);

    /* This core is NTSC-only. PAL and dual-region headers must refuse the new
       image transactionally; no later region API is part of this profile. */
    struct nesturbator *nes = (struct nesturbator *)inst;
    image[12] = 1u;
    check_rejected_without_change(nes, image, size, &counts);
    image[12] = 2u;
    check_rejected_without_change(nes, image, size, &counts);

    nesturbator_destroy(inst);
}

static void test_pattern_read_returns_old_bank_then_updates_latch(void)
{
    static uint8_t image[MMC2_FIXTURE_CAP];
    const size_t size = mmc2_fixture_build(image, sizeof image, 1u);
    struct mmc2_allocator_counts counts = {0u, 0u};
    nesturbator *inst = make_instance(&counts);
    CHECK(inst != NULL);
    if (inst == NULL)
        return;
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    struct nesturbator *nes = (struct nesturbator *)inst;
    nesturbator__bus_write(nes, 0xb000u, 5u); /* FD selector: bank 5 */
    nesturbator__bus_write(nes, 0xc000u, 3u); /* FE selector: bank 3 */

    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x0fd0u), 0xa3u);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x0000u), 0xa5u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[0], 0u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[1], 1u);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x1000u), 0xa0u);
    nesturbator_destroy(inst);
}

static void test_unwatched_mapper_keeps_direct_chr_read_path(void)
{
    static uint8_t image[16u + 0x4000u + 0x2000u];
    struct ines_spec spec;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = 1u;
    const size_t size = ines_build(image, sizeof image, &spec);
    CHECK_EQ_U64(size, sizeof image);
    memset(image + 16u + 0x4000u, 0x5cu, 0x2000u);
    struct mmc2_allocator_counts counts = {0u, 0u};
    nesturbator *inst = make_instance(&counts);
    CHECK(inst != NULL);
    if (inst == NULL)
        return;
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    struct nesturbator *nes = (struct nesturbator *)inst;
    const uint8_t *page = nes->map.chr_r[3];
    CHECK_EQ_U64(nes->map.watch & NESTURBATOR_WATCH_PPU_READ, 0u);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x0fd0u), 0x5cu);
    CHECK(nes->map.chr_r[3] == page);
    nesturbator_destroy(inst);
}

static nesturbator *make_loaded_instance(struct mmc2_allocator_counts *counts)
{
    static uint8_t image[MMC2_FIXTURE_CAP];
    const size_t size = mmc2_fixture_build(image, sizeof image, 1u);
    nesturbator *inst = make_instance(counts);
    if (inst != NULL)
        CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    return inst;
}

static void select_chr_banks(struct nesturbator *nes)
{
    nesturbator__bus_write(nes, 0xb000u, 5u); /* $0000 FD */
    nesturbator__bus_write(nes, 0xc000u, 3u); /* $0000 FE */
    nesturbator__bus_write(nes, 0xd000u, 9u); /* $1000 FD */
    nesturbator__bus_write(nes, 0xe000u, 7u); /* $1000 FE */
}

static void force_latch(struct nesturbator *nes, unsigned half, uint8_t fd)
{
    const uint16_t base = (uint16_t)(half * 0x1000u);
    const uint16_t trigger = (uint16_t)(base + (fd != 0u ? 0x0fd0u : 0x0fe0u));
    (void)nesturbator__ppu_read(nes, trigger);
}

static void test_all_full_row_trigger_offsets(void)
{
    static const uint8_t fd_bank[2] = {5u, 9u};
    static const uint8_t fe_bank[2] = {3u, 7u};
    struct mmc2_allocator_counts counts = {0u, 0u};
    nesturbator *inst = make_loaded_instance(&counts);
    CHECK(inst != NULL);
    if (inst == NULL)
        return;
    struct nesturbator *nes = (struct nesturbator *)inst;
    select_chr_banks(nes);

    for (unsigned half = 0u; half < 2u; ++half) {
        const uint16_t base = (uint16_t)(half * 0x1000u);
        const unsigned other = half ^ 1u;
        const uint16_t other_base = (uint16_t)(other * 0x1000u);
        for (unsigned position = 0u; position < 2u; ++position) {
            const uint8_t target_fd = position == 0u ? 1u : 0u;
            const uint16_t row = (uint16_t)(base + (target_fd != 0u ? 0x0fd0u : 0x0fe0u));
            const uint8_t old_bank = target_fd != 0u ? fe_bank[half] : fd_bank[half];
            const uint8_t new_bank = target_fd != 0u ? fd_bank[half] : fe_bank[half];
            for (unsigned offset = 0u; offset < 16u; ++offset) {
                force_latch(nes, half, (uint8_t)(target_fd == 0u));
                CHECK_EQ_HEX(nesturbator__ppu_read(nes, (uint16_t)(row + offset)),
                             chr_markers[old_bank]);
                CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[half], target_fd == 0u ? 1u : 0u);
                CHECK_EQ_HEX(nesturbator__ppu_read(nes, base), chr_markers[new_bank]);
                CHECK_EQ_HEX(nesturbator__ppu_read(nes, other_base), chr_markers[fe_bank[other]]);
                CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[other], 1u);
            }
        }
    }
    nesturbator_destroy(inst);
}

static void test_trigger_boundaries_and_non_pattern_accesses(void)
{
    static const uint8_t fd_bank[2] = {5u, 9u};
    static const uint8_t fe_bank[2] = {3u, 7u};
    struct mmc2_allocator_counts counts = {0u, 0u};
    nesturbator *inst = make_loaded_instance(&counts);
    CHECK(inst != NULL);
    if (inst == NULL)
        return;
    struct nesturbator *nes = (struct nesturbator *)inst;
    select_chr_banks(nes);
    nes->ppu.mask = 0u;

    for (unsigned half = 0u; half < 2u; ++half) {
        const uint16_t base = (uint16_t)(half * 0x1000u);
        force_latch(nes, half, 0u);
        CHECK_EQ_HEX(nesturbator__ppu_read(nes, (uint16_t)(base + 0x0fcfu)),
                     chr_markers[fe_bank[half]]);
        CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[half], 1u);
        CHECK_EQ_HEX(nesturbator__ppu_read(nes, (uint16_t)(base + 0x0fdfu)),
                     chr_markers[fe_bank[half]]);
        CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[half], 0u);
        CHECK_EQ_HEX(nesturbator__ppu_read(nes, (uint16_t)(base + 0x0fe0u)),
                     chr_markers[fd_bank[half]]);
        CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[half], 1u);
        CHECK_EQ_HEX(nesturbator__ppu_read(nes, (uint16_t)(base + 0x0fefu)),
                     chr_markers[fe_bank[half]]);
        CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[half], 1u);
        force_latch(nes, half, 1u);
        CHECK_EQ_HEX(nesturbator__ppu_read(nes, (uint16_t)(base + 0x0ff0u)),
                     chr_markers[fd_bank[half]]);
        CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[half], 0u);
    }

    force_latch(nes, 0u, 0u);
    force_latch(nes, 1u, 0u);
    CHECK_EQ_HEX(nes->map.chr_r[3][0x3d0u], chr_markers[3]); /* side-effect-free page inspection */
    CHECK_EQ_HEX(nes->map.chr_r[7][0x3d0u], chr_markers[7]);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[0], 1u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[1], 1u);

    nesturbator__ppu_register_write(nes, 0x2006u, 0x0fu);
    nesturbator__ppu_register_write(nes, 0x2006u, 0xd0u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[0], 1u); /* address setup is not a read */
    nesturbator__ppu_write(nes, 0x0fd0u, 0xffu);
    nesturbator__ppu_write(nes, 0x1fd0u, 0xffu);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[0], 1u); /* CHR ROM writes are ignored */
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[1], 1u);
    (void)nesturbator__ppu_read(nes, 0x2000u);
    (void)nesturbator__ppu_read(nes, 0x3f00u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[0], 1u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[1], 1u);

    nesturbator__ppu_register_write(nes, 0x2006u, 0x20u);
    nesturbator__ppu_register_write(nes, 0x2006u, 0x00u);
    (void)nesturbator__ppu_register_read(nes, 0x2007u);
    nesturbator__ppu_register_write(nes, 0x2006u, 0x3fu);
    nesturbator__ppu_register_write(nes, 0x2006u, 0x00u);
    (void)nesturbator__ppu_register_read(nes, 0x2007u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[0], 1u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[1], 1u);
    nesturbator_destroy(inst);
}

static void test_all_chr_selector_values_and_write_order(void)
{
    static const uint16_t registers[4] = {0xbfffu, 0xcfffu, 0xdfffu, 0xefffu};
    static const uint8_t halves[4] = {0u, 0u, 1u, 1u};
    static const uint8_t positions[4] = {0u, 1u, 0u, 1u};
    struct mmc2_allocator_counts counts = {0u, 0u};
    nesturbator *inst = make_loaded_instance(&counts);
    CHECK(inst != NULL);
    if (inst == NULL)
        return;
    struct nesturbator *nes = (struct nesturbator *)inst;
    nes->ppu.mask = 0u;

    for (unsigned selector = 0u; selector < 4u; ++selector) {
        const unsigned half = halves[selector];
        const uint16_t base = (uint16_t)(half * 0x1000u);
        for (unsigned bank = 0u; bank < 32u; ++bank) {
            force_latch(nes, half, (uint8_t)(positions[selector] == 0u));
            nesturbator__bus_write(nes, registers[selector], (uint8_t)(0xe0u | bank));
            CHECK_EQ_U64(nes->mapper.reg.mmc2.chr[selector], bank);
            for (unsigned page = 0u; page < 4u; ++page) {
                const uint16_t page_addr = (uint16_t)(base + page * MMC2_PAGE_1K);
                CHECK_EQ_HEX(nesturbator__ppu_read(nes, page_addr), chr_markers[bank]);
                CHECK_EQ_HEX(nesturbator__ppu_read(nes, (uint16_t)(page_addr + 0x3ffu)),
                             chr_markers[bank]);
            }
        }
    }

    /* Active writes remap immediately; inactive writes wait until the latch selects them. */
    nesturbator__bus_write(nes, 0xb000u, 5u);
    nesturbator__bus_write(nes, 0xc000u, 3u);
    force_latch(nes, 0u, 1u);
    nesturbator__bus_write(nes, 0xc000u, 12u);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x0000u), chr_markers[5]);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x0fe0u), chr_markers[5]);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x0000u), chr_markers[12]);
    force_latch(nes, 0u, 0u);
    nesturbator__bus_write(nes, 0xb000u, 6u);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x0000u), chr_markers[12]);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x0fd0u), chr_markers[12]);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x0000u), chr_markers[6]);
    nesturbator_destroy(inst);
}

static void set_ppu_address(struct nesturbator *nes, uint16_t addr)
{
    nesturbator__ppu_register_write(nes, 0x2006u, (uint8_t)(addr >> 8));
    nesturbator__ppu_register_write(nes, 0x2006u, (uint8_t)addr);
}

static void test_buffered_ppudata_reads_trigger_both_halves(void)
{
    struct mmc2_allocator_counts counts = {0u, 0u};
    nesturbator *inst = make_loaded_instance(&counts);
    CHECK(inst != NULL);
    if (inst == NULL)
        return;
    struct nesturbator *nes = (struct nesturbator *)inst;
    select_chr_banks(nes);
    nes->ppu.mask = 0u;

    nes->ppu.read_buffer = 0x44u;
    set_ppu_address(nes, 0x0fd0u);
    CHECK_EQ_HEX(nesturbator__ppu_register_read(nes, 0x2007u), 0x44u);
    CHECK_EQ_HEX(nes->ppu.read_buffer, 0xa3u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[0], 0u);
    set_ppu_address(nes, 0x0000u);
    CHECK_EQ_HEX(nesturbator__ppu_register_read(nes, 0x2007u), 0xa3u);
    CHECK_EQ_HEX(nes->ppu.read_buffer, 0xa5u);

    nes->ppu.read_buffer = 0x55u;
    set_ppu_address(nes, 0x1fd0u);
    CHECK_EQ_HEX(nesturbator__ppu_register_read(nes, 0x2007u), 0x55u);
    CHECK_EQ_HEX(nes->ppu.read_buffer, 0xa7u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[1], 0u);
    set_ppu_address(nes, 0x1fe0u);
    CHECK_EQ_HEX(nesturbator__ppu_register_read(nes, 0x2007u), 0xa7u);
    CHECK_EQ_HEX(nes->ppu.read_buffer, 0xa9u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[1], 1u);
    set_ppu_address(nes, 0x1000u);
    CHECK_EQ_HEX(nesturbator__ppu_register_read(nes, 0x2007u), 0xa9u);
    CHECK_EQ_HEX(nes->ppu.read_buffer, 0xa7u);
    nesturbator_destroy(inst);
}

static void test_renderer_reads_old_then_new_chr_bank(void)
{
    static uint16_t video[NESTURBATOR_WIDTH * NESTURBATOR_HEIGHT];
    struct mmc2_allocator_counts counts = {0u, 0u};
    nesturbator *inst = make_loaded_instance(&counts);
    CHECK(inst != NULL);
    if (inst == NULL)
        return;
    struct nesturbator *nes = (struct nesturbator *)inst;
    nes->map.ops.cpu_write(nes, 0xb000u, 5u, nes->cpu_cycle);
    nes->map.ops.cpu_write(nes, 0xc000u, 3u, nes->cpu_cycle);
    memset(video, 0, sizeof video);
    memset(nes->ppu.nametable, 0, sizeof nes->ppu.nametable);
    nesturbator__ppu_write(nes, 0x2000u, 0xfdu);
    nes->ppu.palette[3] = 0x15u;
    nes->ppu.control = 0u;
    nes->ppu.mask = 0x0au; /* background and its leftmost eight pixels */
    nes->ppu.reset_flag = 0u;
    nes->ppu.scanline = 0u;
    nes->ppu.dot = 0u;
    nes->ppu.ppu_ticks = 0u;
    nes->ppu.v = 0u;
    nes->ppu.video_output = video;
    nes->ppu.video_pitch = NESTURBATOR_WIDTH;

    nesturbator__ppu_run_until(nes, 6u * 8u);
    CHECK_EQ_HEX(nes->ppu.bg_lo, 0xa3u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[0], 0u);
    nesturbator__ppu_run_until(nes, 8u * 8u);
    CHECK_EQ_HEX(nes->ppu.bg_hi, 0xa5u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[0], 0u);
    nesturbator__ppu_run_until(nes, 17u * 8u);
    CHECK_EQ_HEX(video[16u], 0x15u); /* marker bits 7/7 select palette colour 3 */
    nesturbator_destroy(inst);
}

static void test_latch_and_selector_state_is_per_instance_and_survives_reset(void)
{
    struct mmc2_allocator_counts counts = {0u, 0u};
    nesturbator *inst = make_loaded_instance(&counts);
    nesturbator *peer = make_loaded_instance(&counts);
    CHECK(inst != NULL && peer != NULL);
    if (inst == NULL || peer == NULL) {
        nesturbator_destroy(inst);
        nesturbator_destroy(peer);
        return;
    }
    struct nesturbator *nes = (struct nesturbator *)inst;
    struct nesturbator *other = (struct nesturbator *)peer;
    select_chr_banks(nes);
    nesturbator__bus_write(other, 0xb000u, 12u);
    nesturbator__bus_write(other, 0xc000u, 14u);
    nesturbator__bus_write(other, 0xd000u, 15u);
    nesturbator__bus_write(other, 0xe000u, 11u);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x0fd0u), chr_markers[3]);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[0], 0u);
    CHECK_EQ_U64(other->mapper.reg.mmc2.latch[0], 1u);
    CHECK_EQ_HEX(nesturbator__ppu_read(other, 0x0000u), chr_markers[14]);
    CHECK_EQ_HEX(nesturbator__ppu_read(other, 0x1fd0u), chr_markers[11]);
    CHECK_EQ_U64(other->mapper.reg.mmc2.latch[1], 0u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[1], 1u);

    const struct nesturbator__mapper saved = nes->mapper;
    CHECK_EQ_U64(nesturbator_reset(inst), NESTURBATOR_OK);
    CHECK(memcmp(&saved, &nes->mapper, sizeof saved) == 0);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[0], 0u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[1], 1u);
    nesturbator_destroy(inst);
    nesturbator_destroy(peer);
}

static void mmc2_set_state_registers(struct nesturbator *nes)
{
    /* These are production bus writes, not direct edits to the serialized mapper fields. */
    nesturbator__bus_write(nes, 0xa000u, 12u);
    nesturbator__bus_write(nes, 0xb000u, 5u);
    nesturbator__bus_write(nes, 0xc000u, 3u);
    nesturbator__bus_write(nes, 0xd000u, 9u);
    nesturbator__bus_write(nes, 0xe000u, 7u);
    nesturbator__bus_write(nes, 0xf000u, 1u);
}

static int mmc2_state_save(nesturbator *inst, uint8_t *state, size_t size)
{
    const nesturbator_status status = nesturbator_save_state(inst, state, size);
    CHECK_EQ_U64(status, NESTURBATOR_OK);
    return status == NESTURBATOR_OK;
}

static void mmc2_prepare_uninterrupted_reference(nesturbator *inst, unsigned state_index)
{
    struct nesturbator *nes = (struct nesturbator *)inst;
    CHECK_EQ_U64(st_frame(inst, 0u, NULL, NULL, NULL, NULL), NESTURBATOR_OK);
    mmc2_set_state_registers(nes);
    force_latch(nes, 0u, 0u);
    force_latch(nes, 1u, 0u);
    nesturbator__ppu_write(nes, 0x2000u, 0x41u);
    nesturbator__ppu_write(nes, 0x2400u, 0x42u);
    nesturbator__ppu_write(nes, 0x2800u, 0x43u);
    nesturbator__ppu_write(nes, 0x2c00u, 0x44u);
    if (state_index >= 1u)
        force_latch(nes, 0u, 1u);
    if (state_index >= 2u)
        force_latch(nes, 1u, 1u);
    if (state_index >= 3u)
        force_latch(nes, 0u, 0u);
    if (state_index >= 4u)
        force_latch(nes, 1u, 0u);
}

static void mmc2_check_state_mapper_bytes(const nesturbator *inst, const uint8_t *state,
                                          uint8_t latch0, uint8_t latch1)
{
    static const uint8_t maxima[8] = {15u, 31u, 31u, 31u, 31u, 1u, 1u, 1u};
    const uint8_t values[8] = {12u, 5u, 3u, 9u, 7u, 1u, latch0, latch1};
    struct nesturbator__state_bound bounds[512];
    const size_t count = nesturbator__state_bounds((const struct nesturbator *)inst, bounds,
                                                   sizeof bounds / sizeof bounds[0]);
    CHECK(count >= 8u && count <= sizeof bounds / sizeof bounds[0]);
    if (count < 8u || count > sizeof bounds / sizeof bounds[0])
        return;
    for (size_t i = 0u; i < 8u; ++i) {
        const struct nesturbator__state_bound *bound = &bounds[count - 8u + i];
        CHECK_EQ_U64(bound->width, 1u);
        CHECK_EQ_U64(bound->hi, maxima[i]);
        CHECK_EQ_U64(bound->offset, bounds[count - 8u].offset + i);
        CHECK_EQ_U64(state[bound->offset], values[i]);
    }
}

static void mmc2_check_restored_markers(nesturbator *inst, unsigned latch0, unsigned latch1)
{
    static const uint8_t nt_horizontal[] = {0u, 0u, 1u, 1u};
    struct nesturbator *nes = (struct nesturbator *)inst;
    const uint8_t low_bank = latch0 != 0u ? 3u : 5u;
    const uint8_t high_bank = latch1 != 0u ? 7u : 9u;
    CHECK_EQ_U64(nes->mapper.reg.mmc2.prg, 12u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.chr[0], 5u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.chr[1], 3u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.chr[2], 9u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.chr[3], 7u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.mirror, 1u);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[0], latch0);
    CHECK_EQ_U64(nes->mapper.reg.mmc2.latch[1], latch1);
    CHECK(memcmp(nes->map.nt, nt_horizontal, sizeof nt_horizontal) == 0);
    CHECK_EQ_HEX(nesturbator__map_cpu_read(nes, 0x8000u), prg_markers[12][0]);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x0020u), chr_markers[low_bank]);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x1020u), chr_markers[high_bank]);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x2000u), 0x42u);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x2400u), 0x42u);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x2800u), 0x44u);
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x2c00u), 0x44u);
}

static void test_mapper_state_restore_and_transactional_rejection(void)
{
    static uint8_t image[MMC2_FIXTURE_CAP];
    static uint8_t other_image[MMC2_FIXTURE_CAP];
    static const uint8_t latch_pairs[5][2] = {{1u, 1u}, {0u, 1u}, {0u, 0u}, {1u, 0u}, {1u, 1u}};
    static const unsigned frame_numbers[] = {2u, 3u, 4u, 5u};
    struct mmc2_allocator_counts counts = {0u, 0u};
    const size_t image_size = mmc2_fixture_build(image, sizeof image, 1u);
    nesturbator *source = make_instance(&counts);
    nesturbator *restored = make_instance(&counts);
    nesturbator *peer = make_instance(&counts);
    nesturbator *identity_mismatch = make_instance(&counts);
    uint8_t *states = NULL;
    uint8_t *saved_a = NULL;
    uint8_t *saved_b = NULL;
    uint8_t *mutated = NULL;
    uint32_t expected[4][4] = {{0u}};
    uint64_t expected_ticks[4] = {0u};
    uint64_t expected_frames[4] = {0u};
    size_t state_size = 0u;

    CHECK_EQ_U64(image_size, MMC2_FIXTURE_CAP);
    CHECK(source != NULL && restored != NULL && peer != NULL && identity_mismatch != NULL);
    if (source == NULL || restored == NULL || peer == NULL || identity_mismatch == NULL)
        goto done;
    CHECK_EQ_U64(nesturbator_load_cartridge(source, image, image_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge(restored, image, image_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge(peer, image, image_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge(identity_mismatch, image, image_size), NESTURBATOR_OK);
    state_size = nesturbator_state_size(source);
    CHECK(state_size > 64u);
    states = (uint8_t *)malloc(state_size * 5u);
    saved_a = (uint8_t *)malloc(state_size);
    saved_b = (uint8_t *)malloc(state_size);
    mutated = (uint8_t *)malloc(state_size + 1u);
    CHECK(states != NULL && saved_a != NULL && saved_b != NULL && mutated != NULL);
    if (state_size <= 64u || states == NULL || saved_a == NULL || saved_b == NULL ||
        mutated == NULL)
        goto done;

    /* Start from a real frame boundary, then establish distinct hardware-visible registers. */
    CHECK_EQ_U64(st_frame(source, 0u, NULL, NULL, NULL, NULL), NESTURBATOR_OK);
    CHECK_EQ_U64(st_frame(restored, 0u, NULL, NULL, NULL, NULL), NESTURBATOR_OK);
    CHECK_EQ_U64(st_frame(restored, 0u, NULL, NULL, NULL, NULL), NESTURBATOR_OK);
    mmc2_set_state_registers((struct nesturbator *)source);
    force_latch((struct nesturbator *)source, 0u, 0u);
    force_latch((struct nesturbator *)source, 1u, 0u);
    {
        struct nesturbator *nes = (struct nesturbator *)source;
        nesturbator__ppu_write(nes, 0x2000u, 0x41u);
        nesturbator__ppu_write(nes, 0x2400u, 0x42u);
        nesturbator__ppu_write(nes, 0x2800u, 0x43u);
        nesturbator__ppu_write(nes, 0x2c00u, 0x44u);
    }

    /* State 0 is saved immediately before any latch transition. The remaining states follow an
       actual PPU pattern read each, covering every joint FD/FE combination and read boundary. */
    CHECK(mmc2_state_save(source, states, state_size));
    mmc2_check_state_mapper_bytes(source, states, 1u, 1u);
    force_latch((struct nesturbator *)source, 0u, 1u);
    CHECK(mmc2_state_save(source, states + state_size, state_size));
    force_latch((struct nesturbator *)source, 1u, 1u);
    CHECK(mmc2_state_save(source, states + 2u * state_size, state_size));
    force_latch((struct nesturbator *)source, 0u, 0u);
    CHECK(mmc2_state_save(source, states + 3u * state_size, state_size));
    force_latch((struct nesturbator *)source, 1u, 0u);
    CHECK(mmc2_state_save(source, states + 4u * state_size, state_size));
    for (size_t state_index = 0u; state_index < 5u; ++state_index) {
        const uint8_t *state = states + state_index * state_size;
        mmc2_check_state_mapper_bytes(source, state, latch_pairs[state_index][0],
                                      latch_pairs[state_index][1]);
        CHECK_EQ_U64(state[8], NESTURBATOR_STATE_FORMAT_VERSION);
    }

    /* Make the destination's actual derived mappings different before poisoning state fields. */
    {
        struct nesturbator *dest = (struct nesturbator *)restored;
        mmc2_set_state_registers(dest);
        nesturbator__bus_write(dest, 0xa000u, 2u);
        nesturbator__bus_write(dest, 0xb000u, 10u);
        nesturbator__bus_write(dest, 0xc000u, 11u);
        nesturbator__bus_write(dest, 0xd000u, 12u);
        nesturbator__bus_write(dest, 0xe000u, 13u);
        nesturbator__bus_write(dest, 0xf000u, 0u);
        force_latch(dest, 0u, 1u);
        force_latch(dest, 1u, 1u);
    }
    st_poison(restored);

    for (size_t state_index = 0u; state_index < 5u; ++state_index) {
        const uint8_t *state = states + state_index * state_size;
        nesturbator *reference = make_instance(&counts);
        CHECK(reference != NULL);
        if (reference == NULL)
            goto done;
        CHECK_EQ_U64(nesturbator_load_cartridge(reference, image, image_size), NESTURBATOR_OK);
        mmc2_prepare_uninterrupted_reference(reference, (unsigned)state_index);
        CHECK(mmc2_state_save(reference, saved_a, state_size));
        CHECK(memcmp(saved_a, state, state_size) == 0);

        /* Perturb the candidate's selectors and mappings before every restore, then poison its
           mutable fields so the saved visitor and mapper rebuild are the only recovery path. */
        {
            struct nesturbator *dest = (struct nesturbator *)restored;
            nesturbator__bus_write(dest, 0xa000u, 2u);
            nesturbator__bus_write(dest, 0xb000u, 10u);
            nesturbator__bus_write(dest, 0xc000u, 11u);
            nesturbator__bus_write(dest, 0xd000u, 12u);
            nesturbator__bus_write(dest, 0xe000u, 13u);
            nesturbator__bus_write(dest, 0xf000u, 0u);
            force_latch(dest, 0u, 1u);
            force_latch(dest, 1u, 1u);
            st_poison(restored);
        }
        CHECK_EQ_U64(nesturbator_load_state(restored, state, state_size), NESTURBATOR_OK);
        CHECK(mmc2_state_save(restored, saved_b, state_size));
        CHECK(memcmp(saved_b, state, state_size) == 0);
        mmc2_check_restored_markers(reference, latch_pairs[state_index][0],
                                    latch_pairs[state_index][1]);
        mmc2_check_restored_markers(restored, latch_pairs[state_index][0],
                                    latch_pairs[state_index][1]);

        for (unsigned repeat = 0u; repeat < 2u; ++repeat) {
            if (repeat != 0u) {
                nesturbator_destroy(reference);
                reference = make_instance(&counts);
                CHECK(reference != NULL);
                if (reference == NULL)
                    goto done;
                CHECK_EQ_U64(nesturbator_load_cartridge(reference, image, image_size),
                             NESTURBATOR_OK);
                mmc2_prepare_uninterrupted_reference(reference, (unsigned)state_index);
                CHECK(mmc2_state_save(reference, saved_a, state_size));
                CHECK(memcmp(saved_a, state, state_size) == 0);
                {
                    struct nesturbator *dest = (struct nesturbator *)restored;
                    nesturbator__bus_write(dest, 0xa000u, 2u);
                    nesturbator__bus_write(dest, 0xb000u, 10u);
                    nesturbator__bus_write(dest, 0xc000u, 11u);
                    nesturbator__bus_write(dest, 0xd000u, 12u);
                    nesturbator__bus_write(dest, 0xe000u, 13u);
                    nesturbator__bus_write(dest, 0xf000u, 0u);
                    force_latch(dest, 0u, 1u);
                    force_latch(dest, 1u, 1u);
                }
                st_poison(restored);
                CHECK_EQ_U64(nesturbator_load_state(restored, state, state_size), NESTURBATOR_OK);
                CHECK(mmc2_state_save(restored, saved_b, state_size));
                CHECK(memcmp(saved_b, state, state_size) == 0);
            }
            for (size_t frame = 0u; frame < 4u; ++frame) {
                uint32_t a_video = 0u, a_audio = 0u, b_video = 0u, b_audio = 0u;
                uint64_t a_ticks = 0u, a_frame = 0u, b_ticks = 0u, b_frame = 0u;
                CHECK_EQ_U64(st_frame(reference, st_buttons(frame_numbers[frame]), &a_video,
                                      &a_audio, &a_ticks, &a_frame),
                             NESTURBATOR_OK);
                if (frame == 1u && repeat == 0u) {
                    struct nesturbator *independent = (struct nesturbator *)peer;
                    nesturbator__bus_write(independent, 0xa000u, 4u);
                    nesturbator__bus_write(independent, 0xb000u, 17u);
                    force_latch(independent, 0u, 1u);
                    (void)nesturbator__ppu_read(independent, 0x1fd0u);
                }
                CHECK_EQ_U64(st_frame(restored, st_buttons(frame_numbers[frame]), &b_video,
                                      &b_audio, &b_ticks, &b_frame),
                             NESTURBATOR_OK);
                CHECK_EQ_U64(a_video, b_video);
                CHECK_EQ_U64(a_audio, b_audio);
                CHECK_EQ_U64(a_ticks, b_ticks);
                CHECK_EQ_U64(a_frame, b_frame);
                if (repeat == 0u) {
                    expected[frame][0] = a_video;
                    expected[frame][1] = a_audio;
                    expected_ticks[frame] = a_ticks;
                    expected_frames[frame] = a_frame;
                } else {
                    CHECK_EQ_U64(a_video, expected[frame][0]);
                    CHECK_EQ_U64(a_audio, expected[frame][1]);
                    CHECK_EQ_U64(a_ticks, expected_ticks[frame]);
                    CHECK_EQ_U64(a_frame, expected_frames[frame]);
                }
            }
            CHECK(mmc2_state_save(reference, saved_a, state_size));
            CHECK(mmc2_state_save(restored, saved_b, state_size));
            CHECK(memcmp(saved_a, saved_b, state_size) == 0);
            if (repeat == 0u)
                memcpy(mutated, saved_a, state_size);
        }
        nesturbator_destroy(reference);
    }

    /* Every mapper-owned byte is range-checked with a recomputed checksum, and refusal is fully
       transactional for the live destination. The last eight bounded fields are the mapper-9
       visitor's documented order, so this test does not maintain a second offset table. */
    {
        struct nesturbator__state_bound bounds[512];
        const size_t count = nesturbator__state_bounds((const struct nesturbator *)source, bounds,
                                                       sizeof bounds / sizeof bounds[0]);
        CHECK(count >= 8u && count <= sizeof bounds / sizeof bounds[0]);
        if (count >= 8u && count <= sizeof bounds / sizeof bounds[0]) {
            for (size_t field = count - 8u; field < count; ++field) {
                const size_t offset = bounds[field].offset;
                memcpy(mutated, states, state_size);
                mutated[offset] = (uint8_t)(bounds[field].hi + 1);
                st_recrc(mutated, state_size);
                CHECK(st_refused(restored, mutated, state_size, NESTURBATOR_ERR_STATE_INVALID));
            }
        }
    }
    for (uint16_t version = 5u; version <= 8u; ++version) {
        if (version == NESTURBATOR_STATE_FORMAT_VERSION)
            continue;
        memcpy(mutated, states, state_size);
        mutated[8] = (uint8_t)version;
        mutated[9] = (uint8_t)(version >> 8);
        CHECK(st_refused(restored, mutated, state_size, NESTURBATOR_ERR_STATE_FORMAT));
    }
    CHECK(st_refused(restored, states, state_size - 1u, NESTURBATOR_ERR_STATE_SIZE));
    memcpy(mutated, states, state_size);
    mutated[state_size] = 0u;
    CHECK(st_refused(restored, mutated, state_size + 1u, NESTURBATOR_ERR_STATE_SIZE));

    memcpy(other_image, image, image_size);
    other_image[MMC2_FIXTURE_CAP - 1u] ^= 1u;
    CHECK_EQ_U64(nesturbator_load_cartridge(identity_mismatch, other_image, image_size),
                 NESTURBATOR_OK);
    CHECK(st_refused(identity_mismatch, states, state_size, NESTURBATOR_ERR_STATE_CARTRIDGE));

done:
    free(states);
    free(saved_a);
    free(saved_b);
    free(mutated);
    nesturbator_destroy(source);
    nesturbator_destroy(restored);
    nesturbator_destroy(peer);
    nesturbator_destroy(identity_mismatch);
}

int main(void)
{
    test_profile_refusals_are_transactional();
    test_prg_banks_registers_and_mirroring();
    test_exponent_profile_encoding_and_ntsc_refusals();
    test_pattern_read_returns_old_bank_then_updates_latch();
    test_unwatched_mapper_keeps_direct_chr_read_path();
    test_all_full_row_trigger_offsets();
    test_trigger_boundaries_and_non_pattern_accesses();
    test_all_chr_selector_values_and_write_order();
    test_buffered_ppudata_reads_trigger_both_halves();
    test_renderer_reads_old_then_new_chr_bank();
    test_latch_and_selector_state_is_per_instance_and_survives_reset();
    test_mapper_state_restore_and_transactional_rejection();
    CHECK_DONE();
}
