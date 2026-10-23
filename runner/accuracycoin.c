/* AccuracyCoin's directory and result protocol are data in the selected ROM,
 * not a second list of test names or result addresses. Source: the MIT test
 * assembly at the retained pins in tests/roms/manifest.txt. */
#include "accuracycoin.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clock.h"
#include "exit_status.h"
#include "sha256.h"

#define WIDTH 256u
#define HEIGHT 240u
#define AUDIO_CAPACITY 1024u
#define ACCURACYCOIN_PAGE_TABLE_OFFSET 0x110u
#define ACCURACYCOIN_PRG_END (16u + 32768u)
#define ACCURACYCOIN_ROM_SIZE (16u + 32768u + 8192u)
#define ACCURACYCOIN_RESULT_FIRST 0x0400u
#define ACCURACYCOIN_RESULT_LAST 0x04ffu
#define ACCURACYCOIN_DRAW_RESULT 0x03ffu
#define ACCURACYCOIN_MENU_READY_ADDRESS 0x00ecu
#define ACCURACYCOIN_PAGE_INDEX_ADDRESS 0x0014u
#define ACCURACYCOIN_CURSOR_ADDRESS 0x0016u
#define ACCURACYCOIN_AUTOMATE_ADDRESS 0x0034u
#define ACCURACYCOIN_RUNNING_ADDRESS 0x0035u
#define ACCURACYCOIN_PPU_RESET_RESULT_ADDRESS 0x0360u

#ifndef NESTURBATOR_SOURCE_REVISION
#define NESTURBATOR_SOURCE_REVISION "unknown"
#endif
#ifndef NESTURBATOR_SOURCE_DIRTY
#define NESTURBATOR_SOURCE_DIRTY "unknown"
#endif

static const char new_pin[] = "6bee919529289c555803b612b8c9e4f7eaaeeabb";
static const char old_pin[] = "673ef550db296136d52229961e7d39366116882a";
static const char new_rom_sha[] = "8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc";
static const char old_rom_sha[] = "4fe8c2bc9abc6f4d418da47b73f62cba89fcacd950fae763097a1681d650e839";

/* SHA-256 over each page/title record followed by that page's source-order rows. Page records
 * are `P<TAB>page<TAB>title<LF>`; rows are
 * `R<TAB>page<TAB>row<TAB>kind<TAB>accuracycoin/name<TAB>0xresult<TAB>0xroutine<LF>`.
 * These fingerprints were derived from the independently verified pinned ROMs. */
static const char new_inventory_sha[] = "7d022800d01ad81159109bf7457af94ab6f80e4a682fce716ec476f7c454a4a5";
static const char old_inventory_sha[] = "3271803cdc63c4b4a83cf65453b817dc66ec65ed97863ad19261b56f36821c84";

static uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | (uint16_t)((uint16_t)bytes[1] << 8));
}

static void hash_bytes(const uint8_t *bytes, size_t size, char hex[65])
{
    nesturbator_run_sha256 state;
    uint8_t digest[32];
    nesturbator_run_sha256_init(&state);
    nesturbator_run_sha256_update(&state, bytes, size);
    nesturbator_run_sha256_final(&state, digest);
    nesturbator_run_sha256_hex(digest, hex);
}

static int rom_offset(uint16_t address, size_t *offset)
{
    size_t result;
    if (address < 0x8000u)
        return 0;
    result = 16u + (size_t)(address - 0x8000u);
    if (result >= ACCURACYCOIN_PRG_END)
        return 0;
    *offset = result;
    return 1;
}

static int decode_name(const uint8_t *bytes, size_t limit, size_t *at, char *out,
                       size_t out_capacity, accuracycoin_reason *reason)
{
    static const char *const expansions[] = {" indirect", " zeropage", " absolute",
                                              " immediate", "Unofficial Instructions: "};
    size_t used = 0u;
    size_t cursor = *at;
    while (cursor < limit && bytes[cursor] != 0xffu) {
        const char *expansion = NULL;
        size_t append_length = 1u;
        uint8_t byte = bytes[cursor++];
        if (byte >= 0xf0u && byte <= 0xf4u) {
            expansion = expansions[byte - 0xf0u];
            append_length = strlen(expansion);
        } else if (byte < 0x20u || byte > 0x7eu) {
            *reason = ACCURACYCOIN_REASON_NAME_ENCODING;
            return 0;
        }
        if (append_length >= out_capacity || used > out_capacity - append_length - 1u) {
            *reason = ACCURACYCOIN_REASON_NAME_SIZE;
            return 0;
        }
        if (expansion != NULL) {
            memcpy(out + used, expansion, append_length);
            used += append_length;
        } else {
            out[used++] = (char)byte;
        }
    }
    if (cursor >= limit) {
        *reason = ACCURACYCOIN_REASON_ROW_FIELD;
        return 0;
    }
    if (used == 0u) {
        *reason = ACCURACYCOIN_REASON_NAME_SIZE;
        return 0;
    }
    out[used] = '\0';
    *at = cursor + 1u;
    return 1;
}

static void inventory_hash_add(nesturbator_run_sha256 *hash, const char *format,
                               unsigned page, unsigned row, const char *kind,
                               const char *title_or_name, unsigned result, unsigned routine)
{
    char line[ACCURACYCOIN_NAME_CAPACITY + 96u];
    int written;
    if (row == 0u) {
        written = snprintf(line, sizeof line, "P\t%u\t%s\n", page, title_or_name);
    } else {
        written = snprintf(line, sizeof line, format, page, row, kind, title_or_name, result,
                           routine);
    }
    if (written > 0 && (size_t)written < sizeof line)
        nesturbator_run_sha256_update(hash, line, (size_t)written);
}

