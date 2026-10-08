#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "check.h"

static int write_empty_movie(const char *path)
{
    static const unsigned char bytes[16] = {
        'N', 'M', 'O', 'V', 'I', 'E', '1', 0, 1, 0, 0, 0, 0, 0, 0, 0};
    FILE *file = fopen(path, "wb");
    int ok;
    if (file == NULL)
        return 0;
    ok = fwrite(bytes, 1, sizeof bytes, file) == sizeof bytes;
    if (fclose(file) != 0)
        ok = 0;
    return ok;
}

int main(int argc, char **argv)
{
    char command[8192];
    int status;
    char output[256];
    FILE *file;

    CHECK(argc == 4);
    if (argc != 4)
        CHECK_DONE();
    CHECK(write_empty_movie(argv[2]));
    CHECK(snprintf(command, sizeof command, "\"%s\" --movie \"%s\" > \"%s\" 2>&1",
                   argv[1], argv[2], argv[3]) > 0);
    status = system(command);
    CHECK_EQ_U64(status, 0);
    file = fopen(argv[3], "rb");
    CHECK(file != NULL);
    if (file != NULL) {
        CHECK(fgets(output, sizeof output, file) == NULL);
        fclose(file);
    }
    CHECK_DONE();
}
