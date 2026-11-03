/* D-11: version, info, create and allocator tests, through the public
   header only. */
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "nesturbator.h"
#include "../check.h"

typedef struct config_v1_contract {
    uint32_t size, abi;
    nesturbator_allocator allocator;
} config_v1_contract;

typedef struct info_v1_contract {
    uint32_t size, width, height, fps_num, fps_den, sample_rate_num, sample_rate_den;
} info_v1_contract;

typedef struct frame_v1_contract {
    uint32_t size;
    uint16_t *video;
    uint32_t video_pitch;
    int16_t *audio;
    uint32_t audio_capacity, audio_count;
    uint64_t frame_number, ticks;
} frame_v1_contract;

_Static_assert(sizeof(nesturbator_config) == sizeof(config_v1_contract), "config v1 size retained");
_Static_assert(offsetof(nesturbator_config, allocator) == offsetof(config_v1_contract, allocator),
               "config v1 prefix retained");
_Static_assert(sizeof(nesturbator_info) == sizeof(info_v1_contract), "info v1 size retained");
_Static_assert(offsetof(nesturbator_info, sample_rate_den) ==
                   offsetof(info_v1_contract, sample_rate_den),
               "info v1 prefix retained");
_Static_assert(sizeof(nesturbator_frame) == sizeof(frame_v1_contract), "frame v1 size retained");
_Static_assert(offsetof(nesturbator_frame, ticks) == offsetof(frame_v1_contract, ticks),
               "frame v1 prefix retained");

static nesturbator_status (*const load_region_fn)(nesturbator *, const void *, size_t,
                                                  enum nesturbator_region_preference) =
    nesturbator_load_cartridge_region;
static enum nesturbator_region (*const get_region_fn)(const nesturbator *) = nesturbator_get_region;

/* Counts every call and the bytes passed, to show create and destroy pair
   up and free receives the size alloc was given. */
typedef struct counter {
    unsigned allocs;
    unsigned frees;
    size_t alloc_bytes;
    size_t free_bytes;
    union { /* aligned for any field the instance can hold */
        uint64_t u;
        void *p;
        void (*f)(void);
        unsigned char bytes[65536]; /* room for the instance, RAM included */
    } storage;
    int in_use;
} counter;

static void *counting_alloc(void *user, size_t size)
{
    counter *c = (counter *)user;
    c->allocs++;
    c->alloc_bytes += size;
    if (c->in_use || size > sizeof c->storage.bytes) {
        return NULL;
    }
    c->in_use = 1;
    return c->storage.bytes;
}

static void counting_free(void *user, void *ptr, size_t size)
{
    counter *c = (counter *)user;
    c->frees++;
    c->free_bytes += size;
    if (ptr == c->storage.bytes) {
        c->in_use = 0;
    }
}

static void *failing_alloc(void *user, size_t size)
{
    (void)user;
    (void)size;
    return NULL;
}

static void never_free(void *user, void *ptr, size_t size)
{
    (void)ptr;
    (void)size;
    *(int *)user += 1; /* create must not free what it never got */
}

/* A config as a newer host would send it: this build's struct followed by
   fields this build does not know, all zero unless a test sets one. */
#define TAIL_BYTES 8u
typedef union big_config {
    nesturbator_config cfg;
    unsigned char bytes[sizeof(nesturbator_config) + TAIL_BYTES];
} big_config;

static void make_big(big_config *b)
{
    memset(b, 0, sizeof *b);
    b->cfg.size = (uint32_t)sizeof b->bytes;
    b->cfg.abi = NESTURBATOR_ABI_VERSION;
}

/* A config as the header asks callers to build it: zeroed with memset, then
   size and abi set. The zeroed allocator means malloc and free. */
static void default_config(nesturbator_config *cfg)
{
    memset(cfg, 0, sizeof *cfg);
    cfg->size = (uint32_t)sizeof *cfg;
    cfg->abi = NESTURBATOR_ABI_VERSION;
}

static void test_version(void)
{
    nesturbator_version v;
    memset(&v, 0xAB, sizeof v);
    v.size = (uint32_t)sizeof v;
    nesturbator_get_version(&v);
    CHECK_EQ_U64(v.size, sizeof v);
    CHECK_EQ_U64(v.major, NESTURBATOR_VERSION_MAJOR);
    CHECK_EQ_U64(v.minor, NESTURBATOR_VERSION_MINOR);
    CHECK_EQ_U64(v.patch, NESTURBATOR_VERSION_PATCH);
    CHECK_EQ_U64(v.abi, 1);
    CHECK_EQ_U64(v.behaviour_revision, 17);

    /* An older caller's smaller struct: only its bytes are written. */
    memset(&v, 0xAB, sizeof v);
    v.size = 8u;
    nesturbator_get_version(&v);
    CHECK_EQ_U64(v.size, 8);
    CHECK_EQ_U64(v.major, NESTURBATOR_VERSION_MAJOR);
    CHECK_EQ_HEX(v.minor, 0xABABABABu);

    /* Size 0 and NULL: nothing written, no crash. */
    memset(&v, 0xAB, sizeof v);
    v.size = 0u;
    nesturbator_get_version(&v);
    CHECK_EQ_HEX(v.major, 0xABABABABu);
    nesturbator_get_version(NULL);
}

