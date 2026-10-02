/* FRAME-02: a program that knows only the installed public header creates an
   instance, runs one frame and destroys it. Exits 0 on success, 1 otherwise. */
#include <nesturbator.h>
#include <stdint.h>
#include <stdio.h>

static uint16_t video[256 * 240];
static int16_t audio[1024];

int main(void)
{
    nesturbator_config cfg = NESTURBATOR_CONFIG_INIT;
    nesturbator *inst = NULL;
    nesturbator_frame io = {0};
    nesturbator_status status = nesturbator_create(&cfg, &inst);

    if (status != NESTURBATOR_OK) {
        printf("nesturbator_create: status %d\n", (int)status);
        return 1;
    }
    io.size = (uint32_t)sizeof io;
    io.video = video;
    io.video_pitch = 256;
    io.audio = audio;
    io.audio_capacity = 1024;
    status = nesturbator_run_frame(inst, &io);
    nesturbator_destroy(inst);
    if (status != NESTURBATOR_OK) {
        printf("nesturbator_run_frame: status %d\n", (int)status);
        return 1;
    }
    if (io.frame_number != 1) {
        printf("frame_number %llu, expected 1\n", (unsigned long long)io.frame_number);
        return 1;
    }
    return 0;
}
