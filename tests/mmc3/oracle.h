/* Judgement of blargg's two MMC3 result protocols (09 D-11), shared by the
   mmc3-oracle executable and its offline unit test. Pure functions on bytes
   the caller has read; nothing here touches an instance.

   Exit codes of mmc3-oracle: 0 is a pass. 1 to 0x7F is the result the ROM
   reported. 120 the ROM never reported within the budget, 121 the ROM wrote no
   $6000 signature, 122 the image did not load, 123 bad usage. 77 is never used. */
#ifndef NESTURBATOR_TESTS_MMC3_ORACLE_H
#define NESTURBATOR_TESTS_MMC3_ORACLE_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define MMC3_ORACLE_EXIT_TIMEOUT 120
#define MMC3_ORACLE_EXIT_NO_SIGNATURE 121
#define MMC3_ORACLE_EXIT_LOAD 122
#define MMC3_ORACLE_EXIT_USAGE 123

/* mmc3_test_2 writes blargg's $6000 protocol: signature DE B0 61 at
   $6001-$6003, status at $6000 ($80 running, $81 needs a reset, $00-$7F the
   result) and zero-terminated text from $6004. head is $6000-$6003 and text
   is $6004 onward, zero-terminated by the caller. seen_running is non-zero if
   $80 was seen at $6000 before this read.
   Returns -1 when the test is still running (call again after more frames),
   otherwise the exit code: 0 for $00 with $80 seen before and "Passed" in the
   text; the reported value for 1 to 0x7F; NO_SIGNATURE when the signature is
   absent, or $00 is reported without a prior $80 or without "Passed";
   TIMEOUT for $81. */
static inline int mmc3_oracle_judge_6000(const uint8_t head[4], const char *text, int seen_running)
{
    if (head[1] != 0xDEu || head[2] != 0xB0u || head[3] != 0x61u)
        return MMC3_ORACLE_EXIT_NO_SIGNATURE;
    if (head[0] == 0x80u)
        return -1;
    if (head[0] == 0x81u)
        return MMC3_ORACLE_EXIT_TIMEOUT;
    if (head[0] == 0u)
        return (seen_running != 0 && strstr(text, "Passed") != NULL)
                   ? 0
                   : MMC3_ORACLE_EXIT_NO_SIGNATURE;
    if (head[0] < 0x80u)
        return (int)head[0];
    return MMC3_ORACLE_EXIT_NO_SIGNATURE;
}

/* The final call for a ROM that never reported: still running at the budget. */
static inline int mmc3_oracle_judge_6000_budget(const uint8_t head[4])
{
    if (head[1] != 0xDEu || head[2] != 0xB0u || head[3] != 0x61u)
        return MMC3_ORACLE_EXIT_NO_SIGNATURE;
    return MMC3_ORACLE_EXIT_TIMEOUT;
}

/* mmc3_irq_tests keep their result in zero page $F8 and store 1 on success.
   at_budget is $F8 when the budget ended, after_more is $F8 after 60 further
   frames. 0 when both are 1. Otherwise the reported failure value (2 to 0x7F,
   from after_more, else from at_budget), else TIMEOUT. */
static inline int mmc3_oracle_judge_f8(uint8_t at_budget, uint8_t after_more)
{
    if (at_budget == 1u && after_more == 1u)
        return 0;
    if (after_more >= 2u && after_more < 0x80u)
        return (int)after_more;
    if (at_budget >= 2u && at_budget < 0x80u)
        return (int)at_budget;
    return MMC3_ORACLE_EXIT_TIMEOUT;
}

#endif
