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

static void test_nmi_suppression_cases(void)
{
    static const uint8_t idle[] = {0x4cu, 0x00u, 0x80u};
    struct nesturbator *nes = nmi_fixture_build(idle, sizeof idle, NULL, 0u, 0x8000u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    /* A real $2002 bus read ends at (241,0), one PPU dot before the set event. */
    nmi_fixture_at(nes, 240u, 338u, 0u, 0x80u, 0x15u);
    uint8_t value = nesturbator__bus_read(nes, 0x2002u);
    CHECK_EQ_HEX(value, 0x15u);
    CHECK_EQ_U64(nes->ppu.scanline, 241u);
    CHECK_EQ_U64(nes->ppu.dot, 0u);
    CHECK_EQ_U64(nes->ppu.vblank_suppress, 1u);
    nesturbator__ppu_run_until(nes, nes->ticks + 8u);
    CHECK_EQ_U64(nes->ppu.dot, 1u);
    CHECK_EQ_U64(nes->ppu.status & 0x80u, 0u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 0u);
    CHECK_EQ_U64(nes->ppu.vblank_suppress, 0u);
    printf("CASE guard flag-suppressed ordinary-read before=241:1 result=00 nmi=00\n");
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
    CHECK_EQ_U64(nes->cpu.nmi_pending, 0u);
    printf("CASE guard flag-observed mirrored-read result=80 nmi-suppressed\n");
    nesturbator_destroy((nesturbator *)nes);
}

static void captured_request_survives_vblank_clear_case(void)
{
    nesturbator__ppu_run_until(split_read_nes, split_read_nes->ticks + 8u);
    CHECK_EQ_U64(split_read_nes->ppu.status & 0x80u, 0u);
    nmi_expected_assertion("captured-nmi-survives-pre-render-line-lowering",
                           split_read_nes->cpu.nmi_pending == 1u);
}

static void test_captured_request_survives_clear(void)
{
    static const uint8_t idle[] = {0x4cu, 0x00u, 0x80u};
    split_read_nes = nmi_fixture_build(idle, sizeof idle, NULL, 0u, 0x8000u);
    CHECK(split_read_nes != NULL);
    if (split_read_nes == NULL)
        return;
    nmi_fixture_at(split_read_nes, 261u, 0u, 0x80u, 0x80u, 0x15u);
    split_read_nes->cpu.nmi_pending = 1u; /* captured before the line falls */
    nmi_expect_red("captured-request-survives-vblank-clear",
                   "captured-nmi-survives-pre-render-line-lowering",
                   captured_request_survives_vblank_clear_case);
    nesturbator_destroy((nesturbator *)split_read_nes);
    split_read_nes = NULL;
}

static void test_control_write_decoder(uint16_t address, const char *case_name)
{
    static const uint8_t idle[] = {0x4cu, 0x00u, 0x80u};
    struct nesturbator *nes = nmi_fixture_build(idle, sizeof idle, NULL, 0u, 0x8000u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    nmi_fixture_at(nes, 241u, 8u, 0x80u, 0u, 0u);
    nesturbator__bus_write(nes, address, 0x80u);
    CHECK_EQ_U64(nes->ppu.control, 0x80u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 1u);
    nesturbator__bus_write(nes, address, 0x80u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 1u);
    nesturbator__bus_write(nes, address, 0x00u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 0u);
    nesturbator__bus_write(nes, address, 0x80u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 1u);
    CHECK_EQ_U64(nes->cpu_cycle, 4u);
    printf("CASE guard %s enable=1 unchanged=1 disable=0 reenable=1\n", case_name);
    nesturbator_destroy((nesturbator *)nes);
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
    /* These local observations validate intended diagnostics only. They do not
       execute production mutants; that work remains for the causal fix plans. */
    CHECK(!status_bits_match(0x60u, 0xe0u)); /* +1-dot set / all-at-fall */
    CHECK(!status_bits_match(0xe0u, 0x00u)); /* -1-dot clear */
    CHECK(!status_bits_match(0x80u, 0xe0u)); /* merge all status bits at M2 rise */
    CHECK(!nmi_red_result_matches("captured", "revoked", 1u, 1u));
    CHECK(!integer_observation_matches(1u, 2u));       /* unchanged $80 must not add an NMI */
    CHECK(!integer_observation_matches(6u, 5u));       /* INC $2000 must keep both writes */
    CHECK(!integer_observation_matches(0x11u, 0x10u)); /* Control 8 must not service before LDX */
    printf("CONTROL awaiting-production-control set-shift intended=status-dot\n");
    printf("CONTROL awaiting-production-control clear-shift intended=status-dot\n");
    printf("CONTROL awaiting-production-control merged-status-edges intended=split-status\n");
    printf(
        "CONTROL awaiting-production-control revoke-captured-request intended=pending-history\n");
    printf(
        "CONTROL awaiting-production-control unchanged-enable-extra-edge intended=guest-counter\n");
    printf("CONTROL awaiting-production-control omitted-rmw-write intended=cpu-cycle-count\n");
    printf("CONTROL awaiting-production-control early-nmi-service intended=instruction-marker\n");
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
    assert_m2_split_witness(240u, 241u, 2u, 0x80u);
    assert_m2_split_witness(260u, 261u, 2u, 0u);
    test_set_race_read(0x2002u, "set-boundary-ordinary");
    test_set_race_read(0x3ffau, "set-boundary-mirrored");
    test_clear_race_read(0x2002u, "clear-boundary-ordinary");
    test_clear_race_read(0x3ffau, "clear-boundary-mirrored");
    test_nmi_suppression_cases();
    test_captured_request_survives_clear();
    test_control_write_decoder(0x2000u, "control-write-ordinary");
    test_control_write_decoder(0x3ff8u, "control-write-mirrored");
    test_source_table_oracles();
    test_wrong_model_controls_are_oracle_distinct();
    CHECK_DONE();
}
