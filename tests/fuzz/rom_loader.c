/* Fuzz and replay the public cartridge load/unload lifecycle. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "nesturbator.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    nesturbator *inst = NULL;
    nesturbator_config config = {0};
    config.size = (uint32_t)sizeof config;
    config.abi = NESTURBATOR_ABI_VERSION;
    if (nesturbator_create(&config, &inst) != NESTURBATOR_OK)
        return 0;
    (void)nesturbator_load_cartridge(inst, data, size);
    nesturbator_unload_cartridge(inst);
    nesturbator_destroy(inst);
    return 0;
}

#include "replay_main.h"
