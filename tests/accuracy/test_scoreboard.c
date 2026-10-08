#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../check.h"

#define MAX_SCOREBOARD_ROWS 256u
#define SCOREBOARD_LINE_SIZE 256u

typedef struct scoreboard_row {
    char name[128];
    char status[16];
} scoreboard_row;

static int ci_enabled(void)
{
    const char *ci = getenv("CI");
    return ci != NULL && ci[0] != '\0' && strcmp(ci, "0") != 0 && strcmp(ci, "false") != 0;
}

static int load_scoreboard(const char *path, scoreboard_row *rows, size_t *count)
{
    FILE *file = fopen(path, "r");
    char line[SCOREBOARD_LINE_SIZE];
    char previous[128] = "";
    size_t row_count = 0u;
    CHECK(file != NULL);
    if (file == NULL) {
        (void)fprintf(stderr, "cannot read scoreboard: %s\n", path);
        return 0;
    }

    while (fgets(line, sizeof line, file) != NULL) {
        char *fields[5];
        size_t field = 0u;
        char *start = line;
        char *newline = strchr(line, '\n');
        CHECK(newline != NULL);
        if (newline == NULL) {
            (void)fprintf(stderr, "scoreboard row has no final newline: %s\n", path);
            (void)fclose(file);
            return 0;
        }
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
        if (field != 5u || row_count >= MAX_SCOREBOARD_ROWS) {
            (void)fprintf(stderr, "invalid or oversized scoreboard: %s\n", path);
            (void)fclose(file);
            return 0;
        }
        CHECK(strncmp(fields[0], "accuracycoin/", 13u) == 0);
        CHECK(strcmp(fields[1], "pass") == 0 || strcmp(fields[1], "fail") == 0 ||
              strcmp(fields[1], "unavailable") == 0 || strcmp(fields[1], "unsupported") == 0);
        CHECK(strlen(fields[2]) == 4u && fields[2][0] == '0' && fields[2][1] == 'x');
        CHECK(strcmp(fields[3], "-") == 0);
        CHECK(strcmp(fields[4], "-") == 0);
        CHECK(previous[0] == '\0' || strcmp(previous, fields[0]) < 0);
        if (strncmp(fields[0], "accuracycoin/", 13u) != 0 ||
            (strcmp(fields[1], "pass") != 0 && strcmp(fields[1], "fail") != 0 &&
             strcmp(fields[1], "unavailable") != 0 && strcmp(fields[1], "unsupported") != 0) ||
            strlen(fields[2]) != 4u || fields[2][0] != '0' || fields[2][1] != 'x' ||
            strcmp(fields[3], "-") != 0 || strcmp(fields[4], "-") != 0 ||
            (previous[0] != '\0' && strcmp(previous, fields[0]) >= 0)) {
            (void)fprintf(stderr, "invalid scoreboard row or ordering in: %s\n", path);
            (void)fclose(file);
            return 0;
        }
        (void)snprintf(rows[row_count].name, sizeof rows[row_count].name, "%s", fields[0]);
        (void)snprintf(rows[row_count].status, sizeof rows[row_count].status, "%s", fields[1]);
        (void)snprintf(previous, sizeof previous, "%s", fields[0]);
        row_count++;
    }
    CHECK(ferror(file) == 0);
    CHECK(fclose(file) == 0);
    *count = row_count;
    return 1;
}

static const char *baseline_path(int argc, char **argv)
{
    const char *baseline;
    if (argc == 2)
        return argv[1]; /* Explicit path is used only by the regression fixture. */
    if (argc != 1) {
        (void)fprintf(stderr, "usage: accuracy.scoreboard [regression-baseline]\n");
        return NULL;
    }
    if (ci_enabled()) {
        baseline = getenv("NESTURBATOR_SCOREBOARD_BASELINE");
        if (baseline == NULL || baseline[0] == '\0') {
            (void)fprintf(stderr, "CI requires NESTURBATOR_SCOREBOARD_BASELINE\n");
            return NULL;
        }
    } else {
        baseline = NESTURBATOR_SOURCE_DIR "/tests/accuracy/scoreboard-main.txt";
    }
    return baseline;
}

int main(int argc, char **argv)
{
    scoreboard_row current[MAX_SCOREBOARD_ROWS];
    scoreboard_row baseline_rows[MAX_SCOREBOARD_ROWS];
    size_t current_count = 0u;
    size_t baseline_count = 0u;
    const char *baseline = baseline_path(argc, argv);
    char candidate_path[512];
    int passed = 1;

    if (baseline == NULL)
        return 1;
    (void)snprintf(candidate_path, sizeof candidate_path, "%s/tests/accuracy/scoreboard.txt",
                   NESTURBATOR_SOURCE_DIR);
    if (!load_scoreboard(candidate_path, current, &current_count) || current_count == 0u ||
        !load_scoreboard(baseline, baseline_rows, &baseline_count))
        return 1;

    for (size_t prior = 0u; prior < baseline_count; prior++) {
        if (strcmp(baseline_rows[prior].status, "pass") != 0)
            continue;
        size_t now = 0u;
        while (now < current_count && strcmp(current[now].name, baseline_rows[prior].name) < 0)
            now++;
        if (now == current_count || strcmp(current[now].name, baseline_rows[prior].name) != 0 ||
            strcmp(current[now].status, "pass") != 0) {
            (void)fprintf(stderr, "scoreboard lost protected PASS: %s\n",
                          baseline_rows[prior].name);
            passed = 0;
        }
    }
    CHECK(passed);
    CHECK_DONE();
}
