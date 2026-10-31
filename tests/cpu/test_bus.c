/* bus.unit: ordinary linked bus checks and compile-time observation of the
   production bus/CPU sources. One ordinary access is 24 ticks; DMA stalls
   are observed separately, including their actual mapper sample GET. */
#include <string.h>

#include "../check.h"
#include "internal.h"

enum dma_event_kind {
    NESTURBATOR_DMA_APU_BEFORE = 1,
    NESTURBATOR_DMA_APU_AFTER,
    NESTURBATOR_DMA_HALT_ATTEMPT,
    NESTURBATOR_DMA_HALT_ACCEPTED,
    NESTURBATOR_DMA_PARKED_HALT,
    NESTURBATOR_DMA_PARKED_DUMMY,
    NESTURBATOR_DMA_PARKED_ALIGN,
    NESTURBATOR_DMA_SAMPLE_GET,
    NESTURBATOR_DMA_RESUMED_READ,
    NESTURBATOR_DMA_CPU_READ_COMPLETE,
    NESTURBATOR_DMA_WRITE_RETRY,
    NESTURBATOR_DMA_CPU_WRITE_COMPLETE,
    NESTURBATOR_DMA_OAM_SOURCE,
    NESTURBATOR_DMA_OAM_SOURCE_COMPLETE,
    NESTURBATOR_DMA_OAM_DEST
};

#define DMA_TRACE_CAPACITY 32768u
struct dma_event {
    uint8_t kind;
    uint16_t address, detail;
    uint8_t data, phase, pending, halt_phase, open_bus, buffer_empty, controller_shift, bits;
    uint8_t enable_delay, load_waiting;
    uint8_t frame_irq, frame_irq_clear_pending;
    uint16_t remaining, dmc_counter, dmc_timer;
    uint64_t cpu_cycle, ticks;
};

static struct dma_event dma_trace[DMA_TRACE_CAPACITY];
static size_t dma_trace_count;

static void dma_observe(enum dma_event_kind kind, const struct nesturbator *machine,
                        uint16_t address, uint8_t data, uint16_t detail)
{
    if (dma_trace_count == DMA_TRACE_CAPACITY)
        return;
    struct dma_event *event = &dma_trace[dma_trace_count++];
    event->kind = (uint8_t)kind;
    event->address = address;
    event->detail = detail;
    event->data = data;
    event->phase = machine->bus.apu_get_put_phase;
    event->pending = machine->apu.dmc.dma_pending;
    event->halt_phase = machine->apu.dmc.dma_halt_phase;
    event->open_bus = machine->bus.open_bus;
    event->remaining = machine->apu.dmc.remaining;
    event->buffer_empty = machine->apu.dmc.buffer_empty;
    event->bits = machine->apu.dmc.bits;
    event->enable_delay = machine->apu.dmc.enable_delay;
    event->load_waiting = machine->apu.dmc.dma_load_waiting;
    event->dmc_counter = machine->apu.dmc.counter;
    event->dmc_timer = machine->apu.dmc.timer;
    event->controller_shift = machine->bus.controller_shift[0];
    event->frame_irq = machine->apu.frame_irq;
    event->frame_irq_clear_pending = machine->apu.frame_irq_clear_pending;
    event->cpu_cycle = machine->cpu_cycle;
    event->ticks = machine->ticks;
}

#define NESTURBATOR_DMA_OBSERVE(kind, machine, address, data, detail)                              \
    dma_observe((enum dma_event_kind)(kind), (machine), (uint16_t)(address), (uint8_t)(data),      \
                (uint16_t)(detail))
uint8_t test_observed_bus_read(struct nesturbator *machine, uint16_t address);
void test_observed_bus_write(struct nesturbator *machine, uint16_t address, uint8_t data);
#define nesturbator__bus_read test_observed_bus_read
#define nesturbator__bus_write test_observed_bus_write
#include "../../src/bus.c"
#undef nesturbator__bus_read
#undef nesturbator__bus_write
#undef NESTURBATOR_DMA_OBSERVE

/* Run source-shaped landing snippets through the production CPU decode while
   routing its bus accesses through the same test-local observer. */
void test_observed_cpu_step(struct nesturbator *machine);
void test_observed_cpu_reset(struct nesturbator *machine);
void test_observed_cpu_entry_begin(struct nesturbator *machine,
                                   enum nesturbator_entry_origin origin);
uint8_t test_observed_cpu_entry_action(struct nesturbator *machine);
void test_observed_cpu_entry_drain(struct nesturbator *machine);
#define nesturbator__bus_read test_observed_bus_read
#define nesturbator__bus_write test_observed_bus_write
#define nesturbator__cpu_step test_observed_cpu_step
#define nesturbator__cpu_reset test_observed_cpu_reset
#define nesturbator__cpu_entry_begin test_observed_cpu_entry_begin
#define nesturbator__cpu_entry_action test_observed_cpu_entry_action
#define nesturbator__cpu_entry_drain test_observed_cpu_entry_drain
#include "../../src/cpu.c"
#undef nesturbator__cpu_entry_drain
#undef nesturbator__cpu_entry_action
#undef nesturbator__cpu_entry_begin
#undef nesturbator__cpu_reset
#undef nesturbator__cpu_step
#undef nesturbator__bus_write
#undef nesturbator__bus_read

static struct nesturbator nes;
static uint8_t landing_prg[16u * 1024u];

static void prepare_landing(struct nesturbator *machine, uint8_t initial_phase,
                            uint8_t request_load)
{
    memset(machine, 0, sizeof *machine);
    machine->cart.prg = landing_prg;
    machine->cart.prg_size = sizeof landing_prg;
    for (unsigned page = 16u; page < 48u; ++page)
        machine->map.cpu_r[page] = landing_prg + ((page - 16u) & 15u) * 0x400u;
    machine->bus.apu_get_put_phase = initial_phase;
    machine->cpu_cycle = initial_phase;
    machine->ticks = (uint64_t)initial_phase * 24u;
    machine->bus.ram[0x20u] = 0x5Au;
    machine->bus.open_bus = 0x33u;
    machine->apu.pulse_clock_phase = initial_phase;
    machine->apu.dmc.address = 0xC000u;
    machine->apu.dmc.remaining = 1u;
    machine->apu.dmc.buffer_empty = 1u;
    machine->apu.dmc.counter = 100u;
    machine->apu.dmc.dma_pending = 1u;
    machine->apu.dmc.dma_halt_phase = request_load != 0u ? 1u : 0u;
    landing_prg[0] = 0xA7u;
}

static size_t count_dma_events(enum dma_event_kind kind)
{
    size_t count = 0u;
    for (size_t i = 0u; i < dma_trace_count; ++i)
        count += dma_trace[i].kind == kind;
    return count;
}

static void test9_put(uint16_t address, uint8_t value)
{
    landing_prg[(size_t)(address - 0x8000u)] = value;
}

static void test9_emit8(uint16_t *pc, uint8_t value)
{
    test9_put((*pc)++, value);
}

static void test9_emit16(uint16_t *pc, uint16_t value)
{
    test9_emit8(pc, (uint8_t)value);
    test9_emit8(pc, (uint8_t)(value >> 8));
}

static void test9_emit_jsr(uint16_t *pc, uint16_t target)
{
    test9_emit8(pc, 0x20u);
    test9_emit16(pc, target);
}

static void test9_emit_abs(uint16_t *pc, uint8_t opcode, uint16_t address)
{
    test9_emit8(pc, opcode);
    test9_emit16(pc, address);
}

static void test9_emit_slide(uint16_t address, uint16_t total_cycles)
{
    uint16_t pc = address;
    uint16_t body_cycles = (uint16_t)(total_cycles - 12u); /* JSR and RTS */
    if ((body_cycles & 1u) != 0u) {
        test9_emit8(&pc, 0x24u); /* BIT $00: three cycles, A is unchanged. */
        test9_emit8(&pc, 0x00u);
        body_cycles = (uint16_t)(body_cycles - 3u);
    }
    while (body_cycles != 0u) {
        test9_emit8(&pc, 0xEAu); /* NOP: two cycles. */
        body_cycles = (uint16_t)(body_cycles - 2u);
    }
    test9_emit8(&pc, 0x60u);
}

