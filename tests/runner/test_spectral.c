/* SND-04: deterministic fixed-point spectral checks for the production
   transition-to-PCM synthesizer. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/internal.h"
#include "../check.h"

#define SPECTRAL_SAMPLES 32768u
#define WARMUP_SAMPLES 4096u
#define TOTAL_SAMPLES (WARMUP_SAMPLES + SPECTRAL_SAMPLES)
#define SAMPLE_PERIOD_UNITS NESTURBATOR_AUDIO_TICKS_PER_PERIOD
#define SPECTRAL_HARMONICS 512u
#define CPU_SAMPLE_STEP (24u * NESTURBATOR_AUDIO_SAMPLES_PER_PERIOD)
#define Q30 INT64_C(1073741824)
#define MAX_ANALYSIS_BIN (SPECTRAL_SAMPLES / 3u)
#define PUBLIC_FRAME_SAMPLES_MAX 960u
#define PUBLIC_FRAME_GUARD INT16_C(0x3456)
#define PAL_EXPECTED_RATE_NUM 640u
#define PAL_EXPECTED_RATE_DEN 709379u
#define PAL_FRAME_TICKS UINT64_C(1063920)
#define PAL_FRAME_OVERSHOOT_CPU_BOUND (514u + 8u + 11u * 4u + 34u)
/* Ring-only conservative budget: one 514-cycle OAM transfer, one 8-cycle
   instruction, up to eleven 4-cycle DMC opportunities at the fastest 50-cycle
   PAL rate, and 34 cycles of margin. This does not qualify PAL DMA behavior. */
#define PAL_RING_OVERSHOOT_BOUND                                                                   \
    (((PAL_FRAME_OVERSHOOT_CPU_BOUND * UINT64_C(20480) + PAL_EXPECTED_RATE_DEN - 1u) /             \
      PAL_EXPECTED_RATE_DEN) +                                                                     \
     1u)

static int32_t trig_cos[SPECTRAL_SAMPLES];
static int32_t trig_sin[SPECTRAL_SAMPLES];
static int32_t fft_real[SPECTRAL_SAMPLES];
static int32_t fft_imag[SPECTRAL_SAMPLES];
static int16_t tone_samples[SPECTRAL_SAMPLES];
static int16_t public_capture[TOTAL_SAMPLES];
static uint16_t public_video[256u * 240u];

struct transition_trace {
    struct nesturbator *machine;
    uint64_t last_cycle;
    uint32_t count;
    uint8_t bad;
};

static const int64_t atan_turn_q32[] = {
    INT64_C(536870912), INT64_C(316933406), INT64_C(167458907), INT64_C(85004756),
    INT64_C(42667331),  INT64_C(21354465),  INT64_C(10679838),  INT64_C(5340245),
    INT64_C(2670163),   INT64_C(1335087),   INT64_C(667544),    INT64_C(333772),
    INT64_C(166886),    INT64_C(83443),     INT64_C(41722),     INT64_C(20861),
    INT64_C(10430),     INT64_C(5215),      INT64_C(2608),      INT64_C(1304),
    INT64_C(652),       INT64_C(326),       INT64_C(163),       INT64_C(81),
    INT64_C(41),        INT64_C(20),        INT64_C(10),        INT64_C(5),
    INT64_C(3),         INT64_C(1)};

static nesturbator *make_instance(void)
{
    nesturbator_config config;
    memset(&config, 0, sizeof config);
    config.size = (uint32_t)sizeof config;
    config.abi = NESTURBATOR_ABI_VERSION;
    nesturbator *instance = NULL;
    CHECK_EQ_U64(nesturbator_create(&config, &instance), NESTURBATOR_OK);
    return instance;
}

static void cordic(uint32_t phase, int32_t *cosine, int32_t *sine)
{
    uint32_t quadrant = phase >> 30u;
    int64_t z = (int64_t)(phase & UINT32_C(0x3fffffff));
    int64_t x = INT64_C(652032874);
    int64_t y = 0;
    for (unsigned i = 0u; i < sizeof atan_turn_q32 / sizeof atan_turn_q32[0]; i++) {
        int64_t divisor = INT64_C(1) << i;
        int64_t x_shift = x / divisor;
        int64_t y_shift = y / divisor;
        if (z >= 0) {
            x -= y_shift;
            y += x_shift;
            z -= atan_turn_q32[i];
        } else {
            x += y_shift;
            y -= x_shift;
            z += atan_turn_q32[i];
        }
    }

    switch (quadrant) {
    case 0u:
        *cosine = (int32_t)x;
        *sine = (int32_t)y;
        break;
    case 1u:
        *cosine = (int32_t)-y;
        *sine = (int32_t)x;
        break;
    case 2u:
        *cosine = (int32_t)-x;
        *sine = (int32_t)-y;
        break;
    default:
        *cosine = (int32_t)y;
        *sine = (int32_t)-x;
        break;
    }
}

