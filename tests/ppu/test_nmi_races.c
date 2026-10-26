/* NMI-01: owned dot and real-CPU-bus probes for PPUSTATUS race ordering.
   AccuracyCoin at 6bee919529289c555803b612b8c9e4f7eaaeeabb, TEST_2002FlagTiming
   (AccuracyCoin.asm:1776-1793), states that bit 7 is sampled when M2 rises and
   sprite flags at M2 fall. The NTSC set/clear points are 241:1 and 261:1
   (NESdev PPU frame timing, revision cited by HWP.03). */
#include <stdint.h>
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
    CHECK_EQ_HEX(split_read_value & 0x60u, split_read_expected_sprite_flags);
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
    nmi_expect_red(case_name, "m2-rise-bit7-sample", split_read_case);
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
    nmi_expect_red(case_name, "m2-rise-bit7-sample", split_read_case);
    nesturbator_destroy((nesturbator *)split_read_nes);
    split_read_nes = NULL;
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
    assert_m2_split_witness(240u, 241u, 2u, 0x80u);
    assert_m2_split_witness(260u, 261u, 2u, 0u);
    test_set_race_read(0x2002u, "set-boundary-ordinary");
    test_set_race_read(0x3ffau, "set-boundary-mirrored");
    test_clear_race_read(0x2002u, "clear-boundary-ordinary");
    test_clear_race_read(0x3ffau, "clear-boundary-mirrored");
    CHECK_DONE();
}
