/* D-02: the PPUCTRL NMI edge, end to end. A synthetic NROM program enables NMI, and its NMI
   handler rewrites $2000 with bit 7 still set and counts each NMI in a 16-bit counter at
   $0000/$0001. NESdev Wiki "PPU registers": only "changing NMI enable from 0 to 1 while the
   vblank flag in PPUSTATUS is 1" triggers an NMI, so the handler's own rewrite must not
   re-enter NMI and the count after n frames is n. */
#include <stdint.h>
#include <string.h>
#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"

#define IMAGE_CAP 40000u

/* LDA #$80; STA $2000; JMP $8005 (a jump to itself). */
static const uint8_t reset_code[] = {0xa9u, 0x80u, 0x8du, 0x00u, 0x20u, 0x4cu, 0x05u, 0x80u};
/* INC $00; BNE +2; INC $01; LDA #$80; STA $2000; RTI. */
static const uint8_t nmi_code[] = {0xe6u, 0x00u, 0xd0u, 0x02u, 0xe6u, 0x01u,
                                   0xa9u, 0x80u, 0x8du, 0x00u, 0x20u, 0x40u};

int main(void)
{
    static uint8_t image[IMAGE_CAP];
    static uint16_t video[256u * 240u];
    static int16_t audio[2048];
    static const struct ines_segment seg[] = {{0x10u, nmi_code, sizeof nmi_code}};
    struct ines_spec spec;
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    size_t size;

    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = 1u;
    spec.prg_code = reset_code;
    spec.prg_code_len = sizeof reset_code;
    spec.segments = seg;
    spec.segment_count = 1u;
    spec.nmi_vector = 0x8010u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    size = ines_build(image, sizeof image, &spec);
    CHECK(size != 0u);

    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);

    for (unsigned frame = 1u; frame <= 10u; ++frame) {
        nesturbator_frame io;
        uint8_t lo = 0u, hi = 0u;
        memset(&io, 0, sizeof io);
        io.size = (uint32_t)sizeof io;
        io.video = video;
        io.video_pitch = 256u;
        io.audio = audio;
        io.audio_capacity = 2048u;
        CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_peek_cpu_ram(inst, 0x0000u, &lo), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_peek_cpu_ram(inst, 0x0001u, &hi), NESTURBATOR_OK);
        CHECK_EQ_U64((uint64_t)hi * 256u + lo, frame);
    }
    nesturbator_destroy(inst);
    CHECK_DONE();
}
