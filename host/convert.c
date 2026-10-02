#include "convert.h"

#include <stddef.h>

#define WIDTH 256u
#define HEIGHT 240u

void nesturbator_host_convert(const uint32_t palette[512], const uint16_t *video, uint32_t pitch,
                              uint32_t *out_xrgb, uint32_t out_pitch)
{
    for (uint32_t y = 0; y < HEIGHT; y++) {
        const uint16_t *src = video + (size_t)y * pitch;
        uint32_t *dst = out_xrgb + (size_t)y * out_pitch;
        for (uint32_t x = 0; x < WIDTH; x++) {
            dst[x] = palette[src[x] & 0x1FFu];
        }
    }
}
