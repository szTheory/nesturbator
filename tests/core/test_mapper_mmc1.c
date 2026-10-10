/* BOARD-02: the MMC1 board on synthetic images. One function per behaviour. Every code byte is
   authored here and placed with ines_spec segments; PRG bank b of 16 KiB starts at
   b * INES_PRG_BANK. */
#include <stdint.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"

#define IMAGE_CAP (16u + 32u * INES_PRG_BANK + 4u * INES_CHR_BANK)

static nesturbator *make_instance(void)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    return inst;
}

static void run_frame(nesturbator *inst)
{
    static uint16_t video[256u * 240u];
    static int16_t audio[2048];
    nesturbator_frame io;
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = video;
    io.video_pitch = 256u;
    io.audio = audio;
    io.audio_capacity = 2048u;
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
}

/* Tracer: a 128 KiB CHR-RAM board. The fixed last bank (7, at $C000) writes 1 to the PRG register
   through five non-adjacent serial writes and jumps to $8000. Bank 1 stores $42 at $0010; bank 0
   would store $24, so a wrong bank is visible. */
static void test_tracer_five_writes_switch_bank(void)
{
    static uint8_t image[IMAGE_CAP];
    /* LDA #$01; STA $E000; LSR A; STA $E000 x4; JMP $8000 */
    static const uint8_t boot[] = {0xa9u, 0x01u, 0x8du, 0x00u, 0xe0u, 0x4au, 0x8du,
                                   0x00u, 0xe0u, 0x8du, 0x00u, 0xe0u, 0x8du, 0x00u,
                                   0xe0u, 0x8du, 0x00u, 0xe0u, 0x4cu, 0x00u, 0x80u};
    /* LDA #$42; STA $0010; JMP $8005 */
    static const uint8_t bank1[] = {0xa9u, 0x42u, 0x8du, 0x10u, 0x00u, 0x4cu, 0x05u, 0x80u};
    static const uint8_t bank0[] = {0xa9u, 0x24u, 0x8du, 0x10u, 0x00u, 0x4cu, 0x05u, 0x80u};
    struct ines_segment segs[3];
    struct ines_spec spec;
    nesturbator *inst = make_instance();
    struct nesturbator *nes = inst;
    size_t size;
    segs[0].prg_offset = 0x00000u;
    segs[0].bytes = bank0;
    segs[0].len = sizeof bank0;
    segs[1].prg_offset = 1u * INES_PRG_BANK;
    segs[1].bytes = bank1;
    segs[1].len = sizeof bank1;
    segs[2].prg_offset = 7u * INES_PRG_BANK;
    segs[2].bytes = boot;
    segs[2].len = sizeof boot;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 8u;
    spec.nes2 = 1u;
    spec.mapper = 1u;
    spec.segments = segs;
    spec.segment_count = 3u;
    spec.reset_vector = 0xc000u;
    size = ines_build(image, sizeof image, &spec);
    CHECK(size != 0u);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    run_frame(inst);
    CHECK_EQ_U64(nes->mapper.reg.mmc1.prg, 1u);
    CHECK_EQ_HEX(nes->bus.ram[0x10], 0x42u);
    nesturbator_destroy(inst);
}

/* Loads a 32 KiB mapper 1 NES 2.0 image with CHR-RAM and the given RAM nibbles. The optional
   target byte is placed at PRG offset 0 (bank 0, CPU $8000 at power-on) and code at bank 1
   (CPU $C000), where the reset vector points. */