static void init_trig_table(void)
{
    for (unsigned i = 0u; i < SPECTRAL_SAMPLES; i++) {
        uint32_t phase = i << 17u;
        cordic(phase, &trig_cos[i], &trig_sin[i]);
    }
}

static void fft(void)
{
    unsigned reversed = 0u;
    for (unsigned i = 1u; i < SPECTRAL_SAMPLES; i++) {
        unsigned bit = SPECTRAL_SAMPLES >> 1u;
        while ((reversed & bit) != 0u) {
            reversed ^= bit;
            bit >>= 1u;
        }
        reversed ^= bit;
        if (i < reversed) {
            int32_t temporary = fft_real[i];
            fft_real[i] = fft_real[reversed];
            fft_real[reversed] = temporary;
            temporary = fft_imag[i];
            fft_imag[i] = fft_imag[reversed];
            fft_imag[reversed] = temporary;
        }
    }

    for (unsigned length = 2u; length <= SPECTRAL_SAMPLES; length <<= 1u) {
        unsigned half = length / 2u;
        unsigned twiddle_stride = SPECTRAL_SAMPLES / length;
        for (unsigned base = 0u; base < SPECTRAL_SAMPLES; base += length) {
            for (unsigned j = 0u; j < half; j++) {
                unsigned odd = base + j + half;
                unsigned even = base + j;
                unsigned twiddle = j * twiddle_stride;
                int64_t wr = trig_cos[twiddle];
                int64_t wi = -(int64_t)trig_sin[twiddle];
                int64_t product_real = wr * fft_real[odd] - wi * fft_imag[odd];
                int64_t product_imag = wr * fft_imag[odd] + wi * fft_real[odd];
                int32_t rotated_real = (int32_t)(product_real / Q30);
                int32_t rotated_imag = (int32_t)(product_imag / Q30);
                int32_t even_real = fft_real[even];
                int32_t even_imag = fft_imag[even];
                fft_real[even] = even_real + rotated_real;
                fft_imag[even] = even_imag + rotated_imag;
                fft_real[odd] = even_real - rotated_real;
                fft_imag[odd] = even_imag - rotated_imag;
            }
        }
    }
}

static uint64_t integer_sqrt(uint64_t value)
{
    uint64_t result = 0u;
    uint64_t bit = UINT64_C(1) << 62u;
    while (bit > value)
        bit >>= 2u;
    while (bit != 0u) {
        if (value >= result + bit) {
            value -= result + bit;
            result = (result >> 1u) + bit;
        } else {
            result >>= 1u;
        }
        bit >>= 2u;
    }
    return result;
}

static uint64_t magnitude(unsigned bin)
{
    int64_t real = fft_real[bin];
    int64_t imag = fft_imag[bin];
    uint64_t real_squared = (uint64_t)(real * real);
    uint64_t imag_squared = (uint64_t)(imag * imag);
    return integer_sqrt(real_squared + imag_squared);
}

static int is_harmonic_neighbour(unsigned bin, unsigned fundamental)
{
    /* Include folded aliases of the first 512 harmonic orders. This finite
       order avoids masking every bin after the coherent aliases repeat. */
    for (unsigned harmonic = 1u; harmonic <= SPECTRAL_HARMONICS; harmonic++) {
        unsigned harmonic_bin = (unsigned)(((uint64_t)harmonic * fundamental) % SPECTRAL_SAMPLES);
        if (harmonic_bin > SPECTRAL_SAMPLES / 2u)
            harmonic_bin = SPECTRAL_SAMPLES - harmonic_bin;
        unsigned distance = bin > harmonic_bin ? bin - harmonic_bin : harmonic_bin - bin;
        if (distance <= 1u)
            return 1;
    }
    return 0;
}

