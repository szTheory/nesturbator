/* The test bus for the vector program (D-08, D-11): flat 64 KiB RAM and a
   log of every bus cycle. It defines nesturbator__bus_read and
   nesturbator__bus_write in place of src/bus.c, so a program that links it
   must not link the library. */
#ifndef NESTURBATOR_VECTOR_BUS_H
#define NESTURBATOR_VECTOR_BUS_H

#include <stdint.h>

#include "internal.h"
#include "n65v.h"

#define VECTOR_LOG_CAPACITY 32u

/* nes must stay the first member: the bus functions receive &m->nes and
   convert it back to the machine. */
struct vector_machine {
    struct nesturbator nes;
    uint8_t ram[65536];
    struct n65v_cycle log[VECTOR_LOG_CAPACITY];
    uint32_t log_count;
    uint8_t log_overflow; /* set when a cycle arrived with the log full */
};

/* Empties the cycle log and clears its overflow flag. */
void vector_machine_reset_log(struct vector_machine *m);

#endif
