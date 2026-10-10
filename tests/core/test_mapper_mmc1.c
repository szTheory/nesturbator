/* BOARD-02: the MMC1 board on synthetic images. One function per behaviour. Every code byte is
   authored here and placed with ines_spec segments; PRG bank b of 16 KiB starts at
   b * INES_PRG_BANK. */
#include <stdint.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"

#define IMAGE_CAP (16u + 32u * INES_PRG_BANK + 4u * INES_CHR_BANK)

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

/* Tracer: a 128 KiB CHR-RAM board. The fixed last bank (7, at $C000) writes 1 to the PRG register
   through five non-adjacent serial writes and jumps to $8000. Bank 1 stores $42 at $0010; bank 0
   would store $24, so a wrong bank is visible. */
static void test_tracer_five_writes_switch_bank(void)
{
    static uint8_t image[IMAGE_CAP];
    /* LDA #$01; STA $E000; LSR A; STA $E000 x4; JMP $8000 */
    static const uint8_t boot[] = {0xa9u, 0x01u, 0x8du, 0x00u, 0xe0u, 0x4au, 0x8du,
                                   0x00u, 0xe0u, 0x8du, 0x00u, 0xe0u, 0x8du, 0x00u,
                                   0xe0u, 0x8du, 0x00u, 0xe0u, 0x4cu, 0x00u, 0x80u};
    /* LDA #$42; STA $0010; JMP $8005 */
    static const uint8_t bank1[] = {0xa9u, 0x42u, 0x8du, 0x10u, 0x00u, 0x4cu, 0x05u, 0x80u};
    static const uint8_t bank0[] = {0xa9u, 0x24u, 0x8du, 0x10u, 0x00u, 0x4cu, 0x05u, 0x80u};
    struct ines_segment segs[3];
    struct ines_spec spec;
    nesturbator *inst = make_instance();
    struct nesturbator *nes = inst;
    size_t size;
    segs[0].prg_offset = 0x00000u;
    segs[0].bytes = bank0;
    segs[0].len = sizeof bank0;
    segs[1].prg_offset = 1u * INES_PRG_BANK;
    segs[1].bytes = bank1;
    segs[1].len = sizeof bank1;
    segs[2].prg_offset = 7u * INES_PRG_BANK;
    segs[2].bytes = boot;
    segs[2].len = sizeof boot;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 8u;
    spec.nes2 = 1u;
    spec.mapper = 1u;
    spec.segments = segs;
    spec.segment_count = 3u;
    spec.reset_vector = 0xc000u;
    size = ines_build(image, sizeof image, &spec);
    CHECK(size != 0u);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    run_frame(inst);
    CHECK_EQ_U64(nes->mapper.reg.mmc1.prg, 1u);
    CHECK_EQ_HEX(nes->bus.ram[0x10], 0x42u);
    nesturbator_destroy(inst);
}

int main(void)
{
    test_tracer_five_writes_switch_bank();
    CHECK_DONE();
}
