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
#include "../runahead_model.h"
#include "../state_test.h"
#include "../../src/internal.h"
#include "libretro.h"
#include "nesturbator.h"
#include "movie_fixture.h"
#include "../test_process.h"
#include "../../runner/movie.h"

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
static bool video_data_nonnull;
static const void *video_source;
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

static int env_calls;
static int savestate_context_queries, av_enable_queries;
static int host_savestate_context = RETRO_SAVESTATE_CONTEXT_NORMAL;
static int host_av_enable = 3; /* bit 0 video, bit 1 audio (L1767) */

static bool RETRO_CALLCONV env(unsigned cmd, void *data)
{
    env_calls++;
    if (cmd == RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT) {
        savestate_context_queries++;
        if (data != NULL) {
            *(int *)data = host_savestate_context;
        }
        return true;
    }
    if (cmd == RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE) {
        av_enable_queries++;
        if (data != NULL) {
            *(int *)data = host_av_enable;
        }
        return true;
    }
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
    video_data_nonnull = data != NULL;
    video_source = data;
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

/* D-07, D-09: the adapter keeps one instance, so load, unload and reload run on the same
   instance without retro_deinit. Each pass collects a hash of every video frame and of its
   batch audio, and the audio is also compared with a direct-API instance. */
#define LIFE_FRAMES 8u

static uint64_t fnv_add(uint64_t h, const void *data, size_t len)
{
    const unsigned char *p = (const unsigned char *)data;
    for (size_t i = 0; i < len; i++) {
        h = (h ^ p[i]) * 1099511628211ull;
    }
    return h;
}

static void life_pass(const struct retro_game_info *game, uint64_t *video_hash,
                      uint64_t *audio_hash)
{
    CHECK(p_load_game(game));
    for (unsigned i = 0; i < LIFE_FRAMES; i++) {
        p_run();
        video_hash[i] = fnv_add(14695981039346656037ull, frame, sizeof frame);
        audio_hash[i] = fnv_add(14695981039346656037ull, batch_samples,
                                batch_frames * 2u * sizeof batch_samples[0]);
    }
}

static void check_lifecycle_reuse(void)
{
    static uint8_t image[16u + 2u * INES_PRG_BANK];
    static uint16_t native[W * H];
    static int16_t audio[1024];
    uint64_t v1[LIFE_FRAMES], a1[LIFE_FRAMES], v2[LIFE_FRAMES], a2[LIFE_FRAMES];
    struct retro_game_info game;
    nesturbator_config cfg;
    nesturbator_frame io;
    nesturbator *nes = NULL;
    size_t size = build_save_image(image, sizeof image, 1u, 0u, 7u);

    CHECK(size != 0u);
    if (size == 0u) {
        return;
    }
    p_unload_game();
    memset(&game, 0, sizeof game);
    game.data = image;
    game.size = size;
    life_pass(&game, v1, a1);
    CHECK(p_get_memory_data(RETRO_MEMORY_SAVE_RAM) != NULL);
    p_unload_game();
    CHECK(p_get_memory_data(RETRO_MEMORY_SAVE_RAM) == NULL);
    CHECK_EQ_U64(p_get_memory_size(RETRO_MEMORY_SAVE_RAM), 0u);
    life_pass(&game, v2, a2);
    CHECK(memcmp(v1, v2, sizeof v1) == 0);
    CHECK(memcmp(a1, a2, sizeof a1) == 0);
    p_unload_game();

    /* A direct-API instance gives the same audio on every frame of the second pass. */
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
        for (unsigned i = 0; i < LIFE_FRAMES; i++) {
            uint64_t h = 14695981039346656037ull;
            CHECK_EQ_U64(nesturbator_run_frame(nes, &io), NESTURBATOR_OK);
            for (size_t k = 0; k < io.audio_count && k < 1024u; k++) {
                h = fnv_add(h, &audio[k], sizeof audio[k]);
                h = fnv_add(h, &audio[k], sizeof audio[k]);
            }
            CHECK(h == a2[i]);
        }
        nesturbator_destroy(nes);
    }
}

/* D-09: load, unload, then no content gives the test card of a fresh start. */
static int frame_equals_ppm(const char *path);
static void check_unload_then_no_content(const char *testframe_path)
{
    static uint8_t image[16u + 2u * INES_PRG_BANK];
    struct retro_game_info game;
    size_t size = build_save_image(image, sizeof image, 1u, 0u, 7u);

    CHECK(size != 0u);
    if (size == 0u) {
        return;
    }
    memset(&game, 0, sizeof game);
    game.data = image;
    game.size = size;
    CHECK(p_load_game(&game));
    p_run();
    p_unload_game();
    CHECK(p_load_game(NULL));
    batch_frames = 0u;
    p_run();
    CHECK_EQ_U64(batch_frames, 798u);
    CHECK(frame_equals_ppm(testframe_path));
    CHECK(p_get_memory_data(RETRO_MEMORY_SAVE_RAM) == NULL);
    CHECK_EQ_U64(p_get_memory_size(RETRO_MEMORY_SAVE_RAM), 0u);
    p_unload_game();
}

