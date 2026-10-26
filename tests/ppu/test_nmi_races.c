/* NMI-01: owned dot and real-CPU-bus probes for PPUSTATUS race ordering.
   AccuracyCoin at 6bee919529289c555803b612b8c9e4f7eaaeeabb, TEST_2002FlagTiming
   (AccuracyCoin.asm:1776-1793), states that bit 7 is sampled when M2 rises and
   sprite flags at M2 fall. The NTSC set/clear points are 241:1 and 261:1
   (NESdev PPU frame timing, revision cited by HWP.03). */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "../nmi_fixture.h"

static void test_flag_set_before_on_after_dot(void)
{
    struct nesturbator *nes = nmi_fixture_build(NULL, 0u, NULL, 0u, 0x8000u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    /* With phase origin 0, each run advances one complete 8-tick PPU dot. */
    nmi_fixture_at(nes, 241u, 0u, 0x60u, 0u, 0x15u);
    nesturbator__ppu_run_until(nes, 0u); /* before (241,1) */
    CHECK_EQ_U64(nes->ppu.dot, 0u);
    CHECK_EQ_HEX(nes->ppu.status & 0xe0u, 0x60u);
    nesturbator__ppu_run_until(nes, 8u); /* on (241,1) */
    CHECK_EQ_U64(nes->ppu.dot, 1u);
    CHECK_EQ_HEX(nes->ppu.status & 0xe0u, 0xe0u);
    nesturbator__ppu_run_until(nes, 16u); /* after (241,1) */
    CHECK_EQ_U64(nes->ppu.dot, 2u);
    CHECK_EQ_HEX(nes->ppu.status & 0xe0u, 0xe0u);
    printf("CASE guard ppu-set before=241:0 on=241:1 after=241:2\n");
    nesturbator_destroy((nesturbator *)nes);
}

static void test_flag_clear_before_on_after_dot(void)
{
    struct nesturbator *nes = nmi_fixture_build(NULL, 0u, NULL, 0u, 0x8000u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    nmi_fixture_at(nes, 261u, 0u, 0xe0u, 0u, 0x15u);
    nesturbator__ppu_run_until(nes, 0u); /* before (261,1) */
    CHECK_EQ_U64(nes->ppu.dot, 0u);
    CHECK_EQ_HEX(nes->ppu.status & 0xe0u, 0xe0u);
    nesturbator__ppu_run_until(nes, 8u); /* on (261,1) */
    CHECK_EQ_U64(nes->ppu.dot, 1u);
    CHECK_EQ_HEX(nes->ppu.status & 0xe0u, 0u);
    nesturbator__ppu_run_until(nes, 16u); /* after (261,1) */
    CHECK_EQ_U64(nes->ppu.dot, 2u);
    CHECK_EQ_HEX(nes->ppu.status & 0xe0u, 0u);
    printf("CASE guard ppu-clear before=261:0 on=261:1 after=261:2\n");
    nesturbator_destroy((nesturbator *)nes);
}

static void test_outside_window_control(void)
{
    struct nesturbator *nes = nmi_fixture_build(NULL, 0u, NULL, 0u, 0x8000u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;
    nmi_fixture_at(nes, 240u, 320u, 0u, 0u, 0x15u);
    nesturbator__ppu_run_until(nes, 160u); /* twenty dots, still on scanline 240 */
    CHECK_EQ_U64(nes->ppu.scanline, 240u);
    CHECK_EQ_U64(nes->ppu.dot, 340u);
    CHECK_EQ_U64(nes->ppu.status & 0x80u, 0u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 0u);
    printf("CASE guard outside-set-window scanline=240 dot=340 vblank=0 nmi=0\n");
    nesturbator_destroy((nesturbator *)nes);
}

static void assert_ram_read_does_not_select_status(uint16_t address)
{
    struct nesturbator *nes = nmi_fixture_build(NULL, 0u, NULL, 0u, 0x8000u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    uint16_t ram_index = (uint16_t)(address & 0x07ffu);
    uint8_t expected = (uint8_t)(0x40u | ((address >> 3u) & 0x3fu));
    nmi_fixture_at(nes, 241u, 10u, 0x80u, 0u, 0x15u);
    nes->ppu.address_latch = 1u;
    nes->bus.ram[ram_index] = expected;

    uint8_t value = nesturbator__bus_read(nes, address);
    CHECK_EQ_HEX(value, expected);
    CHECK_EQ_HEX(nes->ppu.status & 0x80u, 0x80u);
    CHECK_EQ_U64(nes->ppu.address_latch, 1u);
    CHECK_EQ_HEX(nes->ppu.io_bus, 0x15u);
    printf("CASE guard ram-read address=%04x index=%03x value=%02x vblank=retained\n", address,
           ram_index, value);
    nesturbator_destroy((nesturbator *)nes);
}

static void test_ram_mirrors_do_not_select_status(void)
{
    static const uint16_t ram_addresses[] = {
        0x0002u, 0x00eau, 0x0802u, 0x1002u, 0x1802u, 0x1ffau, 0x1fffu,
    };
    for (size_t i = 0u; i < sizeof ram_addresses / sizeof ram_addresses[0]; i++)
        assert_ram_read_does_not_select_status(ram_addresses[i]);
}

static void test_dmc_parked_ram_read_does_not_select_status(void)
{
    struct nesturbator *nes = nmi_fixture_build(NULL, 0u, NULL, 0u, 0x8000u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    nmi_fixture_at(nes, 241u, 10u, 0x80u, 0u, 0x15u);
    nes->ppu.address_latch = 1u;
    nes->bus.ram[2u] = 0x5au;
    nes->bus.apu_get_put_phase = 1u;
    nes->apu.dmc.address = 0x8000u;
    nes->apu.dmc.remaining = 1u;
    nes->apu.dmc.dma_pending = 1u;
    nes->apu.dmc.dma_halt_phase = 0u;
    nes->apu.dmc.enable_delay = 0u;
    nes->apu.dmc.buffer_empty = 0u;
    nes->apu.dmc.bits = 8u;
    nes->apu.dmc.counter = 100u;

    uint8_t value = nesturbator__bus_read(nes, 0x0002u);
    CHECK_EQ_HEX(value, 0x5au);
    CHECK_EQ_HEX(nes->ppu.status & 0x80u, 0x80u);
    CHECK_EQ_U64(nes->ppu.address_latch, 1u);
    CHECK_EQ_HEX(nes->ppu.io_bus, 0x15u);
    CHECK_EQ_U64(nes->ticks, 5u * 24u);
    CHECK_EQ_U64(nes->apu.dmc.dma_pending, 0u);
    CHECK_EQ_U64(nes->apu.dmc.remaining, 0u);
    printf("CASE guard dmc-parked-ram-read address=0002 vblank=retained ticks=%llu\n",
           (unsigned long long)nes->ticks);
    nesturbator_destroy((nesturbator *)nes);
}

static void assert_m2_split_witness(uint16_t start_scanline, uint16_t event_scanline,
                                    uint16_t expected_late_dot, uint8_t expected_status)
{
    struct nesturbator *nes = nmi_fixture_build(NULL, 0u, NULL, 0u, 0x8000u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    nmi_fixture_at(nes, start_scanline, 340u, 0u, 0u, 0u);
    nes->ticks += 9u;
    nesturbator__ppu_run_until(nes, nes->ticks); /* M2 rise */
    CHECK_EQ_U64(nes->ppu.scanline, event_scanline);
    CHECK_EQ_U64(nes->ppu.dot, 0u);
    CHECK_EQ_U64(nes->ppu.status & 0x80u, 0u);
    nes->ticks += 15u;
    nesturbator__ppu_run_until(nes, nes->ticks); /* M2 fall */
    CHECK_EQ_U64(nes->ppu.scanline, event_scanline);
    CHECK_EQ_U64(nes->ppu.dot, expected_late_dot);
    CHECK_EQ_HEX(nes->ppu.status & 0xe0u, expected_status);
    printf("CASE guard m2-witness rise=%u:0 fall=%u:%u ticks=9+15 dots=8\n",
           (unsigned)(event_scanline - 1u), (unsigned)event_scanline, (unsigned)expected_late_dot);
    nesturbator_destroy((nesturbator *)nes);
}

static struct nesturbator *split_read_nes;
static uint16_t split_read_address;
static uint8_t split_read_expected_sprite_flags;
static uint8_t split_read_expected_early_vblank;
static uint8_t split_read_expected_post_status;
static uint8_t split_read_value;

static void split_read_case(void)
{
    uint64_t start_ticks = split_read_nes->ticks;
    uint64_t start_ppu_ticks = split_read_nes->ppu.ppu_ticks;
    uint64_t start_cycle = split_read_nes->cpu_cycle;
    split_read_value = nesturbator__bus_read(split_read_nes, split_read_address);

    nmi_expected_assertion("m2-rise-bit7-sample",
                           (split_read_value & 0x80u) == split_read_expected_early_vblank);
    nmi_expected_assertion("m2-fall-sprite-bits",
                           (split_read_value & 0x60u) == split_read_expected_sprite_flags);
    CHECK_EQ_HEX(split_read_value & 0x1fu, 0x15u);
    CHECK_EQ_HEX(split_read_nes->bus.open_bus, split_read_value);
    CHECK_EQ_U64(split_read_nes->ticks - start_ticks, 24u);
    CHECK_EQ_U64(split_read_nes->ppu.ppu_ticks - start_ppu_ticks, 24u);
    CHECK_EQ_U64(split_read_nes->cpu_cycle - start_cycle, 1u);
    CHECK_EQ_HEX(split_read_nes->ppu.status & 0xe0u, split_read_expected_post_status);
    CHECK_EQ_U64(split_read_nes->ppu.address_latch, 0u);
    CHECK_EQ_U64(split_read_nes->ppu.io_bus_age, 0u);
}

static void test_set_race_read(uint16_t address, const char *case_name)
{
    static const uint8_t idle[] = {0x4cu, 0x00u, 0x80u};
    split_read_nes = nmi_fixture_build(idle, sizeof idle, NULL, 0u, 0x8000u);
    CHECK(split_read_nes != NULL);
    if (split_read_nes == NULL)
        return;
    nmi_fixture_at(split_read_nes, 240u, 340u, 0x60u, 0u, 0x15u);
    split_read_nes->ppu.address_latch = 1u;
    split_read_nes->ppu.io_bus_age = 7u;
    split_read_address = address;
    split_read_expected_early_vblank = 0u;
    split_read_expected_sprite_flags = 0x60u;
    split_read_expected_post_status = 0x60u;
    split_read_case();
    printf("CASE guard %s split-status-read\n", case_name);
    nesturbator_destroy((nesturbator *)split_read_nes);
    split_read_nes = NULL;
}

static void test_clear_race_read(uint16_t address, const char *case_name)
{
    static const uint8_t idle[] = {0x4cu, 0x00u, 0x80u};
    split_read_nes = nmi_fixture_build(idle, sizeof idle, NULL, 0u, 0x8000u);
    CHECK(split_read_nes != NULL);
    if (split_read_nes == NULL)
        return;
    nmi_fixture_at(split_read_nes, 260u, 340u, 0xe0u, 0u, 0x15u);
    split_read_nes->ppu.address_latch = 1u;
    split_read_nes->ppu.io_bus_age = 7u;
    split_read_address = address;
    split_read_expected_early_vblank = 0x80u;
    split_read_expected_sprite_flags = 0u;
    split_read_expected_post_status = 0u;
    split_read_case();
    printf("CASE guard %s split-status-read\n", case_name);
    nesturbator_destroy((nesturbator *)split_read_nes);
    split_read_nes = NULL;
}

static int begin_split_read_control(uint16_t scanline, uint8_t status, uint8_t expected_early,
                                    uint8_t expected_sprite_flags)
{
    static const uint8_t idle[] = {0x4cu, 0x00u, 0x80u};
    split_read_nes = nmi_fixture_build(idle, sizeof idle, NULL, 0u, 0x8000u);
    CHECK(split_read_nes != NULL);
    if (split_read_nes == NULL)
        return 0;
    nmi_fixture_at(split_read_nes, scanline, 340u, status, 0u, 0x15u);
    split_read_address = 0x2002u;
    split_read_expected_early_vblank = expected_early;
    split_read_expected_sprite_flags = expected_sprite_flags;
    split_read_expected_post_status = scanline == 240u ? 0x60u : 0u;
    return 1;
}

static void test_wrong_status_phase_controls(void)
{
    if (begin_split_read_control(240u, 0x60u, 0x80u, 0x60u)) {
        nmi_expect_red("production-path-set-shift-all-at-fall", "m2-rise-bit7-sample",
                       split_read_case);
        nesturbator_destroy((nesturbator *)split_read_nes);
        split_read_nes = NULL;
    }
    if (begin_split_read_control(260u, 0xe0u, 0u, 0u)) {
        nmi_expect_red("production-path-clear-shift-one-dot-early", "m2-rise-bit7-sample",
                       split_read_case);
        nesturbator_destroy((nesturbator *)split_read_nes);
        split_read_nes = NULL;
    }
    if (begin_split_read_control(260u, 0xe0u, 0x80u, 0x60u)) {
        nmi_expect_red("production-path-merge-status-at-rise", "m2-fall-sprite-bits",
                       split_read_case);
        nesturbator_destroy((nesturbator *)split_read_nes);
        split_read_nes = NULL;
    }
}

static void test_nmi_suppression_cases(void)
{
    static const uint8_t idle[] = {0x4cu, 0x00u, 0x80u};
    struct nesturbator *nes = nmi_fixture_build(idle, sizeof idle, NULL, 0u, 0x8000u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    /* M2-rise samples (241,0), the source-defined suppression coordinate. */
    nmi_fixture_at(nes, 240u, 340u, 0u, 0x80u, 0x15u);
    uint8_t value = nesturbator__bus_read(nes, 0x2002u);
    CHECK_EQ_HEX(value, 0x15u);
    CHECK_EQ_U64(nes->ppu.scanline, 241u);
    CHECK_EQ_U64(nes->ppu.dot, 2u);
    CHECK_EQ_U64(nes->ppu.vblank_suppress, 0u);
    nesturbator__ppu_run_until(nes, nes->ticks + 8u);
    CHECK_EQ_U64(nes->ppu.dot, 3u);
    CHECK_EQ_U64(nes->ppu.status & 0x80u, 0u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 0u);
    CHECK_EQ_U64(nes->ppu.vblank_suppress, 0u);
    printf("CASE guard flag-suppressed ordinary-read M2-rise=241:0 result=00 nmi=00\n");
    nesturbator_destroy((nesturbator *)nes);

    nes = nmi_fixture_build(idle, sizeof idle, NULL, 0u, 0x8000u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;
    nmi_fixture_at(nes, 241u, 1u, 0x80u, 0x80u, 0x15u);
    nes->cpu.nmi_pending = 1u;
    value = nesturbator__bus_read(nes, 0x3ffau);
    CHECK_EQ_HEX(value & 0x80u, 0x80u);
    CHECK_EQ_U64(nes->ppu.status & 0x80u, 0u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 1u);
    printf("CASE guard flag-observed mirrored-read result=80 captured-request-retained\n");
    nesturbator_destroy((nesturbator *)nes);
}

static void test_captured_request_survives_clear(void)
{
    static const uint8_t handler[] = {0xe6u, 0x02u, 0x40u}; /* INC $02; RTI */
    struct nesturbator *nes = nmi_fixture_build(NULL, 0u, handler, sizeof handler, 0x9000u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    nmi_fixture_at(nes, 241u, 1u, 0x80u, 0x80u, 0x15u);
    uint8_t value = nesturbator__bus_read(nes, 0x3ffau); /* real mirrored $2002 decoder */
    CHECK_EQ_HEX(value & 0x80u, 0x80u);
    CHECK_EQ_HEX(value & 0x1fu, 0x15u);
    CHECK_EQ_U64(nes->ppu.status & 0x80u, 0u);
    CHECK_EQ_U64(nesturbator__ppu_nmi_line(nes), 0u);
    CHECK_EQ_U64(nes->cpu.nmi_prev, 1u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 1u);

    size_t state_size = nesturbator_state_size((const nesturbator *)nes);
    CHECK(state_size != 0u);
    uint8_t *before = (uint8_t *)malloc(state_size);
    uint8_t *roundtrip = (uint8_t *)malloc(state_size);
    uint8_t *first_resume = (uint8_t *)malloc(state_size);
    uint8_t *second_resume = (uint8_t *)malloc(state_size);
    uint8_t *old_format = (uint8_t *)malloc(state_size);
    CHECK(before != NULL && roundtrip != NULL && first_resume != NULL && second_resume != NULL &&
          old_format != NULL);
    if (before == NULL || roundtrip == NULL || first_resume == NULL || second_resume == NULL ||
        old_format == NULL) {
        free(before);
        free(roundtrip);
        free(first_resume);
        free(second_resume);
        free(old_format);
        nesturbator_destroy((nesturbator *)nes);
        return;
    }

    CHECK_EQ_U64(nesturbator_save_state((const nesturbator *)nes, before, state_size),
                 NESTURBATOR_OK);
    memcpy(old_format, before, state_size);
    old_format[8] = 1u;
    old_format[9] = 0u;
    old_format[10] = 63u; /* second defect proves format refusal precedes content checks */
    old_format[11] = 0u;
    uint32_t old_format_crc = nesturbator__crc32(0u, old_format, 28u);
    old_format_crc = nesturbator__crc32(old_format_crc, old_format + 32u, state_size - 32u);
    for (unsigned i = 0u; i < 4u; i++)
        old_format[28u + i] = (uint8_t)(old_format_crc >> (8u * i));
    struct nesturbator unchanged = *nes;
    CHECK_EQ_U64(nesturbator_load_state((nesturbator *)nes, old_format, state_size),
                 NESTURBATOR_ERR_STATE_FORMAT);
    CHECK(memcmp(&unchanged, nes, sizeof unchanged) == 0);
    CHECK_EQ_U64(nesturbator_save_state((const nesturbator *)nes, roundtrip, state_size),
                 NESTURBATOR_OK);
    CHECK(memcmp(before, roundtrip, state_size) == 0);
    printf(
        "TRACE decoded-status-capture addr=3ffa returned=%02x line=%u prev=%u pending=%u "
        "pc=%04x opcode=4c marker=%u ticks=%llu event=241:1 format1=refused-instance=unchanged\n",
        value, (unsigned)nesturbator__ppu_nmi_line(nes), (unsigned)nes->cpu.nmi_prev,
        (unsigned)nes->cpu.nmi_pending, nes->cpu.pc, (unsigned)nes->bus.ram[2],
        (unsigned long long)nes->ticks);
    nesturbator__cpu_step(nes); /* NMI service */
    CHECK_EQ_U64(nes->cpu.pc, 0x9000u);
    nesturbator__cpu_step(nes); /* guest marker: INC $02 */
    nesturbator__cpu_step(nes); /* RTI */
    CHECK_EQ_U64(nes->bus.ram[2], 1u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 0u);
    CHECK_EQ_U64(nesturbator_save_state((const nesturbator *)nes, first_resume, state_size),
                 NESTURBATOR_OK);

    CHECK_EQ_U64(nesturbator_load_state((nesturbator *)nes, before, state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nes->ppu.status & 0x80u, 0u);
    CHECK_EQ_U64(nes->cpu.nmi_prev, 1u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 1u);
    CHECK_EQ_U64(nesturbator_save_state((const nesturbator *)nes, roundtrip, state_size),
                 NESTURBATOR_OK);
    CHECK(memcmp(before, roundtrip, state_size) == 0);
    nesturbator__cpu_step(nes);
    CHECK_EQ_U64(nes->cpu.pc, 0x9000u);
    nesturbator__cpu_step(nes);
    nesturbator__cpu_step(nes);
    CHECK_EQ_U64(nes->bus.ram[2], 1u);
    CHECK_EQ_U64(nesturbator_save_state((const nesturbator *)nes, second_resume, state_size),
                 NESTURBATOR_OK);
    CHECK(memcmp(first_resume, second_resume, state_size) == 0);
    printf("TRACE decoded-status-resume marker=%u line=%u prev=%u pending=%u ticks=%llu "
           "state=byte-identical-after-load\n",
           (unsigned)nes->bus.ram[2], (unsigned)nesturbator__ppu_nmi_line(nes),
           (unsigned)nes->cpu.nmi_prev, (unsigned)nes->cpu.nmi_pending,
           (unsigned long long)nes->ticks);

    free(before);
    free(roundtrip);
    free(first_resume);
    free(second_resume);
    free(old_format);
    nesturbator_destroy((nesturbator *)nes);
}

static struct nesturbator *capture_control_nes;

static void expect_capture_revoked_control(void)
{
    uint8_t value = nesturbator__bus_read(capture_control_nes, 0x3ffau);
    CHECK_EQ_HEX(value & 0x80u, 0x80u);
    CHECK_EQ_U64(nesturbator__ppu_nmi_line(capture_control_nes), 0u);
    nmi_expected_assertion("captured-request-retained", capture_control_nes->cpu.nmi_pending == 0u);
}

static void test_wrong_capture_revoke_control(void)
{
    static const uint8_t handler[] = {0xe6u, 0x02u, 0x40u}; /* INC $02; RTI */
    capture_control_nes = nmi_fixture_build(NULL, 0u, handler, sizeof handler, 0x9000u);
    CHECK(capture_control_nes != NULL);
    if (capture_control_nes == NULL)
        return;
    nmi_fixture_at(capture_control_nes, 241u, 1u, 0x80u, 0x80u, 0x15u);
    nmi_expect_red("production-path-revoke-captured-request", "captured-request-retained",
                   expect_capture_revoked_control);
    nesturbator_destroy((nesturbator *)capture_control_nes);
    capture_control_nes = NULL;
}

static void expect_unchanged_enable_extra_edge_control(void)
{
    nesturbator__bus_write(capture_control_nes, 0x3ff8u, 0x80u);
    nmi_expected_assertion("unchanged-enable-no-new-edge",
                           capture_control_nes->cpu.nmi_pending != 0u);
}

static void test_wrong_unchanged_enable_control(void)
{
    capture_control_nes = nmi_fixture_build(NULL, 0u, NULL, 0u, 0x8000u);
    CHECK(capture_control_nes != NULL);
    if (capture_control_nes == NULL)
        return;
    nmi_fixture_at(capture_control_nes, 241u, 10u, 0x80u, 0x80u, 0x15u);
    capture_control_nes->cpu.nmi_prev = 1u;
    nmi_expect_red("production-path-unchanged-enable-extra-edge", "unchanged-enable-no-new-edge",
                   expect_unchanged_enable_extra_edge_control);
    nesturbator_destroy((nesturbator *)capture_control_nes);
    capture_control_nes = NULL;
}

static void test_control_write_decoder(uint16_t address, const char *case_name)
{
    static const uint8_t idle[] = {0x4cu, 0x00u, 0x80u};
    struct nesturbator *nes = nmi_fixture_build(idle, sizeof idle, NULL, 0u, 0x8000u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    nmi_fixture_at(nes, 241u, 8u, 0x80u, 0u, 0u);
    uint64_t start_cycle = nes->cpu_cycle;
    nesturbator__bus_write(nes, address, 0x80u);
    CHECK_EQ_U64(nes->ppu.control, 0x80u);
    CHECK_EQ_U64(nesturbator__ppu_nmi_line(nes), 1u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 0u);
    CHECK_EQ_U64(nes->cpu_cycle - start_cycle, 1u);
    nesturbator__bus_write(nes, address, 0x80u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 1u);
    nesturbator__bus_write(nes, address, 0x00u);
    CHECK_EQ_U64(nesturbator__ppu_nmi_line(nes), 0u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 1u);
    nesturbator__cpu_step(nes); /* consume the captured request */
    CHECK_EQ_U64(nes->cpu.nmi_pending, 0u);
    (void)nesturbator__bus_read(nes, 0x0000u); /* sample the lowered line */
    nesturbator__bus_write(nes, address, 0x80u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 0u);
    CHECK_EQ_U64(nesturbator__ppu_nmi_line(nes), 1u);
    (void)nesturbator__bus_read(nes, 0x0000u); /* capture the re-enabled line */
    CHECK_EQ_U64(nes->cpu.nmi_pending, 1u);
    CHECK_EQ_U64(nes->cpu_cycle - start_cycle, 13u);
    printf("CASE guard %s line-enable=1 delayed-capture=1 retained-on-disable=1 "
           "re-enable=1\n",
           case_name);
    nesturbator_destroy((nesturbator *)nes);
}

struct nmi_circuit_waveform {
    const char *name;
    uint16_t pin_low_at;
    uint16_t pin_high_at;
    uint8_t expected_close_line[2];
    uint8_t expected_request;
};

static uint8_t waveform_control_wrong_capture;
static const char *waveform_control_assertion;

static void expect_waveform_wrong_capture_control(void)
{
    nmi_expected_assertion(waveform_control_assertion, waveform_control_wrong_capture != 0u);
}

static int circuit_pin_low_at(const struct nmi_circuit_waveform *waveform, uint16_t tick)
{
    return waveform->pin_low_at != UINT16_MAX && tick >= waveform->pin_low_at &&
           tick < waveform->pin_high_at;
}

static int circuit_pulse_overlaps(const struct nmi_circuit_waveform *waveform, uint16_t start,
                                  uint16_t end)
{
    return waveform->pin_low_at != UINT16_MAX && waveform->pin_low_at < end &&
           waveform->pin_high_at > start;
}

static void set_projected_nmi_pin(struct nesturbator *nes, int pin_low)
{
    /* The circuit pad is active-low. The core's future derived line is the
       asserted-high view of PPUSTATUS.7 && PPUCTRL.7. */
    nes->ppu.control = 0x80u;
    nes->ppu.status = pin_low != 0 ? 0x80u : 0u;
}

static void test_circuit_phase_projection(void)
{
    static const uint8_t handler[] = {0xe6u, 0x02u, 0x40u}; /* INC $02; RTI */
    static const struct nmi_circuit_waveform waveforms[] = {
        {"before-fall", 2424u, 2432u, {1u, 0u}, 1u},
        {"after-fall-phi1-short", 2432u, 2436u, {0u, 0u}, 0u},
        {"phi2-short-released-before-close", 2420u, 2424u, {0u, 0u}, 0u},
        {"widened-m2-only", 2438u, 2439u, {0u, 0u}, 0u},
        {"held-through-next-close", 2432u, 2456u, {0u, 1u}, 1u},
        {"no-pulse", UINT16_MAX, UINT16_MAX, {0u, 0u}, 0u},
    };
    static const uint16_t sample_close_ticks[] = {2428u, 2452u};

    for (size_t case_index = 0u; case_index < sizeof waveforms / sizeof waveforms[0];
         case_index++) {
        const struct nmi_circuit_waveform *waveform = &waveforms[case_index];
        struct nesturbator *nes = nmi_fixture_build(NULL, 0u, handler, sizeof handler, 0x9000u);
        CHECK(nes != NULL);
        if (nes == NULL)
            continue;

        /* This is a circuit-stage projection, not a whole-bus waveform replay:
           feed the measured pin level at the two internal-phi2 close events
           into the production bus-cycle sampler. It does not model NMIP's
           transistor transitions, analog setup/hold, PPU output propagation,
           or the paired-chip tie. */
        nmi_fixture_at(nes, 200u, 100u, 0u, 0x80u, 0u);
        nes->ticks = 2404u;
        nes->ppu.ppu_ticks = 2404u;

        unsigned projected_captures = 0u;
        for (size_t sample_index = 0u; sample_index < 2u; sample_index++) {
            uint16_t close_tick = sample_close_ticks[sample_index];
            int pin_low = circuit_pin_low_at(waveform, close_tick);
            uint8_t asserted_line = (uint8_t)(pin_low != 0);
            uint8_t previous_line = nes->cpu.nmi_prev;
            uint8_t expected_capture = (uint8_t)(asserted_line != 0u && previous_line == 0u);
            set_projected_nmi_pin(nes, pin_low);
            CHECK_EQ_U64(asserted_line, waveform->expected_close_line[sample_index]);

            (void)nesturbator__bus_read(nes, 0x0000u);

            projected_captures += expected_capture;
            CHECK_EQ_U64(nes->cpu.nmi_prev, asserted_line);
            CHECK_EQ_U64(nes->cpu.nmi_pending, projected_captures != 0u);
            if (case_index == 2u && sample_index == 0u) {
                /* Diagnostic comparator: an implementation that latches any
                   transient in internal phi2 would retain this pulse even
                   though the production close sample is low. */
                CHECK_EQ_U64(circuit_pulse_overlaps(waveform, close_tick - 12u, close_tick), 1u);
                CHECK_EQ_U64(waveform->expected_close_line[sample_index], 0u);
                CHECK_EQ_U64(expected_capture, 0u);
                waveform_control_wrong_capture = nes->cpu.nmi_pending;
                waveform_control_assertion = "pre-close-transient-not-captured";
                nmi_expect_red("production-projection-pre-close-transient-capture",
                               waveform_control_assertion, expect_waveform_wrong_capture_control);
                printf("CONTROL production-path expected-red pre-close-transient-not-captured "
                       "overlaps-phi2-window=1 close-sample=0 result=not-captured\n");
            }
            if (case_index == 3u && sample_index == 1u) {
                /* Diagnostic comparator: the widened M2 window contains this
                   short pulse, but the phase-close sample must miss it. */
                CHECK_EQ_U64(circuit_pulse_overlaps(waveform, close_tick - 15u, close_tick), 1u);
                CHECK_EQ_U64(waveform->expected_close_line[sample_index], 0u);
                CHECK_EQ_U64(expected_capture, 0u);
                waveform_control_wrong_capture = nes->cpu.nmi_pending;
                waveform_control_assertion = "widened-m2-only-pulse-not-captured";
                nmi_expect_red("production-projection-widened-m2-only-capture",
                               waveform_control_assertion, expect_waveform_wrong_capture_control);
                printf("CONTROL production-path expected-red widened-m2-only-pulse-not-captured "
                       "overlaps-m2-window=1 close-sample=0 result=not-captured\n");
            }
            printf("TRACE circuit-stage-projection case=%s close=%u phi2_open=%u "
                   "m2_rise=%u pin_low=%u asserted=%u captured=%u prev=%u pending=%u "
                   "ticks=%llu pc=%04x opcode=4c marker=%u\n",
                   waveform->name, (unsigned)close_tick, (unsigned)(close_tick - 12u),
                   (unsigned)(close_tick - 15u), (unsigned)(pin_low != 0), (unsigned)asserted_line,
                   (unsigned)expected_capture, (unsigned)nes->cpu.nmi_prev,
                   (unsigned)nes->cpu.nmi_pending, (unsigned long long)nes->ticks, nes->cpu.pc,
                   (unsigned)nes->bus.ram[2]);
        }

        CHECK_EQ_U64(projected_captures, waveform->expected_request);
        CHECK_EQ_U64(nes->cpu.nmi_pending, waveform->expected_request);

        /* Release the modeled pin before guest polling. A recognized request
           remains latched after the physical line falls. */
        set_projected_nmi_pin(nes, 0);
        if (waveform->expected_request != 0u) {
            nesturbator__cpu_step(nes);
            CHECK_EQ_U64(nes->cpu.pc, 0x9000u);
            nesturbator__cpu_step(nes);
            nesturbator__cpu_step(nes);
        } else {
            for (unsigned step = 0u; step < 3u; step++)
                nesturbator__cpu_step(nes);
        }
        for (unsigned step = 0u; step < 8u; step++)
            nesturbator__cpu_step(nes);
        CHECK_EQ_U64(nes->bus.ram[2], waveform->expected_request);
        printf("TRACE circuit-stage-projection case=%s eventual_handler_count=%u "
               "final_pending=%u final_prev=%u\n",
               waveform->name, (unsigned)nes->bus.ram[2], (unsigned)nes->cpu.nmi_pending,
               (unsigned)nes->cpu.nmi_prev);
        nesturbator_destroy((nesturbator *)nes);
    }
}

static int bytes_equal(const uint8_t *actual, const uint8_t *expected, size_t count)
{
    for (size_t i = 0u; i < count; i++) {
        if (actual[i] != expected[i])
            return 0;
    }
    return 1;
}

/* The source's two permitted vectors are whole-array alternatives, never a
   per-sample offset choice. [AccuracyCoin.asm:1839-1900, pinned at 6bee919…] */
static int flag_clear_table_accepts(const uint8_t values[4])
{
    static const uint8_t primary[4] = {0xe0u, 0xe0u, 0x80u, 0x00u};
    static const uint8_t shifted[4] = {0xe0u, 0x80u, 0x80u, 0x00u};
    return bytes_equal(values, primary, 4u) || bytes_equal(values, shifted, 4u);
}

/* AccuracyCoin.asm:1862-1887 masks with E0 and checks $53, $54, $55
   independently: 00, {00,40}, 60. */
static int flag_set_table_accepts(const uint8_t values[3])
{
    return values[0] == 0x00u && (values[1] == 0x00u || values[1] == 0x40u) && values[2] == 0x60u;
}

/* AccuracyCoin.asm:6688-6718 compares ten cells against one of two complete
   sequences; the source's eleventh byte supports the shifted ten-byte view. */
static int nmi_timing_table_accepts(const uint8_t values[10])
{
    static const uint8_t first[10] = {3u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 1u, 1u};
    static const uint8_t shifted[10] = {2u, 2u, 2u, 2u, 2u, 2u, 1u, 1u, 1u, 1u};
    return bytes_equal(values, first, 10u) || bytes_equal(values, shifted, 10u);
}

/* AccuracyCoin.asm:6742-6767 starts at x=1 and skips x=3 and x=6. */
static int suppression_table_accepts(const uint8_t values[10])
{
    static const uint8_t expected[10] = {0xffu, 2u, 2u, 0xffu, 0u, 1u, 0xffu, 3u, 3u, 3u};
    for (size_t i = 1u; i < 10u; i++) {
        if (i == 3u || i == 6u)
            continue;
        if (values[i] != expected[i])
            return 0;
    }
    return 1;
}

/* Both actual instruction sequences in AccuracyCoin.asm:6775-6839 compare
   seven entries with [01,01,01,00,00,00,00]. */
static int vblank_end_table_accepts(const uint8_t values[7])
{
    static const uint8_t expected[7] = {1u, 1u, 1u, 0u, 0u, 0u, 0u};
    return bytes_equal(values, expected, 7u);
}

/* AccuracyCoin.asm:6842-6886 skips only x=3 (the FF cell). */
static int disabled_vblank_table_accepts(const uint8_t values[7])
{
    static const uint8_t expected[7] = {0u, 0u, 0u, 0xffu, 1u, 1u, 1u};
    for (size_t i = 0u; i < 7u; i++) {
        if (i != 3u && values[i] != expected[i])
            return 0;
    }
    return 1;
}

static int status_bits_match(uint8_t expected, uint8_t observed)
{
    return (expected & 0xe0u) == (observed & 0xe0u);
}

static int integer_observation_matches(unsigned expected, unsigned observed)
{
    return expected == observed;
}

static void test_source_table_oracles(void)
{
    /* AccuracyCoin.asm:6775-6814 sequence A is EnableNMI, TYA, store, then
       DisableNMI. Lines 6800-6814 sequence B is LDA #$80; STA $2000; NOP;
       NOP; LDA #$00; STA $2000; TYA; store. The source runs both sequences
       and independently compares each with the same seven-byte vector. */
    static const uint8_t clear_primary[4] = {0xe0u, 0xe0u, 0x80u, 0u};
    static const uint8_t clear_shifted[4] = {0xe0u, 0x80u, 0x80u, 0u};
    static const uint8_t clear_mixed[4] = {0xe0u, 0x80u, 0u, 0u};
    static const uint8_t timing_first[10] = {3u, 2u, 2u, 2u, 2u, 2u, 2u, 1u, 1u, 1u};
    static const uint8_t timing_shifted[10] = {2u, 2u, 2u, 2u, 2u, 2u, 1u, 1u, 1u, 1u};
    static const uint8_t timing_mixed[10] = {3u, 2u, 2u, 2u, 2u, 2u, 1u, 1u, 1u, 1u};
    static const uint8_t suppression[10] = {0x55u, 2u, 2u, 0x44u, 0u, 1u, 0x33u, 3u, 3u, 3u};
    static const uint8_t end_first[7] = {1u, 1u, 1u, 0u, 0u, 0u, 0u};
    static const uint8_t end_second[7] = {1u, 1u, 1u, 0u, 0u, 0u, 0u};
    static const uint8_t end_wrong[7] = {1u, 1u, 1u, 1u, 0u, 0u, 0u};
    static const uint8_t disabled[7] = {0u, 0u, 0u, 0xffu, 1u, 1u, 1u};
    static const uint8_t disabled_skipped[7] = {0u, 0u, 0u, 0x44u, 1u, 1u, 1u};
    static const uint8_t disabled_wrong[7] = {0u, 0u, 0u, 0xffu, 0u, 1u, 1u};
    const uint8_t set_00[3] = {0u, 0u, 0x60u};
    const uint8_t set_40[3] = {0u, 0x40u, 0x60u};
    const uint8_t set_wrong[3] = {0u, 0x80u, 0x60u};

    CHECK(flag_clear_table_accepts(clear_primary));
    CHECK(flag_clear_table_accepts(clear_shifted));
    CHECK(!flag_clear_table_accepts(clear_mixed));
    CHECK(flag_set_table_accepts(set_00));
    CHECK(flag_set_table_accepts(set_40));
    CHECK(!flag_set_table_accepts(set_wrong));
    CHECK(nmi_timing_table_accepts(timing_first));
    CHECK(nmi_timing_table_accepts(timing_shifted));
    CHECK(!nmi_timing_table_accepts(timing_mixed));
    CHECK(suppression_table_accepts(suppression));
    uint8_t suppression_wrong[10];
    memcpy(suppression_wrong, suppression, sizeof suppression_wrong);
    suppression_wrong[7] = 2u;
    CHECK(!suppression_table_accepts(suppression_wrong));
    CHECK(vblank_end_table_accepts(end_first));
    CHECK(vblank_end_table_accepts(end_second));
    CHECK(!vblank_end_table_accepts(end_wrong));
    CHECK(disabled_vblank_table_accepts(disabled));
    CHECK(disabled_vblank_table_accepts(disabled_skipped));
    CHECK(!disabled_vblank_table_accepts(disabled_wrong));
    printf("CASE guard source-table oracles clear/set/timing/suppression/end/end/disabled\n");
}

static void test_wrong_model_controls_are_oracle_distinct(void)
{
    /* These comparator checks are diagnostic only. Production-path expected-
       red controls separately exercise wrong input expectations. */
    CHECK(!status_bits_match(0x60u, 0xe0u)); /* +1-dot set / all-at-fall */
    CHECK(!status_bits_match(0xe0u, 0x00u)); /* -1-dot clear */
    CHECK(!status_bits_match(0x80u, 0xe0u)); /* merge all status bits at M2 rise */
    CHECK(!nmi_red_result_matches("captured", "revoked", 1u, 1u));
    CHECK(!integer_observation_matches(1u, 2u));       /* unchanged $80 must not add an NMI */
    CHECK(!integer_observation_matches(6u, 5u));       /* INC $2000 must keep both writes */
    CHECK(!integer_observation_matches(0x11u, 0x10u)); /* Control 8 must not service before LDX */
    printf("CONTROL diagnostic-only set-shift intended=status-dot\n");
    printf("CONTROL diagnostic-only clear-shift intended=status-dot\n");
    printf("CONTROL diagnostic-only merged-status-edges intended=split-status\n");
    printf("CONTROL diagnostic-only revoke-captured-request intended=pending-history\n");
    printf("CONTROL diagnostic-only unchanged-enable-extra-edge intended=guest-counter\n");
    printf("CONTROL diagnostic-only omitted-rmw-write intended=cpu-cycle-count\n");
    printf("CONTROL diagnostic-only early-nmi-service intended=instruction-marker\n");
}

static void test_expected_red_helper_rejects_unrelated_assertions(void)
{
    CHECK(nmi_red_result_matches("status-bit7", "status-bit7", 1u, 1u));
    CHECK(!nmi_red_result_matches("status-bit7", "setup-check", 1u, 1u));
    CHECK(!nmi_red_result_matches("status-bit7", "status-bit7", 2u, 2u));
    CHECK(!nmi_red_result_matches("status-bit7", "status-bit7", 1u, 2u));
    printf("CASE guard expected-red rejects setup and different assertion identities\n");
}

int main(void)
{
    test_expected_red_helper_rejects_unrelated_assertions();
    test_flag_set_before_on_after_dot();
    test_flag_clear_before_on_after_dot();
    test_outside_window_control();
    test_ram_mirrors_do_not_select_status();
    test_dmc_parked_ram_read_does_not_select_status();
    assert_m2_split_witness(240u, 241u, 2u, 0x80u);
    assert_m2_split_witness(260u, 261u, 2u, 0u);
    test_set_race_read(0x2002u, "set-boundary-ordinary");
    test_set_race_read(0x3ffau, "set-boundary-mirrored");
    test_clear_race_read(0x2002u, "clear-boundary-ordinary");
    test_clear_race_read(0x3ffau, "clear-boundary-mirrored");
    test_wrong_status_phase_controls();
    test_nmi_suppression_cases();
    test_captured_request_survives_clear();
    test_wrong_capture_revoke_control();
    test_wrong_unchanged_enable_control();
    test_control_write_decoder(0x2000u, "control-write-ordinary");
    test_control_write_decoder(0x3ff8u, "control-write-mirrored");
    test_circuit_phase_projection();
    test_source_table_oracles();
    test_wrong_model_controls_are_oracle_distinct();
    CHECK_DONE();
}
