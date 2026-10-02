/* compare_frame RUNNER.ppm SHOT.bmp

   Exits 0 when the BMP is exactly 256x240 and every pixel's R, G and B equal
   the runner's P6 image. Exits 1 on a wrong size or a differing pixel, and
   names it; exits 2 on a file that cannot be read or parsed, or on wrong
   arguments (D-17). */
#include "bmp_ppm.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: compare_frame RUNNER.ppm SHOT.bmp\n");
        return 2;
    }
    char msg[512];
    int rc = nesturbator_test_compare_files(argv[1], argv[2], msg, sizeof msg);
    if (rc != 0)
        fprintf(stderr, "compare_frame: %s\n", msg);
    return rc;
}