/* D-09: double unload, a rejected image after an unload, and a battery image after a plain one. */
static void check_unload_edges(void)
{
    static uint8_t image[16u + 2u * INES_PRG_BANK];
    static const uint8_t bad[1] = {0x4eu};
    struct retro_game_info game;
    size_t size;

    p_unload_game();
    p_unload_game();
    memset(&game, 0, sizeof game);
    game.data = bad;
    game.size = sizeof bad;
    CHECK(!p_load_game(&game));
    CHECK(p_get_memory_data(RETRO_MEMORY_SAVE_RAM) == NULL);
    CHECK_EQ_U64(p_get_memory_size(RETRO_MEMORY_SAVE_RAM), 0u);

    size = build_save_image(image, sizeof image, 0u, 7u, 0u);
    game.data = image;
    game.size = size;
    CHECK(p_load_game(&game));
    CHECK(p_get_memory_data(RETRO_MEMORY_SAVE_RAM) == NULL);
    video_calls = 0;
    p_run();
    CHECK_EQ_U64(video_calls, 1);

    /* A rejected image after a good load drops the old span. */
    game.data = bad;
    game.size = sizeof bad;
    CHECK(!p_load_game(&game));
    CHECK(p_get_memory_data(RETRO_MEMORY_SAVE_RAM) == NULL);
    CHECK_EQ_U64(p_get_memory_size(RETRO_MEMORY_SAVE_RAM), 0u);

    size = build_save_image(image, sizeof image, 1u, 0u, 7u);
    game.data = image;
    game.size = size;
    CHECK(p_load_game(&game));
    CHECK(p_get_memory_data(RETRO_MEMORY_SAVE_RAM) != NULL);
    CHECK_EQ_U64(p_get_memory_size(RETRO_MEMORY_SAVE_RAM), 8192u);
    p_run();
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

static void check_jam_callback(const uint32_t *expected, bool use_batch)
{
    int old_video = video_calls;
    int old_poll = poll_calls;
    int old_input = input_calls;

    p_set_audio_sample(sample);
    p_set_audio_sample_batch(use_batch ? batch : NULL);
    sample_calls = 0;
    batch_calls = 0;
    batch_frames = 0u;
    p_run();
    CHECK_EQ_U64(video_calls, (uint64_t)old_video + 1u);
    CHECK_EQ_U64(poll_calls, (uint64_t)old_poll + 1u);
    CHECK_EQ_U64(input_calls, (uint64_t)old_input + 16u);
    CHECK(video_data_nonnull);
    CHECK_EQ_U64(video_width, W);
    CHECK_EQ_U64(video_height, H);
    CHECK_EQ_U64(video_pitch, W * sizeof frame[0]);
    CHECK(memcmp(frame, expected, sizeof frame) == 0);
    CHECK_EQ_U64(sample_calls, 0u);
    CHECK_EQ_U64(batch_calls, 0u);
    CHECK_EQ_U64(batch_frames, 0u);
}

static bool run_delayed_jam(uint32_t *last_complete)
{
    bool have_complete = false;

    p_set_audio_sample(sample);
    p_set_audio_sample_batch(batch);
    for (unsigned attempt = 0u; attempt < 20u; attempt++) {
        int old_video = video_calls;
        int old_poll = poll_calls;
        int old_input = input_calls;

        sample_calls = 0;
        batch_calls = 0;
        batch_frames = 0u;
        p_run();
        CHECK_EQ_U64(video_calls, (uint64_t)old_video + 1u);
        CHECK_EQ_U64(poll_calls, (uint64_t)old_poll + 1u);
        CHECK_EQ_U64(input_calls, (uint64_t)old_input + 16u);
        CHECK(video_data_nonnull);
        CHECK_EQ_U64(video_width, W);
        CHECK_EQ_U64(video_height, H);
        CHECK_EQ_U64(video_pitch, W * sizeof frame[0]);
        CHECK_EQ_U64(sample_calls, 0u);
        if (batch_calls == 0) {
            CHECK(have_complete);
            if (have_complete) {
                CHECK(memcmp(frame, last_complete, sizeof frame) == 0);
            }
            return have_complete;
        }
        CHECK_EQ_U64(batch_calls, 1u);
        CHECK(batch_frames == 798u || batch_frames == 799u);
        memcpy(last_complete, frame, sizeof frame);
        have_complete = true;
    }
    CHECK(0); /* The generated bounded delay must reach its JAM opcode. */
    return false;
}

static void check_jam_timeline(void);

/* D-10: the adapter repeats only a complete converted frame, and its cache
   lifetime is separate from the native save-state and machine state. */
static void check_jam_presentation(const unsigned char *base_image, size_t image_size)
{
    static const uint8_t delay_then_jam[] = {0xa2u, 0xffu, 0xa0u, 0xffu, 0x88u, 0xd0u,
                                             0xfdu, 0xcau, 0xd0u, 0xf8u, 0xa9u, 0x01u,
                                             0x8du, 0x00u, 0x00u, 0x02u};
    static const uint8_t clear_ram_loop[] = {0xa9u, 0x00u, 0x8du, 0x00u,
                                             0x00u, 0x4cu, 0x05u, 0x80u};
    static const uint8_t reset_gate[] = {0xadu, 0x00u, 0x00u, 0xd0u, 0x07u, 0xa9u, 0x01u,
                                         0x85u, 0x00u, 0x4cu, 0x00u, 0x80u, 0x02u};
    static const uint32_t black[W * H] = {0u};
    uint8_t direct_image[16u + 16384u + 8192u];
    uint8_t delayed_image[16u + 16384u + 8192u];
    uint8_t setup_image[16u + 16384u + 8192u];
    uint32_t last_complete[W * H];
    struct retro_game_info game;
    uint8_t *origin = NULL;
    uint8_t *jammed = NULL;
    uint8_t *roundtrip = NULL;
    size_t state_size;

    CHECK_EQ_U64(image_size, sizeof direct_image);
    if (image_size != sizeof direct_image) {
        return;
    }
    memcpy(direct_image, base_image, image_size);
    direct_image[16u] = 0x02u; /* Immediate JAM at the reset vector. */
    memset(&game, 0, sizeof game);
    game.data = direct_image;
    game.size = image_size;
    CHECK(p_load_game(&game));
    for (unsigned attempt = 0u; attempt < 4u; attempt++) {
        check_jam_callback(black, attempt % 2u == 0u);
    }

    /* Cartridge loading may preserve CPU RAM; explicitly make the reset gate
       marker zero before the initial delayed-JAM run. */
    memcpy(setup_image, base_image, image_size);
    memcpy(setup_image + 16u, clear_ram_loop, sizeof clear_ram_loop);
    game.data = setup_image;
    CHECK(p_load_game(&game));
    p_reset();
    p_set_audio_sample(sample);
    p_set_audio_sample_batch(batch);
    batch_calls = 0;
    p_run();
    CHECK_EQ_U64(batch_calls, 1u);

    /* The first frame completes visibly; a generated nested delay reaches JAM
       later, while the reset gate makes soft reset re-enter JAM immediately. */
    memcpy(delayed_image, base_image, image_size);
    memcpy(delayed_image + 16u + 0x32u, delay_then_jam, sizeof delay_then_jam);
    memcpy(delayed_image + 16u + 0x100u, reset_gate, sizeof reset_gate);
    delayed_image[16u + 0x3ffau] = 0x00u;
    delayed_image[16u + 0x3ffbu] = 0x81u;
    delayed_image[16u + 0x3ffcu] = 0x00u;
    delayed_image[16u + 0x3ffdu] = 0x81u;
    delayed_image[16u + 0x3ffeu] = 0x00u;
    delayed_image[16u + 0x3fffu] = 0x81u;
    game.data = delayed_image;
    CHECK(p_load_game(&game));

    state_size = p_serialize_size();
    CHECK(state_size > 0u);
    if (state_size == 0u) {
        p_unload_game();
        return;
    }
    origin = (uint8_t *)malloc(state_size);
    jammed = (uint8_t *)malloc(state_size);
    roundtrip = (uint8_t *)malloc(state_size);
    CHECK(origin != NULL && jammed != NULL && roundtrip != NULL);
    if (origin == NULL || jammed == NULL || roundtrip == NULL) {
        free(origin);
        free(jammed);
        free(roundtrip);
        p_unload_game();
        return;
    }
    CHECK(p_serialize(origin, state_size));
    CHECK(run_delayed_jam(last_complete));
    CHECK(memcmp(last_complete, black, sizeof black) != 0);

    sample_calls = 0;
    batch_calls = 0;
    batch_frames = 0u;
    CHECK(p_serialize(jammed, state_size));
    CHECK_EQ_U64(sample_calls, 0u);
    CHECK_EQ_U64(batch_calls, 0u);
    check_jam_callback(last_complete, false);

    /* A refused load preserves the complete presentation. A successful load of the state saved
       at the cached frame keeps it (D-16), without putting pixels into the native state. */
    CHECK(!p_unserialize(jammed, state_size - 1u));
    check_jam_callback(last_complete, true);
    CHECK(p_unserialize(jammed, state_size));
    CHECK(video_source != NULL);
    if (video_source != NULL) {
        CHECK(memcmp(video_source, last_complete, sizeof last_complete) == 0);
    }
    CHECK(p_serialize(roundtrip, state_size));
    CHECK(memcmp(roundtrip, jammed, state_size) == 0);
    check_jam_callback(last_complete, false);

    /* A valid pre-JAM origin clears the latch and deliberately executes again. */
    CHECK(p_unserialize(origin, state_size));
    CHECK(run_delayed_jam(last_complete));
    CHECK(memcmp(last_complete, black, sizeof black) != 0);

    /* CPU RAM survives reset. The generated reset gate sees its marker and
       executes JAM at once, so a valid prior frame would be visibly stale. */
    p_reset();
    check_jam_callback(black, true);

    /* Rebuild a valid cached frame, unload, then reload direct JAM content.
       The no-content frame between sessions also remains a normal completion. */
    CHECK(p_unserialize(origin, state_size));
    sample_calls = 0;
    batch_calls = 0;
    batch_frames = 0u;
    p_run();
    CHECK_EQ_U64(batch_calls, 1u);
    CHECK(video_data_nonnull);
    memcpy(last_complete, frame, sizeof frame);
    p_unload_game();
    CHECK(p_load_game(NULL));
    p_run();
    CHECK(video_data_nonnull);
    game.data = direct_image;
    CHECK(p_load_game(&game));
    check_jam_callback(black, false);
    p_unload_game();

    free(origin);
    free(jammed);
    free(roundtrip);
    p_set_audio_sample(sample);
    p_set_audio_sample_batch(batch);
    check_jam_timeline();
}

/* D-16 amendment: a state from a diverged timeline (same frame number, different input) never
   borrows the cached picture of the other timeline. NMI handler: read controller 1, put a
   backdrop that depends on A in $3F00 (rendering stays off), execute JAM when Start is held. */
static void check_jam_timeline(void)
{
    static const uint8_t reset_code[] = {0x78u, 0xa9u, 0x80u, 0x8du, 0x00u,
                                         0x20u, 0x4cu, 0x06u, 0x80u};
    static const uint8_t nmi_code[] = {
        0xa9u, 0x01u, 0x8du, 0x16u, 0x40u, 0xa9u, 0x00u, 0x8du, 0x16u, 0x40u, /* strobe */
        0xadu, 0x16u, 0x40u, 0x29u, 0x01u, 0xaau,                             /* A -> X */
        0xadu, 0x16u, 0x40u, 0xadu, 0x16u, 0x40u, 0xadu, 0x16u, 0x40u,        /* B, Select, Start */
        0x29u, 0x01u, 0xf0u, 0x01u, 0x02u,                                    /* Start: JAM */
        0xa9u, 0x3fu, 0x8du, 0x06u, 0x20u, 0xa9u, 0x00u, 0x8du, 0x06u, 0x20u, 0xa9u,
        0x2au, 0xe0u, 0x00u, 0xf0u, 0x02u, 0xa9u, 0x16u, 0x8du, 0x07u, 0x20u, 0xa9u,
        0x3fu, 0x8du, 0x06u, 0x20u, 0xa9u, 0x00u, 0x8du, 0x06u, 0x20u, /* v back to $3F00 */
        0x40u};
    static const struct ines_segment segments[] = {{0x10u, nmi_code, sizeof nmi_code}};
    static const uint32_t black[W * H] = {0u};
    static uint8_t image[16u + INES_PRG_BANK + INES_CHR_BANK];
    static uint32_t pic_a[W * H];
    static uint32_t pic_b[W * H];
    enum { K = 4 };
    struct ines_spec spec;
    struct retro_game_info game;
    uint8_t *base = NULL, *s_a = NULL, *s_a_prev = NULL, *s_b = NULL;
    size_t state_size;

    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = 1u;
    spec.prg_code = reset_code;
    spec.prg_code_len = sizeof reset_code;
    spec.segments = segments;
    spec.segment_count = 1u;
    spec.nmi_vector = 0x8010u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    memset(&game, 0, sizeof game);
    game.size = ines_build(image, sizeof image, &spec);
    game.data = image;
    CHECK(game.size != 0u);
    if (game.size == 0u || !p_load_game(&game)) {
        CHECK(0);
        return;
    }
    p_set_audio_sample(sample);
    p_set_audio_sample_batch(batch);
    state_size = p_serialize_size();
    base = (uint8_t *)malloc(state_size);
    s_a = (uint8_t *)malloc(state_size);
    s_a_prev = (uint8_t *)malloc(state_size);
    s_b = (uint8_t *)malloc(state_size);
    CHECK(base != NULL && s_a != NULL && s_a_prev != NULL && s_b != NULL);
    if (base == NULL || s_a == NULL || s_a_prev == NULL || s_b == NULL) {
        goto done;
    }
    memset(host_buttons, 0, sizeof host_buttons);
    for (unsigned i = 0u; i < 4u; i++) {
        p_run();
    }
    CHECK(p_serialize(base, state_size));
    /* Timeline B: A held for K frames. */
    host_buttons[0] = NESTURBATOR_BUTTON_A;
    for (unsigned i = 0u; i < (unsigned)K; i++) {
        p_run();
    }
    memcpy(pic_b, frame, sizeof pic_b);
    CHECK(p_serialize(s_b, state_size));
    /* Timeline A: the same base, no buttons for K frames. */
    CHECK(p_unserialize(base, state_size));
    host_buttons[0] = 0u;
    for (unsigned i = 0u; i < (unsigned)K; i++) {
        if (i == (unsigned)K - 1u) {
            CHECK(p_serialize(s_a_prev, state_size));
        }
        p_run();
    }
    memcpy(pic_a, frame, sizeof pic_a);
    CHECK(p_serialize(s_a, state_size));
    CHECK(memcmp(pic_a, pic_b, sizeof pic_a) != 0);
    CHECK(memcmp(pic_a, black, sizeof pic_a) != 0);

    /* The same frame number on the other timeline: black, never picture A. */
    CHECK(p_unserialize(s_b, state_size));
    host_buttons[0] = NESTURBATOR_BUTTON_START;
    check_jam_callback(black, true);
    /* The state the cached frame produced: its picture. */
    CHECK(p_unserialize(s_a, state_size));
    check_jam_callback(pic_a, true);
    /* A different frame number: black. */
    CHECK(p_unserialize(s_a_prev, state_size));
    check_jam_callback(black, true);
    /* Back on the cache's timeline again: the picture. */
    CHECK(p_unserialize(s_a, state_size));
    check_jam_callback(pic_a, true);
done:
    memset(host_buttons, 0, sizeof host_buttons);
    free(base);
    free(s_a);
    free(s_a_prev);
    free(s_b);
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

/* ---- --rom-parity: a whole ROM through the module, compared with the runner (D-18) ---- */

#define PARITY_SENTINEL 0xA5A5A5A5u

static int read_file(const char *path, unsigned char **data, size_t *size)
{
    FILE *f = fopen(path, "rb");
    long end;
    *data = NULL;
    *size = 0u;
    if (f == NULL) {
        return 0;
    }
    if (fseek(f, 0, SEEK_END) != 0 || (end = ftell(f)) <= 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return 0;
    }
    *data = (unsigned char *)malloc((size_t)end);
    if (*data == NULL || fread(*data, 1, (size_t)end, f) != (size_t)end) {
        free(*data);
        *data = NULL;
        fclose(f);
        return 0;
    }
    fclose(f);
    *size = (size_t)end;
    return 1;
}

/* True when every pixel of the captured frame equals the P6 image. */
static int frame_equals_ppm(const char *path)
{
    static unsigned char rgb[W * H * 3u];
    static const char header[] = "P6\n256 240\n255\n";
    unsigned char head[sizeof header - 1u];
    uint32_t i;
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        return 0;
    }
    if (fread(head, 1, sizeof head, f) != sizeof head || memcmp(head, header, sizeof head) != 0 ||
        fread(rgb, 1, sizeof rgb, f) != sizeof rgb) {
        fclose(f);
        return 0;
    }
    fclose(f);
    for (i = 0; i < W * H; i++) {
        uint32_t want = ((uint32_t)rgb[3u * i] << 16) | ((uint32_t)rgb[3u * i + 1u] << 8) |
                        (uint32_t)rgb[3u * i + 2u];
        if ((frame[i] & 0xFFFFFFu) != want) {
            return 0;
        }
    }
    return 1;
}

/* After the last frame, the battery span must be the file's size and bytes (D-18). */
static void check_sav_expect(const char *path)
{
    unsigned char *want = NULL;
    size_t want_size = 0u, span_size, i;
    const unsigned char *span;

    CHECK(read_file(path, &want, &want_size));
    span = (const unsigned char *)p_get_memory_data(RETRO_MEMORY_SAVE_RAM);
    span_size = p_get_memory_size(RETRO_MEMORY_SAVE_RAM);
    CHECK(span != NULL);
    CHECK_EQ_U64(span_size, want_size);
    if (want != NULL && span != NULL && span_size == want_size) {
        for (i = 0u; i < want_size; i++) {
            if (span[i] != want[i]) {
                fprintf(stderr, "libretro.host: save span differs from %s at offset %lu\n", path,
                        (unsigned long)i);
                CHECK(span[i] == want[i]);
                break;
            }
        }
    }
    free(want);
}

/* libretro.host --rom-parity <module> <rom> <frames> <ppm> [--movie FILE] [--testcard PPM]
 *                            [--sav-in FILE] [--sav-expect FILE]
 * --sav-in copies FILE into the battery span before the first frame (its size must
 * equal the span); --sav-expect compares the span with FILE after the last frame.
 * Runs the ROM for <frames> frames through the module, feeding the movie's
 * masks as joypad state, and requires the last frame to equal the runner's
 * image <ppm>. The sentinel fill before each retro_run proves the module wrote
 * the frame; --testcard fails a frame that is just the test card. */
static int rom_parity(int argc, char **argv)
{
    const char *movie_path = NULL, *testcard_path = NULL;
    const char *sav_in_path = NULL, *sav_expect_path = NULL;
    nesturbator_movie movie;
    struct retro_game_info game;
    unsigned char *rom = NULL;
    size_t rom_size = 0u;
    unsigned long frames;
    unsigned long f;
    uint32_t i, unwritten = 0u;
    char *end = NULL;
    int a;

    memset(&movie, 0, sizeof movie);
    frames = strtoul(argv[4], &end, 10);
    if (end == argv[4] || *end != '\0' || frames == 0ul) {
        fprintf(stderr, "libretro.host: bad frame count %s\n", argv[4]);
        return 2;
    }
    for (a = 6; a < argc; a++) {
        if (strcmp(argv[a], "--movie") == 0 && a + 1 < argc) {
            movie_path = argv[++a];
        } else if (strcmp(argv[a], "--testcard") == 0 && a + 1 < argc) {
            testcard_path = argv[++a];
        } else if (strcmp(argv[a], "--sav-in") == 0 && a + 1 < argc) {
            sav_in_path = argv[++a];
        } else if (strcmp(argv[a], "--sav-expect") == 0 && a + 1 < argc) {
            sav_expect_path = argv[++a];
        } else {
            fprintf(stderr, "libretro.host: unknown --rom-parity option %s\n", argv[a]);
            return 2;
        }
    }
    if (!module_open(argv[2])) {
        fprintf(stderr, "cannot load %s\n", argv[2]);
        return 1;
    }
    resolve_all();
    CHECK_EQ_U64(missing, 0);
    if (missing != 0) {
        module_close();
        CHECK_DONE();
    }
    if (movie_path != NULL) {
        CHECK(nesturbator_movie_read(movie_path, &movie));
        CHECK(movie.frame_count >= frames);
        if (movie.masks == NULL || movie.frame_count < frames) {
            module_close();
            CHECK_DONE();
        }
    }
    CHECK(read_file(argv[3], &rom, &rom_size));
    if (rom == NULL) {
        nesturbator_movie_free(&movie);
        module_close();
        CHECK_DONE();
    }

    p_set_environment(env);
    p_init();
    p_set_video_refresh(video);
    p_set_audio_sample(sample);
    p_set_audio_sample_batch(batch);
    p_set_input_poll(on_poll);
    p_set_input_state(input);

    memset(&game, 0, sizeof game);
    game.data = rom;
    game.size = rom_size;
    CHECK(p_load_game(&game));
    if (sav_in_path != NULL) {
        unsigned char *sav = NULL;
        size_t sav_size = 0u;
        void *span = p_get_memory_data(RETRO_MEMORY_SAVE_RAM);
        CHECK(read_file(sav_in_path, &sav, &sav_size));
        CHECK(span != NULL);
        CHECK_EQ_U64(sav_size, p_get_memory_size(RETRO_MEMORY_SAVE_RAM));
        if (sav != NULL && span != NULL && sav_size == p_get_memory_size(RETRO_MEMORY_SAVE_RAM)) {
            memcpy(span, sav, sav_size);
        }
        free(sav);
    }
    video_calls = 0;
    for (f = 0; f < frames; f++) {
        for (i = 0; i < W * H; i++) {
            frame[i] = PARITY_SENTINEL;
        }
        if (movie_path != NULL) {
            host_buttons[0] = movie.masks[f * 2u];
            host_buttons[1] = movie.masks[f * 2u + 1u];
        } else {
            host_buttons[0] = 0u;
            host_buttons[1] = 0u;
        }
        p_run();
    }
    CHECK_EQ_U64(video_calls, frames);
    for (i = 0; i < W * H; i++) {
        unwritten += (frame[i] == PARITY_SENTINEL) ? 1u : 0u;
    }
    CHECK(unwritten < W * H);
    compare_with_ppm(argv[5]);
    if (testcard_path != NULL) {
        CHECK(!frame_equals_ppm(testcard_path));
    }
    if (sav_expect_path != NULL) {
        check_sav_expect(sav_expect_path);
    }
    p_unload_game();
    p_deinit();
    module_close();
    free(rom);
    nesturbator_movie_free(&movie);
    CHECK_DONE();
}

/* ---- --serialize: the save-state entry points through the module (LR-01) ---- */

#define SER_FNV_BASIS 14695981039346656037ull
#define SER_PRE 20u /* frames run before the reference state is taken */
#define SER_ROUND_TRIP 30u
#define SER_PAD 16u /* sentinel bytes past a buffer */
#define SER_SENTINEL 0xCCu
#define SER_MIN_ROMS 5u /* D-20: a pass over fewer ROMs than this cannot succeed */
#define SER_MIN_REFUSALS 12u
#define SER_MIN_FRAMES 120u

static unsigned ser_frames_compared;

/* The input for frame t: a fixed integer pattern on both ports, so every pass of a ROM sees
   the same buttons at the same frame. */
static void ser_set_input(uint32_t t)
{
    uint32_t x = t * 2654435761u + 12345u;
    host_buttons[0] = (uint8_t)(x >> 13);
    host_buttons[1] = (uint8_t)(x >> 5);
}

/* FNV-1a over 32-bit words: the frame is a whole number of pixels, so one multiply per pixel
   instead of per byte gives the same discrimination at a quarter of the cost. */
static uint64_t ser_hash_words(const uint32_t *words, size_t count)
{
    uint64_t h = SER_FNV_BASIS;
    size_t i;
    for (i = 0u; i < count; i++) {
        h = (h ^ words[i]) * 1099511628211ull;
    }
    return h;
}

/* Runs frame t and returns the video and audio hashes the frontend saw. */
static void ser_frame(uint32_t t, uint64_t *video_hash, uint64_t *audio_hash)
{
    ser_set_input(t);
    batch_frames = 0u;
    p_run();
    *video_hash = ser_hash_words(frame, W * H);
    *audio_hash =
        fnv_add(SER_FNV_BASIS, batch_samples, batch_frames * 2u * sizeof batch_samples[0]);
}

/* Frames with the input of frame t and no hashing, for the stretches nothing compares. */
static void ser_run(uint32_t from, uint32_t to)
{
    uint32_t t;
    for (t = from; t < to; t++) {
        ser_set_input(t);
        p_run();
    }
}

static bool ser_load(const unsigned char *rom, size_t size)
{
    struct retro_game_info game;
    memset(&game, 0, sizeof game);
    game.data = rom;
    game.size = size;
    return p_load_game(&game);
}

/* The callbacks the core made, so a refused call can be shown to have made none. */
struct ser_calls {
    int video, batch, sample, env, poll, input;
};

static struct ser_calls ser_calls_now(void)
{
    struct ser_calls c;
    c.video = video_calls;
    c.batch = batch_calls;
    c.sample = sample_calls;
    c.env = env_calls;
    c.poll = poll_calls;
    c.input = input_calls;
    return c;
}

static int ser_calls_equal(struct ser_calls a, struct ser_calls b)
{
    return a.video == b.video && a.batch == b.batch && a.sample == b.sample && a.env == b.env &&
           a.poll == b.poll && a.input == b.input;
}

/* The state size of the same bytes loaded straight into the library. */
static size_t ser_direct_size(const unsigned char *rom, size_t rom_size)
{
    nesturbator_config cfg;
    nesturbator *nes = NULL;
    size_t size = 0u;

    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &nes), NESTURBATOR_OK);
    if (nes != NULL) {
        CHECK_EQ_U64(nesturbator_load_cartridge(nes, rom, rom_size), NESTURBATOR_OK);
        size = nesturbator_state_size(nes);
        nesturbator_destroy(nes);
    }
    return size;
}

