/* SND-04: deterministic fixed-point spectral checks for the production
   transition-to-PCM synthesizer. */
#include <stdint.h>
#include <stdio.h>
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

static int32_t trig_cos[SPECTRAL_SAMPLES];
static int32_t trig_sin[SPECTRAL_SAMPLES];
static int32_t fft_real[SPECTRAL_SAMPLES];
static int32_t fft_imag[SPECTRAL_SAMPLES];
static int16_t tone_samples[SPECTRAL_SAMPLES];

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
    CHECK_DONE();
}
