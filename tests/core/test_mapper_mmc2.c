/* Phase 30 red-path smoke: load the owned marker image through the public API,
   trigger MMC2 from an actual PPU pattern read, and compare native continuation. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../mmc2_fixture.h"

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

static void run_frame(nesturbator *inst, uint16_t *video, int16_t *audio, nesturbator_frame *io)
{
    memset(io, 0, sizeof *io);
    io->size = (uint32_t)sizeof *io;
    io->video = video;
    io->video_pitch = 256u;
    io->audio = audio;
    io->audio_capacity = 2048u;
    CHECK_EQ_U64(nesturbator_run_frame(inst, io), NESTURBATOR_OK);
}

static void test_public_load_read_restore_continuation(void)
{
    static uint8_t image[MMC2_FIXTURE_CAP];
    static uint16_t video_a[256u * 240u];
    static uint16_t video_b[256u * 240u];
    static int16_t audio_a[2048u];
    static int16_t audio_b[2048u];
    const size_t image_size = mmc2_fixture_build(image, sizeof image, 1u);
    nesturbator *source = make_instance();
    nesturbator *restored = make_instance();
    uint8_t *state = NULL;
    size_t state_size = 0u;
    nesturbator_status status;

    CHECK(source != NULL && restored != NULL);
    if (source == NULL || restored == NULL)
        goto done;
    CHECK_EQ_U64(image_size, MMC2_FIXTURE_CAP);
    status = nesturbator_load_cartridge(source, image, image_size);
    CHECK_EQ_U64(status, NESTURBATOR_OK);
    if (status != NESTURBATOR_OK)
        goto done;

    struct nesturbator *nes = (struct nesturbator *)source;
    nesturbator__bus_write(nes, 0xb000u, 5u); /* FD selector */
    nesturbator__bus_write(nes, 0xc000u, 3u); /* FE selector */
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x0fd0u), 0xa3u); /* old FE bank */
    CHECK_EQ_HEX(nesturbator__ppu_read(nes, 0x0000u), 0xa5u); /* new FD bank */

    state_size = nesturbator_state_size(source);
    CHECK(state_size > 64u);
    state = (uint8_t *)malloc(state_size);
    CHECK(state != NULL);
    if (state == NULL)
        goto done;
    CHECK_EQ_U64(nesturbator_save_state(source, state, state_size), NESTURBATOR_OK);
    status = nesturbator_load_cartridge(restored, image, image_size);
    CHECK_EQ_U64(status, NESTURBATOR_OK);
    if (status != NESTURBATOR_OK)
        goto done;
    CHECK_EQ_U64(nesturbator_load_state(restored, state, state_size), NESTURBATOR_OK);

    nesturbator_frame frame_a, frame_b;
    run_frame(source, video_a, audio_a, &frame_a);
    run_frame(restored, video_b, audio_b, &frame_b);
    CHECK_EQ_U64(frame_a.audio_count, frame_b.audio_count);
    CHECK_EQ_U64(frame_a.frame_number, frame_b.frame_number);
    CHECK_EQ_U64(frame_a.ticks, frame_b.ticks);
    CHECK(memcmp(video_a, video_b, sizeof video_a) == 0);
    CHECK(memcmp(audio_a, audio_b, (size_t)frame_a.audio_count * sizeof audio_a[0]) == 0);

done:
    free(state);
    nesturbator_destroy(restored);
    nesturbator_destroy(source);
}

int main(void)
{
    test_public_load_read_restore_continuation();
    return check_failures == 0u ? 0 : 1;
}