const char *accuracycoin_reason_text(accuracycoin_reason reason)
{
    static const char *const text[] = {"ok",
                                       "wrong image size",
                                       "invalid iNES header or geometry",
                                       "ROM bytes do not match a retained pin",
                                       "invalid page directory table",
                                       "page pointer is invalid or out of order",
                                       "empty or malformed page title",
                                       "page contains no rows",
                                       "truncated row fields",
                                       "unsupported encoded name byte",
                                       "empty or over-capacity decoded name",
                                       "result address is outside AccuracyCoin result RAM",
                                       "test routine pointer is outside PRG ROM",
                                       "duplicate decoded scored key",
                                       "duplicate scored result address",
                                       "DRAW sentinel count is not five",
                                       "inventory does not have the expected counts",
                                       "inventory fingerprint differs from the pin"};
    size_t index = (size_t)reason;
    if (index >= sizeof text / sizeof text[0])
        return "unknown inventory error";
    return text[index];
}

const char *accuracycoin_pin_text(accuracycoin_pin pin)
{
    return pin == ACCURACYCOIN_PIN_NEW ? new_pin : old_pin;
}

static int expected_counts(accuracycoin_pin pin, unsigned *scored, unsigned *draw,
                           const char **fingerprint)
{
    if (pin == ACCURACYCOIN_PIN_NEW) {
        *scored = 146u;
        *draw = 5u;
        *fingerprint = new_inventory_sha;
        return 1;
    }
    if (pin == ACCURACYCOIN_PIN_OLD) {
        *scored = 144u;
        *draw = 5u;
        *fingerprint = old_inventory_sha;
        return 1;
    }
    return 0;
}

int accuracycoin_parse_for_pin(const uint8_t *bytes, size_t size, accuracycoin_pin pin,
                               accuracycoin_directory *out, accuracycoin_reason *reason)
{
    static const char row_format[] = "R\t%u\t%u\t%s\taccuracycoin/%s\t0x%04x\t0x%04x\n";
    accuracycoin_directory result;
    size_t page_offsets[ACCURACYCOIN_PAGE_COUNT];
    unsigned expected_scored;
    unsigned expected_draw;
    const char *expected_fingerprint;
    unsigned draw_count = 0u;
    unsigned scored_count = 0u;
    unsigned total_rows = 0u;
    nesturbator_run_sha256 inventory_hash;
    uint8_t digest[32];
    accuracycoin_reason local_reason;
    if (reason == NULL)
        reason = &local_reason;
    *reason = ACCURACYCOIN_REASON_IMAGE_SIZE;
    if (bytes == NULL || out == NULL || size != ACCURACYCOIN_ROM_SIZE)
        return 0;
    *reason = ACCURACYCOIN_REASON_IMAGE_HEADER;
    if (memcmp(bytes, "NES\x1a", 4u) != 0 || bytes[4] != 2u || bytes[5] != 1u ||
        (bytes[6] & 0x04u) != 0u || (bytes[7] & 0x0cu) != 0u || bytes[12] != 0u ||
        bytes[13] != 0u || bytes[14] != 0u || bytes[15] != 0u)
        return 0;
    *reason = ACCURACYCOIN_REASON_PAGE_TABLE;
    if (!expected_counts(pin, &expected_scored, &expected_draw, &expected_fingerprint) ||
        ACCURACYCOIN_PAGE_TABLE_OFFSET + ACCURACYCOIN_PAGE_COUNT * 2u >= ACCURACYCOIN_PRG_END ||
        bytes[ACCURACYCOIN_PAGE_TABLE_OFFSET] == 0xffu)
        return 0;
    memset(&result, 0, sizeof result);
    result.pin_id = (uint8_t)pin;
    (void)snprintf(result.pin, sizeof result.pin, "%s", accuracycoin_pin_text(pin));
    hash_bytes(bytes, size, result.rom_sha256);
    nesturbator_run_sha256_init(&inventory_hash);
    for (unsigned i = 0u; i < ACCURACYCOIN_PAGE_COUNT; i++) {
        uint16_t address = read_le16(bytes + ACCURACYCOIN_PAGE_TABLE_OFFSET + (size_t)i * 2u);
        size_t offset;
        if (!rom_offset(address, &offset) ||
            (i == 0u && offset != ACCURACYCOIN_PAGE_TABLE_OFFSET +
                                       ACCURACYCOIN_PAGE_COUNT * 2u) ||
            (i > 0u && offset <= page_offsets[i - 1u])) {
            *reason = ACCURACYCOIN_REASON_PAGE_POINTER;
            return 0;
        }
        page_offsets[i] = offset;
    }
    for (unsigned page_index = 0u; page_index < ACCURACYCOIN_PAGE_COUNT; page_index++) {
        size_t limit = page_index + 1u < ACCURACYCOIN_PAGE_COUNT
                           ? page_offsets[page_index + 1u]
                           : ACCURACYCOIN_PRG_END;
        size_t cursor = page_offsets[page_index];
        unsigned page_number = page_index + 1u;
        accuracycoin_page *page = &result.pages[page_index];
        if (reason != NULL)
            *reason = ACCURACYCOIN_REASON_PAGE_TITLE;
        if (!decode_name(bytes, limit, &cursor, page->title, sizeof page->title, reason))
            return 0;
        page->first_row = (uint16_t)total_rows;
        inventory_hash_add(&inventory_hash, NULL, page_number, 0u, NULL, page->title, 0u, 0u);
        while (cursor < limit && bytes[cursor] != 0xffu) {
            accuracycoin_test *row;
            uint16_t result_address;
            uint16_t routine_address;
            size_t routine_offset;
            unsigned row_number = (unsigned)page->row_count + 1u;
            if (page->row_count >= ACCURACYCOIN_MAX_PAGE_ROWS ||
                total_rows >= ACCURACYCOIN_ROW_COUNT) {
                *reason = ACCURACYCOIN_REASON_INVENTORY_COUNT;
                return 0;
            }
            row = &result.rows[total_rows];
            *reason = ACCURACYCOIN_REASON_NAME_SIZE;
            if (!decode_name(bytes, limit, &cursor, row->name, sizeof row->name, reason))
                return 0;
            *reason = ACCURACYCOIN_REASON_ROW_FIELD;
            if (limit - cursor < 4u)
                return 0;
            result_address = read_le16(bytes + cursor);
            routine_address = read_le16(bytes + cursor + 2u);
            cursor += 4u;
            if (result_address == ACCURACYCOIN_DRAW_RESULT) {
                row->kind = (uint8_t)ACCURACYCOIN_KIND_DRAW;
                draw_count++;
                if (page_number != 16u) {
                    *reason = ACCURACYCOIN_REASON_SCORE_ADDRESS;
                    return 0;
                }
            } else if (result_address >= ACCURACYCOIN_RESULT_FIRST &&
                       result_address <= ACCURACYCOIN_RESULT_LAST) {
                row->kind = (uint8_t)ACCURACYCOIN_KIND_TEST;
                scored_count++;
                for (unsigned previous = 0u; previous < total_rows; previous++) {
                    const accuracycoin_test *old = &result.rows[previous];
                    if (old->kind == (uint8_t)ACCURACYCOIN_KIND_TEST &&
                        old->result_address == result_address) {
                        *reason = ACCURACYCOIN_REASON_DUPLICATE_ADDRESS;
                        return 0;
                    }
                    if (old->kind == (uint8_t)ACCURACYCOIN_KIND_TEST &&
                        strcmp(old->name, row->name) == 0) {
                        *reason = ACCURACYCOIN_REASON_DUPLICATE_KEY;
                        return 0;
                    }
                }
            } else {
                *reason = ACCURACYCOIN_REASON_SCORE_ADDRESS;
                return 0;
            }
            if (!rom_offset(routine_address, &routine_offset)) {
                *reason = ACCURACYCOIN_REASON_ROUTINE_ADDRESS;
                return 0;
            }
            (void)routine_offset;
            row->result_address = result_address;
            row->routine_address = routine_address;
            row->page = (uint8_t)page_number;
            row->row = (uint8_t)row_number;
            inventory_hash_add(&inventory_hash, row_format, page_number, row_number,
                               row->kind == (uint8_t)ACCURACYCOIN_KIND_DRAW ? "DRAW" : "TEST",
                               row->name, (unsigned)result_address, (unsigned)routine_address);
            page->row_count++;
            total_rows++;
        }
        if (cursor >= limit || page->row_count == 0u) {
            *reason = ACCURACYCOIN_REASON_EMPTY_PAGE;
            return 0;
        }
        cursor++;
    }
    if (draw_count != expected_draw) {
        *reason = ACCURACYCOIN_REASON_DRAW_COUNT;
        return 0;
    }
    if (scored_count != expected_scored || total_rows != expected_scored + expected_draw) {
        *reason = ACCURACYCOIN_REASON_INVENTORY_COUNT;
        return 0;
    }
    nesturbator_run_sha256_final(&inventory_hash, digest);
    nesturbator_run_sha256_hex(digest, result.inventory_sha256);
    if (strcmp(result.inventory_sha256, expected_fingerprint) != 0) {
        *reason = ACCURACYCOIN_REASON_INVENTORY_FINGERPRINT;
        return 0;
    }
    result.scored_count = (uint16_t)scored_count;
    result.draw_count = (uint16_t)draw_count;
    result.row_count = (uint16_t)total_rows;
    *out = result;
    *reason = ACCURACYCOIN_REASON_OK;
    return 1;
}

