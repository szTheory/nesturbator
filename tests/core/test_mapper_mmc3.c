/* BOARD-03: the MMC3 board on synthetic images. One function per behaviour. Every code byte is
   authored here and placed with ines_spec segments. PRG bank b of 8 KiB starts at b * BANK_8K and
   its first byte (or the byte after the code) is b, so a wrong bank is visible. */
#include <stdint.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"

#define BANK_8K 8192u
#define PRG_16K_UNITS 8u /* 128 KiB of PRG: sixteen 8 KiB banks */
#define IMAGE_CAP (16u + 64u * INES_PRG_BANK + 32u * INES_CHR_BANK)

static nesturbator *make_instance(void)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
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

/* An NES 2.0 mapper 4 image: prg_16k units of PRG, chr_8k units of CHR ROM (0 is CHR-RAM), the
   RAM nibbles and the submapper. Each 8 KiB bank's byte at offset 0x20 is its bank number; the
   optional code is placed at offset 0 of the last bank, where the reset vector $E000 points. */
static nesturbator *load_board(uint8_t prg_16k, uint8_t chr_8k, uint8_t submapper,
                               uint8_t ram_shift, uint8_t nvram_shift, uint8_t battery,
                               const uint8_t *code, size_t code_len)
{
    static uint8_t image[IMAGE_CAP];
    static uint8_t ids[64];
    static struct ines_segment segs[65];
    struct ines_spec spec;
    nesturbator *inst = make_instance();
    const uint32_t banks = (uint32_t)prg_16k * 2u;
    size_t size, n = 0u;
    for (uint32_t b = 0u; b < banks; ++b) {
        ids[b] = (uint8_t)b;
        segs[n].prg_offset = b * BANK_8K + 0x20u;
        segs[n].bytes = &ids[b];
        segs[n].len = 1u;
        n++;
    }
    if (code != NULL) {
        segs[n].prg_offset = (banks - 1u) * BANK_8K;
        segs[n].bytes = code;
        segs[n].len = code_len;
        n++;
    }
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = prg_16k;
    spec.chr_8k = chr_8k;
    spec.nes2 = 1u;
    spec.mapper = 4u;
    spec.submapper = submapper;
    spec.battery = battery;
    spec.prg_ram_shift = ram_shift;
    spec.prg_nvram_shift = nvram_shift;
    spec.segments = segs;
    spec.segment_count = n;
    spec.reset_vector = 0xe000u;
    size = ines_build(image, sizeof image, &spec);
    CHECK(size != 0u);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    return inst;
}

/* The byte a CPU read at addr sees through the page tables. */
static uint8_t peek(struct nesturbator *nes, uint16_t addr)
{
    return nesturbator__map_cpu_read(nes, addr);
}

/* Tracer: a 128 KiB board with 8 KiB of battery RAM. The reset code in the last bank writes 6 to
   $8000 and 3 to $8001, reads bank 3's marker through $8000 and stores it at $0010. */
static void test_tracer_bank_switch_through_cpu(void)
{
    /* LDA #$06; STA $8000; LDA #$03; STA $8001; LDA $8020; STA $0010; JMP $E011 (spin) */
    static const uint8_t boot[] = {0xa9u, 0x06u, 0x8du, 0x00u, 0x80u, 0xa9u, 0x03u, 0x8du,
                                   0x01u, 0x80u, 0xadu, 0x20u, 0x80u, 0x8du, 0x10u, 0x00u,
                                   0x4cu, 0x10u, 0xe0u};
    nesturbator *inst = load_board(PRG_16K_UNITS, 1u, 0u, 0u, 7u, 1u, boot, sizeof boot);
    struct nesturbator *nes = inst;
    /* Power-on: $8000 is bank 0, $A000 bank 0, $C000 the second-last, $E000 the last. */
    CHECK_EQ_HEX(peek(nes, 0xc020u), 14u);
    CHECK_EQ_HEX(peek(nes, 0xe020u), 15u);
    run_frame(inst);
    CHECK_EQ_U64(nes->mapper.reg.mmc3.select, 6u);
    CHECK_EQ_U64(nes->mapper.reg.mmc3.r[6], 3u);
    CHECK_EQ_HEX(nes->bus.ram[0x10], 3u);
    CHECK_EQ_HEX(peek(nes, 0x8020u), 3u);
    nesturbator_destroy(inst);
}

