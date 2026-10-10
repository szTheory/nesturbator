/* FRAME-04: loads the built libretro module at run time and calls it in the
 * order a frontend such as RetroArch does (01-RESEARCH Pattern 3), then
 * compares the frame it receives with the runner's P6 image.
 *
 *   libretro.host <module> <testframe.ppm> <content.ppm> <runner> <generated.nes>
 *
 * The four hard-coded pixels (D-05) do not go through the shared palette
 * path, so an indexing bug there cannot cancel out in the comparison. */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../check.h"
#include "../ines.h"
#include "../../src/internal.h"
#include "libretro.h"
#include "nesturbator.h"
#include "movie_fixture.h"
#include "../test_process.h"

#define W 256u
#define H 240u
#define AUDIO_CAPTURE_CAPACITY 1024u

/* ---- module loading ---- */

#ifdef _WIN32
static HMODULE module;
static int module_open(const char *path)
{
    module = LoadLibraryA(path);
    return module != NULL;
}
static void *module_sym(const char *name)
{
    FARPROC f = GetProcAddress(module, name);
    void *p = NULL;
    memcpy(&p, &f, sizeof p);
    return p;
}
static void module_close(void)
{
    FreeLibrary(module);
}
#else
static void *module;
static int module_open(const char *path)
{
    module = dlopen(path, RTLD_NOW);
    if (module == NULL) {
        fprintf(stderr, "dlopen: %s\n", dlerror());
    }
    return module != NULL;
}
static void *module_sym(const char *name)
{
    return dlsym(module, name);
}
static void module_close(void)
{
    (void)dlclose(module);
}
#endif

/* The 25 entry points, typed as libretro.h declares them (L7511-7840). */
static void(RETRO_CALLCONV *p_set_environment)(retro_environment_t);
static void(RETRO_CALLCONV *p_set_video_refresh)(retro_video_refresh_t);
static void(RETRO_CALLCONV *p_set_audio_sample)(retro_audio_sample_t);
static void(RETRO_CALLCONV *p_set_audio_sample_batch)(retro_audio_sample_batch_t);
static void(RETRO_CALLCONV *p_set_input_poll)(retro_input_poll_t);
static void(RETRO_CALLCONV *p_set_input_state)(retro_input_state_t);
static void(RETRO_CALLCONV *p_init)(void);
static void(RETRO_CALLCONV *p_deinit)(void);
static unsigned(RETRO_CALLCONV *p_api_version)(void);
static void(RETRO_CALLCONV *p_get_system_info)(struct retro_system_info *);
static void(RETRO_CALLCONV *p_get_system_av_info)(struct retro_system_av_info *);
static void(RETRO_CALLCONV *p_set_controller_port_device)(unsigned, unsigned);
static void(RETRO_CALLCONV *p_reset)(void);
static void(RETRO_CALLCONV *p_run)(void);
static size_t(RETRO_CALLCONV *p_serialize_size)(void);
static bool(RETRO_CALLCONV *p_serialize)(void *, size_t);
static bool(RETRO_CALLCONV *p_unserialize)(const void *, size_t);
static void(RETRO_CALLCONV *p_cheat_reset)(void);
static void(RETRO_CALLCONV *p_cheat_set)(unsigned, bool, const char *);
static bool(RETRO_CALLCONV *p_load_game)(const struct retro_game_info *);
static bool(RETRO_CALLCONV *p_load_game_special)(unsigned, const struct retro_game_info *, size_t);
static void(RETRO_CALLCONV *p_unload_game)(void);
static unsigned(RETRO_CALLCONV *p_get_region)(void);
static void *(RETRO_CALLCONV *p_get_memory_data)(unsigned);
static size_t(RETRO_CALLCONV *p_get_memory_size)(unsigned);

static int missing = 0;

/* ISO C has no conversion from void * to a function pointer, so the
   address is copied byte for byte (01-RESEARCH Pitfall 8). */
#define RESOLVE(var, name)                                                                         \
    do {                                                                                           \
        void *p_ = module_sym(name);                                                               \
        if (p_ == NULL) {                                                                          \
            fprintf(stderr, "missing symbol %s\n", name);                                          \
            missing++;                                                                             \
        } else {                                                                                   \
            memcpy(&(var), &p_, sizeof(var));                                                      \
        }                                                                                          \
    } while (0)

static void resolve_all(void)
{
    RESOLVE(p_set_environment, "retro_set_environment");
    RESOLVE(p_set_video_refresh, "retro_set_video_refresh");
    RESOLVE(p_set_audio_sample, "retro_set_audio_sample");
    RESOLVE(p_set_audio_sample_batch, "retro_set_audio_sample_batch");
    RESOLVE(p_set_input_poll, "retro_set_input_poll");
    RESOLVE(p_set_input_state, "retro_set_input_state");
    RESOLVE(p_init, "retro_init");
    RESOLVE(p_deinit, "retro_deinit");
    RESOLVE(p_api_version, "retro_api_version");
    RESOLVE(p_get_system_info, "retro_get_system_info");
    RESOLVE(p_get_system_av_info, "retro_get_system_av_info");
    RESOLVE(p_set_controller_port_device, "retro_set_controller_port_device");
    RESOLVE(p_reset, "retro_reset");
    RESOLVE(p_run, "retro_run");
    RESOLVE(p_serialize_size, "retro_serialize_size");
    RESOLVE(p_serialize, "retro_serialize");
    RESOLVE(p_unserialize, "retro_unserialize");
    RESOLVE(p_cheat_reset, "retro_cheat_reset");
    RESOLVE(p_cheat_set, "retro_cheat_set");
    RESOLVE(p_load_game, "retro_load_game");
    RESOLVE(p_load_game_special, "retro_load_game_special");
    RESOLVE(p_unload_game, "retro_unload_game");
    RESOLVE(p_get_region, "retro_get_region");
    RESOLVE(p_get_memory_data, "retro_get_memory_data");
    RESOLVE(p_get_memory_size, "retro_get_memory_size");
}

/* ---- frontend callbacks: record what the core does ---- */

