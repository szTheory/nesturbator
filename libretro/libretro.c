/* The libretro adapter: the 25 entry points of libretro.h, written as a thin
 * client of nesturbator.h. "Lnnn" is a line of the vendored libretro.h
 * (RetroArch v1.22.2, see PROVENANCE.md).
 *
 * Without content the core shows its built-in test card; mapper-0 content is
 * copied into the instance before the frontend's buffer expires. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "convert.h"
#include "libretro.h"
#include "nesturbator.h"

#define NT_WIDTH 256u
#define NT_HEIGHT 240u
#define NT_PIXELS (NT_WIDTH * NT_HEIGHT)
/* A frame yields 798 or 799 samples (nesturbator.h); 1024 leaves room. */
#define NT_AUDIO_CAPACITY 1024u

#define NT_STR_(x) #x
#define NT_STR(x) NT_STR_(x)
#define NT_VERSION                                                                                 \
    NT_STR(NESTURBATOR_VERSION_MAJOR)                                                              \
    "." NT_STR(NESTURBATOR_VERSION_MINOR) "." NT_STR(NESTURBATOR_VERSION_PATCH)

/* All adapter state. A frontend may keep the module loaded between sessions,
   so retro_deinit returns every one of these to its initial value (L7594). */
static retro_environment_t env_cb;
static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static nesturbator *inst;
static uint32_t palette[512];
static uint16_t video[NT_PIXELS];
static uint32_t xrgb[NT_PIXELS];
static int16_t mono[NT_AUDIO_CAPACITY];
static int16_t stereo[NT_AUDIO_CAPACITY * 2u];

/* L7511: called before retro_init. Declaring no-game support (18, L1051)
   makes the frontend call retro_load_game(NULL) (L1040-1051). */
void retro_set_environment(retro_environment_t cb)
{
    bool no_game = true;
    env_cb = cb;
    if (cb != NULL) {
        (void)cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_game);
    }
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
    video_cb = cb;
}
/* L7453: the frontend may provide either audio callback; retro_run prefers
   the batch callback when both are registered. */
void retro_set_audio_sample(retro_audio_sample_t cb)
{
    audio_cb = cb;
}
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    audio_batch_cb = cb;
}
void retro_set_input_poll(retro_input_poll_t cb)
{
    input_poll_cb = cb;
}
void retro_set_input_state(retro_input_state_t cb)
{
    input_state_cb = cb;
}

void retro_init(void)
{
}

/* L7594: reset every global; the module may be used again. */
void retro_deinit(void)
{
    nesturbator_destroy(inst);
    inst = NULL;
    env_cb = NULL;
    video_cb = NULL;
    audio_cb = NULL;
    audio_batch_cb = NULL;
    input_poll_cb = NULL;
    input_state_cb = NULL;
    memset(palette, 0, sizeof palette);
    memset(video, 0, sizeof video);
    memset(xrgb, 0, sizeof xrgb);
    memset(mono, 0, sizeof mono);
    memset(stereo, 0, sizeof stereo);
}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

/* L7627: static strings. need_fullpath false lets the frontend load, unzip
   and patch the content (L6008). */
void retro_get_system_info(struct retro_system_info *info)
{
    if (info == NULL) {
        return;
    }
    memset(info, 0, sizeof *info);
    info->library_name = "nesturbator";
    info->library_version = NT_VERSION;
    info->valid_extensions = "nes";
    info->need_fullpath = false;
    info->block_extract = false;
}

/* L7642, L6247-6305: geometry and timing. The rates are exact fractions in
   the core; dividing them here is the only floating point, and it stays in
   the adapter. aspect_ratio 0 means width / height. */
void retro_get_system_av_info(struct retro_system_av_info *info)
{
    nesturbator_info ni;
    if (info == NULL) {
        return;
    }
    memset(info, 0, sizeof *info);
    memset(&ni, 0, sizeof ni);
    ni.size = (uint32_t)sizeof ni;
    nesturbator_get_info(inst, &ni);
    info->geometry.base_width = NT_WIDTH;
    info->geometry.base_height = NT_HEIGHT;
    info->geometry.max_width = NT_WIDTH;
    info->geometry.max_height = NT_HEIGHT;
    info->geometry.aspect_ratio = 0.0f;
    if (ni.fps_den != 0u) {
        info->timing.fps = (double)ni.fps_num / (double)ni.fps_den;
    }
    if (ni.sample_rate_den != 0u) {
        info->timing.sample_rate = (double)ni.sample_rate_num / (double)ni.sample_rate_den;
    }
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
    (void)port;
    (void)device;
}

