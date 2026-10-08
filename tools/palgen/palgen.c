/* palgen: writes src/palette_ntsc.c, the 512-entry native pixel to XRGB8888
   table, from NTSC signal facts on the NESdev Wiki.
   Host tool only; nothing shipped links it. Built with floating-point
   contraction off so that every platform writes the same bytes (D-13).

     palgen <output.c>

   Facts used, from NESdev Wiki "NTSC video" rev 24244, "PPU palettes"
   rev 24257 and "Colour emphasis" rev 23220:
   - The PPU outputs a square wave at one of four levels, low or high on
     each of 12 subcarrier phases. Colour $xY with hue Y is high on phase p
     when (Y + p) mod 12 < 6. Hue 0 is high on every phase; hues 13-15 are
     low on every phase. $xE and $xF output the black level ($1D, 312 mV).
   - Each emphasis bit attenuates the signal on the six phases where its
     wave is high: native bit 6 (PPUMASK bit 5) on wave 12, bit 7 (PPUMASK
     bit 6) on wave 4, bit 8 (PPUMASK bit 7) on wave 8. $xE/$xF are excepted.
   - Colorburst is phase 8, on the -U axis.
   - The decoder averages the 12 samples for Y and demodulates U and V with
     a chroma gain of 2 at reference angles 15 + 30k degrees, then applies
     the YUV to R'G'B' matrix built on 0.299/0.587/0.114, 0.492111 (B-Y)
     and 0.877283 (R-Y).
   The angles' sines and cosines are +-(sqrt6-sqrt2)/4, +-sqrt2/2 and
   +-(sqrt6+sqrt2)/4. For display conversion, the generator applies the
   525-line BT.601 2.4 transfer curve, converts the linear primaries to
   sRGB/D65, then applies the sRGB transfer curve. The primary matrix is
   derived from the ICC BT.601 registry's 525-line chromaticities and the
   W3C sRGB specification. Out-of-gamut channels are clipped and channels
   are rounded to 8 bits. Sources: registry.color.org/rgb-registry/bt601
   and w3.org/Graphics/Color/srgb. */
#include <math.h>
#include <stdint.h>
#include <stdio.h>

/* Signal levels in millivolts, indexed by level 0-3 (NTSC video). */
static const int32_t LOW_MV[4] = {228, 312, 552, 880};
static const int32_t HIGH_MV[4] = {616, 840, 1100, 1100};
static const int32_t LOW_ATTEN_MV[4] = {192, 256, 448, 712};
static const int32_t HIGH_ATTEN_MV[4] = {500, 676, 896, 896};
#define BLACK_MV 312
#define WHITE_MV 1100

/* Matrix ratios, each numerator over its denominator. */
#define KR_NUM 299
#define KG_NUM 587
#define KB_NUM 114
#define K_DEN 1000
#define U_SCALE_NUM 492111 /* U = 0.492111 (B - Y) */
#define V_SCALE_NUM 877283 /* V = 0.877283 (R - Y) */
#define UV_SCALE_DEN 1000000
#define CHROMA_GAIN 2

/* Emphasis waves for native bits 6, 7 and 8 (Colour emphasis). */
static const int EMPHASIS_WAVE[3] = {12, 4, 8};

/* True if wave w is high on phase p. */
static int wave_high(int w, int p)
{
    return (w + p) % 12 < 6;
}

/* Cosine of 15 + 30k degrees, k in 0..11, from square roots only. */
static double cos_ref(int k)
{
    double s2 = sqrt(2.0);
    double s6 = sqrt(6.0);
    double mag;
    switch (k % 6) {
    case 0:
    case 5:
        mag = (s6 + s2) / 4.0; /* cos 15 */
        break;
    case 1:
    case 4:
        mag = s2 / 2.0; /* cos 45 */
        break;
    default:
        mag = (s6 - s2) / 4.0; /* cos 75 */
        break;
    }
    return (k >= 3 && k <= 8) ? -mag : mag;
}

/* Sine of 15 + 30k degrees: the sine of t is the cosine of t - 90 degrees,
   and 90 degrees is 3 steps. */
static double sin_ref(int k)
{
    return cos_ref((k + 9) % 12);
}

/* Signal on phase p for native pixel n, in millivolts. */
static int32_t signal_mv(int n, int p)
{
    int hue = n & 15;
    int level = (n >> 4) & 3;
    if (hue >= 14) {
        return BLACK_MV;
    }
    int high = hue == 0 ? 1 : hue >= 13 ? 0 : wave_high(hue, p);
    int atten = 0;
    for (int b = 0; b < 3; b++) {
        if (((n >> (6 + b)) & 1) != 0 && wave_high(EMPHASIS_WAVE[b], p)) {
            atten = 1;
        }
    }
    if (atten) {
        return high ? HIGH_ATTEN_MV[level] : LOW_ATTEN_MV[level];
    }
    return high ? HIGH_MV[level] : LOW_MV[level];
}

/* Y, U, V of native pixel n. Phase p is sampled at reference angle index
   (p + 2) mod 12, that is 75 + 30p degrees. */
static void decode(int n, double *y, double *u, double *v)
{
    double sy = 0.0;
    double su = 0.0;
    double sv = 0.0;
    for (int p = 0; p < 12; p++) {
        double s = (double)(signal_mv(n, p) - BLACK_MV) / (double)(WHITE_MV - BLACK_MV);
        int k = (p + 2) % 12;
        sy += s;
        su += s * sin_ref(k);
        sv += s * cos_ref(k);
    }
    *y = sy / 12.0;
    *u = su * (double)CHROMA_GAIN / 12.0;
    *v = sv * (double)CHROMA_GAIN / 12.0;
}