static int no_game_calls, no_game_value;
static int pixel_format_calls;
static enum retro_pixel_format pixel_format = RETRO_PIXEL_FORMAT_0RGB1555;
static int video_calls;
static unsigned video_width, video_height;
static size_t video_pitch;
static uint32_t frame[W * H];
static int poll_calls;
static int input_calls;
static uint8_t host_buttons[2];
static int sample_calls;
static int16_t sample_samples[AUDIO_CAPTURE_CAPACITY * 2u];
static int batch_calls;
static size_t batch_frames;
static int batch_nonzero;
static int16_t batch_samples[AUDIO_CAPTURE_CAPACITY * 2u];

static bool RETRO_CALLCONV env(unsigned cmd, void *data)
{
    if (cmd == RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME) {
        no_game_calls++;
        no_game_value = *(const bool *)data ? 1 : 0;
        return true;
    }
    if (cmd == RETRO_ENVIRONMENT_SET_PIXEL_FORMAT) {
        pixel_format_calls++;
        pixel_format = *(const enum retro_pixel_format *)data;
        return true;
    }
    return false;
}

static void RETRO_CALLCONV video(const void *data, unsigned width, unsigned height, size_t pitch)
{
    unsigned y;
    video_calls++;
    video_width = width;
    video_height = height;
    video_pitch = pitch;
    if (data == NULL || width != W || height != H || pitch < W * 4u) {
        return;
    }
    for (y = 0; y < H; y++) {
        memcpy(&frame[y * W], (const unsigned char *)data + y * pitch, W * 4u);
    }
}

static void RETRO_CALLCONV sample(int16_t left, int16_t right)
{
    if ((size_t)sample_calls < AUDIO_CAPTURE_CAPACITY) {
        sample_samples[2u * (size_t)sample_calls] = left;
        sample_samples[2u * (size_t)sample_calls + 1u] = right;
    }
    sample_calls++;
}

static size_t RETRO_CALLCONV batch(const int16_t *data, size_t frames)
{
    size_t i;
    batch_calls++;
    batch_frames = frames;
    CHECK(frames <= AUDIO_CAPTURE_CAPACITY);
    if (frames <= AUDIO_CAPTURE_CAPACITY)
        memcpy(batch_samples, data, frames * 2u * sizeof batch_samples[0]);
    for (i = 0; i < frames * 2u; i++) {
        if (data[i] != 0) {
            batch_nonzero++;
        }
    }
    return frames;
}

static void RETRO_CALLCONV on_poll(void)
{
    poll_calls++;
}

static int16_t RETRO_CALLCONV input(unsigned port, unsigned device, unsigned index, unsigned id)
{
    static const uint8_t button_bits[9] = {NESTURBATOR_BUTTON_B,      0u,
                                           NESTURBATOR_BUTTON_SELECT, NESTURBATOR_BUTTON_START,
                                           NESTURBATOR_BUTTON_UP,     NESTURBATOR_BUTTON_DOWN,
                                           NESTURBATOR_BUTTON_LEFT,   NESTURBATOR_BUTTON_RIGHT,
                                           NESTURBATOR_BUTTON_A};
    input_calls++;
    if (port > 1u || device != RETRO_DEVICE_JOYPAD || index != 0u || id >= 9u) {
        CHECK(0);
        return 0;
    }
    return (host_buttons[port] & button_bits[id]) != 0u ? 1 : 0;
}

static void compare_with_ppm(const char *path);

static void check_input_frame_parity(unsigned char *image, size_t image_size, const char *runner,
                                     const char *rom_path, const char *ppm_path)
{
    static const uint8_t program[] = {
        0xa9, 0x3f, 0x8d, 0x06, 0x20, 0xa9, 0x00, 0x8d, 0x06, 0x20, 0xa9, 0x01, 0x8d, 0x16, 0x40,
        0xa9, 0x00, 0x8d, 0x16, 0x40, 0xad, 0x16, 0x40, 0x29, 0x01, 0xd0, 0x05, 0xa9, 0x27, 0x4c,
        0x22, 0x80, 0xa9, 0x16, 0x8d, 0x07, 0x20, 0xa9, 0x0a, 0x8d, 0x01, 0x20, 0x4c, 0x00, 0x80};
    nesturbator_config cfg;
    nesturbator_input scripted;
    nesturbator *nes = NULL;
    nesturbator_frame io;
    struct retro_game_info game;
    uint16_t native[W * H];
    int16_t audio[1024];
    uint32_t palette[512];
    uint32_t mismatches = 0u;

    p_unload_game();
    memset(image, 0, image_size);
    image[0] = 'N';
    image[1] = 'E';
    image[2] = 'S';
    image[3] = 0x1a;
    image[4] = 1;
    image[5] = 1;
    memcpy(image + 16u, program, sizeof program);
    image[16u + 0x3ffau] = 0x00;
    image[16u + 0x3ffbu] = 0x80;
    image[16u + 0x3ffcu] = 0x00;
    image[16u + 0x3ffdu] = 0x80;
    image[16u + 0x3ffeu] = 0x00;
    image[16u + 0x3fffu] = 0x80;
    memset(&game, 0, sizeof game);
    game.data = image;
    game.size = image_size;
    CHECK(p_load_game(&game));

    host_buttons[0] = MOVIE_FIXTURE_PORT0;
    host_buttons[1] = MOVIE_FIXTURE_PORT1;
    input_calls = 0;
    video_calls = 0;
    p_run();
    CHECK_EQ_U64(video_calls, 1u);
    CHECK_EQ_U64(input_calls, 16u);

    /* Replay the same owned fixture through the runner and compare host pixels. */
    {
        unsigned char movie_bytes[20] = {'N',
                                         'M',
                                         'O',
                                         'V',
                                         'I',
                                         'E',
                                         '1',
                                         0,
                                         1,
                                         0,
                                         0,
                                         0,
                                         1,
                                         0,
                                         0,
                                         0,
                                         MOVIE_FIXTURE_PORT0,
                                         0,
                                         MOVIE_FIXTURE_PORT1,
                                         0};
        char movie_path[4096];
        char frame_argument[4096];
        const char *const command[] = {runner,     "--rom",        rom_path,       "--movie",
                                       movie_path, "--dump-frame", frame_argument, NULL};
        FILE *rom = fopen(rom_path, "wb");
        FILE *movie = NULL;
        CHECK(rom != NULL);
        if (rom != NULL) {
            CHECK(fwrite(image, 1, image_size, rom) == image_size);
            CHECK(fclose(rom) == 0);
        }
        CHECK(snprintf(movie_path, sizeof movie_path, "%s.movie", rom_path) > 0);
        CHECK(snprintf(frame_argument, sizeof frame_argument, "1:%s", ppm_path) > 0);
        movie = fopen(movie_path, "wb");
        CHECK(movie != NULL);
        if (movie != NULL) {
            CHECK(fwrite(movie_bytes, 1, sizeof movie_bytes, movie) == sizeof movie_bytes);
            CHECK(fclose(movie) == 0);
        }
        CHECK_EQ_U64(test_process_run(command, NULL), 0u);
        compare_with_ppm(ppm_path);
        remove(movie_path);
    }

    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &nes), NESTURBATOR_OK);
    if (nes != NULL) {
        CHECK_EQ_U64(nesturbator_load_cartridge(nes, image, image_size), NESTURBATOR_OK);
        memset(&scripted, 0, sizeof scripted);
        scripted.size = (uint32_t)sizeof scripted;
        scripted.buttons[0] = host_buttons[0];
        scripted.buttons[1] = host_buttons[1];
        CHECK_EQ_U64(nesturbator_set_input(nes, &scripted), NESTURBATOR_OK);
        memset(&io, 0, sizeof io);
        io.size = (uint32_t)sizeof io;
        io.video = native;
        io.video_pitch = W;
        io.audio = audio;
        io.audio_capacity = 1024u;
        CHECK_EQ_U64(nesturbator_run_frame(nes, &io), NESTURBATOR_OK);
        nesturbator_get_palette(nes, palette, 512u);
        for (uint32_t pixel = 0; pixel < W * H; pixel++) {
            if (frame[pixel] != palette[native[pixel] & 0x1ffu]) {
                mismatches++;
            }
        }
        CHECK_EQ_U64(mismatches, 0u);
        CHECK_EQ_HEX(frame[0], 0xBA3100u);
        nesturbator_destroy(nes);
    }
    p_unload_game();
    memset(host_buttons, 0, sizeof host_buttons);
}