static void test9_emit_frame_irq_wait(uint16_t address)
{
    uint16_t pc = address;
    test9_emit8(&pc, 0xA9u); /* The source helper resets $4017 before its delay. */
    test9_emit8(&pc, 0u);
    test9_emit_abs(&pc, 0x8Du, 0x4017u);
    test9_emit_jsr(&pc, 0x8900u);
    test9_emit8(&pc, 0x60u);
    pc = 0x8900u;
    test9_emit8(&pc, 0xA0u); /* LDY #100 */
    test9_emit8(&pc, 100u);
    uint16_t outer = pc;
    test9_emit8(&pc, 0xA2u); /* LDX #58 */
    test9_emit8(&pc, 58u);
    uint16_t inner = pc;
    test9_emit8(&pc, 0xCAu); /* DEX */
    test9_emit8(&pc, 0xD0u); /* BNE inner */
    uint16_t inner_operand = pc++;
    test9_emit8(&pc, 0x88u); /* DEY */
    test9_emit8(&pc, 0xD0u); /* BNE outer */
    uint16_t outer_operand = pc++;
    /* 29,601 loop cycles + 387 padding + JSR/RTS = exactly 30,000.
       There is no enabled DMA or NMI in this pre-sync delay. */
    test9_emit8(&pc, 0x24u);
    test9_emit8(&pc, 0u);
    for (unsigned i = 0u; i < 192u; ++i)
        test9_emit8(&pc, 0xEAu);
    test9_emit8(&pc, 0x60u); /* RTS */
    landing_prg[(size_t)(inner_operand - 0x8000u)] = (uint8_t)(inner - (inner_operand + 1u));
    landing_prg[(size_t)(outer_operand - 0x8000u)] = (uint8_t)(outer - (outer_operand + 1u));
}

static uint16_t test9_emit_dmasync(void)
{
    const uint16_t base = 0x8200u;
    uint16_t pc = base;
    test9_emit8(&pc, 0xA5u); /* LDA <result_DMCDMASync_PreTest */
    test9_emit8(&pc, 0x12u);
    test9_emit8(&pc, 0xF0u); /* BEQ open-bus self-test (not selected). */
    uint16_t first_beq_operand = pc++;
    test9_emit8(&pc, 0xC9u); /* CMP #1 */
    test9_emit8(&pc, 0x01u);
    test9_emit8(&pc, 0xF0u); /* BEQ DMASync_TheGoodOne */
    uint16_t good_beq_operand = pc++;
    test9_emit8(&pc, 0x4Cu); /* JMP unselected fallback. */
    uint16_t fallback_jmp_operand = pc;
    test9_emit16(&pc, 0u);

    uint16_t good = pc;
    test9_emit8(&pc, 0x85u); /* STA Copy_A ($13). */
    test9_emit8(&pc, 0x13u);
    test9_emit8(&pc, 0xA9u); /* LDA #$4F */
    test9_emit8(&pc, 0x4Fu);
    test9_emit_abs(&pc, 0x8Du, 0x4010u);
    test9_emit8(&pc, 0xA9u); /* LDA #0 */
    test9_emit8(&pc, 0x00u);
    test9_emit_abs(&pc, 0x8Du, 0x4011u);
    test9_emit8(&pc, 0xA9u); /* LDA #$FF */
    test9_emit8(&pc, 0xFFu);
    test9_emit_abs(&pc, 0x8Du, 0x4012u);
    test9_emit8(&pc, 0xA9u); /* LDA #0 */
    test9_emit8(&pc, 0x00u);
    test9_emit_abs(&pc, 0x8Du, 0x4013u);
    test9_emit8(&pc, 0xA9u); /* LDA #$10 */
    test9_emit8(&pc, 0x10u);
    test9_emit_abs(&pc, 0x8Du, 0x4015u);
    test9_emit8(&pc, 0xEAu); /* NOP */
    test9_emit8(&pc, 0xEAu); /* NOP */
    uint16_t loop = pc;
    test9_emit_abs(&pc, 0xADu, 0x4000u); /* LDA open bus, wait for the DMC GET. */
    test9_emit8(&pc, 0xD0u);             /* BNE loop */
    int16_t loop_delta = (int16_t)loop - (int16_t)(pc + 1u);
    test9_emit8(&pc, (uint8_t)loop_delta);
    test9_emit8(&pc, 0xA9u); /* LDA #$0F: stop looping. */
    test9_emit8(&pc, 0x0Fu);
    test9_emit_abs(&pc, 0x8Du, 0x4010u);
    test9_emit8(&pc, 0xA5u); /* LDA $00 */
    test9_emit8(&pc, 0x00u);
    test9_emit_abs(&pc, 0xADu, 0x0013u); /* LDA Copy_A (absolute, four cycles). */
    test9_emit8(&pc, 0x60u);             /* RTS */
    uint16_t fallback = pc;
    test9_emit8(&pc, 0x85u); /* TEST_DoesTheDMAUpdateOpenBus: save A. */
    test9_emit8(&pc, 0x13u);
    test9_emit8(&pc, 0xA9u); /* LDA #$4F */
    test9_emit8(&pc, 0x4Fu);
    test9_emit_abs(&pc, 0x8Du, 0x4010u);
    test9_emit8(&pc, 0xA9u); /* LDA #0 */
    test9_emit8(&pc, 0x00u);
    test9_emit_abs(&pc, 0x8Du, 0x4011u);
    test9_emit8(&pc, 0xA9u); /* LDA #$FF */
    test9_emit8(&pc, 0xFFu);
    test9_emit_abs(&pc, 0x8Du, 0x4012u);
    test9_emit8(&pc, 0xA9u); /* LDA #0 */
    test9_emit8(&pc, 0x00u);
    test9_emit_abs(&pc, 0x8Du, 0x4013u);
    test9_emit8(&pc, 0xA9u); /* LDA #$10 */
    test9_emit8(&pc, 0x10u);
    test9_emit_abs(&pc, 0x8Du, 0x4015u);
    test9_emit8(&pc, 0xA2u); /* LDX #0 */
    test9_emit8(&pc, 0x00u);
    test9_emit8(&pc, 0xA0u); /* LDY #0 */
    test9_emit8(&pc, 0x00u);
    uint16_t open_bus_loop = pc;
    test9_emit8(&pc, 0xCAu); /* DEX */
    test9_emit8(&pc, 0xD0u); /* BNE loop post-decrement. */
    uint16_t x_branch_operand = pc++;
    test9_emit8(&pc, 0x88u); /* DEY */
    test9_emit8(&pc, 0xF0u); /* BEQ fail after all attempts. */
    uint16_t y_branch_operand = pc++;
    uint16_t loop_post_decrement = pc;
    test9_emit_abs(&pc, 0xADu, 0x4000u);
    test9_emit8(&pc, 0xD0u); /* BNE loop until DMA updates open bus. */
    uint16_t open_bus_branch_operand = pc++;
    test9_emit_abs(&pc, 0xADu, 0x4000u);
    test9_emit8(&pc, 0xF0u); /* BEQ fail if the next open-bus read is 0. */
    uint16_t verify_branch_operand = pc++;
    test9_emit8(&pc, 0xA9u); /* LDA #1; store successful pre-test result. */
    test9_emit8(&pc, 0x01u);
    test9_emit8(&pc, 0x85u);
    test9_emit8(&pc, 0x12u);
    test9_emit8(&pc, 0x4Cu); /* JMP DMASync. */
    test9_emit16(&pc, base);
    uint16_t fail = pc;
    test9_emit8(&pc, 0xA9u); /* LDA #2; the unselected failure outcome. */
    test9_emit8(&pc, 0x02u);
    test9_emit8(&pc, 0x85u);
    test9_emit8(&pc, 0x12u);
    test9_emit8(&pc, 0x4Cu);
    test9_emit16(&pc, base);

    landing_prg[(size_t)(first_beq_operand - 0x8000u)] =
        (uint8_t)(fallback - (first_beq_operand + 1u));
    landing_prg[(size_t)(good_beq_operand - 0x8000u)] = (uint8_t)(good - (good_beq_operand + 1u));
    landing_prg[(size_t)(fallback_jmp_operand - 0x8000u)] = (uint8_t)fallback;
    landing_prg[(size_t)(fallback_jmp_operand - 0x8000u + 1u)] = (uint8_t)(fallback >> 8);
    landing_prg[(size_t)(x_branch_operand - 0x8000u)] =
        (uint8_t)(loop_post_decrement - (x_branch_operand + 1u));
    landing_prg[(size_t)(y_branch_operand - 0x8000u)] = (uint8_t)(fail - (y_branch_operand + 1u));
    landing_prg[(size_t)(open_bus_branch_operand - 0x8000u)] =
        (uint8_t)(open_bus_loop - (open_bus_branch_operand + 1u));
    landing_prg[(size_t)(verify_branch_operand - 0x8000u)] =
        (uint8_t)(fail - (verify_branch_operand + 1u));
    return base;
}

