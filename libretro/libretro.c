/* The libretro adapter: the 25 entry points of libretro.h, written as a thin
 * client of nesturbator.h. "Lnnn" is a line of the vendored libretro.h
 * (RetroArch v1.22.2, see PROVENANCE.md).
 *
 * One instance lives from retro_init to retro_deinit; retro_load_game and
 * retro_unload_game call nesturbator_unload_cartridge.
 *
 * Without content the core shows its built-in test card; cartridge content is
 * copied into the instance before the frontend's buffer expires; nesturbator.h
 * lists the supported boards. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
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
static uint8_t *save_data;
static size_t save_size;
static uint32_t palette[512];
static uint16_t video[NT_PIXELS];
static uint32_t xrgb[NT_PIXELS];
/* Never written: the black frame sent when the cache does not apply, so that showing black does
   not destroy the cached picture (a later load may return to its timeline). */
static const uint32_t blank[NT_PIXELS];
static bool completed_frame_valid;
static int16_t mono[NT_AUDIO_CAPACITY];
static int16_t stereo[NT_AUDIO_CAPACITY * 2u];

/* The presentation cache and its identity. JAM leaves the native frame buffers unspecified, so
   only this adapter-owned converted frame may be repeated, and it is shown only when it belongs
   to the running timeline. None of this is written into the native save state.

   D-16 narrows Phase 18 D-10, which dropped the cache on every successful load. Run-ahead loads a
   state on every tick, so that rule showed black instead of a frozen picture once the real
   timeline reached a JAM. The cache now carries a tag: the frame_number of the frame that filled
   it (cached_frame_number) and a timeline generation (cached_timeline). It is shown on STOP_JAM
   only when the tag equals the adapter-tracked frame_number (current_frame_number, from
   io.frame_number after each run and from state bytes 32-39 after each load) and the instance is
   on the cache's generation (timeline).

   The D-16 amendment of 2026-10-25 adds the generation, because a frame number alone cannot tell
   a state from a diverged timeline (rewind, or another save slot, with different input) from the
   one the cache was drawn on. A successful load keeps the generation only when the loaded state's
   CRC (header bytes 28-31) equals the CRC of the state the cached frame produced
   (cached_state_crc) or of the jammed state reached from it (jam_state_crc); any other state
   moves to a new generation. The CRC is recorded when a frame is converted, not only in
   retro_serialize: with RetroArch's second-instance run-ahead the instance that shows the picture
   never serializes. The cost is one nesturbator_save_state into scratch per converted frame.

   Accepted residual (D-16 amendment): when a mispredicted input leads the real timeline into a
   JAM on the first real frame after a load, the frozen real frame was never retained, so the
   cache belongs to another generation and STOP_JAM shows black. Black is the safe direction:
   the adapter never shows another timeline's picture. */
static uint64_t cached_frame_number;
static uint64_t current_frame_number;
static uint32_t timeline;
static uint32_t cached_timeline;
static uint32_t cached_state_crc;
static uint32_t jam_state_crc;
static bool jam_state_crc_valid;
static uint8_t *scratch;
static size_t scratch_size;

#define NT_STATE_CRC_OFFSET 28u
#define NT_STATE_FRAME_OFFSET 32u