int accuracycoin_parse_pinned(const uint8_t *bytes, size_t size, accuracycoin_directory *out,
                              accuracycoin_reason *reason)
{
    char rom_sha[65];
    accuracycoin_pin pin;
    if (reason != NULL)
        *reason = ACCURACYCOIN_REASON_IMAGE_SIZE;
    if (bytes == NULL || size != ACCURACYCOIN_ROM_SIZE)
        return 0;
    hash_bytes(bytes, size, rom_sha);
    if (strcmp(rom_sha, new_rom_sha) == 0)
        pin = ACCURACYCOIN_PIN_NEW;
    else if (strcmp(rom_sha, old_rom_sha) == 0)
        pin = ACCURACYCOIN_PIN_OLD;
    else {
        if (reason != NULL)
            *reason = ACCURACYCOIN_REASON_PIN;
        return 0;
    }
    if (!accuracycoin_parse_for_pin(bytes, size, pin, out, reason))
        return 0;
    if (strcmp(out->rom_sha256, rom_sha) != 0) {
        if (reason != NULL)
            *reason = ACCURACYCOIN_REASON_PIN;
        return 0;
    }
    return 1;
}

int accuracycoin_write_inventory(FILE *out, const accuracycoin_directory *directory)
{
    if (out == NULL || directory == NULL ||
        fprintf(out, "page\trow\tkind\tkey\tresult_address\troutine_address\n") < 0)
        return 0;
    for (unsigned i = 0u; i < directory->row_count; i++) {
        const accuracycoin_test *row = &directory->rows[i];
        if (fprintf(out, "%u\t%u\t%s\taccuracycoin/%s\t0x%04x\t0x%04x\n",
                    (unsigned)row->page, (unsigned)row->row,
                    row->kind == (uint8_t)ACCURACYCOIN_KIND_DRAW ? "DRAW" : "TEST", row->name,
                    (unsigned)row->result_address, (unsigned)row->routine_address) < 0)
            return 0;
    }
    return fflush(out) == 0;
}

const char *accuracycoin_status_text(accuracycoin_result_status status)
{
    static const char *const text[] = {"pass", "fail", "skip", "not-run", "timeout", "invalid"};
    size_t index = (size_t)status;
    if (index >= sizeof text / sizeof text[0])
        return "invalid";
    return text[index];
}