/* One ROM argument, with the module's state after SER_PRE frames. */
struct ser_rom {
    const char *path;
    unsigned char *data;
    size_t size;
    size_t state_size;
    unsigned char *state;
};

static int ser_prepare(struct ser_rom *r)
{
    CHECK(read_file(r->path, &r->data, &r->size));
    if (r->data == NULL) {
        return 0;
    }
    CHECK(ser_load(r->data, r->size));
    ser_run(0u, SER_PRE);
    r->state_size = p_serialize_size();
    CHECK(r->state_size != 0u);
    CHECK_EQ_U64(r->state_size, ser_direct_size(r->data, r->size));
    r->state = (unsigned char *)malloc(r->state_size != 0u ? r->state_size : 1u);
    if (r->state == NULL || r->state_size < 64u + 2u) {
        return 0;
    }
    CHECK(p_serialize(r->state, r->state_size));
    p_unload_game();
    return 1;
}

/* D-01, D-04: the size is the library's for the loaded board, constant while the game runs,
   and 0 with no content. */
static void check_serialize_size(const struct ser_rom *r, const struct ser_rom *other)
{
    size_t size;
    uint32_t t;

    CHECK(ser_load(r->data, r->size));
    size = p_serialize_size();
    CHECK_EQ_U64(size, r->state_size);
    for (t = 0u; t < 120u; t++) {
        ser_run(t, t + 1u);
        CHECK_EQ_U64(p_serialize_size(), size);
    }
    ser_frames_compared += 120u;
    p_reset();
    CHECK_EQ_U64(p_serialize_size(), size);
    p_set_controller_port_device(0u, RETRO_DEVICE_JOYPAD);
    CHECK_EQ_U64(p_serialize_size(), size);
    p_set_controller_port_device(1u, RETRO_DEVICE_NONE);
    CHECK_EQ_U64(p_serialize_size(), size);
    p_unload_game();
    CHECK_EQ_U64(p_serialize_size(), 0u);
    CHECK(ser_load(other->data, other->size));
    CHECK_EQ_U64(p_serialize_size(), other->state_size);
    p_unload_game();
    CHECK_EQ_U64(p_serialize_size(), 0u);
}

