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

static void apply_irq_events(struct vector_machine *m, uint32_t access, uint8_t after_open)
{
    if (m->irq_schedule_enabled == 0u)
        return;
    while (m->irq_event_next < m->irq_event_count) {
        const struct vector_irq_event *event = &m->irq_events[m->irq_event_next];
        if (event->access != access || event->after_open != after_open)
            break;
        m->nes.cpu.irq_line = event->line;
        m->irq_event_next++;
    }
}

static uint32_t open_irq_cycle(struct vector_machine *m)
{
    uint32_t index = m->log_count;
    if (m->irq_schedule_enabled != 0u) {
        m->nes.cpu.poll_latch = m->nes.cpu.irq_captured;
        if (index < VECTOR_LOG_CAPACITY) {
            m->poll_sample[index] = m->nes.cpu.poll_latch;
            m->captured_at_poll[index] = m->nes.cpu.irq_captured;
            m->line_at_poll[index] = m->nes.cpu.irq_line;
        }
        apply_irq_events(m, index + 1u, 1u);
    }
    return index;
}

static void close_irq_cycle(struct vector_machine *m, uint32_t index)
{
    if (m->irq_schedule_enabled == 0u)
        return;
    apply_irq_events(m, index + 1u, 0u);
    m->nes.cpu.irq_captured = m->nes.cpu.irq_line;
    if (index < VECTOR_LOG_CAPACITY) {
        m->line_after_access[index] = m->nes.cpu.irq_line;
        m->captured_after_access[index] = m->nes.cpu.irq_captured;
    }
}

uint8_t nesturbator__bus_read(struct nesturbator *nes, uint16_t addr)
{
    struct vector_machine *m = machine(nes);
    uint32_t index = open_irq_cycle(m);
    uint8_t v = m->ram[addr];
    log_cycle(m, addr, v, N65V_KIND_READ);
    if (m->irq_schedule_enabled != 0u && m->irq_ack_enabled != 0u && addr == m->irq_ack_address) {
        m->irq_ack_count++;
        m->nes.cpu.irq_line = 0u;
    }
    close_irq_cycle(m, index);
    return v;
}

void nesturbator__bus_write(struct nesturbator *nes, uint16_t addr, uint8_t value)
{
    struct vector_machine *m = machine(nes);
    uint32_t index = open_irq_cycle(m);
    m->ram[addr] = value;
    log_cycle(m, addr, value, N65V_KIND_WRITE);
    close_irq_cycle(m, index);
}

void vector_machine_reset_log(struct vector_machine *m)
{
    m->log_count = 0u;
    m->log_overflow = 0u;
}

void vector_irq_schedule(struct vector_machine *m, uint32_t access, uint8_t after_open,
                         uint8_t line)
{
    if (m->irq_event_count >= VECTOR_IRQ_EVENT_CAPACITY) {
        m->irq_event_overflow = 1u;
        return;
    }
    struct vector_irq_event event = {access, (uint8_t)(after_open != 0u), (uint8_t)(line != 0u)};
    uint8_t insert = m->irq_event_count;
    while (insert > 0u) {
        const struct vector_irq_event *previous = &m->irq_events[insert - 1u];
        if (previous->access < event.access ||
            (previous->access == event.access && previous->after_open >= event.after_open))
            break;
        m->irq_events[insert] = *previous;
        insert--;
    }
    m->irq_events[insert] = event;
    m->irq_event_count++;
    m->irq_schedule_enabled = 1u;
}

void vector_irq_ack_on_read(struct vector_machine *m, uint16_t address)
{
    m->irq_schedule_enabled = 1u;
    m->irq_ack_enabled = 1u;
    m->irq_ack_address = address;
}
