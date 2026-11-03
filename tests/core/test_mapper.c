/* MAP-01 (D-03 to D-08): the mapper seam on a synthetic NROM image with a test board
   installed. One function per behaviour. */
#include <stdint.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"
#include "../mapper_test.h"

#define IMAGE_CAP 40000u

#define REGIONAL_IMAGE_CAP (16u + 8u * INES_PRG_BANK + 2u * INES_CHR_BANK)
#define REGIONAL_TONE_TIMER 100u
#define REGIONAL_FRAME_COUNT 8u
#define REGIONAL_PCM_CAP (REGIONAL_FRAME_COUNT * 960u)
#define REGIONAL_TRACE_RISES 32u
#define REGIONAL_PAL_FRAME_TICKS UINT64_C(1063920)
#define REGIONAL_NTSC_FRAME_TICKS UINT64_C(714732)
#define REGIONAL_PAL_TICKS_PER_SECOND UINT64_C(53203425)
#define REGIONAL_NTSC_CPU_NUMERATOR UINT64_C(236250000)
#define REGIONAL_NTSC_CPU_DENOMINATOR UINT64_C(132)

static uint8_t regional_image[REGIONAL_IMAGE_CAP];
static uint8_t regional_chr[2u * INES_CHR_BANK];
static uint16_t regional_video[256u * 240u];
static uint16_t regional_video_peer[256u * 240u];
static uint16_t regional_video_ntsc[256u * 240u];
static int16_t regional_pal_pcm[REGIONAL_PCM_CAP];
static int16_t regional_ntsc_pcm[REGIONAL_PCM_CAP];

struct regional_trace {
    int32_t previous_level;
    uint64_t rising_cycles[REGIONAL_TRACE_RISES];
    uint32_t rising_count;
};

/* Builds an NROM image (16 KiB PRG, CHR ROM) and loads it. code is placed at $8000 and the
   reset vector points there. */
static struct nesturbator *make_instance(const uint8_t *code, size_t code_len, int trainer,
                                         int vertical)
{
    static uint8_t image[IMAGE_CAP];
    static const uint8_t trainer_bytes[INES_TRAINER_SIZE] = {0x6du};
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    struct ines_spec spec;
    size_t size;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = 1u;
    spec.mirroring_vertical = (uint8_t)(vertical != 0);
    spec.trainer = trainer != 0 ? trainer_bytes : NULL;
    spec.prg_code = code;
    spec.prg_code_len = code_len;
    spec.nmi_vector = 0x8000u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    size = ines_build(image, IMAGE_CAP, &spec);
    CHECK(size != 0u);
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    return (struct nesturbator *)inst;
}

static struct nesturbator *make_plain(void)
{
    static const uint8_t jmp_self[3] = {0x4cu, 0x00u, 0x80u};
    return make_instance(jmp_self, sizeof jmp_self, 0, 0);
}

/* Writes to $8000, $8001 and $FFFF land on NULL write pages (ROM) and still reach the hook, in
   order, each stamped one cycle after the last (D-14: the stamp is the write's own cycle index). */
static void check_write_stamps(void)
{
    struct nesturbator *nes = make_plain();
    CHECK(nes->map.cpu_w[16] == NULL);
    CHECK(nes->map.cpu_w[47] == NULL);
    mapper_test_install(nes, NESTURBATOR_WATCH_CPU_WRITE);
    nesturbator__bus_write(nes, 0x8000u, 0x11u);
    uint64_t c0 = nes->cpu_cycle - 1u; /* the write's own zero-based cycle index */
    nesturbator__bus_write(nes, 0x8001u, 0x22u);
    uint64_t c1 = nes->cpu_cycle - 1u; /* the write's own zero-based cycle index */
    nesturbator__bus_write(nes, 0xffffu, 0x33u);
    uint64_t c2 = nes->cpu_cycle - 1u; /* the write's own zero-based cycle index */
    CHECK_EQ_U64(mapper_test_write_count, 3u);
    CHECK_EQ_HEX(mapper_test_writes[0].addr, 0x8000u);
    CHECK_EQ_HEX(mapper_test_writes[1].addr, 0x8001u);
    CHECK_EQ_HEX(mapper_test_writes[2].addr, 0xffffu);
    CHECK_EQ_HEX(mapper_test_writes[0].value, 0x11u);
    CHECK_EQ_HEX(mapper_test_writes[1].value, 0x22u);
    CHECK_EQ_HEX(mapper_test_writes[2].value, 0x33u);
    CHECK_EQ_U64(mapper_test_writes[0].cpu_cycle, c0);
    CHECK_EQ_U64(mapper_test_writes[1].cpu_cycle, c1);
    CHECK_EQ_U64(mapper_test_writes[2].cpu_cycle, c2);
    CHECK_EQ_U64(c1, c0 + 1u);
    CHECK_EQ_U64(c2, c1 + 1u);
    nesturbator_destroy((nesturbator *)nes);
}

/* D-14: after a load cpu_cycle is 0 and the first write reaches the hook with stamp 0; the next
   carries 1. */