static void check_ines_builder(void)
{
    static uint8_t buf[16u + INES_TRAINER_SIZE + 2u * INES_PRG_BANK + INES_CHR_BANK];
    struct ines_spec spec;
    size_t size;

    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 2u;
    spec.chr_8k = 1u;
    spec.mapper = 0x102u;
    spec.submapper = 1u;
    spec.nes2 = 1u;
    spec.mirroring_vertical = 1u;
    size = ines_build(buf, sizeof buf, &spec);
    CHECK_EQ_U64(size, 16u + 32768u + 8192u);
    CHECK_EQ_U64(size, ines_size(&spec));
    CHECK_EQ_HEX(buf[6], 0x21u);
    CHECK_EQ_HEX(buf[7], 0x08u);
    CHECK_EQ_HEX(buf[8], 0x11u);
    CHECK_EQ_U64(ines_build(buf, size - 1u, &spec), 0u);
    spec.prg_16k = 0u;
    CHECK_EQ_U64(ines_build(buf, sizeof buf, &spec), 0u);
}

static void check_trainer_frame_parity(const char *runner, const char *rom_path,
                                       const char *ppm_path)
{
    /* The trainer is copied to $7000 by the loader; only its code sets the backdrop to $16 and
       turns the background on. PRG just jumps there. */
    static const uint8_t trainer_code[23] = {0xa9, 0x3f, 0x8d, 0x06, 0x20, 0xa9, 0x00, 0x8d,
                                             0x06, 0x20, 0xa9, 0x16, 0x8d, 0x07, 0x20, 0xa9,
                                             0x0a, 0x8d, 0x01, 0x20, 0x4c, 0x14, 0x70};
    static const uint8_t prg_code[3] = {0x4c, 0x00, 0x70};
    static uint8_t trainer[INES_TRAINER_SIZE];
    static uint8_t image[16u + INES_TRAINER_SIZE + INES_PRG_BANK + INES_CHR_BANK];
    struct ines_spec spec;
    struct retro_game_info game;
    size_t image_size;
    FILE *rom;
    char frame_argument[4096];
    const char *const command[] = {runner, "--rom",        rom_path,       "--frames",
                                   "1",    "--dump-frame", frame_argument, NULL};

    memcpy(trainer, trainer_code, sizeof trainer_code);
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = 1u;
    spec.trainer = trainer;
    spec.prg_code = prg_code;
    spec.prg_code_len = sizeof prg_code;
    spec.nmi_vector = 0x8000u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    image_size = ines_build(image, sizeof image, &spec);
    CHECK(image_size != 0u);
    if (image_size == 0u) {
        return;
    }

    p_unload_game();
    memset(&game, 0, sizeof game);
    game.data = image;
    game.size = image_size;
    CHECK(p_load_game(&game));
    p_run();
    CHECK_EQ_HEX(frame[0], 0xBA3100u);

    rom = fopen(rom_path, "wb");
    CHECK(rom != NULL);
    if (rom != NULL) {
        CHECK(fwrite(image, 1, image_size, rom) == image_size);
        CHECK(fclose(rom) == 0);
    }
    CHECK(snprintf(frame_argument, sizeof frame_argument, "1:%s", ppm_path) > 0);
    CHECK_EQ_U64(test_process_run(command, NULL), 0u);
    compare_with_ppm(ppm_path);

    /* Negative control: without the trainer the same PRG jumps into empty RAM, so the colour
       came from the trainer and not from the PRG. */
    p_unload_game();
    spec.trainer = NULL;
    image_size = ines_build(image, sizeof image, &spec);
    CHECK(image_size != 0u);
    game.data = image;
    game.size = image_size;
    CHECK(p_load_game(&game));
    p_run();
    CHECK(frame[0] != 0xBA3100u);
    p_unload_game();
}

/* Reset parity (TUNE-06, D-18): the module's frames after retro_reset equal a direct-API
   instance given the same frames, reset and frames. The reset handler counts resets in RAM $10
   (RAM survives a soft reset) and uses the count to pick the backdrop colour, so the post-reset
   frames differ from the pre-reset ones only if the reset ran the program again. The handler
   follows the NESdev Wiki "PPU power up state" init: an unconditional BIT $2002 clears a vblank
   flag kept from before the reset, then two vblank waits; the second vblank falls about 57,165
   CPU cycles in, after the write-ignore window (29,667 cycles). Three frames after the reset
   (about 89,000 cycles) are enough for the palette write and the $2001 write to land. */
