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
#define NESTURBATOR_VERSION_PATCH 9 /* x-release-please-patch */

/* Binary interface number. nesturbator_create refuses a config whose abi
   field differs from the library's. A shared library's SOVERSION equals it. */
#define NESTURBATOR_ABI_VERSION 1

/* Raised whenever emulated behaviour changes the frame or audio output for
   the same inputs. */
#define NESTURBATOR_BEHAVIOUR_REVISION 5

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
    /* Cartridge bytes are malformed or outside a supported board profile (mappers 0 NROM, 1 MMC1,
       2 UxROM, 3 CNROM and 7 AxROM). */
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
 * Video: native pixels, one uint16_t each, rows 0-239 in physical visible
 * scanline order, with row 0 rendered after pre-render scanline 261. A sprite
 * with OAM Y=$FF can wrap into row 0. Row y starts at
 * video[y * video_pitch]. A pixel is a palette entry 0-63 in bits 0-5 and
 * the three PPUMASK emphasis bits in bits 6-8; bits 9-15 are zero. Background
 * pixels reflect the cartridge's nametable mirroring, pattern/attribute data, scroll,
 * grayscale and emphasis. Sprite pixels use OAM order, transparency, palette,
 * horizontal/vertical flip, 8x8 or 8x16 pattern selection and the priority bit;
 * `$2001` controls left-edge clipping. Sprite-zero hit and overflow are
 * tracked in PPU status at their scanline/pixel timing. After eight selected
 * sprites, the 2C02 diagonal OAM scan can set overflow from a non-Y byte,
 * while a skipped in-range Y does not; the flag clears at pre-render dot 1.
 * The first eight selected sprites each take eight dots to copy, so the ninth
 * Y is compared at dot 130 after its dot-129 read; `$2002` reports overflow
 * only after that comparison.
 * Native pixels are the
 * canonical frame-hash input; display conversion is separate. A CPU write to
 * `$4014` queues an OAM DMA. The next CPU read is
 * halted while it reads one 256-byte page through the normal bus and writes
 * OAM starting at `$2003`'s address; it stalls the CPU for 513 or 514 cycles
 * by cycle parity while PPU time continues.
 *
 * Audio: mono signed 16-bit samples at the info sample rate. The libretro
 * adapter duplicates each sample into left and right channels, preferring its
 * batch callback and using the single-sample callback only when batch is
 * unavailable (libretro.h L7453, L7465). A frame yields
 * 798 or 799 samples; the fraction carries over to the next frame. The
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
                                since create, including the 7 CPU cycles of
                                each nesturbator_reset; instructions may carry residual
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
 * the public 65x02 test vectors on every opcode and bus cycle. The CPU runs
 * during every frame.
 * A new instance has no cartridge: each frame is a fixed test pattern and
 * silence. */
nesturbator_status nesturbator_create(const nesturbator_config *cfg, nesturbator **out);

/* Sets the next frame's standard controller state. A NULL instance or input
   returns NESTURBATOR_ERR_ARGUMENT; a bad size tag returns
   NESTURBATOR_ERR_STRUCT_SIZE. Refused calls leave the instance unchanged. */
nesturbator_status nesturbator_set_input(nesturbator *inst, const nesturbator_input *input);

/* Frees an instance through the allocator it was created with. Accepts NULL. */
void nesturbator_destroy(nesturbator *inst);

/* Copies one bounded iNES v1 or NES 2 image into the instance. Supported
   boards are mapper 0 (NROM: 16 or 32 KiB PRG), mapper 1 (MMC1, submapper 0,
   or 5 with 32 KiB PRG: PRG a power of two from 32 to 512 KiB, 512 KiB only
   with 8 KiB CHR; 8 to 128 KiB CHR ROM or 8 KiB CHR RAM; PRG RAM of 0, 8, 16
   or 32 KiB in total, above 8 KiB only with 8 KiB CHR; the battery bit equal
   to "battery RAM present"; no CHR NVRAM; see the battery notes below), mapper 2 (UxROM: PRG in
   16 KiB banks up to 4 MiB, submappers 0 to 2; the last bank is fixed at
   $C000 and a write to $8000-$FFFF selects the bank at $8000, ANDed with the
   ROM byte under that write for submappers 0 and 2, raw for submapper 1) and
   mapper 3 (CNROM: 16 or 32 KiB PRG and 8, 16 or 32 KiB CHR ROM, submappers
   0 to 2; a write to $8000-$FFFF selects the 8 KiB CHR bank, with the same
   AND rule; writes to CHR ROM are ignored) and mapper 7 (AxROM: 32 to 256 KiB
   PRG and 8 KiB declared CHR RAM; a write to $8000-$FFFF selects the 32 KiB
   bank with bits 0 to 2 and the single-screen nametable page with bit 4,
   ignoring the header mirroring bit; ANDed with the ROM byte under the write
   for submapper 2 only; the reset vector is read from bank 0). NROM and UxROM
   take either 8 KiB CHR ROM or 8 KiB declared CHR RAM.
   Mapper 1 is the MMC1B chip: its serial port at $8000-$FFFF takes five
   writes, a write on the CPU cycle right after another write is ignored
   unless bit 7 is set (the reset), PRG RAM is enabled at power-on and bit 4
   of the PRG register disables it, and the SNROM, SOROM, SUROM and SXROM
   variants are derived from the header sizes. The registers and the battery
   RAM are kept by nesturbator_reset. MMC1A, mapper 155 and 2ME are not
   emulated.
   A trainer initializes the writable 8 KiB PRG RAM at CPU $7000-$71FF
   (on a mapper 1 image with no declared RAM the trainer's 8 KiB is work RAM
   with no save span); other addresses in $6000-$7FFF start at zero. Mappers
   0, 2, 3 and 7 refuse the battery bit and every RAM size. Other mappers, unsupported
   console/region/RAM profiles, malformed headers, truncation, extra payload, and images above 64
   MiB return NESTURBATOR_ERR_CARTRIDGE before cartridge allocation and leave a previously loaded
   cartridge untouched. Allocation failure returns NESTURBATOR_ERR_NO_MEMORY. A loaded image starts
   the CPU at the reset vector read through the board's power-on banks. An image whose mapper has no
   board is refused before any allocation. Unload releases cartridge state. */
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