static void check_stamp_is_cycle_index(void)
{
    struct nesturbator *nes = make_plain();
    CHECK_EQ_U64(nes->cpu_cycle, 0u);
    mapper_test_install(nes, NESTURBATOR_WATCH_CPU_WRITE);
    nesturbator__bus_write(nes, 0x8000u, 0x11u);
    nesturbator__bus_write(nes, 0x8001u, 0x22u);
    CHECK_EQ_U64(mapper_test_write_count, 2u);
    CHECK_EQ_U64(mapper_test_writes[0].cpu_cycle, 0u);
    CHECK_EQ_U64(mapper_test_writes[1].cpu_cycle, 1u);
    CHECK_EQ_U64(nes->cpu_cycle, 2u);
    nesturbator_destroy((nesturbator *)nes);
}

/* WR-03: below $4020 there is no cartridge page; a read through the table is the open-bus latch. */
static void check_map_cpu_read_below_4020(void)
{
    struct nesturbator *nes = make_plain();
    nes->bus.open_bus = 0x9du;
    CHECK_EQ_HEX(nesturbator__map_cpu_read(nes, 0x4018u), 0x9du);
    CHECK_EQ_HEX(nesturbator__map_cpu_read(nes, 0x401fu), 0x9du);
    CHECK_EQ_HEX(nesturbator__map_cpu_read(nes, 0x0000u), 0x9du);
    nesturbator_destroy((nesturbator *)nes);
}

/* A trainer image has PRG RAM at $6000: the write both stores and reaches the hook. */
static void check_hook_on_writable_page(void)
{
    static const uint8_t jmp_self[3] = {0x4cu, 0x00u, 0x80u};
    struct nesturbator *nes = make_instance(jmp_self, sizeof jmp_self, 1, 0);
    mapper_test_install(nes, NESTURBATOR_WATCH_CPU_WRITE);
    nesturbator__bus_write(nes, 0x6000u, 0xa5u);
    CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x6000u), 0xa5u);
    CHECK_EQ_U64(mapper_test_write_count, 1u);
    CHECK_EQ_HEX(mapper_test_writes[0].addr, 0x6000u);
    nesturbator_destroy((nesturbator *)nes);
}

/* The oracle cpu_cycle * 24 == ticks holds for NTSC only (24 ticks per CPU cycle). D-04 keeps
   cpu_cycle because that ratio is not true on other regions. */
static void check_cpu_stamps(void)
{
    /* STA $8000; STA $8001; JMP $8006 */
    static const uint8_t code[] = {0x8du, 0x00u, 0x80u, 0x8du, 0x01u, 0x80u, 0x4cu, 0x06u, 0x80u};
    struct nesturbator *nes = make_instance(code, sizeof code, 0, 0);
    mapper_test_install(nes, NESTURBATOR_WATCH_CPU_WRITE);
    nesturbator__cpu_step(nes);
    CHECK_EQ_U64(nes->cpu_cycle * 24u, nes->ticks);
    nesturbator__cpu_step(nes);
    CHECK_EQ_U64(nes->cpu_cycle * 24u, nes->ticks);
    CHECK_EQ_U64(mapper_test_write_count, 2u);
    CHECK_EQ_U64(mapper_test_writes[1].cpu_cycle - mapper_test_writes[0].cpu_cycle, 4u);
    nesturbator_destroy((nesturbator *)nes);
}

/* An OAM DMA between two writes advances the stamp by the DMA's cycles: one write to $4014, 513
   or 514 DMA cycles, one read and the second write. The ticks ratio is an NTSC-only oracle. */
static void check_dma_stamps(void)
{
    struct nesturbator *nes = make_plain();
    mapper_test_install(nes, NESTURBATOR_WATCH_CPU_WRITE);
    nesturbator__bus_write(nes, 0x8000u, 0x01u);
    uint64_t ticks_before = nes->ticks;
    CHECK_EQ_U64(nes->cpu_cycle * 24u, nes->ticks);
    nesturbator__bus_write(nes, 0x4014u, 2u);
    (void)nesturbator__bus_read(nes, 0x0000u);
    nesturbator__bus_write(nes, 0x8001u, 0x02u);
    CHECK_EQ_U64(nes->cpu_cycle * 24u, nes->ticks);
    CHECK_EQ_U64(mapper_test_write_count, 2u);
    uint64_t delta = mapper_test_writes[1].cpu_cycle - mapper_test_writes[0].cpu_cycle;
    CHECK_EQ_U64(delta, (nes->ticks - ticks_before) / 24u);
    CHECK(delta == 516u || delta == 517u);
    nesturbator_destroy((nesturbator *)nes);
}

