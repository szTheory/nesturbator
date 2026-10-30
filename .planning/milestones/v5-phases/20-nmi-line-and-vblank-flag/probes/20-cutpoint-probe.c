#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "nesturbator.h"
#include "ines.h"

#define IMAGE_CAP (16u + INES_PRG_BANK + INES_CHR_BANK)

static uint8_t image[IMAGE_CAP];

static int load_fixture(struct nesturbator **out)
{
    static const uint8_t loop[] = {0x4cu, 0x00u, 0x80u};
    struct ines_spec spec;
    nesturbator_config cfg;
    nesturbator *pub = NULL;
    size_t n;

    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = 1u;
    spec.prg_code = loop;
    spec.prg_code_len = sizeof loop;
    spec.nmi_vector = 0x8000u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    n = ines_build(image, sizeof image, &spec);
    if (n == 0u)
        return 0;

    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    if (nesturbator_create(&cfg, &pub) != NESTURBATOR_OK)
        return 0;
    if (nesturbator_load_cartridge(pub, image, n) != NESTURBATOR_OK) {
        nesturbator_destroy(pub);
        return 0;
    }
    *out = (struct nesturbator *)pub;
    return 1;
}

static int run_cutpoint(const char *name, uint64_t dots, uint32_t cycles, uint16_t line,
                        uint16_t dot, uint8_t vblank)
{
    struct nesturbator *a = NULL;
    struct nesturbator *b = NULL;
    size_t n;
    uint8_t *state = NULL;
    uint8_t *copy = NULL;
    uint8_t *next_a = NULL;
    uint8_t *next_b = NULL;
    uint64_t steps = cycles / 3u;
    int ok = (cycles % 3u == 0u);

    if (!ok || !load_fixture(&a) || !load_fixture(&b))
        return 0;

    /* A valid rendering-off internal fixture origin chosen by the phase context. */
    a->ticks = 0u;
    a->ppu.ppu_ticks = 0u;
    a->ppu.scanline = 261u;
    a->ppu.dot = 0u;
    a->ppu.odd_frame = 0u;
    a->ppu.mask = 0u;
    a->ppu.control = 0u;
    a->ppu.status = 0u;
    a->ppu.vblank_suppress = 0u;
    a->cpu.pc = 0x8000u;
    a->cpu.nmi_pending = 0u;
    a->cpu.nmi_prev = 0u;

    /* Seed saved APU/synth history without producing a frame-scoped PCM buffer. */
    nesturbator__apu_write(a, 0x4011u, 64u);
    if (a->synth.mixed_level <= 0 || a->apu.sample_output != NULL || a->synth.pcm_count != 0u)
        ok = 0;

    /* JMP absolute is exactly three CPU cycles; every endpoint is an instruction boundary. */
    for (uint64_t i = 0u; i < steps; ++i)
        nesturbator__cpu_step(a);

    if (a->cpu_cycle != cycles || a->ticks != (uint64_t)cycles * 24u ||
        a->ppu.ppu_ticks != dots * 8u || a->ppu.scanline != line || a->ppu.dot != dot ||
        ((a->ppu.status >> 7) & 1u) != vblank || a->ppu.control != 0u || a->cpu.nmi_pending != 0u ||
        a->apu.sample_phase == 0u || a->synth.mixed_level <= 0 || a->apu.sample_output != NULL ||
        a->synth.pcm_count != 0u)
        ok = 0;

    printf("%s exact checkpoint: cpu_cycles=%u instructions=%llu ticks=%llu ppu_ticks=%llu "
           "ppu=(%u,%u) status=%02x control=%02x nmi_pending=%u audio_phase=%u synth_level=%d "
           "impulse0=%lld pcm_count=%u\n",
           name, cycles, (unsigned long long)steps, (unsigned long long)a->ticks,
           (unsigned long long)a->ppu.ppu_ticks, (unsigned)a->ppu.scanline, (unsigned)a->ppu.dot,
           (unsigned)a->ppu.status, (unsigned)a->ppu.control, (unsigned)a->cpu.nmi_pending,
           (unsigned)a->apu.sample_phase, a->synth.mixed_level, (long long)a->synth.impulse[0],
           (unsigned)a->synth.pcm_count);

    n = nesturbator_state_size((nesturbator *)a);
    state = (uint8_t *)malloc(n);
    copy = (uint8_t *)malloc(n);
    next_a = (uint8_t *)malloc(n);
    next_b = (uint8_t *)malloc(n);
    if (!state || !copy || !next_a || !next_b)
        ok = 0;
    if (ok && nesturbator_save_state((nesturbator *)a, state, n) != NESTURBATOR_OK)
        ok = 0;
    if (ok && nesturbator_load_state((nesturbator *)b, state, n) != NESTURBATOR_OK)
        ok = 0;
    if (ok && (nesturbator_save_state((nesturbator *)b, copy, n) != NESTURBATOR_OK ||
               memcmp(state, copy, n) != 0))
        ok = 0;
    if (ok) {
        for (unsigned i = 0u; i < 7u; ++i) {
            nesturbator__cpu_step(a);
            nesturbator__cpu_step(b);
        }
        if (nesturbator_save_state((nesturbator *)a, next_a, n) != NESTURBATOR_OK ||
            nesturbator_save_state((nesturbator *)b, next_b, n) != NESTURBATOR_OK ||
            memcmp(next_a, next_b, n) != 0)
            ok = 0;
    }

    printf("%s save/load/continuation: %s (canonical state equal at load and after seven whole "
           "instructions)\n",
           name, ok ? "PASS" : "FAIL");
    free(state);
    free(copy);
    free(next_a);
    free(next_b);
    nesturbator_destroy((nesturbator *)a);
    nesturbator_destroy((nesturbator *)b);
    return ok;
}

int main(void)
{
    int set = run_cutpoint("VBlank set", 261207u, 87069u, 241u, 1u, 1u);
    int clear = run_cutpoint("VBlank clear", 89343u, 29781u, 261u, 1u, 0u);
    return set && clear ? 0 : 1;
}