/* A write straight into the hook, as the bus would deliver it. */
static void hook(struct nesturbator *nes, uint16_t addr, uint8_t value)
{
    nes->map.ops.cpu_write(nes, addr, value, nes->cpu_cycle);
}

/* Select register sel, then write value to it. */
static void set_reg(struct nesturbator *nes, uint8_t sel, uint8_t value)
{
    hook(nes, 0x8000u, sel);
    hook(nes, 0x8001u, value);
}

/* The 8 KiB PRG window (0 is $8000) must show bank. */
static void check_window(struct nesturbator *nes, unsigned window, uint32_t bank, int line)
{
    if (nes->map.cpu_r[16u + window * 8u] != nes->cart.prg + (size_t)bank * BANK_8K) {
        fprintf(stderr, "line %d: window %u is not bank %u\n", line, window, (unsigned)bank);
        check_failures++;
    }
}
#define WIN(nes, w, b) check_window((nes), (w), (b), __LINE__)

/* The 1 KiB CHR page must show bank_1k. */
static void check_chr(struct nesturbator *nes, unsigned page, uint32_t bank_1k, int line)
{
    if (nes->map.chr_r[page] != nes->cart.chr + (size_t)bank_1k * 1024u) {
        fprintf(stderr, "line %d: chr page %u is not bank %u\n", line, page, (unsigned)bank_1k);
        check_failures++;
    }
}
#define CHR(nes, p, b) check_chr((nes), (p), (b), __LINE__)

/* D-07: both PRG modes, with R6/R7 high bits ignored and $8000 bit 5 without effect. */
static void test_prg_modes(void)
{
    nesturbator *inst = load_board(PRG_16K_UNITS, 4u, 0u, 0u, 7u, 1u, NULL, 0u);
    struct nesturbator *nes = inst;
    set_reg(nes, 6u, 0xc5u); /* R6 = 5 once bits 7-6 are dropped */
    set_reg(nes, 7u, 0xc9u); /* R7 = 9 */
    WIN(nes, 0u, 5u);
    WIN(nes, 1u, 9u);
    WIN(nes, 2u, 14u);
    WIN(nes, 3u, 15u);
    hook(nes, 0x8000u, 0x46u); /* mode 1 */
    WIN(nes, 0u, 14u);
    WIN(nes, 1u, 9u);
    WIN(nes, 2u, 5u);
    WIN(nes, 3u, 15u);
    hook(nes, 0x8000u, 0x26u); /* bit 5 only: mode 0 again */
    WIN(nes, 0u, 5u);
    WIN(nes, 2u, 14u);
    nesturbator_destroy(inst);
}

/* D-07: CHR registers in both inversions; R0/R1 ignore bit 0. */
static void test_chr_banks(void)
{
    nesturbator *inst = load_board(PRG_16K_UNITS, 4u, 0u, 0u, 7u, 1u, NULL, 0u);
    struct nesturbator *nes = inst;
    set_reg(nes, 0u, 0x05u); /* 2 KiB bank, 4 */
    set_reg(nes, 1u, 0x0bu); /* 10 */
    set_reg(nes, 2u, 20u);
    set_reg(nes, 3u, 21u);
    set_reg(nes, 4u, 22u);
    set_reg(nes, 5u, 23u);
    hook(nes, 0x8000u, 0x00u);
    CHR(nes, 0u, 4u);
    CHR(nes, 1u, 5u);
    CHR(nes, 2u, 10u);
    CHR(nes, 3u, 11u);
    CHR(nes, 4u, 20u);
    CHR(nes, 5u, 21u);
    CHR(nes, 6u, 22u);
    CHR(nes, 7u, 23u);
    hook(nes, 0x8000u, 0x80u);
    CHR(nes, 0u, 20u);
    CHR(nes, 1u, 21u);
    CHR(nes, 2u, 22u);
    CHR(nes, 3u, 23u);
    CHR(nes, 4u, 4u);
    CHR(nes, 5u, 5u);
    CHR(nes, 6u, 10u);
    CHR(nes, 7u, 11u);
    /* CHR ROM is never writable. */
    CHECK(nes->map.chr_w[0] == NULL);
    nesturbator_destroy(inst);
}