/* D-06: iNES flags 6 bit 0 clear is horizontal mirroring, set is vertical. */
static void check_nametable_map(void)
{
    static const uint8_t jmp_self[3] = {0x4cu, 0x00u, 0x80u};
    struct nesturbator *nes = make_instance(jmp_self, sizeof jmp_self, 0, 0);
    CHECK_EQ_U64(nes->map.nt[0], 0u);
    CHECK_EQ_U64(nes->map.nt[1], 0u);
    CHECK_EQ_U64(nes->map.nt[2], 1u);
    CHECK_EQ_U64(nes->map.nt[3], 1u);
    nesturbator_destroy((nesturbator *)nes);
    nes = make_instance(jmp_self, sizeof jmp_self, 0, 1);
    CHECK_EQ_U64(nes->map.nt[0], 0u);
    CHECK_EQ_U64(nes->map.nt[1], 1u);
    CHECK_EQ_U64(nes->map.nt[2], 0u);
    CHECK_EQ_U64(nes->map.nt[3], 1u);
    nesturbator_destroy((nesturbator *)nes);
}

/* NESdev "Bus conflict": the ROM drives the bus too, so the board sees the AND. */
static void check_bus_conflict(void)
{
    struct nesturbator *nes = make_plain();
    nes->cart.prg[0x10] = 0x5au;
    mapper_test_install(nes, NESTURBATOR_WATCH_CPU_WRITE | NESTURBATOR_WATCH_BUS_CONFLICT);
    nesturbator__bus_write(nes, 0x8010u, 0x0fu);
    CHECK_EQ_U64(mapper_test_write_count, 1u);
    CHECK_EQ_HEX(mapper_test_writes[0].value, 0x0au);
    mapper_test_install(nes, NESTURBATOR_WATCH_CPU_WRITE);
    nesturbator__bus_write(nes, 0x8010u, 0x0fu);
    CHECK_EQ_U64(mapper_test_write_count, 1u);
    CHECK_EQ_HEX(mapper_test_writes[0].value, 0x0fu);
    nesturbator_destroy((nesturbator *)nes);
}

static void check_watch_zero(void)
{
    struct nesturbator *nes = make_plain();
    mapper_test_install(nes, 0u);
    nesturbator__bus_write(nes, 0x8000u, 0x01u);
    CHECK_EQ_U64(mapper_test_write_count, 0u);
    nesturbator_destroy((nesturbator *)nes);
}

/* A zeroed instance has no cartridge: NULL pages, watch 0 (D-03, D-06). */
static void check_empty_instance(void)
{
    static struct nesturbator nes;
    memset(&nes, 0, sizeof nes);
    nes.bus.open_bus = 0xc3u;
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x5000u), 0xc3u);
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x9000u), 0xc3u);
    nesturbator__bus_write(&nes, 0x8000u, 0x12u); /* dropped; the NULL ops are never called */
    CHECK_EQ_U64(nes.map.watch, 0u);
    CHECK_EQ_U64(nesturbator__ppu_read(&nes, 0x0123u), 0u);
    nesturbator__ppu_write(&nes, 0x0123u, 0x55u); /* NULL CHR write page: dropped */
    CHECK_EQ_U64(nesturbator__ppu_read(&nes, 0x0123u), 0u);
}

/* D-03: a bank number is reduced with a true modulo against the validated size. */
static void check_bank_modulo(void)
{
    struct nesturbator *nes = make_plain();
    CHECK_EQ_U64(nes->cart.prg_size, 16384u);
    CHECK(nesturbator__map_prg(nes, 16u) == nesturbator__map_prg(nes, 0u));
    CHECK(nesturbator__map_prg(nes, 47u) == nes->cart.prg + 15u * 1024u);
    CHECK(nesturbator__map_chr(nes, 8u) == nesturbator__map_chr(nes, 0u));
    nesturbator_destroy((nesturbator *)nes);
}

/* D-05: the CPU IRQ line is frame (not inhibited) OR DMC OR mapper. */
static void check_irq_or(void)
{
    struct nesturbator *nes = make_plain();
    for (unsigned c = 0u; c < 8u; ++c) {
        nes->apu.frame_irq_inhibit = 0u;
        nes->apu.frame_irq = (uint8_t)(c & 1u);
        nes->apu.dmc.irq = (uint8_t)((c >> 1) & 1u);
        nes->mapper.irq = (uint8_t)((c >> 2) & 1u);
        nesturbator__irq_update(nes);
        CHECK_EQ_U64(nes->cpu.irq_line, c != 0u ? 1u : 0u);
    }
    nes->apu.frame_irq = 1u;
    nes->apu.frame_irq_inhibit = 1u;
    nes->apu.dmc.irq = 0u;
    nes->mapper.irq = 0u;
    nesturbator__irq_update(nes);
    CHECK_EQ_U64(nes->cpu.irq_line, 0u);

    nes->apu.frame_irq = 0u;
    nes->apu.frame_irq_inhibit = 0u;
    nes->mapper.irq = 1u;
    nes->apu.dmc.irq = 1u;
    nesturbator__irq_update(nes);
    CHECK_EQ_U64(nes->cpu.irq_line, 1u);
    nes->mapper.irq = 0u;
    nesturbator__irq_update(nes);
    CHECK_EQ_U64(nes->cpu.irq_line, 1u);
    nes->apu.dmc.irq = 0u;
    nesturbator__irq_update(nes);
    CHECK_EQ_U64(nes->cpu.irq_line, 0u);
    nesturbator_destroy((nesturbator *)nes);
}

