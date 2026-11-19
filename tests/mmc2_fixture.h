/* Shared owned mapper-9 image builder for core and client-parity tests. The
   marker values are checked against literal tables in each behavior test. */
#ifndef NESTURBATOR_TESTS_MMC2_FIXTURE_H
#define NESTURBATOR_TESTS_MMC2_FIXTURE_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ines.h"

#define MMC2_BANK_8K 8192u
#define MMC2_BANK_4K 4096u
#define MMC2_PAGE_1K 1024u
#define MMC2_PRG_SIZE (16u * MMC2_BANK_8K)
#define MMC2_CHR_SIZE (32u * MMC2_BANK_4K)
#define MMC2_FIXTURE_CAP (16u + MMC2_PRG_SIZE + MMC2_CHR_SIZE)
#define MMC2_IMAGE_CAP (MMC2_FIXTURE_CAP + INES_TRAINER_SIZE + 1u)

/* Build 16 PRG banks with eight independently marked 1 KiB pages each, 32
   CHR banks, and fixed-bank code/vectors. Expected marker tables stay outside
   this generator so they remain independent behavior oracles. */
static size_t mmc2_fixture_build(uint8_t *out, size_t cap, uint8_t vertical)
{
    /* Keep each 6502 instruction grouped with its byte operands and annotation. */
    // clang-format off
    static const uint8_t boot[] = {
        0x78u,                         /* SEI */
        0xa9u, 0x0cu, 0x8du, 0, 0xa0u, /* LDA #$0C; STA $A000: select PRG bank 12 */
        0xa9u, 0x05u, 0x8du, 0, 0xb0u, /* select $0000-$0FFF FD bank 5 */
        0xa9u, 0x03u, 0x8du, 0, 0xc0u, /* select $0000-$0FFF FE bank 3 */
        0xa9u, 0x09u, 0x8du, 0, 0xd0u, /* select $1000-$1FFF FD bank 9 */
        0xa9u, 0x07u, 0x8du, 0, 0xe0u, /* select $1000-$1FFF FE bank 7 */
        0xa9u, 0x01u, 0x8du, 0, 0xf0u, /* horizontal nametable mirroring */
        0xa9u, 0x3fu, 0x8du, 0x06u, 0x20u,
        0xa9u, 0x05u, 0x8du, 0x06u, 0x20u,
        0xa9u, 0x16u, 0x8du, 0x07u, 0x20u,
        0xa9u, 0x27u, 0x8du, 0x07u, 0x20u,
        0xa9u, 0x30u, 0x8du, 0x07u, 0x20u,
        0xa9u, 0x0fu, 0x8du, 0x07u, 0x20u,
        0xa9u, 0x20u, 0x8du, 0x06u, 0x20u,
        0xa9u, 0x00u, 0x8du, 0x06u, 0x20u,
        0xa2u, 0x04u,             /* LDX #4: write four nametable/attribute pages */
        0xa0u, 0x00u,             /* outer page loop starts with Y=0 */
        0xa9u, 0xfdu,             /* use tile $FD to drive the low MMC2 latch */
        0x8du, 0x07u, 0x20u,      /* fill: STA $2007 */
        0xc8u,                    /* INY */
        0xd0u, 0xfau,             /* BNE fill */
        0xcau,                    /* DEX */
        0xd0u, 0xf7u,             /* BNE fill, Y wrapped to zero */
        0xa9u, 0x20u, 0x8du, 0x06u, 0x20u, /* set visible nametable tile 0 */
        0xa9u, 0x00u, 0x8du, 0x06u, 0x20u,
        0xa9u, 0xfeu, 0x8du, 0x07u, 0x20u, /* visible FE tile beside the FD field */
        0xa9u, 0x01u, 0x8du, 0x15u, 0x40u, /* enable pulse 1 before its length write */
        0xa9u, 0x9fu, 0x8du, 0, 0x40u, /* pulse 1: constant volume and duty */
        0xa9u, 0x80u, 0x8du, 0x02u, 0x40u, /* timer low */
        0xa9u, 0xf8u, 0x8du, 0x03u, 0x40u, /* timer high and long length */
        0xa9u, 0x00u, 0x8du, 0x00u, 0x20u, /* background uses pattern table $0000 */
        0xa9u, 0x0au, 0x8du, 0x01u, 0x20u, /* show background and left edge */
        0x4cu, 0x00u, 0x00u       /* JMP to the fixed idle loop (address patched below) */
    };
    // clang-format on
    struct ines_spec spec;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 8u;
    spec.chr_8k = 16u;
    spec.nes2 = 1u;
    spec.mapper = 9u;
    spec.submapper = 0u;
    spec.mirroring_vertical = vertical;
    spec.reset_vector = 0xfe00u;
    spec.nmi_vector = 0xfe00u;
    spec.irq_vector = 0xfe00u;
    const size_t size = ines_build(out, cap, &spec);
    if (size != MMC2_FIXTURE_CAP)
        return 0u;

    uint8_t *prg = out + 16u;
    for (unsigned bank = 0u; bank < 16u; ++bank) {
        for (unsigned page = 0u; page < 8u; ++page) {
            const size_t offset = (size_t)bank * MMC2_BANK_8K + (size_t)page * MMC2_PAGE_1K;
            memset(prg + offset, (int)(0x10u + bank * 8u + page), MMC2_PAGE_1K);
        }
    }
    memcpy(prg + 15u * MMC2_BANK_8K + 0x1e00u, boot, sizeof boot);
    {
        const uint16_t loop = (uint16_t)(0xfe00u + sizeof boot - 3u);
        uint8_t *loop_operand = prg + 15u * MMC2_BANK_8K + 0x1e00u + sizeof boot - 2u;
        loop_operand[0] = (uint8_t)loop;
        loop_operand[1] = (uint8_t)(loop >> 8u);
    }
    for (unsigned i = 0u; i < 3u; ++i) {
        prg[MMC2_PRG_SIZE - 6u + 2u * i] = 0x00u;
        prg[MMC2_PRG_SIZE - 5u + 2u * i] = 0xfeu;
    }

    uint8_t *chr = prg + MMC2_PRG_SIZE;
    for (unsigned bank = 0u; bank < 32u; ++bank)
        memset(chr + (size_t)bank * MMC2_BANK_4K, (int)(0xa0u + bank), MMC2_BANK_4K);
    return size;
}

#endif /* NESTURBATOR_TESTS_MMC2_FIXTURE_H */