static uint32_t channel8(double x)
{
    if (x < 0.0) {
        x = 0.0;
    }
    if (x > 1.0) {
        x = 1.0;
    }
    return (uint32_t)(x * 255.0 + 0.5);
}

/* Linear 525-line BT.601 RGB to linear sRGB, both D65-relative. */
static void to_srgb_linear(double r, double g, double b, double *out_r, double *out_g,
                           double *out_b)
{
    *out_r = 0.939443387958641 * r + 0.050176359766052 * g + 0.010208624524547 * b;
    *out_g = 0.017849769637086 * r + 0.965754755942175 * g + 0.016440602870588 * b;
    *out_b = -0.001599027842839 * r - 0.004368737565189 * g + 1.006026237566082 * b;
}

static double encode_srgb(double x)
{
    if (x < 0.0) {
        x = 0.0;
    }
    if (x > 1.0) {
        x = 1.0;
    }
    return x <= 0.0031308 ? 12.92 * x : 1.055 * pow(x, 1.0 / 2.4) - 0.055;
}

static uint32_t xrgb(int n)
{
    double y, u, v;
    decode(n, &y, &u, &v);
    double kr = (double)KR_NUM / (double)K_DEN;
    double kg = (double)KG_NUM / (double)K_DEN;
    double kb = (double)KB_NUM / (double)K_DEN;
    double r_prime = y + v * (double)UV_SCALE_DEN / (double)V_SCALE_NUM;
    double b_prime = y + u * (double)UV_SCALE_DEN / (double)U_SCALE_NUM;
    double g_prime = (y - kr * r_prime - kb * b_prime) / kg;
    double r, g, b;
    to_srgb_linear(pow(fmax(0.0, r_prime), 2.4), pow(fmax(0.0, g_prime), 2.4),
                   pow(fmax(0.0, b_prime), 2.4), &r, &g, &b);
    return channel8(encode_srgb(r)) << 16 | channel8(encode_srgb(g)) << 8 |
           channel8(encode_srgb(b));
}

/* Colorburst is phase 8 on -U: hue 8 must decode to V = 0 and U < 0. */
static int self_check(void)
{
    double y, u, v;
    decode(0x18, &y, &u, &v);
    return fabs(v) < 1e-12 && u < 0.0;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: palgen <output.c>\n");
        return 2;
    }
    if (!self_check()) {
        fprintf(stderr, "palgen: hue 8 does not decode to -U; reference phase is wrong\n");
        return 1;
    }
    FILE *f = fopen(argv[1], "wb");
    if (f == NULL) {
        fprintf(stderr, "palgen: cannot open %s\n", argv[1]);
        return 1;
    }
    fprintf(f,
            "/* generated by tools/palgen -- do not edit.\n"
            "   Native pixel (palette entry | emphasis << 6) to XRGB8888 0x00RRGGBB.\n"
            "   Sources: NESdev Wiki \"NTSC video\" rev 24244, \"PPU palettes\" rev 24257,\n"
            "   \"Colour emphasis\" rev 23220; ICC BT.601 525-line registry; W3C sRGB.\n"
            "   - BT.601 525-line gamma 2.4; linear-light primary matrix to sRGB/D65;\n"
            "     sRGB transfer curve; out-of-gamut clipping and nearest 8-bit rounding\n"
            "   Parameters:\n"
            "   - levels low %d %d %d %d mV, high %d %d %d %d mV\n"
            "   - attenuated low %d %d %d %d mV, high %d %d %d %d mV\n"
            "   - black %d mV, white %d mV; 12 phases; colour $xY high when\n"
            "     (Y + p) mod 12 < 6; emphasis waves %d %d %d for native bits 6 7 8\n"
            "   - chroma gain %d; reference angles 15 + 30k degrees; colorburst phase 8\n"
            "   - Kr %d/%d, Kg %d/%d, Kb %d/%d; U = %d/%d (B - Y), V = %d/%d (R - Y)\n"
            "   - each channel clipped to 0..1 and rounded to 0..255 */\n"
            "#include \"internal.h\"\n"
            "\n"
            "/* Eight entries per line, two lines per row of 16. */\n"
            "/* clang-format off */\n"
            "const uint32_t nesturbator__palette_ntsc[512] = {\n",
            (int)LOW_MV[0], (int)LOW_MV[1], (int)LOW_MV[2], (int)LOW_MV[3], (int)HIGH_MV[0],
            (int)HIGH_MV[1], (int)HIGH_MV[2], (int)HIGH_MV[3], (int)LOW_ATTEN_MV[0],
            (int)LOW_ATTEN_MV[1], (int)LOW_ATTEN_MV[2], (int)LOW_ATTEN_MV[3], (int)HIGH_ATTEN_MV[0],
            (int)HIGH_ATTEN_MV[1], (int)HIGH_ATTEN_MV[2], (int)HIGH_ATTEN_MV[3], BLACK_MV, WHITE_MV,
            EMPHASIS_WAVE[0], EMPHASIS_WAVE[1], EMPHASIS_WAVE[2], CHROMA_GAIN, KR_NUM, K_DEN,
            KG_NUM, K_DEN, KB_NUM, K_DEN, U_SCALE_NUM, UV_SCALE_DEN, V_SCALE_NUM, UV_SCALE_DEN);
    for (int n = 0; n < 512; n++) {
        fprintf(f, "%s0x%06lX,%s", n % 8 == 0 ? "    " : " ", (unsigned long)xrgb(n),
                n % 8 == 7 ? "\n" : "");
    }
    fprintf(f, "};\n/* clang-format on */\n");
    if (fclose(f) != 0) {
        fprintf(stderr, "palgen: cannot write %s\n", argv[1]);
        return 1;
    }
    return 0;
}
