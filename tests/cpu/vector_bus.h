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
#define VECTOR_IRQ_EVENT_CAPACITY 8u

struct vector_irq_event {
    uint32_t access;    /* one-based bus access number */
    uint8_t after_open; /* apply after the scheduled poll sample */
    uint8_t line;
};

/* nes must stay the first member: the bus functions receive &m->nes and
   convert it back to the machine. */
struct vector_machine {
    struct nesturbator nes;
    uint8_t ram[65536];
    struct n65v_cycle log[VECTOR_LOG_CAPACITY];
    uint32_t log_count;
    uint8_t log_overflow; /* set when a cycle arrived with the log full */
    /* Optional CPU-only schedule. This is deliberately not a production
       source sampler: tests inject IRQ line transitions at flat-bus actions. */
    uint8_t irq_schedule_enabled;
    struct vector_irq_event irq_events[VECTOR_IRQ_EVENT_CAPACITY];
    uint8_t irq_event_count;
    uint8_t irq_event_overflow;
    uint8_t irq_event_next;
    uint8_t irq_ack_enabled;
    uint16_t irq_ack_address;
    uint32_t irq_ack_count;
    uint8_t poll_sample[VECTOR_LOG_CAPACITY];
    uint8_t captured_at_poll[VECTOR_LOG_CAPACITY];
    uint8_t line_at_poll[VECTOR_LOG_CAPACITY];
    uint8_t line_after_access[VECTOR_LOG_CAPACITY];
    uint8_t captured_after_access[VECTOR_LOG_CAPACITY];
};

/* Empties the cycle log and clears its overflow flag. */
void vector_machine_reset_log(struct vector_machine *m);

/* Inject a level change during one flat-bus access. The opening sample has
   already happened when after_open is nonzero. */
void vector_irq_schedule(struct vector_machine *m, uint32_t access, uint8_t after_open,
                         uint8_t line);

/* Treat reads of address as test-bus acknowledgements that lower IRQ. */
void vector_irq_ack_on_read(struct vector_machine *m, uint16_t address);

#endif