static nesturbator *load_small(uint8_t ram_shift, uint8_t nvram_shift, uint8_t battery,
                               const uint8_t *code, size_t code_len, const uint8_t *target)
{
    static uint8_t image[IMAGE_CAP];
    struct ines_segment segs[2];
    struct ines_spec spec;
    nesturbator *inst = make_instance();
    size_t size, n = 0u;
    if (target != NULL) {
        segs[n].prg_offset = 0u;
        segs[n].bytes = target;
        segs[n].len = 1u;
        n++;
    }
    if (code != NULL) {
        segs[n].prg_offset = INES_PRG_BANK;
        segs[n].bytes = code;
        segs[n].len = code_len;
        n++;
    }
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 2u;
    spec.nes2 = 1u;
    spec.mapper = 1u;
    spec.battery = battery;
    spec.prg_ram_shift = ram_shift;
    spec.prg_nvram_shift = nvram_shift;
    spec.segments = segs;
    spec.segment_count = n;
    spec.reset_vector = 0xc000u;
    size = ines_build(image, sizeof image, &spec);
    CHECK(size != 0u);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    return inst;
}

/* A write straight into the hook, as the bus would deliver it. */
static void hook(struct nesturbator *nes, uint16_t addr, uint8_t value, uint64_t stamp)
{
    nes->map.ops.cpu_write(nes, addr, value, stamp);
}

/* The five bits of v, LSB first, to addr at stamps start, start+2, ...; returns the next free
   stamp. Never adjacent. */
static uint64_t feed(struct nesturbator *nes, uint16_t addr, uint8_t v, uint64_t start)
{
    for (unsigned i = 0u; i < 5u; ++i) {
        hook(nes, addr, (uint8_t)((v >> i) & 1u), start);
        start += 2u;
    }
    return start;
}

static uint8_t control_of(const struct nesturbator *nes)
{
    return (uint8_t)(nes->mapper.reg.mmc1.control_x ^ 0x0cu);
}

/* D-15, D-17: writes at (n, n+1) shift only the first bit; (n, n+2) shift both; last_write is the
   latest stamp plus one even after an ignored write. */
static void test_hook_adjacency(void)
{
    nesturbator *inst = load_small(0u, 0u, 0u, NULL, 0u, NULL);
    struct nesturbator *nes = inst;
    hook(nes, 0x8000u, 1u, 10u);
    hook(nes, 0x8000u, 1u, 11u);
    CHECK_EQ_U64(nes->mapper.reg.mmc1.count, 1u);
    CHECK_EQ_U64(nes->mapper.reg.mmc1.last_write, 12u);
    nesturbator_destroy(inst);
    inst = load_small(0u, 0u, 0u, NULL, 0u, NULL);
    nes = inst;
    hook(nes, 0x8000u, 1u, 10u);
    hook(nes, 0x8000u, 1u, 12u);
    CHECK_EQ_U64(nes->mapper.reg.mmc1.count, 2u);
    CHECK_EQ_U64(nes->mapper.reg.mmc1.last_write, 13u);
    nesturbator_destroy(inst);
}

/* D-15: a zeroed last_write means never written, so a first write at stamp 0 or 1 is accepted. */
static void test_hook_first_write_not_adjacent(void)
{
    for (uint64_t stamp = 0u; stamp < 2u; ++stamp) {
        nesturbator *inst = load_small(0u, 0u, 0u, NULL, 0u, NULL);
        struct nesturbator *nes = inst;
        CHECK_EQ_U64(nes->mapper.reg.mmc1.last_write, 0u);
        hook(nes, 0x8000u, 1u, stamp);
        CHECK_EQ_U64(nes->mapper.reg.mmc1.count, 1u);
        nesturbator_destroy(inst);
    }
}

/* D-15: the ignore applies even when the first write was a RAM write below $8000. */
static void test_hook_ram_write_then_serial(void)
{
    nesturbator *inst = load_small(0u, 0u, 0u, NULL, 0u, NULL);
    struct nesturbator *nes = inst;
    hook(nes, 0x6000u, 0x55u, 20u);
    hook(nes, 0x8000u, 1u, 21u);
    CHECK_EQ_U64(nes->mapper.reg.mmc1.count, 0u);
    CHECK_EQ_U64(nes->mapper.reg.mmc1.last_write, 22u);
    nesturbator_destroy(inst);
}