static uint16_t test9_emit_program(uint16_t *final_read)
{
    const uint16_t sync50 = 0x8100u;
    const uint16_t sync = test9_emit_dmasync();
    const uint16_t slide50 = 0x8300u;
    const uint16_t slide350 = 0x8400u;
    const uint16_t slide422 = 0x8500u;
    const uint16_t slide432 = 0x8600u;
    const uint16_t slide409 = 0x8700u;
    const uint16_t frame_wait = 0x8800u;

    uint16_t pc = 0x8000u;
    /* Test 8's frame-IRQ/OAM/write sequence immediately before Test 9. The
       helper's 30,000-cycle delay has an independently checked duration. */
    test9_emit8(&pc, 0xA9u);
    test9_emit8(&pc, 0x00u);
    test9_emit_abs(&pc, 0x8Du, 0x4015u); /* Disable DMC. */
    test9_emit_jsr(&pc, frame_wait);
    test9_emit8(&pc, 0xA9u);
    test9_emit8(&pc, 0x10u);
    test9_emit_abs(&pc, 0x8Du, 0x4014u); /* Align using OAM DMA. */
    test9_emit8(&pc, 0xA5u);
    test9_emit8(&pc, 0x00u); /* LDA $00, three cycles. */
    test9_emit_abs(&pc, 0x8Du, 0x4015u);
    test9_emit_abs(&pc, 0x8Du, 0x4015u);
    test9_emit_abs(&pc, 0xADu, 0x4015u);
    test9_emit8(&pc, 0x29u); /* AND #$40; preserve the source assertion. */
    test9_emit8(&pc, 0x40u);
    test9_emit8(&pc, 0xF0u); /* BEQ fail if frame IRQ was cleared. */
    uint16_t test8_fail_operand = pc++;
    test9_emit8(&pc, 0xE6u); /* INC ErrorCode (RAM $11). */
    test9_emit8(&pc, 0x11u);
    test9_emit_jsr(&pc, sync50);
    test9_emit_jsr(&pc, slide50);
    test9_emit8(&pc, 0xA9u);
    test9_emit8(&pc, 0x00u);
    test9_emit_abs(&pc, 0x8Du, 0x4015u);
    test9_emit_jsr(&pc, slide422);
    test9_emit_jsr(&pc, slide432);
    test9_emit8(&pc, 0xA9u);
    test9_emit8(&pc, 0x4Fu);
    test9_emit_abs(&pc, 0x8Du, 0x4010u);
    test9_emit8(&pc, 0xA9u);
    test9_emit8(&pc, 0x10u);
    test9_emit_abs(&pc, 0x8Du, 0x4015u);
    test9_emit_abs(&pc, 0x8Du, 0x5000u);
    test9_emit_jsr(&pc, slide409);
    *final_read = pc;
    test9_emit_abs(&pc, 0xADu, 0x4000u);
    test9_emit8(&pc, 0xD0u); /* Pinned assertion: BNE on nonzero open bus fails. */
    uint16_t test9_fail_operand = pc++;
    uint16_t fail = pc;
    test9_emit8(&pc, 0x02u); /* JAM is an observable stop for either failed assertion. */
    landing_prg[(size_t)(test8_fail_operand - 0x8000u)] =
        (uint8_t)(fail - (test8_fail_operand + 1u));
    landing_prg[(size_t)(test9_fail_operand - 0x8000u)] =
        (uint8_t)(fail - (test9_fail_operand + 1u));

    pc = frame_wait;
    test9_emit_frame_irq_wait(frame_wait);
    test9_emit_slide(slide50, 50u);
    test9_emit_slide(slide350, 350u);
    test9_emit_slide(slide422, 422u);
    test9_emit_slide(slide432, 432u);
    test9_emit_slide(slide409, 409u);
    pc = sync50;
    test9_emit_jsr(&pc, sync);
    test9_emit_jsr(&pc, slide350);
    test9_emit8(&pc, 0x60u);
    return 0x8000u;
}

static const struct dma_event *find_dma_event(enum dma_event_kind kind)
{
    for (size_t i = 0u; i < dma_trace_count; ++i) {
        if (dma_trace[i].kind == kind)
            return &dma_trace[i];
    }
    return NULL;
}

static const char *dma_event_name(enum dma_event_kind kind)
{
    switch (kind) {
    case NESTURBATOR_DMA_APU_BEFORE:
        return "apu-before";
    case NESTURBATOR_DMA_APU_AFTER:
        return "apu-after";
    case NESTURBATOR_DMA_HALT_ATTEMPT:
        return "halt-attempt";
    case NESTURBATOR_DMA_HALT_ACCEPTED:
        return "halt-accepted";
    case NESTURBATOR_DMA_PARKED_HALT:
        return "parked-halt";
    case NESTURBATOR_DMA_PARKED_DUMMY:
        return "parked-dummy";
    case NESTURBATOR_DMA_PARKED_ALIGN:
        return "parked-align";
    case NESTURBATOR_DMA_SAMPLE_GET:
        return "sample-get";
    case NESTURBATOR_DMA_RESUMED_READ:
        return "resumed-read";
    case NESTURBATOR_DMA_CPU_READ_COMPLETE:
        return "cpu-read";
    case NESTURBATOR_DMA_WRITE_RETRY:
        return "write-retry";
    case NESTURBATOR_DMA_CPU_WRITE_COMPLETE:
        return "cpu-write";
    case NESTURBATOR_DMA_OAM_SOURCE:
        return "oam-source";
    case NESTURBATOR_DMA_OAM_SOURCE_COMPLETE:
        return "oam-source-complete";
    case NESTURBATOR_DMA_OAM_DEST:
        return "oam-dest";
    }
    return "unknown";
}

static void print_dma_trace(uint8_t initial_phase, uint8_t write_count, uint8_t request_load)
{
    const char *request = request_load == 2u ? "4015-load" : request_load != 0u ? "load" : "reload";
    printf("landing phase=%u writes=%u request=%s events=%lu\n", (unsigned)initial_phase,
           (unsigned)write_count, request, (unsigned long)dma_trace_count);
    for (size_t i = 0u; i < dma_trace_count; ++i) {
        const struct dma_event *event = &dma_trace[i];
        uint8_t is_request =
            (uint8_t)(event->kind == NESTURBATOR_DMA_APU_AFTER && event->pending != 0u && i != 0u &&
                      dma_trace[i - 1u].kind == NESTURBATOR_DMA_APU_BEFORE &&
                      dma_trace[i - 1u].cpu_cycle == event->cpu_cycle &&
                      dma_trace[i - 1u].pending == 0u);
        if (event->kind == NESTURBATOR_DMA_APU_BEFORE ||
            (event->kind == NESTURBATOR_DMA_APU_AFTER && is_request == 0u))
            continue;
        printf("  cycle=%llu kind=%s phase=%s addr=%04x data=%02x pending=%u target=%u "
               "open=%02x remaining=%u irq=%u irq-clear-pending=%u controller=%02x detail=%u\n",
               (unsigned long long)event->cpu_cycle, dma_event_name(event->kind),
               event->phase != 0u ? "GET" : "PUT", (unsigned)event->address, (unsigned)event->data,
               (unsigned)event->pending, (unsigned)event->halt_phase, (unsigned)event->open_bus,
               (unsigned)event->remaining, (unsigned)event->frame_irq,
               (unsigned)event->frame_irq_clear_pending, (unsigned)event->controller_shift,
               (unsigned)event->detail);
    }
}