static int64_t log2_q20(uint64_t ratio_q30)
{
    int64_t exponent = 0;
    while (ratio_q30 < (uint64_t)Q30) {
        ratio_q30 <<= 1u;
        exponent--;
    }
    while (ratio_q30 >= (uint64_t)(Q30 * 2)) {
        ratio_q30 >>= 1u;
        exponent++;
    }
    uint32_t fraction = 0u;
    for (unsigned i = 0u; i < 20u; i++) {
        ratio_q30 = (ratio_q30 * ratio_q30) >> 30u;
        if (ratio_q30 >= (uint64_t)(Q30 * 2)) {
            ratio_q30 >>= 1u;
            fraction |= UINT32_C(1) << (19u - i);
        }
    }
    return (exponent * INT64_C(1048576)) + (int64_t)fraction;
}

static int32_t ratio_db_centi(uint64_t peak, uint64_t fundamental)
{
    if (peak == 0u || fundamental == 0u)
        return -20000;
    uint64_t ratio_q30 = (peak * (uint64_t)Q30) / fundamental;
    if (ratio_q30 == 0u)
        return -20000;
    int64_t log_ratio = log2_q20(ratio_q30);
    return (int32_t)((log_ratio * INT64_C(60206)) / INT64_C(104857600));
}

static void check_fft_with_coherent_sine(void)
{
    const unsigned bin = 512u;
    for (unsigned i = 0u; i < SPECTRAL_SAMPLES; i++) {
        unsigned phase_index = (i * bin) % SPECTRAL_SAMPLES;
        int64_t window = (Q30 - trig_cos[i]) / 2;
        int64_t sample = ((int64_t)trig_sin[phase_index] * INT64_C(12000)) / Q30;
        fft_real[i] = (int32_t)((sample * window) / Q30);
        fft_imag[i] = 0;
    }
    fft();
    uint64_t fundamental = magnitude(bin);
    uint64_t peak = 0u;
    for (unsigned candidate = 1u; candidate <= MAX_ANALYSIS_BIN; candidate++) {
        if (is_harmonic_neighbour(candidate, bin) == 0) {
            uint64_t candidate_magnitude = magnitude(candidate);
            if (candidate_magnitude > peak)
                peak = candidate_magnitude;
        }
    }
    CHECK(fundamental != 0u);
    CHECK(peak * UINT64_C(10000) < fundamental);
}

static unsigned native_bin(unsigned period, int triangle)
{
    uint64_t denominator =
        (triangle != 0 ? UINT64_C(32) : UINT64_C(16)) * ((uint64_t)period + 1u) * UINT64_C(48000);
    uint64_t numerator = UINT64_C(1789773) * SPECTRAL_SAMPLES;
    return (unsigned)((numerator + denominator / 2u) / denominator);
}

static int32_t triangle_level(unsigned phase)
{
    unsigned position = phase & 31u;
    unsigned dac = position < 16u ? 15u - position : position - 16u;
    return (int32_t)(dac * 1000u);
}