#define RESET_FRAMES 3u
static uint32_t reset_before[W * H];
static uint32_t reset_after[RESET_FRAMES][W * H];

static void check_reset_frame_parity(void)
{
    static const uint8_t handler[] = {
        0x78,             /* 8000 SEI */
        0xe6, 0x10,       /* 8001 INC $10: count resets */
        0x2c, 0x02, 0x20, /* 8003 BIT $2002: clear a kept vblank flag */
        0x2c, 0x02, 0x20, /* 8006 wait1: BIT $2002 */
        0x10, 0xfb,       /* 8009 BPL wait1 */
        0x2c, 0x02, 0x20, /* 800B wait2: BIT $2002 */
        0x10, 0xfb,       /* 800E BPL wait2 */
        0xa9, 0x3f,       /* 8010 LDA #$3F */
        0x8d, 0x06, 0x20, /* 8012 STA $2006 */
        0xa9, 0x00,       /* 8015 LDA #$00 */
        0x8d, 0x06, 0x20, /* 8017 STA $2006 */
        0xa5, 0x10,       /* 801A LDA $10 */
        0x29, 0x03,       /* 801C AND #3 */
        0x18,             /* 801E CLC */
        0x69, 0x11,       /* 801F ADC #$11: backdrop = count + $11 */
        0x8d, 0x07, 0x20, /* 8021 STA $2007 */
        0xa9, 0x0a,       /* 8024 LDA #$0A */
        0x8d, 0x01, 0x20, /* 8026 STA $2001: background on */
        0x4c, 0x29, 0x80  /* 8029 JMP $8029 */
    };
    static uint8_t trainer[INES_TRAINER_SIZE];
    static uint8_t image[16u + INES_TRAINER_SIZE + INES_PRG_BANK + INES_CHR_BANK];
    static uint16_t native[W * H];
    static int16_t audio[1024];
    static uint32_t palette[512];
    struct ines_spec spec;
    struct retro_game_info game;
    nesturbator_config cfg;
    nesturbator_frame io;
    nesturbator *nes = NULL;
    size_t image_size;
    uint8_t counter_before = 0u;
    uint8_t counter_after = 0u;
    uint32_t mismatches = 0u;

    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = 1u;
    spec.trainer = trainer;
    spec.prg_code = handler;
    spec.prg_code_len = sizeof handler;
    spec.nmi_vector = 0x8000u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    image_size = ines_build(image, sizeof image, &spec);
    CHECK(image_size != 0u);
    if (image_size == 0u) {
        return;
    }

    p_unload_game();
    memset(&game, 0, sizeof game);
    game.data = image;
    game.size = image_size;
    CHECK(p_load_game(&game));
    for (unsigned i = 0; i < 3u; i++) {
        p_run();
    }
    memcpy(reset_before, frame, sizeof frame);
    p_reset();
    for (unsigned i = 0; i < RESET_FRAMES; i++) {
        p_run();
        memcpy(reset_after[i], frame, sizeof frame);
    }
    p_unload_game();

    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &nes), NESTURBATOR_OK);
    if (nes == NULL) {
        return;
    }
    CHECK_EQ_U64(nesturbator_load_cartridge(nes, image, image_size), NESTURBATOR_OK);
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = native;
    io.video_pitch = W;
    io.audio = audio;
    io.audio_capacity = 1024u;
    nesturbator_get_palette(nes, palette, 512u);
    for (unsigned i = 0; i < 3u; i++) {
        CHECK_EQ_U64(nesturbator_run_frame(nes, &io), NESTURBATOR_OK);
    }
    CHECK_EQ_U64(nesturbator_peek_cpu_ram(nes, 0x0010u, &counter_before), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_reset(nes), NESTURBATOR_OK);
    for (unsigned i = 0; i < RESET_FRAMES; i++) {
        CHECK_EQ_U64(nesturbator_run_frame(nes, &io), NESTURBATOR_OK);
        for (uint32_t pixel = 0; pixel < W * H; pixel++) {
            if (reset_after[i][pixel] != palette[native[pixel] & 0x1ffu]) {
                mismatches++;
            }
        }
    }
    CHECK_EQ_U64(mismatches, 0u);
    CHECK_EQ_U64(nesturbator_peek_cpu_ram(nes, 0x0010u, &counter_after), NESTURBATOR_OK);
    CHECK_EQ_U64(counter_after, (uint64_t)counter_before + 1u);
    CHECK(reset_after[RESET_FRAMES - 1u][0] != reset_before[0]);
    nesturbator_destroy(nes);
}

/* SAVE-05 (D-21): RETRO_MEMORY_SAVE_RAM is the battery span. The adapter owns its instance and
   the module exports only retro_*, so the span is compared by its bytes with a direct instance
   run for the same frames (equal only if the adapter returns the instance's real span), and by
   its size, a stable address and NULL/0 in every other case. LDA #$A5; STA $6000; LDA #$5A;
   STA $7FFF; JMP * (at $800A). */
static size_t build_save_image(uint8_t *out, size_t cap, uint8_t battery, uint8_t ram_shift,
                               uint8_t nvram_shift)
{
    static const uint8_t code[] = {0xa9u, 0xa5u, 0x8du, 0x00u, 0x60u, 0xa9u, 0x5au,
                                   0x8du, 0xffu, 0x7fu, 0x4cu, 0x0au, 0x80u};
    struct ines_segment seg;
    struct ines_spec spec;
    seg.prg_offset = 0u;
    seg.bytes = code;
    seg.len = sizeof code;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 2u;
    spec.nes2 = 1u;
    spec.mapper = 1u;
    spec.battery = battery;
    spec.prg_ram_shift = ram_shift;
    spec.prg_nvram_shift = nvram_shift;
    spec.segments = &seg;
    spec.segment_count = 1u;
    spec.nmi_vector = 0x8000u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    return ines_build(out, cap, &spec);
}