static void run_landing_case(uint8_t initial_phase, uint8_t write_count, uint8_t request_load,
                             uint8_t print_trace)
{
    struct nesturbator normal, observed;
    prepare_landing(&normal, initial_phase, request_load);
    prepare_landing(&observed, initial_phase, request_load);
    dma_trace_count = 0u;

    for (uint8_t i = 0u; i < write_count; ++i) {
        uint16_t address = (uint16_t)(0x10u + i);
        uint8_t value = (uint8_t)(0x80u + i);
        nesturbator__bus_write(&normal, address, value);
        test_observed_bus_write(&observed, address, value);
    }
    uint8_t expected = nesturbator__bus_read(&normal, 0x0020u);
    uint8_t actual = test_observed_bus_read(&observed, 0x0020u);

    CHECK_EQ_HEX(actual, 0x5Au);
    CHECK_EQ_HEX(actual, expected);
    CHECK(memcmp(&normal, &observed, sizeof normal) == 0);
    CHECK_EQ_U64(count_dma_events(NESTURBATOR_DMA_CPU_WRITE_COMPLETE), write_count);
    CHECK_EQ_U64(count_dma_events(NESTURBATOR_DMA_SAMPLE_GET), 1u);
    CHECK_EQ_U64(count_dma_events(NESTURBATOR_DMA_RESUMED_READ), 1u);
    CHECK_EQ_U64(count_dma_events(NESTURBATOR_DMA_HALT_ACCEPTED), 1u);
    uint8_t expected_retries = (uint8_t)(write_count - 1u);
    uint8_t first_write_phase = (uint8_t)(initial_phase ^ 1u);
    uint8_t target_phase = request_load != 0u ? 1u : 0u;
    if (first_write_phase == target_phase)
        expected_retries = write_count;
    CHECK_EQ_U64(count_dma_events(NESTURBATOR_DMA_WRITE_RETRY), expected_retries);

    const struct dma_event *get = find_dma_event(NESTURBATOR_DMA_SAMPLE_GET);
    const struct dma_event *halt = find_dma_event(NESTURBATOR_DMA_HALT_ACCEPTED);
    const struct dma_event *resume = find_dma_event(NESTURBATOR_DMA_RESUMED_READ);
    uint8_t expected_halt_phase = (uint8_t)(initial_phase ^ ((write_count + 1u) & 1u));
    uint8_t expected_dma_cycles = expected_halt_phase != 0u ? 3u : 4u;
    CHECK(get != NULL && get->address == 0xC000u && get->data == 0xA7u);
    CHECK(get != NULL && get->phase == 1u);
    CHECK(halt != NULL && halt->phase == expected_halt_phase);
    CHECK(get != NULL && halt != NULL &&
          get->cpu_cycle - halt->cpu_cycle + 1u == expected_dma_cycles);
    CHECK(resume != NULL && resume->address == 0x0020u && resume->data == 0x5Au);
    CHECK_EQ_HEX(observed.bus.ram[0x10u], write_count == 0u ? 0u : 0x80u);
    if (write_count > 1u)
        CHECK_EQ_HEX(observed.bus.ram[(uint8_t)(0x10u + write_count - 1u)],
                     (uint8_t)(0x80u + write_count - 1u));
    CHECK_EQ_U64(observed.apu.dmc.remaining, 0u);
    CHECK_EQ_U64(observed.apu.dmc.dma_pending, 0u);
    if (print_trace != 0u)
        print_dma_trace(initial_phase, write_count, request_load);
}

static void test_dma_landing_on_write(void)
{
    for (uint8_t phase = 0u; phase <= 1u; ++phase) {
        for (uint8_t writes = 1u; writes <= 3u; ++writes) {
            run_landing_case(phase, writes, 0u, 0u);
            run_landing_case(phase, writes, 1u, 0u);
        }
    }
}

static void run_landing_load_case(uint8_t initial_phase, uint8_t print_trace)
{
    struct nesturbator normal, observed;
    prepare_landing(&normal, initial_phase, 0u);
    prepare_landing(&observed, initial_phase, 0u);
    normal.apu.dmc.remaining = 0u;
    observed.apu.dmc.remaining = 0u;
    normal.apu.dmc.dma_pending = 0u;
    observed.apu.dmc.dma_pending = 0u;
    normal.apu.dmc.counter = 100u;
    observed.apu.dmc.counter = 100u;
    dma_trace_count = 0u;

    nesturbator__bus_write(&normal, 0x4015u, 0x10u);
    test_observed_bus_write(&observed, 0x4015u, 0x10u);
    CHECK_EQ_U64(observed.apu.dmc.remaining, 1u);
    CHECK_EQ_U64(observed.apu.dmc.dma_pending, 1u);
    CHECK(observed.apu.dmc.enable_delay == 3u || observed.apu.dmc.enable_delay == 4u);
    for (uint16_t operand = 0u; operand < 3u; ++operand) {
        (void)nesturbator__bus_read(&normal, (uint16_t)(0x8000u + operand));
        (void)test_observed_bus_read(&observed, (uint16_t)(0x8000u + operand));
    }
    nesturbator__bus_write(&normal, 0x5000u, 0xC3u);
    test_observed_bus_write(&observed, 0x5000u, 0xC3u);
    uint8_t expected = nesturbator__bus_read(&normal, 0x0020u);
    uint8_t actual = test_observed_bus_read(&observed, 0x0020u);

    CHECK_EQ_HEX(actual, 0x5Au);
    CHECK_EQ_HEX(actual, expected);
    CHECK(memcmp(&normal, &observed, sizeof normal) == 0);
    CHECK_EQ_HEX(observed.bus.open_bus, 0x5Au);
    CHECK_EQ_U64(count_dma_events(NESTURBATOR_DMA_WRITE_RETRY), initial_phase == 0u ? 1u : 0u);
    CHECK_EQ_U64(count_dma_events(NESTURBATOR_DMA_SAMPLE_GET), 1u);
    const struct dma_event *get = find_dma_event(NESTURBATOR_DMA_SAMPLE_GET);
    const struct dma_event *resume = find_dma_event(NESTURBATOR_DMA_RESUMED_READ);
    uint16_t expected_resume = initial_phase == 0u ? 0x0020u : 0x8002u;
    CHECK(get != NULL && get->phase == 1u && get->address == 0xC000u && get->data == 0xA7u);
    CHECK(resume != NULL && resume->address == expected_resume);
    if (print_trace != 0u)
        print_dma_trace(initial_phase, 1u, 2u);
}

static const struct dma_event *find_last_dma_event(enum dma_event_kind kind)
{
    for (size_t i = dma_trace_count; i != 0u; --i) {
        if (dma_trace[i - 1u].kind == kind)
            return &dma_trace[i - 1u];
    }
    return NULL;
}

static nesturbator *make_test9_powered_instance(void)
{
    const size_t image_size = 16u + 16384u + 8192u;
    uint8_t image[16u + 16384u + 8192u];
    memset(image, 0, sizeof image);
    memcpy(image, "NES\032", 4u);
    image[4] = 1u;
    image[5] = 1u;
    memcpy(image + 16u, landing_prg, 16384u);
    image[16u + 0x3FFCu] = 0x00u;
    image[16u + 0x3FFDu] = 0x80u;

    nesturbator_config config;
    memset(&config, 0, sizeof config);
    config.size = (uint32_t)sizeof config;
    config.abi = NESTURBATOR_ABI_VERSION;
    nesturbator *instance = NULL;
    CHECK_EQ_U64(nesturbator_create(&config, &instance), NESTURBATOR_OK);
    if (instance == NULL)
        return NULL;
    nesturbator_status load_status = nesturbator_load_cartridge(instance, image, image_size);
    CHECK_EQ_U64(load_status, NESTURBATOR_OK);
    if (load_status != NESTURBATOR_OK) {
        nesturbator_destroy(instance);
        return NULL;
    }
    return instance;
}