static void run_tone(unsigned period, int triangle, unsigned raw_bin, unsigned *fundamental_bin)
{
    const uint64_t event_denominator = (triangle != 0 ? UINT64_C(32) : UINT64_C(2)) * raw_bin;
    const uint64_t event_numerator = (uint64_t)SPECTRAL_SAMPLES * SAMPLE_PERIOD_UNITS;
    const uint64_t sample_period = SAMPLE_PERIOD_UNITS;
    const uint64_t total = TOTAL_SAMPLES;
    nesturbator *instance = make_instance();
    CHECK(instance != NULL);
    if (instance == NULL)
        return;
    struct nesturbator *nes = (struct nesturbator *)instance;
    nesturbator__synth_reset(nes);
    nes->apu.sample_phase = 0u;
    int32_t level = triangle != 0 ? triangle_level(0u) : 12000;
    nesturbator__synth_transition(nes, level);

    unsigned event_index = 1u;
    int16_t drained_sample = 0;
    int32_t window_start_level = 0;
    int level_recorded = 0;
    for (uint64_t sample = 0u; sample <= total; sample++) {
        if (sample == WARMUP_SAMPLES) {
            window_start_level = nes->synth.mixed_level;
            level_recorded = 1;
        }
        if (sample == total) {
            CHECK(level_recorded != 0);
            CHECK_EQ_U64((uint32_t)nes->synth.mixed_level, (uint32_t)window_start_level);
            break;
        }

        uint64_t boundary = (sample + 1u) * sample_period;
        while (((uint64_t)event_index * event_numerator) / event_denominator <= boundary) {
            uint64_t event_time = ((uint64_t)event_index * event_numerator) / event_denominator;
            uint32_t phase = (uint32_t)(event_time % sample_period);
            /* The core observes an edge on the CPU cycle immediately before
               a due sample, so a mathematically exact sample boundary uses
               the last representable phase of the preceding sample period. */
            nes->apu.sample_phase =
                phase == 0u ? (uint32_t)(sample_period - CPU_SAMPLE_STEP) : phase;
            if (triangle != 0)
                level = triangle_level(event_index);
            else
                level = (event_index & 1u) != 0u ? 0 : 12000;
            nesturbator__synth_transition(nes, level);
            event_index++;
        }

        nesturbator__synth_sample(nes);
        CHECK_EQ_U64(nesturbator__synth_drain(nes, &drained_sample, 1u), 1u);
        if (sample >= WARMUP_SAMPLES)
            tone_samples[sample - WARMUP_SAMPLES] = drained_sample;
    }

    CHECK_EQ_U64(nes->synth.overflow, 0u);
    *fundamental_bin = raw_bin <= SPECTRAL_SAMPLES / 2u ? raw_bin : SPECTRAL_SAMPLES - raw_bin;
    CHECK(*fundamental_bin != 0u);

    for (unsigned i = 0u; i < SPECTRAL_SAMPLES; i++) {
        int64_t window = (Q30 - trig_cos[i]) / 2;
        fft_real[i] = (int32_t)(((int64_t)tone_samples[i] * window) / Q30);
        fft_imag[i] = 0;
    }
    fft();

    uint64_t fundamental_magnitude = magnitude(*fundamental_bin);
    uint64_t peak_magnitude = 0u;
    unsigned peak_bin = 0u;
    for (unsigned bin = 1u; bin <= MAX_ANALYSIS_BIN; bin++) {
        if (is_harmonic_neighbour(bin, raw_bin) != 0)
            continue;
        uint64_t candidate = magnitude(bin);
        if (candidate > peak_magnitude) {
            peak_magnitude = candidate;
            peak_bin = bin;
        }
    }
    int32_t db_centi = ratio_db_centi(peak_magnitude, fundamental_magnitude);
    printf("%s period %u: fundamental bin %u, largest non-harmonic bin %u at %s%d.%02d dB\n",
           triangle != 0 ? "triangle" : "pulse", period, *fundamental_bin, peak_bin,
           db_centi < 0 ? "-" : "", db_centi < 0 ? (-db_centi / 100) : (db_centi / 100),
           db_centi < 0 ? (-db_centi % 100) : (db_centi % 100));
    CHECK(fundamental_magnitude != 0u);
    CHECK(peak_magnitude * UINT64_C(10000) < fundamental_magnitude);
    nesturbator_destroy(instance);
}

static void observe_transition(void *context, uint64_t cpu_cycle, int32_t level)
{
    struct transition_trace *trace = (struct transition_trace *)context;
    (void)level;
    if (cpu_cycle != trace->machine->cpu_cycle ||
        (trace->count != 0u && cpu_cycle < trace->last_cycle))
        trace->bad = 1u;
    trace->last_cycle = cpu_cycle;
    trace->count++;
}

static void make_public_image(uint8_t *image, size_t size, unsigned channel, uint8_t timer)
{
    memset(image, 0, size);
    memcpy(image, "NES\032", 4u);
    image[4] = 1u;
    image[5] = 1u;
    if (channel == 1u) {
        static const uint8_t program[] = {0xa9u, 0x01u, 0x8du, 0x15u, 0x40u, 0xa9u, 0xbfu, 0x8du,
                                          0x00u, 0x40u, 0xa9u, 0x00u, 0x8du, 0x02u, 0x40u, 0xa9u,
                                          0x00u, 0x8du, 0x03u, 0x40u, 0x4cu, 0x14u, 0x80u};
        memcpy(image + 16u, program, sizeof program);
        image[16u + 11u] = timer;
    } else if (channel == 2u) {
        static const uint8_t program[] = {0xa9u, 0x04u, 0x8du, 0x15u, 0x40u, 0xa9u, 0xffu, 0x8du,
                                          0x08u, 0x40u, 0xa9u, 0x00u, 0x8du, 0x0au, 0x40u, 0xa9u,
                                          0x00u, 0x8du, 0x0bu, 0x40u, 0x4cu, 0x14u, 0x80u};
        memcpy(image + 16u, program, sizeof program);
        image[16u + 11u] = timer;
    } else {
        static const uint8_t silent_loop[] = {0x4cu, 0x00u, 0x80u};
        memcpy(image + 16u, silent_loop, sizeof silent_loop);
    }
    image[16u + 0x3ffcu] = 0u;
    image[16u + 0x3ffdu] = 0x80u;
}

