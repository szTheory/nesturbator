/* The per-board cartridge interface (MAP-01). A board is a small module that
   fills the ops below and owns its bank registers in nesturbator__mapper. The
   bus and the PPU never name a board: they look up the page tables and the
   four-entry nametable map in nesturbator__map, and call a hook only when its
   bit in map.watch is set. Source: NESdev Wiki "Cartridge connector",
   "Bus conflict" and "IRQ".

   Hook rules: a hook receives time as an argument and never reads nes->ticks.
   It never advances time, touches the bus or allocates. It updates only
   nes->mapper and may call nesturbator__irq_update. A member is called only
   when its watch bit is set, never chosen by a NULL test, so a board leaves
   the members it does not watch NULL. */
#ifndef NESTURBATOR_MAPPER_H
#define NESTURBATOR_MAPPER_H

#include <stdint.h>

struct nesturbator;

/* Events a board asks the core to report. 8u (PPU_READ, for MMC2 and MMC5) and
   16u (CPU_CLOCK, for FME-7 and Namco 163) are reserved numbers with no code
   until a board needs them. */
#define NESTURBATOR_WATCH_CPU_WRITE 1u
#define NESTURBATOR_WATCH_BUS_CONFLICT 2u
#define NESTURBATOR_WATCH_PPU_A12 4u

struct nesturbator__mapper_ops {
    /* Sets map.watch. Registers are zero from the loader's memset and kept by a
       state load, so init does not touch them. */
    void (*init)(struct nesturbator *nes);
    /* Page tables and nt[] from the bank registers. */
    void (*rebuild)(struct nesturbator *nes);
    /* A CPU write to $4020-$FFFF; cpu_cycle is the index of the write's cycle. */
    void (*cpu_write)(struct nesturbator *nes, uint16_t addr, uint8_t value, uint64_t cpu_cycle);
    /* The PPU address bus bit 12 changed to level at PPU tick tick. */
    void (*ppu_a12)(struct nesturbator *nes, uint8_t level, uint64_t tick);
    /* Reserved for MMC2 and MMC5; never called. */
    uint8_t (*ppu_read)(struct nesturbator *nes, uint16_t addr);
};

/* Serialisable board state: no pointers. */
struct nesturbator__mapper {
    uint16_t id;
    uint8_t submapper;
    uint8_t irq; /* the board's IRQ line, ORed in by nesturbator__irq_update */
    union {
        struct {
            uint8_t unused; /* NROM has no registers; C17 forbids an empty union */
        } nrom;
        struct {
            uint8_t bank; /* 16 KiB bank at $8000: the raw written byte */
        } uxrom;
    } reg;
};

/* Derived state, rebuilt by nesturbator__mapper_load and never serialised
   (ARCHITECTURE section 6). Pages are 1 KiB. cpu_r and cpu_w cover
   $4000-$FFFF at index (addr - 0x4000) >> 10; chr_r and chr_w cover the
   pattern tables; nt[] gives the CIRAM bank (0 or 1) of each nametable. A
   NULL page reads as open bus (CPU) or 0 (PPU), and a write to a NULL page
   is dropped. A zeroed instance therefore has an empty cartridge. */
struct nesturbator__map {
    struct nesturbator__mapper_ops ops;
    const uint8_t *cpu_r[48];
    uint8_t *cpu_w[48];
    const uint8_t *chr_r[8];
    uint8_t *chr_w[8];
    uint8_t nt[4];
    uint8_t watch;
    uint8_t a12; /* last A12 level seen on the PPU address bus */
};

/* The only switch on the board id: fills out and returns 1 when a board
   exists, else 0 (src/cartridge.c). */
int nesturbator__mapper_ops_for(uint16_t id, struct nesturbator__mapper_ops *out);

/* Chooses the board from nes->mapper.id, then runs init and rebuild
   (src/cartridge.c). Returns 1, or 0 when no board matches the id; registers
   are kept, so a state load can call it. */
int nesturbator__mapper_load(struct nesturbator *nes);

/* Fills out with the NROM board (src/mapper_nrom.c). */
void nesturbator__mapper_nrom_ops(struct nesturbator__mapper_ops *out);

/* Fills out with the UxROM board (src/mapper_uxrom.c). */
void nesturbator__mapper_uxrom_ops(struct nesturbator__mapper_ops *out);

#endif /* NESTURBATOR_MAPPER_H */
