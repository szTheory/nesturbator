/* Focused CPU-visible PPU register and dot-boundary checks. */
#include <string.h>

#include "../check.h"
#include "internal.h"

static struct nesturbator nes;

static void reset(void)
{
    memset(&nes, 0, sizeof nes);
}

static void test_register_latches_and_palette_aliases(void)
{
    reset();
    nesturbator__ppu_register_write(&nes, 0x2005u, 0x2du);
    CHECK_EQ_U64(nes.ppu.fine_x, 5u);
    CHECK_EQ_U64(nes.ppu.address_latch, 1u);
    (void)nesturbator__ppu_register_read(&nes, 0x2002u);
    nesturbator__ppu_register_write(&nes, 0x2006u, 0x3fu);
    nesturbator__ppu_register_write(&nes, 0x2006u, 0x00u);
    CHECK_EQ_U64(nes.ppu.address_latch, 0u);
    CHECK_EQ_U64(nes.ppu.v, 0x3f00u);
    nesturbator__ppu_write(&nes, 0x3f10u, 0x2au);
    CHECK_EQ_U64(nesturbator__ppu_read(&nes, 0x3f00u), 0x2au);
    nesturbator__ppu_write(&nes, 0x3f20u, 0x15u);
    CHECK_EQ_U64(nesturbator__ppu_read(&nes, 0x3f00u), 0x15u);
}

static void test_status_read_just_before_vblank_suppresses_flag(void)
{
    reset();
    nes.ppu.scanline = 241u;
    nes.ppu.dot = 0u;
    nes.ppu.control = 0x80u;
    nesturbator__ppu_register_write(&nes, 0x2000u, 0x80u);
    nesturbator__ppu_run_until(&nes, 0u);
    CHECK_EQ_U64(nesturbator__ppu_register_read(&nes, 0x2002u) & 0x80u, 0u);
    nesturbator__ppu_run_until(&nes, 8u);
    CHECK_EQ_U64(nes.ppu.status & 0x80u, 0u);
    CHECK_EQ_U64(nes.cpu.nmi_pending, 0u);
}

static void test_status_read_on_vblank_dot_observes_then_clears_flag(void)
{
    reset();
    nes.ppu.scanline = 241u;
    nes.ppu.dot = 0u;
    nes.ppu.control = 0x80u;
    nesturbator__ppu_run_until(&nes, 8u);
    CHECK_EQ_U64(nes.ppu.status & 0x80u, 0x80u);
    CHECK_EQ_U64(nes.cpu.nmi_pending, 1u);
    CHECK_EQ_U64(nesturbator__ppu_register_read(&nes, 0x2002u) & 0x80u, 0x80u);
    CHECK_EQ_U64(nes.ppu.status & 0x80u, 0u);
    CHECK_EQ_U64(nes.cpu.nmi_pending, 0u);
}

static void test_nametable_reads_follow_board_map(void)
{
    static const uint8_t horizontal[4] = {0u, 0u, 1u, 1u};
    static const uint8_t vertical[4] = {0u, 1u, 0u, 1u};
    reset();
    memcpy(nes.map.nt, horizontal, sizeof horizontal);
    nesturbator__ppu_write(&nes, 0x2000u, 0x11u);
    CHECK_EQ_U64(nesturbator__ppu_read(&nes, 0x2400u), 0x11u);
    CHECK_EQ_U64(nesturbator__ppu_read(&nes, 0x2800u), 0u);

    memset(nes.ppu.nametable, 0, sizeof nes.ppu.nametable);
    memcpy(nes.map.nt, vertical, sizeof vertical);
    nesturbator__ppu_write(&nes, 0x2000u, 0x22u);
    CHECK_EQ_U64(nesturbator__ppu_read(&nes, 0x2800u), 0x22u);
    CHECK_EQ_U64(nesturbator__ppu_read(&nes, 0x2400u), 0u);
}

static void test_register_open_bus_and_ppudata_buffer(void)
{
    reset();
    nesturbator__ppu_register_write(&nes, 0x2001u, 0x1bu);
    CHECK_EQ_U64(nesturbator__ppu_register_read(&nes, 0x2000u), 0x1bu);
    nes.ppu.status = 0x80u;
    CHECK_EQ_U64(nesturbator__ppu_register_read(&nes, 0x2002u), 0x9bu);

    nesturbator__ppu_write(&nes, 0x2000u, 0x42u);
    nesturbator__ppu_register_write(&nes, 0x2006u, 0x20u);
    nesturbator__ppu_register_write(&nes, 0x2006u, 0x00u);
    nes.ppu.read_buffer = 0x11u;
    CHECK_EQ_U64(nesturbator__ppu_register_read(&nes, 0x2007u), 0x11u);
    CHECK_EQ_U64(nes.ppu.read_buffer, 0x42u);

    nesturbator__ppu_write(&nes, 0x2f00u, 0x35u);
    nesturbator__ppu_write(&nes, 0x3f00u, 0x2au);
    nes.ppu.mask = 0u;
    nesturbator__ppu_register_write(&nes, 0x2006u, 0x3fu);
    nesturbator__ppu_register_write(&nes, 0x2006u, 0x00u);
    CHECK_EQ_U64(nesturbator__ppu_register_read(&nes, 0x2007u), 0x2au);
    CHECK_EQ_U64(nes.ppu.read_buffer, 0x35u);

    nes.ppu.mask = 1u;
    nes.ppu.io_bus = 0xc0u;
    nes.ppu.palette[0x0cu] = 0x1au;
    nes.ppu.v = 0x3f1cu;
    CHECK_EQ_U64(nesturbator__ppu_register_read(&nes, 0x2007u), 0xd0u);

    nes.ppu.io_bus = 0xffu;
    nes.ppu.io_bus_age = 0u;
    nesturbator__ppu_run_until(&nes, 160000u * 8u);
    CHECK_EQ_U64(nes.ppu.io_bus, 0u);
}

int main(void)
{
    test_register_latches_and_palette_aliases();
    test_status_read_just_before_vblank_suppresses_flag();
    test_status_read_on_vblank_dot_observes_then_clears_flag();
    test_nametable_reads_follow_board_map();
    test_register_open_bus_and_ppudata_buffer();
    CHECK_DONE();
}