static nesturbator *make_public_machine(enum nesturbator_region_preference preference,
                                        unsigned channel, uint8_t timer)
{
    const size_t image_size = 16u + 16384u + 8192u;
    uint8_t *image = (uint8_t *)calloc(1u, image_size);
    nesturbator *instance = make_instance();
    CHECK(image != NULL && instance != NULL);
    if (image == NULL || instance == NULL) {
        free(image);
        nesturbator_destroy(instance);
        return NULL;
    }
    make_public_image(image, image_size, channel, timer);
    CHECK_EQ_U64(nesturbator_load_cartridge_region(instance, image, image_size, preference),
                 NESTURBATOR_OK);
    free(image);
    return instance;
}

static uint32_t expected_public_samples(int pal, uint32_t *remainder)
{
    const uint64_t frame_ticks = pal != 0 ? PAL_FRAME_TICKS : UINT64_C(714732);
    const uint32_t rate_num = pal != 0 ? PAL_EXPECTED_RATE_NUM : 352u;
    const uint32_t rate_den = pal != 0 ? PAL_EXPECTED_RATE_DEN : 315000u;
    uint64_t accumulated = *remainder + frame_ticks * rate_num;
    uint32_t count = (uint32_t)(accumulated / rate_den);
    *remainder = (uint32_t)(accumulated % rate_den);
    return count;
}

static uint32_t run_exact_public_frame(nesturbator *instance, int16_t *storage,
                                       uint32_t expected_count)
{
    CHECK(expected_count <= PUBLIC_FRAME_SAMPLES_MAX);
    storage[0] = PUBLIC_FRAME_GUARD;
    storage[expected_count + 1u] = PUBLIC_FRAME_GUARD;
    nesturbator_frame io;
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = public_video;
    io.video_pitch = 256u;
    io.audio = storage + 1u;
    io.audio_capacity = expected_count;
    CHECK_EQ_U64(nesturbator_run_frame(instance, &io), NESTURBATOR_OK);
    CHECK_EQ_U64(io.audio_count, expected_count);
    CHECK_EQ_U64((uint16_t)storage[0], (uint16_t)PUBLIC_FRAME_GUARD);
    CHECK_EQ_U64((uint16_t)storage[expected_count + 1u], (uint16_t)PUBLIC_FRAME_GUARD);
    return io.audio_count;
}

static unsigned pal_tone_expected_bin(unsigned timer, int triangle)
{
    /* Independent rational oracle: PAL CPU/(16*(timer+1)) for pulse and
       PAL CPU/(32*(timer+1)) for triangle. */
    const uint64_t numerator = UINT64_C(53203425) * SPECTRAL_SAMPLES;
    const uint64_t steps = triangle != 0 ? 32u : 16u;
    const uint64_t denominator = UINT64_C(32) * steps * (timer + 1u) * 48000u;
    return (unsigned)((numerator + denominator / 2u) / denominator);
}

