/* nesturbator_get_palette: copies the native-to-XRGB8888 table. */
#include "internal.h"

void nesturbator_get_palette(const nesturbator *inst, uint32_t *out_xrgb8888, uint32_t count)
{
    /* One NTSC table in Phase 1; inst will select a per-PPU-revision table
       (D-08). */
    (void)inst;
    if (out_xrgb8888 == NULL) {
        return;
    }
    if (count > 512u) {
        count = 512u;
    }
    for (uint32_t i = 0; i < count; i++) {
        out_xrgb8888[i] = nesturbator__palette_ntsc[i];
    }
}
