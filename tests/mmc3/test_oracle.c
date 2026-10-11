/* mmc3.oracle.unit: the judgement of mmc3-oracle on synthetic memory, offline (09 D-11). Every
   refusal that keeps a stale or partial result from passing has a case here. Nothing fetches. */
#include <stdint.h>

#include "../check.h"
#include "oracle.h"

static const uint8_t sig_running[4] = {0x80u, 0xDEu, 0xB0u, 0x61u};
static const uint8_t sig_pass[4] = {0x00u, 0xDEu, 0xB0u, 0x61u};
static const uint8_t sig_fail3[4] = {0x03u, 0xDEu, 0xB0u, 0x61u};
static const uint8_t sig_reset[4] = {0x81u, 0xDEu, 0xB0u, 0x61u};
static const uint8_t sig_missing[4] = {0x00u, 0x00u, 0x00u, 0x00u};

static void test_6000(void)
{
    /* Running is not final. */
    CHECK_EQ_U64((unsigned)(mmc3_oracle_judge_6000(sig_running, "", 0) + 1), 0u);
    /* $80 seen, then 0 with "Passed": a pass. */
    CHECK_EQ_U64(mmc3_oracle_judge_6000(sig_pass, "\nPassed\n", 1), 0u);
    /* Final 0 without $80 ever seen is a stale result, not a pass. */
    CHECK_EQ_U64(mmc3_oracle_judge_6000(sig_pass, "\nPassed\n", 0), MMC3_ORACLE_EXIT_NO_SIGNATURE);
    /* Text without "Passed" is not a pass. */
    CHECK_EQ_U64(mmc3_oracle_judge_6000(sig_pass, "\nFailed\n", 1), MMC3_ORACLE_EXIT_NO_SIGNATURE);
    /* A missing signature is no-signature. */
    CHECK_EQ_U64(mmc3_oracle_judge_6000(sig_missing, "Passed", 1), MMC3_ORACLE_EXIT_NO_SIGNATURE);
    /* A reported failure is its value. */
    CHECK_EQ_U64(mmc3_oracle_judge_6000(sig_fail3, "", 1), 3u);
    /* $81 asks for a reset, which this lane cannot do: a timeout. */
    CHECK_EQ_U64(mmc3_oracle_judge_6000(sig_reset, "", 1), MMC3_ORACLE_EXIT_TIMEOUT);
    /* Still running at the budget is a timeout; no signature at the budget is no-signature. */
    CHECK_EQ_U64(mmc3_oracle_judge_6000_budget(sig_running), MMC3_ORACLE_EXIT_TIMEOUT);
    CHECK_EQ_U64(mmc3_oracle_judge_6000_budget(sig_missing), MMC3_ORACLE_EXIT_NO_SIGNATURE);
}

static void test_f8(void)
{
    /* $F8 == 1 and still 1 after 60 more frames is a pass. */
    CHECK_EQ_U64(mmc3_oracle_judge_f8(1u, 1u), 0u);
    /* 1 that turns into a failure value within 60 frames is that failure. */
    CHECK_EQ_U64(mmc3_oracle_judge_f8(1u, 3u), 3u);
    /* A failure value at the budget is reported. */
    CHECK_EQ_U64(mmc3_oracle_judge_f8(2u, 2u), 2u);
    /* Never written, or still the running value $80, is a timeout. */
    CHECK_EQ_U64(mmc3_oracle_judge_f8(0u, 0u), MMC3_ORACLE_EXIT_TIMEOUT);
    CHECK_EQ_U64(mmc3_oracle_judge_f8(0x80u, 0x80u), MMC3_ORACLE_EXIT_TIMEOUT);
    /* 1 only after the budget is not a pass. */
    CHECK_EQ_U64(mmc3_oracle_judge_f8(0u, 1u), MMC3_ORACLE_EXIT_TIMEOUT);
}

int main(void)
{
    test_6000();
    test_f8();
    CHECK_DONE();
}
