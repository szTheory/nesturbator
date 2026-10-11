/* BOARD-02: the MMC1 board on synthetic images. One function per behaviour. Every code byte is
   authored here and placed with ines_spec segments; PRG bank b of 16 KiB starts at
   b * INES_PRG_BANK. */
#include <stdint.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"

#define IMAGE_CAP (16u + 32u * INES_PRG_BANK + 16u * INES_CHR_BANK)

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

/* Loads a mapper 1 NES 2.0 image with the given sizes and RAM nibbles. chr_8k 0 is CHR-RAM. The
   optional target byte is placed at PRG offset 0 (bank 0, CPU $8000 at power-on) and code at bank
   1 (CPU $C000 on a 32 KiB board), where the reset vector points. */
static nesturbator *load_board(uint8_t prg_16k, uint8_t chr_8k, uint8_t ram_shift,
                               uint8_t nvram_shift, uint8_t battery, const uint8_t *code,
                               size_t code_len, const uint8_t *target)
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
    spec.prg_16k = prg_16k;
    spec.chr_8k = chr_8k;
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

/* A 32 KiB CHR-RAM board. */
static nesturbator *load_small(uint8_t ram_shift, uint8_t nvram_shift, uint8_t battery,
                               const uint8_t *code, size_t code_len, const uint8_t *target)
{
    return load_board(2u, 0u, ram_shift, nvram_shift, battery, code, code_len, target);
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

/* D-10: control bits 0-1 give one-screen lower, one-screen upper, vertical and horizontal; the
   header mirroring bit is ignored (NESdev Wiki "MMC1"). */
static void test_mirroring(void)
{
    static const uint8_t expect[4][4] = {
        {0u, 0u, 0u, 0u}, {1u, 1u, 1u, 1u}, {0u, 1u, 0u, 1u}, {0u, 0u, 1u, 1u}};
    nesturbator *inst = load_small(0u, 0u, 0u, NULL, 0u, NULL);
    struct nesturbator *nes = inst;
    uint64_t t = 10u;
    /* Power-on is one-screen lower. */
    CHECK(memcmp(nes->map.nt, expect[0], 4u) == 0);
    for (uint8_t m = 0u; m < 4u; ++m) {
        t = feed(nes, 0x8000u, (uint8_t)(0x0cu | m), t);
        CHECK(memcmp(nes->map.nt, expect[m], 4u) == 0);
    }
    nesturbator_destroy(inst);
}

/* D-10: control bit 4 clear maps one 8 KiB bank from CHR0 & $1E; set maps two 4 KiB banks from
   CHR0 and CHR1. */
static void test_chr_modes(void)
{
    nesturbator *inst = load_board(2u, 16u, 0u, 0u, 0u, NULL, 0u, NULL);
    struct nesturbator *nes = inst;
    uint64_t t = 10u;
    t = feed(nes, 0xa000u, 5u, t);
    for (uint32_t i = 0u; i < 4u; ++i) {
        CHECK(nes->map.chr_r[i] == nes->cart.chr + 4u * 4096u + i * 1024u);
        CHECK(nes->map.chr_r[4u + i] == nes->cart.chr + 5u * 4096u + i * 1024u);
        CHECK(nes->map.chr_w[i] == NULL);
    }
    t = feed(nes, 0x8000u, 0x1cu, t);
    t = feed(nes, 0xa000u, 3u, t);
    t = feed(nes, 0xc000u, 6u, t);
    for (uint32_t i = 0u; i < 4u; ++i) {
        CHECK(nes->map.chr_r[i] == nes->cart.chr + 3u * 4096u + i * 1024u);
        CHECK(nes->map.chr_r[4u + i] == nes->cart.chr + 6u * 4096u + i * 1024u);
    }
    nesturbator_destroy(inst);
}

/* D-10: PRG RAM is enabled at power-on and $E000 bit 4 disables it: reads give the open-bus latch,
   writes drop and the save generation does not move. */
static void test_ram_enable(void)
{
    nesturbator *inst = load_small(0u, 7u, 1u, NULL, 0u, NULL);
    struct nesturbator *nes = inst;
    uint8_t *data = NULL;
    size_t n = 0u;
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &data, &n),
                 NESTURBATOR_OK);
    CHECK(nes->map.cpu_r[8] != NULL);
    nesturbator__bus_write(nes, 0x6000u, 0x77u);
    CHECK_EQ_HEX(data[0], 0x77u);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 1u);
    feed(nes, 0xe000u, 0x10u, 10u);
    CHECK(nes->map.cpu_r[8] == NULL);
    CHECK(nes->map.cpu_w[8] == NULL);
    nesturbator__bus_write(nes, 0x6000u, 0x99u);
    nes->bus.open_bus = 0x5cu;
    CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x6000u), 0x5cu);
    CHECK_EQ_HEX(data[0], 0x77u);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 1u);
    feed(nes, 0xe000u, 0x00u, 40u);
    CHECK(nes->map.cpu_r[8] != NULL);
    nesturbator_destroy(inst);
}

/* D-05, D-10: SNROM's CHR0 bit 4 disables RAM only on a CHR-RAM board with PRG <= 256 KiB and RAM
   <= 8 KiB. CHR-ROM boards and 512 KiB boards keep it. */
