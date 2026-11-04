/* nesturbator_get_palette: copies the native-to-XRGB8888 table. */
#include "internal.h"
#include "palette_pal.inc"

void nesturbator_get_palette(const nesturbator *inst, uint32_t *out_xrgb8888, uint32_t count)
{
    const uint32_t *table = inst != NULL && nesturbator__is_pal(inst) != 0
                                ? nesturbator__palette_pal
                                : nesturbator__palette_ntsc;
    if (out_xrgb8888 == NULL) {
        return;
    }
    if (count > 512u) {
        count = 512u;
    }
    for (uint32_t i = 0; i < count; i++) {
        out_xrgb8888[i] = table[i];
    }
}