/* Soft reset: the console's Reset button. Call it between frames.
 * inst NULL gives NESTURBATOR_ERR_ARGUMENT. With no cartridge loaded it
 * returns NESTURBATOR_OK and changes nothing. It never allocates or frees.
 * CPU RAM, cartridge RAM (PRG RAM and CHR RAM), the registers A, X and Y and
 * the host's input state are kept, so battery saves survive. The controller
 * strobe and shift registers and a pending OAM DMA are cleared. The CPU sets
 * the I flag, clears a JAM latch, lowers S by 3 and takes the reset vector in
 * 7 bus cycles, which count in ticks; frame_number does not change.
 * The PPU restarts at the top of the picture (scanline 0, dot 0) and ignores
 * writes to $2000, $2001, $2005 and $2006 until the end of the next vblank:
 * 29,667 CPU cycles from the reset (NESdev documents about 29,658). Its
 * control, mask, scroll latch and read buffer are cleared; v, status, OAM
 * address and video memory are kept. The APU is silenced as by a $4015 = 0
 * write, its IRQs are cleared and the last $4017 mode is re-applied; its
 * output filters are kept. Load is unchanged: no write-ignore window and no
 * startup sequence at power-on, so frame and audio hashes from load stay. */
nesturbator_status nesturbator_reset(nesturbator *inst);

/* Battery-backed memory. The core does no file I/O: it hands the host a span
 * of its own memory and counts the CPU writes that reach it, and the host
 * keeps the file.
 *
 * Mapper 1 (MMC1) cartridges carry PRG RAM laid out as [V bytes work][N bytes
 * NVRAM], work RAM first. The save span is the N battery bytes; a SOROM image
 * with a work half exposes only its battery half. The size comes from the
 * header: iNES 1 gives 8 KiB (32 KiB above 256 KiB of PRG), all of it battery
 * when flag 6 bit 1 is set and none otherwise; NES 2 gives V from the low
 * and N from the high nibble of byte 10, and its battery bit must equal N != 0.
 * Sizes for iNES 1 mapper 1: PRG up to 256 KiB with battery, an 8 KiB span;
 * above 256 KiB, 32 KiB; without battery, the RAM is work RAM and there is no
 * span. V = N = 0 leaves $6000-$7FFF as open bus. An iNES 1 SOROM dump gets
 * 8 KiB (its work half cannot be told apart); a true SOROM needs a NES 2
 * header.
 *
 * nesturbator_get_memory writes the span to *data and *size. A NULL inst, data
 * or size, or a kind other than NESTURBATOR_MEMORY_SAVE_RAM, returns
 * NESTURBATOR_ERR_ARGUMENT and writes nothing. No cartridge, or a cartridge
 * without a span, returns NESTURBATOR_OK with NULL and 0. The pointer is valid
 * from a successful load until unload, the next successful load or destroy;
 * nesturbator_reset and a refused load keep it. The host copies a saved file
 * in after load and before the first frame, and may read the span at any time
 * between calls.
 *
 * nesturbator_save_generation counts CPU bus writes that reached the span
 * while the board had that RAM enabled and writable, whether or not the byte
 * changed; a read-modify-write counts both of its writes. It runs from
 * nesturbator_create and never resets or decreases, so hosts compare it with
 * != . Load (including trainer bytes), reset, unload and writes the host makes
 * through the pointer do not count. nesturbator_save_generation(NULL) is 0. */
enum nesturbator_memory { NESTURBATOR_MEMORY_SAVE_RAM = 0 };
typedef enum nesturbator_memory nesturbator_memory;

nesturbator_status nesturbator_get_memory(nesturbator *inst, nesturbator_memory kind,
                                          uint8_t **data, size_t *size);
uint64_t nesturbator_save_generation(const nesturbator *inst);

#ifdef __cplusplus
}
#endif

#endif /* NESTURBATOR_H */