static void print_test9_trace(void)
{
    size_t last_test_read = dma_trace_count;
    for (size_t i = dma_trace_count; i != 0u; --i) {
        if (dma_trace[i - 1u].kind == NESTURBATOR_DMA_CPU_READ_COMPLETE &&
            dma_trace[i - 1u].address == 0x4000u) {
            last_test_read = i - 1u;
            break;
        }
    }
    printf("landing Test9 production prelude cycles=%llu events=%lu\n",
           (unsigned long long)find_last_dma_event(NESTURBATOR_DMA_CPU_READ_COMPLETE)->cpu_cycle,
           (unsigned long)dma_trace_count);
    for (size_t i = 0u; i < dma_trace_count; ++i) {
        const struct dma_event *event = &dma_trace[i];
        if (event->kind == NESTURBATOR_DMA_APU_AFTER && event->pending != 0u && i != 0u &&
            dma_trace[i - 1u].kind == NESTURBATOR_DMA_APU_BEFORE &&
            dma_trace[i - 1u].pending == 0u) {
            printf("  cycle=%llu request phase=%s halt-target=%s remaining=%u bits=%u "
                   "buffer-empty=%u enable-delay=%u load-waiting=%u counter=%u timer=%u\n",
                   (unsigned long long)event->cpu_cycle, event->phase != 0u ? "GET" : "PUT",
                   event->halt_phase != 0u ? "GET" : "PUT", (unsigned)event->remaining,
                   (unsigned)event->bits, (unsigned)event->buffer_empty,
                   (unsigned)event->enable_delay, (unsigned)event->load_waiting,
                   (unsigned)event->dmc_counter, (unsigned)event->dmc_timer);
        } else if (event->kind == NESTURBATOR_DMA_CPU_WRITE_COMPLETE &&
                   (event->address == 0x4010u || event->address == 0x4011u ||
                    event->address == 0x4012u || event->address == 0x4013u ||
                    event->address == 0x4015u || event->address == 0x5000u)) {
            printf("  cycle=%llu write %04x=%02x phase=%s pending=%u remaining=%u "
                   "enable-delay=%u load-waiting=%u buffer-empty=%u counter=%u timer=%u\n",
                   (unsigned long long)event->cpu_cycle, (unsigned)event->address,
                   (unsigned)event->data, event->phase != 0u ? "GET" : "PUT",
                   (unsigned)event->pending, (unsigned)event->remaining,
                   (unsigned)event->enable_delay, (unsigned)event->load_waiting,
                   (unsigned)event->buffer_empty, (unsigned)event->dmc_counter,
                   (unsigned)event->dmc_timer);
        } else if (event->kind == NESTURBATOR_DMA_WRITE_RETRY ||
                   event->kind == NESTURBATOR_DMA_HALT_ACCEPTED ||
                   event->kind == NESTURBATOR_DMA_SAMPLE_GET ||
                   event->kind == NESTURBATOR_DMA_RESUMED_READ ||
                   (event->kind == NESTURBATOR_DMA_CPU_READ_COMPLETE && event->address == 0x4000u &&
                    i == last_test_read)) {
            printf("  cycle=%llu kind=%s addr=%04x data=%02x phase=%s target=%s pending=%u "
                   "remaining=%u enable-delay=%u load-waiting=%u buffer-empty=%u "
                   "counter=%u timer=%u\n",
                   (unsigned long long)event->cpu_cycle, dma_event_name(event->kind),
                   (unsigned)event->address, (unsigned)event->data,
                   event->phase != 0u ? "GET" : "PUT", event->halt_phase != 0u ? "GET" : "PUT",
                   (unsigned)event->pending, (unsigned)event->remaining,
                   (unsigned)event->enable_delay, (unsigned)event->load_waiting,
                   (unsigned)event->buffer_empty, (unsigned)event->dmc_counter,
                   (unsigned)event->dmc_timer);
        }
    }
}

static void run_landing_test9_reload_boundary(uint8_t print_trace)
{
    uint16_t final_read = 0u;
    (void)test9_emit_program(&final_read);
    landing_prg[0x3FC0u] = 0u; /* The source samples $FFC0 after synchronization. */
    nesturbator *powered = make_test9_powered_instance();
    if (powered == NULL)
        return;
    struct nesturbator normal = *(struct nesturbator *)powered;
    struct nesturbator observed = normal;
    dma_trace_count = 0u;

    size_t steps = 0u;
    uint8_t pretest_done = 0u;
    uint64_t pretest_end_cycle = 0u;
    uint64_t wait_enter_cycle = 0u;
    uint8_t wait_checked = 0u;
    while (normal.cpu.pc != final_read && steps < 1000000u) {
        if (normal.cpu.pc == 0x8800u)
            wait_enter_cycle = normal.cpu_cycle;
        if (normal.cpu.pc == 0x8008u && wait_enter_cycle != 0u) {
            CHECK_EQ_U64(normal.cpu_cycle - wait_enter_cycle, 30012u);
            wait_checked = 1u;
        }
        if (pretest_done == 0u && normal.cpu.pc == 0x8200u && normal.bus.ram[0x12u] == 1u) {
            pretest_done = 1u;
            pretest_end_cycle = normal.cpu_cycle;
            dma_trace_count = 0u;
        }
        nesturbator__cpu_step(&normal);
        test_observed_cpu_step(&observed);
        CHECK_EQ_HEX(observed.cpu.pc, normal.cpu.pc);
        CHECK_EQ_U64(observed.cpu_cycle, normal.cpu_cycle);
        steps++;
        if (normal.cpu.pc == 0u) {
            fprintf(stderr,
                    "Test9 synthetic CPU reached $0000 at step=%lu cycle=%llu S=%02x A=%02x "
                    "DMC pending=%u remaining=%u address=%04x buffer-empty=%u\n",
                    (unsigned long)steps, (unsigned long long)normal.cpu_cycle,
                    (unsigned)normal.cpu.s, (unsigned)normal.cpu.a,
                    (unsigned)normal.apu.dmc.dma_pending, (unsigned)normal.apu.dmc.remaining,
                    (unsigned)normal.apu.dmc.address, (unsigned)normal.apu.dmc.buffer_empty);
            break;
        }
    }
    CHECK(pretest_done != 0u);
    CHECK(wait_checked != 0u);
    CHECK(steps < 1000000u);
    CHECK_EQ_HEX(normal.cpu.pc, final_read);
    nesturbator__cpu_step(&normal);
    test_observed_cpu_step(&observed);

    CHECK_EQ_HEX(observed.cpu.a, 0u);
    CHECK_EQ_HEX(observed.cpu.a, normal.cpu.a);
    CHECK(count_dma_events(NESTURBATOR_DMA_SAMPLE_GET) >= 2u);
    const struct dma_event *halt = find_last_dma_event(NESTURBATOR_DMA_HALT_ACCEPTED);
    const struct dma_event *get = find_last_dma_event(NESTURBATOR_DMA_SAMPLE_GET);
    const struct dma_event *resume = find_last_dma_event(NESTURBATOR_DMA_RESUMED_READ);
    CHECK(halt != NULL && halt->address == 0x4000u);
    CHECK(get != NULL && get->address == 0xFFC0u && get->data == 0u && get->phase == 1u);
    CHECK(resume != NULL && resume->address == 0x4000u && resume->data == 0u);
    CHECK(memcmp(&normal, &observed, sizeof normal) == 0);

    if (print_trace != 0u)
        printf("landing Test9 powered reset; open-bus pretest complete at cycle=%llu; "
               "test 9 start was production-reachable\n",
               (unsigned long long)pretest_end_cycle);
    if (print_trace != 0u)
        print_test9_trace();
    nesturbator_destroy(powered);
}

static int silent_bit_clock_valid(uint8_t before, uint8_t after)
{
    return after == (before == 0u ? 7u : (uint8_t)(before - 1u));
}

/* Independent APU-reference oracle: an empty memory buffer does not stop the
   output unit's eight-clock silent cycles. Use real power-on state and CPU
   instruction fetches; no DMC field or GET/PUT phase is injected. This is an
   default regression; the named frozen-bit control must be rejected. */