static void test_snrom_ram_disable(void)
{
    nesturbator *inst = load_board(8u, 0u, 7u, 0u, 0u, NULL, 0u, NULL);
    struct nesturbator *nes = inst;
    CHECK(nes->map.cpu_r[8] != NULL);
    feed(nes, 0xa000u, 0x10u, 10u);
    CHECK(nes->map.cpu_r[8] == NULL);
    CHECK(nes->map.cpu_w[8] == NULL);
    feed(nes, 0xa000u, 0x00u, 40u);
    CHECK(nes->map.cpu_r[8] != NULL);
    nesturbator_destroy(inst);
    inst = load_board(8u, 1u, 7u, 0u, 0u, NULL, 0u, NULL);
    nes = inst;
    feed(nes, 0xa000u, 0x10u, 10u);
    CHECK(nes->map.cpu_r[8] != NULL);
    nesturbator_destroy(inst);
    inst = load_board(32u, 0u, 7u, 0u, 0u, NULL, 0u, NULL);
    nes = inst;
    feed(nes, 0xa000u, 0x10u, 10u);
    CHECK(nes->map.cpu_r[8] != NULL);
    nesturbator_destroy(inst);
}

/* D-05, D-10, C4: SOROM banks its two 8 KiB chips on CHR0 bit 3 only (NESdev Wiki "MMC1": "SOROM
   implements only this bit"). Bank 0 is the work half and does not count; bank 1 is the span. */
static void test_sorom_banking(void)
{
    nesturbator *inst = load_board(8u, 1u, 7u, 7u, 1u, NULL, 0u, NULL);
    struct nesturbator *nes = inst;
    uint8_t *data = NULL;
    size_t n = 0u;
    uint64_t t = 10u;
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &data, &n),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(n, 8192u);
    CHECK(nes->map.cpu_r[8] == nes->cart.prg_ram);
    nesturbator__bus_write(nes, 0x6000u, 0xa1u);
    CHECK_EQ_HEX(nes->cart.prg_ram[0], 0xa1u);
    CHECK_EQ_HEX(data[0], 0x00u);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 0u);
    t = feed(nes, 0xa000u, 0x08u, t);
    CHECK(nes->map.cpu_r[8] == data);
    nesturbator__bus_write(nes, 0x6000u, 0xb2u);
    CHECK_EQ_HEX(data[0], 0xb2u);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 1u);
    /* Bit 2 alone does not select the second chip. */
    t = feed(nes, 0xa000u, 0x04u, t);
    CHECK(nes->map.cpu_r[8] == nes->cart.prg_ram);
    nesturbator_destroy(inst);
}

/* D-05, D-10: SXROM's CHR0 bits 3-2 select four 8 KiB RAM banks; span offset = bank x 8 KiB. */
static void test_sxrom_ram_banks(void)
{
    nesturbator *inst = load_board(32u, 0u, 0u, 9u, 1u, NULL, 0u, NULL);
    struct nesturbator *nes = inst;
    uint8_t *data = NULL;
    size_t n = 0u;
    uint64_t t = 10u;
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &data, &n),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(n, 32768u);
    for (uint8_t b = 0u; b < 4u; ++b) {
        t = feed(nes, 0xa000u, (uint8_t)(b << 2), t);
        nesturbator__bus_write(nes, 0x6000u, (uint8_t)(0xb0u + b));
    }
    for (uint8_t b = 0u; b < 4u; ++b)
        CHECK_EQ_HEX(data[(size_t)b * 8192u], 0xb0u + b);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 4u);
    nesturbator_destroy(inst);
}

/* D-10, Pitfall 7: on a 512 KiB board CHR0 bit 4 selects the 256 KiB PRG half for both windows,
   the fixed bank included. */
static void test_surom_sxrom_prg_halves(void)
{
    nesturbator *inst = load_board(32u, 0u, 0u, 7u, 1u, NULL, 0u, NULL);
    struct nesturbator *nes = inst;
    uint64_t t = 10u;
    const uint8_t *prg = nes->cart.prg;
    /* Mode 3, PRG register 2, CHR0 bit 4 clear: banks 2 and 15. */
    t = feed(nes, 0xe000u, 2u, t);
    CHECK(nes->map.cpu_r[16] == prg + 2u * 16384u);
    CHECK(nes->map.cpu_r[32] == prg + 15u * 16384u);
    t = feed(nes, 0xa000u, 0x10u, t);
    CHECK(nes->map.cpu_r[16] == prg + 18u * 16384u);
    CHECK(nes->map.cpu_r[32] == prg + 31u * 16384u);
    /* Mode 2: the first bank of the half at $8000. */
    t = feed(nes, 0x8000u, 0x08u, t);
    CHECK(nes->map.cpu_r[16] == prg + 16u * 16384u);
    CHECK(nes->map.cpu_r[32] == prg + 18u * 16384u);
    /* Modes 0 and 1: a 32 KiB bank; the low register bit is ignored. */
    t = feed(nes, 0xa000u, 0x00u, t);
    t = feed(nes, 0xe000u, 3u, t);
    for (uint8_t mode = 0u; mode < 2u; ++mode) {
        t = feed(nes, 0x8000u, (uint8_t)(mode << 2), t);
        CHECK(nes->map.cpu_r[16] == prg + 2u * 16384u);
        CHECK(nes->map.cpu_r[32] == prg + 3u * 16384u);
    }
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
    test_mirroring();
    test_chr_modes();
    test_ram_enable();
    test_snrom_ram_disable();
    test_sorom_banking();
    test_sxrom_ram_banks();
    test_surom_sxrom_prg_halves();
    CHECK_DONE();
}