static void check_save_memory(void)
{
    static uint8_t image[16u + 2u * INES_PRG_BANK];
    static uint16_t native[W * H];
    static int16_t audio[1024];
    struct retro_game_info game;
    nesturbator_config cfg;
    nesturbator_frame io;
    nesturbator *nes = NULL;
    uint8_t *direct = NULL;
    size_t direct_size = 0u;
    uint8_t *data;
    size_t size;

    p_unload_game();
    CHECK(p_get_memory_data(RETRO_MEMORY_SAVE_RAM) == NULL);
    CHECK_EQ_U64(p_get_memory_size(RETRO_MEMORY_SAVE_RAM), 0u);

    size = build_save_image(image, sizeof image, 1u, 0u, 7u);
    CHECK(size != 0u);
    if (size == 0u) {
        return;
    }
    memset(&game, 0, sizeof game);
    game.data = image;
    game.size = size;
    CHECK(p_load_game(&game));
    data = (uint8_t *)p_get_memory_data(RETRO_MEMORY_SAVE_RAM);
    CHECK(data != NULL);
    CHECK_EQ_U64(p_get_memory_size(RETRO_MEMORY_SAVE_RAM), 8192u);
    CHECK(p_get_memory_data(RETRO_MEMORY_SYSTEM_RAM) == NULL);
    CHECK_EQ_U64(p_get_memory_size(RETRO_MEMORY_SYSTEM_RAM), 0u);
    for (unsigned i = 0; i < 3u; i++) {
        p_run();
    }
    CHECK(p_get_memory_data(RETRO_MEMORY_SAVE_RAM) == data);

    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &nes), NESTURBATOR_OK);
    if (nes != NULL) {
        CHECK_EQ_U64(nesturbator_load_cartridge(nes, image, size), NESTURBATOR_OK);
        memset(&io, 0, sizeof io);
        io.size = (uint32_t)sizeof io;
        io.video = native;
        io.video_pitch = W;
        io.audio = audio;
        io.audio_capacity = 1024u;
        for (unsigned i = 0; i < 3u; i++) {
            CHECK_EQ_U64(nesturbator_run_frame(nes, &io), NESTURBATOR_OK);
        }
        CHECK_EQ_U64(
            nesturbator_get_memory(nes, NESTURBATOR_MEMORY_SAVE_RAM, &direct, &direct_size),
            NESTURBATOR_OK);
        CHECK_EQ_U64(direct_size, p_get_memory_size(RETRO_MEMORY_SAVE_RAM));
        CHECK(direct != NULL && direct_size == 8192u);
        if (direct != NULL && direct_size == 8192u) {
            CHECK_EQ_HEX(data[0], 0xa5u);
            CHECK(memcmp(direct, data, direct_size) == 0);
        }
        nesturbator_destroy(nes);
    }
    p_reset();
    CHECK(p_get_memory_data(RETRO_MEMORY_SAVE_RAM) == data);
    CHECK_EQ_U64(p_get_memory_size(RETRO_MEMORY_SAVE_RAM), 8192u);
    p_unload_game();
    CHECK(p_get_memory_data(RETRO_MEMORY_SAVE_RAM) == NULL);
    CHECK_EQ_U64(p_get_memory_size(RETRO_MEMORY_SAVE_RAM), 0u);

    /* Work RAM without a battery is not a save. */
    size = build_save_image(image, sizeof image, 0u, 7u, 0u);
    CHECK(size != 0u);
    game.size = size;
    CHECK(p_load_game(&game));
    CHECK(p_get_memory_data(RETRO_MEMORY_SAVE_RAM) == NULL);
    CHECK_EQ_U64(p_get_memory_size(RETRO_MEMORY_SAVE_RAM), 0u);
    p_unload_game();
}

/* With no game loaded, retro_reset leaves the next frame equal to the one before it. */
static void check_test_card_reset(void)
{
    static uint32_t before[W * H];

    p_unload_game();
    CHECK(p_load_game(NULL));
    p_run();
    memcpy(before, frame, sizeof frame);
    p_reset();
    p_run();
    CHECK(memcmp(before, frame, sizeof frame) == 0);
    p_unload_game();
}

