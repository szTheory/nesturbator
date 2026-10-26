/* D-11: the public header compiles alone as C17 under -pedantic -Werror, and
   its layout holds. Building this file is the test; main only links it. */
#include "nesturbator.h"

/* Every boundary struct starts with its size tag (D-07). */
_Static_assert(offsetof(nesturbator_version, size) == 0, "version size first");
_Static_assert(offsetof(nesturbator_config, size) == 0, "config size first");
_Static_assert(offsetof(nesturbator_info, size) == 0, "info size first");
_Static_assert(offsetof(nesturbator_frame, size) == 0, "frame size first");
/* The 64-bit counters are naturally aligned for every host. */
_Static_assert(offsetof(nesturbator_frame, frame_number) % 8 == 0, "frame_number aligned");
/* Structs of uint32_t only have no padding. */
_Static_assert(sizeof(nesturbator_version) == 24, "version is 6 x uint32_t");
_Static_assert(sizeof(nesturbator_info) == 28, "info is 7 x uint32_t");

/* The save API is declared: the kind constant, its typedef and both functions. */
static nesturbator_status (*const get_memory_fn)(nesturbator *, nesturbator_memory, uint8_t **,
                                                 size_t *) = nesturbator_get_memory;
static uint64_t (*const save_generation_fn)(const nesturbator *) = nesturbator_save_generation;
static const nesturbator_memory save_kind = NESTURBATOR_MEMORY_SAVE_RAM;

/* The save-state API is declared with its format version and last status. */
static size_t (*const state_size_fn)(const nesturbator *) = nesturbator_state_size;
static nesturbator_status (*const save_state_fn)(const nesturbator *, void *,
                                                 size_t) = nesturbator_save_state;
static nesturbator_status (*const load_state_fn)(nesturbator *, const void *,
                                                 size_t) = nesturbator_load_state;
_Static_assert(NESTURBATOR_STATE_FORMAT_VERSION == 2, "state format version");
_Static_assert(NESTURBATOR_ERR_STATE_INVALID == 12, "last status value");

int main(void)
{
    /* The initialiser lists every member: no missing-initializer warning. */
    nesturbator_config c = NESTURBATOR_CONFIG_INIT;
    /* The typed initialisers above prove both signatures; a null test on a
       const function pointer is always true and gcc -Waddress may reject it. */
    (void)get_memory_fn;
    (void)save_generation_fn;
    (void)state_size_fn;
    (void)save_state_fn;
    (void)load_state_fn;
    return c.abi == NESTURBATOR_ABI_VERSION && c.size == sizeof c &&
                   save_kind == NESTURBATOR_MEMORY_SAVE_RAM
               ? 0
               : 1;
}