static void run_silent_output_probe(uint8_t print_trace)
{
    memset(landing_prg, 0xEA, sizeof landing_prg);
    nesturbator *powered = make_test9_powered_instance();
    if (powered == NULL)
        return;
    struct nesturbator normal = *powered;
    struct nesturbator observed = normal;
    dma_trace_count = 0u;
    for (unsigned step = 0u; step < 2400u; ++step) {
        nesturbator__cpu_step(&normal);
        test_observed_cpu_step(&observed);
    }
    CHECK(memcmp(&normal, &observed, sizeof normal) == 0);
    unsigned clocks = 0u;
    unsigned changing_bit_counts = 0u;
    for (size_t i = 1u; i < dma_trace_count; ++i) {
        const struct dma_event *before = &dma_trace[i - 1u];
        const struct dma_event *after = &dma_trace[i];
        if (before->kind != NESTURBATOR_DMA_APU_BEFORE ||
            after->kind != NESTURBATOR_DMA_APU_AFTER || after->dmc_counter <= before->dmc_counter)
            continue;
        CHECK_EQ_U64(before->buffer_empty, 1u);
        CHECK_EQ_U64(after->remaining, 0u);
        CHECK(silent_bit_clock_valid(before->bits, after->bits));
        if (clocks == 0u)
            CHECK(!silent_bit_clock_valid(before->bits, 0u)); /* frozen-bit control */
        changing_bit_counts += before->bits != after->bits;
        clocks++;
        if (print_trace != 0u)
            printf("silent-output clock=%u cycle=%llu counter=%u->%u bits=%u->%u "
                   "buffer-empty=%u remaining=%u\n",
                   clocks, (unsigned long long)after->cpu_cycle, (unsigned)before->dmc_counter,
                   (unsigned)after->dmc_counter, (unsigned)before->bits, (unsigned)after->bits,
                   (unsigned)after->buffer_empty, (unsigned)after->remaining);
    }
    CHECK(clocks >= 10u); /* Includes an entire eight-clock silent cycle. */
    /* Named wrong model: freeze-at-zero-while-empty. */
    CHECK(changing_bit_counts >= 3u);
    CHECK_EQ_U64(normal.apu.dmc.output, powered->apu.dmc.output);
    nesturbator_destroy(powered);
}

static void run_conflict_probe(uint16_t sample_address, uint8_t sample_data, uint8_t frame_irq,
                               uint8_t controller_shift)
{
    struct nesturbator machine;
    prepare_landing(&machine, 0u, 0u);
    machine.bus.apu_get_put_phase = 0u;
    machine.bus.open_bus = 0u;
    machine.bus.controller_strobe = 0u;
    machine.bus.controller_shift[0] = controller_shift;
    machine.apu.frame_irq = frame_irq;
    machine.apu.dmc.counter = 100u;
    machine.apu.dmc.dma_pending = 1u;
    machine.apu.dmc.dma_halt_phase = 1u;
    machine.apu.dmc.address = sample_address;
    machine.map.cpu_r[47] = landing_prg + 0x3c00u;
    landing_prg[0x3c00u + (sample_address & 0x03ffu)] = sample_data;
    dma_trace_count = 0u;

    uint8_t returned = test_observed_bus_read(&machine, 0x4000u);
    uint16_t internal_select = (uint16_t)(0x4000u | (sample_address & 0x001fu));
    uint8_t expected_return = sample_data;
    if (internal_select == 0x4016u)
        expected_return = (uint8_t)((sample_data & 0xe0u) | (controller_shift & 1u));

    printf("conflict parked=$4000 internal-expected=$%04x external=$%04x/%02x "
           "internal-observed=none returned=%02x expected=%02x frame-irq=%u->%u "
           "controller=%02x->%02x first-divergence=sample-get-internal-select\n",
           (unsigned)internal_select, (unsigned)sample_address, (unsigned)sample_data,
           (unsigned)returned, (unsigned)expected_return, (unsigned)frame_irq,
           (unsigned)machine.apu.frame_irq, (unsigned)controller_shift,
           (unsigned)machine.bus.controller_shift[0]);
    const struct dma_event *get = find_dma_event(NESTURBATOR_DMA_SAMPLE_GET);
    CHECK(get != NULL && get->address == sample_address && get->data == sample_data);
}

static void run_bus_conflict_diagnostics(void)
{
    /* AccuracyCoin's external reads at $FF16 and $FFD5 select $4016 and
       $4015 internally while the CPU is parked at $4000. */
    run_conflict_probe(0xFF16u, 0xA0u, 0u, 1u);
    run_conflict_probe(0xFFD5u, 0xA5u, 1u, 0u);
}

/* $4015 reads the internal CPU bus and does not drive the external bus.
   A following controller read still sees the external sample's high bits.
   Two consecutive status reads also need the internal D5 history, as can
   occur at the dummy/effective reads of indexed read-modify-write. */
static void run_conflict_latch_probe(void)
{
    struct nesturbator machine;
    prepare_landing(&machine, 0u, 1u);
    machine.bus.open_bus = 0x40u;
    machine.bus.controller_shift[0] = 1u;
    machine.apu.frame_irq = 1u;
    machine.apu.dmc.address = 0xFF15u;
    landing_prg[0x3F15u] = 0xA5u;
    dma_trace_count = 0u;
    uint8_t first = test_observed_bus_read(&machine, 0x4015u);
    uint8_t external = machine.bus.open_bus;
    uint8_t second = test_observed_bus_read(&machine, 0x4015u);
    uint8_t controller = test_observed_bus_read(&machine, 0x4016u);
    const struct dma_event *get = find_dma_event(NESTURBATOR_DMA_SAMPLE_GET);
    CHECK(get != NULL && get->address == 0xFF15u && get->data == 0xA5u && get->phase == 1u);
    CHECK_EQ_HEX(first, 0u);
    CHECK_EQ_HEX(second, 0u);
    CHECK_EQ_HEX(external, 0xA5u);
    CHECK_EQ_HEX(controller, 0xA1u);
    printf("conflict-latches parked=4015 sample=ff15/a5 internal-D5=0 "
           "status=%02x/%02x external=%02x controller=%02x expected=00/00,a5,a1\n",
           (unsigned)first, (unsigned)second, (unsigned)external, (unsigned)controller);
}