static uint32_t read_le32(const uint8_t *b)
{
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

static uint64_t read_le64(const uint8_t *b)
{
    return (uint64_t)read_le32(b) | ((uint64_t)read_le32(b + 4u) << 32);
}

/* Reset, load_game, unload, init and deinit start a new session. */
static void invalidate_presentation(void)
{
    completed_frame_valid = false;
    cached_frame_number = 0u;
    current_frame_number = 0u;
    timeline = 0u;
    cached_timeline = 0u;
    cached_state_crc = 0u;
    jam_state_crc = 0u;
    jam_state_crc_valid = false;
}

static void free_scratch(void)
{
    free(scratch);
    scratch = NULL;
    scratch_size = 0u;
}

static bool cache_is_current(void)
{
    return completed_frame_valid && cached_frame_number == current_frame_number &&
           cached_timeline == timeline;
}

/* Saves the running state into scratch and returns its CRC field. False without scratch (the
   test card has no state) or when the save fails. */
static bool scratch_state_crc(uint32_t *crc)
{
    if (scratch == NULL || nesturbator_save_state(inst, scratch, scratch_size) != NESTURBATOR_OK) {
        return false;
    }
    *crc = read_le32(scratch + NT_STATE_CRC_OFFSET);
    return true;
}

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

/* L7580: one instance lives from here to retro_deinit. A failed create leaves
   inst NULL and retro_load_game then returns false. */
void retro_init(void)
{
    nesturbator_config cfg;

    invalidate_presentation();
    free_scratch();
    nesturbator_destroy(inst);
    inst = NULL;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    if (nesturbator_create(&cfg, &inst) != NESTURBATOR_OK) {
        inst = NULL;
    }
}

/* L7594: reset every global; the module may be used again. */
void retro_deinit(void)
{
    nesturbator_destroy(inst);
    inst = NULL;
    free_scratch();
    save_data = NULL;
    save_size = 0u;
    env_cb = NULL;
    video_cb = NULL;
    audio_cb = NULL;
    audio_batch_cb = NULL;
    input_poll_cb = NULL;
    input_state_cb = NULL;
    memset(palette, 0, sizeof palette);
    memset(video, 0, sizeof video);
    invalidate_presentation();
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

/* libretro.h's retro_reset "Resets the current game": the console's Reset button. The core runs
   its soft reset, which keeps CPU RAM and cartridge RAM, and never unloads or recreates the
   instance. A test-card instance has nothing to reset; the core returns OK and changes nothing.
   The status is dropped because libretro's reset returns void. */
void retro_reset(void)
{
    if (inst != NULL && nesturbator_reset(inst) == NESTURBATOR_OK) {
        invalidate_presentation();
    }
}

/* L7694: one frame. Input is polled once (L7685), the video callback is
   called once with XRGB8888 rows of 1024 bytes, and mono samples are
   duplicated into left/right pairs. Prefer the batch callback (L7465); use
   the per-sample callback only when no batch callback is registered (L7453).
   A JAM still polls input and sends the cached converted frame when it belongs to the running
   timeline (see the presentation cache above), or black; it sends no audio and does not reset
   the instance. Other failed frames send no video or audio. */
void retro_run(void)
{
    nesturbator_frame io;
    nesturbator_input input;
    nesturbator_status status;
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
    status = nesturbator_run_frame(inst, &io);
    if (status == NESTURBATOR_STOP_JAM) {
        const bool show_cache = cache_is_current();
        uint32_t crc;

        /* The jammed state reached from the cached frame stays on its timeline when loaded. */
        if (show_cache && scratch_state_crc(&crc)) {
            jam_state_crc = crc;
            jam_state_crc_valid = true;
        }
        if (video_cb != NULL) {
            video_cb(show_cache ? xrgb : blank, NT_WIDTH, NT_HEIGHT,
                     (size_t)NT_WIDTH * sizeof xrgb[0]);
        }
        return;
    }
    if (status != NESTURBATOR_OK) {
        return;
    }
    /* D-15: the runner's conversion loop. */
    nesturbator_host_convert(palette, video, NT_WIDTH, xrgb, NT_WIDTH);
    current_frame_number = io.frame_number;
    cached_frame_number = io.frame_number;
    cached_timeline = timeline;
    jam_state_crc_valid = false;
    completed_frame_valid = scratch == NULL || scratch_state_crc(&cached_state_crc);
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

/* L7707 retro_serialize_size, followed by retro_serialize and retro_unserialize. The state is
   the library's fixed-size, platform-independent save state (nesturbator.h, "Save states"), so
   the three entry points only translate its status codes to the size and bool the frontend
   expects. The size is 0 with no content and for the test card, because neither has a
   cartridge to save; with content it is constant for the loaded board. Only the exact size
   loads, and a refused load leaves the running game unchanged. The core sets no serialization
   quirks and ignores the savestate context and the audio/video enable flags, so the bytes are
   the same whichever the frontend reports. The same bytes serve RetroArch's rewind and its
   single-instance run-ahead (save, run ahead, load every frame); because the savestate context
   is ignored, run-ahead's second instance, rollback netplay and every other context get them
   too. The format is fixed little-endian and byte-identical on every platform, while
   libretro.h L5695-5707 asks the rollback-netplay context for big-endian integers, and netplay
   has no end-to-end test here. inst is NULL before retro_init, which both library calls
   accept. */
size_t retro_serialize_size(void)
{
    return nesturbator_state_size(inst);
}

bool retro_serialize(void *data, size_t len)
{
    return nesturbator_save_state(inst, data, len) == NESTURBATOR_OK;
}

bool retro_unserialize(const void *data, size_t len)
{
    const uint8_t *b = (const uint8_t *)data;
    uint32_t crc;

    if (nesturbator_load_state(inst, data, len) != NESTURBATOR_OK) {
        return false;
    }
    /* A load that returned OK has validated len as the state size, so the header is present. */
    current_frame_number = read_le64(b + NT_STATE_FRAME_OFFSET);
    crc = read_le32(b + NT_STATE_CRC_OFFSET);
    if (crc == cached_state_crc || (jam_state_crc_valid && crc == jam_state_crc)) {
        timeline = cached_timeline;
    } else {
        timeline = cached_timeline + 1u;
    }
    return true;
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

    if (game != NULL && (game->data == NULL || game->size == 0u)) {
        return false;
    }
    if (env_cb == NULL || !env_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &format)) {
        return false;
    }
    if (inst == NULL) {
        return false;
    }
    /* Drop the cached span before the cartridge it points into is released. */
    save_data = NULL;
    save_size = 0u;
    invalidate_presentation();
    free_scratch();
    nesturbator_unload_cartridge(inst);
    if (game != NULL &&
        nesturbator_load_cartridge(inst, game->data, game->size) != NESTURBATOR_OK) {
        return false;
    }
    /* Room for the state the presentation identity reads; the test card has none (size 0). */
    scratch_size = nesturbator_state_size(inst);
    if (scratch_size != 0u && (scratch = (uint8_t *)malloc(scratch_size)) == NULL) {
        scratch_size = 0u;
        nesturbator_unload_cartridge(inst);
        return false;
    }
    nesturbator_get_palette(inst, palette, 512u);
    /* The span is fetched at each successful load and dropped at unload: the host reads and
       writes it directly (L498-517). */
    if (game != NULL && nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &save_data,
                                               &save_size) != NESTURBATOR_OK) {
        save_data = NULL;
        save_size = 0u;
    }
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

/* L7790: the instance stays; the cartridge goes and the span with it. */
void retro_unload_game(void)
{
    save_data = NULL;
    save_size = 0u;
    invalidate_presentation();
    free_scratch();
    nesturbator_unload_cartridge(inst);
}

/* L7812: 0 is NTSC. */
unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}

/* L7826, L498-517: RETRO_MEMORY_SAVE_RAM is the battery span. The host reads it to write the
   .srm and writes a loaded .srm into it after retro_load_game and before the first retro_run;
   the adapter does no file I/O. Every other id, no game and a board without battery RAM give
   NULL and 0 (L498 allows it). */
void *retro_get_memory_data(unsigned id)
{
    return id == RETRO_MEMORY_SAVE_RAM ? save_data : NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    return id == RETRO_MEMORY_SAVE_RAM ? save_size : 0u;
}