accuracycoin_result_status accuracycoin_classify(const char *key, int observed, uint8_t raw,
                                                  int active_budget_expired,
                                                  const char **cause)
{
    if (cause != NULL)
        *cause = "invalid-result";
    if (observed == 0) {
        if (cause != NULL)
            *cause = "missing-raw";
        return ACCURACYCOIN_RESULT_INVALID;
    }
    if (raw == 0xffu) {
        if (cause != NULL)
            *cause = "upstream-skip";
        return ACCURACYCOIN_RESULT_SKIP;
    }
    if (raw == 0u) {
        if (cause != NULL)
            *cause = "unstarted";
        return ACCURACYCOIN_RESULT_NOT_RUN;
    }
    if ((raw & 3u) == 3u) {
        if (raw != 0x03u) {
            if (cause != NULL)
                *cause = "unsupported-running-code";
            return ACCURACYCOIN_RESULT_INVALID;
        }
        if (active_budget_expired != 0) {
            if (cause != NULL)
                *cause = "active-test-budget-exhausted";
            return ACCURACYCOIN_RESULT_TIMEOUT;
        }
        if (cause != NULL)
            *cause = "active-test";
        return ACCURACYCOIN_RESULT_INVALID;
    }
    if ((raw & 3u) == 1u) {
        if (raw == 0x01u ||
            (raw == 0x41u && key != NULL && strcmp(key, "accuracycoin/PPU Read Buffer") == 0)) {
            if (cause != NULL)
                *cause = raw == 0x41u ? "source-defined-pass-variant" : "terminal-pass";
            return ACCURACYCOIN_RESULT_PASS;
        }
        if (cause != NULL)
            *cause = "unsupported-pass-variant";
        return ACCURACYCOIN_RESULT_INVALID;
    }
    if ((raw & 3u) == 2u) {
        if (cause != NULL)
            *cause = "terminal-fail";
        return ACCURACYCOIN_RESULT_FAIL;
    }
    return ACCURACYCOIN_RESULT_INVALID;
}

static int accuracy_ram(const nesturbator *inst, uint16_t address, uint8_t *out)
{
    return nesturbator_peek_cpu_ram(inst, address, out) == NESTURBATOR_OK;
}

static nesturbator_status accuracy_frame(nesturbator *inst, uint16_t buttons, uint16_t *video,
                                         int16_t *audio,
                                         nesturbator_run_audio_hash *audio_hash)
{
    nesturbator_input input;
    nesturbator_frame frame;
    memset(&input, 0, sizeof input);
    input.size = (uint32_t)sizeof input;
    input.buttons[0] = (uint8_t)buttons;
    if (nesturbator_set_input(inst, &input) != NESTURBATOR_OK)
        return NESTURBATOR_ERR_ARGUMENT;
    memset(&frame, 0, sizeof frame);
    frame.size = (uint32_t)sizeof frame;
    frame.video = video;
    frame.video_pitch = WIDTH;
    frame.audio = audio;
    frame.audio_capacity = AUDIO_CAPACITY;
    nesturbator_status status = nesturbator_run_frame(inst, &frame);
    if (status == NESTURBATOR_OK && audio_hash != NULL)
        nesturbator_run_audio_hash_pcm(audio_hash, audio, frame.audio_count);
    return status;
}

static int compare_test_name(const void *left, const void *right)
{
    const accuracycoin_test *a = (const accuracycoin_test *)left;
    const accuracycoin_test *b = (const accuracycoin_test *)right;
    return strcmp(a->name, b->name);
}

static int required_page14_test(const char *name)
{
    static const char *const required[] = {"Length Counter", "Length Table", "Frame Counter IRQ",
                                            "Frame Counter 4-step", "Frame Counter 5-step",
                                            "Delta Modulation Channel"};
    for (size_t i = 0u; i < sizeof required / sizeof required[0]; i++)
        if (strcmp(name, required[i]) == 0)
            return 1;
    return 0;
}

static int compare_scoreboard(const char *path, const accuracycoin_test *tests,
                              const uint8_t *raw, size_t count)
{
    FILE *file = fopen(path, "r");
    char line[256];
    size_t found = 0u;
    size_t last_index = 0u;
    int have_last = 0;
    if (file == NULL || count > sizeof(size_t) * 8u) {
        if (file != NULL)
            fclose(file);
        return 0;
    }
    while (fgets(line, sizeof line, file) != NULL) {
        if (strchr(line, '\n') == NULL && !feof(file)) {
            fclose(file);
            return 0;
        }
        for (size_t i = 0u; i < count; i++) {
            char key[160];
            char expected[192];
            char full_key[ACCURACYCOIN_NAME_CAPACITY + 16u];
            const char *cause = NULL;
            (void)snprintf(full_key, sizeof full_key, "accuracycoin/%s", tests[i].name);
            accuracycoin_result_status status =
                accuracycoin_classify(full_key, 1, raw[i], 0, &cause);
            (void)cause;
            (void)snprintf(key, sizeof key, "accuracycoin/%s\t", tests[i].name);
            if (strncmp(line, key, strlen(key)) == 0) {
                (void)snprintf(expected, sizeof expected,
                               "accuracycoin/%s\t%s\t0x%02x\t-\t-\n", tests[i].name,
                               accuracycoin_status_text(status), (unsigned)raw[i]);
                if (strcmp(line, expected) != 0 || (found & ((size_t)1u << i)) != 0u) {
                    fclose(file);
                    return 0;
                }
                if (have_last != 0 && i <= last_index) {
                    fclose(file);
                    return 0;
                }
                found |= (size_t)1u << i;
                last_index = i;
                have_last = 1;
            }
        }
    }
    fclose(file);
    return found == (((size_t)1u << count) - 1u);
}

