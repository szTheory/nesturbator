#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "accuracycoin.h"
#include "sha256.h"

#define MAX_SCOREBOARD_ROWS 256u
#define MAX_MIGRATION_ROWS 32u
#define MAX_REPORT_ROWS ACCURACYCOIN_MAX_PAGE_ROWS
#define MAX_METADATA_FIELDS 64u
#define SCOREBOARD_LINE_SIZE 4096u
#define SCOREBOARD_KEY_SIZE 128u
#define SCOREBOARD_VALUE_SIZE 256u

#ifndef NESTURBATOR_SOURCE_DIR
#define NESTURBATOR_SOURCE_DIR "."
#endif
#ifndef NESTURBATOR_PROTECTED_EVIDENCE_DIR
#define NESTURBATOR_PROTECTED_EVIDENCE_DIR "build/accuracycoin/protected"
#endif

typedef struct scoreboard_row {
    char key[SCOREBOARD_KEY_SIZE];
    char status[16];
    char raw[8];
} scoreboard_row;

typedef struct metadata_field {
    char key[64];
    char value[SCOREBOARD_VALUE_SIZE];
} metadata_field;

typedef struct metadata {
    metadata_field fields[MAX_METADATA_FIELDS];
    size_t count;
} metadata;

typedef struct protected_migration {
    char old_pin[41];
    char old_key[SCOREBOARD_KEY_SIZE];
    char old_result_symbol[128];
    char old_routine_symbol[128];
    char new_pin[41];
    char new_key[SCOREBOARD_KEY_SIZE];
    char new_result_symbol[128];
    char new_routine_symbol[128];
    char disposition[32];
    char source_reference[1024];
    char assertion_change[512];
    char new_observation_reference[512];
} protected_migration;

typedef struct observation_row {
    char key[SCOREBOARD_KEY_SIZE];
    char status[16];
    char raw[8];
    char cause[64];
    char attempt_id[64];
    unsigned page;
    unsigned row;
    unsigned observed;
    uint64_t frames;
} observation_row;

typedef struct page_observation {
    metadata meta;
    observation_row rows[MAX_REPORT_ROWS];
    size_t count;
} page_observation;

typedef struct protected_context {
    scoreboard_row baseline[MAX_SCOREBOARD_ROWS];
    scoreboard_row candidate[MAX_SCOREBOARD_ROWS];
    size_t baseline_count;
    size_t candidate_count;
    protected_migration migrations[MAX_MIGRATION_ROWS];
    size_t migration_count;
    accuracycoin_directory old_directory;
    accuracycoin_directory new_directory;
    accuracycoin_directory candidate_directory;
    page_observation old_pages[3];
    page_observation new_pages[3];
    char protected_revision[41];
    char baseline_pin[41];
    char candidate_pin[41];
} protected_context;

static int set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0u)
        (void)snprintf(error, error_size, "%s", message);
    return 0;
}

static int copy_text(char *out, size_t capacity, const char *text)
{
    size_t length = strlen(text);
    if (length >= capacity)
        return 0;
    memcpy(out, text, length + 1u);
    return 1;
}

static int split_tabs(char *line, char **fields, size_t capacity, size_t *count)
{
    size_t length = strlen(line);
    size_t field_count = 0u;
    char *start = line;
    if (length == 0u || line[length - 1u] != '\n')
        return 0;
    line[--length] = '\0';
    if (length > 0u && line[length - 1u] == '\r')
        line[--length] = '\0';
    for (char *cursor = line;; cursor++) {
        if (*cursor == '\t' || *cursor == '\0') {
            if (field_count >= capacity)
                return 0;
            fields[field_count++] = start;
            if (*cursor == '\0')
                break;
            *cursor = '\0';
            start = cursor + 1;
        }
    }
    *count = field_count;
    return 1;
}

static int parse_unsigned(const char *text, uint64_t *value)
{
    char *end = NULL;
    unsigned long long parsed;
    if (text == NULL || text[0] == '\0' || text[0] == '-')
        return 0;
    parsed = strtoull(text, &end, 10);
    if (end == text || end == NULL || *end != '\0')
        return 0;
    *value = (uint64_t)parsed;
    return 1;
}

static int valid_hex(const char *text, size_t digits)
{
    if (strlen(text) != digits)
        return 0;
    for (size_t i = 0u; i < digits; i++)
        if (!isdigit((unsigned char)text[i]) && !(text[i] >= 'a' && text[i] <= 'f'))
            return 0;
    return 1;
}

static const char *metadata_value(const metadata *meta, const char *key)
{
    for (size_t i = 0u; i < meta->count; i++)
        if (strcmp(meta->fields[i].key, key) == 0)
            return meta->fields[i].value;
    return NULL;
}

static const char *metadata_or_empty(const metadata *meta, const char *key)
{
    const char *value = metadata_value(meta, key);
    return value != NULL ? value : "";
}

static int load_metadata(const char *path, metadata *out, char *error, size_t error_size)
{
    FILE *file = fopen(path, "r");
    char line[SCOREBOARD_LINE_SIZE];
    memset(out, 0, sizeof *out);
    if (file == NULL)
        return set_error(error, error_size, "cannot read metadata file");
    while (fgets(line, sizeof line, file) != NULL) {
        char *equals = strchr(line, '=');
        size_t length = strlen(line);
        metadata_field *field;
        if (length == 0u || line[length - 1u] != '\n' || equals == NULL || equals == line ||
            equals[1] == '\0' || strchr(equals + 1, '=') != NULL) {
            (void)fclose(file);
            return set_error(error, error_size, "metadata row syntax is invalid");
        }
        line[length - 1u] = '\0';
        if (out->count >= MAX_METADATA_FIELDS) {
            (void)fclose(file);
            return set_error(error, error_size, "metadata has too many fields");
        }
        *equals = '\0';
        if (strlen(line) >= sizeof out->fields[0].key ||
            strlen(equals + 1) >= sizeof out->fields[0].value) {
            (void)fclose(file);
            return set_error(error, error_size, "metadata field exceeds its bound");
        }
        for (size_t i = 0u; i < out->count; i++)
            if (strcmp(out->fields[i].key, line) == 0) {
                (void)fclose(file);
                return set_error(error, error_size, "metadata has duplicate keys");
            }
        field = &out->fields[out->count++];
        (void)copy_text(field->key, sizeof field->key, line);
        (void)copy_text(field->value, sizeof field->value, equals + 1);
    }
    if (ferror(file) != 0 || fclose(file) != 0)
        return set_error(error, error_size, "metadata read failed");
    return 1;
}

