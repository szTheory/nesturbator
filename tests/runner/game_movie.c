/* Writes fixed input scripts (NMOVIE1) for native hash regression and for the
   nes-runner boot-to-play proof. No movie bytes are committed (D-05); every
   file is generated from the constants below at build time.

   Scenarios, one name per run:
     p0, p1, both       180-frame DABG scripts (the pinned dabg/... hash rows).
     nesrunner-pos      600 frames. Start (mask 0x08) at indexes 60-63 on
                        "Start Game", then six A (mask 0x01) jumps. Index k is
                        the input for frame k + 1; port 1 stays zero.
     nesrunner-none     600 frames, all zero. The baseline N for the A/B
                        predicates and the "no input" control.
     nesrunner-attract  600 frames of menu navigation that never plays: Down
                        0x20 at 60-61, Up 0x10 at 72-73 and 84-85, then Start
                        0x08 at 110-113 (enters Credits).
     nesrunner-startonly 600 frames, Start at 60-63 and no jumps. The player
                        dies near frame 265 and the score freezes.

   How the nes-runner constants were pinned (D-06). The tool was a scratch
   program that links the library, snapshots the instance with memcpy of
   struct nesturbator plus the cartridge's PRG RAM, and replays. It is not
   committed.
     1. Fix Start at index 60, hold 4.
     2. With the jump list so far, find the first frame d at which the game
        state byte leaves "playing".
     3. Sweep the next jump's start s downward from d - 1 and its hold h from 3
        to 28; keep the pairs for which the run is still alive 40 frames past d.
        With no jumps the player dies at frame 265.
     4. The latest feasible start is the edge: a start one frame later dies at
        d. Pin the middle of the feasible window, which leaves more slack
        against future timing churn than the edge does.
     5. Repeat for the next obstacle, then verify the finished list by shifting
        each jump start and hold by -2..+2 and requiring survival to frame 600.

   Latest-feasible list, reference only (these are the edge, not the pin):
     {261,21} {309,21} {357,21} {416,23} {469,21} {561,25}
   Pinned mid-window list, the one the constants below hold, as {start index,
   hold}:
     {251,21} {303,21} {351,21} {408,23} {463,21} {558,25}
   Measured survival windows around the pinned list (start shifts that keep the
   run alive, hold fixed): jump 1 -11..+10, jump 2 -13..+5, jump 3 -10..+6,
   jump 4 -7..+8, jump 5 -11..+6, jump 6 -3..+3 (the narrowest). Hold changes of
   +-2 did not change survival.

   The list is specific to Start index 60: another Start index shifts the first
   obstacle (death is always about Start + 205), so changing it means
   re-running the search. If core timing changes, the boot-to-play score
   threshold fails loudly and the search is re-run the same way. */
#include <stdio.h>
#include <string.h>

#define DABG_FRAMES 180u
#define NESRUNNER_FRAMES 600u
#define MAX_FRAMES NESRUNNER_FRAMES

#define BTN_A 0x01u
#define BTN_START 0x08u
#define BTN_UP 0x10u
#define BTN_DOWN 0x20u

struct span {
    unsigned start; /* first movie index */
    unsigned hold;  /* number of frames the button stays down */
};

/* The pinned mid-window jump list; the header comment explains the choice. */
static const struct span nesrunner_jumps[] = {{251u, 21u}, {303u, 21u}, {351u, 21u},
                                              {408u, 23u}, {463u, 21u}, {558u, 25u}};

static unsigned char in_span(unsigned frame, unsigned start, unsigned hold, unsigned char mask)
{
    return (frame >= start && frame < start + hold) ? mask : 0u;
}

static unsigned char nesrunner_mask(const char *scenario, unsigned frame)
{
    unsigned char mask = 0u;
    if (strcmp(scenario, "nesrunner-none") == 0)
        return 0u;
    if (strcmp(scenario, "nesrunner-attract") == 0) {
        mask |= in_span(frame, 60u, 2u, BTN_DOWN);
        mask |= in_span(frame, 72u, 2u, BTN_UP);
        mask |= in_span(frame, 84u, 2u, BTN_UP);
        mask |= in_span(frame, 110u, 4u, BTN_START);
        return mask;
    }
    mask |= in_span(frame, 60u, 4u, BTN_START);
    if (strcmp(scenario, "nesrunner-pos") == 0) {
        for (unsigned i = 0u; i < sizeof nesrunner_jumps / sizeof nesrunner_jumps[0]; i++)
            mask |= in_span(frame, nesrunner_jumps[i].start, nesrunner_jumps[i].hold, BTN_A);
    }
    return mask;
}

static int is_nesrunner(const char *scenario)
{
    return strcmp(scenario, "nesrunner-pos") == 0 || strcmp(scenario, "nesrunner-none") == 0 ||
           strcmp(scenario, "nesrunner-attract") == 0 ||
           strcmp(scenario, "nesrunner-startonly") == 0;
}

static int write_movie(const char *path, const char *scenario)
{
    static unsigned char bytes[16u + MAX_FRAMES * 4u];
    int port0 = strcmp(scenario, "p0") == 0 || strcmp(scenario, "both") == 0;
    int port1 = strcmp(scenario, "p1") == 0 || strcmp(scenario, "both") == 0;
    int runner = is_nesrunner(scenario);
    unsigned frame_count = runner ? NESRUNNER_FRAMES : DABG_FRAMES;
    size_t size = 16u + (size_t)frame_count * 4u;
    FILE *file;

    if (!port0 && !port1 && !runner)
        return 2;
    memset(bytes, 0, sizeof bytes);
    memcpy(bytes, "NMOVIE1", 7u);
    bytes[8] = 1u;
    bytes[12] = (unsigned char)(frame_count & 0xffu);
    bytes[13] = (unsigned char)((frame_count >> 8) & 0xffu);
    bytes[14] = (unsigned char)((frame_count >> 16) & 0xffu);
    bytes[15] = (unsigned char)((frame_count >> 24) & 0xffu);
    for (unsigned frame = 0; frame < frame_count; frame++) {
        unsigned char p0 = 0u, p1 = 0u;
        if (runner) {
            p0 = nesrunner_mask(scenario, frame);
        } else if (frame >= 24u && frame < 26u) {
            p0 = port0 ? 0x08u : 0u; /* Start */
            p1 = port1 ? 0x08u : 0u;
        } else if ((frame >= 48u && frame < 50u) || (frame >= 72u && frame < 74u) ||
                   (frame >= 96u && frame < 98u)) {
            p0 = port0 ? 0x01u : 0u; /* A */
            p1 = port1 ? 0x01u : 0u;
        } else if (frame >= 128u && frame < 160u) {
            p0 = port0 ? 0x80u : 0u; /* Right */
            p1 = port1 ? 0x40u : 0u; /* Left */
        }
        bytes[16u + frame * 4u] = p0;
        bytes[18u + frame * 4u] = p1;
    }
    file = fopen(path, "wb");
    if (file == NULL)
        return 1;
    int ok = fwrite(bytes, 1u, size, file) == size;
    if (fclose(file) != 0)
        ok = 0;
    return ok ? 0 : 1;
}

int main(int argc, char **argv)
{
    if (argc != 3)
        return 2;
    return write_movie(argv[1], argv[2]);
}