/* The test card has nothing to reset. */
void retro_reset(void)
{
}

/* L7694: one frame. Input is polled once (L7685), the video callback is
   called once with XRGB8888 rows of 1024 bytes, and mono samples are
   duplicated into left/right pairs. Prefer the batch callback (L7465); use
   the per-sample callback only when no batch callback is registered (L7453).
   A failed frame sends no video or audio. */
void retro_run(void)
{
    nesturbator_frame io;
    nesturbator_input input;
    uint32_t i;

    if (input_poll_cb != NULL) {
        input_poll_cb();
    }
    if (inst == NULL) {
        return;
    }
    memset(&input, 0, sizeof input);
    input.size = (uint32_t)sizeof input;
    for (unsigned port = 0; port < 2u; port++) {
        static const unsigned ids[8] = {
            RETRO_DEVICE_ID_JOYPAD_A,      RETRO_DEVICE_ID_JOYPAD_B,
            RETRO_DEVICE_ID_JOYPAD_SELECT, RETRO_DEVICE_ID_JOYPAD_START,
            RETRO_DEVICE_ID_JOYPAD_UP,     RETRO_DEVICE_ID_JOYPAD_DOWN,
            RETRO_DEVICE_ID_JOYPAD_LEFT,   RETRO_DEVICE_ID_JOYPAD_RIGHT};
        for (unsigned button = 0; button < 8u; button++) {
            if (input_state_cb != NULL &&
                input_state_cb(port, RETRO_DEVICE_JOYPAD, 0u, ids[button]) != 0) {
                input.buttons[port] |= (uint8_t)(1u << button);
            }
        }
    }
    (void)nesturbator_set_input(inst, &input);
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = video;
    io.video_pitch = NT_WIDTH;
    io.audio = mono;
    io.audio_capacity = NT_AUDIO_CAPACITY;
    if (nesturbator_run_frame(inst, &io) != NESTURBATOR_OK) {
        return;
    }
    /* D-15: the runner's conversion loop. */
    nesturbator_host_convert(palette, video, NT_WIDTH, xrgb, NT_WIDTH);
    if (video_cb != NULL) {
        video_cb(xrgb, NT_WIDTH, NT_HEIGHT, (size_t)NT_WIDTH * sizeof xrgb[0]);
    }
    for (i = 0; i < io.audio_count; i++) {
        stereo[2u * i] = mono[i];
        stereo[2u * i + 1u] = mono[i];
    }
    if (audio_batch_cb != NULL) {
        (void)audio_batch_cb(stereo, io.audio_count);
    } else if (audio_cb != NULL) {
        for (i = 0; i < io.audio_count; i++) {
            audio_cb(stereo[2u * i], stereo[2u * i + 1u]);
        }
    }
}

/* L7707: size 0 means no save states. */
size_t retro_serialize_size(void)
{
    return 0;
}

bool retro_serialize(void *data, size_t len)
{
    (void)data;
    (void)len;
    return false;
}

bool retro_unserialize(const void *data, size_t len)
{
    (void)data;
    (void)len;
    return false;
}

void retro_cheat_reset(void)
{
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
    (void)index;
    (void)enabled;
    (void)code;
}

/* L7761. A NULL game starts the test card; non-NULL content is loaded through
   the public NROM API. XRGB8888 is chosen here (L861); the
   default 0RGB1555 is deprecated (L5620-5647). */
bool retro_load_game(const struct retro_game_info *game)
{
    enum retro_pixel_format format = RETRO_PIXEL_FORMAT_XRGB8888;
    nesturbator_config cfg;

    if (inst != NULL || (game != NULL && (game->data == NULL || game->size == 0u))) {
        return false;
    }
    if (env_cb == NULL || !env_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &format)) {
        return false;
    }
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    if (nesturbator_create(&cfg, &inst) != NESTURBATOR_OK) {
        inst = NULL;
        return false;
    }
    if (game != NULL &&
        nesturbator_load_cartridge(inst, game->data, game->size) != NESTURBATOR_OK) {
        nesturbator_destroy(inst);
        inst = NULL;
        return false;
    }
    nesturbator_get_palette(inst, palette, 512u);
    return true;
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info,
                             size_t num_info)
{
    (void)game_type;
    (void)info;
    (void)num_info;
    return false;
}

void retro_unload_game(void)
{
    nesturbator_destroy(inst);
    inst = NULL;
}

/* L7812: 0 is NTSC. */
unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}

/* L7826: NULL and 0 are allowed (L498); no memory is exposed yet. */
void *retro_get_memory_data(unsigned id)
{
    (void)id;
    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    (void)id;
    return 0;
}
