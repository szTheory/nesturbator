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

int main(void)
{
    test_tracer_bank_switch_through_cpu();
    CHECK_DONE();
}