/* D-07: writes mirror across each 8 KiB range, even to the select/data pair. */
static void test_even_odd_decode(void)
{
    nesturbator *inst = load_board(PRG_16K_UNITS, 4u, 0u, 0u, 7u, 1u, NULL, 0u);
    struct nesturbator *nes = inst;
    hook(nes, 0x9ffeu, 0x07u); /* acts as $8000 */
    hook(nes, 0x9fffu, 0x0cu); /* acts as $8001: R7 = 12 */
    WIN(nes, 1u, 12u);
    hook(nes, 0xbfffu, 0x40u); /* acts as $A001: protected */
    CHECK(nes->map.cpu_w[8] == NULL);
    hook(nes, 0xbffeu, 0x01u); /* acts as $A000: horizontal */
    CHECK_EQ_U64(nes->map.nt[1], 0u);
    CHECK_EQ_U64(nes->map.nt[2], 1u);
    nesturbator_destroy(inst);
}

/* D-07, PITFALLS pitfall 8: $A000 bit 0 clear is vertical mirroring (a horizontal arrangement of
   screens: nametables 0,1,0,1); set is horizontal mirroring (a vertical arrangement: 0,0,1,1). */
static void test_mirroring_polarity(void)
{
    nesturbator *inst = load_board(PRG_16K_UNITS, 4u, 0u, 0u, 7u, 1u, NULL, 0u);
    struct nesturbator *nes = inst;
    const uint8_t vertical[4] = {0u, 1u, 0u, 1u};
    const uint8_t horizontal[4] = {0u, 0u, 1u, 1u};
    CHECK(memcmp(nes->map.nt, vertical, 4u) == 0); /* power-on */
    hook(nes, 0xa000u, 0x01u);
    CHECK(memcmp(nes->map.nt, horizontal, 4u) == 0);
    hook(nes, 0xa000u, 0xfeu); /* only bit 0 counts */
    CHECK(memcmp(nes->map.nt, vertical, 4u) == 0);
    hook(nes, 0xa000u, 0xffu);
    CHECK(memcmp(nes->map.nt, horizontal, 4u) == 0);
    nesturbator_destroy(inst);
}

/* Edge: a 16 KiB PRG boots with both fixed windows aliasing bank 0 and bank 1 by modulo. */
static void test_16k_prg_aliases(void)
{
    nesturbator *inst = load_board(1u, 1u, 0u, 0u, 7u, 1u, NULL, 0u);
    struct nesturbator *nes = inst;
    WIN(nes, 0u, 0u);
    WIN(nes, 1u, 0u);
    WIN(nes, 2u, 0u); /* second-last */
    WIN(nes, 3u, 1u); /* last */
    set_reg(nes, 6u, 3u);
    WIN(nes, 0u, 1u); /* 3 mod 2 */
    set_reg(nes, 7u, 2u);
    WIN(nes, 1u, 0u);
    nesturbator_destroy(inst);
}