static void test_cpu_ram_peek_is_read_only_and_mirrored(void)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    uint8_t value = 0xa5u;
    default_config(&cfg);
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_peek_cpu_ram(inst, 0x0000u, &value), NESTURBATOR_OK);
    CHECK_EQ_U64(value, 0u);
    CHECK_EQ_U64(nesturbator_peek_cpu_ram(inst, 0x1800u, &value), NESTURBATOR_OK);
    CHECK_EQ_U64(value, 0u);
    value = 0xa5u;
    CHECK_EQ_U64(nesturbator_peek_cpu_ram(inst, 0x2000u, &value), NESTURBATOR_ERR_ARGUMENT);
    CHECK_EQ_U64(value, 0xa5u);
    CHECK_EQ_U64(nesturbator_peek_cpu_ram(NULL, 0u, &value), NESTURBATOR_ERR_ARGUMENT);
    CHECK_EQ_U64(nesturbator_peek_cpu_ram(inst, 0u, NULL), NESTURBATOR_ERR_ARGUMENT);
    nesturbator_destroy(inst);
}

static void test_info(void)
{
    nesturbator_config cfg;
    default_config(&cfg);
    nesturbator *inst = NULL;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);

    nesturbator_info info;
    memset(&info, 0, sizeof info);
    info.size = (uint32_t)sizeof info;
    nesturbator_get_info(inst, &info);
    CHECK_EQ_U64(info.size, sizeof info);
    CHECK_EQ_U64(info.width, 256);
    CHECK_EQ_U64(info.height, 240);
    CHECK_EQ_U64(info.fps_num, 39375000);
    CHECK_EQ_U64(info.fps_den, 655171);
    CHECK_EQ_U64(info.sample_rate_num, 48000);
    CHECK_EQ_U64(info.sample_rate_den, 1);

    /* A caller with the original shorter prefix gets only those bytes. */
    union {
        nesturbator_info info;
        uint8_t bytes[sizeof(nesturbator_info) + 8u];
    } short_info;
    memset(&short_info, 0xa5, sizeof short_info);
    short_info.info.size = 8u;
    nesturbator_get_info(inst, &short_info.info);
    CHECK_EQ_U64(short_info.info.size, 8u);
    CHECK_EQ_U64(short_info.info.width, 256u);
    CHECK_EQ_HEX(short_info.bytes[8], 0xa5u);
    CHECK_EQ_HEX(short_info.bytes[sizeof(nesturbator_info)], 0xa5u);

    /* NULL instance: nothing written. */
    memset(&info, 0, sizeof info);
    info.size = (uint32_t)sizeof info;
    nesturbator_get_info(NULL, &info);
    CHECK_EQ_U64(info.width, 0);
    nesturbator_get_info(inst, NULL);

    nesturbator_destroy(inst);
    nesturbator_destroy(NULL);
}

static void make_region_image(uint8_t *image, uint8_t timing)
{
    const size_t size = 16u + 16384u + 8192u;
    memset(image, 0, size);
    memcpy(image, "NES\032", 4u);
    image[4] = 1u;
    image[5] = 1u;
    image[7] = 8u;
    image[12] = timing;
    image[16u + 0x3ffdu] = 0x80u;
}

static void test_region_api_auto_compatibility(void)
{
    static const uint8_t timings[] = {0u, 1u, 2u};
    const size_t size = 16u + 16384u + 8192u;
    uint8_t image[16u + 16384u + 8192u];
    nesturbator_config cfg;
    nesturbator *legacy = NULL, *additive = NULL;
    default_config(&cfg);
    CHECK_EQ_U64(NESTURBATOR_ABI_VERSION, 1u);
    CHECK_EQ_U64(nesturbator_create(&cfg, &legacy), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_create(&cfg, &additive), NESTURBATOR_OK);
    CHECK_EQ_U64(get_region_fn(NULL), NESTURBATOR_REGION_NTSC);
    CHECK_EQ_U64(get_region_fn(legacy), NESTURBATOR_REGION_NTSC);

    for (size_t i = 0u; i < sizeof timings / sizeof timings[0]; ++i) {
        make_region_image(image, timings[i]);
        CHECK_EQ_U64(nesturbator_load_cartridge(legacy, image, size), NESTURBATOR_OK);
        CHECK_EQ_U64(load_region_fn(additive, image, size, NESTURBATOR_REGION_PREFERENCE_AUTO),
                     NESTURBATOR_OK);
        CHECK_EQ_U64(get_region_fn(legacy), get_region_fn(additive));
        CHECK_EQ_U64(get_region_fn(legacy),
                     timings[i] == 1u ? NESTURBATOR_REGION_PAL : NESTURBATOR_REGION_NTSC);
    }

    nesturbator_unload_cartridge(legacy);
    CHECK_EQ_U64(get_region_fn(legacy), NESTURBATOR_REGION_NTSC);
    nesturbator_destroy(additive);
    nesturbator_destroy(legacy);
}