static int metadata_number(const metadata *meta, const char *key, uint64_t *out)
{
    const char *text = metadata_value(meta, key);
    return text != NULL && parse_unsigned(text, out);
}

static int load_scoreboard(const char *path, scoreboard_row *rows, size_t *count,
                           char *error, size_t error_size)
{
    FILE *file = fopen(path, "r");
    char line[SCOREBOARD_LINE_SIZE];
    size_t row_count = 0u;
    char previous[SCOREBOARD_KEY_SIZE] = "";
    uint64_t frame_count;
    if (file == NULL)
        return set_error(error, error_size, "cannot read scoreboard");
    while (fgets(line, sizeof line, file) != NULL) {
        char *fields[5];
        size_t field_count = 0u;
        if (!split_tabs(line, fields, 5u, &field_count) || field_count != 5u ||
            row_count >= MAX_SCOREBOARD_ROWS || strncmp(fields[0], "accuracycoin/", 13u) != 0 ||
            strlen(fields[0]) >= sizeof rows[0].key || strlen(fields[1]) >= sizeof rows[0].status ||
            strlen(fields[2]) >= sizeof rows[0].raw ||
            (strcmp(fields[1], "pass") != 0 && strcmp(fields[1], "fail") != 0 &&
             strcmp(fields[1], "skip") != 0 && strcmp(fields[1], "not-run") != 0 &&
             strcmp(fields[1], "timeout") != 0 && strcmp(fields[1], "unavailable") != 0 &&
             strcmp(fields[1], "unsupported") != 0) ||
            (strcmp(fields[2], "-") != 0 &&
             (strlen(fields[2]) != 4u || fields[2][0] != '0' || fields[2][1] != 'x' ||
              !valid_hex(fields[2] + 2, 2u))) ||
            (strcmp(fields[3], "-") != 0 && !parse_unsigned(fields[3], &frame_count)) ||
            (strcmp(fields[4], "-") != 0 && !valid_hex(fields[4], 64u)) ||
            (previous[0] != '\0' && strcmp(previous, fields[0]) >= 0)) {
            (void)fclose(file);
            return set_error(error, error_size, "scoreboard row is invalid, duplicated, or unsorted");
        }
        (void)copy_text(rows[row_count].key, sizeof rows[row_count].key, fields[0]);
        (void)copy_text(rows[row_count].status, sizeof rows[row_count].status, fields[1]);
        (void)copy_text(rows[row_count].raw, sizeof rows[row_count].raw, fields[2]);
        (void)copy_text(previous, sizeof previous, fields[0]);
        row_count++;
    }
    if (ferror(file) != 0 || fclose(file) != 0)
        return set_error(error, error_size, "scoreboard read failed");
    if (row_count == 0u)
        return set_error(error, error_size, "scoreboard is empty");
    *count = row_count;
    return 1;
}

static int hash_file(const char *path, char out[65])
{
    FILE *file = fopen(path, "rb");
    uint8_t buffer[4096];
    uint8_t digest[32];
    nesturbator_run_sha256 hash;
    if (file == NULL)
        return 0;
    nesturbator_run_sha256_init(&hash);
    for (;;) {
        size_t count = fread(buffer, 1u, sizeof buffer, file);
        if (count > 0u)
            nesturbator_run_sha256_update(&hash, buffer, count);
        if (count < sizeof buffer) {
            if (ferror(file) != 0 || fclose(file) != 0)
                return 0;
            break;
        }
    }
    nesturbator_run_sha256_final(&hash, digest);
    nesturbator_run_sha256_hex(digest, out);
    return 1;
}

