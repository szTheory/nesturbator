/* bus.unit: the library bus in src/bus.c, compiled in directly with no CPU
   (D-12). One call is one CPU cycle of 24 ticks; RAM is 2048 bytes mirrored
   over 0x0000 to 0x1FFF; any other read returns the last value on the bus,
   and any other write changes no RAM. */
#include <string.h>

#include "../check.h"
#include "internal.h"

static struct nesturbator nes;

static void reset(void)
{
    memset(&nes, 0, sizeof nes);
}

/* 24 ticks per read and per write, counted from zero. */
static void test_ticks(void)
{
    reset();
    nesturbator__bus_read(&nes, 0x0000u);
    CHECK_EQ_U64(nes.ticks, 24u);
    for (int i = 1; i < 10; i++) {
        nesturbator__bus_read(&nes, 0x0000u);
    }
    CHECK_EQ_U64(nes.ticks, 240u);
    nesturbator__bus_write(&nes, 0x0000u, 1u);
    CHECK_EQ_U64(nes.ticks, 264u);
}

/* A byte written at 0x0001 reads back at each of its three mirrors. */
static void test_mirror(void)
{
    reset();
    nesturbator__bus_write(&nes, 0x0001u, 0x5Au);
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x0801u), 0x5Au);
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x1001u), 0x5Au);
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x1801u), 0x5Au);
    /* A write through a mirror lands in the same byte. */
    nesturbator__bus_write(&nes, 0x1FFFu, 0x3Cu);
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x07FFu), 0x3Cu);
}

/* An unmapped read returns the last value written or read. */
static void test_open_bus(void)
{
    reset();
    nesturbator__bus_write(&nes, 0x0002u, 0x77u);
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x4020u), 0x77u);
    nesturbator__bus_write(&nes, 0x0001u, 0x5Au);
    nesturbator__bus_write(&nes, 0x0003u, 0x11u);
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x0001u), 0x5Au);
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x8000u), 0x5Au);
}

/* A write outside RAM changes no RAM byte. */
static void test_unmapped_write(void)
{
    reset();
    for (uint32_t i = 0u; i < sizeof nes.bus.ram; i++) {
        nes.bus.ram[i] = (uint8_t)i;
    }
    uint8_t before[sizeof nes.bus.ram];
    memcpy(before, nes.bus.ram, sizeof before);
    nesturbator__bus_write(&nes, 0x6000u, 0xA5u);
    CHECK(memcmp(before, nes.bus.ram, sizeof before) == 0);
    CHECK_EQ_HEX(nes.bus.open_bus, 0xA5u);
}

int main(void)
{
    test_ticks();
    test_mirror();
    test_open_bus();
    test_unmapped_write();
    CHECK_DONE();
}
