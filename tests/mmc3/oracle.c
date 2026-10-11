/* mmc3-oracle --rom FILE --frames N --protocol 6000|f8: runs one blargg MMC3
   ROM and judges it (09 D-11). Test-only; the shipped runner knows nothing of
   this. Protocol 6000 reads $6000-$600x through the mapper's CPU page table;
   protocol f8 reads zero page $F8 through nesturbator_peek_cpu_ram, then runs
   60 more frames and reads it again. Prints one line
   "mmc3-oracle: <file> <result> code 0xNN frames N" and exits with the code
   from oracle.h. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "nesturbator.h"
#include "oracle.h"

#define IMAGE_CAP (1u << 20)
#define MORE_FRAMES 60u

static uint16_t video[256u * 240u];
static int16_t audio[2048];

static int step(nesturbator *inst)
{
    nesturbator_frame io;
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = video;
    io.video_pitch = 256u;
    io.audio = audio;
    io.audio_capacity = 2048u;
    return nesturbator_run_frame(inst, &io) == NESTURBATOR_OK;
}

static int finish(const char *rom, const char *result, int code, uint32_t frames)
{
    const char *base = strrchr(rom, '/');
    printf("mmc3-oracle: %s %s code 0x%02X frames %lu\n", base != NULL ? base + 1 : rom, result,
           (unsigned)code, (unsigned long)frames);
    return code;
}

static const char *result_word(int code)
{
    if (code == 0)
        return "pass";
    if (code == MMC3_ORACLE_EXIT_TIMEOUT)
        return "timeout";
    if (code == MMC3_ORACLE_EXIT_NO_SIGNATURE)
        return "no-signature";
    return "reported";
}

int main(int argc, char **argv)
{
    static uint8_t image[IMAGE_CAP];
    nesturbator_config cfg = NESTURBATOR_CONFIG_INIT;
    nesturbator *inst = NULL;
    const char *rom = NULL;
    const char *protocol = NULL;
    unsigned long budget = 0;
    size_t size;
    FILE *f;
    int code = -1;
    int seen_running = 0;
    uint32_t frames = 0;
    int i;

    for (i = 1; i + 1 < argc; i += 2) {
        if (strcmp(argv[i], "--rom") == 0)
            rom = argv[i + 1];
        else if (strcmp(argv[i], "--frames") == 0)
            budget = strtoul(argv[i + 1], NULL, 10);
        else if (strcmp(argv[i], "--protocol") == 0)
            protocol = argv[i + 1];
        else
            break;
    }
    if (i != argc || rom == NULL || budget == 0 || protocol == NULL ||
        (strcmp(protocol, "6000") != 0 && strcmp(protocol, "f8") != 0)) {
        fprintf(stderr, "usage: mmc3-oracle --rom FILE --frames N --protocol 6000|f8\n");
        return MMC3_ORACLE_EXIT_USAGE;
    }
    f = fopen(rom, "rb");
    if (f == NULL) {
        fprintf(stderr, "mmc3-oracle: cannot open %s\n", rom);
        return MMC3_ORACLE_EXIT_LOAD;
    }
    size = fread(image, 1, IMAGE_CAP, f);
    fclose(f);
    if (nesturbator_create(&cfg, &inst) != NESTURBATOR_OK ||
        nesturbator_load_cartridge(inst, image, size) != NESTURBATOR_OK) {
        nesturbator_destroy(inst);
        return finish(rom, "load-failed", MMC3_ORACLE_EXIT_LOAD, 0);
    }

    if (strcmp(protocol, "6000") == 0) {
        uint8_t head[4] = {0, 0, 0, 0};
        char text[256];
        while (frames < budget) {
            uint32_t k;
            if (!step(inst))
                break;
            frames++;
            for (k = 0; k < 4u; ++k)
                head[k] = nesturbator__map_cpu_read(inst, (uint16_t)(0x6000u + k));
            for (k = 0; k + 1u < sizeof text; ++k) {
                text[k] = (char)nesturbator__map_cpu_read(inst, (uint16_t)(0x6004u + k));
                if (text[k] == '\0')
                    break;
            }
            text[sizeof text - 1u] = '\0';
            code = mmc3_oracle_judge_6000(head, text, seen_running);
            if (head[0] == 0x80u && head[1] == 0xDEu)
                seen_running = 1;
            /* No signature yet is not final: the ROM may not have written it. */
            if (code >= 0 && code != MMC3_ORACLE_EXIT_NO_SIGNATURE)
                break;
        }
        if (code < 0 || code == MMC3_ORACLE_EXIT_NO_SIGNATURE)
            code = mmc3_oracle_judge_6000_budget(head);
    } else {
        uint8_t at_budget = 0, after_more = 0;
        while (frames < budget && step(inst))
            frames++;
        (void)nesturbator_peek_cpu_ram(inst, 0x00F8u, &at_budget);
        for (i = 0; i < (int)MORE_FRAMES && step(inst); ++i)
            frames++;
        (void)nesturbator_peek_cpu_ram(inst, 0x00F8u, &after_more);
        code = mmc3_oracle_judge_f8(at_budget, after_more);
    }
    nesturbator_destroy(inst);
    return finish(rom, result_word(code), code, frames);
}