/* D-02: serialize writes exactly size bytes, or nothing. */
static void check_serialize_writes(const struct ser_rom *r)
{
    size_t n = r->state_size, i;
    unsigned char *a = (unsigned char *)malloc(n + SER_PAD);
    unsigned char *b = (unsigned char *)malloc(n);
    uint64_t v1, a1, v2, a2;

    CHECK(a != NULL && b != NULL);
    if (a == NULL || b == NULL) {
        free(a);
        free(b);
        return;
    }
    CHECK(ser_load(r->data, r->size));
    ser_run(0u, SER_PRE);

    memset(a, SER_SENTINEL, n + SER_PAD);
    CHECK(!p_serialize(a, n - 1u));
    CHECK(!p_serialize(a, 0u));
    CHECK(!p_serialize(NULL, n));
    for (i = 0u; i < n + SER_PAD; i++) {
        if (a[i] != SER_SENTINEL) {
            CHECK_EQ_HEX(a[i], SER_SENTINEL);
            break;
        }
    }
    CHECK(p_serialize(a, n + SER_PAD));
    for (i = n; i < n + SER_PAD; i++) {
        if (a[i] != SER_SENTINEL) {
            CHECK_EQ_HEX(a[i], SER_SENTINEL);
            break;
        }
    }
    CHECK(memcmp(a, r->state, n) == 0);
    CHECK(p_serialize(b, n));
    CHECK(memcmp(a, b, n) == 0);

    /* Loading the same state twice gives the same next frame. */
    CHECK(p_unserialize(a, n));
    ser_frame(SER_PRE, &v1, &a1);
    CHECK(p_unserialize(a, n));
    ser_frame(SER_PRE, &v2, &a2);
    CHECK_EQ_HEX(v1, v2);
    CHECK_EQ_HEX(a1, a2);
    ser_frames_compared += 1u;

    /* The tracer: 30 frames after the state, then the state again and the same 30 frames. */
    {
        uint64_t rv[SER_ROUND_TRIP], ra[SER_ROUND_TRIP];
        uint32_t t;
        CHECK(p_unserialize(a, n));
        for (t = 0u; t < SER_ROUND_TRIP; t++) {
            ser_frame(SER_PRE + t, &rv[t], &ra[t]);
        }
        CHECK(p_unserialize(a, n));
        for (t = 0u; t < SER_ROUND_TRIP; t++) {
            ser_frame(SER_PRE + t, &v1, &a1);
            CHECK_EQ_HEX(v1, rv[t]);
            CHECK_EQ_HEX(a1, ra[t]);
        }
        ser_frames_compared += SER_ROUND_TRIP;
    }
    p_unload_game();
    free(a);
    free(b);
}

/* D-03: every refusal returns false, makes no callback and changes nothing: the next frame
   and the next state equal a control that never made the call. Returns the cases run. */
static unsigned check_unserialize_rejects(const struct ser_rom *r, const struct ser_rom *other)
{
    size_t n = r->state_size;
    unsigned char *buf = (unsigned char *)malloc(2u * n + 1u);
    unsigned char *control = (unsigned char *)malloc(n);
    unsigned char *post = (unsigned char *)malloc(n);
    uint64_t cv, ca, tv, ta;
    unsigned cases = 0u, c;

    CHECK(buf != NULL && control != NULL && post != NULL);
    if (buf == NULL || control == NULL || post == NULL) {
        free(buf);
        free(control);
        free(post);
        return 0u;
    }
    CHECK(ser_load(r->data, r->size));
    ser_run(0u, SER_PRE);
    ser_frame(SER_PRE, &cv, &ca);
    CHECK(p_serialize(control, n));
    p_unload_game();

    for (c = 0u; c < 13u; c++) {
        const void *arg = buf;
        size_t len = n;
        struct ser_calls before, after;
        bool ok;

        memcpy(buf, r->state, n);
        buf[n] = 0u;
        switch (c) {
        case 0:
            arg = NULL;
            break;
        case 1:
            len = 0u;
            break;
        case 2:
            len = 63u;
            break;
        case 3:
            len = n - 1u;
            break;
        case 4:
            len = n + 1u;
            break;
        case 5:
            memcpy(buf + n, r->state, n);
            len = 2u * n;
            break;
        case 6:
            buf[0] ^= 0x01u;
            break;
        case 7:
            buf[8] ^= 0x01u;
            break;
        case 8:
            buf[10] ^= 0x01u;
            break;
        case 9:
            buf[20] ^= 0x01u;
            break;
        case 10:
            buf[28] ^= 0x01u;
            break;
        case 11:
            buf[64u + (n - 64u) / 2u] ^= 0x01u;
            break;
        default:
            if (other == r) {
                continue;
            }
            arg = other->state;
            len = other->state_size;
            break;
        }
        if (c == 0u) {
            CHECK(ser_load(r->data, r->size));
        }
        /* The reference state puts the module at frame SER_PRE; the call under test follows. */
        CHECK(p_unserialize(r->state, n));
        before = ser_calls_now();
        ok = p_unserialize(arg, len);
        after = ser_calls_now();
        CHECK(!ok);
        CHECK(ser_calls_equal(before, after));
        ser_frame(SER_PRE, &tv, &ta);
        CHECK_EQ_HEX(tv, cv);
        CHECK_EQ_HEX(ta, ca);
        CHECK(p_serialize(post, n));
        CHECK(memcmp(post, control, n) == 0);
        ser_frames_compared += 1u;
        cases++;
    }
    p_unload_game();
    free(buf);
    free(control);
    free(post);
    return cases;
}

/* D-05: a state loaded straight after load, and straight after reset, continues as saved. */
static void check_unserialize_before_run(const struct ser_rom *r)
{
    enum { N = 12 };
    size_t n = r->state_size;
    unsigned char *s_load = (unsigned char *)malloc(n);
    unsigned char *s_mid = (unsigned char *)malloc(n);
    uint64_t v1[N], a1[N], v3[N], a3[N], v, a;
    uint32_t t;

    CHECK(s_load != NULL && s_mid != NULL);
    if (s_load == NULL || s_mid == NULL) {
        free(s_load);
        free(s_mid);
        return;
    }
    CHECK(ser_load(r->data, r->size));
    CHECK(p_serialize(s_load, n));
    for (t = 0u; t < N; t++) {
        ser_frame(t, &v1[t], &a1[t]);
    }
    CHECK(p_serialize(s_mid, n));
    CHECK(memcmp(s_load, s_mid, n) != 0);
    for (t = N; t < 2u * N; t++) {
        ser_frame(t, &v3[t - N], &a3[t - N]);
    }

    CHECK(ser_load(r->data, r->size));
    CHECK(p_unserialize(s_load, n));
    for (t = 0u; t < N; t++) {
        ser_frame(t, &v, &a);
        CHECK_EQ_HEX(v, v1[t]);
        CHECK_EQ_HEX(a, a1[t]);
    }

    CHECK(ser_load(r->data, r->size));
    ser_run(0u, N);
    ser_run(N, N + 10u);
    p_reset();
    CHECK(p_unserialize(s_mid, n));
    for (t = N; t < 2u * N; t++) {
        ser_frame(t, &v, &a);
        CHECK_EQ_HEX(v, v3[t - N]);
        CHECK_EQ_HEX(a, a3[t - N]);
    }
    ser_frames_compared += 2u * N;
    p_unload_game();
    free(s_load);
    free(s_mid);
}

/* D-07, D-08: the bytes are the same whichever savestate context and audio/video flags the
   host reports. The core queries neither, so the host's query counters stay zero. */
