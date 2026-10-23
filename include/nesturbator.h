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

/* Library version of this header. Release tooling changes only the numbers,
   without realigning neighbouring comments. This narrow formatting guard
   preserves formatting when a version component gains a digit. */
/* clang-format off */
#define NESTURBATOR_VERSION_MAJOR 0 /* x-release-please-major */
#define NESTURBATOR_VERSION_MINOR 1 /* x-release-please-minor */
#define NESTURBATOR_VERSION_PATCH 10 /* x-release-please-patch */
/* clang-format on */

/* Binary interface number. nesturbator_create refuses a config whose abi
   field differs from the library's. A shared library's SOVERSION equals it. */
#define NESTURBATOR_ABI_VERSION 1

/* Raised whenever emulated behaviour changes the frame or audio output for
   the same inputs. Revision 6 is the PPUCTRL NMI edge: a $2000 write raises
   an NMI only when bit 7 changes from 0 to 1 while the vblank flag is set, so a
   game that rewrites $2000 inside its NMI handler no longer re-enters NMI.
   Revision 5 was the PPU fetch pipeline. */
#define NESTURBATOR_BEHAVIOUR_REVISION 6

/* The layout of the bytes nesturbator_save_state writes, and the only layout
   nesturbator_load_state reads. Any other version is refused with
   NESTURBATOR_ERR_STATE_FORMAT; there is no converter. Before the first release that contains
   save states it stays 1 and the layout may change. After that release, any change to the state
   bytes (a field added, removed, reordered or resized, or its meaning changed) raises it in the
   same commit. A fix that changes only emulated output raises
   NESTURBATOR_BEHAVIOUR_REVISION instead, which a state records but a load does not enforce.
   Raising it is not a breaking change to the C interface and leaves NESTURBATOR_ABI_VERSION as it
   is. */
#define NESTURBATOR_STATE_FORMAT_VERSION 1

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
       2 UxROM, 3 CNROM, 4 MMC3 and 7 AxROM). Every rejected image returns this status with
       the instance unchanged. */
    NESTURBATOR_ERR_CARTRIDGE = 7,
    /* Not a state this library reads: the magic is wrong or the format version is not
       NESTURBATOR_STATE_FORMAT_VERSION. */
    NESTURBATOR_ERR_STATE_FORMAT = 8,
    /* No cartridge is loaded, or the state's mapper, submapper, region or ROM CRC-32 differs from
       the loaded cartridge's: the state belongs to another game or board. */
    NESTURBATOR_ERR_STATE_CARTRIDGE = 9,
    /* The length is wrong: shorter than the 64-byte header, unequal to nesturbator_state_size, or
       unequal to the total size in the header. Truncation at any length gives this status. */
    NESTURBATOR_ERR_STATE_SIZE = 10,
    /* The state's CRC-32 does not match its bytes: they were damaged. */
    NESTURBATOR_ERR_STATE_CHECKSUM = 11,
    /* The CRC-32 matches, but a field is out of range, the header size is not 64 or a reserved
       byte is not zero: the state did not come from this library. */
    NESTURBATOR_ERR_STATE_INVALID = 12
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
    uint64_t frame_number;   /* out: frames run since power-on, 1 after the first;
                                restored by nesturbator_load_state */
    uint64_t ticks;          /* out: emulated time in half master-clock periods
                                since power-on, restored by nesturbator_load_state,
                                including the 7 CPU cycles of
                                each nesturbator_reset; instructions may carry residual
                                ticks across the nominal 714732-tick boundary.
                                PPU vblank begins at scanline 241 dot 1 and ends
                                at scanline 261 dot 1; a status read just before
                                the start dot suppresses that frame's NMI edge.
                                A $2000 write raises an NMI only when bit 7
                                changes from 0 to 1 while vblank is set;
                                rewriting it with bit 7 still set does not,
                                and clearing bit 7 suppresses a pending NMI. */
} nesturbator_frame;

