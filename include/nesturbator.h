/*
 * nesturbator: a NES emulator core in C.
 *
 * This is the one public header. It includes only <stdint.h> and <stddef.h>
 * and compiles as C17 and as C++.
 *
 * Conventions (fixed; later releases only append):
 *
 * - Functions and types start with nesturbator_, macros and constants with
 *   NESTURBATOR_. The instance is opaque.
 * - Every struct that crosses the boundary starts with a uint32_t size field,
 *   which the caller sets to sizeof the struct it compiled against.
 *   - Input structs: a size of 0, or below the struct's first released size,
 *     gives NESTURBATOR_ERR_STRUCT_SIZE. A size larger than this library's
 *     struct is accepted only if every byte past this library's struct is
 *     zero; otherwise NESTURBATOR_ERR_STRUCT_SIZE.
 *   - Output structs: the library writes min(size, its own sizeof) bytes and
 *     leaves the size field holding the caller's value.
 * - Struct fields are fixed-width integers and pointers only.
 * - Status codes have fixed values and are only ever appended.
 * - A call that returns a status other than NESTURBATOR_OK changes no state.
 * - NESTURBATOR_ABI_VERSION rises only on a breaking change to this header.
 *   Appending functions, struct fields or status codes does not raise it.
 *   The library version (NESTURBATOR_VERSION_*) is separate from it.
 */
#ifndef NESTURBATOR_H
#define NESTURBATOR_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Library version of this header. Updated by the release tooling. */
#define NESTURBATOR_VERSION_MAJOR 0 /* x-release-please-major */
#define NESTURBATOR_VERSION_MINOR 0 /* x-release-please-minor */
#define NESTURBATOR_VERSION_PATCH 0 /* x-release-please-patch */

/* Binary interface number. nesturbator_create refuses a config whose abi
   field differs from the library's. A shared library's SOVERSION equals it. */
#define NESTURBATOR_ABI_VERSION 1

/* Raised whenever emulated behaviour changes the frame or audio output for
   the same inputs. */
#define NESTURBATOR_BEHAVIOUR_REVISION 1

/* Result of a call. Values are fixed and only appended. */
enum nesturbator_status {
    NESTURBATOR_OK = 0,
    /* A required pointer is NULL, or an argument is out of range. */
    NESTURBATOR_ERR_ARGUMENT = 1,
    /* A struct's size field is 0, too small, or larger with a non-zero tail. */
    NESTURBATOR_ERR_STRUCT_SIZE = 2,
    /* The config's abi field differs from NESTURBATOR_ABI_VERSION. */
    NESTURBATOR_ERR_ABI = 3,
    /* The allocator returned NULL. */
    NESTURBATOR_ERR_NO_MEMORY = 4,
    /* A caller-owned buffer is too small for the output. */
    NESTURBATOR_ERR_BUFFER_TOO_SMALL = 5
};
typedef enum nesturbator_status nesturbator_status;

/* An emulator instance. All emulation state lives in it; instances are
   independent of each other. */
typedef struct nesturbator nesturbator;

/* Output of nesturbator_get_version. */
typedef struct nesturbator_version {
    uint32_t size;  /* in: sizeof(nesturbator_version) */
    uint32_t major; /* library version */
    uint32_t minor;
    uint32_t patch;
    uint32_t abi;                /* NESTURBATOR_ABI_VERSION of the library */
    uint32_t behaviour_revision; /* NESTURBATOR_BEHAVIOUR_REVISION of the library */
} nesturbator_version;

/* Memory hooks. Either all three members are NULL, which means the C
   library's malloc and free, or alloc and free are both set (user may be
   NULL). Any other combination is NESTURBATOR_ERR_ARGUMENT. alloc returns
   memory aligned as malloc's is: suitable for any object type, at least the
   alignment of max_align_t. free receives the size that was passed to alloc,
   so arena allocators work. The library allocates only in nesturbator_create,
   never while running. */
typedef struct nesturbator_allocator {
    void *(*alloc)(void *user, size_t size);
    void (*free)(void *user, void *ptr, size_t size);
    void *user;
} nesturbator_allocator;

/* Input of nesturbator_create. Initialise it with NESTURBATOR_CONFIG_INIT so
   that size and abi describe the header the host compiled against. */
typedef struct nesturbator_config {
    uint32_t size;                   /* sizeof(nesturbator_config) */
    uint32_t abi;                    /* NESTURBATOR_ABI_VERSION */
    nesturbator_allocator allocator; /* all NULL: malloc and free */
} nesturbator_config;

/* Every member is listed, so the initialiser is warning-free in C and does
   not narrow in C++. Kept out of clang-format, which would lay the braces out
   as a block. */