static void check_serialize_contexts(const struct ser_rom *r)
{
    static const int contexts[6] = {RETRO_SAVESTATE_CONTEXT_NORMAL,
                                    RETRO_SAVESTATE_CONTEXT_RUNAHEAD_SAME_INSTANCE,
                                    RETRO_SAVESTATE_CONTEXT_RUNAHEAD_SAME_BINARY,
                                    RETRO_SAVESTATE_CONTEXT_ROLLBACK_NETPLAY,
                                    RETRO_SAVESTATE_CONTEXT_NORMAL,
                                    RETRO_SAVESTATE_CONTEXT_ROLLBACK_NETPLAY};
    static const int av_flags[6] = {3, 3, 3, 3, 0, 0};
    size_t n = r->state_size;
    unsigned char *ref = (unsigned char *)malloc(3u * n);
    unsigned char *got = (unsigned char *)malloc(n);
    unsigned pass;

    CHECK(ref != NULL && got != NULL);
    if (ref == NULL || got == NULL) {
        free(ref);
        free(got);
        return;
    }
    for (pass = 0u; pass < 6u; pass++) {
        unsigned k;
        host_savestate_context = contexts[pass];
        host_av_enable = av_flags[pass];
        CHECK(ser_load(r->data, r->size));
        for (k = 0u; k < 3u; k++) {
            ser_run(k * 3u, (k + 1u) * 3u);
            if (pass == 0u) {
                CHECK(p_serialize(ref + k * n, n));
            } else {
                CHECK(p_serialize(got, n));
                CHECK(memcmp(got, ref + k * n, n) == 0);
            }
        }
        p_unload_game();
    }
    host_savestate_context = RETRO_SAVESTATE_CONTEXT_NORMAL;
    host_av_enable = 3;
    ser_frames_compared += 9u;
    free(ref);
    free(got);
}

/* D-09: a load every frame leaves the battery span where it was, and loading the same state
   again never moves the save generation. */
static void check_unserialize_battery(const struct ser_rom *r)
{
    static uint16_t native[W * H];
    static int16_t audio[1024];
    nesturbator_config cfg;
    nesturbator_frame io;
    nesturbator *nes = NULL;
    size_t n = r->state_size;
    unsigned char *buf = (unsigned char *)malloc(n);
    void *span;
    unsigned i, loads = 0u;

    CHECK(buf != NULL);
    if (buf == NULL) {
        return;
    }
    CHECK(ser_load(r->data, r->size));
    span = p_get_memory_data(RETRO_MEMORY_SAVE_RAM);
    CHECK(span != NULL);
    CHECK_EQ_U64(p_get_memory_size(RETRO_MEMORY_SAVE_RAM), 8192u);
    ser_run(0u, 5u);
    CHECK(p_serialize(buf, n));
    for (i = 0u; i < 60u; i++) {
        ser_run(5u + i, 6u + i);
        CHECK(p_unserialize(buf, n));
        loads++;
        CHECK(p_get_memory_data(RETRO_MEMORY_SAVE_RAM) == span);
        CHECK_EQ_U64(p_get_memory_size(RETRO_MEMORY_SAVE_RAM), 8192u);
    }
    CHECK_EQ_U64(loads, 60u);
    p_unload_game();

    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &nes), NESTURBATOR_OK);
    if (nes != NULL) {
        CHECK_EQ_U64(nesturbator_load_cartridge(nes, r->data, r->size), NESTURBATOR_OK);
        memset(&io, 0, sizeof io);
        io.size = (uint32_t)sizeof io;
        io.video = native;
        io.video_pitch = W;
        io.audio = audio;
        io.audio_capacity = 1024u;
        for (i = 0u; i < 5u; i++) {
            CHECK_EQ_U64(nesturbator_run_frame(nes, &io), NESTURBATOR_OK);
        }
        CHECK_EQ_U64(nesturbator_save_state(nes, buf, n), NESTURBATOR_OK);
        loads = 0u;
        for (i = 0u; i < 60u; i++) {
            uint64_t before, after;
            CHECK_EQ_U64(nesturbator_run_frame(nes, &io), NESTURBATOR_OK);
            before = nesturbator_save_generation(nes);
            CHECK_EQ_U64(nesturbator_load_state(nes, buf, n), NESTURBATOR_OK);
            after = nesturbator_save_generation(nes);
            /* nesturbator.h: a load counts once when it changes the battery bytes and not at
               all when it leaves them alone, so the frames since the save may cost one. */
            CHECK(after == before || after == before + 1u);
            /* The same state again, as rewind does, never moves it. */
            CHECK_EQ_U64(nesturbator_load_state(nes, buf, n), NESTURBATOR_OK);
            CHECK_EQ_U64(nesturbator_save_generation(nes), after);
            loads++;
        }
        CHECK_EQ_U64(loads, 60u);
        nesturbator_destroy(nes);
    }
    free(buf);
}

/* D-01, D-02, D-03 with nothing to save: no content, the test card, a refused load, unload. */
static void check_serialize_no_content(const struct ser_rom *r)
{
    static unsigned char bad[1] = {0x4eu};
    unsigned char *buf = (unsigned char *)malloc(r->state_size + SER_PAD);
    uint64_t cv[4], ca[4], tv, ta;
    uint32_t t;
    unsigned pass;
    size_t i;

    CHECK(buf != NULL);
    if (buf == NULL) {
        return;
    }
    for (pass = 0u; pass < 4u; pass++) {
        bool ok;
        p_unload_game();
        switch (pass) {
        case 0:
            break;
        case 1:
            CHECK(p_load_game(NULL));
            break;
        case 2:
            CHECK(ser_load(r->data, r->size));
            CHECK(!ser_load(bad, sizeof bad));
            break;
        default:
            CHECK(ser_load(r->data, r->size));
            p_unload_game();
            break;
        }
        CHECK_EQ_U64(p_serialize_size(), 0u);
        memset(buf, SER_SENTINEL, r->state_size + SER_PAD);
        ok = p_serialize(buf, r->state_size + SER_PAD);
        CHECK(!ok);
        for (i = 0u; i < r->state_size + SER_PAD; i++) {
            if (buf[i] != SER_SENTINEL) {
                CHECK_EQ_HEX(buf[i], SER_SENTINEL);
                break;
            }
        }
        CHECK(!p_unserialize(r->state, r->state_size));
        CHECK(!p_unserialize(NULL, r->state_size));
    }

    /* The test card's frames are unchanged by the refused calls. */
    p_unload_game();
    CHECK(p_load_game(NULL));
    for (t = 0u; t < 4u; t++) {
        ser_frame(t, &cv[t], &ca[t]);
    }
    CHECK(p_load_game(NULL));
    ser_run(0u, 2u);
    {
        struct ser_calls before = ser_calls_now(), after;
        CHECK(!p_serialize(buf, r->state_size));
        CHECK(!p_unserialize(r->state, r->state_size));
        after = ser_calls_now();
        CHECK(ser_calls_equal(before, after));
    }
    for (t = 2u; t < 4u; t++) {
        ser_frame(t, &tv, &ta);
        CHECK_EQ_HEX(tv, cv[t]);
        CHECK_EQ_HEX(ta, ca[t]);
    }
    ser_frames_compared += 2u;
    p_unload_game();
    free(buf);
}

/* D-04: a jammed CPU does not change the size. */
static void check_serialize_jam(void)
{
    static unsigned char image[16u + 16384u + 8192u];
    size_t size;

    memset(image, 0, sizeof image);
    memcpy(image, "NES\x1a", 4u);
    image[4] = 1u;
    image[5] = 1u;
    image[16] = 0x02u; /* JAM */
    image[16u + 0x3ffau] = 0x00u;
    image[16u + 0x3ffbu] = 0x80u;
    image[16u + 0x3ffcu] = 0x00u;
    image[16u + 0x3ffdu] = 0x80u;
    image[16u + 0x3ffeu] = 0x00u;
    image[16u + 0x3fffu] = 0x80u;
    CHECK(ser_load(image, sizeof image));
    size = p_serialize_size();
    CHECK_EQ_U64(size, ser_direct_size(image, sizeof image));
    CHECK(size != 0u);
    p_run();
    p_run();
    CHECK_EQ_U64(p_serialize_size(), size);
    p_unload_game();
}

enum fixture_role { FIXTURE_ROLE_GENERIC, FIXTURE_ROLE_NESTEROIDS, FIXTURE_ROLE_MMC1_BATTERY };

/* Fixture roles use only the exact final filename; path bytes stay untouched. */
static const char *fixture_final_component(const char *path)
{
    const char *component;

    if (path == NULL) {
        return NULL;
    }
    component = path;
    for (const char *cursor = path; *cursor != '\0'; cursor++) {
        if (*cursor == '/') {
            component = cursor + 1;
        }
#if defined(_WIN32)
        else if (*cursor == '\\') {
            component = cursor + 1;
        }
#endif
    }
    if (*component == '\0') {
        return NULL;
    }
    return component;
}

static enum fixture_role fixture_role_for_path(const char *path)
{
    const char *component = fixture_final_component(path);

    if (component != NULL && strcmp(component, "nesteroids.nes") == 0) {
        return FIXTURE_ROLE_NESTEROIDS;
    }
    if (component != NULL && strcmp(component, "M1_P128K_C128K_S8K.nes") == 0) {
        return FIXTURE_ROLE_MMC1_BATTERY;
    }
    return FIXTURE_ROLE_GENERIC;
}

static int fixture_role_selftest(void)
{
    unsigned cases = 0u;

    CHECK_EQ_U64(fixture_role_for_path("games/nesteroids.nes"), FIXTURE_ROLE_NESTEROIDS);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path("roms/M1_P128K_C128K_S8K.nes"), FIXTURE_ROLE_MMC1_BATTERY);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path("nesteroids/generic.nes"), FIXTURE_ROLE_GENERIC);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path("M1_P128K_C128K_S8K/generic.nes"), FIXTURE_ROLE_GENERIC);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path("games/not-nesteroids.nes"), FIXTURE_ROLE_GENERIC);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path("games/nesteroids.nes.bak"), FIXTURE_ROLE_GENERIC);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path("games/xM1_P128K_C128K_S8K.nes"), FIXTURE_ROLE_GENERIC);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path("games/M1_P128K_C128K_S8K.nes.old"), FIXTURE_ROLE_GENERIC);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path("games/Nesteroids.nes"), FIXTURE_ROLE_GENERIC);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path("games/m1_P128K_C128K_S8K.nes"), FIXTURE_ROLE_GENERIC);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path(""), FIXTURE_ROLE_GENERIC);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path(NULL), FIXTURE_ROLE_GENERIC);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path("games/nesteroids.nes/"), FIXTURE_ROLE_GENERIC);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path("games/M1_P128K_C128K_S8K.nes/"), FIXTURE_ROLE_GENERIC);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path("games with spaces/nesteroids.nes"),
                 FIXTURE_ROLE_NESTEROIDS);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path("games with spaces/M1_P128K_C128K_S8K.nes"),
                 FIXTURE_ROLE_MMC1_BATTERY);
    cases++;
#if defined(_WIN32)
    CHECK_EQ_U64(fixture_role_for_path("games\\nesteroids.nes"), FIXTURE_ROLE_NESTEROIDS);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path("games\\M1_P128K_C128K_S8K.nes"), FIXTURE_ROLE_MMC1_BATTERY);
    cases++;
