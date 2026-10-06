/* Test definitions of the CPU's two bus functions: flat RAM and a cycle log. */
#include "vector_bus.h"

/* A pointer to a structure, suitably converted, points to its first member,
   and back (C17 6.7.2.1p15); nes is the first member of vector_machine. */
static struct vector_machine *machine(struct nesturbator *nes)
{
    return (struct vector_machine *)(void *)nes;
}

static void log_cycle(struct vector_machine *m, uint16_t addr, uint8_t value, uint8_t kind)
{
    if (m->log_count >= VECTOR_LOG_CAPACITY) {
        m->log_overflow = 1u;
        return;
    }
    struct n65v_cycle *c = &m->log[m->log_count];
    c->addr = addr;
    c->value = value;
    c->kind = kind;
    m->log_count++;
}

uint8_t nesturbator__bus_read(struct nesturbator *nes, uint16_t addr)
{
    struct vector_machine *m = machine(nes);
    uint8_t v = m->ram[addr];
    log_cycle(m, addr, v, N65V_KIND_READ);
    return v;
}

void nesturbator__bus_write(struct nesturbator *nes, uint16_t addr, uint8_t value)
{
    struct vector_machine *m = machine(nes);
    m->ram[addr] = value;
    log_cycle(m, addr, value, N65V_KIND_WRITE);
}

void vector_machine_reset_log(struct vector_machine *m)
{
    m->log_count = 0u;
    m->log_overflow = 0u;
}
