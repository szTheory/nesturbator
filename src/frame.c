/* nesturbator_run_frame: checks, then the no-cartridge frame. */
#include <string.h>

#include "internal.h"

nesturbator_status nesturbator_run_frame(nesturbator *inst, nesturbator_frame *io)
{
    /* Every check comes before any state changes. */
    if (inst == NULL || io == NULL) {
        return NESTURBATOR_ERR_ARGUMENT;
    }
    if (inst->cpu.jammed != 0u) {
        return NESTURBATOR_STOP_JAM;
    }
    nesturbator_status st =
        nesturbator__check_size_in(io, NESTURBATOR_FRAME_SIZE_V1, (uint32_t)sizeof *io);
    if (st != NESTURBATOR_OK) {
        return st;
    }
    if (io->video == NULL) {
        return NESTURBATOR_ERR_ARGUMENT;
    }
    if (io->video_pitch < NESTURBATOR_WIDTH) {
        return NESTURBATOR_ERR_BUFFER_TOO_SMALL;
    }
    /* Samples this frame: 352 per 315000 ticks, with the remainder carried
       over (D-09). 798 or 799. */
    uint64_t acc = (uint64_t)inst->audio_rem +
                   (uint64_t)NESTURBATOR_TICKS_PER_FRAME * NESTURBATOR_AUDIO_SAMPLES_PER_PERIOD;
    uint32_t n = (uint32_t)(acc / NESTURBATOR_AUDIO_TICKS_PER_PERIOD);
    if (io->audio == NULL && io->audio_capacity > 0u) {
        return NESTURBATOR_ERR_ARGUMENT;
    }
    if (io->audio_capacity < n) {
        return NESTURBATOR_ERR_BUFFER_TOO_SMALL;
    }

    if (inst->cart.bytes == NULL) {
        /* No cartridge: test pattern and silence. */
        for (uint32_t y = 0; y < NESTURBATOR_HEIGHT; y++) {
            uint16_t *row = io->video + (size_t)y * io->video_pitch;
            for (uint32_t x = 0; x < NESTURBATOR_WIDTH; x++) {
                row[x] = nesturbator__test_pixel(x, y);
            }
        }
        inst->ticks += NESTURBATOR_TICKS_PER_FRAME;
    } else {
        uint64_t target = inst->ticks + NESTURBATOR_TICKS_PER_FRAME;
        while (inst->ticks < target && inst->cpu.jammed == 0u) {
            nesturbator__cpu_step(inst);
        }
        if (inst->cpu.jammed != 0u) {
            return NESTURBATOR_STOP_JAM;
        }
        nesturbator__ppu_run_until(inst, inst->ticks);
        nesturbator__ppu_render(inst, io->video, io->video_pitch);
    }
    if (n > 0u) {
        memset(io->audio, 0, (size_t)n * sizeof io->audio[0]);
    }

    inst->audio_rem = (uint32_t)(acc % NESTURBATOR_AUDIO_TICKS_PER_PERIOD);
    inst->frame_number += 1u;
    io->audio_count = n;
    io->frame_number = inst->frame_number;
    io->ticks = inst->ticks;
    return NESTURBATOR_OK;
}