#else
    CHECK_EQ_U64(fixture_role_for_path("games\\nesteroids.nes"), FIXTURE_ROLE_GENERIC);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path("games\\M1_P128K_C128K_S8K.nes"), FIXTURE_ROLE_GENERIC);
    cases++;
#endif
#if defined(_WIN32)
    CHECK_EQ_U64(fixture_role_for_path("games\\nesteroids.nes\\"), FIXTURE_ROLE_GENERIC);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path("games\\M1_P128K_C128K_S8K.nes\\"), FIXTURE_ROLE_GENERIC);
    cases++;
#else
    CHECK_EQ_U64(fixture_role_for_path("games/nesteroids.nes\\"), FIXTURE_ROLE_GENERIC);
    cases++;
    CHECK_EQ_U64(fixture_role_for_path("games/M1_P128K_C128K_S8K.nes\\"), FIXTURE_ROLE_GENERIC);
    cases++;
#endif
    printf("libretro.fixture_roles: cases=%u\n", cases);
    CHECK_EQ_U64(cases, 20u);
    CHECK_DONE();
}

/* libretro.host --serialize <module> <rom>... */
static int serialize_mode(int argc, char **argv)
{
    struct ser_rom *roms;
    unsigned count, i, min_refusals = 1000u, battery_roms = 0u;

    if (argc < 4) {
        fprintf(stderr, "usage: libretro.host --serialize <module> <rom>...\n");
        return 2;
    }
    if (!module_open(argv[2])) {
        fprintf(stderr, "cannot load %s\n", argv[2]);
        return 1;
    }
    resolve_all();
    CHECK_EQ_U64(missing, 0);
    if (missing != 0) {
        module_close();
        CHECK_DONE();
    }
    count = (unsigned)(argc - 3);
    roms = (struct ser_rom *)calloc(count, sizeof roms[0]);
    CHECK(roms != NULL);
    if (roms == NULL) {
        module_close();
        CHECK_DONE();
    }
    p_set_environment(env);
    p_init();
    p_set_video_refresh(video);
    p_set_audio_sample(sample);
    p_set_audio_sample_batch(batch);
    p_set_input_poll(on_poll);
    p_set_input_state(input);

    for (i = 0u; i < count; i++) {
        roms[i].path = argv[3u + i];
        if (!ser_prepare(&roms[i])) {
            CHECK(0);
            goto done;
        }
    }
    check_serialize_no_content(&roms[0]);
    check_serialize_jam();
    for (i = 0u; i < count; i++) {
        unsigned refusals;
        check_serialize_size(&roms[i], &roms[(i + 1u) % count]);
        check_serialize_writes(&roms[i]);
        refusals = check_unserialize_rejects(&roms[i], &roms[(i + 1u) % count]);
        if (refusals < min_refusals) {
            min_refusals = refusals;
        }
        check_unserialize_before_run(&roms[i]);
        check_serialize_contexts(&roms[i]);
        if (fixture_role_for_path(roms[i].path) == FIXTURE_ROLE_MMC1_BATTERY) {
            check_unserialize_battery(&roms[i]);
            battery_roms++;
        }
    }
    /* D-20: counts that rule out a pass that did nothing. */
    CHECK(count >= SER_MIN_ROMS);
    CHECK(min_refusals >= SER_MIN_REFUSALS);
    CHECK(ser_frames_compared >= SER_MIN_FRAMES * count);
    CHECK_EQ_U64(battery_roms, 1u);
    CHECK_EQ_U64(savestate_context_queries, 0u);
    CHECK_EQ_U64(av_enable_queries, 0u);
    printf("libretro.serialize: roms=%u min_refusals=%u frames_compared=%u battery_roms=%u "
           "context_queries=%d av_queries=%d\n",
           count, min_refusals, ser_frames_compared, battery_roms, savestate_context_queries,
           av_enable_queries);
done:
    for (i = 0u; i < count; i++) {
        free(roms[i].data);
        free(roms[i].state);
    }
    free(roms);
    p_deinit();
    module_close();
    CHECK_DONE();
}

/* ---- --runahead: a run-ahead host through the module's entry points (LR-02) ---- */

/* RetroArch's single-instance run-ahead order, from docs.libretro.com/guides/runahead: run the
   real frame, save the state, run N frames ahead, show the last of them, load the state. The
   guide does not say whose audio is kept, so both streams are asserted: the real frames' audio
   equals the uninterrupted run, and the last speculative frame's audio equals the uninterrupted
   run shifted by N frames. */
#define RA_SPECULATIVE_ORDER "run, save, run ahead, show last, load"

enum rah_control {
    RAH_NORMAL,
    RAH_SKIP_LOAD,  /* never load the saved state */
    RAH_STALE_LOAD, /* load the state saved one tick earlier */
    RAH_LATE_INPUT, /* apply every input one frame late */
    RAH_AUDIO_LEAK  /* keep the speculative frame's audio in the real stream */
};

#define RAH_MIN_ROMS 7u /* D-20: a pass over fewer ROMs than this cannot succeed */

struct rah_result {
    unsigned ticks;      /* ticks run */
    unsigned states;     /* states compared with the oracle */
    unsigned mismatches; /* comparisons that differed */
    int report;          /* print the first mismatch */
    /* JAM oracle only (D-17) */
    unsigned silent_real; /* real ticks that delivered zero audio frames */
    unsigned load_skips;  /* ticks whose load-has-work check was skipped */
    unsigned cls[4];      /* window classes: no JAM in sight, JAM in the speculative frames,
                             the boundary t = J-1, a jammed state loaded */
};

static void rah_mismatch(struct rah_result *r, const char *what, uint32_t t)
{
    if (r->mismatches == 0u && r->report) {
        fprintf(stderr, "libretro.runahead: %s differs at tick %u\n", what, (unsigned)t);
    }
    r->mismatches++;
}

static void rah_set_buttons(uint16_t b)
{
    host_buttons[0] = (uint8_t)(b & 0xffu);
    host_buttons[1] = (uint8_t)(b >> 8);
}

/* The input the host applies at frame k of the script; the late-input control shifts it. */
static uint16_t rah_input(const uint16_t *in, enum rah_control ctl, uint32_t k)
{
    if (ctl == RAH_LATE_INPUT) {
        return k == 0u ? 0u : in[k - 1u];
    }
    return in[k];
}

/* The video the host showed, hashed the way the oracle hashes a frame. */
static uint64_t rah_frame_hash(void)
{
    return ra_hash_words(frame, W * H);
}

static uint64_t rah_audio_hash(void)
{
    return ra_audio_hash_stereo(batch_samples, batch_frames);
}

/* Runs T displayed ticks of the module at depth N over the script in (T + RA_MAX_DEPTH
   entries) and compares every tick with the oracle o (T + RA_MAX_DEPTH frames). two is NULL in
   real-input mode, where the speculative frames get the script's real future inputs and the
   visible frame must equal the oracle's frame t+N; otherwise it is the two-instance model's
   visible hash per tick, and the speculative frames repeat in[t] as RetroArch does. */
static void rah_run(const struct ser_rom *rom, unsigned N, const uint16_t *in, uint32_t T,
                    const struct ra_oracle *o, const uint64_t *two, enum rah_control ctl,
                    struct rah_result *res)
{
    size_t ss;
    unsigned char *r_now = NULL, *r_prev = NULL, *pre = NULL;
    const uint32_t J = o->jam_frame;
    const int jam = J != RA_NO_JAM;

    if (!ser_load(rom->data, rom->size)) {
        CHECK(0);
        return;
    }
    ss = p_serialize_size();
    CHECK_EQ_U64(ss, o->state_size);
    r_now = (unsigned char *)malloc(ss);
    r_prev = (unsigned char *)malloc(ss);
    pre = (unsigned char *)malloc(ss);
    if (r_now == NULL || r_prev == NULL || pre == NULL || ss != o->state_size) {
        CHECK(0);
        goto out;
    }
    CHECK(p_serialize(r_prev, ss));
    for (uint32_t t = 0u; t < T; t++) {
        uint64_t real_video, real_audio, spec_audio = 0u, visible;

        /* The real frame: its video is dropped, its audio kept. */
        rah_set_buttons(rah_input(in, ctl, t));
        batch_frames = 0u;
        p_run();
        real_video = rah_frame_hash();
        real_audio = rah_audio_hash();
        if (jam && batch_frames == 0u) {
            res->silent_real++;
        }
        if (real_video != o->video[t]) {
            rah_mismatch(res, "real video", t);
        }
        CHECK(p_serialize(r_now, ss));
        res->states++;
        if (memcmp(r_now, o->state + (size_t)t * ss, ss) != 0) {
            rah_mismatch(res, "saved state", t);
        }

        /* N speculative frames: their audio is dropped; the last one's video is shown. */
        visible = real_video;
        for (unsigned j = 1u; j <= N; j++) {
            rah_set_buttons(rah_input(in, ctl, two != NULL ? t : t + j));
            batch_frames = 0u;
            p_run();
        }
        if (N > 0u) {
            visible = rah_frame_hash();
            spec_audio = rah_audio_hash();
        }
        if (ctl == RAH_AUDIO_LEAK) {
            real_audio = ra_fnv_bytes(real_audio, &spec_audio, sizeof spec_audio);
        }
        if (real_audio != o->audio[t]) {
            rah_mismatch(res, "real audio", t);
        }
        if (visible != (two != NULL ? two[t] : o->video[t + N])) {
            rah_mismatch(res, "visible video", t);
        }
        if (N > 0u && two == NULL && spec_audio != o->audio[t + N]) {
            rah_mismatch(res, "speculative audio", t);
        }
        /* D-17: from the tick whose window reaches the JAM on, the picture is the last converted
           frame, never black. */
        if (jam && ctl == RAH_NORMAL && t + N >= J && visible != o->rec[J - 1u].hash) {
            rah_mismatch(res, "frozen picture", t);
        }
        if (jam && ctl == RAH_NORMAL) {
            res->cls[0] += t + N < J ? 1u : 0u;
            res->cls[1] += t < J && J <= t + N ? 1u : 0u;
            res->cls[2] += t + 1u == J ? 1u : 0u;
            res->cls[3] += t >= J ? 1u : 0u;
        }

        /* The load has work to do: the state after the speculative frames is not R(t+1). Once the
           real frame has JAMmed, the latched state is all there is, and the check is skipped. */
        if (N > 0u && ctl == RAH_NORMAL) {
            CHECK(p_serialize(pre, ss));
            if (jam && !o->rec[t].valid) {
                res->load_skips++;
            } else {
                CHECK(memcmp(pre, r_now, ss) != 0);
            }
        }
        if (ctl == RAH_STALE_LOAD) {
            CHECK(p_unserialize(r_prev, ss));
        } else if (ctl != RAH_SKIP_LOAD) {
            CHECK(p_unserialize(r_now, ss));
        }
        memcpy(r_prev, r_now, ss);
        res->ticks++;
    }
    /* The final state is the uninterrupted run's. */
    CHECK(p_serialize(r_now, ss));
    res->states++;
    if (memcmp(r_now, o->state + (size_t)(T - 1u) * ss, ss) != 0) {
        rah_mismatch(res, "final state", T);
    }
out:
    free(r_now);
    free(r_prev);
    free(pre);
    p_unload_game();
}

