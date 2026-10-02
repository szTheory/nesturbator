// D-11: the public header compiles alone as C++ with warnings as errors, and
// its layout holds. Building this file is the test; main only links it.
#include "nesturbator.h"

// Every boundary struct starts with its size tag (D-07).
static_assert(offsetof(nesturbator_version, size) == 0, "version size first");
static_assert(offsetof(nesturbator_config, size) == 0, "config size first");
static_assert(offsetof(nesturbator_info, size) == 0, "info size first");
static_assert(offsetof(nesturbator_frame, size) == 0, "frame size first");
// The 64-bit counters are naturally aligned for every host.
static_assert(offsetof(nesturbator_frame, frame_number) % 8 == 0, "frame_number aligned");
// Structs of uint32_t only have no padding.
static_assert(sizeof(nesturbator_version) == 24, "version is 6 x uint32_t");
static_assert(sizeof(nesturbator_info) == 28, "info is 7 x uint32_t");

int main()
{
    // Brace initialisation from the macro must not narrow (01-RESEARCH
    // Pitfall 6).
    nesturbator_config c = NESTURBATOR_CONFIG_INIT;
    return c.abi == NESTURBATOR_ABI_VERSION && c.size == sizeof c ? 0 : 1;
}
