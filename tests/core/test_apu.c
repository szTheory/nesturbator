/* SND-01: a CPU-programmed pulse reaches the public caller-owned PCM buffer. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "nesturbator.h"
#include "../check.h"

#define FRAME_SAMPLES 798u
static uint16_t pixels[256u * 240u];
static int16_t samples[FRAME_SAMPLES];

static void make_image(uint8_t *image, size_t size)
{
    memset(image, 0, size);
    memcpy(image, "NES\032", 4u);
    image[4] = 1u;
    image[5] = 1u;
    /* LDA #1; STA $4015; LDA #$BF; STA $4000; timer=$008; enable pulse. */
    static const uint8_t program[] = {
        0xa9u, 0x01u, 0x8du, 0x15u, 0x40u, 0xa9u, 0xbfu, 0x8du, 0x00u, 0x40u,
        0xa9u, 0x08u, 0x8du, 0x02u, 0x40u, 0xa9u, 0x00u, 0x8du, 0x03u, 0x40u,
        0x4cu, 0x14u, 0x80u};
    memcpy(image + 16u, program, sizeof program);
    image[16u + 0x3ffcu] = 0u;
    image[16u + 0x3ffdu] = 0x80u;
}

static nesturbator *make(void)
{
    nesturbator_config cfg;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    nesturbator *inst = NULL;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    return inst;
}

static void test_public_pulse_pcm(void)
{
    const size_t image_size = 16u + 16384u + 8192u;
    uint8_t *image = calloc(1u, image_size);
    nesturbator *inst = make();
    CHECK(image != NULL);
    if (image == NULL || inst == NULL) {
        free(image);
        nesturbator_destroy(inst);
        return;
    }
    make_image(image, image_size);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, image_size), NESTURBATOR_OK);
    nesturbator_frame io;
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = pixels;
    io.video_pitch = 256u;
    io.audio = samples;
    io.audio_capacity = FRAME_SAMPLES;
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
    CHECK_EQ_U64(io.audio_count, FRAME_SAMPLES);
    unsigned nonzero = 0u;
    for (uint32_t i = 0u; i < io.audio_count; i++)
        nonzero += samples[i] != 0;
    CHECK(nonzero > 0u);
    nesturbator_destroy(inst);
    free(image);
}

int main(void)
{
    test_public_pulse_pcm();
    CHECK_DONE();
}
