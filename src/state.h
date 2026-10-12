/* The state visitor: one field list per struct drives four passes over the same bytes (STATE-01).
   SIZE counts the bytes, WRITE stores the fields, VALIDATE reads the bytes and range-checks them
   without touching the instance, APPLY reads the bytes into the fields. VALIDATE and APPLY walk the
   same lists over the same buffer, so once VALIDATE passes, APPLY cannot fail.

   Every multi-byte value is written one byte at a time, least significant first, so the bytes are
   the same on every platform (D-16). The primitives are static inline so the board files link
   without state.c. Each returns the value seen in the stream: the field for SIZE and WRITE, the
   decoded value for VALIDATE and APPLY. A signed primitive returns 0 for a value that fails its
   range in VALIDATE, so a caller may add the results without overflow. */
#ifndef NESTURBATOR_STATE_H
#define NESTURBATOR_STATE_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
    NESTURBATOR__SIO_SIZE,
    NESTURBATOR__SIO_WRITE,
    NESTURBATOR__SIO_VALIDATE,
    NESTURBATOR__SIO_APPLY
};

struct nesturbator__sio {
    int mode;
    uint8_t *w;       /* WRITE cursor */
    const uint8_t *r; /* VALIDATE and APPLY cursor */
    size_t n;         /* SIZE: bytes counted so far */
    int bad;          /* VALIDATE: a range or cross-field rule failed */
    uint64_t ticks;   /* VALIDATE: the header's ticks, for the cross-field rule on ppu_ticks */
};

/* A cross-field rule: in VALIDATE, a false ok marks the state invalid. */
static inline void nesturbator__sio_check(struct nesturbator__sio *io, int ok)
{
    if (io->mode == NESTURBATOR__SIO_VALIDATE && !ok)
        io->bad = 1;
}

/* Moves width bytes little-endian: counts, writes v, or returns the decoded value. */
static inline uint64_t nesturbator__sio_raw(struct nesturbator__sio *io, uint64_t v, unsigned width)
{
    switch (io->mode) {
    case NESTURBATOR__SIO_SIZE:
        io->n += width;
        return v;
    case NESTURBATOR__SIO_WRITE:
        for (unsigned i = 0u; i < width; ++i)
            io->w[i] = (uint8_t)(v >> (8u * i));
        io->w += width;
        return v;
    default:
        v = 0u;
        for (unsigned i = 0u; i < width; ++i)
            v |= (uint64_t)io->r[i] << (8u * i);
        io->r += width;
        return v;
    }
}

static inline uint8_t nesturbator__sio_u8(struct nesturbator__sio *io, uint8_t *v, uint8_t max)
{
    uint8_t x = (uint8_t)nesturbator__sio_raw(io, *v, 1u);
    nesturbator__sio_check(io, x <= max);
    if (io->mode == NESTURBATOR__SIO_APPLY)
        *v = x;
    return x;
}

static inline uint16_t nesturbator__sio_u16(struct nesturbator__sio *io, uint16_t *v, uint16_t max)
{
    uint16_t x = (uint16_t)nesturbator__sio_raw(io, *v, 2u);
    nesturbator__sio_check(io, x <= max);
    if (io->mode == NESTURBATOR__SIO_APPLY)
        *v = x;
    return x;
}

static inline uint32_t nesturbator__sio_u32(struct nesturbator__sio *io, uint32_t *v, uint32_t max)
{
    uint32_t x = (uint32_t)nesturbator__sio_raw(io, *v, 4u);
    nesturbator__sio_check(io, x <= max);
    if (io->mode == NESTURBATOR__SIO_APPLY)
        *v = x;
    return x;
}

/* Every 64-bit pattern is legal. */
static inline uint64_t nesturbator__sio_u64(struct nesturbator__sio *io, uint64_t *v)
{
    uint64_t x = nesturbator__sio_raw(io, *v, 8u);
    if (io->mode == NESTURBATOR__SIO_APPLY)
        *v = x;
    return x;
}

/* A signed value travels as its two's-complement bit pattern. The unsigned pattern is converted
   back without an implementation-defined cast: a pattern above the signed maximum is rebuilt from
   its distance to the unsigned maximum. */
static inline int32_t nesturbator__sio_i32(struct nesturbator__sio *io, int32_t *v, int32_t lo,
                                           int32_t hi)
{
    uint32_t u = (uint32_t)nesturbator__sio_raw(io, (uint32_t)*v, 4u);
    int32_t x = u <= (uint32_t)INT32_MAX ? (int32_t)u : -(int32_t)(UINT32_MAX - u) - 1;
    if (io->mode == NESTURBATOR__SIO_VALIDATE && (x < lo || x > hi)) {
        io->bad = 1;
        return 0;
    }
    if (io->mode == NESTURBATOR__SIO_APPLY)
        *v = x;
    return x;
}

/* The range is -bound to bound; bound is below the 64-bit signed maximum. */
static inline int64_t nesturbator__sio_i64(struct nesturbator__sio *io, int64_t *v, int64_t bound)
{
    uint64_t u = nesturbator__sio_raw(io, (uint64_t)*v, 8u);
    int64_t x = u <= (uint64_t)INT64_MAX ? (int64_t)u : -(int64_t)(UINT64_MAX - u) - 1;
    if (io->mode == NESTURBATOR__SIO_VALIDATE && (x < -bound || x > bound)) {
        io->bad = 1;
        return 0;
    }
    if (io->mode == NESTURBATOR__SIO_APPLY)
        *v = x;
    return x;
}

/* n bytes of any value. */
static inline void nesturbator__sio_bytes(struct nesturbator__sio *io, uint8_t *p, size_t n)
{
    if (n == 0u)
        return;
    switch (io->mode) {
    case NESTURBATOR__SIO_SIZE:
        io->n += n;
        break;
    case NESTURBATOR__SIO_WRITE:
        memcpy(io->w, p, n);
        io->w += n;
        break;
    case NESTURBATOR__SIO_VALIDATE:
        io->r += n;
        break;
    default:
        memcpy(p, io->r, n);
        io->r += n;
        break;
    }
}

#endif /* NESTURBATOR_STATE_H */