/* Tallies over the whole run. expected_ticks is what the calls asked for, so a pass that
   returned early cannot reach the counts. */
struct rah_totals {
    unsigned roms, ticks, states, repeat_ticks, expected_ticks, expected_repeat;
    unsigned controls_tripped;
    unsigned jam_controls_tripped;
    unsigned jam_cls[RA_MAX_DEPTH + 1u][4]; /* window classes per depth, JAM image only */
    uint32_t jam_frame;                     /* J, read from the oracle */
};

static void rah_add(struct rah_totals *tot, const struct rah_result *res, int repeat)
{
    if (repeat) {
        tot->repeat_ticks += res->ticks;
    } else {
        tot->ticks += res->ticks;
    }
    tot->states += res->states;
}

/* D-17: exactly T-J real ticks are silent, and the load-has-work check is skipped exactly T-J
   times (at depths above 0). */
static void rah_check_jam_counts(const struct rah_result *res, uint32_t J, uint32_t T, unsigned N)
{
    CHECK_EQ_U64(res->silent_real, T - J);
    if (N > 0u) {
        CHECK_EQ_U64(res->load_skips, T - J);
    }
}

/* One script over one ROM, at the depths in depth_mask (bit N is depth N; depth 0 is the
   uninterrupted run itself and must match trivially). Each depth runs the module in real-input
   mode against the oracle; with repeat set, depths above 0 also run in repeat mode against the
   library's two-instance model. */
static void rah_matrix(const struct ser_rom *rom, uint32_t T, unsigned script, unsigned depth_mask,
                       int repeat, int jam, struct rah_totals *tot)
{
    uint16_t *in = ra_script_make(script, script == 1u ? RA_SEED_1 : RA_SEED_2, T + RA_MAX_DEPTH);
    uint64_t *visible = (uint64_t *)malloc((size_t)T * sizeof *visible);
    struct ra_oracle oracle;

    CHECK(in != NULL && visible != NULL);
    if (in == NULL || visible == NULL ||
        !ra_run_uninterrupted_ex(rom->data, rom->size, in, T + RA_MAX_DEPTH, &oracle, jam)) {
        CHECK(0);
        free(in);
        free(visible);
        return;
    }
    if (jam) {
        /* D-17: J is whatever the oracle's first STOP_JAM is, within bounds that keep every
           window class inside the run. */
        tot->jam_frame = oracle.jam_frame;
        CHECK(oracle.jam_frame != RA_NO_JAM);
        CHECK(oracle.jam_frame >= RA_MAX_DEPTH + 2u && oracle.jam_frame <= T - RA_MAX_DEPTH - 2u);
        if (oracle.jam_frame == RA_NO_JAM || oracle.jam_frame < RA_MAX_DEPTH + 2u ||
            oracle.jam_frame > T - RA_MAX_DEPTH - 2u) {
            ra_oracle_free(&oracle);
            free(in);
            free(visible);
            return;
        }
    }
    for (unsigned N = 0u; N <= RA_MAX_DEPTH; N++) {
        struct rah_result res;
        struct ra_oracle model;

        if ((depth_mask & (1u << N)) == 0u) {
            continue;
        }
        memset(&res, 0, sizeof res);
        res.report = 1;
        rah_run(rom, N, in, T, &oracle, NULL, RAH_NORMAL, &res);
        if (res.mismatches != 0u) {
            fprintf(stderr, "libretro.runahead: %s script %u depth %u: %u mismatches\n", rom->path,
                    script, N, res.mismatches);
        }
        CHECK_EQ_U64(res.mismatches, 0u);
        if (jam) {
            rah_check_jam_counts(&res, oracle.jam_frame, T, N);
        }
        if (N == 0u) {
            continue;
        }
        if (jam) {
            for (unsigned c = 0u; c < 4u; c++) {
                tot->jam_cls[N][c] += res.cls[c];
            }
        }
        rah_add(tot, &res, 0);
        tot->expected_ticks += T;
        if (!repeat) {
            continue;
        }
        memset(&res, 0, sizeof res);
        res.report = 1;
        if (ra_run_two_instance_ex(rom->data, rom->size, in, T, N, &model, visible, jam)) {
            rah_run(rom, N, in, T, &oracle, visible, RAH_NORMAL, &res);
            ra_oracle_free(&model);
        } else {
            CHECK(0);
        }
        if (res.mismatches != 0u) {
            fprintf(stderr, "libretro.runahead: %s script %u depth %u repeat: %u mismatches\n",
                    rom->path, script, N, res.mismatches);
        }
        CHECK_EQ_U64(res.mismatches, 0u);
        if (jam) {
            rah_check_jam_counts(&res, oracle.jam_frame, T, N);
            for (unsigned c = 0u; c < 4u; c++) {
                tot->jam_cls[N][c] += res.cls[c];
            }
        }
        rah_add(tot, &res, 1);
        tot->expected_repeat += T;
    }
    ra_oracle_free(&oracle);
    free(visible);
    free(in);
}

/* D-16: four broken hosts run through the same comparator and each must differ from the
   uninterrupted run. A control that matches would mean the comparator proves nothing. */
static unsigned rah_controls(const struct ser_rom *rom, uint32_t T, int jam)
{
    static const struct {
        enum rah_control ctl;
        const char *name;
    } controls[] = {{RAH_SKIP_LOAD, "skip-load"},
                    {RAH_STALE_LOAD, "stale-load"},
                    {RAH_LATE_INPUT, "late-input"},
                    {RAH_AUDIO_LEAK, "audio-leak"}};
    unsigned tripped = 0u;
    uint16_t *in = ra_script_make(1u, RA_SEED_1, T + RA_MAX_DEPTH);
    struct ra_oracle oracle;

    CHECK(in != NULL);
    if (in == NULL ||
        !ra_run_uninterrupted_ex(rom->data, rom->size, in, T + RA_MAX_DEPTH, &oracle, jam)) {
        CHECK(0);
        free(in);
        return 0u;
    }
    if (jam) {
        /* D-17 black-oracle control: an oracle that expects black from the JAM on. The host
           shows the frozen picture, so the comparison must differ. */
        static const uint32_t black[W * H];
        struct rah_result res;
        const uint64_t black_hash = ra_hash_words(black, W * H);

        CHECK(oracle.jam_frame != RA_NO_JAM &&
              black_hash != oracle.rec[oracle.jam_frame - 1u].hash);
        for (uint32_t i = oracle.jam_frame; i < oracle.frames && oracle.jam_frame != RA_NO_JAM;
             i++) {
            oracle.video[i] = black_hash;
        }
        memset(&res, 0, sizeof res);
        rah_run(rom, 2u, in, T, &oracle, NULL, RAH_NORMAL, &res);
        if (res.mismatches == 0u) {
            fprintf(stderr,
                    "libretro.runahead: control black-oracle matched the uninterrupted run\n");
            CHECK(res.mismatches != 0u);
        } else {
            tripped++;
            printf("libretro.runahead: control black-oracle tripped (%u mismatches)\n",
                   res.mismatches);
        }
        /* Restore the true oracle for the four controls below. */
        ra_oracle_free(&oracle);
        if (!ra_run_uninterrupted_ex(rom->data, rom->size, in, T + RA_MAX_DEPTH, &oracle, jam)) {
            CHECK(0);
            free(in);
            return tripped;
        }
    }
    for (unsigned i = 0u; i < sizeof controls / sizeof controls[0]; i++) {
        struct rah_result res;
        memset(&res, 0, sizeof res);
        rah_run(rom, 2u, in, T, &oracle, NULL, controls[i].ctl, &res);
        if (res.mismatches == 0u) {
            fprintf(stderr, "libretro.runahead: control %s matched the uninterrupted run\n",
                    controls[i].name);
            CHECK(res.mismatches != 0u);
        } else {
            tripped++;
            printf("libretro.runahead: control %s tripped (%u mismatches)\n", controls[i].name,
                   res.mismatches);
        }
    }
    ra_oracle_free(&oracle);
    free(in);
    return tripped;
}

/* D-17: the synthetic JAM image. NROM, input ignored, rendering off, pulse 1 playing. Each NMI
   counts up and writes counter + $10 to $3F00, so every frame before the JAM has its own
   backdrop; the thirteenth NMI executes the JAM opcode $02. The VRAM address is put back to
   $3F00 after the write so the displayed backdrop is that entry. Returns the image size. */
static size_t rah_jam_image(uint8_t *out, size_t cap)
{
    static const uint8_t reset_code[] = {
        0x78u,                                                     /* SEI */
        0xa9u, 0x00u, 0x85u, 0x00u,                                /* LDA #0, STA $00 */
        0xa9u, 0x01u, 0x8du, 0x15u, 0x40u,                         /* $4015: pulse 1 on */
        0xa9u, 0xbfu, 0x8du, 0x00u, 0x40u,                         /* $4000: duty 2, volume F */
        0xa9u, 0xfdu, 0x8du, 0x02u, 0x40u,                         /* $4002: timer low */
        0xa9u, 0x08u, 0x8du, 0x03u, 0x40u,                         /* $4003: timer high */
        0xa9u, 0x80u, 0x8du, 0x00u, 0x20u,                         /* $2000: NMI on */
        0x4cu, 0x1eu, 0x80u};                                      /* JMP self */
    static const uint8_t nmi_code[] = {0xe6u, 0x00u,               /* INC $00 */
                                       0xa5u, 0x00u,               /* LDA $00 */
                                       0xc9u, 0x0du,               /* CMP #13 */
                                       0xd0u, 0x01u,               /* BNE +1 */
                                       0x02u,                      /* JAM */
                                       0x18u, 0x69u, 0x10u, 0xaau, /* CLC, ADC #$10, TAX */
                                       0xa9u, 0x3fu, 0x8du, 0x06u, 0x20u, 0xa9u, 0x00u,
                                       0x8du, 0x06u, 0x20u, 0x8eu, 0x07u, 0x20u, /* STX $2007 */
                                       0xa9u, 0x3fu, 0x8du, 0x06u, 0x20u, 0xa9u, 0x00u,
                                       0x8du, 0x06u, 0x20u, 0x40u}; /* RTI */
    static const struct ines_segment segments[] = {{0x30u, nmi_code, sizeof nmi_code}};
    struct ines_spec spec;

    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = 1u;
    spec.prg_code = reset_code;
    spec.prg_code_len = sizeof reset_code;
    spec.segments = segments;
    spec.segment_count = 1u;
    spec.nmi_vector = 0x8030u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    return ines_build(out, cap, &spec);
}