static void check_irq_through_hook(void)
{
    struct nesturbator *nes = make_plain();
    mapper_test_install(nes, NESTURBATOR_WATCH_CPU_WRITE);
    nesturbator__bus_write(nes, 0xe000u, 1u);
    CHECK_EQ_U64(nes->cpu.irq_line, 1u);
    nesturbator__bus_write(nes, 0xe000u, 0u);
    CHECK_EQ_U64(nes->cpu.irq_line, 0u);
    nes->apu.dmc.irq = 1u;
    nesturbator__bus_write(nes, 0xe000u, 1u);
    nesturbator__bus_write(nes, 0xe000u, 0u);
    CHECK_EQ_U64(nes->cpu.irq_line, 1u);
    nesturbator_destroy((nesturbator *)nes);
}

/* Mapper registers live in nes->mapper, which a soft reset keeps; the reset runs 7 CPU cycles. */
static void check_reset_keeps_board(void)
{
    struct nesturbator *nes = make_plain();
    struct nesturbator__mapper before;
    nes->mapper.irq = 1u;
    memcpy(&before, &nes->mapper, sizeof before);
    uint64_t cycles = nes->cpu_cycle;
    CHECK_EQ_U64(nesturbator_reset((nesturbator *)nes), NESTURBATOR_OK);
    CHECK_EQ_U64(memcmp(&before, &nes->mapper, sizeof before), 0);
    CHECK_EQ_U64(nes->cpu_cycle, cycles + 7u);
    nesturbator_destroy((nesturbator *)nes);
}

static void regional_emit_store(uint8_t *code, size_t capacity, size_t *length, uint16_t addr,
                                uint8_t value)
{
    if (*length > capacity - 5u) {
        CHECK(0);
        return;
    }
    code[(*length)++] = 0xa9u; /* LDA #value */
    code[(*length)++] = value;
    code[(*length)++] = 0x8du; /* STA addr */
    code[(*length)++] = (uint8_t)addr;
    code[(*length)++] = (uint8_t)(addr >> 8u);
}

/* Each program plays one fixed-register pulse and then holds at a 3-cycle JMP. The actual board
   write is part of the same guest program, so the six-board acceptance never installs the fake
   mapper used by the seam tests above. */
static size_t regional_make_program(uint16_t mapper, uint16_t start, uint8_t *code, size_t capacity,
                                    uint16_t *loop_addr)
{
    size_t length = 0u;
    regional_emit_store(code, capacity, &length, 0x4015u, 0x01u);
    regional_emit_store(code, capacity, &length, 0x4000u, 0xbfu);
    regional_emit_store(code, capacity, &length, 0x4002u, (uint8_t)REGIONAL_TONE_TIMER);
    regional_emit_store(code, capacity, &length, 0x4003u, (uint8_t)(REGIONAL_TONE_TIMER >> 8u));

    switch (mapper) {
    case 1u:
        /* MMC1 serial Control = $0D: switchable-last-bank mode, page 1 mirroring. */
        for (unsigned bit = 0u; bit < 5u; ++bit)
            regional_emit_store(code, capacity, &length, 0x8000u, (uint8_t)((0x0du >> bit) & 1u));
        break;
    case 2u:
        regional_emit_store(code, capacity, &length, 0x8000u, 1u);
        break;
    case 3u:
        regional_emit_store(code, capacity, &length, 0x8000u, 1u);
        break;
    case 4u:
        regional_emit_store(code, capacity, &length, 0xc000u, 0u);
        regional_emit_store(code, capacity, &length, 0xc001u, 0u);
        regional_emit_store(code, capacity, &length, 0xe001u, 1u);
        regional_emit_store(code, capacity, &length, 0x2000u, 0x18u);
        regional_emit_store(code, capacity, &length, 0x2001u, 0x18u);
        break;
    case 7u:
        /* Keep PRG bank 0 while selecting AxROM's single-screen upper page. */
        regional_emit_store(code, capacity, &length, 0x8000u, 0x10u);
        break;
    default:
        break;
    }

    if (length > capacity - 3u) {
        CHECK(0);
        return 0u;
    }
    *loop_addr = (uint16_t)(start + length);
    code[length++] = 0x4cu;
    code[length++] = (uint8_t)*loop_addr;
    code[length++] = (uint8_t)(*loop_addr >> 8u);
    return length;
}