/* Writes the library's version into *out. Writes nothing when out is NULL or
   out->size is 0; otherwise writes min(out->size, sizeof) bytes. A host
   compares the result with NESTURBATOR_VERSION_* when the library it runs may
   differ from the header it was built with, as with a shared library;
   nesturbator-run --version prints it. */
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
   for submapper 2 only; at power-on the reset vector is read from bank 0, and
   nesturbator_reset keeps the bank register, so a soft reset reads it from the
   selected bank). NROM and UxROM
   take either 8 KiB CHR ROM or 8 KiB declared CHR RAM.
   Supported NTSC MMC3: mapper 4, submappers 0/4; MMC6=no; four-screen=no.
   Metadata checks require exactly one canonical MMC3 support clause in this
   header, the README and each checked information file, rejecting duplicate
   clauses. Package checks reject semicolons, ASCII control characters and
   duplicate ZIP members, allow only component directories, and require each
   file pattern to match once. Listed names must agree with raw ZIP names;
   Unicode path aliases cannot substitute allowed names.
   The supported mappers are 0, 1, 2, 3, 4 and 7. Mapper 4 is the MMC3 in its
   Sharp (submapper 0, and every iNES 1 image) or NEC (submapper 4) revision:
   PRG a power of two from 16 to 512 KiB; 8 to 256 KiB CHR ROM or 8 KiB CHR
   RAM; PRG RAM none, 8 KiB work or 8 KiB battery, the battery bit equal to
   "battery RAM present"; an iNES 1 image gets 8 KiB (battery when flag 6
   bit 1 is set). $A001 enables (bit 7) and write-protects (bit 6) the RAM,
   which is enabled and writable at power-on; nesturbator_reset keeps the
   registers. MMC6, other submappers, four-screen and mappers 118, 119, 206
   and 249 are refused.
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
   board is refused before any allocation. Every rejection returns
   NESTURBATOR_ERR_CARTRIDGE with the instance unchanged. Unload releases
   cartridge state. */
nesturbator_status nesturbator_load_cartridge(nesturbator *inst, const void *data, size_t size);
/* Releases the cartridge and returns the instance to its just-created state with no
   cartridge: the test card, counters at zero, the PPU at the pre-render line. Safe with no
   cartridge loaded and with NULL. It does not change nesturbator_save_generation. After it the
   save span from nesturbator_get_memory is NULL and 0. */
void nesturbator_unload_cartridge(nesturbator *inst);

/* Reads CPU internal RAM without changing emulator state. Addresses $0000-$1FFF
   are accepted and use the NES's 2 KiB RAM mirrors; other addresses are
   rejected. This inspection path does not perform bus side effects and is
   intended for conformance harnesses. The runner's pinned AccuracyCoin
   observer uses it to connect decoded ROM result addresses to guest result
   bytes; it adds no library ABI or state-format requirement. On error,
   *value is unchanged. */
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
 * strobe and shift registers and a pending OAM DMA are cleared. Mapper
 * registers are kept, so the reset vector is read through the board's current
 * banks. The CPU sets the I flag, clears a JAM latch, lowers S by 3 and takes
 * the reset vector in 7 bus cycles, which count in ticks; frame_number does
 * not change.
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
 * header. Mapper 4 (MMC3) uses the same layout with V and N each 0 or 8 KiB;
 * an iNES 1 MMC3 image always gets 8 KiB (battery when flag 6 bit 1 is set), so
 * a game that expects no RAM needs a NES 2 header with V = N = 0.
 *
 * nesturbator_get_memory writes the span to *data and *size. A NULL inst, data
 * or size, or a kind other than NESTURBATOR_MEMORY_SAVE_RAM, returns
 * NESTURBATOR_ERR_ARGUMENT and writes nothing. No cartridge, or a cartridge
 * without a span, returns NESTURBATOR_OK with NULL and 0. The pointer is valid
 * from a successful load until unload, the next successful load or destroy;
 * nesturbator_reset, nesturbator_load_state (it copies into the same
 * allocation) and a refused load keep it. The host copies a saved file
 * in after load and before the first frame, and may read the span at any time
 * between calls.
 *
 * nesturbator_save_generation counts CPU bus writes that reached the span
 * while the board had that RAM enabled and writable, whether or not the byte
 * changed; a read-modify-write counts both of its writes. It runs from
 * nesturbator_create and never resets or decreases, so hosts compare it with
 * != . Load (including trainer bytes), reset, unload and writes the host makes
 * through the pointer do not count. A state load counts once when it changes
 * the battery bytes (the state's span differs from the instance's) and not at
 * all when it leaves them unchanged; a refused load never counts. So loading
 * the same state again, as rewind does, never moves it. See
 * nesturbator_load_state. nesturbator_save_generation(NULL) is 0. */