/* libretro.host --runahead <module> [--short] <rom>... */
static int runahead_mode(int argc, char **argv)
{
    struct rah_totals tot;
    uint32_t T = 180u;
    int first = 3, a;
    unsigned depth, covered_depths = 0u, control_roms = 0u;
    struct ser_rom *roms;
    unsigned count = 0u, stress_images = 0u, jam_images = 0u;
    int stress = 0;
    uint16_t *s1, *s2;

    if (argc > first && strcmp(argv[first], "--short") == 0) {
        T = 30u;
        first++;
    }
    if (argc > first && strcmp(argv[first], "--stress") == 0) {
        stress = 1;
        first++;
    }
    if (argc <= first) {
        fprintf(stderr, "usage: libretro.host --runahead <module> [--short] [--stress] <rom>...\n");
        return 2;
    }
    if (!module_open(argv[2])) {
        fprintf(stderr, "cannot load %s\n", argv[2]);
        return 1;
    }
    resolve_all();
    CHECK_EQ_U64(missing, 0);
    if (missing != 0) {
        module_close();
        CHECK_DONE();
    }
    p_set_environment(env);
    p_init();
    p_set_video_refresh(video);
    p_set_audio_sample(sample);
    p_set_audio_sample_batch(batch);
    p_set_input_poll(on_poll);
    p_set_input_state(input);

    /* D-13: the two scripts together put a change at every alignment of every depth. */
    s1 = ra_script_make(1u, RA_SEED_1, T + RA_MAX_DEPTH);
    s2 = ra_script_make(2u, RA_SEED_2, T + RA_MAX_DEPTH);
    CHECK(s1 != NULL && s2 != NULL);
    for (depth = 1u; depth <= RA_MAX_DEPTH && s1 != NULL && s2 != NULL; depth++) {
        int covered = ra_alignment_covered(s1, s2, T, depth);
        CHECK(covered);
        covered_depths += covered ? 1u : 0u;
    }
    free(s1);
    free(s2);

    memset(&tot, 0, sizeof tot);
    roms = (struct ser_rom *)calloc((size_t)(argc - first), sizeof roms[0]);
    CHECK(roms != NULL);
    for (a = first; a < argc && roms != NULL; a++) {
        struct ser_rom *r = &roms[count];
        r->path = argv[a];
        CHECK(read_file(r->path, &r->data, &r->size));
        if (r->data == NULL) {
            continue;
        }
        count++;
        if (fixture_role_for_path(r->path) == FIXTURE_ROLE_NESTEROIDS) {
            /* The full matrix: both scripts, depths 0 to 3, repeat mode on script 1, and the
               four controls. */
            rah_matrix(r, T, 1u, 0xfu, 1, 0, &tot);
            rah_matrix(r, T, 2u, 0xeu, 0, 0, &tot);
            tot.controls_tripped += rah_controls(r, T, 0);
            control_roms++;
        } else {
            /* One script per board (alternating) at the claimed bounds, depths 1 and 3. */
            rah_matrix(r, T, 1u + (count & 1u), 0xau, 0, 0, &tot);
        }
        tot.roms++;
    }
    /* D-17: the in-memory JAM image, all depths in both modes, plus its controls. It is not a
       file ROM, so RAH_MIN_ROMS does not count it. */
    {
        static uint8_t jam_image[16u + INES_PRG_BANK + INES_CHR_BANK];
        struct ser_rom jr;

        memset(&jr, 0, sizeof jr);
        jr.path = "jam-image";
        jr.data = jam_image;
        jr.size = rah_jam_image(jam_image, sizeof jam_image);
        CHECK(jr.size != 0u);
        if (jr.size != 0u) {
            rah_matrix(&jr, T, 1u, 0xfu, 1, 1, &tot);
            tot.jam_controls_tripped += rah_controls(&jr, T, 1);
            jam_images++;
        }
    }
    /* The Phase 12 stress images keep DMA and interrupts in flight across frame boundaries. */
    for (unsigned m = 0u; stress && m < 2u; m++) {
        static const uint16_t mappers[2] = {0u, 4u};
        static const char *const names[2] = {"stress-nrom", "stress-mmc3"};
        static uint8_t image[16u + 2u * INES_PRG_BANK + INES_CHR_BANK];
        struct ser_rom sr;

        memset(&sr, 0, sizeof sr);
        sr.path = names[m];
        sr.size = st_stress_image(image, sizeof image, mappers[m]);
        CHECK(sr.size != 0u);
        sr.data = (unsigned char *)malloc(sr.size != 0u ? sr.size : 1u);
        if (sr.data != NULL && sr.size != 0u) {
            memcpy(sr.data, image, sr.size);
            rah_matrix(&sr, T, 1u + m, 0xau, 0, 0, &tot);
            stress_images++;
        }
        free(sr.data);
    }
    /* D-20: counts that rule out a pass that did nothing. */
    CHECK(count >= RAH_MIN_ROMS);
    CHECK_EQ_U64(stress_images, stress ? 2u : 0u);
    CHECK_EQ_U64(control_roms, 1u);
    CHECK_EQ_U64(tot.controls_tripped, 4u);
    CHECK_EQ_U64(jam_images, 1u);
    CHECK_EQ_U64(tot.jam_controls_tripped, 5u); /* four controls and the black oracle */
    for (depth = 1u; depth <= RA_MAX_DEPTH; depth++) {
        for (unsigned c = 0u; c < 4u; c++) {
            CHECK(tot.jam_cls[depth][c] != 0u);
        }
    }
    CHECK_EQ_U64(covered_depths, RA_MAX_DEPTH);
    CHECK_EQ_U64(tot.ticks, tot.expected_ticks);
    CHECK_EQ_U64(tot.repeat_ticks, tot.expected_repeat);
    CHECK(tot.ticks >= (RA_MAX_DEPTH * 2u + 2u * (count - 1u + stress_images)) * T);
    CHECK(tot.repeat_ticks >= RA_MAX_DEPTH * T);
    CHECK(tot.states >= tot.ticks + tot.repeat_ticks);
    printf("libretro.runahead: order=\"%s\" roms=%u stress=%u ticks=%u repeat_ticks=%u states=%u "
           "controls_tripped=%u jam_images=%u jam_frame=%u jam_controls_tripped=%u\n",
           RA_SPECULATIVE_ORDER, count, stress_images, tot.ticks, tot.repeat_ticks, tot.states,
           tot.controls_tripped, jam_images, (unsigned)tot.jam_frame, tot.jam_controls_tripped);
    for (depth = 1u; depth <= RA_MAX_DEPTH; depth++) {
        printf("libretro.runahead: jam depth %u windows: no_jam=%u in_speculative=%u boundary=%u "
               "jammed_loaded=%u\n",
               depth, tot.jam_cls[depth][0], tot.jam_cls[depth][1], tot.jam_cls[depth][2],
               tot.jam_cls[depth][3]);
    }
    for (a = 0; a < (int)count; a++) {
        free(roms[a].data);
    }
    free(roms);
    p_deinit();
    module_close();
    CHECK_DONE();
}

/* Test-only host mode: record the exact argv the CMake fixture wrapper delivers. */
static int fixture_argv_capture(int argc, char **argv, const char *path)
{
    FILE *output = fopen(path, "wb");
    int index;
    if (output == NULL) {
        fprintf(stderr, "cannot create fixture argv capture\n");
        return 1;
    }
    if (fprintf(output, "%d\n", argc - 1) < 0) {
        fclose(output);
        return 1;
    }
    for (index = 1; index < argc; index++) {
        size_t length = strlen(argv[index]);
        if (fprintf(output, "%lu:", (unsigned long)length) < 0 ||
            fwrite(argv[index], 1, length, output) != length || fputc('\n', output) == EOF) {
            fclose(output);
            return 1;
        }
    }
    if (fclose(output) != 0) {
        fprintf(stderr, "cannot finish fixture argv capture\n");
        return 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    struct retro_system_info sys;
    struct retro_system_av_info av;
    struct retro_game_info dummy;
    static unsigned char dummy_bytes[16u + 16384u + 8192u];
    static unsigned char jam_base[16u + 16384u + 8192u];
    static uint32_t first_content_frame[W * H];
    double fps_diff;
    const char *fixture_argv_path = getenv("NESTURBATOR_FIXTURE_ARGV_CAPTURE");

    if (fixture_argv_path != NULL) {
        return fixture_argv_capture(argc, argv, fixture_argv_path);
    }
    if (argc == 2 && strcmp(argv[1], "--fixture-role-selftest") == 0) {
        return fixture_role_selftest();
    }
    if (argc >= 6 && strcmp(argv[1], "--rom-parity") == 0) {
        return rom_parity(argc, argv);
    }
    if (argc >= 2 && strcmp(argv[1], "--serialize") == 0) {
        return serialize_mode(argc, argv);
    }
    if (argc >= 2 && strcmp(argv[1], "--runahead") == 0) {
        return runahead_mode(argc, argv);
    }
    if (argc != 6) {
        fprintf(stderr, "usage: libretro.host --rom-parity <module> <rom> <frames> <ppm> "
                        "[--movie FILE] [--testcard PPM] [--sav-in FILE] [--sav-expect FILE]\n"
                        "       libretro.host --fixture-role-selftest\n"
                        "       libretro.host --serialize <module> <rom>...\n"
                        "       libretro.host --runahead <module> [--short] <rom>...\n"
                        "       libretro.host <module> <testframe.ppm> <content.ppm> <runner> "
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
    memcpy(jam_base, dummy_bytes, sizeof jam_base);
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
    check_lifecycle_reuse();
    check_unload_then_no_content(argv[2]);
    check_unload_edges();
    check_jam_presentation(jam_base, sizeof jam_base);
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