static void test_public_pal_pcm_phase_and_pitch(void)
{
    const unsigned timer = 100u;
    nesturbator *pal_a = make_public_machine(NESTURBATOR_REGION_PREFERENCE_PAL, 1, (uint8_t)timer);
    nesturbator *pal_b = make_public_machine(NESTURBATOR_REGION_PREFERENCE_PAL, 1, (uint8_t)timer);
    nesturbator *ntsc = make_public_machine(NESTURBATOR_REGION_PREFERENCE_NTSC, 1, (uint8_t)timer);
    CHECK(pal_a != NULL && pal_b != NULL && ntsc != NULL);
    if (pal_a == NULL || pal_b == NULL || ntsc == NULL) {
        nesturbator_destroy(pal_a);
        nesturbator_destroy(pal_b);
        nesturbator_destroy(ntsc);
        return;
    }

    struct transition_trace trace;
    memset(&trace, 0, sizeof trace);
    trace.machine = (struct nesturbator *)pal_a;
    nesturbator__apu_set_transition_sink(trace.machine, observe_transition, &trace);

    int16_t pal_a_frame[PUBLIC_FRAME_SAMPLES_MAX + 2u];
    int16_t pal_b_frame[PUBLIC_FRAME_SAMPLES_MAX + 2u];
    int16_t ntsc_frame[PUBLIC_FRAME_SAMPLES_MAX + 2u];
    uint32_t pal_a_remainder = 0u;
    uint32_t pal_b_remainder = 0u;
    uint32_t ntsc_remainder = 0u;
    uint32_t captured = 0u;
    uint64_t pal_hash_a = UINT64_C(14695981039346656037);
    uint64_t pal_hash_b = UINT64_C(14695981039346656037);

    for (unsigned frame = 0u; frame < 40u; frame++) {
        uint32_t expected_a = expected_public_samples(1, &pal_a_remainder);
        uint32_t expected_b = expected_public_samples(1, &pal_b_remainder);
        uint32_t expected_ntsc = expected_public_samples(0, &ntsc_remainder);
        CHECK_EQ_U64(expected_a, expected_b);
        CHECK_EQ_U64(run_exact_public_frame(pal_a, pal_a_frame, expected_a), expected_a);
        CHECK_EQ_U64(run_exact_public_frame(ntsc, ntsc_frame, expected_ntsc), expected_ntsc);
        CHECK_EQ_U64(run_exact_public_frame(pal_b, pal_b_frame, expected_b), expected_b);
        CHECK_EQ_U64(
            memcmp(pal_a_frame + 1u, pal_b_frame + 1u, (size_t)expected_a * sizeof pal_a_frame[0]),
            0u);
        for (uint32_t i = 0u; i < expected_a; i++) {
            pal_hash_a = (pal_hash_a ^ (uint16_t)pal_a_frame[i + 1u]) * UINT64_C(1099511628211);
            pal_hash_b = (pal_hash_b ^ (uint16_t)pal_b_frame[i + 1u]) * UINT64_C(1099511628211);
        }
        uint32_t copy_count = expected_a;
        if (copy_count > TOTAL_SAMPLES - captured)
            copy_count = TOTAL_SAMPLES - captured;
        memcpy(public_capture + captured, pal_a_frame + 1u,
               (size_t)copy_count * sizeof public_capture[0]);
        captured += copy_count;

        struct nesturbator *machine = (struct nesturbator *)pal_a;
        CHECK_EQ_U64(machine->synth.overflow, 0u);
        CHECK(machine->synth.pcm_count <= PAL_RING_OVERSHOOT_BOUND);
        CHECK(machine->apu.sample_phase < PAL_EXPECTED_RATE_DEN);
        CHECK_EQ_U64(machine->audio_rem, pal_a_remainder);
        uint64_t nominal_ticks = (uint64_t)(frame + 1u) * PAL_FRAME_TICKS;
        CHECK(machine->ticks >= nominal_ticks);
        CHECK(machine->ticks - nominal_ticks <=
              (uint64_t)PAL_FRAME_OVERSHOOT_CPU_BOUND * 32u + 16u);
    }
    CHECK_EQ_U64(captured, TOTAL_SAMPLES);
    CHECK_EQ_U64(pal_hash_a, pal_hash_b);
    CHECK(trace.count != 0u);
    CHECK_EQ_U64(trace.bad, 0u);

    memcpy(tone_samples, public_capture + WARMUP_SAMPLES, sizeof tone_samples);
    for (unsigned i = 0u; i < SPECTRAL_SAMPLES; i++) {
        int64_t window = (Q30 - trig_cos[i]) / 2;
        fft_real[i] = (int32_t)(((int64_t)tone_samples[i] * window) / Q30);
        fft_imag[i] = 0;
    }
    fft();
    unsigned expected_bin = pal_tone_expected_bin(timer, 0);
    unsigned peak_bin = 0u;
    uint64_t peak = 0u;
    for (unsigned bin = 1u; bin <= MAX_ANALYSIS_BIN; bin++) {
        uint64_t candidate = magnitude(bin);
        if (candidate > peak) {
            peak = candidate;
            peak_bin = bin;
        }
    }
    CHECK(peak != 0u);
    CHECK(peak_bin + 1u >= expected_bin && peak_bin <= expected_bin + 1u);

    /* A loaded PAL machine with no enabled channels emits exact silence. */
    nesturbator *silent = make_public_machine(NESTURBATOR_REGION_PREFERENCE_PAL, 0, 0u);
    CHECK(silent != NULL);
    if (silent != NULL) {
        int16_t silence_frame[PUBLIC_FRAME_SAMPLES_MAX + 2u];
        uint32_t remainder = 0u;
        for (unsigned frame = 0u; frame < 3u; frame++) {
            uint32_t expected = expected_public_samples(1, &remainder);
            run_exact_public_frame(silent, silence_frame, expected);
            for (uint32_t i = 0u; i < expected; i++)
                CHECK_EQ_U64((uint16_t)silence_frame[i + 1u], 0u);
        }
        nesturbator_destroy(silent);
    }
    nesturbator_destroy(pal_a);
    nesturbator_destroy(pal_b);
    nesturbator_destroy(ntsc);
}