static size_t regional_build_image(uint8_t *image, size_t capacity, uint16_t mapper, int nes2,
                                   uint8_t timing, uint16_t *loop_addr)
{
    struct ines_spec spec;
    struct ines_segment segments[2];
    static const uint8_t uxrom_conflict_byte = 0xffu;
    size_t segment_count = 0u;
    uint8_t code[128];
    uint32_t code_offset;
    uint16_t code_addr;
    size_t code_len;

    memset(&spec, 0, sizeof spec);
    spec.nes2 = (uint8_t)(nes2 != 0);
    spec.mapper = mapper;
    switch (mapper) {
    case 0u:
        spec.prg_16k = 1u;
        spec.chr_8k = 1u;
        code_offset = 0u;
        code_addr = 0x8000u;
        break;
    case 1u:
        spec.prg_16k = 2u;
        spec.chr_8k = 0u;
        spec.prg_ram_shift = 7u;
        code_offset = INES_PRG_BANK;
        code_addr = 0xc000u;
        break;
    case 2u:
        spec.prg_16k = 2u;
        spec.chr_8k = 1u;
        spec.submapper = nes2 != 0 ? 1u : 0u;
        /* Legacy UxROM defaults to bus conflicts. Keep the bank-select write's ROM-side value
           high so both that profile and NES 2.0 submapper 1 select bank 1. */
        segments[segment_count].prg_offset = 0u;
        segments[segment_count].bytes = &uxrom_conflict_byte;
        segments[segment_count].len = 1u;
        segment_count++;
        code_offset = INES_PRG_BANK;
        code_addr = 0xc000u;
        break;
    case 3u:
        spec.prg_16k = 1u;
        spec.chr_8k = 2u;
        spec.submapper = nes2 != 0 ? 1u : 0u;
        code_offset = 0u;
        code_addr = 0x8000u;
        memset(regional_chr, 0, sizeof regional_chr);
        regional_chr[0] = 0x11u;
        regional_chr[INES_CHR_BANK] = 0x5au;
        spec.chr_data = regional_chr;
        spec.chr_data_len = sizeof regional_chr;
        break;
    case 4u:
        spec.prg_16k = 8u;
        spec.chr_8k = 0u;
        spec.submapper = 0u;
        code_offset = 15u * 8192u;
        code_addr = 0xe000u;
        break;
    case 7u:
        spec.prg_16k = 2u;
        spec.chr_8k = 0u;
        code_offset = 0u;
        code_addr = 0x8000u;
        break;
    default:
        CHECK(0);
        return 0u;
    }

    code_len = regional_make_program(mapper, code_addr, code, sizeof code, loop_addr);
    if (code_len == 0u)
        return 0u;
    segments[segment_count].prg_offset = code_offset;
    segments[segment_count].bytes = code;
    segments[segment_count].len = code_len;
    segment_count++;
    spec.segments = segments;
    spec.segment_count = segment_count;
    spec.reset_vector = code_addr;
    size_t image_size = ines_build(image, capacity, &spec);
    if (image_size != 0u && nes2 != 0)
        image[12] = timing;
    return image_size;
}

static nesturbator *regional_load(uint16_t mapper, int nes2, uint8_t timing,
                                  enum nesturbator_region_preference preference,
                                  uint16_t *loop_addr)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    if (inst == NULL)
        return NULL;
    size_t image_size = regional_build_image(regional_image, sizeof regional_image, mapper, nes2,
                                             timing, loop_addr);
    CHECK(image_size != 0u);
    if (image_size == 0u || nesturbator_load_cartridge_region(inst, regional_image, image_size,
                                                              preference) != NESTURBATOR_OK) {
        CHECK(0);
        nesturbator_destroy(inst);
        return NULL;
    }
    return inst;
}

static uint32_t regional_expected_samples(int pal, uint64_t *remainder)
{
    const uint64_t frame_ticks = pal != 0 ? REGIONAL_PAL_FRAME_TICKS : REGIONAL_NTSC_FRAME_TICKS;
    const uint64_t rate_num = pal != 0 ? 640u : 352u;
    const uint64_t rate_den = pal != 0 ? 709379u : 315000u;
    uint64_t accumulated = *remainder + frame_ticks * rate_num;
    *remainder = accumulated % rate_den;
    return (uint32_t)(accumulated / rate_den);
}

static uint64_t regional_expected_ticks(int pal, uint32_t frames)
{
    const uint64_t frame_ticks = pal != 0 ? REGIONAL_PAL_FRAME_TICKS : REGIONAL_NTSC_FRAME_TICKS;
    const uint64_t cpu_ticks = pal != 0 ? 32u : 24u;
    const uint64_t quantum = cpu_ticks * 3u; /* the owned guest's loop JMP */
    if (pal != 0) {
        uint64_t target = (uint64_t)frames * frame_ticks;
        return ((target + quantum - 1u) / quantum) * quantum;
    }
    uint64_t one_frame = ((frame_ticks + quantum - 1u) / quantum) * quantum;
    return (uint64_t)frames * one_frame;
}

static void regional_observe_transition(void *context, uint64_t cpu_cycle, int32_t level)
{
    struct regional_trace *trace = context;
    if (level > 0 && trace->previous_level <= 0 && trace->rising_count < REGIONAL_TRACE_RISES)
        trace->rising_cycles[trace->rising_count++] = cpu_cycle;
    trace->previous_level = level;
}