/* clang-format off */
#define NESTURBATOR_CONFIG_INIT \
    { (uint32_t)sizeof(nesturbator_config), NESTURBATOR_ABI_VERSION, { NULL, NULL, NULL } }
/* clang-format on */

/* Output of nesturbator_get_info. Rates are exact fractions num/den. */
typedef struct nesturbator_info {
    uint32_t size;    /* in: sizeof(nesturbator_info) */
    uint32_t width;   /* pixels per row of the video output: 256 */
    uint32_t height;  /* rows of the video output: 240 */
    uint32_t fps_num; /* frames per second, NTSC: 39375000 / 655171 */
    uint32_t fps_den;
    uint32_t sample_rate_num; /* audio samples per second: 48000 / 1 */
    uint32_t sample_rate_den;
} nesturbator_info;

/* In/out of nesturbator_run_frame. The caller owns both buffers.
 *
 * Video: native pixels, one uint16_t each, row y starting at
 * video[y * video_pitch]. A pixel is a palette entry 0-63 in bits 0-5 and
 * the three emphasis bits in bits 6-8; bits 9-15 are zero.
 *
 * Audio: mono signed 16-bit samples at the info sample rate. A frame yields
 * 798 or 799 samples; the fraction carries over to the next frame. */
typedef struct nesturbator_frame {
    uint32_t size;           /* in: sizeof(nesturbator_frame) */
    uint16_t *video;         /* in: at least 240 rows of video_pitch pixels */
    uint32_t video_pitch;    /* in: pixels per row of video, at least 256 */
    int16_t *audio;          /* in: room for audio_capacity samples; may be
                                NULL only when audio_capacity is 0 */
    uint32_t audio_capacity; /* in: samples that fit in audio */
    uint32_t audio_count;    /* out: samples written to audio */
    uint64_t frame_number;   /* out: frames run by this instance, 1 after the first */
    uint64_t ticks;          /* out: emulated time in half master-clock periods
                                since create; 714732 per NTSC frame */
} nesturbator_frame;

/* Writes the library's version into *out. Writes nothing when out is NULL or
   out->size is 0; otherwise writes min(out->size, sizeof) bytes. */
void nesturbator_get_version(nesturbator_version *out);

/* Creates an instance and stores it in *out.
 * Checks in order: cfg or out NULL gives NESTURBATOR_ERR_ARGUMENT; a bad size
 * tag gives NESTURBATOR_ERR_STRUCT_SIZE; an abi other than
 * NESTURBATOR_ABI_VERSION gives NESTURBATOR_ERR_ABI; a partly NULL allocator
 * gives NESTURBATOR_ERR_ARGUMENT; a failed allocation gives
 * NESTURBATOR_ERR_NO_MEMORY. *out is written only on NESTURBATOR_OK.
 * A new instance has no cartridge: each frame is a fixed test pattern and
 * silence. */
nesturbator_status nesturbator_create(const nesturbator_config *cfg, nesturbator **out);

/* Frees an instance through the allocator it was created with. Accepts NULL. */
void nesturbator_destroy(nesturbator *inst);

/* Writes the video and audio format into *out. Writes nothing when inst or
   out is NULL or out->size is 0; otherwise min(out->size, sizeof) bytes. */
void nesturbator_get_info(const nesturbator *inst, nesturbator_info *out);

/* Runs one frame into the caller's buffers.
 * Every check happens before any state changes: inst or io NULL, or video
 * NULL, gives NESTURBATOR_ERR_ARGUMENT; a bad size tag gives
 * NESTURBATOR_ERR_STRUCT_SIZE; audio NULL with audio_capacity above 0 gives
 * NESTURBATOR_ERR_ARGUMENT; video_pitch below 256, or audio_capacity below
 * this frame's sample count, gives NESTURBATOR_ERR_BUFFER_TOO_SMALL.
 * On success it fills 256x240 pixels and audio_count samples, advances the
 * instance by one frame and writes audio_count, frame_number and ticks.
 * No cartridge: test pattern and silence. */
nesturbator_status nesturbator_run_frame(nesturbator *inst, nesturbator_frame *io);

/* Copies up to `count` XRGB8888 entries (0x00RRGGBB), indexed by native
 * pixel value (palette entry | emphasis << 6). Display data only; never part
 * of a hash. Copies min(count, 512) entries; out NULL or count 0 does
 * nothing. inst is reserved for per-PPU-revision tables and may be NULL. */
void nesturbator_get_palette(const nesturbator *inst, uint32_t *out_xrgb8888, uint32_t count);

#ifdef __cplusplus
}
#endif

#endif /* NESTURBATOR_H */