static int load_directory(const char *path, accuracycoin_directory *directory,
                          char *error, size_t error_size)
{
    FILE *file = fopen(path, "rb");
    uint8_t *bytes = NULL;
    long size;
    accuracycoin_reason reason;
    int ok;
    if (file == NULL)
        return set_error(error, error_size, "cannot read pinned AccuracyCoin ROM");
    if (fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        (void)fclose(file);
        return set_error(error, error_size, "cannot size pinned AccuracyCoin ROM");
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (bytes == NULL || fread(bytes, 1u, (size_t)size, file) != (size_t)size ||
        fclose(file) != 0) {
        free(bytes);
        return set_error(error, error_size, "cannot read complete pinned AccuracyCoin ROM");
    }
    ok = accuracycoin_parse_pinned(bytes, (size_t)size, directory, &reason);
    free(bytes);
    if (!ok)
        return set_error(error, error_size, accuracycoin_reason_text(reason));
    return 1;
}

static const accuracycoin_test *inventory_row(const accuracycoin_directory *directory,
                                               const char *key)
{
    const char *name = strncmp(key, "accuracycoin/", 13u) == 0 ? key + 13u : "";
    for (size_t i = 0u; i < directory->row_count; i++)
        if (directory->rows[i].kind == (uint8_t)ACCURACYCOIN_KIND_TEST &&
            strcmp(directory->rows[i].name, name) == 0)
            return &directory->rows[i];
    return NULL;
}

static const scoreboard_row *scoreboard_find(const scoreboard_row *rows, size_t count,
                                             const char *key)
{
    for (size_t i = 0u; i < count; i++)
        if (strcmp(rows[i].key, key) == 0)
            return &rows[i];
    return NULL;
}

static const observation_row *observation_find(const page_observation pages[3],
                                               const char *key)
{
    for (size_t page = 0u; page < 3u; page++)
        for (size_t row = 0u; row < pages[page].count; row++)
            if (strcmp(pages[page].rows[row].key, key) == 0)
                return &pages[page].rows[row];
    return NULL;
}

static int parse_raw(const char *text, uint8_t *raw)
{
    char *end = NULL;
    unsigned long value;
    if (strlen(text) != 4u || text[0] != '0' || text[1] != 'x' || !valid_hex(text + 2, 2u))
        return 0;
    value = strtoul(text + 2, &end, 16);
    if (end == text + 2 || end == NULL || *end != '\0' || value > 255u)
        return 0;
    *raw = (uint8_t)value;
    return 1;
}

static int parse_hex_value(const char *text, uint64_t *value)
{
    char *end = NULL;
    unsigned long long parsed;
    if (text == NULL || text[0] == '\0' || text[0] == '-')
        return 0;
    parsed = strtoull(text, &end, 0);
    if (end == text || end == NULL || *end != '\0')
        return 0;
    *value = (uint64_t)parsed;
    return 1;
}

static int load_final_raw(const char *path, const accuracycoin_directory *directory,
                          unsigned page, const observation_row *report_rows, size_t report_count,
                          char *error, size_t error_size)
{
    static const char header[] = "stage\tpage\trow\tkey\tresult_address\tobserved\traw\n";
    FILE *file = fopen(path, "r");
    char line[SCOREBOARD_LINE_SIZE];
    size_t final_count = 0u;
    if (file == NULL)
        return set_error(error, error_size, "cannot read raw observation evidence");
    if (fgets(line, sizeof line, file) == NULL || strcmp(line, header) != 0) {
        (void)fclose(file);
        return set_error(error, error_size, "raw observation header is invalid");
    }
    while (fgets(line, sizeof line, file) != NULL) {
        char *fields[7];
        size_t count = 0u;
        uint64_t page_value, row_value, address_value;
        const accuracycoin_test *expected;
        if (!split_tabs(line, fields, 7u, &count) || count != 7u || strcmp(fields[0], "final") != 0)
            continue;
        if (!parse_unsigned(fields[1], &page_value) || !parse_unsigned(fields[2], &row_value) ||
            !parse_hex_value(fields[4], &address_value) || page_value != page || row_value == 0u ||
            row_value > ACCURACYCOIN_MAX_PAGE_ROWS) {
            (void)fclose(file);
            return set_error(error, error_size, "final raw evidence row is malformed");
        }
        expected = inventory_row(directory, fields[3]);
        if (expected == NULL || expected->page != page || expected->row != row_value ||
            expected->result_address != address_value || strcmp(fields[5], "1") != 0) {
            (void)fclose(file);
            return set_error(error, error_size, "final raw evidence does not map to the pinned inventory");
        }
        uint8_t raw;
        if (!parse_raw(fields[6], &raw)) {
            (void)fclose(file);
            return set_error(error, error_size, "final raw evidence byte is invalid");
        }
        int found = 0;
        for (size_t i = 0u; i < report_count; i++)
            if (strcmp(report_rows[i].key, fields[3]) == 0) {
                uint8_t report_raw;
                if (!parse_raw(report_rows[i].raw, &report_raw) || report_raw != raw) {
                    (void)fclose(file);
                    return set_error(error, error_size, "report raw byte differs from final sample");
                }
                found++;
            }
        if (found != 1) {
            (void)fclose(file);
            return set_error(error, error_size, "final raw evidence key is missing or duplicated");
        }
        final_count++;
    }
    if (ferror(file) != 0 || fclose(file) != 0)
        return set_error(error, error_size, "raw observation evidence read failed");
    if (final_count != report_count)
        return set_error(error, error_size, "final raw evidence is incomplete");
    return 1;
}

static int load_page_observation(const char *evidence_root, const char *pin_name,
                                 unsigned page, const accuracycoin_directory *directory,
                                 page_observation *out, char *error, size_t error_size)
{
    char directory_path[1024];
    char metadata_path[1100];
    char report_path[1100];
    char raw_path[1100];
    char line[SCOREBOARD_LINE_SIZE];
    static const char report_header[] =
        "key\tpage\trow\tstatus\tobserved\traw\tframes\tcause\tattempt_id\n";
    FILE *report;
    uint64_t number;
    const char *value;
    const accuracycoin_page *inventory_page = &directory->pages[page - 1u];
    memset(out, 0, sizeof *out);
    if (snprintf(directory_path, sizeof directory_path, "%s/%s/page-%02u", evidence_root,
                 pin_name, page) < 0 ||
        snprintf(metadata_path, sizeof metadata_path, "%s/metadata.txt", directory_path) < 0 ||
        snprintf(report_path, sizeof report_path, "%s/report.tsv", directory_path) < 0 ||
        snprintf(raw_path, sizeof raw_path, "%s/raw.tsv", directory_path) < 0)
        return set_error(error, error_size, "protected observation path is too long");
    if (!load_metadata(metadata_path, &out->meta, error, error_size))
        return 0;
    if (strcmp(metadata_or_empty(&out->meta, "report_schema"), "1") != 0 ||
        strcmp(metadata_or_empty(&out->meta, "inventory_schema"), "1") != 0 ||
        strcmp(metadata_or_empty(&out->meta, "rom_pin"), directory->pin) != 0 ||
        strcmp(metadata_or_empty(&out->meta, "rom_sha256"), directory->rom_sha256) != 0 ||
        strcmp(metadata_or_empty(&out->meta, "inventory_sha256"), directory->inventory_sha256) != 0 ||
        strcmp(metadata_or_empty(&out->meta, "mode"), "observe") != 0 ||
        strcmp(metadata_or_empty(&out->meta, "origin"), "cold-page") != 0 ||
        strcmp(metadata_or_empty(&out->meta, "requested_row"), "all") != 0 ||
        strcmp(metadata_or_empty(&out->meta, "input_start_policy"),
               "cold-boot;page-header-A;one-frame-A-press;one-frame-release") != 0 ||
        strcmp(metadata_or_empty(&out->meta, "verdict"), "complete") != 0 ||
        strcmp(metadata_or_empty(&out->meta, "stop_cause"), "all-page-results-terminal") != 0 ||
        strcmp(metadata_or_empty(&out->meta, "selection_observed"), "1") != 0 ||
        strcmp(metadata_or_empty(&out->meta, "selected_cursor"), "255") != 0 ||
        strcmp(metadata_or_empty(&out->meta, "a_press_observed"), "1") != 0 ||
        strcmp(metadata_or_empty(&out->meta, "a_release_observed"), "1") != 0 ||
        strcmp(metadata_or_empty(&out->meta, "release_frames"), "1") != 0 ||
        strcmp(metadata_or_empty(&out->meta, "run_state_observed"), "1") != 0 ||
        strcmp(metadata_or_empty(&out->meta, "result_transition_observed"), "1") != 0 ||
        strcmp(metadata_or_empty(&out->meta, "start_attributed"), "1") != 0 ||
        strcmp(metadata_or_empty(&out->meta, "source_revision"), "") == 0 ||
        !metadata_number(&out->meta, "observation_started_ns", &number) || number == 0u ||
        !metadata_number(&out->meta, "page", &number) || number != page ||
        !metadata_number(&out->meta, "selected_page", &number) || number != page - 1u ||
        !metadata_number(&out->meta, "boot_frame_budget", &number) || number != 32u ||
        !metadata_number(&out->meta, "navigation_frame_budget_per_page", &number) || number != 5u ||
        !metadata_number(&out->meta, "execution_frame_budget", &number) || number != 1800u ||
        !metadata_number(&out->meta, "execution_frames", &number) || number == 0u || number > 1800u ||
        !metadata_number(&out->meta, "total_frame_budget", &number) ||
        number != 32u + (uint64_t)(page - 1u) * 5u + 2u + 1800u)
        return set_error(error, error_size, "protected observation metadata does not prove a qualified bounded start");
    char expected_attempt[64];
    (void)snprintf(expected_attempt, sizeof expected_attempt, "page-%02u-cold-01", page);
    value = metadata_value(&out->meta, "attempt_id");
    if (value == NULL || strcmp(value, expected_attempt) != 0)
        return set_error(error, error_size, "protected observation attempt identity is invalid");

    report = fopen(report_path, "r");
    if (report == NULL)
        return set_error(error, error_size, "cannot read protected page report");
    if (fgets(line, sizeof line, report) == NULL || strcmp(line, report_header) != 0) {
        (void)fclose(report);
        return set_error(error, error_size, "protected page report header is invalid");
    }
    while (fgets(line, sizeof line, report) != NULL) {
        char *fields[9];
        size_t field_count = 0u;
        uint64_t row_page, row_number, observed, frames;
        observation_row *row;
        const accuracycoin_test *inventory;
        uint8_t raw = 0u;
        const char *cause = NULL;
        accuracycoin_result_status classified;
        if (!split_tabs(line, fields, 9u, &field_count) || field_count != 9u ||
            out->count >= MAX_REPORT_ROWS || !parse_unsigned(fields[1], &row_page) ||
            !parse_unsigned(fields[2], &row_number) || !parse_unsigned(fields[4], &observed) ||
            !parse_unsigned(fields[6], &frames) || row_page != page || row_number == 0u ||
            observed > 1u || frames == 0u || frames > 1800u || strlen(fields[0]) >= SCOREBOARD_KEY_SIZE ||
            strlen(fields[3]) >= sizeof out->rows[0].status || strlen(fields[5]) >= sizeof out->rows[0].raw ||
            strlen(fields[7]) >= sizeof out->rows[0].cause || strlen(fields[8]) >= sizeof out->rows[0].attempt_id ||
            strcmp(fields[8], expected_attempt) != 0) {
            (void)fclose(report);
            return set_error(error, error_size, "protected page report row is malformed");
        }
        inventory = inventory_row(directory, fields[0]);
        if (inventory == NULL || inventory->page != page || inventory->row != row_number) {
            (void)fclose(report);
            return set_error(error, error_size, "protected page report key differs from the pinned inventory");
        }
        for (size_t i = 0u; i < out->count; i++)
            if (strcmp(out->rows[i].key, fields[0]) == 0) {
                (void)fclose(report);
                return set_error(error, error_size, "protected page report duplicates a key");
            }
        if (observed == 1u) {
            if (!parse_raw(fields[5], &raw)) {
                (void)fclose(report);
                return set_error(error, error_size, "protected page report raw byte is invalid");
            }
        } else if (strcmp(fields[5], "-") != 0) {
            (void)fclose(report);
            return set_error(error, error_size, "unobserved protected page row has a raw byte");
        }
        classified = accuracycoin_classify(fields[0], (int)observed, raw,
                                            strcmp(fields[7], "execution-cap-active-test") == 0,
                                            &cause);
        if (classified == ACCURACYCOIN_RESULT_INVALID ||
            strcmp(fields[3], accuracycoin_status_text(classified)) != 0) {
            (void)fclose(report);
            return set_error(error, error_size, "protected page report status does not match its raw byte");
        }
        row = &out->rows[out->count++];
        (void)copy_text(row->key, sizeof row->key, fields[0]);
        (void)copy_text(row->status, sizeof row->status, fields[3]);
        (void)copy_text(row->raw, sizeof row->raw, fields[5]);
        (void)copy_text(row->cause, sizeof row->cause, fields[7]);
        (void)copy_text(row->attempt_id, sizeof row->attempt_id, fields[8]);
        row->page = (unsigned)row_page;
        row->row = (unsigned)row_number;
        row->observed = (unsigned)observed;
        row->frames = frames;
    }
    if (ferror(report) != 0 || fclose(report) != 0)
        return set_error(error, error_size, "protected page report read failed");
    if (out->count != inventory_page->row_count)
        return set_error(error, error_size, "protected page report does not cover the full page inventory");
    if (!load_final_raw(raw_path, directory, page, out->rows, out->count, error, error_size))
        return 0;
    return 1;
}

static int load_manifest_identity(const char *path, const accuracycoin_directory *directory,
                                  char *error, size_t error_size)
{
    FILE *file = fopen(path, "r");
    char line[SCOREBOARD_LINE_SIZE];
    unsigned matches = 0u;
    if (file == NULL)
        return set_error(error, error_size, "cannot read candidate ROM manifest");
    while (fgets(line, sizeof line, file) != NULL) {
        char *fields[6];
        size_t count = 0u;
        if (line[0] == '#')
            continue;
        if (!split_tabs(line, fields, 6u, &count) || count != 5u) {
            (void)fclose(file);
            return set_error(error, error_size, "candidate ROM manifest row is malformed");
        }
        if (strcmp(fields[0], "tests/roms/accuracycoin.nes") == 0) {
            matches++;
            char expected_url[256];
            (void)snprintf(expected_url, sizeof expected_url,
                           "https://github.com/100thCoin/AccuracyCoin/tree/%s", directory->pin);
            if (strcmp(fields[1], expected_url) != 0 || strcmp(fields[2], directory->pin) != 0 ||
                strcmp(fields[3], "MIT") != 0 || strcmp(fields[4], directory->rom_sha256) != 0) {
                (void)fclose(file);
                return set_error(error, error_size, "candidate manifest identity does not match candidate ROM bytes");
            }
        }
    }
    if (ferror(file) != 0 || fclose(file) != 0)
        return set_error(error, error_size, "candidate ROM manifest read failed");
    if (matches != 1u)
        return set_error(error, error_size, "candidate manifest must identify exactly one canonical AccuracyCoin ROM");
    return 1;
}

static int load_migrations(const char *path, protected_context *context,
                           char *error, size_t error_size)
{
    static const char header[] =
        "old_pin\told_key\told_result_symbol\told_routine_symbol\tnew_pin\tnew_key\t"
        "new_result_symbol\tnew_routine_symbol\tdisposition\tsource_reference\t"
        "assertion_change\tnew_observation_reference\n";
    FILE *file = fopen(path, "r");
    char line[SCOREBOARD_LINE_SIZE];
    if (file == NULL)
        return set_error(error, error_size, "cannot read protected migration table");
    if (fgets(line, sizeof line, file) == NULL || strcmp(line, header) != 0) {
        (void)fclose(file);
        return set_error(error, error_size, "protected migration header is invalid");
    }
    while (fgets(line, sizeof line, file) != NULL) {
        char *fields[12];
        size_t count = 0u;
        protected_migration *migration;
        if (!split_tabs(line, fields, 12u, &count) || count != 12u ||
            context->migration_count >= MAX_MIGRATION_ROWS) {
            (void)fclose(file);
            return set_error(error, error_size, "protected migration row is malformed or oversized");
        }
        migration = &context->migrations[context->migration_count++];
        char *targets[] = {migration->old_pin, migration->old_key, migration->old_result_symbol,
                           migration->old_routine_symbol, migration->new_pin, migration->new_key,
                           migration->new_result_symbol, migration->new_routine_symbol,
                           migration->disposition, migration->source_reference,
                           migration->assertion_change, migration->new_observation_reference};
        size_t capacities[] = {sizeof migration->old_pin, sizeof migration->old_key,
                               sizeof migration->old_result_symbol, sizeof migration->old_routine_symbol,
                               sizeof migration->new_pin, sizeof migration->new_key,
                               sizeof migration->new_result_symbol, sizeof migration->new_routine_symbol,
                               sizeof migration->disposition, sizeof migration->source_reference,
                               sizeof migration->assertion_change, sizeof migration->new_observation_reference};
        for (size_t i = 0u; i < 12u; i++)
            if (!copy_text(targets[i], capacities[i], fields[i])) {
                (void)fclose(file);
                return set_error(error, error_size, "protected migration field exceeds its bound");
            }
    }
    if (ferror(file) != 0 || fclose(file) != 0)
        return set_error(error, error_size, "protected migration table read failed");
    if (context->migration_count != 17u)
        return set_error(error, error_size, "protected migration table must cover all 17 retained keys");
    return 1;
}

static const protected_migration *migration_find(const protected_context *context,
                                                  const char *key, int use_new_key)
{
    for (size_t i = 0u; i < context->migration_count; i++) {
        const char *candidate = use_new_key ? context->migrations[i].new_key :
                                              context->migrations[i].old_key;
        if (strcmp(candidate, key) == 0)
            return &context->migrations[i];
    }
    return NULL;
}

static int is_reviewed_changed_key(const char *key)
{
    static const char *const keys[] = {
        "accuracycoin/Frame Counter 4-step",
        "accuracycoin/Frame Counter 5-step",
        "accuracycoin/Frame Counter IRQ",
        "accuracycoin/PPU Register Open Bus"};
    for (size_t i = 0u; i < sizeof keys / sizeof keys[0]; i++)
        if (strcmp(key, keys[i]) == 0)
            return 1;
    return 0;
}

static int validate_migration_row(const protected_context *context,
                                  const protected_migration *migration,
                                  const observation_row *new_observation,
                                  char *error, size_t error_size)
{
    char old_source[128];
    char new_source[128];
    char expected_reference[512];
    const accuracycoin_test *old_inventory = inventory_row(&context->old_directory, migration->old_key);
    const accuracycoin_test *new_inventory = inventory_row(&context->new_directory, migration->new_key);
    if (strcmp(migration->old_pin, context->old_directory.pin) != 0 ||
        strcmp(migration->new_pin, context->new_directory.pin) != 0 ||
        strncmp(migration->old_result_symbol, "result_", 7u) != 0 ||
        strncmp(migration->old_routine_symbol, "TEST_", 5u) != 0 ||
        strncmp(migration->new_result_symbol, "result_", 7u) != 0 ||
        strncmp(migration->new_routine_symbol, "TEST_", 5u) != 0 ||
        strstr(migration->source_reference, "github.com/100thCoin/AccuracyCoin/blob/") == NULL ||
        strstr(migration->source_reference, migration->old_pin) == NULL ||
        strstr(migration->source_reference, migration->new_pin) == NULL ||
        migration->assertion_change[0] == '\0')
        return set_error(error, error_size, "migration disposition lacks exact pin-bound source facts");
    (void)snprintf(old_source, sizeof old_source, "blob/%s/AccuracyCoin.asm", migration->old_pin);
    (void)snprintf(new_source, sizeof new_source, "blob/%s/AccuracyCoin.asm", migration->new_pin);
    if (strstr(migration->source_reference, old_source) == NULL ||
        strstr(migration->source_reference, new_source) == NULL)
        return set_error(error, error_size, "migration source references do not name both exact source pins");
    if (strcmp(migration->disposition, "unchanged") == 0) {
        if (old_inventory == NULL || new_inventory == NULL ||
            strcmp(migration->old_key, migration->new_key) != 0)
            return set_error(error, error_size, "unchanged disposition does not map the same decoded key");
    } else if (strcmp(migration->disposition, "changed-meaning") == 0) {
        if (!is_reviewed_changed_key(migration->old_key) || old_inventory == NULL || new_inventory == NULL ||
            strcmp(migration->old_key, migration->new_key) != 0)
            return set_error(error, error_size, "changed-meaning disposition is not one of the four reviewed source rows");
    } else if (strcmp(migration->disposition, "renamed") == 0) {
        if (old_inventory == NULL || new_inventory == NULL ||
            strcmp(migration->old_key, migration->new_key) == 0)
            return set_error(error, error_size, "renamed disposition does not prove two present distinct keys");
    } else if (strcmp(migration->disposition, "removed") == 0) {
        if (old_inventory == NULL || migration->new_key[0] != '\0' || new_observation != NULL ||
            strstr(migration->assertion_change, "removed") == NULL)
            return set_error(error, error_size, "removed disposition lacks an explicit absent-row source fact");
        return 1;
    } else {
        return set_error(error, error_size, "migration disposition is unrecognized");
    }
    if (new_observation == NULL || new_observation->observed != 1u)
        return set_error(error, error_size, "migration has no actual new-pin observation");
    (void)snprintf(expected_reference, sizeof expected_reference,
                   "new/page-%02u/report.tsv#attempt=%s;row=%u;raw=%s;frames=%llu;status=%s",
                   new_observation->page, new_observation->attempt_id, new_observation->row,
                   new_observation->raw, (unsigned long long)new_observation->frames,
                   new_observation->status);
    if (strcmp(expected_reference, migration->new_observation_reference) != 0)
        return set_error(error, error_size, "migration new-observation link differs from fresh measured evidence");
    return 1;
}

static int verify_context(protected_context *context, const char *baseline_path,
                          const char *baseline_metadata_path, const char *candidate_scoreboard_path,
                          const char *candidate_rom_path, const char *candidate_manifest_path,
                          const char *migration_path, const char *evidence_root,
                          const char *expected_revision, char *error, size_t error_size)
{
    metadata baseline_meta;
    char digest[65];
    uint64_t value;
    const char *text;
    static const unsigned pages[] = {2u, 14u, 17u};

    if (!load_scoreboard(baseline_path, context->baseline, &context->baseline_count,
                         error, error_size))
        return 0;
    if (!load_scoreboard(candidate_scoreboard_path, context->candidate, &context->candidate_count,
                         error, error_size))
        return 0;
    if (!load_metadata(baseline_metadata_path, &baseline_meta, error, error_size))
        return 0;
    if (!valid_hex(expected_revision, 40u))
        return set_error(error, error_size, "protected authority revision is not an immutable commit id");
    (void)copy_text(context->protected_revision, sizeof context->protected_revision, expected_revision);
    text = metadata_value(&baseline_meta, "protected_revision");
    if (text == NULL || strcmp(text, context->protected_revision) != 0)
        return set_error(error, error_size, "protected metadata revision differs from fetched authority revision");
    text = metadata_value(&baseline_meta, "source");
    if (strcmp(metadata_or_empty(&baseline_meta, "schema"), "1") != 0 ||
        (strcmp(metadata_or_empty(&baseline_meta, "authority"), "protected-main") != 0 &&
         strcmp(metadata_or_empty(&baseline_meta, "authority"), "local-snapshot") != 0) ||
        text == NULL || text[0] == '\0')
        return set_error(error, error_size, "protected metadata authority identity is invalid");
    if (!hash_file(baseline_path, digest) ||
        strcmp(digest, metadata_or_empty(&baseline_meta, "scoreboard_sha256")) != 0)
        return set_error(error, error_size, "protected scoreboard bytes do not match same-revision metadata");
    if (!metadata_number(&baseline_meta, "scoreboard_schema", &value) || value != 1u ||
        !metadata_number(&baseline_meta, "scoreboard_rows", &value) || value != context->baseline_count ||
        !metadata_number(&baseline_meta, "scoreboard_denominator", &value) || value != context->baseline_count ||
        context->baseline_count != 17u)
        return set_error(error, error_size, "protected scoreboard denominator or schema is invalid");
    for (size_t i = 0u; i < context->baseline_count; i++)
        if (strcmp(context->baseline[i].status, "pass") != 0)
            return set_error(error, error_size, "protected baseline contains a non-pass row");

    if (!load_directory(NESTURBATOR_SOURCE_DIR "/tests/roms/accuracycoin-673ef.nes",
                        &context->old_directory, error, error_size) ||
        !load_directory(NESTURBATOR_SOURCE_DIR "/tests/roms/accuracycoin.nes",
                        &context->new_directory, error, error_size) ||
        !load_directory(candidate_rom_path, &context->candidate_directory, error, error_size))
        return 0;
    if (strcmp(context->old_directory.pin, "673ef550db296136d52229961e7d39366116882a") != 0 ||
        strcmp(context->new_directory.pin, "6bee919529289c555803b612b8c9e4f7eaaeeabb") != 0)
        return set_error(error, error_size, "candidate or comparison ROM pin identity is invalid");
    if (!load_manifest_identity(candidate_manifest_path, &context->candidate_directory,
                                error, error_size))
        return 0;
    (void)snprintf(context->baseline_pin, sizeof context->baseline_pin, "%s",
                   metadata_or_empty(&baseline_meta, "rom_pin"));
    (void)snprintf(context->candidate_pin, sizeof context->candidate_pin, "%s",
                   context->candidate_directory.pin);
    const accuracycoin_directory *baseline_directory =
        strcmp(context->baseline_pin, context->old_directory.pin) == 0 ? &context->old_directory :
        (strcmp(context->baseline_pin, context->new_directory.pin) == 0 ? &context->new_directory : NULL);
    if (baseline_directory == NULL ||
        strcmp(metadata_or_empty(&baseline_meta, "rom_sha256"), baseline_directory->rom_sha256) != 0 ||
        strcmp(metadata_or_empty(&baseline_meta, "inventory_sha256"), baseline_directory->inventory_sha256) != 0 ||
        !metadata_number(&baseline_meta, "inventory_schema", &value) || value != 1u ||
        !metadata_number(&baseline_meta, "inventory_rows", &value) || value != baseline_directory->row_count ||
        !metadata_number(&baseline_meta, "inventory_scored_rows", &value) || value != baseline_directory->scored_count ||
        !metadata_number(&baseline_meta, "inventory_draw_rows", &value) || value != baseline_directory->draw_count)
        return set_error(error, error_size, "protected metadata does not bind pin, inventory schema, and denominator");

    for (size_t pin_index = 0u; pin_index < 2u; pin_index++) {
        const char *pin_name = pin_index == 0u ? "old" : "new";
        const accuracycoin_directory *directory = pin_index == 0u ? &context->old_directory :
                                                                    &context->new_directory;
        page_observation *observations = pin_index == 0u ? context->old_pages : context->new_pages;
        for (size_t page_index = 0u; page_index < 3u; page_index++)
            if (!load_page_observation(evidence_root, pin_name, pages[page_index], directory,
                                       &observations[page_index], error, error_size))
                return 0;
    }
    if (!load_migrations(migration_path, context, error, error_size))
        return 0;

    int cross_pin = strcmp(context->candidate_pin, context->baseline_pin) != 0;
    if (cross_pin && (strcmp(context->baseline_pin, context->old_directory.pin) != 0 ||
                      strcmp(context->candidate_pin, context->new_directory.pin) != 0))
        return set_error(error, error_size, "candidate pin changed outside the reviewed old-to-new transition");
    if (!cross_pin && strcmp(context->candidate_pin, context->old_directory.pin) != 0 &&
        strcmp(context->candidate_pin, context->new_directory.pin) != 0)
        return set_error(error, error_size, "candidate pin has no protected measurement policy");

    for (size_t i = 0u; i < context->baseline_count; i++) {
        const scoreboard_row *baseline = &context->baseline[i];
        int baseline_is_new = strcmp(context->baseline_pin, context->new_directory.pin) == 0;
        const protected_migration *migration = migration_find(context, baseline->key, baseline_is_new);
        const observation_row *old_observation;
        const observation_row *new_observation;
        const observation_row *baseline_observation;
        const observation_row *candidate_observation;
        const scoreboard_row *candidate;
        const char *active_key;
        uint8_t actual_raw;
        uint8_t baseline_raw;
        accuracycoin_result_status classified;
        if (migration == NULL)
            return set_error(error, error_size, "protected key has no exact migration disposition");
        old_observation = observation_find(context->old_pages, migration->old_key);
        new_observation = observation_find(context->new_pages, migration->new_key);
        if (!validate_migration_row(context, migration, new_observation, error, error_size))
            return 0;
        if (old_observation == NULL || old_observation->observed != 1u ||
            strcmp(old_observation->status, "pass") != 0 || !parse_raw(old_observation->raw, &actual_raw)) {
            char message[160];
            const char *status = old_observation == NULL ? "missing" : old_observation->status;
            (void)snprintf(message, sizeof message,
                           "retained old protected key lacks an actual passing observation: %s", status);
            return set_error(error, error_size, message);
        }
        classified = accuracycoin_classify(migration->old_key, 1, actual_raw, 0, NULL);
        baseline_observation = baseline_is_new ? new_observation : old_observation;
        if (classified != ACCURACYCOIN_RESULT_PASS || baseline_observation == NULL ||
            strcmp(baseline_observation->status, "pass") != 0 ||
            !parse_raw(baseline_observation->raw, &baseline_raw) ||
            !parse_raw(baseline->raw, &actual_raw) || baseline_raw != actual_raw)
            return set_error(error, error_size, "old protected measurement differs from retained baseline pass");

        if (cross_pin) {
            if (strcmp(migration->disposition, "unchanged") == 0 &&
                strcmp(new_observation->status, "pass") != 0)
                return set_error(error, error_size, "unchanged cross-pin key lacks a measured passing observation");
            if (strcmp(migration->disposition, "changed-meaning") != 0 &&
                strcmp(migration->disposition, "unchanged") != 0)
                return set_error(error, error_size, "cross-pin key requires an ordinary reviewed mapping");
            active_key = migration->new_key;
            candidate_observation = new_observation;
        } else if (strcmp(context->candidate_pin, context->old_directory.pin) == 0) {
            active_key = migration->old_key;
            candidate_observation = old_observation;
        } else {
            active_key = migration->new_key;
            candidate_observation = new_observation;
        }
        if (candidate_observation == NULL || candidate_observation->observed != 1u ||
            !parse_raw(candidate_observation->raw, &actual_raw))
            return set_error(error, error_size, "candidate protected key has no actual raw observation");
        if (!cross_pin && strcmp(candidate_observation->status, "pass") != 0)
            return set_error(error, error_size, "same-pin protected pass is not measured passing now");
        candidate = scoreboard_find(context->candidate, context->candidate_count, active_key);
        if (candidate == NULL)
            return set_error(error, error_size, "candidate scoreboard lost a protected key");
        if (strcmp(candidate->status, candidate_observation->status) != 0 ||
            strcmp(candidate->raw, candidate_observation->raw) != 0)
            return set_error(error, error_size, "candidate scoreboard claim differs from measured result");
        if (cross_pin && strcmp(migration->disposition, "changed-meaning") == 0 &&
            strcmp(candidate_observation->status, "pass") != 0 &&
            strcmp(candidate_observation->status, "fail") != 0)
            return set_error(error, error_size, "changed-meaning disposition only permits a measured terminal failure");
        classified = accuracycoin_classify(active_key, 1, actual_raw, 0, NULL);
        if (classified == ACCURACYCOIN_RESULT_INVALID ||
            strcmp(candidate_observation->status, accuracycoin_status_text(classified)) != 0)
            return set_error(error, error_size, "candidate protected status is not valid for its raw byte");
    }
    return 1;
}

static int verify_files(const char *baseline_path, const char *baseline_metadata_path,
                        const char *revision, const char *candidate_scoreboard_path,
                        const char *candidate_rom_path, const char *candidate_manifest_path,
                        const char *migration_path, const char *evidence_root)
{
    protected_context context;
    char error[256] = "protected verdict failed";
    memset(&context, 0, sizeof context);
    if (!verify_context(&context, baseline_path, baseline_metadata_path,
                        candidate_scoreboard_path, candidate_rom_path, candidate_manifest_path,
                        migration_path, evidence_root, revision, error, sizeof error)) {
        (void)fprintf(stderr, "protected-verdict: %s\n", error);
        return 1;
    }
    (void)printf("protected AccuracyCoin verdict PASS: %lu old passes, %lu new observations, candidate pin %s\n",
                 (unsigned long)context.baseline_count, (unsigned long)context.migration_count,
                 context.candidate_pin);
    return 0;
}

static int ci_enabled(void)
{
    const char *ci = getenv("CI");
    return ci != NULL && ci[0] != '\0' && strcmp(ci, "0") != 0 && strcmp(ci, "false") != 0;
}

static int main_verify(void)
{
    const char *baseline = getenv("NESTURBATOR_SCOREBOARD_BASELINE");
    const char *metadata_path = getenv("NESTURBATOR_SCOREBOARD_BASELINE_METADATA");
    const char *revision = getenv("NESTURBATOR_SCOREBOARD_BASELINE_REVISION");
    char revision_from_file[41];
    metadata local_meta;
    char error[256];
    if (ci_enabled()) {
        if (baseline == NULL || baseline[0] == '\0' || metadata_path == NULL ||
            metadata_path[0] == '\0' || revision == NULL || revision[0] == '\0') {
            (void)fprintf(stderr, "protected-verdict: CI requires protected scoreboard, metadata, and revision handoff\n");
            return 1;
        }
    } else {
        baseline = NESTURBATOR_SOURCE_DIR "/tests/accuracy/scoreboard-main.txt";
        metadata_path = NESTURBATOR_SOURCE_DIR "/tests/accuracy/scoreboard-main-metadata.txt";
        if (!load_metadata(metadata_path, &local_meta, error, sizeof error) ||
            !copy_text(revision_from_file, sizeof revision_from_file,
                       metadata_or_empty(&local_meta, "protected_revision"))) {
            (void)fprintf(stderr, "protected-verdict: local fallback metadata is invalid\n");
            return 1;
        }
        revision = revision_from_file;
    }
    return verify_files(baseline, metadata_path, revision,
                        NESTURBATOR_SOURCE_DIR "/tests/accuracy/scoreboard.txt",
                        NESTURBATOR_SOURCE_DIR "/tests/roms/accuracycoin.nes",
                        NESTURBATOR_SOURCE_DIR "/tests/roms/manifest.txt",
                        NESTURBATOR_SOURCE_DIR "/tests/accuracy/migration.tsv",
                        NESTURBATOR_PROTECTED_EVIDENCE_DIR);
}

static int scoreboard_syntax(void)
{
    scoreboard_row rows[MAX_SCOREBOARD_ROWS];
    size_t count = 0u;
    char error[256];
    if (!load_scoreboard(NESTURBATOR_SOURCE_DIR "/tests/accuracy/scoreboard.txt",
                         rows, &count, error, sizeof error)) {
        (void)fprintf(stderr, "scoreboard: %s\n", error);
        return 1;
    }
    (void)printf("scoreboard syntax PASS (%lu rows)\n", (unsigned long)count);
    return 0;
}

static int no_lost_pass(const char *baseline_path)
{
    scoreboard_row current[MAX_SCOREBOARD_ROWS];
    scoreboard_row baseline[MAX_SCOREBOARD_ROWS];
    size_t current_count = 0u;
    size_t baseline_count = 0u;
    char error[256];
    if (!load_scoreboard(NESTURBATOR_SOURCE_DIR "/tests/accuracy/scoreboard.txt",
                         current, &current_count, error, sizeof error) ||
        !load_scoreboard(baseline_path, baseline, &baseline_count, error, sizeof error)) {
        (void)fprintf(stderr, "scoreboard: %s\n", error);
        return 1;
    }
    for (size_t i = 0u; i < baseline_count; i++) {
        if (strcmp(baseline[i].status, "pass") == 0 &&
            scoreboard_find(current, current_count, baseline[i].key) == NULL) {
            (void)fprintf(stderr, "scoreboard lost protected PASS: %s\n", baseline[i].key);
            return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 1)
        return main_verify();
    if (argc == 2 && strcmp(argv[1], "--scoreboard-syntax") == 0)
        return scoreboard_syntax();
    if (argc == 2)
        return no_lost_pass(argv[1]);
    if (argc == 10 && strcmp(argv[1], "--verify") == 0)
        return verify_files(argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9]);
    (void)fprintf(stderr,
                  "usage: accuracy.scoreboard [--scoreboard-syntax | --verify BASELINE METADATA REVISION SCOREBOARD ROM MANIFEST MIGRATION EVIDENCE_ROOT]\n");
    return 2;
}
