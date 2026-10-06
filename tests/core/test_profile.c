/* core.profile: a new instance carries the NTSC RP2A03G machine profile,
   ANE and LXA constants of 0xEE (D-14). The profile is not public, so the
   test includes the internal header and reads the instance through it. */
#include <string.h>

#include "internal.h"
#include "../check.h"

int main(void)
{
    nesturbator_config cfg;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;

    nesturbator *inst = NULL;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    CHECK(inst != NULL);
    if (inst != NULL) {
        const struct nesturbator *nes = (const struct nesturbator *)inst;
        CHECK_EQ_HEX(nes->profile.ane_magic, 0xEEu);
        CHECK_EQ_HEX(nes->profile.lxa_magic, 0xEEu);
        nesturbator_destroy(inst);
    }
    CHECK_DONE();
}
