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
 *   - The caller zeroes the whole struct with memset before setting any
 *     field, so its padding and any bytes a newer header appends are zero.
 *     A brace or designated initialiser, or a struct copy, does not zero
 *     padding. This is what lets a struct from a newer header pass the
 *     larger-size check of an older library.
 * - Struct fields are fixed-width integers and pointers only.
 * - Status codes have fixed values and are only ever appended.
 * - An error status changes no state. NESTURBATOR_STOP_JAM is a latched stop
 *   condition and leaves the CPU stopped at its JAM opcode.
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
#define NESTURBATOR_VERSION_MINOR 1 /* x-release-please-minor */
#define NESTURBATOR_VERSION_PATCH 4 /* x-release-please-patch */

/* Binary interface number. nesturbator_create refuses a config whose abi
   field differs from the library's. A shared library's SOVERSION equals it. */
#define NESTURBATOR_ABI_VERSION 1

/* Raised whenever emulated behaviour changes the frame or audio output for
   the same inputs. */
#define NESTURBATOR_BEHAVIOUR_REVISION 3

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
    NESTURBATOR_ERR_BUFFER_TOO_SMALL = 5,
    /* The loaded cartridge executed a JAM opcode; the instance is latched. */
    NESTURBATOR_STOP_JAM = 6,
    /* Cartridge bytes are malformed or outside the supported mapper-0 profile. */
    NESTURBATOR_ERR_CARTRIDGE = 7
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
   so arena allocators work. The library allocates at create and cartridge
   load, never while running. */
typedef struct nesturbator_allocator {
    void *(*alloc)(void *user, size_t size);
    void (*free)(void *user, void *ptr, size_t size);
    void *user;
} nesturbator_allocator;

/* Input of nesturbator_create. memset it to zero, then set size to
   sizeof(nesturbator_config) and abi to NESTURBATOR_ABI_VERSION. The zeroed
   allocator means malloc and free. */
typedef struct nesturbator_config {
    uint32_t size;                   /* sizeof(nesturbator_config) */
    uint32_t abi;                    /* NESTURBATOR_ABI_VERSION */
    nesturbator_allocator allocator; /* all NULL: malloc and free */
} nesturbator_config;

/* Standard controller buttons. The bit values match the order read from the
   NES serial ports, A first and Right last. */
enum nesturbator_button {
    NESTURBATOR_BUTTON_A = 1u << 0,
    NESTURBATOR_BUTTON_B = 1u << 1,
    NESTURBATOR_BUTTON_SELECT = 1u << 2,
    NESTURBATOR_BUTTON_START = 1u << 3,
    NESTURBATOR_BUTTON_UP = 1u << 4,
    NESTURBATOR_BUTTON_DOWN = 1u << 5,
    NESTURBATOR_BUTTON_LEFT = 1u << 6,
    NESTURBATOR_BUTTON_RIGHT = 1u << 7
};

/* Input for one frame. Port 0 is $4016 and port 1 is $4017. Values are
   sampled at the start of nesturbator_run_frame; setting input during a frame
   affects the next call. Runner movie replay submits one mask pair through this
   contract before each frame. Zero unused/reserved bytes before calling. */
typedef struct nesturbator_input {
    uint32_t size;      /* in: sizeof(nesturbator_input) */
    uint8_t buttons[2]; /* in: NESTURBATOR_BUTTON_* mask for each port */
} nesturbator_input;