static void run_oam_probe(uint16_t requested_offset, uint8_t load, uint8_t phase, uint8_t verify)
{
    struct nesturbator normal, observed;
    prepare_landing(&normal, phase, 0u);
    prepare_landing(&observed, phase, 0u);
    for (uint16_t i = 0u; i < 256u; ++i) {
        normal.bus.ram[i] = (uint8_t)i;
        observed.bus.ram[i] = (uint8_t)i;
    }
    normal.apu.dmc.remaining = 0u;
    normal.apu.dmc.dma_pending = 0u;
    observed.apu.dmc.remaining = 1u;
    observed.apu.dmc.dma_pending = 0u;
    observed.apu.dmc.dma_load_waiting = load;
    uint16_t first_edge = (uint16_t)(requested_offset + 2u);
    if (first_edge > 214u) {
        /* Delay until the second output edge using valid timer/counter
           bounds and a full buffer, rather than injecting counter > 214. */
        observed.apu.dmc.timer = 214u;
        observed.apu.dmc.counter = (uint16_t)(first_edge - 214u);
        observed.apu.dmc.bits = 1u;
        observed.apu.dmc.buffer_empty = 0u;
    } else {
        observed.apu.dmc.counter = first_edge;
    }
    struct nesturbator linked = observed;
    dma_trace_count = 0u;

    nesturbator__bus_write(&normal, 0x4014u, 0u);
    nesturbator__bus_write(&linked, 0x4014u, 0u);
    test_observed_bus_write(&observed, 0x4014u, 0u);
    uint64_t normal_start = normal.cpu_cycle;
    uint64_t observed_start = observed.cpu_cycle;
    (void)nesturbator__bus_read(&normal, 0x8000u);
    uint8_t expected_return = nesturbator__bus_read(&linked, 0x8000u);
    uint8_t actual_return = test_observed_bus_read(&observed, 0x8000u);
    CHECK_EQ_HEX(actual_return, expected_return);
    CHECK(memcmp(&linked, &observed, sizeof linked) == 0);
    uint64_t normal_cycles = normal.cpu_cycle - normal_start;
    uint64_t observed_cycles = observed.cpu_cycle - observed_start;
    const struct dma_event *get = find_dma_event(NESTURBATOR_DMA_SAMPLE_GET);
    const struct dma_event *halt = find_dma_event(NESTURBATOR_DMA_HALT_ACCEPTED);
    uint16_t collision_offset = UINT16_MAX;
    uint16_t resume_offset = UINT16_MAX;
    uint8_t resume_phase = 0u, destination_phase = 0u;
    size_t destinations = 0u;
    if (get != NULL) {
        for (size_t i = 0u; i < dma_trace_count; ++i) {
            if (dma_trace[i].kind == NESTURBATOR_DMA_SAMPLE_GET)
                break;
            if (dma_trace[i].kind == NESTURBATOR_DMA_OAM_SOURCE)
                collision_offset = dma_trace[i].detail;
        }
        for (size_t i = 0u; i < dma_trace_count; ++i) {
            if (dma_trace[i].kind == NESTURBATOR_DMA_OAM_SOURCE_COMPLETE &&
                dma_trace[i].cpu_cycle > get->cpu_cycle) {
                resume_offset = dma_trace[i].detail;
                resume_phase = dma_trace[i].phase;
                break;
            }
        }
    }
    for (size_t i = 0u; i < dma_trace_count; ++i) {
        const struct dma_event *event = &dma_trace[i];
        if (verify != 0u && event->kind == NESTURBATOR_DMA_OAM_SOURCE_COMPLETE)
            CHECK_EQ_U64(event->phase, 1u);
        if (event->kind == NESTURBATOR_DMA_OAM_DEST) {
            if (verify != 0u)
                CHECK_EQ_U64(event->phase, 0u);
            CHECK_EQ_U64(event->detail, destinations);
            CHECK_EQ_U64(event->data, (uint8_t)destinations);
            if (event->detail == collision_offset)
                destination_phase = event->phase;
            destinations++;
        }
    }
    CHECK_EQ_U64(destinations, 256u);
    if (verify != 0u) {
        CHECK(get != NULL && get->phase == 1u && get->address == 0xC000u && get->data == 0xA7u &&
              halt != NULL);
        CHECK_EQ_U64(observed.apu.dmc.dma_pending, 0u);
        CHECK_EQ_U64(count_dma_events(NESTURBATOR_DMA_SAMPLE_GET), 1u);
        CHECK_EQ_U64(count_dma_events(NESTURBATOR_DMA_CPU_READ_COMPLETE), 1u);
        const struct dma_event *cpu_resume = find_last_dma_event(NESTURBATOR_DMA_CPU_READ_COMPLETE);
        CHECK(cpu_resume != NULL && cpu_resume->address == 0x8000u && cpu_resume->data == 0xA7u);
        if (get != NULL && halt != NULL) {
            /* Fixed stimulus/oracle table, independent of the observed halt:
               interior +2; these two tail timer placements hit the documented
               final-source / final-PUT windows. DMA rev 23450 overlap examples. */
            uint64_t expected_extra = requested_offset == 128u ? 2u
                                      : phase == 0u ? (requested_offset == 254u ? 1u : 3u)
                                                    : (requested_offset == 254u ? 2u : 1u);
            uint16_t expected_collision =
                requested_offset == 255u ? 255u : (uint16_t)(requested_offset + (phase == 0u));
            CHECK_EQ_U64(collision_offset, expected_collision);
            CHECK_EQ_U64(observed_cycles - normal_cycles, expected_extra);
            if (expected_collision < 255u) {
                CHECK_EQ_U64(resume_offset, expected_collision + 1u);
                CHECK_EQ_U64(resume_phase, 1u);
            } else {
                CHECK_EQ_U64(resume_offset, UINT16_MAX);
            }
            uint64_t dma_get = halt->cpu_cycle + (halt->phase != 0u ? 2u : 3u);
            uint64_t oam_end = normal_start + normal_cycles - 1u;
            uint64_t extra = dma_get <= oam_end ? 2u : dma_get - oam_end;
            CHECK_EQ_U64(get->cpu_cycle, dma_get);
            CHECK_EQ_U64(extra, expected_extra);
            if (expected_extra == 1u || expected_extra == 3u)
                CHECK_EQ_U64(get->cpu_cycle, oam_end + expected_extra);
            else
                CHECK(get->cpu_cycle < oam_end);
            CHECK(cpu_resume != NULL && cpu_resume->phase == (uint8_t)(1u ^ (extra & 1u)));
        }
    }
    printf("oam requested-offset=%u load=%u initial-phase=%u collision-offset=%s%u get-phase=%s "
           "source-resume=%s%u/%s destination-phase=%s destinations=%lu "
           "base-cycles=%llu observed-cycles=%llu extra=%lld pending-after-oam=%u\n",
           (unsigned)requested_offset, (unsigned)load, (unsigned)phase,
           collision_offset == UINT16_MAX ? "none/" : "", (unsigned)collision_offset,
           get == NULL ? "none" : (get->phase != 0u ? "GET" : "PUT"),
           resume_offset == UINT16_MAX ? "none/" : "", (unsigned)resume_offset,
           resume_phase != 0u ? "GET" : "PUT", destination_phase != 0u ? "GET" : "PUT",
           (unsigned long)destinations, (unsigned long long)normal_cycles,
           (unsigned long long)observed_cycles,
           (long long)observed_cycles - (long long)normal_cycles,
           (unsigned)observed.apu.dmc.dma_pending);
}

static void test_dma_oam_collision(void)
{
    for (uint8_t phase = 0u; phase <= 1u; ++phase)
        for (uint8_t load = 0u; load <= 1u; ++load)
            run_oam_probe(128u, load, phase, 1u);
}

static void test_dma_oam_final_write(void)
{
    for (uint8_t phase = 0u; phase <= 1u; ++phase)
        for (uint8_t load = 0u; load <= 1u; ++load)
            for (uint16_t offset = 254u; offset <= 255u; ++offset)
                run_oam_probe(offset, load, phase, 1u);
}

/* Drive the documented stop windows from genuine output/buffer history.
   No pending request, bit count or GET/PUT phase is injected. */
static nesturbator *make_abort_instance(uint8_t control, uint8_t length, uint8_t start)
{
    memset(landing_prg, 0xEA, sizeof landing_prg);
    nesturbator *machine = make_test9_powered_instance();
    if (machine == NULL)
        return NULL;
    test_observed_bus_write(machine, 0x4010u, control);
    test_observed_bus_write(machine, 0x4012u, 0u);
    test_observed_bus_write(machine, 0x4013u, length);
    if (start != 0u)
        test_observed_bus_write(machine, 0x4015u, 0x10u);
    return machine;
}

static void run_explicit_abort_probe(uint8_t intervening_write, uint16_t lead_counter,
                                     uint8_t half_phase)
{
    nesturbator *machine = make_abort_instance(0x4Fu, 0u, 1u);
    if (machine == NULL)
        return;
    unsigned reads = 0u;
    /* A stop on the intervening non-timer half-cycle is two CPU clocks
       before the documented PUT halt. DMA wiki rev 23450, Bugs. */
    while (!(machine->apu.dmc.bits == 0u && machine->apu.dmc.buffer_empty == 0u &&
             machine->apu.dmc.counter == lead_counter &&
             machine->apu.pulse_clock_phase == half_phase && machine->apu.dmc.dma_pending == 0u) &&
           reads++ < 10000u)
        (void)test_observed_bus_read(machine, 0x0020u);
    CHECK(reads < 10000u);
    dma_trace_count = 0u;
    uint64_t start = machine->cpu_cycle;
    test_observed_bus_write(machine, 0x4015u, 0u);
    uint8_t stop_phase = machine->bus.apu_get_put_phase;
    uint8_t write_position = stop_phase != 0u ? 3u : 2u;
    uint8_t returned = 0u;
    for (uint8_t access = 1u; access <= 9u; ++access) {
        if (intervening_write != 0u && access == write_position)
            test_observed_bus_write(machine, 0x0021u, 0x91u);
        else
            returned = test_observed_bus_read(machine, 0x0020u);
    }
    uint64_t cycles = machine->cpu_cycle - start;
    CHECK_EQ_U64(count_dma_events(NESTURBATOR_DMA_SAMPLE_GET), 0u);
    CHECK_EQ_U64(machine->apu.dmc.remaining, 0u);
    if (intervening_write != 0u)
        CHECK_EQ_HEX(machine->bus.ram[0x21u], 0x91u);
    int expected_abort =
        (lead_counter == 1u && half_phase == 0u) || (lead_counter == 2u && half_phase == 1u)
            ? (intervening_write != 0u ? 0 : 1)
            : -1;
    printf("explicit-abort stop-cycle=%llu counter=%u half=%u write-on-halt=%u ordinary-cycles=10 "
           "observed-cycles=%llu expected-abort-extra=%d sample-gets=%lu "
           "pending=%u bits=%u buffer-empty=%u read=%02x\n",
           (unsigned long long)(start + 1u), (unsigned)lead_counter, (unsigned)half_phase,
           (unsigned)intervening_write, (unsigned long long)cycles, expected_abort,
           (unsigned long)count_dma_events(NESTURBATOR_DMA_SAMPLE_GET),
           (unsigned)machine->apu.dmc.dma_pending, (unsigned)machine->apu.dmc.bits,
           (unsigned)machine->apu.dmc.buffer_empty, (unsigned)returned);
    print_dma_trace(0u, intervening_write, 0u);
    nesturbator_destroy(machine);
}

