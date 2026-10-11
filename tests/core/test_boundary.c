/* D-16: at every frame boundary no frame-scoped pointer is left in the instance and the PCM ring is
   empty. Phase 12 saves state at a frame boundary, so a state must never need the caller's video
   or audio buffers, or buffered PCM. Images are synthetic (a header, a reset vector and a few
   instructions); no ROM bytes. */
#include <stdint.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"

#define IMAGE_CAP 45000u

/* LDA #$0F; STA $4015; pulse 1: LDA #$BF; STA $4000; LDA #$40; STA $4002; LDA #$08; STA $4003;
   triangle: LDA #$FF; STA $4008; LDA #$40; STA $400A; LDA #$08; STA $400B; JMP * (at $8023). */
static const uint8_t apu_code[] = {
    0xa9u, 0x0fu, 0x8du, 0x15u, 0x40u, 0xa9u, 0xbfu, 0x8du, 0x00u, 0x40u, 0xa9u, 0x40u,
    0x8du, 0x02u, 0x40u, 0xa9u, 0x08u, 0x8du, 0x03u, 0x40u, 0xa9u, 0xffu, 0x8du, 0x08u,
    0x40u, 0xa9u, 0x40u, 0x8du, 0x0au, 0x40u, 0xa9u, 0x08u, 0x8du, 0x0bu, 0x40u, 0x4cu,
    0x23u, 0x80u};
static const uint8_t jam_code[] = {0x02u};

static size_t build(uint8_t *out, const uint8_t *code, size_t len)
{
    struct ines_spec spec;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 2u;
    spec.chr_8k = 1u;
    spec.prg_code = code;
    spec.prg_code_len = len;
    spec.nmi_vector = 0x8000u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    return ines_build(out, IMAGE_CAP, &spec);
}

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

static int boundary_clean(const nesturbator *n)
{
    return n->ppu.video_output == NULL && n->ppu.video_pitch == 0u &&
           n->apu.sample_output == NULL && n->apu.sample_limit == 0u &&
           n->apu.sample_count == 0u && n->synth.pcm_count == 0u &&
           n->synth.pcm_read == n->synth.pcm_write && n->synth.overflow == 0u;
}

/* Runs one frame with the given audio capacity and returns its status. */
static nesturbator_status frame(nesturbator *inst, uint32_t capacity, uint32_t *produced)
{
    static uint16_t video[256u * 240u];
    static int16_t audio[1024];
    nesturbator_frame io;
    nesturbator_status st;
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = video;
    io.video_pitch = 256u;
    io.audio = audio;
    io.audio_capacity = capacity;
    st = nesturbator_run_frame(inst, &io);
    if (produced != NULL)
        *produced = io.audio_count;
    return st;
}

static void frames_clean(nesturbator *inst, int count, int use_alternation)
{
    uint32_t seen_low = 0u, seen_high = 0u;
    for (int i = 0; i < count; i++) {
        uint32_t n = 0u;
        CHECK_EQ_U64(frame(inst, 1024u, &n), NESTURBATOR_OK);
        CHECK(boundary_clean(inst));
        if (n == 798u)
            seen_low++;
        if (n == 799u)
            seen_high++;
    }
    if (use_alternation)
        CHECK(seen_low > 0u && seen_high > 0u);
}

static void test_card(void)
{
    nesturbator *inst = make_instance();
    CHECK(boundary_clean(inst));
    frames_clean(inst, 10, 0);
    nesturbator_destroy(inst);
}

static void apu_frames_and_unload(void)
{
    static uint8_t image[IMAGE_CAP];
    size_t size = build(image, apu_code, sizeof apu_code);
    nesturbator *inst = make_instance();
    CHECK(size != 0u);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    frames_clean(inst, 60, 1);
    /* Too-small audio buffer: the call fails and leaves nothing behind. */
    CHECK_EQ_U64(frame(inst, 10u, NULL), NESTURBATOR_ERR_BUFFER_TOO_SMALL);
    CHECK(boundary_clean(inst));
    nesturbator_unload_cartridge(inst);
    CHECK(boundary_clean(inst));
    frames_clean(inst, 3, 0);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    frames_clean(inst, 5, 0);
    nesturbator_destroy(inst);
}

static void jam_frame(void)
{
    static uint8_t image[IMAGE_CAP];
    size_t size = build(image, jam_code, sizeof jam_code);
    nesturbator *inst = make_instance();
    int jammed = 0;
    CHECK(size != 0u);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    for (int i = 0; i < 3 && !jammed; i++) {
        nesturbator_status st = frame(inst, 1024u, NULL);
        CHECK(boundary_clean(inst));
        jammed = st == NESTURBATOR_STOP_JAM;
    }
    CHECK(jammed);
    nesturbator_destroy(inst);
}

int main(void)
{
    test_card();
    apu_frames_and_unload();
    jam_frame();
    CHECK_DONE();
}