static uint64_t regional_expected_pitch_period_q8(int pal)
{
    const uint64_t pulse_period_cycles = UINT64_C(16) * (REGIONAL_TONE_TIMER + 1u);
    const uint64_t cpu_numerator =
        pal != 0 ? REGIONAL_PAL_TICKS_PER_SECOND : REGIONAL_NTSC_CPU_NUMERATOR;
    const uint64_t cpu_denominator = pal != 0 ? 32u : REGIONAL_NTSC_CPU_DENOMINATOR;
    const uint64_t sample_period_num =
        UINT64_C(48000) * pulse_period_cycles * cpu_denominator * 256u;
    return (sample_period_num + cpu_numerator / 2u) / cpu_numerator;
}

static unsigned regional_measure_pitch_lag(const int16_t *samples, uint32_t count,
                                           uint64_t expected_period_q8)
{
    if (count < 100u)
        return 0u;
    int64_t sum = 0;
    for (uint32_t i = 0u; i < count; ++i)
        sum += samples[i];
    const int32_t mean = (int32_t)(sum / count);
    const unsigned expected = (unsigned)((expected_period_q8 + 128u) / 256u);
    const unsigned first = expected > 3u ? expected - 3u : 1u;
    const unsigned last = expected + 3u;
    int64_t best_correlation = INT64_MIN;
    unsigned best_lag = 0u;
    for (unsigned lag = first; lag <= last; ++lag) {
        int64_t correlation = 0;
        for (uint32_t i = 0u; i + lag < count; ++i) {
            int64_t a = (int32_t)samples[i] - mean;
            int64_t b = (int32_t)samples[i + lag] - mean;
            correlation += a * b;
        }
        if (correlation > best_correlation) {
            best_correlation = correlation;
            best_lag = lag;
        }
    }
    return best_lag;
}

static void regional_check_board_behavior(uint16_t mapper, struct nesturbator *nes,
                                          uint16_t loop_addr)
{
    CHECK_EQ_U64(nes->mapper.id, mapper);
    CHECK_EQ_HEX(nes->cpu.pc, loop_addr);
    switch (mapper) {
    case 0u:
        CHECK(nes->map.cpu_r[16] == nes->cart.prg);
        CHECK(nes->map.cpu_r[32] == nes->cart.prg);
        break;
    case 1u:
        CHECK_EQ_HEX(nes->mapper.reg.mmc1.control_x ^ 0x0cu, 0x0du);
        CHECK_EQ_U64(nes->mapper.reg.mmc1.count, 0u);
        CHECK_EQ_U64(nes->mapper.reg.mmc1.last_write, 54u);
        CHECK(nes->map.cpu_r[32] == nes->cart.prg + INES_PRG_BANK);
        for (unsigned i = 0u; i < 4u; ++i)
            CHECK_EQ_U64(nes->map.nt[i], 1u);
        break;
    case 2u:
        CHECK_EQ_U64(nes->mapper.reg.uxrom.bank, 1u);
        CHECK(nes->map.cpu_r[16] == nes->cart.prg + INES_PRG_BANK);
        CHECK(nes->map.cpu_r[32] == nes->cart.prg + INES_PRG_BANK);
        break;
    case 3u:
        CHECK_EQ_U64(nes->mapper.reg.cnrom.bank, 1u);
        CHECK(nes->map.chr_r[0] == nes->cart.chr + INES_CHR_BANK);
        CHECK_EQ_HEX(nes->cart.chr[INES_CHR_BANK], 0x5au);
        break;
    case 4u:
        CHECK_EQ_U64(nes->mapper.reg.mmc3.latch, 0u);
        CHECK_EQ_U64(nes->mapper.reg.mmc3.irq_enable, 1u);
        CHECK_EQ_U64(nes->mapper.irq, 1u);
        CHECK_EQ_HEX(nes->ppu.mask, 0x18u);
        CHECK(nes->map.cpu_r[40] == nes->cart.prg + 15u * 8192u);
        break;
    case 7u:
        CHECK_EQ_HEX(nes->mapper.reg.axrom.bank, 0x10u);
        CHECK(nes->map.cpu_r[16] == nes->cart.prg);
        for (unsigned i = 0u; i < 4u; ++i)
            CHECK_EQ_U64(nes->map.nt[i], 1u);
        break;
    default:
        CHECK(0);
        break;
    }
}

