/* FRAME-04: loads the built libretro module at run time and calls it in the
 * order a frontend such as RetroArch does (01-RESEARCH Pattern 3), then
 * compares the frame it receives with the runner's P6 image.
 *
 *   libretro.host <module> <frame1.ppm>
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
#include <string.h>

#include "../check.h"
#include "libretro.h"

#define W 256u
#define H 240u

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
static int sample_calls;
static int batch_calls;
static size_t batch_frames;
static int batch_nonzero;

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
    (void)left;
    (void)right;
    sample_calls++;
}

static size_t RETRO_CALLCONV batch(const int16_t *data, size_t frames)
{
    size_t i;
    batch_calls++;
    batch_frames = frames;
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
    (void)port;
    (void)device;
    (void)index;
    (void)id;
    return 0;
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

int main(int argc, char **argv)
{
    struct retro_system_info sys;
    struct retro_system_av_info av;
    struct retro_game_info dummy;
    static const unsigned char dummy_bytes[16] = {0};
    double fps_diff;

    if (argc != 3) {
        fprintf(stderr, "usage: libretro.host <module> <frame1.ppm>\n");
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

    /* D-10: content is refused in Phase 1. */
    memset(&dummy, 0, sizeof dummy);
    dummy.data = dummy_bytes;
    dummy.size = sizeof dummy_bytes;
    CHECK(!p_load_game(&dummy));

    /* No content: the test card, in XRGB8888 (L861-865). */
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
    CHECK_EQ_HEX(frame[18 * W + 100] & 0xFFFFFFu, 0xC23400); /* native $16 */
    CHECK_EQ_HEX(frame[46 * W + 100] & 0xFFFFFFu, 0xC52700); /* native $56 */
    CHECK_EQ_HEX(frame[10 * W + 213] & 0xFFFFFFu, 0x000000); /* native $0D */

    /* FRAME-04: the same frame the runner writes. */
    compare_with_ppm(argv[2]);

    p_unload_game();
    p_deinit();
    module_close();
    CHECK_DONE();
}