static void run_implicit_abort_probe(uint8_t loop, uint8_t length, uint8_t intervening_write,
                                     uint16_t lead_counter, uint8_t half_phase)
{
    nesturbator *machine =
        make_abort_instance((uint8_t)(0x0Eu | (loop != 0u ? 0x40u : 0u)), length, 0u);
    if (machine == NULL)
        return;
    unsigned reads = 0u;
    /* Enabling on the next CPU write, nine clocks before an output-cycle
       boundary, places the first sample GET three clocks before that boundary.
       Its implicit stop is in the revision-independent one-cycle-abort window. */
    while (!(machine->apu.dmc.bits == 0u && machine->apu.dmc.counter == lead_counter &&
             machine->apu.pulse_clock_phase == half_phase) &&
           reads++ < 10000u)
        (void)test_observed_bus_read(machine, 0x0020u);
    CHECK(reads < 10000u);
    CHECK_EQ_U64(machine->apu.dmc.buffer_empty, 1u);
    dma_trace_count = 0u;
    uint64_t start = machine->cpu_cycle;
    test_observed_bus_write(machine, 0x4015u, 0x10u);
    while (machine->cpu_cycle - start < 20u) {
        if (intervening_write != 0u && machine->cpu_cycle - start == 9u)
            test_observed_bus_write(machine, 0x0021u, 0x91u);
        else
            (void)test_observed_bus_read(machine, 0x0020u);
    }
    const struct dma_event *first_get = find_dma_event(NESTURBATOR_DMA_SAMPLE_GET);
    CHECK(first_get != NULL && first_get->phase == 1u && first_get->address == 0xC000u);
    printf("implicit-abort enable-cycle=%llu counter=%u half=%u loop=%u length-reg=%u "
           "write-on-halt=%u "
           "first-sample-offset=%llu sample-gets=%lu accepted=%lu pending=%u remaining=%u "
           "bits=%u buffer-empty=%u\n",
           (unsigned long long)(start + 1u), (unsigned)lead_counter, (unsigned)half_phase,
           (unsigned)loop, (unsigned)length, (unsigned)intervening_write,
           (unsigned long long)(first_get != NULL ? first_get->cpu_cycle - start : 0u),
           (unsigned long)count_dma_events(NESTURBATOR_DMA_SAMPLE_GET),
           (unsigned long)count_dma_events(NESTURBATOR_DMA_HALT_ACCEPTED),
           (unsigned)machine->apu.dmc.dma_pending, (unsigned)machine->apu.dmc.remaining,
           (unsigned)machine->apu.dmc.bits, (unsigned)machine->apu.dmc.buffer_empty);
    print_dma_trace(0u, intervening_write, 1u);
    nesturbator_destroy(machine);
}

static void reset(void)
{
    memset(&nes, 0, sizeof nes);
}

/* 24 ticks per read and per write, counted from zero. */
static void test_ticks(void)
{
    reset();
    nesturbator__bus_read(&nes, 0x0000u);
    CHECK_EQ_U64(nes.ticks, 24u);
    for (int i = 1; i < 10; i++) {
        nesturbator__bus_read(&nes, 0x0000u);
    }
    CHECK_EQ_U64(nes.ticks, 240u);
    nesturbator__bus_write(&nes, 0x0000u, 1u);
    CHECK_EQ_U64(nes.ticks, 264u);
}

/* A byte written at 0x0001 reads back at each of its three mirrors. */
static void test_mirror(void)
{
    reset();
    nesturbator__bus_write(&nes, 0x0001u, 0x5Au);
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x0801u), 0x5Au);
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x1001u), 0x5Au);
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x1801u), 0x5Au);
    /* A write through a mirror lands in the same byte. */
    nesturbator__bus_write(&nes, 0x1FFFu, 0x3Cu);
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x07FFu), 0x3Cu);
}

/* An unmapped read returns the last value written or read. */
static void test_open_bus(void)
{
    reset();
    nesturbator__bus_write(&nes, 0x0002u, 0x77u);
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x4020u), 0x77u);
    nesturbator__bus_write(&nes, 0x0001u, 0x5Au);
    nesturbator__bus_write(&nes, 0x0003u, 0x11u);
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x0001u), 0x5Au);
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x8000u), 0x5Au);
}

/* A write outside RAM changes no RAM byte. */
static void test_unmapped_write(void)
{
    reset();
    for (uint32_t i = 0u; i < sizeof nes.bus.ram; i++) {
        nes.bus.ram[i] = (uint8_t)i;
    }
    uint8_t before[sizeof nes.bus.ram];
    memcpy(before, nes.bus.ram, sizeof before);
    nesturbator__bus_write(&nes, 0x6000u, 0xA5u);
    CHECK(memcmp(before, nes.bus.ram, sizeof before) == 0);
    CHECK_EQ_HEX(nes.bus.open_bus, 0xA5u);
}

int main(int argc, char **argv)
{
    if (argc == 3 && strcmp(argv[1], "--dma-trace") == 0 &&
        strcmp(argv[2], "conflict-latches") == 0) {
        run_conflict_latch_probe();
        CHECK_DONE();
    }
    if (argc == 3 && strcmp(argv[1], "--dma-trace") == 0 && strcmp(argv[2], "silence") == 0) {
        run_silent_output_probe(1u);
        CHECK_DONE();
    }
    if (argc == 3 && strcmp(argv[1], "--dma-trace") == 0 && strcmp(argv[2], "landing") == 0) {
        run_landing_test9_reload_boundary(1u);
        CHECK_DONE();
    }
    if (argc == 3 && strcmp(argv[1], "--dma-trace") == 0 && strcmp(argv[2], "remaining") == 0) {
        for (uint8_t request_load = 0u; request_load <= 1u; ++request_load) {
            for (uint8_t phase = 0u; phase <= 1u; ++phase) {
                for (uint8_t writes = 1u; writes <= 3u; ++writes)
                    run_landing_case(phase, writes, request_load, 1u);
            }
        }
        run_landing_load_case(0u, 1u);
        run_landing_load_case(1u, 1u);
        run_landing_test9_reload_boundary(1u);
        run_bus_conflict_diagnostics();
        for (uint8_t phase = 0u; phase <= 1u; ++phase) {
            for (uint8_t load = 0u; load <= 1u; ++load) {
                run_oam_probe(128u, load, phase, 0u);
                run_oam_probe(254u, load, phase, 0u);
                run_oam_probe(255u, load, phase, 0u);
            }
        }
        for (uint16_t counter = 1u; counter <= 3u; ++counter) {
            for (uint8_t half = 0u; half <= 1u; ++half) {
                run_explicit_abort_probe(0u, counter, half);
                run_explicit_abort_probe(1u, counter, half);
            }
        }
        for (uint16_t counter = 4u; counter <= 6u; ++counter) {
            for (uint8_t half = 0u; half <= 1u; ++half) {
                run_implicit_abort_probe(0u, 0u, 0u, counter, half);
                run_implicit_abort_probe(0u, 0u, 1u, counter, half);
                run_implicit_abort_probe(1u, 0u, 0u, counter, half);
                run_implicit_abort_probe(0u, 1u, 0u, counter, half);
            }
        }
        CHECK_DONE();
    }
    if (argc != 1)
        return 2;
    test_ticks();
    test_mirror();
    test_open_bus();
    test_unmapped_write();
    test_dma_landing_on_write();
    run_landing_load_case(0u, 0u);
    run_landing_load_case(1u, 0u);
    run_silent_output_probe(0u);
    run_landing_test9_reload_boundary(0u);
    test_dma_oam_collision();
    test_dma_oam_final_write();
    CHECK_DONE();
}