/* D-15: address bits 14-13 of the fifth write pick the register; bits arrive LSB first. */
static void test_hook_register_select_and_bit_order(void)
{
    nesturbator *inst = load_small(0u, 0u, 0u, NULL, 0u, NULL);
    struct nesturbator *nes = inst;
    uint64_t t = 10u;
    /* 0b10110 is 0x16; the first write carries bit 0, which is 0. */
    t = feed(nes, 0xa000u, 0x16u, t);
    CHECK_EQ_HEX(nes->mapper.reg.mmc1.chr0, 0x16u);
    CHECK_EQ_U64(nes->mapper.reg.mmc1.count, 0u);
    t = feed(nes, 0xc000u, 0x16u, t);
    CHECK_EQ_HEX(nes->mapper.reg.mmc1.chr1, 0x16u);
    t = feed(nes, 0xe000u, 0x16u, t);
    CHECK_EQ_HEX(nes->mapper.reg.mmc1.prg, 0x16u);
    t = feed(nes, 0x8000u, 0x16u, t);
    CHECK_EQ_HEX(control_of(nes), 0x16u);
    /* Only the fifth write's address picks the register. */
    hook(nes, 0xa000u, 1u, t);
    hook(nes, 0xc000u, 0u, t + 2u);
    hook(nes, 0xe000u, 0u, t + 4u);
    hook(nes, 0x9fffu, 0u, t + 6u);
    hook(nes, 0xdfffu, 0u, t + 8u);
    CHECK_EQ_HEX(nes->mapper.reg.mmc1.chr1, 0x01u);
    CHECK_EQ_HEX(nes->mapper.reg.mmc1.chr0, 0x16u);
    nesturbator_destroy(inst);
}

/* D-15, D-17: bit 7 clears the shift register and sets Control |= $0C; mirroring, CHR mode and the
   other registers stay, and it is never ignored. */
static void test_hook_reset_bit(void)
{
    nesturbator *inst = load_small(0u, 0u, 0u, NULL, 0u, NULL);
    struct nesturbator *nes = inst;
    uint64_t t = 10u;
    t = feed(nes, 0x8000u, 0x11u, t);
    t = feed(nes, 0xa000u, 0x05u, t);
    t = feed(nes, 0xc000u, 0x06u, t);
    t = feed(nes, 0xe000u, 0x03u, t);
    CHECK_EQ_HEX(control_of(nes), 0x11u);
    hook(nes, 0x8000u, 1u, t);
    hook(nes, 0x8000u, 0u, t + 2u);
    CHECK_EQ_U64(nes->mapper.reg.mmc1.count, 2u);
    /* The reset write lands on the cycle right after the last one and is still honoured. */
    hook(nes, 0x8000u, 0x80u, t + 3u);
    CHECK_EQ_U64(nes->mapper.reg.mmc1.count, 0u);
    CHECK_EQ_U64(nes->mapper.reg.mmc1.shift, 0u);
    CHECK_EQ_HEX(control_of(nes), 0x1du);
    CHECK_EQ_HEX(nes->mapper.reg.mmc1.chr0, 0x05u);
    CHECK_EQ_HEX(nes->mapper.reg.mmc1.chr1, 0x06u);
    CHECK_EQ_HEX(nes->mapper.reg.mmc1.prg, 0x03u);
    nesturbator_destroy(inst);
}

/* Runs "INC abs; JMP *" for one frame against a target byte, and returns the instance. */
static nesturbator *run_inc(uint16_t abs_addr, uint8_t ram_nibble, uint8_t battery,
                            const uint8_t *target)
{
    uint8_t code[6];
    nesturbator *inst;
    code[0] = 0xeeu;
    code[1] = (uint8_t)(abs_addr & 0xffu);
    code[2] = (uint8_t)(abs_addr >> 8);
    code[3] = 0x4cu;
    code[4] = 0x03u;
    code[5] = 0xc0u;
    inst = load_small(0u, ram_nibble, battery, code, sizeof code, target);
    run_frame(inst);
    return inst;
}