static void check_sound_frame_parity(unsigned char *image, size_t image_size)
{
    static const uint8_t program[] = {0xa9u, 0x01u, 0x8du, 0x15u, 0x40u, 0xa9u, 0x1fu, 0x8du,
                                      0x00u, 0x40u, 0xa9u, 0x64u, 0x8du, 0x02u, 0x40u, 0xa9u,
                                      0x00u, 0x8du, 0x03u, 0x40u, 0x4cu, 0x14u, 0x80u};
    static uint16_t native[W * H];
    int16_t mono[AUDIO_CAPTURE_CAPACITY];
    nesturbator_config cfg;
    nesturbator_frame io;
    nesturbator *nes = NULL;
    struct retro_game_info game;
    int audible = 0;
    bool loaded;

    p_unload_game();
    memset(image, 0, image_size);
    image[0] = 'N';
    image[1] = 'E';
    image[2] = 'S';
    image[3] = 0x1a;
    image[4] = 1u;
    image[5] = 1u;
    memcpy(image + 16u, program, sizeof program);
    image[16u + 0x3ffau] = 0x00u;
    image[16u + 0x3ffbu] = 0x80u;
    image[16u + 0x3ffcu] = 0x00u;
    image[16u + 0x3ffdu] = 0x80u;
    image[16u + 0x3ffeu] = 0x00u;
    image[16u + 0x3fffu] = 0x80u;
    memset(&game, 0, sizeof game);
    game.data = image;
    game.size = image_size;
    loaded = p_load_game(&game);
    CHECK(loaded);
    if (!loaded)
        return;

    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &nes), NESTURBATOR_OK);
    if (nes == NULL) {
        p_unload_game();
        return;
    }
    CHECK_EQ_U64(nesturbator_load_cartridge(nes, image, image_size), NESTURBATOR_OK);
    memset(host_buttons, 0, sizeof host_buttons);
    for (unsigned frame_index = 0u; frame_index < 2u; frame_index++) {
        batch_calls = 0;
        batch_frames = 0u;
        batch_nonzero = 0;
        sample_calls = 0;
        p_set_audio_sample(sample);
        p_set_audio_sample_batch(NULL);
        p_run();
        CHECK_EQ_U64(batch_calls, 0u);

        memset(&io, 0, sizeof io);
        io.size = (uint32_t)sizeof io;
        io.video = native;
        io.video_pitch = W;
        io.audio = mono;
        io.audio_capacity = AUDIO_CAPTURE_CAPACITY;
        CHECK_EQ_U64(nesturbator_run_frame(nes, &io), NESTURBATOR_OK);
        CHECK_EQ_U64(sample_calls, io.audio_count);
        if (sample_calls == (int)io.audio_count && io.audio_count <= AUDIO_CAPTURE_CAPACITY) {
            for (size_t i = 0u; i < io.audio_count; i++) {
                CHECK_EQ_U64((uint16_t)sample_samples[2u * i], (uint16_t)mono[i]);
                CHECK_EQ_U64((uint16_t)sample_samples[2u * i + 1u], (uint16_t)mono[i]);
                audible |= mono[i] != 0;
            }
        }
    }
    CHECK(audible != 0);
    p_set_audio_sample(sample);
    p_set_audio_sample_batch(batch);
    for (unsigned frame_index = 0u; frame_index < 2u; frame_index++) {
        batch_calls = 0;
        batch_frames = 0u;
        batch_nonzero = 0;
        sample_calls = 0;
        p_run();
        CHECK_EQ_U64(batch_calls, 1u);
        CHECK_EQ_U64(sample_calls, 0u);

        memset(&io, 0, sizeof io);
        io.size = (uint32_t)sizeof io;
        io.video = native;
        io.video_pitch = W;
        io.audio = mono;
        io.audio_capacity = AUDIO_CAPTURE_CAPACITY;
        CHECK_EQ_U64(nesturbator_run_frame(nes, &io), NESTURBATOR_OK);
        CHECK_EQ_U64(batch_frames, io.audio_count);
        if (batch_frames == io.audio_count && io.audio_count <= AUDIO_CAPTURE_CAPACITY) {
            for (size_t i = 0u; i < io.audio_count; i++) {
                CHECK_EQ_U64((uint16_t)batch_samples[2u * i], (uint16_t)mono[i]);
                CHECK_EQ_U64((uint16_t)batch_samples[2u * i + 1u], (uint16_t)mono[i]);
            }
        }
        CHECK(batch_nonzero > 0);
    }

    /* Failed frames must not deliver stale or partial audio to either callback. */
    p_unload_game();
    image[16u] = 0x02u; /* JAM */
    CHECK_EQ_U64(nesturbator_load_cartridge(nes, image, image_size), NESTURBATOR_OK);
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = native;
    io.video_pitch = W;
    io.audio = mono;
    io.audio_capacity = AUDIO_CAPTURE_CAPACITY;
    CHECK_EQ_U64(nesturbator_run_frame(nes, &io), NESTURBATOR_STOP_JAM);

    game.data = image;
    game.size = image_size;
    loaded = p_load_game(&game);
    CHECK(loaded);
    if (loaded) {
        for (unsigned callback_mode = 0u; callback_mode < 2u; callback_mode++) {
            sample_calls = 0;
            batch_calls = 0;
            batch_frames = 0u;
            if (callback_mode == 0u) {
                p_set_audio_sample_batch(NULL);
            } else {
                p_set_audio_sample_batch(batch);
            }
            p_run();
            CHECK_EQ_U64(sample_calls, 0u);
            CHECK_EQ_U64(batch_calls, 0u);
            CHECK_EQ_U64(batch_frames, 0u);
        }
    }

    p_set_audio_sample(NULL);
    p_set_audio_sample_batch(batch);
    nesturbator_destroy(nes);
    p_unload_game();
}

/* ---- the runner's image ---- */

/* Compares the captured frame with the runner's P6 file, pixel by pixel. */
static void compare_with_ppm(const char *path)
{
    static const char header[] = "P6\n256 240\n255\n";
    static unsigned char rgb[W * H * 3u];
    unsigned char head[sizeof header - 1u];
    uint32_t i, mismatches = 0;
    FILE *f = fopen(path, "rb");

    CHECK(f != NULL);
    if (f == NULL) {
        return;
    }
    CHECK(fread(head, 1, sizeof head, f) == sizeof head);
    CHECK(memcmp(head, header, sizeof head) == 0);
    CHECK(fread(rgb, 1, sizeof rgb, f) == sizeof rgb);
    fclose(f);
    for (i = 0; i < W * H; i++) {
        uint32_t want = ((uint32_t)rgb[3u * i] << 16) | ((uint32_t)rgb[3u * i + 1u] << 8) |
                        (uint32_t)rgb[3u * i + 2u];
        if ((frame[i] & 0xFFFFFFu) != want) {
            if (mismatches == 0) {
                fprintf(stderr, "first mismatch at (%u,%u): 0x%06x != 0x%06x\n", (unsigned)(i % W),
                        (unsigned)(i / W), (unsigned)(frame[i] & 0xFFFFFFu), (unsigned)want);
            }
            mismatches++;
        }
    }
    CHECK_EQ_U64(mismatches, 0);
}