static void check_regional_mapper_matrix(void)
{
    static const uint16_t mapper_ids[] = {0u, 1u, 2u, 3u, 4u, 7u};
    for (size_t m = 0u; m < sizeof mapper_ids / sizeof mapper_ids[0]; ++m) {
        const uint16_t mapper = mapper_ids[m];
        uint16_t pal_loop = 0u;
        uint16_t peer_loop = 0u;
        uint16_t ntsc_loop = 0u;
        nesturbator *pal =
            regional_load(mapper, 1, 1u, NESTURBATOR_REGION_PREFERENCE_AUTO, &pal_loop);
        nesturbator *pal_peer =
            regional_load(mapper, 0, 0u, NESTURBATOR_REGION_PREFERENCE_PAL, &peer_loop);
        nesturbator *ntsc =
            regional_load(mapper, 1, 0u, NESTURBATOR_REGION_PREFERENCE_AUTO, &ntsc_loop);
        CHECK(pal != NULL && pal_peer != NULL && ntsc != NULL);
        if (pal == NULL || pal_peer == NULL || ntsc == NULL) {
            nesturbator_destroy(pal);
            nesturbator_destroy(pal_peer);
            nesturbator_destroy(ntsc);
            continue;
        }

        CHECK_EQ_U64(nesturbator_get_region(pal), NESTURBATOR_REGION_PAL);
        CHECK_EQ_U64(nesturbator_get_region(pal_peer), NESTURBATOR_REGION_PAL);
        CHECK_EQ_U64(nesturbator_get_region(ntsc), NESTURBATOR_REGION_NTSC);
        CHECK_EQ_U64(pal_loop, peer_loop);
        CHECK_EQ_U64(pal_loop, ntsc_loop);

        nesturbator_info info;
        memset(&info, 0, sizeof info);
        info.size = (uint32_t)sizeof info;
        nesturbator_get_info(pal, &info);
        CHECK_EQ_U64(info.width, 256u);
        CHECK_EQ_U64(info.height, 240u);
        CHECK_EQ_U64(info.fps_num, 322445u);
        CHECK_EQ_U64(info.fps_den, 6448u);
        memset(&info, 0, sizeof info);
        info.size = (uint32_t)sizeof info;
        nesturbator_get_info(ntsc, &info);
        CHECK_EQ_U64(info.fps_num, 39375000u);
        CHECK_EQ_U64(info.fps_den, 655171u);

        struct regional_trace trace;
        memset(&trace, 0, sizeof trace);
        nesturbator__apu_set_transition_sink((struct nesturbator *)pal, regional_observe_transition,
                                             &trace);
        uint64_t pal_remainder = 0u;
        uint64_t peer_remainder = 0u;
        uint64_t ntsc_remainder = 0u;
        uint64_t pal_sample_total = 0u;
        uint64_t peer_sample_total = 0u;
        uint64_t ntsc_sample_total = 0u;
        uint32_t pal_capture = 0u;
        uint32_t ntsc_capture = 0u;
        int16_t pal_audio[1024];
        int16_t peer_audio[1024];
        int16_t ntsc_audio[1024];

        for (uint32_t frame = 0u; frame < REGIONAL_FRAME_COUNT; ++frame) {
            nesturbator_frame pal_io;
            nesturbator_frame peer_io;
            nesturbator_frame ntsc_io;
            memset(&pal_io, 0, sizeof pal_io);
            pal_io.size = (uint32_t)sizeof pal_io;
            pal_io.video = regional_video;
            pal_io.video_pitch = 256u;
            pal_io.audio = pal_audio;
            pal_io.audio_capacity = 1024u;
            CHECK_EQ_U64(nesturbator_run_frame(pal, &pal_io), NESTURBATOR_OK);

            memset(&peer_io, 0, sizeof peer_io);
            peer_io.size = (uint32_t)sizeof peer_io;
            peer_io.video = regional_video_peer;
            peer_io.video_pitch = 256u;
            peer_io.audio = peer_audio;
            peer_io.audio_capacity = 1024u;
            CHECK_EQ_U64(nesturbator_run_frame(pal_peer, &peer_io), NESTURBATOR_OK);

            memset(&ntsc_io, 0, sizeof ntsc_io);
            ntsc_io.size = (uint32_t)sizeof ntsc_io;
            ntsc_io.video = regional_video_ntsc;
            ntsc_io.video_pitch = 256u;
            ntsc_io.audio = ntsc_audio;
            ntsc_io.audio_capacity = 1024u;
            CHECK_EQ_U64(nesturbator_run_frame(ntsc, &ntsc_io), NESTURBATOR_OK);

            const uint32_t expected_pal = regional_expected_samples(1, &pal_remainder);
            const uint32_t expected_peer = regional_expected_samples(1, &peer_remainder);
            const uint32_t expected_ntsc = regional_expected_samples(0, &ntsc_remainder);
            pal_sample_total += expected_pal;
            peer_sample_total += expected_peer;
            ntsc_sample_total += expected_ntsc;
            CHECK_EQ_U64(pal_io.audio_count, expected_pal);
            CHECK_EQ_U64(peer_io.audio_count, expected_peer);
            CHECK_EQ_U64(ntsc_io.audio_count, expected_ntsc);
            CHECK_EQ_U64(pal_io.frame_number, frame + 1u);
            CHECK_EQ_U64(peer_io.frame_number, frame + 1u);
            CHECK_EQ_U64(ntsc_io.frame_number, frame + 1u);
            CHECK_EQ_U64(pal_io.ticks, regional_expected_ticks(1, frame + 1u));
            CHECK_EQ_U64(peer_io.ticks, regional_expected_ticks(1, frame + 1u));
            CHECK_EQ_U64(ntsc_io.ticks, regional_expected_ticks(0, frame + 1u));

            struct nesturbator *pal_machine = (struct nesturbator *)pal;
            struct nesturbator *peer_machine = (struct nesturbator *)pal_peer;
            struct nesturbator *ntsc_machine = (struct nesturbator *)ntsc;
            CHECK_EQ_U64(pal_machine->ticks, pal_machine->cpu_cycle * 32u);
            CHECK_EQ_U64(peer_machine->ticks, peer_machine->cpu_cycle * 32u);
            CHECK_EQ_U64(ntsc_machine->ticks, ntsc_machine->cpu_cycle * 24u);
            CHECK_EQ_U64(pal_machine->ppu.ppu_ticks, (pal_machine->ticks / 10u) * 10u);
            uint64_t pal_dots = pal_machine->ppu.ppu_ticks / 10u;
            uint64_t pal_dot_in_frame = pal_dots % (312u * 341u);
            CHECK_EQ_U64(pal_machine->ppu.scanline, pal_dot_in_frame / 341u);
            CHECK_EQ_U64(pal_machine->ppu.dot, pal_dot_in_frame % 341u);

            CHECK_EQ_U64(peer_io.audio_count, pal_io.audio_count);
            CHECK(memcmp(pal_audio, peer_audio, (size_t)pal_io.audio_count * sizeof pal_audio[0]) ==
                  0);
            CHECK(memcmp(regional_video, regional_video_peer, sizeof regional_video) == 0);
            if (frame == 0u && mapper == 0u) {
                /* Exact PAL arithmetic rejects a rounded 50 Hz, 960-samples/frame model. */
                CHECK_EQ_U64(expected_pal, 959u);
            }
            if (frame != 0u) {
                uint32_t pal_copy = pal_io.audio_count;
                uint32_t ntsc_copy = ntsc_io.audio_count;
                if (pal_copy > REGIONAL_PCM_CAP - pal_capture)
                    pal_copy = REGIONAL_PCM_CAP - pal_capture;
                if (ntsc_copy > REGIONAL_PCM_CAP - ntsc_capture)
                    ntsc_copy = REGIONAL_PCM_CAP - ntsc_capture;
                memcpy(regional_pal_pcm + pal_capture, pal_audio,
                       (size_t)pal_copy * sizeof pal_audio[0]);
                memcpy(regional_ntsc_pcm + ntsc_capture, ntsc_audio,
                       (size_t)ntsc_copy * sizeof ntsc_audio[0]);
                pal_capture += pal_copy;
                ntsc_capture += ntsc_copy;
            }
        }

        CHECK(pal_remainder == peer_remainder);
        CHECK(pal_remainder != 0u);
        CHECK(pal_sample_total == peer_sample_total);
        CHECK(pal_sample_total != (uint64_t)REGIONAL_FRAME_COUNT * 960u);
        CHECK(ntsc_sample_total != pal_sample_total);
        regional_check_board_behavior(mapper, (struct nesturbator *)pal, pal_loop);
        regional_check_board_behavior(mapper, (struct nesturbator *)pal_peer, peer_loop);
        regional_check_board_behavior(mapper, (struct nesturbator *)ntsc, ntsc_loop);

        CHECK(trace.rising_count >= 3u);
        const uint64_t pulse_period_cycles = UINT64_C(16) * (REGIONAL_TONE_TIMER + 1u);
        for (uint32_t i = 1u; i < trace.rising_count; ++i)
            CHECK_EQ_U64(trace.rising_cycles[i] - trace.rising_cycles[i - 1u], pulse_period_cycles);

        const uint64_t pal_period_q8 = regional_expected_pitch_period_q8(1);
        const uint64_t ntsc_period_q8 = regional_expected_pitch_period_q8(0);
        const unsigned pal_lag =
            regional_measure_pitch_lag(regional_pal_pcm, pal_capture, pal_period_q8);
        const unsigned ntsc_lag =
            regional_measure_pitch_lag(regional_ntsc_pcm, ntsc_capture, ntsc_period_q8);
        CHECK(pal_lag != 0u && ntsc_lag != 0u);
        CHECK((uint64_t)pal_lag * 256u + 256u >= pal_period_q8 &&
              (uint64_t)pal_lag * 256u <= pal_period_q8 + 256u);
        CHECK((uint64_t)ntsc_lag * 256u + 256u >= ntsc_period_q8 &&
              (uint64_t)ntsc_lag * 256u <= ntsc_period_q8 + 256u);
        CHECK((uint64_t)pal_lag * 256u + 256u < ntsc_period_q8 ||
              (uint64_t)ntsc_lag * 256u + 256u < pal_period_q8);

        nesturbator_destroy(pal);
        nesturbator_destroy(pal_peer);
        nesturbator_destroy(ntsc);
    }
}

int main(void)
{
    check_write_stamps();
    check_stamp_is_cycle_index();
    check_map_cpu_read_below_4020();
    check_hook_on_writable_page();
    check_cpu_stamps();
    check_dma_stamps();
    check_nametable_map();
    check_bus_conflict();
    check_watch_zero();
    check_empty_instance();
    check_bank_modulo();
    check_irq_or();
    check_irq_through_hook();
    check_reset_keeps_board();
    check_regional_mapper_matrix();
    CHECK_DONE();
}