/* D-17: INC $8000 on a byte with bit 7 clear raises the shift count by 1, not 2. */
static void test_cpu_inc_bit7_clear(void)
{
    static const uint8_t target = 0x02u;
    nesturbator *inst = run_inc(0x8000u, 0u, 0u, &target);
    struct nesturbator *nes = inst;
    CHECK_EQ_U64(nes->mapper.reg.mmc1.count, 1u);
    nesturbator_destroy(inst);
}

/* D-17: on $FF the first write resets and the adjacent $00 data write is dropped. */
static void test_cpu_inc_ff(void)
{
    static const uint8_t target = 0xffu;
    nesturbator *inst = run_inc(0x8000u, 0u, 0u, &target);
    struct nesturbator *nes = inst;
    CHECK_EQ_U64(nes->mapper.reg.mmc1.count, 0u);
    CHECK_EQ_U64((control_of(nes) >> 2) & 3u, 3u);
    nesturbator_destroy(inst);
}

/* D-17: on $7F the data bit shifts in and then the $80 write resets. */
static void test_cpu_inc_7f(void)
{
    static const uint8_t target = 0x7fu;
    nesturbator *inst = run_inc(0x8000u, 0u, 0u, &target);
    struct nesturbator *nes = inst;
    CHECK_EQ_U64(nes->mapper.reg.mmc1.count, 0u);
    CHECK_EQ_U64(nes->mapper.reg.mmc1.shift, 0u);
    CHECK_EQ_HEX(control_of(nes) & 0x0cu, 0x0cu);
    nesturbator_destroy(inst);
}

/* D-12: INC $6000 on a battery image counts both writes of the read-modify-write. */
static void test_cpu_inc_ram_counts_two(void)
{
    nesturbator *inst = run_inc(0x6000u, 7u, 1u, NULL);
    uint8_t *data = NULL;
    size_t n = 0u;
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &data, &n),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(n, 8192u);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 2u);
    CHECK(data != NULL);
    CHECK_EQ_HEX(data[0], 0x01u);
    nesturbator_destroy(inst);
}

/* D-16, D-17: nesturbator_reset keeps the registers, the shift register, last_write and the
   battery RAM, and a second reset keeps them again. */
static void test_reset_keeps_state(void)
{
    nesturbator *inst = load_small(0u, 7u, 1u, NULL, 0u, NULL);
    struct nesturbator *nes = inst;
    struct nesturbator__mapper before;
    uint8_t *data = NULL;
    size_t n = 0u;
    uint64_t t = 10u;
    t = feed(nes, 0x8000u, 0x13u, t);
    t = feed(nes, 0xa000u, 0x05u, t);
    t = feed(nes, 0xe000u, 0x02u, t);
    hook(nes, 0x8000u, 1u, t);
    hook(nes, 0x8000u, 1u, t + 2u);
    CHECK_EQ_U64(nes->mapper.reg.mmc1.count, 2u);
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &data, &n),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(n, 8192u);
    data[0] = 0xc3u;
    data[8191] = 0x3cu;
    before = nes->mapper;
    for (unsigned i = 0u; i < 2u; ++i) {
        CHECK_EQ_U64(nesturbator_reset(inst), NESTURBATOR_OK);
        CHECK(memcmp(&before.reg.mmc1, &nes->mapper.reg.mmc1, sizeof before.reg.mmc1) == 0);
        CHECK_EQ_HEX(data[0], 0xc3u);
        CHECK_EQ_HEX(data[8191], 0x3cu);
    }
    CHECK_EQ_HEX(control_of(nes), 0x13u);
    CHECK_EQ_U64(nes->mapper.reg.mmc1.count, 2u);
    nesturbator_destroy(inst);
}

int main(void)
{
    test_tracer_five_writes_switch_bank();
    test_hook_adjacency();
    test_hook_first_write_not_adjacent();
    test_hook_ram_write_then_serial();
    test_hook_register_select_and_bit_order();
    test_hook_reset_bit();
    test_cpu_inc_bit7_clear();
    test_cpu_inc_ff();
    test_cpu_inc_7f();
    test_cpu_inc_ram_counts_two();
    test_reset_keeps_state();
    CHECK_DONE();
}