static void check_core_timing_and_jam(unsigned char *image, size_t image_size)
{
    uint16_t first[W * H], second[W * H];
    int16_t audio[1024];
    nesturbator_config cfg;
    nesturbator *a = NULL, *b = NULL, *jam = NULL;
    nesturbator_frame fa, fb;
    uint64_t previous_ticks = 0u;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK(nesturbator_create(&cfg, &a) == NESTURBATOR_OK);
    CHECK(nesturbator_create(&cfg, &b) == NESTURBATOR_OK);
    if (a == NULL || b == NULL)
        goto done;
    CHECK(nesturbator_load_cartridge(a, image, image_size) == NESTURBATOR_OK);
    CHECK(nesturbator_load_cartridge(b, image, image_size) == NESTURBATOR_OK);
    for (uint64_t frame_number = 1; frame_number <= 6u; frame_number++) {
        memset(&fa, 0, sizeof fa);
        memset(&fb, 0, sizeof fb);
        fa.size = (uint32_t)sizeof fa;
        fa.video = first;
        fa.video_pitch = W;
        fa.audio = audio;
        fa.audio_capacity = 1024u;
        fb = fa;
        fb.video = second;
        CHECK(nesturbator_run_frame(a, &fa) == NESTURBATOR_OK);
        CHECK(nesturbator_run_frame(b, &fb) == NESTURBATOR_OK);
        CHECK(fa.ticks >= previous_ticks + 714732u);
        CHECK(fa.ticks <= previous_ticks + 714900u);
        CHECK_EQ_U64(fa.ticks % 24u, 0u);
        previous_ticks = fa.ticks;
        CHECK(memcmp(first, second, sizeof first) == 0);
    }
    image[16] = 0x02u; /* JAM */
    CHECK(nesturbator_create(&cfg, &jam) == NESTURBATOR_OK);
    if (jam != NULL) {
        CHECK(nesturbator_load_cartridge(jam, image, image_size) == NESTURBATOR_OK);
        memset(&fa, 0, sizeof fa);
        fa.size = (uint32_t)sizeof fa;
        fa.video = first;
        fa.video_pitch = W;
        fa.audio = audio;
        fa.audio_capacity = 1024u;
        fa.frame_number = 99u;
        fa.ticks = 99u;
        CHECK(nesturbator_run_frame(jam, &fa) == NESTURBATOR_STOP_JAM);
        CHECK_EQ_U64(fa.frame_number, 99u);
        CHECK_EQ_U64(fa.ticks, 99u);
        fa.frame_number = 99u;
        fa.ticks = 99u;
        CHECK(nesturbator_run_frame(jam, &fa) == NESTURBATOR_STOP_JAM);
        CHECK_EQ_U64(fa.frame_number, 99u);
        CHECK_EQ_U64(fa.ticks, 99u);
        fa.frame_number = 100u;
        fa.ticks = 100u;
        CHECK(nesturbator_run_frame(jam, &fa) == NESTURBATOR_STOP_JAM);
        CHECK_EQ_U64(fa.frame_number, 100u);
        CHECK_EQ_U64(fa.ticks, 100u);
    }
done:
    nesturbator_destroy(jam);
    nesturbator_destroy(b);
    nesturbator_destroy(a);
    image[16] = 0xa9u;
}

static void check_ppu_frame_edges(unsigned char *image, size_t image_size)
{
    nesturbator_config cfg;
    nesturbator *probe = NULL;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK(nesturbator_create(&cfg, &probe) == NESTURBATOR_OK);
    if (probe == NULL)
        return;
    image[16u + 0x3ffau] = 0x50u;
    image[16u + 0x3ffbu] = 0x80u;
    CHECK(nesturbator_load_cartridge(probe, image, image_size) == NESTURBATOR_OK);

    probe->ppu.scanline = 241u;
    probe->ppu.dot = 0u;
    probe->ppu.control = 0x80u;
    nesturbator__ppu_run_until(probe, 8u);
    CHECK((probe->ppu.status & 0x80u) != 0u);
    CHECK_EQ_U64(probe->cpu.nmi_pending, 1u);
    (void)nesturbator__bus_read(probe, 0x2002u);
    CHECK((probe->ppu.status & 0x80u) == 0u);
    CHECK_EQ_U64(probe->cpu.nmi_pending, 0u);

    probe->ppu.ppu_ticks = 0u;
    probe->ppu.scanline = 241u;
    probe->ppu.dot = 0u;
    probe->ppu.status = 0u;
    nesturbator__ppu_run_until(probe, 8u);
    CHECK_EQ_U64(probe->cpu.nmi_pending, 1u);
    nesturbator__cpu_step(probe);
    CHECK_EQ_U64(probe->cpu.pc, 0x8050u);
    CHECK_EQ_U64(probe->cpu.nmi_pending, 0u);

    probe->ppu.ppu_ticks = 0u;
    probe->ppu.scanline = 261u;
    probe->ppu.dot = 339u;
    probe->ppu.mask = 0x08u;
    probe->ppu.odd_frame = 1u;
    nesturbator__ppu_run_until(probe, 8u);
    CHECK_EQ_U64(probe->ppu.scanline, 0u);
    CHECK_EQ_U64(probe->ppu.dot, 0u);
    CHECK_EQ_U64(probe->ppu.odd_frame, 0u);

    probe->ppu.ppu_ticks = 0u;
    probe->ppu.scanline = 261u;
    probe->ppu.dot = 339u;
    probe->ppu.odd_frame = 0u;
    nesturbator__ppu_run_until(probe, 8u);
    CHECK_EQ_U64(probe->ppu.scanline, 261u);
    CHECK_EQ_U64(probe->ppu.dot, 340u);
    nesturbator_destroy(probe);
}

