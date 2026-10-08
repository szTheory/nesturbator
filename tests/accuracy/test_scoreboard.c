#include <stdio.h>
#include <string.h>

#include "../check.h"

int main(void)
{
    FILE *file = fopen(NESTURBATOR_SOURCE_DIR "/tests/accuracy/scoreboard.txt", "r");
    char line[256];
    char previous[128] = "";
    unsigned rows = 0u;
    CHECK(file != NULL);
    if (file == NULL)
        return 1;
    while (fgets(line, sizeof line, file) != NULL) {
        char *fields[5];
        size_t field = 0u;
        char *start = line;
        char *newline = strchr(line, '\n');
        CHECK(newline != NULL);
        if (newline == NULL)
            break;
        *newline = '\0';
        for (char *p = line;; p++) {
            if (*p == '\t' || *p == '\0') {
                CHECK(field < 5u);
                if (field >= 5u)
                    break;
                fields[field++] = start;
                if (*p == '\0')
                    break;
                *p = '\0';
                start = p + 1;
            }
        }
        CHECK(field == 5u);
        if (field != 5u)
            continue;
        CHECK(strncmp(fields[0], "accuracycoin/", 13u) == 0);
        CHECK(strcmp(fields[1], "pass") == 0);
        CHECK(strlen(fields[2]) == 4u && fields[2][0] == '0' && fields[2][1] == 'x');
        CHECK(strcmp(fields[3], "-") == 0);
        CHECK(strcmp(fields[4], "-") == 0);
        CHECK(previous[0] == '\0' || strcmp(previous, fields[0]) < 0);
        (void)snprintf(previous, sizeof previous, "%s", fields[0]);
        rows++;
    }
    CHECK(fclose(file) == 0);
    CHECK(rows > 0u);
    CHECK_DONE();
}
