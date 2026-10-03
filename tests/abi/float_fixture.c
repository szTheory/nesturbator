/* Deliberate floating point for abi.float_fixture: built with the core's
   -mgeneral-regs-only, this must either fail to compile or leave soft-float
   calls that the undefined-symbol check rejects. Never part of the core. */
#include <stdint.h>

uint32_t nesturbator_float_fixture(uint32_t x);

uint32_t nesturbator_float_fixture(uint32_t x)
{
    double a = (double)x;
    return (uint32_t)(a * 1.5);
}