enum nesturbator_memory { NESTURBATOR_MEMORY_SAVE_RAM = 0 };
typedef enum nesturbator_memory nesturbator_memory;

nesturbator_status nesturbator_get_memory(nesturbator *inst, nesturbator_memory kind,
                                          uint8_t **data, size_t *size);
uint64_t nesturbator_save_generation(const nesturbator *inst);

/* Save states. A state is the whole machine at a frame boundary, in a fixed-size byte string
 * that is the same on every platform: save, load into any instance holding the same cartridge,
 * and every later frame, audio sample and tick equals the uninterrupted run's. The library does
 * no file I/O and never allocates for a state; the host owns the buffer.
 *
 * nesturbator_state_size returns the exact length of a state of the loaded cartridge: 0 for a
 * NULL instance or with no cartridge, otherwise constant from a successful load until unload. It
 * does not change the instance.
 *
 * nesturbator_save_state writes exactly nesturbator_state_size bytes to buf and leaves any bytes
 * beyond them alone. It changes nothing in the instance and writes buf only when it returns
 * NESTURBATOR_OK. A NULL buf is never a size query; call nesturbator_state_size for that.
 * Checks in order: inst or buf NULL gives NESTURBATOR_ERR_ARGUMENT; no cartridge gives
 * NESTURBATOR_ERR_STATE_CARTRIDGE; capacity below the state size gives
 * NESTURBATOR_ERR_BUFFER_TOO_SMALL. Saving a jammed instance succeeds.
 *
 * nesturbator_load_state replaces the instance's machine state with the state's, all or nothing:
 * every refusal leaves the instance byte for byte as it was. size must equal
 * nesturbator_state_size exactly; padded or truncated input is refused. Checks in order:
 *  1. inst or buf NULL gives NESTURBATOR_ERR_ARGUMENT.
 *  2. size below the 64-byte header gives NESTURBATOR_ERR_STATE_SIZE.
 *  3. A bad magic, or a format version other than NESTURBATOR_STATE_FORMAT_VERSION, gives
 *     NESTURBATOR_ERR_STATE_FORMAT.
 *  4. No cartridge loaded, or a mapper, submapper, region or ROM CRC-32 that differs from the
 *     loaded cartridge's, gives NESTURBATOR_ERR_STATE_CARTRIDGE. Identity comes before length
 *     because the length depends on the cartridge.
 *  5. size other than nesturbator_state_size, or other than the header's total size, gives
 *     NESTURBATOR_ERR_STATE_SIZE.
 *  6. A CRC-32 mismatch gives NESTURBATOR_ERR_STATE_CHECKSUM.
 *  7. A header size other than 64, a reserved header byte other than zero, or any field out of
 *     range gives NESTURBATOR_ERR_STATE_INVALID. Out of range includes a PPU position that is
 *     ahead of the header's ticks or a whole frame or more behind them, and a header ticks above
 *     2^62 (a count near 2^64 would wrap the frame loop), so a hostile state cannot
 *     make the next frame run for longer than about one frame of emulated work.
 * A flipped byte therefore reports the first check it breaks: the magic or version gives
 * FORMAT, an identity field gives CARTRIDGE, the total size gives SIZE, and any other byte,
 * including the recorded behaviour revision, gives CHECKSUM. The behaviour revision is recorded
 * and not enforced, so a state saved before an emulation fix loads but may diverge.
 * Test oracles that require the current behavior must explicitly bind the recorded revision,
 * content and intended absolute frame in addition to successful native loading. A displayed
 * frame (including a host's run-ahead lead) does not prove committed state or speculative audio.
 * Host containers are outside this native API; the test-only RetroArch reader qualifies one
 * measured uncompressed released-binary envelope and a bounded visible-marker window.
 * The registered libretro host tests select their battery and run-ahead fixtures by exact,
 * case-sensitive final filename; their CMake wrapper hashes the supplied special-file copy
 * against its manifest record before host launch. Slash always separates components; backslash
 * also separates them on Windows and is an ordinary filename byte on POSIX. The role self-test
 * covers adversarial names and path edges, while identity controls reject corrupted copies before
 * launch. The wrapper's argv probe recreates its owned output directory on every run before
 * checking paths with spaces, semicolons and backslashes; stale captures cannot satisfy it.
 * Direct host calls do not authenticate files.
 * Its negative controls distinguish native-valid wrong origins and stale revisions from parse
 * or checksum failures; screenshots qualify displayed identity, not every saved machine field.
 * The separate private progression verdict binds each displayed observation to one retained
 * native origin and rejects ambiguous frame candidates. A valid prefix is evidence only for that
 * declared continuation; the complete host path separately checks its load-plus-one-frame probe,
 * fixed original and rewound origins, paced coarse rewind and measured run-ahead display lead.
 * The network driver gives each attempt a private local tree and monotonic child, operation and
 * attempt deadlines. Its owned command-port lock is released if attempt-tree or journal setup
 * fails, preserving the setup status before any host or transport starts. It validates all
 * sixteen displayed-state boundaries against the generated
 * absolute-frame marker window, retaining native origins before slot reuse. Only the exact
 * read-only readiness reply retries; only a complete exact reply for the isolated path is accepted,
 * after its child exits or is reaped. The 1-second nc timeout sits inside a 5-second supervised
 * child slice, further capped by live operation and attempt time. Actions, state saves/loads,
 * screenshots and QUIT are one-shot, except for the prescribed forty individually paced rewind
 * sends. A hosted fault wrapper applies only to those one-shot mutations; readiness reads use the
 * real local transport, and interruption cleanup reaps the wrapper's owned send child. This is
 * bounded local test evidence and does not guarantee that UDP sends reach a host. A separate pair
 * of valid startup captures must show strict forward progress before the first PAUSE_TOGGLE; the
 * next two equal-frame captures prove the toggle held before the original anchor is saved. A host
 * that starts paused or stalls fails closed rather than treating a static image as pause evidence.
 * Those two startup captures are retained outside the sixteen canonical boundaries. A UDP send
 * accepted by the local socket is not proof that RetroArch received it; each retained image and
 * native origin is checked by the shared progression verdict. The journal writes and sanitizes
 * each row in one Perl process, using one monotonic observation for attempt elapsed/remaining time.
 * The POSIX interruption harness and its private selector share one unchanged TERM assertion:
 * genuine grep runs once, with its numeric status and exact output bytes retained separately
 * from recorder status. Binary pre/post journal snapshots copy through one open handle and bind
 * that handle's device, inode and size; they do not identify the bytes grep read. Synthetic tests
 * cover statuses 0, 1 and controlled 2, recorder errors, pathname replacement and event ownership.
 * Cooperative local session invocations reserve loopback TCP port 55355 with a bound, non-listening
 * SOCK_STREAM descriptor held at FD 9, separate from the host UDP command transport.
 * Bundled Perl binds and validates the descriptor, then
 * re-enters the eight-argument shell driver in the same PID while carrying the original monotonic
 * start sample. Release closes only that descriptor after owned children and the host are reaped;
 * it never unlinks or renames a shared path. Legacy lock and owner-sidecar paths are read-only
 * refusal observations, not ownership authority. Test-only port selection and release rendezvous
 * stay inside the harness; a private lock-reservation.tsv records the observed address, port, PID
 * and descriptor observations. Device and inode may both be zero and never establish ownership.
 * The harness observes inherited FD 9 independently before delegating to the real writer, binds
 * provenance to the actual driver and invocation, and collects launched children once on failure.
 * Release-gate FIFO reads and writes, including partial lines, share the remaining attempt and
 * cleanup deadline. The driver closes FD 9 in outliving command children. SIGKILL closes
 * the driver's descriptor but cannot guarantee cleanup or teardown of a noncooperating host.
 * The local reservation coordinates cooperating invocations that bind the same address and port;
 * it does not claim same-UID isolation or control over arbitrary processes that ignore it.
 * A source-only interruption trace include is shared by the driver and harness. The POSIX test
 * harness derives its source root from the canonical path of its executing script, then validates
 * the supplied driver and fixed trace include against that root before sourcing or launching host
 * or transport work. The serial retroarch.session.source_root test runs its preflight in the
 * POSIX ci and asan lanes. It exercises the repository root, CTest binary cwd and
 * an owned cwd containing spaces, and rejects a copied driver/include before the copied include can
 * run. Each cwd repeats all four finalizer faults and checks wait 143, the exact first reason,
 * cleanup_anomaly=0, oracle not-run and exact-port re-entry after owned-child reap. Before fixture
 * or driver work, offline checks call the actual writer and validator through handler entry,
 * journal close and cleanup. The exact 16-field trace admits host/child PIDs as positive decimal
 * or unavailable after cleanup clears them, while liveness remains unavailable when kill-0 is
 * inconclusive. Validation binds the unique invocation and observed driver PID, event sequence,
 * monotonic samples and reason-specific field checks. The shared capture boundary receives the
 * owned session ID explicitly; its nested `capture` directory name does not choose identity. The
 * offline selector uses the same boundary and final validator for two independently identified
 * sessions under `session-A/capture` and `session-B/capture`, rejecting wrong expected IDs, mixed
 * outer or inner rows and cross-session ledger reuse. An invalid ID marks the recorder unusable
 * without replacing the genuine assertion status. These identity checks supplement the seven
 * capture cases and 40 retained trace cases (36 original plus four lock controls), all without
 * launching the driver. Eleven lifecycle/QUIT cases require stable host/child PID observations in
 * the first two rows, live-with-PID consistency, unavailable observations to remain admissible,
 * final PID/liveness clearing and no QUIT before journal close. The retained mixed-ID observation
 * remains unchanged. The two-session examples remain generic ten-row identity checks. The actual
 * interruption control passes observed driver, host and harness PIDs to a separate strict 21-row
 * admission gate before recorder acceptance. It requires the exact event order, eight fields,
 * role-specific PIDs, statuses and details, contiguous sequence, nondecreasing monotonic time and
 * a final newline. Recorder rejection remains separate from the genuine assertion decision, and
 * host-after-wait preserves the kill-0 ambiguity. Trace field 14 reports the invocation's
 * validated held socket descriptor, not filesystem occupancy. Occupied, dangling, stale or
 * replaced legacy paths grant no authority to mutate them; they cause a conservative refusal.
 * A cleanup QUIT request write failure records zero sends when possible and emits
 * CLEANUP_REQUEST_WRITE_FAILED; owned teardown still preserves the original exit status.
 * The prior corrected candidate
 * `0268d3450f64392911a187ce5c040847fda5f8f8` passed focused CI 10/10, ASan 2/2 and normal CI
 * 649/649 with three ZIPs. Its source remained unchanged at review candidate
 * `7a6b1aaa87c9f8eb244aa1e901becc32352ecf4e`, whose independent review left five open findings;
 * these historical results do not accept this correction. The authoritative corrected-source
 * candidate/result register for Plan 17 gates and review is
 * `.planning/phases/16-host-evidence-and-roadmap/16-17-SOURCE-ACCEPTANCE.md`. The original
 * historical assertion and native timing causes remain unknown. Fresh native/exact-head acceptance
 * and owner consent, plus actual release publication, remain separate work in Plans 07 and 08.
 * A stopped operation uses its captured terminal observation, a live operation uses the row
 * observation, and an inactive row carries the last completed operation values. The focused
 * readiness control runs the delayed exact-reply and response-stall full prefixes once each, in
 * order. Its timing line is emitted before the strict full-driver duration assertion, preserving
 * available phase measurements and sanitized driver status/reason on failure; unavailable phases
 * stay explicitly unavailable. Its enclosing test interval includes fixture generation, captures,
 * native validation and cleanup, distinct from each readiness operation interval. The registered
 * readiness test also checks the timing report with synthetic over-bound measurements. The required
 * pinned-host run uses generated marker content in a distinct session, requires the complete
 * sixteen-boundary verdict, and checks dropped/doubled
 * advances and a native-valid wrong-origin branch through that same verdict. The test-only
 * prefix validator retains cold-start rendering when a displayed image is the native anchor,
 * because frame pixels are not serialized; for later targets it loads the already validated anchor
 * before advancing, skipping the overwritten pre-load replay. One bounded local comparison showed
 * a lower third-prefix process time but no overall readiness improvement, and does not identify
 * or clear the separate hosted timing failures. These observations prove the declared displayed
 * progression and retained origins; they do not prove every committed
 * machine field or audio. Portable whole-state and audio tests retain their broader proofs. Loading
 * restores frame_number and ticks, so the first frame after a load reports the frame number and
 * ticks of the uninterrupted run. The frame buffer and the audio are not in a state. Load never
 * returns NESTURBATOR_STOP_JAM: a state saved after a JAM opcode restores the latch and the next
 * nesturbator_run_frame returns NESTURBATOR_STOP_JAM, and a state saved before one clears it.
 *
 * Layout (every length and offset is in bytes; every multi-byte field is little-endian, written
 * one byte at a time, never as a struct copy):
 *    0  8  magic 89 4E 54 53 0D 0A 1A 0A, so text-mode transfers break it
 *    8  2  format version, NESTURBATOR_STATE_FORMAT_VERSION
 *   10  2  header size, 64
 *   12  4  total size, equal to nesturbator_state_size
 *   16  4  behaviour revision, NESTURBATOR_BEHAVIOUR_REVISION of the saving library
 *   20  2  mapper id
 *   22  1  submapper
 *   23  1  region, 0 for NTSC
 *   24  4  ROM CRC-32 (IEEE, reflected polynomial 0xEDB88320) of the 16 iNES header bytes, the
 *          PRG ROM and the CHR ROM; not CHR RAM, not the trainer
 *   28  4  state CRC-32 of every byte of the state except these four
 *   32  8  frame_number
 *   40  8  ticks
 *   48 16  reserved, zero
 * The machine fields follow, then the whole PRG RAM, then the 8 KiB CHR RAM when the board has
 * it. A state carries no pointer, host path, time stamp or padding.
 * Battery RAM is part of the PRG RAM, so a load replaces it; rewinding past an in-game save
 * reverts the save RAM. */
size_t nesturbator_state_size(const nesturbator *inst);
nesturbator_status nesturbator_save_state(const nesturbator *inst, void *buf, size_t capacity);
nesturbator_status nesturbator_load_state(nesturbator *inst, const void *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* NESTURBATOR_H */