int accuracycoin_run_strict_page(const accuracycoin_directory *directory, uint32_t page_number,
                                 const char *scoreboard_path, nesturbator *inst, uint16_t *video,
                                 int16_t *audio, nesturbator_run_audio_hash *audio_hash)
{
    accuracycoin_test tests[ACCURACYCOIN_MAX_PAGE_ROWS];
    uint8_t results[ACCURACYCOIN_MAX_PAGE_ROWS];
    const accuracycoin_page *page;
    size_t count;
    uint32_t frames = 0u;
    uint8_t value = 0u;
    int complete = 0;
    if (directory == NULL || page_number == 0u || page_number > ACCURACYCOIN_PAGE_COUNT ||
        scoreboard_path == NULL || inst == NULL || video == NULL || audio == NULL)
        return RUNNER_EXIT_FAILURE;
    page = &directory->pages[page_number - 1u];
    count = page->row_count;
    if (count == 0u || count > ACCURACYCOIN_MAX_PAGE_ROWS)
        return RUNNER_EXIT_FAILURE;
    memcpy(tests, &directory->rows[page->first_row], count * sizeof tests[0]);
    qsort(tests, count, sizeof tests[0], compare_test_name);
    if (page_number == 14u) {
        for (size_t i = 0u; i < 6u; i++) {
            static const char *const required[] = {"Length Counter", "Length Table",
                                                    "Frame Counter IRQ", "Frame Counter 4-step",
                                                    "Frame Counter 5-step",
                                                    "Delta Modulation Channel"};
            int found = 0;
            for (size_t j = 0u; j < count; j++)
                found |= strcmp(tests[j].name, required[i]) == 0;
            if (found == 0) {
                fprintf(stderr, "nesturbator-run: AccuracyCoin page 14 is missing required APU tests\n");
                return RUNNER_EXIT_FAILURE;
            }
        }
    }
    for (size_t i = 1u; i < count; i++) {
        if (strcmp(tests[i - 1u].name, tests[i].name) == 0) {
            fprintf(stderr, "nesturbator-run: duplicate AccuracyCoin test name '%s'\n",
                    tests[i].name);
            return RUNNER_EXIT_FAILURE;
        }
    }
    while (frames < 120u) {
        nesturbator_status status = accuracy_frame(inst, 0u, video, audio, audio_hash);
        frames++;
        if (status != NESTURBATOR_OK ||
            !accuracy_ram(inst, ACCURACYCOIN_MENU_READY_ADDRESS, &value))
            return RUNNER_EXIT_FAILURE;
        if (value == 0x0au)
            break;
    }
    if (value != 0x0au) {
        fprintf(stderr, "nesturbator-run: AccuracyCoin menu did not become ready within 120 frames\n");
        return RUNNER_EXIT_FAILURE;
    }
    for (uint32_t p = 1u; p < page_number; p++) {
        if (accuracy_frame(inst, NESTURBATOR_BUTTON_RIGHT, video, audio, audio_hash) !=
                NESTURBATOR_OK ||
            accuracy_frame(inst, 0u, video, audio, audio_hash) != NESTURBATOR_OK) {
            fprintf(stderr, "nesturbator-run: AccuracyCoin page navigation failed\n");
            return RUNNER_EXIT_FAILURE;
        }
        frames += 2u;
        for (uint32_t settle = 0u; settle < 3u; settle++) {
            if (accuracy_frame(inst, 0u, video, audio, audio_hash) != NESTURBATOR_OK)
                return RUNNER_EXIT_FAILURE;
            frames++;
        }
    }
    uint8_t selected_page = 0xffu;
    uint8_t cursor = 0u;
    if (!accuracy_ram(inst, ACCURACYCOIN_PAGE_INDEX_ADDRESS, &selected_page) ||
        !accuracy_ram(inst, ACCURACYCOIN_CURSOR_ADDRESS, &cursor) ||
        selected_page != (uint8_t)(page_number - 1u) || cursor != 0xffu) {
        fprintf(stderr,
                "nesturbator-run: AccuracyCoin menu selection mismatch (page-index=%u cursor=0x%02x)\n",
                (unsigned)selected_page, (unsigned)cursor);
        return RUNNER_EXIT_FAILURE;
    }
    if (accuracy_frame(inst, NESTURBATOR_BUTTON_A, video, audio, audio_hash) != NESTURBATOR_OK ||
        accuracy_frame(inst, NESTURBATOR_BUTTON_A, video, audio, audio_hash) != NESTURBATOR_OK ||
        accuracy_frame(inst, 0u, video, audio, audio_hash) != NESTURBATOR_OK)
        return RUNNER_EXIT_FAILURE;
    frames += 3u;
    while (frames < 1800u) {
        complete = 1;
        for (size_t i = 0u; i < count; i++) {
            const char *cause = NULL;
            if (!accuracy_ram(inst, tests[i].result_address, &results[i]))
                return RUNNER_EXIT_FAILURE;
            accuracycoin_result_status status = accuracycoin_classify(
                tests[i].name, 1, results[i], 0, &cause);
            if (status == ACCURACYCOIN_RESULT_NOT_RUN || results[i] == 0x03u)
                complete = 0;
        }
        if (complete != 0)
            break;
        if (accuracy_frame(inst, 0u, video, audio, audio_hash) != NESTURBATOR_OK)
            return RUNNER_EXIT_FAILURE;
        frames++;
    }
    if (complete == 0 || frames >= 1800u) {
        for (size_t i = 0u; i < count; i++) {
            (void)accuracy_ram(inst, tests[i].result_address, &results[i]);
            fprintf(stderr, "nesturbator-run: timeout result '%s'=0x%02x\n", tests[i].name,
                    (unsigned)results[i]);
        }
        (void)accuracy_ram(inst, ACCURACYCOIN_MENU_READY_ADDRESS, &value);
        selected_page = 0u;
        cursor = 0u;
        uint8_t automate = 0u;
        uint8_t running = 0u;
        (void)accuracy_ram(inst, ACCURACYCOIN_PAGE_INDEX_ADDRESS, &selected_page);
        (void)accuracy_ram(inst, ACCURACYCOIN_CURSOR_ADDRESS, &cursor);
        (void)accuracy_ram(inst, ACCURACYCOIN_AUTOMATE_ADDRESS, &automate);
        (void)accuracy_ram(inst, ACCURACYCOIN_RUNNING_ADDRESS, &running);
        fprintf(stderr, "nesturbator-run: AccuracyCoin page %u timed out after %u frames\n",
                page_number, frames);
        fprintf(stderr,
                "nesturbator-run: menu $00ec=0x%02x page-index=%u cursor=0x%02x automate=%u running=%u\n",
                (unsigned)value, (unsigned)selected_page, (unsigned)cursor, (unsigned)automate,
                (unsigned)running);
        return RUNNER_EXIT_FAILURE;
    }
    accuracycoin_test required_tests[ACCURACYCOIN_MAX_PAGE_ROWS];
    uint8_t required_results[ACCURACYCOIN_MAX_PAGE_ROWS];
    size_t required_count = 0u;
    int all_passed = 1;
    for (size_t i = 0u; i < count; i++) {
        const char *cause = NULL;
        accuracycoin_result_status result_status;
        char full_key[ACCURACYCOIN_NAME_CAPACITY + 16u];
        if (!accuracy_ram(inst, tests[i].result_address, &results[i]))
            return RUNNER_EXIT_FAILURE;
        (void)snprintf(full_key, sizeof full_key, "accuracycoin/%s", tests[i].name);
        result_status = accuracycoin_classify(full_key, 1, results[i], 0, &cause);
        printf("accuracycoin/%s\t%s\t0x%02x\t-\t-\n", tests[i].name,
               accuracycoin_status_text(result_status), (unsigned)results[i]);
        int required = page_number != 14u || required_page14_test(tests[i].name);
        if (page_number != 14u || required != 0) {
            required_tests[required_count] = tests[i];
            required_results[required_count] = results[i];
            required_count++;
        }
        if (required != 0 && result_status != ACCURACYCOIN_RESULT_PASS) {
            fprintf(stderr, "nesturbator-run: AccuracyCoin page %u test '%s' returned 0x%02x (%s)\n",
                    page_number, tests[i].name, (unsigned)results[i],
                    cause != NULL ? cause : "unknown result");
            all_passed = 0;
        }
    }
    if (!all_passed)
        return RUNNER_EXIT_FAILURE;
    if (!compare_scoreboard(scoreboard_path, required_tests, required_results, required_count)) {
        fprintf(stderr, "nesturbator-run: AccuracyCoin page %u RAM results differ from scoreboard\n",
                page_number);
        return RUNNER_EXIT_FAILURE;
    }
    printf("accuracycoin page %u PASS (%lu tests, %u frames)\n", page_number,
           (unsigned long)count, frames);
    return RUNNER_EXIT_DONE;
}

