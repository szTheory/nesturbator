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

int main(void)
{
    /* The initialiser lists every member: no missing-initializer warning. */
    nesturbator_config c = NESTURBATOR_CONFIG_INIT;
    return c.abi == NESTURBATOR_ABI_VERSION && c.size == sizeof c ? 0 : 1;
}