/* D-03, D-07, D-13: $A001 enable and protect through NULL pages, and the save counter. */
static void test_ram_gating(void)
{
    nesturbator *inst = load_board(PRG_16K_UNITS, 4u, 0u, 0u, 7u, 1u, NULL, 0u);
    struct nesturbator *nes = inst;
    /* Power-on, no $A001 write: enabled and writable (D-08). */
    CHECK_EQ_U64(nesturbator_save_generation(inst), 0u);
    nesturbator__bus_write(nes, 0x6000u, 0x11u);
    CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x6000u), 0x11u);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 1u);
    /* Protected: reads, drops writes, bits 5-0 ignored. */
    nesturbator__bus_write(nes, 0xa001u, 0xffu);
    nesturbator__bus_write(nes, 0x6000u, 0x22u);
    CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x6000u), 0x11u);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 1u);
    /* Disabled: open bus, dropped writes. */
    nesturbator__bus_write(nes, 0xa001u, 0x00u);
    nes->bus.open_bus = 0xabu;
    CHECK_EQ_HEX(peek(nes, 0x6000u), 0xabu);
    nesturbator__bus_write(nes, 0x6000u, 0x33u);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 1u);
    /* Bits 5-0 have no effect: $80 alone enables and unprotects; the byte is still 0x11. */
    nesturbator__bus_write(nes, 0xa001u, 0x80u);
    CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x6000u), 0x11u);
    nesturbator__bus_write(nes, 0xa001u, 0xbfu);
    nesturbator__bus_write(nes, 0x6001u, 0x44u);
    CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x6001u), 0x44u);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 2u);
    /* $E000 leaves the RAM alone (D-03). */
    nesturbator__bus_write(nes, 0xe000u, 0x00u);
    CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x6000u), 0x11u);
    CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x6001u), 0x44u);
    nesturbator_destroy(inst);
}

/* Edge: without declared RAM $6000 is open bus whatever $A001 holds. */
static void test_no_ram_is_open_bus(void)
{
    nesturbator *inst = load_board(PRG_16K_UNITS, 4u, 0u, 0u, 0u, 0u, NULL, 0u);
    struct nesturbator *nes = inst;
    CHECK(nes->cart.prg_ram == NULL);
    CHECK(nes->map.cpu_r[8] == NULL);
    hook(nes, 0xa001u, 0x80u);
    CHECK(nes->map.cpu_r[8] == NULL);
    CHECK(nes->map.cpu_w[8] == NULL);
    nesturbator_destroy(inst);
}

/* D-08: a zeroed block is power-on, and nesturbator_reset keeps every field. */
static void test_power_on_and_reset(void)
{
    nesturbator *inst = load_board(PRG_16K_UNITS, 4u, 0u, 0u, 7u, 1u, NULL, 0u);
    struct nesturbator *nes = inst;
    struct nesturbator__mapper before;
    CHECK_EQ_U64(nes->mapper.reg.mmc3.select, 0u);
    CHECK_EQ_U64(nes->mapper.reg.mmc3.irq_enable, 0u);
    CHECK_EQ_U64(nes->mapper.irq, 0u);
    WIN(nes, 0u, 0u);
    WIN(nes, 2u, 14u);
    WIN(nes, 3u, 15u);
    CHECK(nes->map.cpu_r[8] == nes->cart.prg_ram);
    CHECK(nes->map.cpu_w[8] == nes->cart.prg_ram);
    set_reg(nes, 6u, 7u);
    hook(nes, 0xa000u, 0x01u);
    hook(nes, 0xa001u, 0xc0u);
    hook(nes, 0xc000u, 0x33u);
    memcpy(&before, &nes->mapper, sizeof before);
    CHECK_EQ_U64(nesturbator_reset(inst), NESTURBATOR_OK);
    CHECK(memcmp(&before, &nes->mapper, sizeof before) == 0);
    WIN(nes, 0u, 7u);
    CHECK(nes->map.cpu_w[8] == NULL);
    nesturbator_destroy(inst);
}

int main(void)
{
    test_tracer_bank_switch_through_cpu();
    test_prg_modes();
    test_chr_banks();
    test_even_odd_decode();
    test_mirroring_polarity();
    test_16k_prg_aliases();
    test_ram_gating();
    test_no_ram_is_open_bus();
    test_power_on_and_reset();
    CHECK_DONE();
}
