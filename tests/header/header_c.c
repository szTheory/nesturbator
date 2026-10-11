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

int main(void)
{
    /* The initialiser lists every member: no missing-initializer warning. */
    nesturbator_config c = NESTURBATOR_CONFIG_INIT;
    /* The typed initialisers above prove both signatures; a null test on a
       const function pointer is always true and gcc -Waddress may reject it. */
    (void)get_memory_fn;
    (void)save_generation_fn;
    return c.abi == NESTURBATOR_ABI_VERSION && c.size == sizeof c &&
                   save_kind == NESTURBATOR_MEMORY_SAVE_RAM
               ? 0
               : 1;
}
