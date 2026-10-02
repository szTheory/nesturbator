/* The host-side colour conversion shared by the runner and the libretro
 * adapter (D-15), so both turn native pixels into the same RGB. Not part of
 * the core and not installed. */
#ifndef NESTURBATOR_HOST_CONVERT_H
#define NESTURBATOR_HOST_CONVERT_H

#include <stdint.h>

/* Converts one 256x240 frame of native pixels to XRGB8888.
 * For each pixel, out = palette[video & 0x1FF]: a native pixel is a palette
 * entry (6 bits) and the emphasis bits (3 bits above it), so only the low 9
 * bits index the 512-entry table from nesturbator_get_palette.
 * pitch and out_pitch count elements per row (each at least 256); only
 * columns 0-255 of each output row are written. */
void nesturbator_host_convert(const uint32_t palette[512], const uint16_t *video, uint32_t pitch,
                              uint32_t *out_xrgb, uint32_t out_pitch);

#endif /* NESTURBATOR_HOST_CONVERT_H */