typedef struct accuracycoin_controls {
    uint8_t menu_ready;
    uint8_t page_index;
    uint8_t cursor;
    uint8_t automate;
    uint8_t running;
} accuracycoin_controls;

static int read_controls(const nesturbator *inst, accuracycoin_controls *controls)
{
    return accuracy_ram(inst, ACCURACYCOIN_MENU_READY_ADDRESS, &controls->menu_ready) &&
           accuracy_ram(inst, ACCURACYCOIN_PAGE_INDEX_ADDRESS, &controls->page_index) &&
           accuracy_ram(inst, ACCURACYCOIN_CURSOR_ADDRESS, &controls->cursor) &&
           accuracy_ram(inst, ACCURACYCOIN_AUTOMATE_ADDRESS, &controls->automate) &&
           accuracy_ram(inst, ACCURACYCOIN_RUNNING_ADDRESS, &controls->running);
}

static int sample_rows(const nesturbator *inst, const accuracycoin_test *rows, size_t count,
                      uint8_t *raw)
{
    for (size_t i = 0u; i < count; i++)
        if (!accuracy_ram(inst, rows[i].result_address, &raw[i]))
            return 0;
    return 1;
}

static int output_path(char out[4096], const char *directory, const char *name)
{
    int length = snprintf(out, 4096u, "%s/%s", directory, name);
    return length > 0 && (size_t)length < 4096u;
}

static int write_raw_sample(FILE *file, const accuracycoin_test *rows, size_t count,
                            const char *stage, const uint8_t *raw)
{
    for (size_t i = 0u; i < count; i++)
        if (fprintf(file, "%s\t%u\t%u\taccuracycoin/%s\t0x%04x\t1\t0x%02x\n", stage,
                    (unsigned)rows[i].page, (unsigned)rows[i].row, rows[i].name,
                    (unsigned)rows[i].result_address, (unsigned)raw[i]) < 0)
            return 0;
    return fflush(file) == 0;
}

static int write_diagnostic(FILE *file, const char *stage, uint32_t frame,
                            const accuracycoin_controls *controls, int transition)
{
    return fprintf(file, "%s\t%u\t%u\t%u\t%u\t%u\t%u\t%d\n", stage, frame,
                   (unsigned)controls->menu_ready, (unsigned)controls->page_index,
                   (unsigned)controls->cursor, (unsigned)controls->automate,
                   (unsigned)controls->running, transition) >= 0 && fflush(file) == 0;
}

static int frame_and_sample(nesturbator *inst, uint16_t buttons, uint16_t *video, int16_t *audio,
                            const accuracycoin_test *rows, size_t count, uint8_t *raw,
                            accuracycoin_controls *controls)
{
    return accuracy_frame(inst, buttons, video, audio, NULL) == NESTURBATOR_OK &&
           read_controls(inst, controls) && sample_rows(inst, rows, count, raw);
}

