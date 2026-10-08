/* Controller serial-port behavior through the CPU bus and size-tagged API. */
#include <string.h>

#include "internal.h"
#include "../check.h"

static nesturbator *make(void)
{
    nesturbator_config cfg;
    nesturbator *nes = NULL;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &nes), NESTURBATOR_OK);
    return nes;
}

static void latch(nesturbator *nes)
{
    nesturbator__bus_write(nes, 0x4016u, 1u);
    nesturbator__bus_write(nes, 0x4016u, 0u);
}

static void test_two_ports_shift_independently(void)
{
    nesturbator *nes = make();
    nesturbator_input input;
    static const uint8_t expected0[] = {1u, 0u, 1u, 0u, 0u, 1u, 0u, 1u};
    static const uint8_t expected1[] = {0u, 1u, 1u, 0u, 1u, 0u, 1u, 0u};
    memset(&input, 0, sizeof input);
    input.size = (uint32_t)sizeof input;
    input.buttons[0] = (uint8_t)(NESTURBATOR_BUTTON_A | NESTURBATOR_BUTTON_SELECT |
                                 NESTURBATOR_BUTTON_DOWN | NESTURBATOR_BUTTON_RIGHT);
    input.buttons[1] = (uint8_t)(NESTURBATOR_BUTTON_B | NESTURBATOR_BUTTON_SELECT |
                                 NESTURBATOR_BUTTON_UP | NESTURBATOR_BUTTON_LEFT);
    CHECK_EQ_U64(nesturbator_set_input(nes, &input), NESTURBATOR_OK);
    nesturbator__controller_begin_frame(nes);
    latch(nes);
    for (uint32_t i = 0; i < 8u; i++) {
        CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4016u) & 1u, expected0[i]);
        CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4017u) & 1u, expected1[i]);
    }
    CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4016u) & 1u, 1u);
    CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4017u) & 1u, 1u);
    nesturbator_destroy(nes);
}

static void test_high_strobe_tracks_a_and_invalid_input_is_unchanged(void)
{
    nesturbator *nes = make();
    nesturbator_input input, invalid;
    memset(&input, 0, sizeof input);
    input.size = (uint32_t)sizeof input;
    input.buttons[0] = NESTURBATOR_BUTTON_A;
    input.buttons[1] = NESTURBATOR_BUTTON_B;
    CHECK_EQ_U64(nesturbator_set_input(nes, &input), NESTURBATOR_OK);
    nesturbator__controller_begin_frame(nes);
    nesturbator__bus_write(nes, 0x4016u, 1u);
    CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4016u) & 1u, 1u);
    input.buttons[0] = 0u;
    CHECK_EQ_U64(nesturbator_set_input(nes, &input), NESTURBATOR_OK);
    nesturbator__controller_begin_frame(nes);
    memset(&invalid, 0, sizeof invalid);
    invalid.size = 0u;
    CHECK_EQ_U64(nesturbator_set_input(nes, &invalid), NESTURBATOR_ERR_STRUCT_SIZE);
    CHECK_EQ_U64(nes->bus.input_pending[0], 0u);
    CHECK_EQ_U64(nes->bus.input_pending[1], NESTURBATOR_BUTTON_B);
    CHECK_EQ_U64(nesturbator_set_input(nes, NULL), NESTURBATOR_ERR_ARGUMENT);
    CHECK_EQ_U64(nesturbator_set_input(NULL, &input), NESTURBATOR_ERR_ARGUMENT);
    nesturbator_destroy(nes);
}

int main(void)
{
    test_two_ports_shift_independently();
    test_high_strobe_tracks_a_and_invalid_input_is_unchanged();
    CHECK_DONE();
}