int main(int argc, char **argv)
{
    struct retro_system_info sys;
    struct retro_system_av_info av;
    struct retro_game_info dummy;
    static unsigned char dummy_bytes[16u + 16384u + 8192u];
    static uint32_t first_content_frame[W * H];
    double fps_diff;

    if (argc != 6) {
        fprintf(stderr, "usage: libretro.host <module> <testframe.ppm> <content.ppm> <runner> "
                        "<generated.nes>\n");
        return 2;
    }
    if (!module_open(argv[1])) {
        fprintf(stderr, "cannot load %s\n", argv[1]);
        return 1;
    }
    resolve_all();
    CHECK_EQ_U64(missing, 0);
    if (missing != 0) {
        module_close();
        CHECK_DONE();
    }

    CHECK_EQ_U64(p_api_version(), RETRO_API_VERSION);
    CHECK_EQ_U64(RETRO_API_VERSION, 1);

    p_set_environment(env);
    CHECK_EQ_U64(no_game_calls, 1);
    CHECK_EQ_U64(no_game_value, 1);

    p_init();

    memset(&sys, 0, sizeof sys);
    p_get_system_info(&sys);
    CHECK(sys.library_name != NULL && strcmp(sys.library_name, "nesturbator") == 0);
    CHECK(sys.valid_extensions != NULL && strcmp(sys.valid_extensions, "nes") == 0);
    CHECK(!sys.need_fullpath);

    p_set_video_refresh(video);
    p_set_audio_sample(sample);
    p_set_audio_sample_batch(batch);
    p_set_input_poll(on_poll);
    p_set_input_state(input);

    /* Task 1: an owned synthetic mapper-0 image must load through libretro. */
    memset(dummy_bytes, 0, sizeof dummy_bytes);
    dummy_bytes[0] = 'N';
    dummy_bytes[1] = 'E';
    dummy_bytes[2] = 'S';
    dummy_bytes[3] = 0x1a;
    dummy_bytes[4] = 1;
    dummy_bytes[5] = 1;
    {
        static const unsigned char program[] = {
            0xa9, 0x3f, 0x8d, 0x06, 0x20, 0xa9, 0x00, 0x8d, 0x06, 0x20, 0xa9, 0x0f, 0x8d, 0x07,
            0x20, 0xa9, 0x16, 0x8d, 0x07, 0x20, 0xa9, 0x27, 0x8d, 0x07, 0x20, 0xa9, 0x30, 0x8d,
            0x07, 0x20, 0xa9, 0x20, 0x8d, 0x06, 0x20, 0xa9, 0x00, 0x8d, 0x06, 0x20, 0xa9, 0x01,
            0x8d, 0x07, 0x20, 0xa9, 0x0a, 0x8d, 0x01, 0x20, 0x4c, 0x32, 0x80};
        memcpy(dummy_bytes + 16u, program, sizeof program);
        memset(dummy_bytes + 16u + 16384u + 16u, 0xff, 8u);
    }
    dummy_bytes[16u + 0x3ffau] = 0x00;
    dummy_bytes[16u + 0x3ffbu] = 0x80;
    dummy_bytes[16u + 0x3ffcu] = 0x00;
    dummy_bytes[16u + 0x3ffdu] = 0x80;
    dummy_bytes[16u + 0x3ffeu] = 0x00;
    dummy_bytes[16u + 0x3fffu] = 0x80;
    check_core_timing_and_jam(dummy_bytes, sizeof dummy_bytes);
    check_ppu_frame_edges(dummy_bytes, sizeof dummy_bytes);
    memset(&dummy, 0, sizeof dummy);
    dummy.data = dummy_bytes;
    dummy.size = sizeof dummy_bytes;
    CHECK(p_load_game(&dummy));
    {
        FILE *rom = fopen(argv[5], "wb");
        CHECK(rom != NULL);
        if (rom != NULL) {
            CHECK(fwrite(dummy_bytes, 1, sizeof dummy_bytes, rom) == sizeof dummy_bytes);
            fclose(rom);
        }
        {
            char frame_argument[4096];
            const char *const command[] = {argv[4],        "--frames",     "1", "--rom", argv[5],
                                           "--dump-frame", frame_argument, NULL};
            CHECK(snprintf(frame_argument, sizeof frame_argument, "1:%s", argv[3]) > 0);
            CHECK_EQ_U64(test_process_run(command, NULL), 0);
        }
    }
    p_run();
    CHECK_EQ_U64(video_calls, 1);
    compare_with_ppm(argv[3]);
    CHECK(frame[0] != frame[W + 8u]);
    /* The generated cartridge writes its nametable during the first frame;
       compare steady-state frames after that startup write has completed. */
    p_run();
    CHECK_EQ_U64(video_calls, 2);
    memcpy(first_content_frame, frame, sizeof frame);
    p_run();
    CHECK_EQ_U64(video_calls, 3);
    CHECK(memcmp(frame, first_content_frame, sizeof frame) == 0);
    p_unload_game();
    check_input_frame_parity(dummy_bytes, sizeof dummy_bytes, argv[4], argv[5], argv[3]);
    check_ines_builder();
    check_trainer_frame_parity(argv[4], argv[5], argv[3]);
    check_reset_frame_parity();
    check_test_card_reset();
    check_save_memory();
    check_sound_frame_parity(dummy_bytes, sizeof dummy_bytes);
    video_calls = 0;
    sample_calls = 0;
    batch_calls = 0;
    batch_frames = 0;
    batch_nonzero = 0;

    /* No content remains supported: the test card, in XRGB8888. */
    pixel_format_calls = 0;
    CHECK(p_load_game(NULL));
    CHECK_EQ_U64(pixel_format_calls, 1);
    CHECK_EQ_U64(pixel_format, RETRO_PIXEL_FORMAT_XRGB8888);

    memset(&av, 0, sizeof av);
    p_get_system_av_info(&av);
    CHECK_EQ_U64(av.geometry.base_width, 256);
    CHECK_EQ_U64(av.geometry.base_height, 240);
    CHECK_EQ_U64(av.geometry.max_width, 256);
    CHECK_EQ_U64(av.geometry.max_height, 240);
    fps_diff = av.timing.fps - 39375000.0 / 655171.0;
    CHECK(fps_diff < 1e-9 && fps_diff > -1e-9);
    CHECK(av.timing.sample_rate == 48000.0);
    CHECK_EQ_U64(p_get_region(), RETRO_REGION_NTSC);

    CHECK(p_get_memory_data(RETRO_MEMORY_SAVE_RAM) == NULL);
    CHECK_EQ_U64(p_get_memory_size(RETRO_MEMORY_SAVE_RAM), 0u);
    p_run();
    CHECK_EQ_U64(video_calls, 1);
    CHECK_EQ_U64(video_width, 256);
    CHECK_EQ_U64(video_height, 240);
    CHECK(video_pitch >= 1024u);
    CHECK(poll_calls >= 1);
    CHECK_EQ_U64(sample_calls, 0);
    CHECK_EQ_U64(batch_calls, 1);
    CHECK_EQ_U64(batch_frames, 798);
    CHECK_EQ_U64(batch_nonzero, 0);

    /* D-05: four pixels from src/palette_ntsc.c, written out here. */
    CHECK_EQ_HEX(frame[0 * W + 0] & 0xFFFFFFu, 0xFFFFFF);    /* native $30 */
    CHECK_EQ_HEX(frame[18 * W + 100] & 0xFFFFFFu, 0xBA3100); /* native $16 */
    CHECK_EQ_HEX(frame[46 * W + 100] & 0xFFFFFFu, 0xBC2700); /* native $56 */
    CHECK_EQ_HEX(frame[10 * W + 213] & 0xFFFFFFu, 0x000000); /* native $0D */

    /* FRAME-04: the same frame the runner writes. */
    compare_with_ppm(argv[2]);

    p_unload_game();
    p_deinit();
    module_close();
    CHECK_DONE();
}
