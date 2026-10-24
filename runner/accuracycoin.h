/* AccuracyCoin directory, result protocol and observation runner. */
#ifndef NESTURBATOR_RUNNER_ACCURACYCOIN_H
#define NESTURBATOR_RUNNER_ACCURACYCOIN_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "audio_hash.h"
#include "nesturbator.h"

#define ACCURACYCOIN_PAGE_COUNT 22u
#define ACCURACYCOIN_ROW_COUNT 151u
#define ACCURACYCOIN_MAX_PAGE_ROWS 32u
#define ACCURACYCOIN_NAME_CAPACITY 128u

typedef enum accuracycoin_pin {
    ACCURACYCOIN_PIN_OLD = 0,
    ACCURACYCOIN_PIN_NEW = 1
} accuracycoin_pin;

typedef enum accuracycoin_reason {
    ACCURACYCOIN_REASON_OK = 0,
    ACCURACYCOIN_REASON_IMAGE_SIZE,
    ACCURACYCOIN_REASON_IMAGE_HEADER,
    ACCURACYCOIN_REASON_PIN,
    ACCURACYCOIN_REASON_PAGE_TABLE,
    ACCURACYCOIN_REASON_PAGE_POINTER,
    ACCURACYCOIN_REASON_PAGE_TITLE,
    ACCURACYCOIN_REASON_EMPTY_PAGE,
    ACCURACYCOIN_REASON_ROW_FIELD,
    ACCURACYCOIN_REASON_NAME_ENCODING,
    ACCURACYCOIN_REASON_NAME_SIZE,
    ACCURACYCOIN_REASON_SCORE_ADDRESS,
    ACCURACYCOIN_REASON_ROUTINE_ADDRESS,
    ACCURACYCOIN_REASON_DUPLICATE_KEY,
    ACCURACYCOIN_REASON_DUPLICATE_ADDRESS,
    ACCURACYCOIN_REASON_DRAW_COUNT,
    ACCURACYCOIN_REASON_INVENTORY_COUNT,
    ACCURACYCOIN_REASON_INVENTORY_FINGERPRINT
} accuracycoin_reason;

typedef enum accuracycoin_kind {
    ACCURACYCOIN_KIND_TEST = 0,
    ACCURACYCOIN_KIND_DRAW = 1
} accuracycoin_kind;

typedef struct accuracycoin_test {
    char name[ACCURACYCOIN_NAME_CAPACITY];
    uint16_t result_address;
    uint16_t routine_address;
    uint8_t page;
    uint8_t row;
    uint8_t kind;
} accuracycoin_test;

typedef struct accuracycoin_page {
    char title[ACCURACYCOIN_NAME_CAPACITY];
    uint16_t first_row;
    uint16_t row_count;
} accuracycoin_page;

typedef struct accuracycoin_directory {
    accuracycoin_page pages[ACCURACYCOIN_PAGE_COUNT];
    accuracycoin_test rows[ACCURACYCOIN_ROW_COUNT];
    char pin[41];
    char rom_sha256[65];
    char inventory_sha256[65];
    uint16_t scored_count;
    uint16_t draw_count;
    uint16_t row_count;
    uint8_t pin_id;
} accuracycoin_directory;

typedef struct accuracycoin_run_budgets {
    uint32_t boot_frames;
    uint32_t navigation_frames_per_page;
    uint32_t execution_frames;
    uint64_t total_frames;
} accuracycoin_run_budgets;

typedef enum accuracycoin_result_status {
    ACCURACYCOIN_RESULT_PASS = 0,
    ACCURACYCOIN_RESULT_FAIL,
    ACCURACYCOIN_RESULT_SKIP,
    ACCURACYCOIN_RESULT_NOT_RUN,
    ACCURACYCOIN_RESULT_TIMEOUT,
    ACCURACYCOIN_RESULT_DRAW,
    ACCURACYCOIN_RESULT_INVALID
} accuracycoin_result_status;

/* Parse a complete inventory against the selected immutable inventory
 * fingerprint. The image hash is checked separately by parse_pinned(). */
int accuracycoin_parse_for_pin(const uint8_t *bytes, size_t size, accuracycoin_pin pin,
                               accuracycoin_directory *out, accuracycoin_reason *reason);

/* Authenticate exact selected bytes by full SHA-256, then parse those same
 * bytes to populate out. Only the retained old and approved new pins pass. */
int accuracycoin_parse_pinned(const uint8_t *bytes, size_t size, accuracycoin_directory *out,
                              accuracycoin_reason *reason);

const char *accuracycoin_reason_text(accuracycoin_reason reason);
const char *accuracycoin_pin_text(accuracycoin_pin pin);
int accuracycoin_write_inventory(FILE *out, const accuracycoin_directory *directory);

/* Result protocol: $FF is skip, $00 is not-run, $01 is pass, failures encode
 * an error index with low bits 10 (including source-defined $02), and $03 is
 * active until an observed guest-frame budget expires. Pinned source-defined
 * pass variants are accepted only for their named tests. */
accuracycoin_result_status accuracycoin_classify(const char *key, int observed, uint8_t raw,
                                                 int active_budget_expired, const char **cause);
/* DRAW rows carry the shared sentinel as inventory data; return DRAW without
 * interpreting its arbitrary byte as a scored status. */
accuracycoin_result_status accuracycoin_classify_row(const accuracycoin_test *row, int observed,
                                                     uint8_t raw, int active_budget_expired,
                                                     const char **cause);
const char *accuracycoin_status_text(accuracycoin_result_status status);

/* Driver start evidence uses result transitions for scored pages and the
 * source-defined row-cursor transition for DRAW-only pages. */
const char *accuracycoin_validate_start(uint32_t requested_page, uint8_t menu_ready,
                                        uint8_t selected_page, uint8_t cursor, int a_press_observed,
                                        int a_release_observed, int run_state_observed,
                                        int result_transition_observed, int draw_only_page,
                                        int draw_cursor_transition_observed);

int accuracycoin_run_strict_page(const accuracycoin_directory *directory, uint32_t page,
                                 const char *scoreboard_path, nesturbator *inst, uint16_t *video,
                                 int16_t *audio, nesturbator_run_audio_hash *audio_hash);

/* Run one cold-selected page and retain inventory-bound raw samples, cursor/start
 * diagnostics, a classified report and metadata. The host monotonic start stamp
 * distinguishes this invocation's evidence from a previously copied report. */
int accuracycoin_run_observe_page(const accuracycoin_directory *directory, uint32_t page,
                                  const accuracycoin_run_budgets *budgets, const char *output_dir,
                                  nesturbator *inst, uint16_t *video, int16_t *audio);

/* Collect one bounded campaign under output_dir. Page child outcomes are produced by the
 * CMake launcher; only validated per-page evidence is classified as complete observation. */
int accuracycoin_collect_campaign(const accuracycoin_directory *directory, const char *output_dir);

#endif /* NESTURBATOR_RUNNER_ACCURACYCOIN_H */