int accuracycoin_run_observe_page(const accuracycoin_directory *directory, uint32_t page_number,
                                  uint32_t frame_budget, const char *output_dir,
                                  nesturbator *inst, uint16_t *video, int16_t *audio)
{
    accuracycoin_test rows[ACCURACYCOIN_MAX_PAGE_ROWS];
    uint8_t before[ACCURACYCOIN_MAX_PAGE_ROWS];
    uint8_t current[ACCURACYCOIN_MAX_PAGE_ROWS];
    uint8_t after_press[ACCURACYCOIN_MAX_PAGE_ROWS];
    accuracycoin_result_status statuses[ACCURACYCOIN_MAX_PAGE_ROWS];
    const char *causes[ACCURACYCOIN_MAX_PAGE_ROWS];
    accuracycoin_controls controls;
    const accuracycoin_page *page;
    char raw_path[4096];
    char diagnostic_path[4096];
    char report_path[4096];
    char metadata_path[4096];
    FILE *raw_file = NULL;
    FILE *diagnostic_file = NULL;
    FILE *report_file = NULL;
    FILE *metadata_file = NULL;
    size_t count;
    uint32_t frames = 0u;
    uint32_t execution_frames = 0u;
    uint8_t ppu_reset_pretest = 0u;
    int saw_transition = 0;
    int saw_run_flag = 0;
    int any_active = 0;
    int any_not_run = 0;
    uint64_t started_ns;
    uint64_t ended_ns;
    uint64_t elapsed_ns;
    const char *verdict;
    int status = RUNNER_EXIT_FAILURE;
    if (directory == NULL || page_number == 0u || page_number > ACCURACYCOIN_PAGE_COUNT ||
        frame_budget < 16u || output_dir == NULL || output_dir[0] == '\0' || inst == NULL ||
        video == NULL || audio == NULL)
        return RUNNER_EXIT_FAILURE;
    page = &directory->pages[page_number - 1u];
    count = page->row_count;
    if (count == 0u || count > ACCURACYCOIN_MAX_PAGE_ROWS ||
        !output_path(raw_path, output_dir, "raw.tsv") ||
        !output_path(diagnostic_path, output_dir, "diagnostics.tsv") ||
        !output_path(report_path, output_dir, "report.tsv") ||
        !output_path(metadata_path, output_dir, "metadata.txt")) {
        fprintf(stderr, "nesturbator-run: invalid AccuracyCoin output path or page\n");
        return RUNNER_EXIT_FAILURE;
    }
    memcpy(rows, &directory->rows[page->first_row], count * sizeof rows[0]);
    raw_file = fopen(raw_path, "w");
    diagnostic_file = fopen(diagnostic_path, "w");
    if (raw_file == NULL || diagnostic_file == NULL ||
        fprintf(raw_file, "stage\tpage\trow\tkey\tresult_address\tobserved\traw\n") < 0 ||
        fprintf(diagnostic_file,
                "stage\tframe\tmenu_ready\tpage_index\tcursor\tautomate_test_suite\trunning_all_tests\tresult_transition\n") < 0) {
        fprintf(stderr, "nesturbator-run: cannot create AccuracyCoin diagnostics in output directory\n");
        goto done;
    }
    started_ns = nesturbator_run_clock_now_ns();
    memset(&controls, 0, sizeof controls);
    if (!accuracy_ram(inst, ACCURACYCOIN_PPU_RESET_RESULT_ADDRESS, &ppu_reset_pretest))
        goto done;
    while (frames < 120u && frames < frame_budget) {
        if (!frame_and_sample(inst, 0u, video, audio, rows, count, current, &controls))
            goto done;
        frames++;
        if (!write_diagnostic(diagnostic_file, "boot", frames, &controls, 0))
            goto done;
        if (controls.menu_ready == 0x0au)
            break;
    }
    if (controls.menu_ready != 0x0au) {
        fprintf(stderr, "nesturbator-run: AccuracyCoin menu did not become ready within 120 frames\n");
        goto done;
    }
    for (uint32_t target = 1u; target < page_number; target++) {
        for (unsigned step = 0u; step < 5u; step++) {
            uint16_t buttons = step == 0u ? NESTURBATOR_BUTTON_RIGHT : 0u;
            if (frames >= frame_budget ||
                !frame_and_sample(inst, buttons, video, audio, rows, count, current, &controls))
                goto done;
            frames++;
            if (!write_diagnostic(diagnostic_file, "navigate", frames, &controls, 0))
                goto done;
        }
    }
    if (controls.menu_ready != 0x0au || controls.page_index != (uint8_t)(page_number - 1u) ||
        controls.cursor != 0xffu) {
        fprintf(stderr,
                "nesturbator-run: AccuracyCoin page selection mismatch (ready=0x%02x page-index=%u cursor=0x%02x)\n",
                (unsigned)controls.menu_ready, (unsigned)controls.page_index,
                (unsigned)controls.cursor);
        goto done;
    }
    if (!sample_rows(inst, rows, count, before) ||
        !write_raw_sample(raw_file, rows, count, "before-start", before))
        goto done;
    if (frames + 2u > frame_budget ||
        !frame_and_sample(inst, NESTURBATOR_BUTTON_A, video, audio, rows, count, after_press,
                          &controls))
        goto done;
    frames++;
    saw_run_flag |= controls.automate != 0u || controls.running != 0u;
    for (size_t i = 0u; i < count; i++)
        saw_transition |= after_press[i] != before[i];
    if (!write_raw_sample(raw_file, rows, count, "a-press", after_press) ||
        !write_diagnostic(diagnostic_file, "a-press", frames, &controls, saw_transition))
        goto done;
    if (!frame_and_sample(inst, 0u, video, audio, rows, count, current, &controls))
        goto done;
    frames++;
    saw_run_flag |= controls.automate != 0u || controls.running != 0u;
    for (size_t i = 0u; i < count; i++)
        saw_transition |= current[i] != before[i];
    if (!write_raw_sample(raw_file, rows, count, "a-release", current) ||
        !write_diagnostic(diagnostic_file, "a-release", frames, &controls, saw_transition))
        goto done;
    if (!saw_transition) {
        /* The selected page is established; allow the guest a full frame to
         * enter the source-defined automated-page path and mark an attempt. */
        while (frames < frame_budget && !saw_transition) {
            if (!frame_and_sample(inst, 0u, video, audio, rows, count, current, &controls))
                goto done;
            frames++;
            execution_frames++;
            saw_run_flag |= controls.automate != 0u || controls.running != 0u;
            for (size_t i = 0u; i < count; i++)
                saw_transition |= current[i] != before[i];
            if (!write_diagnostic(diagnostic_file, "start-wait", frames, &controls,
                                  saw_transition))
                goto done;
        }
    }
    if (!saw_transition || !saw_run_flag) {
        fprintf(stderr, "nesturbator-run: AccuracyCoin page start was not qualified by a result transition and run state\n");
        goto done;
    }
    memcpy(after_press, current, count);
    while (frames < frame_budget) {
        any_active = 0;
        any_not_run = 0;
        for (size_t i = 0u; i < count; i++) {
            any_active |= current[i] == 0x03u;
            any_not_run |= current[i] == 0u;
        }
        if (!any_active && !any_not_run)
            break;
        if (!frame_and_sample(inst, 0u, video, audio, rows, count, current, &controls))
            goto done;
        frames++;
        execution_frames++;
        saw_run_flag |= controls.automate != 0u || controls.running != 0u;
        for (size_t i = 0u; i < count; i++)
            saw_transition |= current[i] != before[i];
        if (!write_diagnostic(diagnostic_file, "observe", frames, &controls, saw_transition))
            goto done;
    }
    if (!sample_rows(inst, rows, count, current) ||
        !write_raw_sample(raw_file, rows, count, "final", current))
        goto done;
    any_active = 0;
    any_not_run = 0;
    for (size_t i = 0u; i < count; i++) {
        any_active |= current[i] == 0x03u;
        any_not_run |= current[i] == 0u;
    }
    verdict = any_active != 0 ? "guest-timeout" : (any_not_run != 0 ? "bounded-incomplete" : "complete");
    ended_ns = nesturbator_run_clock_now_ns();
    elapsed_ns = started_ns != 0u && ended_ns >= started_ns ? ended_ns - started_ns : 0u;
    report_file = fopen(report_path, "w");
    if (report_file == NULL ||
        fprintf(report_file, "key\tpage\trow\tstatus\tobserved\traw\tframes\tcause\tattempt_id\n") < 0) {
        fprintf(stderr, "nesturbator-run: cannot create AccuracyCoin report\n");
        goto done;
    }
    for (size_t i = 0u; i < count; i++) {
        char key[ACCURACYCOIN_NAME_CAPACITY + 16u];
        (void)snprintf(key, sizeof key, "accuracycoin/%s", rows[i].name);
        statuses[i] = accuracycoin_classify(key, 1, current[i], any_active, &causes[i]);
        if (statuses[i] == ACCURACYCOIN_RESULT_INVALID ||
            fprintf(report_file, "%s\t%u\t%u\t%s\t1\t0x%02x\t%u\t%s\tpage-%02u-cold-01\n",
                    key, (unsigned)rows[i].page, (unsigned)rows[i].row,
                    accuracycoin_status_text(statuses[i]), (unsigned)current[i], execution_frames,
                    causes[i], (unsigned)page_number) < 0) {
            fprintf(stderr, "nesturbator-run: invalid AccuracyCoin result byte or failed report write\n");
            goto done;
        }
    }
    if (fflush(report_file) != 0)
        goto done;
    metadata_file = fopen(metadata_path, "w");
    if (metadata_file == NULL) {
        fprintf(stderr, "nesturbator-run: cannot create AccuracyCoin metadata\n");
        goto done;
    }
    nesturbator_version version;
    memset(&version, 0, sizeof version);
    version.size = (uint32_t)sizeof version;
    nesturbator_get_version(&version);
    if (fprintf(metadata_file,
                "report_schema=1\ninventory_schema=1\nrom_pin=%s\nrom_sha256=%s\ninventory_sha256=%s\nsource_revision=%s\nsource_dirty=%s\nlibrary_version=%u.%u.%u\nbehaviour_revision=%u\nmode=observe\npage=%u\ninput_start_policy=cold-boot;page-header-A;fresh-A-press-release\nboot_frame_budget=120\ntotal_frame_budget=%u\nframes_used=%u\nexecution_frames=%u\nstartup_ppu_reset_pretest=0x%02x\nrun_state_observed=%d\nresult_transition_observed=%d\nverdict=%s\nhost_elapsed_ns=%llu\n",
                directory->pin, directory->rom_sha256, directory->inventory_sha256,
                NESTURBATOR_SOURCE_REVISION, NESTURBATOR_SOURCE_DIRTY,
                (unsigned)version.major, (unsigned)version.minor, (unsigned)version.patch,
                (unsigned)version.behaviour_revision, page_number, frame_budget, frames,
                execution_frames, (unsigned)ppu_reset_pretest, saw_run_flag, saw_transition,
                verdict, (unsigned long long)elapsed_ns) < 0 || fflush(metadata_file) != 0) {
        fprintf(stderr, "nesturbator-run: failed writing AccuracyCoin metadata\n");
        goto done;
    }
    int close_failed = 0;
    if (fclose(raw_file) != 0)
        close_failed = 1;
    raw_file = NULL;
    if (fclose(diagnostic_file) != 0)
        close_failed = 1;
    diagnostic_file = NULL;
    if (fclose(report_file) != 0)
        close_failed = 1;
    report_file = NULL;
    if (fclose(metadata_file) != 0)
        close_failed = 1;
    metadata_file = NULL;
    if (close_failed != 0) {
        fprintf(stderr, "nesturbator-run: failed closing AccuracyCoin evidence files\n");
        goto done;
    }
    printf("accuracycoin observe page %u %s (%lu rows, %u frames; inventory %s)\n", page_number,
           verdict, (unsigned long)count, frames, directory->inventory_sha256);
    status = RUNNER_EXIT_DONE;
done:
    if (raw_file != NULL && fclose(raw_file) != 0)
        status = RUNNER_EXIT_FAILURE;
    if (diagnostic_file != NULL && fclose(diagnostic_file) != 0)
        status = RUNNER_EXIT_FAILURE;
    if (report_file != NULL && fclose(report_file) != 0)
        status = RUNNER_EXIT_FAILURE;
    if (metadata_file != NULL && fclose(metadata_file) != 0)
        status = RUNNER_EXIT_FAILURE;
    return status;
}