/* D-13: the save API through the public header only, with no cartridge. */
static void test_save_api_without_cartridge(void)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    uint8_t byte = 0u;
    uint8_t *data = &byte;
    size_t n = 9u;
    default_config(&cfg);
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &data, &n),
                 NESTURBATOR_OK);
    CHECK(data == NULL);
    CHECK_EQ_U64(n, 0u);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 0u);
    nesturbator_destroy(inst);
}

static void test_create_sizes(void)
{
    nesturbator *inst = NULL;
    nesturbator_config cfg;
    default_config(&cfg);

    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    CHECK(inst != NULL);
    nesturbator_destroy(inst);

    /* Size 0: NESTURBATOR_ERR_STRUCT_SIZE. */
    cfg.size = 0u;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_ERR_STRUCT_SIZE);

    /* Below the first released size (today's sizeof): ERR_STRUCT_SIZE. */
    cfg.size = (uint32_t)sizeof cfg - 1u;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_ERR_STRUCT_SIZE);

    /* Larger, with every unknown byte zero: accepted. */
    big_config big;
    make_big(&big);
    inst = NULL;
    CHECK_EQ_U64(nesturbator_create(&big.cfg, &inst), NESTURBATOR_OK);
    CHECK(inst != NULL);
    nesturbator_destroy(inst);

    /* Larger, with one non-zero byte past this build's struct:
       ERR_STRUCT_SIZE. */
    make_big(&big);
    big.bytes[sizeof(nesturbator_config) + TAIL_BYTES - 1u] = 1u;
    CHECK_EQ_U64(nesturbator_create(&big.cfg, &inst), NESTURBATOR_ERR_STRUCT_SIZE);
}

static void test_create_arguments(void)
{
    nesturbator *inst = NULL;
    nesturbator_config cfg;
    default_config(&cfg);

    /* NULL cfg or out: ERR_ARGUMENT. */
    CHECK_EQ_U64(nesturbator_create(NULL, &inst), NESTURBATOR_ERR_ARGUMENT);
    CHECK_EQ_U64(nesturbator_create(&cfg, NULL), NESTURBATOR_ERR_ARGUMENT);

    /* Another ABI: ERR_ABI. */
    cfg.abi = 2u;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_ERR_ABI);
    cfg.abi = NESTURBATOR_ABI_VERSION;

    /* Partly NULL allocators: ERR_ARGUMENT. */
    counter c;
    memset(&c, 0, sizeof c);
    cfg.allocator.alloc = counting_alloc;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_ERR_ARGUMENT);
    cfg.allocator.alloc = NULL;
    cfg.allocator.free = counting_free;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_ERR_ARGUMENT);
    cfg.allocator.free = NULL;
    cfg.allocator.user = &c;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_ERR_ARGUMENT);
    CHECK_EQ_U64(c.allocs, 0);
}

static void test_allocators(void)
{
    /* A failing allocator: ERR_NO_MEMORY, *out untouched, nothing freed. */
    static unsigned char marker;
    nesturbator *const untouched = (nesturbator *)(void *)&marker;
    nesturbator *inst = untouched;
    int frees = 0;
    nesturbator_config cfg;
    default_config(&cfg);
    cfg.allocator.alloc = failing_alloc;
    cfg.allocator.free = never_free;
    cfg.allocator.user = &frees;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_ERR_NO_MEMORY);
    CHECK(inst == untouched);
    CHECK_EQ_U64(frees, 0);

    /* A counting allocator: one alloc, one free of the same size, the user
       pointer passed through. */
    counter c;
    memset(&c, 0, sizeof c);
    cfg.allocator.alloc = counting_alloc;
    cfg.allocator.free = counting_free;
    cfg.allocator.user = &c;
    inst = NULL;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    CHECK(inst == (nesturbator *)(void *)c.storage.bytes);
    CHECK_EQ_U64(c.allocs, 1);
    CHECK_EQ_U64(c.frees, 0);

    /* Running frames allocates nothing. */
    static uint16_t video[256 * 240];
    static int16_t audio[1024];
    nesturbator_frame io;
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = video;
    io.video_pitch = 256u;
    io.audio = audio;
    io.audio_capacity = 1024u;
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
    CHECK_EQ_U64(c.allocs, 1);

    nesturbator_destroy(inst);
    CHECK_EQ_U64(c.allocs, c.frees);
    CHECK_EQ_U64(c.alloc_bytes, c.free_bytes);
    CHECK(c.alloc_bytes > 0u);
    CHECK_EQ_U64(c.in_use, 0);
}

int main(void)
{
    test_version();
    test_info();
    test_region_api_auto_compatibility();
    test_create_sizes();
    test_save_api_without_cartridge();
    test_create_arguments();
    test_allocators();
    test_cpu_ram_peek_is_read_only_and_mirrored();
    CHECK_DONE();
}