static void test_public_pal_triangle_pitch(void)
{
    const unsigned timer = 100u;
    nesturbator *triangle =
        make_public_machine(NESTURBATOR_REGION_PREFERENCE_PAL, 2u, (uint8_t)timer);
    CHECK(triangle != NULL);
    if (triangle == NULL)
        return;
    int16_t frame_samples[PUBLIC_FRAME_SAMPLES_MAX + 2u];
    uint32_t remainder = 0u;
    uint32_t captured = 0u;
    for (unsigned frame = 0u; frame < 40u; frame++) {
        uint32_t expected = expected_public_samples(1, &remainder);
        run_exact_public_frame(triangle, frame_samples, expected);
        uint32_t copy_count = expected;
        if (copy_count > TOTAL_SAMPLES - captured)
            copy_count = TOTAL_SAMPLES - captured;
        memcpy(public_capture + captured, frame_samples + 1u,
               (size_t)copy_count * sizeof public_capture[0]);
        captured += copy_count;
        struct nesturbator *machine = (struct nesturbator *)triangle;
        CHECK_EQ_U64(machine->synth.overflow, 0u);
        CHECK(machine->synth.pcm_count <= PAL_RING_OVERSHOOT_BOUND);
        CHECK(machine->apu.sample_phase < PAL_EXPECTED_RATE_DEN);
    }
    CHECK_EQ_U64(captured, TOTAL_SAMPLES);
    memcpy(tone_samples, public_capture + WARMUP_SAMPLES, sizeof tone_samples);
    for (unsigned i = 0u; i < SPECTRAL_SAMPLES; i++) {
        int64_t window = (Q30 - trig_cos[i]) / 2;
        fft_real[i] = (int32_t)(((int64_t)tone_samples[i] * window) / Q30);
        fft_imag[i] = 0;
    }
    fft();
    unsigned expected_bin = pal_tone_expected_bin(timer, 1);
    unsigned peak_bin = 0u;
    uint64_t peak = 0u;
    for (unsigned bin = 1u; bin <= MAX_ANALYSIS_BIN; bin++) {
        uint64_t candidate = magnitude(bin);
        if (candidate > peak) {
            peak = candidate;
            peak_bin = bin;
        }
    }
    CHECK(peak != 0u);
    CHECK(peak_bin + 1u >= expected_bin && peak_bin <= expected_bin + 1u);
    nesturbator_destroy(triangle);
}

int main(void)
{
    init_trig_table();
    check_fft_with_coherent_sine();
    static const unsigned pulse_periods[] = {100u, 40u, 12u, 8u};
    for (unsigned i = 0u; i < sizeof pulse_periods / sizeof pulse_periods[0]; i++) {
        unsigned period = pulse_periods[i];
        unsigned bin = native_bin(period, 0);
        unsigned fundamental_bin = 0u;
        run_tone(period, 0, bin, &fundamental_bin);
    }
    unsigned triangle_native_bin = native_bin(1u, 1);
    unsigned triangle_bin = triangle_native_bin <= SPECTRAL_SAMPLES / 2u
                                ? triangle_native_bin
                                : SPECTRAL_SAMPLES - triangle_native_bin;
    unsigned fundamental_bin = 0u;
    run_tone(1u, 1, triangle_bin, &fundamental_bin);
    test_public_pal_pcm_phase_and_pitch();
    test_public_pal_triangle_pitch();
    CHECK_DONE();
}