/* The same default values for C and C++ sources. A config passed to
   nesturbator_create must still be zeroed with memset first, because an
   initialised declaration leaves padding and tail bytes unspecified; do not
   use this macro as the initialiser of a struct handed to the library.
   Every member is listed, so the initialiser is warning-free in C and does
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
 * the three PPUMASK emphasis bits in bits 6-8; bits 9-15 are zero. Background
 * pixels reflect mapper-0 nametable mirroring, pattern/attribute data, scroll,
 * grayscale and emphasis. Sprite pixels use OAM order, transparency, palette,
 * horizontal/vertical flip, 8x8 or 8x16 pattern selection and the priority bit;
 * `$2001` controls left-edge clipping. Sprite-zero hit and overflow are
 * tracked in PPU status at their scanline/pixel timing. Native pixels are the
 * canonical frame-hash input; display conversion is separate. A CPU write to
 * `$4014` queues an OAM DMA. The next CPU read is
 * halted while it reads one 256-byte page through the normal bus and writes
 * OAM starting at `$2003`'s address; it stalls the CPU for 513 or 514 cycles
 * by cycle parity while PPU time continues.
 *
 * Audio: mono signed 16-bit samples at the info sample rate. A frame yields
 * 798 or 799 samples; the fraction carries over to the next frame. Mapper-0
 * pulse, triangle, noise and DMC state advances on CPU bus cycles and
 * contributes its DAC level to PCM. With no cartridge loaded, every sample is
 * zero. Channel timer, sequence, envelope/sweep/linear-counter and gating
 * behavior follows NES-HARDWARE-CPU-APU.md section 4 (HWC.05, HWC.08,
 * HWC.10). The pulse and 16 x 16 x 128 TND integer mixer is from section 5
 * (HWC.11). Mixed-level transitions use the fixed-point 16-tap, 32-phase
 * band-limited kernel, followed by the NES 90 Hz and 440 Hz high-pass and
 * 14 kHz low-pass filters (HWC.28-HWC.30). Synthesis and filter history are
 * per-instance and persist across frame calls; the bounded PCM staging ring
 * is drained into the caller's frame buffer. For fixed content, behavior
 * revision and input sequence, PCM samples are bit-for-bit deterministic. */
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
                                since create; instructions may carry residual
                                ticks across the nominal 714732-tick boundary.
                                PPU vblank begins at scanline 241 dot 1 and ends
                                at scanline 261 dot 1; a status read just before
                                the start dot suppresses that frame's NMI edge. */
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
 * An instance holds a 2A03 CPU, a 6502 without decimal mode, that matches
 * the public 65x02 test vectors on every opcode and bus cycle. The CPU does
 * not yet run during frames.
 * A new instance has no cartridge: each frame is a fixed test pattern and
 * silence. */
nesturbator_status nesturbator_create(const nesturbator_config *cfg, nesturbator **out);

/* Sets the next frame's standard controller state. A NULL instance or input
   returns NESTURBATOR_ERR_ARGUMENT; a bad size tag returns
   NESTURBATOR_ERR_STRUCT_SIZE. Refused calls leave the instance unchanged. */
nesturbator_status nesturbator_set_input(nesturbator *inst, const nesturbator_input *input);

/* Frees an instance through the allocator it was created with. Accepts NULL. */
void nesturbator_destroy(nesturbator *inst);

/* Copies one bounded mapper-0 iNES v1 or NES 2 image into the instance.
   Accepts 16 or 32 KiB PRG and either 8 KiB CHR ROM or 8 KiB declared CHR RAM;
   trainers are accepted. Other mappers, unsupported console/region/RAM
   profiles, malformed headers, truncation, extra payload, and images above
   64 MiB return NESTURBATOR_ERR_CARTRIDGE before cartridge allocation and
   leave a previously loaded cartridge untouched. Allocation failure returns
   NESTURBATOR_ERR_NO_MEMORY. A loaded image resets the CPU from its PRG reset
   vector at the end of the PRG data (including the upper bank of 32 KiB NROM).
   Unload releases cartridge state. */
nesturbator_status nesturbator_load_cartridge(nesturbator *inst, const void *data, size_t size);
void nesturbator_unload_cartridge(nesturbator *inst);

/* Reads CPU internal RAM without changing emulator state. Addresses $0000-$1FFF
   are accepted and use the NES's 2 KiB RAM mirrors; other addresses are
   rejected. This inspection path does not perform bus side effects and is
   intended for conformance harnesses. On error, *value is unchanged. */
nesturbator_status nesturbator_peek_cpu_ram(const nesturbator *inst, uint16_t address,
                                            uint8_t *value);

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
 * At the start of a successful call, the latest input set through
 * nesturbator_set_input is sampled for that frame. Instructions complete
 * across the requested frame boundary; the next input is sampled only on the
 * next call, so a crossing instruction cannot mix frame masks. Cartridge
 * audio clocks the NTSC RP2A03 channels and frame sequencer on CPU cycles;
 * frame-counter and DMC interrupt status are independent. The CPU samples an
 * enabled IRQ at instruction boundaries and enters the IRQ vector (HWC.02).
 * No cartridge: test pattern and silence. A JAM opcode stops with
 * NESTURBATOR_STOP_JAM; that instance then remains latched and does not
 * advance on later frame calls. Audio is mono signed 16-bit PCM; no-cartridge
 * audio is zero. */
nesturbator_status nesturbator_run_frame(nesturbator *inst, nesturbator_frame *io);

/* Copies up to `count` calibrated NTSC-to-sRGB XRGB8888 entries (0x00RRGGBB),
 * indexed by native pixel value (palette entry | emphasis << 6). Display data
 * only; the native pixels remain the canonical frame-hash input. Copies
 * min(count, 512) entries; out NULL or count 0 does nothing. inst is reserved
 * for per-PPU-revision tables and may be NULL. */
void nesturbator_get_palette(const nesturbator *inst, uint32_t *out_xrgb8888, uint32_t count);

#ifdef __cplusplus
}
#endif

#endif /* NESTURBATOR_H */
