/* AccuracyCoin's directory and result protocol are data in the selected ROM,
 * not a second list of test names or result addresses. Source: the MIT test
 * assembly at the retained pins in tests/roms/manifest.txt. */
#include "accuracycoin.h"

#include <errno.h>
#include <limits.h>
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
static const char new_rom_sha[] =
    "8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc";
static const char old_rom_sha[] =
    "4fe8c2bc9abc6f4d418da47b73f62cba89fcacd950fae763097a1681d650e839";

/* SHA-256 over each page/title record followed by that page's source-order rows. Page records
 * are `P<TAB>page<TAB>title<LF>`; rows are
 * `R<TAB>page<TAB>row<TAB>kind<TAB>accuracycoin/name<TAB>0xresult<TAB>0xroutine<LF>`.
 * These fingerprints were derived from the independently verified pinned ROMs. */
static const char new_inventory_sha[] =
    "7d022800d01ad81159109bf7457af94ab6f80e4a682fce716ec476f7c454a4a5";
static const char old_inventory_sha[] =
    "3271803cdc63c4b4a83cf65453b817dc66ec65ed97863ad19261b56f36821c84";

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
    static const char *const expansions[] = {" indirect", " zeropage", " absolute", " immediate",
                                             "Unofficial Instructions: "};
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

static void inventory_hash_add(nesturbator_run_sha256 *hash, const char *format, unsigned page,
                               unsigned row, const char *kind, const char *title_or_name,
                               unsigned result, unsigned routine)
{
    char line[ACCURACYCOIN_NAME_CAPACITY + 96u];
    int written;
    if (row == 0u) {
        written = snprintf(line, sizeof line, "P\t%u\t%s\n", page, title_or_name);
    } else {
        written =
            snprintf(line, sizeof line, format, page, row, kind, title_or_name, result, routine);
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
            (i == 0u && offset != ACCURACYCOIN_PAGE_TABLE_OFFSET + ACCURACYCOIN_PAGE_COUNT * 2u) ||
            (i > 0u && offset <= page_offsets[i - 1u])) {
            *reason = ACCURACYCOIN_REASON_PAGE_POINTER;
            return 0;
        }
        page_offsets[i] = offset;
    }
    for (unsigned page_index = 0u; page_index < ACCURACYCOIN_PAGE_COUNT; page_index++) {
        size_t limit = page_index + 1u < ACCURACYCOIN_PAGE_COUNT ? page_offsets[page_index + 1u]
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
        if (fprintf(out, "%u\t%u\t%s\taccuracycoin/%s\t0x%04x\t0x%04x\n", (unsigned)row->page,
                    (unsigned)row->row,
                    row->kind == (uint8_t)ACCURACYCOIN_KIND_DRAW ? "DRAW" : "TEST", row->name,
                    (unsigned)row->result_address, (unsigned)row->routine_address) < 0)
            return 0;
    }
    return fflush(out) == 0;
}

const char *accuracycoin_status_text(accuracycoin_result_status status)
{
    static const char *const text[] = {"pass",    "fail", "skip",   "not-run",
                                       "timeout", "draw", "invalid"};
    size_t index = (size_t)status;
    if (index >= sizeof text / sizeof text[0])
        return "invalid";
    return text[index];
}

const char *accuracycoin_validate_start(uint32_t requested_page, uint8_t menu_ready,
                                        uint8_t selected_page, uint8_t cursor, int a_press_observed,
                                        int a_release_observed, int run_state_observed,
                                        int result_transition_observed, int draw_only_page,
                                        int draw_cursor_transition_observed)
{
    if (requested_page == 0u || requested_page > ACCURACYCOIN_PAGE_COUNT || menu_ready != 0x0au)
        return "menu-not-ready";
    if (selected_page != (uint8_t)(requested_page - 1u))
        return "selected-page-mismatch";
    if (cursor != 0xffu)
        return "cursor-not-page-header";
    if (a_press_observed == 0)
        return "a-press-not-observed";
    if (a_release_observed == 0)
        return "a-release-not-observed";
    if (run_state_observed == 0 || (draw_only_page != 0 ? draw_cursor_transition_observed == 0
                                                        : result_transition_observed == 0))
        return "start-not-attributed";
    return "start-attributed";
}

static accuracycoin_result_status accuracycoin_classify_final_row(const accuracycoin_test *row,
                                                                  int observed, uint8_t raw,
                                                                  int active_state,
                                                                  const char **cause)
{
    if (row != NULL && row->kind == (uint8_t)ACCURACYCOIN_KIND_TEST && observed != 0 &&
        raw == 0x03u && active_state == ACCURACYCOIN_ACTIVE_RUNNING) {
        if (cause != NULL)
            *cause = "noncurrent-running-marker";
        return ACCURACYCOIN_RESULT_NOT_RUN;
    }
    return accuracycoin_classify_row(row, observed, raw, active_state, cause);
}

/* AccuracyCoin sometimes returns a passing result that encodes which
 * hardware-defined branch was observed. Keep these exceptions tied to the
 * decoded test key; low bits alone do not make an arbitrary byte a pass.
 * Source: MIT AccuracyCoin.asm at pins 673ef550 and 6bee919. */
static int source_pass_variant(const char *key, uint8_t raw)
{
    static const char *const sha_shs_keys[] = {"accuracycoin/$93   SHA indirect,Y",
                                               "accuracycoin/$9F   SHA absolute,Y",
                                               "accuracycoin/$9B   SHS absolute,Y"};
    static const char *const five_nine_keys[] = {
        "accuracycoin/DMA + $2002 Read",       "accuracycoin/APU Register Activation",
        "accuracycoin/DMA + $4016 Read",       "accuracycoin/Implicit DMA Abort",
        "accuracycoin/Controller Clocking",    "accuracycoin/Sprites On Scanline 0",
        "accuracycoin/Misaligned OAM2 Address"};
    if (key == NULL)
        return 0;
    if ((raw == 0x39u || raw == 0x41u) && (strcmp(key, "accuracycoin/PPU Read Buffer") == 0 ||
                                           strcmp(key, "accuracycoin/Address $2004 behavior") == 0))
        return 1;
    if (raw == 0x05u || raw == 0x09u) {
        for (size_t i = 0u; i < sizeof five_nine_keys / sizeof five_nine_keys[0]; i++)
            if (strcmp(key, five_nine_keys[i]) == 0)
                return 1;
    }
    if (raw == 0x05u || raw == 0x09u || raw == 0x0du || raw == 0x11u) {
        for (size_t i = 0u; i < sizeof sha_shs_keys / sizeof sha_shs_keys[0]; i++)
            if (strcmp(key, sha_shs_keys[i]) == 0)
                return 1;
    }
    return 0;
}

accuracycoin_result_status accuracycoin_classify(const char *key, int observed, uint8_t raw,
                                                 int active_state, const char **cause)
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
        if (active_state == ACCURACYCOIN_ACTIVE_JAM) {
            /* The guest executed a JAM opcode while this row's running marker
             * was set; the row can never finish, so it is a failure, not a
             * timeout. Terminal bytes were classified above and stay as is. */
            if (cause != NULL)
                *cause = "guest-jam";
            return ACCURACYCOIN_RESULT_FAIL;
        }
        if (active_state != ACCURACYCOIN_ACTIVE_RUNNING) {
            if (cause != NULL)
                *cause = "active-test-budget-exhausted";
            return ACCURACYCOIN_RESULT_TIMEOUT;
        }
        if (cause != NULL)
            *cause = "active-test";
        return ACCURACYCOIN_RESULT_INVALID;
    }
    if ((raw & 3u) == 1u) {
        if (raw == 0x01u || source_pass_variant(key, raw)) {
            if (cause != NULL)
                *cause = raw == 0x01u ? "terminal-pass" : "source-defined-pass-variant";
            return ACCURACYCOIN_RESULT_PASS;
        }
        if (cause != NULL)
            *cause = "unsupported-pass-variant";
        return ACCURACYCOIN_RESULT_INVALID;
    }
    if ((raw & 3u) == 2u) {
        /* TEST_Fail returns (ErrorCode << 2) | $02. Some pinned instructions
         * explicitly return $02 for an invalid instruction length, error 0. */
        if (cause != NULL)
            *cause = "terminal-fail";
        return ACCURACYCOIN_RESULT_FAIL;
    }
    return ACCURACYCOIN_RESULT_INVALID;
}

static int accuracycoin_make_full_key(char *out, size_t capacity, const char *name);

accuracycoin_result_status accuracycoin_classify_row(const accuracycoin_test *row, int observed,
                                                     uint8_t raw, int active_state,
                                                     const char **cause)
{
    char key[ACCURACYCOIN_NAME_CAPACITY + 16u];
    if (cause != NULL)
        *cause = "invalid-row";
    if (row == NULL)
        return ACCURACYCOIN_RESULT_INVALID;
    if (row->kind == (uint8_t)ACCURACYCOIN_KIND_DRAW) {
        if (cause != NULL)
            *cause = "inventory-only-draw";
        return ACCURACYCOIN_RESULT_DRAW;
    }
    if (row->kind != (uint8_t)ACCURACYCOIN_KIND_TEST)
        return ACCURACYCOIN_RESULT_INVALID;
    if (!accuracycoin_make_full_key(key, sizeof key, row->name))
        return ACCURACYCOIN_RESULT_INVALID;
    return accuracycoin_classify(key, observed, raw, active_state, cause);
}

static int accuracy_ram(const nesturbator *inst, uint16_t address, uint8_t *out)
{
    return nesturbator_peek_cpu_ram(inst, address, out) == NESTURBATOR_OK;
}

static nesturbator_status accuracy_frame(nesturbator *inst, uint16_t buttons, uint16_t *video,
                                         int16_t *audio, nesturbator_run_audio_hash *audio_hash)
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

static int accuracycoin_make_full_key(char *out, size_t capacity, const char *name)
{
    static const char prefix[] = "accuracycoin/";
    const size_t prefix_size = sizeof prefix - 1u;
    size_t name_size = 0u;
    if (out == NULL || name == NULL || capacity <= prefix_size)
        return 0;
    while (name_size < ACCURACYCOIN_NAME_CAPACITY && name[name_size] != '\0')
        name_size++;
    if (name_size == ACCURACYCOIN_NAME_CAPACITY || name_size >= capacity - prefix_size)
        return 0;
    memcpy(out, prefix, prefix_size);
    memcpy(out + prefix_size, name, name_size + 1u);
    return 1;
}

static int accuracycoin_make_scoreboard_line(char *out, size_t capacity, const char *full_key,
                                             accuracycoin_result_status status, uint8_t raw)
{
    static const char hex[] = "0123456789abcdef";
    static const char tail[] = "\t-\t-\n";
    const char *status_text = accuracycoin_status_text(status);
    size_t key_size;
    size_t status_size;
    size_t offset = 0u;
    if (out == NULL || full_key == NULL || status_text == NULL)
        return 0;
    key_size = strlen(full_key);
    status_size = strlen(status_text);
    if (capacity <= key_size || capacity - key_size < status_size + 12u)
        return 0;
    memcpy(out + offset, full_key, key_size);
    offset += key_size;
    out[offset++] = '\t';
    memcpy(out + offset, status_text, status_size);
    offset += status_size;
    out[offset++] = '\t';
    out[offset++] = '0';
    out[offset++] = 'x';
    out[offset++] = hex[(raw >> 4u) & 0x0fu];
    out[offset++] = hex[raw & 0x0fu];
    memcpy(out + offset, tail, sizeof tail - 1u);
    offset += sizeof tail - 1u;
    out[offset] = '\0';
    return 1;
}

static int required_page14_test(const char *name)
{
    static const char *const required[] = {"Length Counter",       "Length Table",
                                           "Frame Counter IRQ",    "Frame Counter 4-step",
                                           "Frame Counter 5-step", "Delta Modulation Channel"};
    for (size_t i = 0u; i < sizeof required / sizeof required[0]; i++)
        if (strcmp(name, required[i]) == 0)
            return 1;
    return 0;
}

static int compare_scoreboard(const char *path, const accuracycoin_test *tests, const uint8_t *raw,
                              size_t count)
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
            char expected[192];
            char full_key[ACCURACYCOIN_NAME_CAPACITY + 16u];
            const char *cause = NULL;
            if (!accuracycoin_make_full_key(full_key, sizeof full_key, tests[i].name)) {
                fclose(file);
                return 0;
            }
            accuracycoin_result_status status =
                accuracycoin_classify(full_key, 1, raw[i], 0, &cause);
            (void)cause;
            size_t full_key_size = strlen(full_key);
            if (strncmp(line, full_key, full_key_size) == 0 && line[full_key_size] == '\t') {
                if (!accuracycoin_make_scoreboard_line(expected, sizeof expected, full_key, status,
                                                       raw[i])) {
                    fclose(file);
                    return 0;
                }
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
            static const char *const required[] = {
                "Length Counter",       "Length Table",         "Frame Counter IRQ",
                "Frame Counter 4-step", "Frame Counter 5-step", "Delta Modulation Channel"};
            int found = 0;
            for (size_t j = 0u; j < count; j++)
                found |= strcmp(tests[j].name, required[i]) == 0;
            if (found == 0) {
                fprintf(stderr,
                        "nesturbator-run: AccuracyCoin page 14 is missing required APU tests\n");
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
        fprintf(stderr,
                "nesturbator-run: AccuracyCoin menu did not become ready within 120 frames\n");
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
        fprintf(
            stderr,
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
            accuracycoin_result_status status =
                accuracycoin_classify(tests[i].name, 1, results[i], 0, &cause);
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
                "nesturbator-run: menu $00ec=0x%02x page-index=%u cursor=0x%02x automate=%u "
                "running=%u\n",
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
        if (!accuracycoin_make_full_key(full_key, sizeof full_key, tests[i].name))
            return RUNNER_EXIT_FAILURE;
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
            fprintf(stderr,
                    "nesturbator-run: AccuracyCoin page %u test '%s' returned 0x%02x (%s)\n",
                    page_number, tests[i].name, (unsigned)results[i],
                    cause != NULL ? cause : "unknown result");
            all_passed = 0;
        }
    }
    if (!all_passed)
        return RUNNER_EXIT_FAILURE;
    if (!compare_scoreboard(scoreboard_path, required_tests, required_results, required_count)) {
        fprintf(stderr,
                "nesturbator-run: AccuracyCoin page %u RAM results differ from scoreboard\n",
                page_number);
        return RUNNER_EXIT_FAILURE;
    }
    printf("accuracycoin page %u PASS (%lu tests, %u frames)\n", page_number, (unsigned long)count,
           frames);
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
    for (size_t i = 0u; i < count; i++) {
        if (rows[i].kind == (uint8_t)ACCURACYCOIN_KIND_DRAW) {
            raw[i] = 0u;
            continue;
        }
        if (!accuracy_ram(inst, rows[i].result_address, &raw[i]))
            return 0;
    }
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
    for (size_t i = 0u; i < count; i++) {
        int observed = rows[i].kind == (uint8_t)ACCURACYCOIN_KIND_TEST;
        char raw_text[5];
        if (observed != 0)
            (void)snprintf(raw_text, sizeof raw_text, "0x%02x", (unsigned)raw[i]);
        else
            (void)snprintf(raw_text, sizeof raw_text, "-");
        if (fprintf(file, "%s\t%u\t%u\taccuracycoin/%s\t0x%04x\t%d\t%s\n", stage,
                    (unsigned)rows[i].page, (unsigned)rows[i].row, rows[i].name,
                    (unsigned)rows[i].result_address, observed, raw_text) < 0)
            return 0;
    }
    return fflush(file) == 0;
}

static int scored_result_transition(const accuracycoin_test *rows, size_t count,
                                    const uint8_t *before, const uint8_t *after)
{
    for (size_t i = 0u; i < count; i++)
        if (rows[i].kind == (uint8_t)ACCURACYCOIN_KIND_TEST && before[i] != after[i])
            return 1;
    return 0;
}

static int write_diagnostic(FILE *file, const char *stage, uint64_t frame,
                            const accuracycoin_controls *controls, int transition)
{
    return fprintf(file, "%s\t%llu\t%u\t%u\t%u\t%u\t%u\t%d\n", stage, (unsigned long long)frame,
                   (unsigned)controls->menu_ready, (unsigned)controls->page_index,
                   (unsigned)controls->cursor, (unsigned)controls->automate,
                   (unsigned)controls->running, transition) >= 0 &&
           fflush(file) == 0;
}

/* NESTURBATOR_OK: the frame ran and both samples were taken. NESTURBATOR_STOP_JAM
 * passes through unchanged so callers can name a guest JAM. Any other failure
 * (a frame error or an unreadable sample) is a non-OK, non-JAM status. */
static nesturbator_status frame_and_sample(nesturbator *inst, uint16_t buttons, uint16_t *video,
                                           int16_t *audio, const accuracycoin_test *rows,
                                           size_t count, uint8_t *raw,
                                           accuracycoin_controls *controls)
{
    nesturbator_status status = accuracy_frame(inst, buttons, video, audio, NULL);
    if (status != NESTURBATOR_OK)
        return status == NESTURBATOR_STOP_JAM ? NESTURBATOR_STOP_JAM : NESTURBATOR_ERR_ARGUMENT;
    if (!read_controls(inst, controls) || !sample_rows(inst, rows, count, raw))
        return NESTURBATOR_ERR_ARGUMENT;
    return NESTURBATOR_OK;
}

/* A guest JAM (the CPU executed a KIL/JAM opcode) is named as such in the
 * observe verdict and stop cause, with the total frames completed before it. */
static void observe_note_jam(const char **stop_cause, const char **verdict, char *jam_frame,
                             size_t jam_frame_capacity, uint64_t frames)
{
    *stop_cause = "guest-jam";
    *verdict = "guest-jam";
    (void)snprintf(jam_frame, jam_frame_capacity, "%llu", (unsigned long long)frames);
}

int accuracycoin_run_observe_page(const accuracycoin_directory *directory, uint32_t page_number,
                                  const accuracycoin_run_budgets *budgets, const char *output_dir,
                                  nesturbator *inst, uint16_t *video, int16_t *audio)
{
    accuracycoin_test rows[ACCURACYCOIN_MAX_PAGE_ROWS];
    uint8_t before[ACCURACYCOIN_MAX_PAGE_ROWS];
    uint8_t current[ACCURACYCOIN_MAX_PAGE_ROWS];
    uint8_t after_press[ACCURACYCOIN_MAX_PAGE_ROWS];
    accuracycoin_result_status result_status;
    accuracycoin_controls controls;
    accuracycoin_controls selected_controls;
    const accuracycoin_page *page;
    const char *cause;
    char raw_path[4096];
    char diagnostic_path[4096];
    char report_path[4096];
    char metadata_path[4096];
    FILE *raw_file = NULL;
    FILE *diagnostic_file = NULL;
    FILE *report_file = NULL;
    FILE *metadata_file = NULL;
    size_t count;
    uint64_t frames = 0u;
    uint64_t boot_frames = 0u;
    uint64_t navigation_frames = 0u;
    uint64_t start_frames = 0u;
    uint64_t execution_frames = 0u;
    uint64_t started_ns = 0u;
    uint64_t ended_ns = 0u;
    uint64_t elapsed_ns = 0u;
    uint64_t nav_max;
    uint64_t required_frames;
    uint8_t pretest_dmc = 0u;
    uint8_t pretest_vblank = 0u;
    uint8_t pretest_ppu_reset = 0u;
    uint8_t selected_page = 0xffu;
    uint8_t selected_cursor = 0xffu;
    int pretest_dmc_observed = 0;
    int pretest_vblank_observed = 0;
    int pretest_ppu_reset_observed = 0;
    int selection_observed = 0;
    int a_press_observed = 0;
    int a_release_observed = 0;
    int saw_transition = 0;
    int saw_run_flag = 0;
    int start_attributed = 0;
    int draw_only_page = 1;
    int draw_cursor_transition_observed = 0;
    int menu_returned_to_header_observed = 0;
    int any_active = 0;
    int active_count = 0;
    int any_not_run = 0;
    size_t active_index = 0u;
    const char *verdict = "startup-failure";
    const char *stop_cause = "runner-error";
    char active_key[ACCURACYCOIN_NAME_CAPACITY + 16u] = "-";
    char active_raw[8] = "-";
    char jam_frame[24] = "-";
    int jam_stop = 0;
    nesturbator_version version;
    int status = RUNNER_EXIT_FAILURE;

    if (directory == NULL || page_number == 0u || page_number > ACCURACYCOIN_PAGE_COUNT ||
        budgets == NULL || budgets->boot_frames == 0u ||
        budgets->navigation_frames_per_page == 0u || budgets->execution_frames == 0u ||
        budgets->total_frames == 0u || output_dir == NULL || output_dir[0] == '\0' ||
        inst == NULL || video == NULL || audio == NULL)
        return RUNNER_EXIT_FAILURE;

    nav_max = (uint64_t)(page_number - 1u) * budgets->navigation_frames_per_page;
    if (UINT64_MAX - (uint64_t)budgets->boot_frames < nav_max) {
        fprintf(stderr, "nesturbator-run: AccuracyCoin navigation budget overflows frame total\n");
        return RUNNER_EXIT_USAGE;
    }
    required_frames = (uint64_t)budgets->boot_frames + nav_max;
    if (UINT64_MAX - required_frames < 2u) {
        fprintf(stderr, "nesturbator-run: AccuracyCoin start budget overflows frame total\n");
        return RUNNER_EXIT_USAGE;
    }
    required_frames += 2u;
    if (UINT64_MAX - required_frames < (uint64_t)budgets->execution_frames ||
        required_frames + (uint64_t)budgets->execution_frames > budgets->total_frames) {
        fprintf(stderr,
                "nesturbator-run: AccuracyCoin stage caps exceed checked total frame bound\n");
        return RUNNER_EXIT_USAGE;
    }

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
    for (size_t i = 0u; i < count; i++)
        if (rows[i].kind == (uint8_t)ACCURACYCOIN_KIND_TEST)
            draw_only_page = 0;
    memset(&controls, 0, sizeof controls);
    memset(&selected_controls, 0, sizeof selected_controls);

    raw_file = fopen(raw_path, "w");
    diagnostic_file = fopen(diagnostic_path, "w");
    if (raw_file == NULL || diagnostic_file == NULL ||
        fprintf(raw_file, "stage\tpage\trow\tkey\tresult_address\tobserved\traw\n") < 0 ||
        fprintf(diagnostic_file, "stage\tframe\tmenu_ready\tpage_index\tcursor\tautomate_test_"
                                 "suite\trunning_all_tests\tresult_transition\n") < 0) {
        stop_cause = "evidence-open-failure";
        fprintf(stderr,
                "nesturbator-run: cannot create AccuracyCoin diagnostics in output directory\n");
        goto done;
    }

    started_ns = nesturbator_run_clock_now_ns();
    while (boot_frames < budgets->boot_frames && frames < budgets->total_frames) {
        if (frame_and_sample(inst, 0u, video, audio, rows, count, current, &controls) !=
            NESTURBATOR_OK) {
            stop_cause = "boot-frame-failure";
            verdict = "guest-failure";
            goto done;
        }
        frames++;
        boot_frames++;
        if (!write_diagnostic(diagnostic_file, "boot", frames, &controls, 0)) {
            stop_cause = "diagnostic-write-failure";
            goto done;
        }
        if (controls.menu_ready == 0x0au)
            break;
    }
    pretest_dmc_observed = accuracy_ram(inst, 0x0012u, &pretest_dmc);
    pretest_vblank_observed = accuracy_ram(inst, 0x003au, &pretest_vblank);
    pretest_ppu_reset_observed =
        accuracy_ram(inst, ACCURACYCOIN_PPU_RESET_RESULT_ADDRESS, &pretest_ppu_reset);
    if (!pretest_dmc_observed || !pretest_vblank_observed || !pretest_ppu_reset_observed) {
        stop_cause = "startup-pretest-unreadable";
        goto done;
    }
    if (controls.menu_ready != 0x0au) {
        stop_cause = "boot-cap-exhausted";
        verdict = "startup-timeout";
        status = RUNNER_EXIT_DONE;
        goto done;
    }

    for (uint32_t target = 1u; target < page_number; target++) {
        for (uint32_t step = 0u; step < budgets->navigation_frames_per_page; step++) {
            uint16_t buttons = step == 0u ? NESTURBATOR_BUTTON_RIGHT : 0u;
            if (frames >= budgets->total_frames ||
                frame_and_sample(inst, buttons, video, audio, rows, count, current, &controls) !=
                    NESTURBATOR_OK) {
                stop_cause = "navigation-frame-failure";
                verdict = "guest-failure";
                goto done;
            }
            frames++;
            navigation_frames++;
            if (!write_diagnostic(diagnostic_file, "navigate", frames, &controls, 0)) {
                stop_cause = "diagnostic-write-failure";
                goto done;
            }
        }
        if (controls.menu_ready != 0x0au || controls.page_index != (uint8_t)target ||
            controls.cursor != 0xffu) {
            stop_cause = "navigation-selection-mismatch";
            verdict = "driver-failure";
            goto done;
        }
    }

    selected_page = controls.page_index;
    selected_cursor = controls.cursor;
    selection_observed = 1;
    if (controls.menu_ready != 0x0au || selected_page != (uint8_t)(page_number - 1u) ||
        selected_cursor != 0xffu) {
        stop_cause = "selected-page-mismatch";
        verdict = "driver-failure";
        goto done;
    }
    selected_controls = controls;
    if (!sample_rows(inst, rows, count, before) ||
        !write_raw_sample(raw_file, rows, count, "before-start", before)) {
        stop_cause = "before-start-sample-failure";
        goto done;
    }

    if (frames >= budgets->total_frames ||
        frame_and_sample(inst, NESTURBATOR_BUTTON_A, video, audio, rows, count, after_press,
                         &controls) != NESTURBATOR_OK) {
        stop_cause = "a-press-frame-failure";
        verdict = "driver-failure";
        goto done;
    }
    frames++;
    start_frames++;
    a_press_observed = 1;
    saw_run_flag |= controls.automate != 0u || controls.running != 0u;
    saw_transition |= scored_result_transition(rows, count, before, after_press);
    draw_cursor_transition_observed = draw_only_page != 0 && selected_controls.cursor == 0xffu &&
                                      controls.page_index == selected_controls.page_index &&
                                      controls.cursor == 0u && controls.automate != 0u;
    if (!write_raw_sample(raw_file, rows, count, "a-press", after_press) ||
        !write_diagnostic(diagnostic_file, "a-press", frames, &controls, saw_transition)) {
        stop_cause = "start-evidence-write-failure";
        goto done;
    }

    if (frames >= budgets->total_frames ||
        frame_and_sample(inst, 0u, video, audio, rows, count, current, &controls) !=
            NESTURBATOR_OK) {
        stop_cause = "a-release-frame-failure";
        verdict = "driver-failure";
        goto done;
    }
    frames++;
    start_frames++;
    a_release_observed = 1;
    saw_run_flag |= controls.automate != 0u || controls.running != 0u;
    saw_transition |= scored_result_transition(rows, count, before, current);
    if (!write_raw_sample(raw_file, rows, count, "a-release", current) ||
        !write_diagnostic(diagnostic_file, "a-release", frames, &controls, saw_transition)) {
        stop_cause = "start-evidence-write-failure";
        goto done;
    }

    cause = accuracycoin_validate_start(
        page_number, selected_controls.menu_ready, selected_controls.page_index,
        selected_controls.cursor, a_press_observed, a_release_observed, saw_run_flag,
        saw_transition, draw_only_page, draw_cursor_transition_observed);
    if (strcmp(cause, "start-attributed") != 0) {
        stop_cause = cause;
        verdict = "driver-failure";
        fprintf(stderr, "nesturbator-run: AccuracyCoin page %u start was not attributed (%s)\n",
                page_number, cause);
        goto done;
    }
    start_attributed = 1;
    while (execution_frames < budgets->execution_frames) {
        any_active = 0;
        any_not_run = 0;
        active_count = 0;
        if (draw_only_page != 0) {
            any_not_run = controls.automate != 0u || controls.cursor != 0xffu;
        } else {
            for (size_t i = 0u; i < count; i++) {
                if (rows[i].kind != (uint8_t)ACCURACYCOIN_KIND_TEST)
                    continue;
                if (current[i] == 0x03u) {
                    any_active = 1;
                    active_count++;
                    active_index = i;
                }
                any_not_run |= current[i] == 0u;
            }
        }
        int menu_execution_complete = controls.menu_ready == 0x0au &&
                                      controls.page_index == (uint8_t)(page_number - 1u) &&
                                      controls.cursor == 0xffu && controls.automate == 0u;
        if (!any_active && !any_not_run && menu_execution_complete)
            break;
        if (frames >= budgets->total_frames) {
            stop_cause = "execution-frame-failure";
            verdict = "guest-failure";
            goto done;
        }
        nesturbator_status frame_status =
            frame_and_sample(inst, 0u, video, audio, rows, count, current, &controls);
        if (frame_status == NESTURBATOR_STOP_JAM) {
            /* Checked before the generic failure branch: a JAM is the guest's
             * own stop, not a driver or emulator error (JAM-01). */
            observe_note_jam(&stop_cause, &verdict, jam_frame, sizeof jam_frame, frames);
            jam_stop = 1;
            break;
        }
        if (frame_status != NESTURBATOR_OK) {
            stop_cause = "execution-frame-failure";
            verdict = "guest-failure";
            goto done;
        }
        frames++;
        execution_frames++;
        saw_run_flag |= controls.automate != 0u || controls.running != 0u;
        saw_transition |= scored_result_transition(rows, count, before, current);
        if (!write_diagnostic(diagnostic_file, "observe", frames, &controls, saw_transition)) {
            stop_cause = "diagnostic-write-failure";
            goto done;
        }
    }

    if (!sample_rows(inst, rows, count, current) ||
        !write_raw_sample(raw_file, rows, count, "final", current)) {
        stop_cause = "final-sample-failure";
        goto done;
    }
    /* After a JAM the controls still hold the last completed frame; refresh them
     * from CPU RAM, which stays readable, so the attribution reads the cursor at
     * the JAM. */
    if (jam_stop != 0 && !read_controls(inst, &controls)) {
        stop_cause = "final-sample-failure";
        goto done;
    }
    any_active = 0;
    any_not_run = 0;
    active_count = 0;
    active_index = count;
    for (size_t i = 0u; i < count; i++) {
        if (rows[i].kind != (uint8_t)ACCURACYCOIN_KIND_TEST)
            continue;
        if (current[i] == 0x03u) {
            any_active = 1;
            active_count++;
        }
        any_not_run |= current[i] == 0u;
    }
    menu_returned_to_header_observed = controls.menu_ready == 0x0au &&
                                       controls.page_index == (uint8_t)(page_number - 1u) &&
                                       controls.cursor == 0xffu && controls.automate == 0u;
    if (jam_stop != 0) {
        /* The JAM ended the loop early, so the cap checks below do not apply
         * (they would misread it as active-row-not-current). The JAM belongs to
         * the row the suite is running when automation is active, the cursor
         * names a TEST row and that row's byte is still 0x03; otherwise it is
         * unattributed. stop_cause and verdict were set by observe_note_jam. */
        if (draw_only_page == 0 && controls.automate != 0u && controls.cursor < count &&
            rows[controls.cursor].kind == (uint8_t)ACCURACYCOIN_KIND_TEST &&
            current[controls.cursor] == 0x03u) {
            active_index = controls.cursor;
            (void)snprintf(active_key, sizeof active_key, "accuracycoin/%s",
                           rows[active_index].name);
            (void)snprintf(active_raw, sizeof active_raw, "0x%02x",
                           (unsigned)current[active_index]);
        }
        fprintf(stderr, "nesturbator-run: JAM stopped AccuracyCoin page %u at frame %s (%s=%s)\n",
                page_number, jam_frame, "active_key", active_key);
        status = RUNNER_EXIT_DONE;
    } else if (draw_only_page != 0) {
        if (menu_returned_to_header_observed == 0) {
            stop_cause = execution_frames >= budgets->execution_frames
                             ? "draw-sequence-cap-exhausted"
                             : "draw-sequence-incomplete";
            verdict = "bounded-incomplete";
            status = RUNNER_EXIT_DONE;
        } else {
            stop_cause = "draw-sequence-complete";
            verdict = "complete";
            status = RUNNER_EXIT_DONE;
        }
    } else if (active_count != 0) {
        if (controls.automate == 0u || controls.cursor >= count ||
            rows[controls.cursor].kind != (uint8_t)ACCURACYCOIN_KIND_TEST ||
            current[controls.cursor] != 0x03u || execution_frames != budgets->execution_frames) {
            stop_cause = "active-row-not-current-in-automation";
            verdict = "driver-failure";
            goto done;
        }
        active_index = controls.cursor;
        (void)snprintf(active_key, sizeof active_key, "accuracycoin/%s", rows[active_index].name);
        (void)snprintf(active_raw, sizeof active_raw, "0x%02x", (unsigned)current[active_index]);
        stop_cause = "execution-cap-active-test";
        verdict = "guest-timeout";
        status = RUNNER_EXIT_DONE;
    } else if (any_active != 0 || any_not_run != 0 || menu_returned_to_header_observed == 0) {
        stop_cause = "execution-ended-with-not-run-rows";
        if (any_active == 0 && any_not_run == 0)
            stop_cause = "execution-cap-before-menu-return";
        verdict = "bounded-incomplete";
        status = RUNNER_EXIT_DONE;
    } else {
        stop_cause = "all-page-results-terminal";
        verdict = "complete";
        status = RUNNER_EXIT_DONE;
    }

    report_file = fopen(report_path, "w");
    if (report_file == NULL ||
        fprintf(report_file, "key\tpage\trow\tstatus\tobserved\traw\tframes\tcause\tattempt_id\n") <
            0) {
        stop_cause = "report-open-failure";
        verdict = "output-failure";
        status = RUNNER_EXIT_FAILURE;
        goto done;
    }
    for (size_t i = 0u; i < count; i++) {
        char raw_text[5];
        int observed = rows[i].kind == (uint8_t)ACCURACYCOIN_KIND_TEST;
        result_status = accuracycoin_classify_final_row(
            &rows[i], observed, current[i],
            i != active_index ? ACCURACYCOIN_ACTIVE_RUNNING
            : jam_stop != 0   ? ACCURACYCOIN_ACTIVE_JAM
                              : ACCURACYCOIN_ACTIVE_BUDGET,
            &cause);
        if (observed != 0)
            (void)snprintf(raw_text, sizeof raw_text, "0x%02x", (unsigned)current[i]);
        else
            (void)snprintf(raw_text, sizeof raw_text, "-");
        if (result_status == ACCURACYCOIN_RESULT_INVALID ||
            fprintf(report_file,
                    "accuracycoin/%s\t%u\t%u\t%s\t%d\t%s\t%llu\t%s\tpage-%02u-cold-01\n",
                    rows[i].name, (unsigned)rows[i].page, (unsigned)rows[i].row,
                    accuracycoin_status_text(result_status), observed, raw_text,
                    (unsigned long long)execution_frames, cause, (unsigned)page_number) < 0) {
            stop_cause = "invalid-result-or-report-write-failure";
            verdict = "output-failure";
            status = RUNNER_EXIT_FAILURE;
            goto done;
        }
    }
    if (fflush(report_file) != 0) {
        stop_cause = "report-flush-failure";
        verdict = "output-failure";
        status = RUNNER_EXIT_FAILURE;
        goto done;
    }

done:
    ended_ns = nesturbator_run_clock_now_ns();
    elapsed_ns = started_ns != 0u && ended_ns >= started_ns ? ended_ns - started_ns : 0u;
    memset(&version, 0, sizeof version);
    version.size = (uint32_t)sizeof version;
    nesturbator_get_version(&version);
    if (metadata_file == NULL && output_path(metadata_path, output_dir, "metadata.txt"))
        metadata_file = fopen(metadata_path, "w");
    if (metadata_file == NULL ||
        fprintf(
            metadata_file,
            "report_schema=1\ninventory_schema=1\nrom_pin=%s\nrom_sha256=%s\ninventory_sha256=%"
            "s\nsource_revision=%s\nsource_dirty=%s\nlibrary_version=%u.%u.%u\nbehaviour_revision=%"
            "u\nmode=observe\npage=%u\nattempt_id=page-%02u-cold-01\norigin=cold-page\nrequested_"
            "row=all\ninput_start_policy=cold-boot;page-header-A;one-frame-A-press;one-frame-"
            "release\nboot_frame_budget=%u\nnavigation_frame_budget_per_page=%u\nexecution_frame_"
            "budget=%u\ntotal_frame_budget=%llu\nboot_frames_used=%llu\nnavigation_frames_used=%"
            "llu\nstart_frames_used=%llu\nexecution_frames=%llu\nframes_used=%llu\nstartup_dmc_"
            "pretest_observed=%d\nstartup_dmc_pretest=0x%02x\nstartup_vblank_pretest_observed=%"
            "d\nstartup_vblank_pretest=0x%02x\nstartup_ppu_reset_pretest_observed=%d\nstartup_ppu_"
            "reset_pretest=0x%02x\nselection_observed=%d\nselected_page=%u\nselected_cursor=%u\na_"
            "press_observed=%d\na_release_observed=%d\nrelease_frames=%d\nrun_state_observed=%"
            "d\nresult_transition_observed=%d\ndraw_cursor_transition_observed=%d\nmenu_returned_"
            "to_header_observed=%d\nstart_attributed=%d\nstop_cause=%s\nactive_key=%s\nactive_raw=%"
            "s\nverdict=%s\njam_frame=%s\nobservation_started_ns=%llu\nhost_elapsed_ns=%llu\n",
            directory->pin, directory->rom_sha256, directory->inventory_sha256,
            NESTURBATOR_SOURCE_REVISION, NESTURBATOR_SOURCE_DIRTY, (unsigned)version.major,
            (unsigned)version.minor, (unsigned)version.patch, (unsigned)version.behaviour_revision,
            (unsigned)page_number, (unsigned)page_number, (unsigned)budgets->boot_frames,
            (unsigned)budgets->navigation_frames_per_page, (unsigned)budgets->execution_frames,
            (unsigned long long)budgets->total_frames, (unsigned long long)boot_frames,
            (unsigned long long)navigation_frames, (unsigned long long)start_frames,
            (unsigned long long)execution_frames, (unsigned long long)frames, pretest_dmc_observed,
            (unsigned)pretest_dmc, pretest_vblank_observed, (unsigned)pretest_vblank,
            pretest_ppu_reset_observed, (unsigned)pretest_ppu_reset, selection_observed,
            (unsigned)selected_page, (unsigned)selected_cursor, a_press_observed,
            a_release_observed, a_release_observed, saw_run_flag, saw_transition,
            draw_cursor_transition_observed, menu_returned_to_header_observed, start_attributed,
            stop_cause, active_key, active_raw, verdict, jam_frame, (unsigned long long)started_ns,
            (unsigned long long)elapsed_ns) < 0 ||
        fflush(metadata_file) != 0) {
        fprintf(stderr, "nesturbator-run: failed writing AccuracyCoin metadata\n");
        status = RUNNER_EXIT_FAILURE;
    }

    int close_failed = 0;
    if (raw_file != NULL && fclose(raw_file) != 0)
        close_failed = 1;
    if (diagnostic_file != NULL && fclose(diagnostic_file) != 0)
        close_failed = 1;
    if (report_file != NULL && fclose(report_file) != 0)
        close_failed = 1;
    if (metadata_file != NULL && fclose(metadata_file) != 0)
        close_failed = 1;
    if (close_failed != 0) {
        fprintf(stderr, "nesturbator-run: failed closing AccuracyCoin evidence files\n");
        status = RUNNER_EXIT_FAILURE;
    }
    if (status == RUNNER_EXIT_DONE) {
        printf("accuracycoin observe page %u %s (%lu rows, %llu frames; inventory %s)\n",
               page_number, verdict, (unsigned long)count, (unsigned long long)frames,
               directory->inventory_sha256);
        if (fflush(stdout) != 0 || ferror(stdout)) {
            fprintf(stderr, "nesturbator-run: cannot write AccuracyCoin summary\n");
            status = RUNNER_EXIT_FAILURE;
        }
    }
    return status;
}

#define ACCURACYCOIN_CAMPAIGN_TEXT_CAPACITY 65536u
#define ACCURACYCOIN_CAMPAIGN_LINE_CAPACITY 2048u
#define ACCURACYCOIN_CAMPAIGN_STAGE_COUNT 5u

typedef struct accuracycoin_outer_outcome {
    char outcome[24];
    char exit_code[32];
    uint32_t deadline_seconds;
} accuracycoin_outer_outcome;

typedef struct accuracycoin_controls_evidence {
    uint8_t last_menu_ready;
    uint8_t last_page_index;
    uint8_t last_cursor;
    uint8_t last_automate;
    uint8_t last_running;
    uint8_t press_page_index;
    uint8_t press_cursor;
    uint8_t press_automate;
    int saw_run_state;
    int saw_result_transition;
    int have_press_record;
} accuracycoin_controls_evidence;

static int campaign_read_text(const char *path, char *buffer, size_t capacity, int required,
                              int *present)
{
    FILE *file = fopen(path, "rb");
    size_t count;
    int failed;
    if (file == NULL) {
        if (errno == ENOENT) {
            *present = 0;
            return required == 0;
        }
        return 0;
    }
    *present = 1;
    count = fread(buffer, 1u, capacity - 1u, file);
    failed = ferror(file) != 0 || count == capacity - 1u;
    if (fclose(file) != 0)
        failed = 1;
    if (failed != 0)
        return 0;
    if (memchr(buffer, '\0', count) != NULL)
        return 0;
    buffer[count] = '\0';
    {
        size_t normalized = 0u;
        for (size_t i = 0u; i < count; i++) {
            if (buffer[i] == '\r' && i + 1u < count && buffer[i + 1u] == '\n')
                continue;
            buffer[normalized++] = buffer[i];
        }
        count = normalized;
        buffer[count] = '\0';
    }
    if (count != 0u && buffer[count - 1u] != '\n')
        return 0;
    return 1;
}

static int campaign_value(const char *text, const char *key, char *out, size_t capacity)
{
    size_t key_length = strlen(key);
    const char *line = text;
    size_t matches = 0u;
    while (*line != '\0') {
        const char *end = strchr(line, '\n');
        size_t length = end != NULL ? (size_t)(end - line) : strlen(line);
        if (length > key_length && line[key_length] == '=' && memcmp(line, key, key_length) == 0) {
            size_t value_length = length - key_length - 1u;
            if (value_length >= capacity)
                return 0;
            memcpy(out, line + key_length + 1u, value_length);
            out[value_length] = '\0';
            matches++;
        }
        if (end == NULL)
            break;
        line = end + 1;
    }
    return matches == 1u;
}

static int campaign_parse_u64(const char *text, uint64_t *out)
{
    uint64_t value = 0u;
    if (text == NULL || text[0] == '\0')
        return 0;
    for (size_t i = 0u; text[i] != '\0'; i++) {
        unsigned digit;
        if (text[i] < '0' || text[i] > '9')
            return 0;
        digit = (unsigned)(text[i] - '0');
        if (value > (UINT64_MAX - digit) / 10u)
            return 0;
        value = value * 10u + digit;
    }
    *out = value;
    return 1;
}

static int campaign_parse_bool(const char *text, int *out)
{
    if (text == NULL || (strcmp(text, "0") != 0 && strcmp(text, "1") != 0))
        return 0;
    *out = text[0] == '1';
    return 1;
}

static int campaign_parse_hex_byte(const char *text, uint8_t *out)
{
    unsigned value = 0u;
    if (text == NULL || strlen(text) != 4u || text[0] != '0' || text[1] != 'x')
        return 0;
    for (size_t i = 2u; i < 4u; i++) {
        unsigned digit;
        if (text[i] >= '0' && text[i] <= '9')
            digit = (unsigned)(text[i] - '0');
        else if (text[i] >= 'a' && text[i] <= 'f')
            digit = (unsigned)(text[i] - 'a') + 10u;
        else if (text[i] >= 'A' && text[i] <= 'F')
            digit = (unsigned)(text[i] - 'A') + 10u;
        else
            return 0;
        value = value * 16u + digit;
    }
    *out = (uint8_t)value;
    return 1;
}

static int campaign_strip_line_ending(char *line)
{
    char *end = strchr(line, '\n');
    if (end == NULL)
        return 0;
    if (end > line && end[-1] == '\r')
        end[-1] = '\0';
    else
        *end = '\0';
    return 1;
}

static int campaign_split(char *line, char **fields, size_t expected)
{
    size_t field = 0u;
    char *start = line;
    if (!campaign_strip_line_ending(line))
        return 0;
    for (char *cursor = line;; cursor++) {
        if (*cursor == '\t' || *cursor == '\0') {
            if (field >= expected)
                return 0;
            fields[field++] = start;
            if (*cursor == '\0')
                break;
            *cursor = '\0';
            start = cursor + 1;
        }
    }
    return field == expected;
}

static int campaign_parse_outcomes(const char *path,
                                   accuracycoin_outer_outcome outcomes[ACCURACYCOIN_PAGE_COUNT])
{
    FILE *file = fopen(path, "rb");
    char line[ACCURACYCOIN_CAMPAIGN_LINE_CAPACITY];
    if (file == NULL)
        return 0;
    if (fgets(line, sizeof line, file) == NULL || !campaign_strip_line_ending(line) ||
        strcmp(line, "page\toutcome\texit_code\tdeadline_seconds") != 0) {
        (void)fclose(file);
        return 0;
    }
    memset(outcomes, 0, sizeof(accuracycoin_outer_outcome) * ACCURACYCOIN_PAGE_COUNT);
    for (unsigned expected_page = 1u; expected_page <= ACCURACYCOIN_PAGE_COUNT; expected_page++) {
        char *fields[4];
        uint64_t page;
        uint64_t deadline;
        uint64_t exit_code;
        if (fgets(line, sizeof line, file) == NULL || !campaign_split(line, fields, 4u) ||
            !campaign_parse_u64(fields[0], &page) || !campaign_parse_u64(fields[3], &deadline) ||
            page != expected_page || deadline == 0u || deadline > UINT32_MAX ||
            (strcmp(fields[1], "EXIT_ZERO") != 0 && strcmp(fields[1], "CHILD_FAILURE") != 0 &&
             strcmp(fields[1], "OUTER_TIMEOUT") != 0) ||
            fields[2][0] == '\0' || strlen(fields[1]) >= sizeof outcomes[0].outcome ||
            strlen(fields[2]) >= sizeof outcomes[0].exit_code) {
            (void)fclose(file);
            return 0;
        }
        if (strcmp(fields[2], "-") != 0 &&
            (!campaign_parse_u64(fields[2], &exit_code) || exit_code > INT_MAX)) {
            (void)fclose(file);
            return 0;
        }
        if ((strcmp(fields[1], "EXIT_ZERO") == 0 && strcmp(fields[2], "0") != 0) ||
            (strcmp(fields[1], "OUTER_TIMEOUT") == 0 && strcmp(fields[2], "-") != 0)) {
            (void)fclose(file);
            return 0;
        }
        (void)snprintf(outcomes[expected_page - 1u].outcome,
                       sizeof outcomes[expected_page - 1u].outcome, "%s", fields[1]);
        (void)snprintf(outcomes[expected_page - 1u].exit_code,
                       sizeof outcomes[expected_page - 1u].exit_code, "%s", fields[2]);
        outcomes[expected_page - 1u].deadline_seconds = (uint32_t)deadline;
    }
    int invalid_tail = fgets(line, sizeof line, file) != NULL || ferror(file) != 0;
    if (fclose(file) != 0)
        invalid_tail = 1;
    if (invalid_tail != 0)
        return 0;
    return 1;
}

static int campaign_stage(const char *stage)
{
    static const char *const stages[ACCURACYCOIN_CAMPAIGN_STAGE_COUNT] = {
        "before-start", "a-press", "a-release", "observe", "final"};
    for (size_t i = 0u; i < ACCURACYCOIN_CAMPAIGN_STAGE_COUNT; i++)
        if (strcmp(stage, stages[i]) == 0)
            return (int)i;
    return -1;
}

static int campaign_valid_diagnostics(char *text, accuracycoin_controls_evidence *controls)
{
    static const char header[] = "stage\tframe\tmenu_ready\tpage_index\tcursor\tautomate_test_"
                                 "suite\trunning_all_tests\tresult_transition\n";
    char *line = text;
    uint64_t previous_frame = 0u;
    unsigned records = 0u;
    memset(controls, 0, sizeof *controls);
    if (strncmp(line, header, sizeof header - 1u) != 0)
        return 0;
    line += sizeof header - 1u;
    while (*line != '\0') {
        char *end = strchr(line, '\n');
        char *fields[8];
        uint64_t frame;
        uint64_t menu_ready;
        uint64_t page_index;
        uint64_t cursor;
        uint64_t automate;
        uint64_t running;
        int transition;
        if (end == NULL)
            return 0;
        if (!campaign_split(line, fields, 8u) ||
            (strcmp(fields[0], "boot") != 0 && strcmp(fields[0], "navigate") != 0 &&
             strcmp(fields[0], "a-press") != 0 && strcmp(fields[0], "a-release") != 0 &&
             strcmp(fields[0], "observe") != 0) ||
            !campaign_parse_u64(fields[1], &frame) || frame <= previous_frame ||
            !campaign_parse_u64(fields[2], &menu_ready) || menu_ready > UINT8_MAX ||
            !campaign_parse_u64(fields[3], &page_index) || page_index > UINT8_MAX ||
            !campaign_parse_u64(fields[4], &cursor) || cursor > UINT8_MAX ||
            !campaign_parse_u64(fields[5], &automate) || automate > 1u ||
            !campaign_parse_u64(fields[6], &running) || running > 1u ||
            !campaign_parse_bool(fields[7], &transition))
            return 0;
        if (strcmp(fields[0], "a-press") == 0) {
            if (controls->have_press_record != 0)
                return 0;
            controls->have_press_record = 1;
            controls->press_page_index = (uint8_t)page_index;
            controls->press_cursor = (uint8_t)cursor;
            controls->press_automate = (uint8_t)automate;
        }
        controls->last_menu_ready = (uint8_t)menu_ready;
        controls->last_page_index = (uint8_t)page_index;
        controls->last_cursor = (uint8_t)cursor;
        controls->last_automate = (uint8_t)automate;
        controls->last_running = (uint8_t)running;
        controls->saw_run_state |= automate != 0u || running != 0u;
        controls->saw_result_transition |= transition;
        previous_frame = frame;
        records++;
        line = end + 1;
    }
    return records != 0u && controls->have_press_record != 0;
}

static int campaign_path(char out[4096], const char *directory, const char *page_name,
                         const char *file_name)
{
    int result = snprintf(out, 4096u, "%s/%s/%s", directory, page_name, file_name);
    return result > 0 && (size_t)result < 4096u;
}

int accuracycoin_collect_campaign(const accuracycoin_directory *directory, const char *output_dir)
{
    accuracycoin_outer_outcome outcomes[ACCURACYCOIN_PAGE_COUNT];
    unsigned status_counts[5] = {0u, 0u, 0u, 0u, 0u};
    char path[4096];
    char outer_path[4096];
    char inventory_path[4096];
    char attempts_path[4096];
    char observations_path[4096];
    char report_path[4096];
    char summary_path[4096];
    char metadata_path[4096];
    FILE *inventory_file = NULL;
    FILE *attempts_file = NULL;
    FILE *observations_file = NULL;
    FILE *report_file = NULL;
    FILE *summary_file = NULL;
    FILE *metadata_file = NULL;
    char campaign_source[64] = "";
    char campaign_dirty[8] = "";
    char campaign_library[32] = "";
    char campaign_behaviour[32] = "";
    char campaign_policy[192] = "";
    uint32_t campaign_boot_cap = 0u;
    uint32_t campaign_navigation_cap = 0u;
    uint32_t campaign_execution_cap = 0u;
    uint64_t campaign_total_cap = 0u;
    uint64_t campaign_host_elapsed_ns = 0u;
    unsigned attempted_pages = 0u;
    unsigned attempted_scored_rows = 0u;
    unsigned observed_rows = 0u;
    unsigned terminal_rows = 0u;
    unsigned draw_rows = 0u;
    int have_identity = 0;
    int infrastructure_failed = 0;
    int evidence_invalid = 0;
    char error[160] = "invalid campaign evidence";
    if (directory == NULL || output_dir == NULL || output_dir[0] == '\0')
        return RUNNER_EXIT_FAILURE;
    if (!output_path(outer_path, output_dir, "outer-outcomes.tsv") ||
        !output_path(inventory_path, output_dir, "inventory.tsv") ||
        !output_path(attempts_path, output_dir, "attempts.tsv") ||
        !output_path(observations_path, output_dir, "observations.tsv") ||
        !output_path(report_path, output_dir, "report.tsv") ||
        !output_path(summary_path, output_dir, "summary.txt") ||
        !output_path(metadata_path, output_dir, "metadata.txt") ||
        !campaign_parse_outcomes(outer_path, outcomes)) {
        fprintf(stderr,
                "nesturbator-run: AccuracyCoin campaign has missing or malformed outer outcomes\n");
        return RUNNER_EXIT_FAILURE;
    }
    inventory_file = fopen(inventory_path, "w");
    attempts_file = fopen(attempts_path, "w");
    observations_file = fopen(observations_path, "w");
    report_file = fopen(report_path, "w");
    summary_file = fopen(summary_path, "w");
    metadata_file = fopen(metadata_path, "w");
    if (inventory_file == NULL || attempts_file == NULL || observations_file == NULL ||
        report_file == NULL || summary_file == NULL || metadata_file == NULL ||
        !accuracycoin_write_inventory(inventory_file, directory) ||
        fprintf(attempts_file,
                "attempt_id\torigin\trequested_page\trequested_row\tprocess_outcome\texit_"
                "code\tselection_observed\tselected_page\tselected_cursor\tstart_"
                "attributed\tstartup_dmc_pretest\tstartup_vblank_pretest\tstartup_ppu_reset_"
                "pretest\tboot_frames_used\tnavigation_frames_used\tstart_frames_used\texecution_"
                "frames\tframes_used\tstop_cause\tactive_key\tactive_raw\tevidence_complete\n") <
            0 ||
        fprintf(observations_file,
                "attempt_id\tstage\tpage\trow\tkey\tresult_address\tobserved\traw\n") < 0 ||
        fprintf(report_file,
                "key\tpage\trow\tkind\tstatus\tobserved\traw\tframes\tcause\tattempt_id\n") < 0) {
        (void)snprintf(error, sizeof error, "cannot open AccuracyCoin campaign outputs");
        evidence_invalid = 1;
        goto collect_done;
    }

    {
        FILE *outer_file = fopen(outer_path, "rb");
        char line[ACCURACYCOIN_CAMPAIGN_LINE_CAPACITY];
        if (outer_file == NULL || fgets(line, sizeof line, outer_file) == NULL ||
            !campaign_strip_line_ending(line) ||
            strcmp(line, "page\toutcome\texit_code\tdeadline_seconds") != 0) {
            if (outer_file != NULL)
                (void)fclose(outer_file);
            (void)snprintf(error, sizeof error, "cannot re-read outer outcome table");
            evidence_invalid = 1;
            goto collect_done;
        }
        (void)fclose(outer_file);
    }

    for (uint32_t page_number = 1u; page_number <= ACCURACYCOIN_PAGE_COUNT; page_number++) {
        char page_dir[32];
        char raw_path[4096];
        char diag_path[4096];
        char page_report_path[4096];
        char page_metadata_path[4096];
        char metadata[ACCURACYCOIN_CAMPAIGN_TEXT_CAPACITY];
        char attempt_id[64];
        char stop_cause[64] = "outer-failure";
        char verdict[40] = "incomplete";
        char origin[32] = "cold-page";
        char source_revision[64] = "";
        char source_dirty[8] = "";
        char library_version[32] = "";
        char behaviour_revision[32] = "";
        char policy[192] = "";
        char active_key[ACCURACYCOIN_NAME_CAPACITY + 16u] = "-";
        char active_raw[8] = "-";
        char value[256];
        uint64_t boot_cap = 0u;
        uint64_t navigation_cap = 0u;
        uint64_t execution_cap = 0u;
        uint64_t total_cap = 0u;
        uint64_t boot_used = 0u;
        uint64_t navigation_used = 0u;
        uint64_t start_used = 0u;
        uint64_t execution_used = 0u;
        uint64_t total_used = 0u;
        uint64_t host_elapsed = 0u;
        int metadata_present = 0;
        int raw_present = 0;
        int diagnostics_present = 0;
        int page_report_present = 0;
        int selection_observed = 0;
        int selected_page = 0xff;
        int selected_cursor = 0xff;
        int draw_cursor_transition_observed = 0;
        int menu_returned_to_header_observed = 0;
        int draw_only_page = 1;
        uint64_t parsed_value = 0u;
        int start_attributed = 0;
        int a_press = 0;
        int a_release = 0;
        int release_frames = 0;
        int run_state = 0;
        int transition = 0;
        int pretest_dmc_observed = 0;
        int pretest_vblank_observed = 0;
        int pretest_ppu_observed = 0;
        uint8_t pretest_dmc = 0u;
        uint8_t pretest_vblank = 0u;
        uint8_t pretest_ppu = 0u;
        uint8_t final_raw[ACCURACYCOIN_MAX_PAGE_ROWS];
        uint8_t final_observed[ACCURACYCOIN_MAX_PAGE_ROWS];
        uint8_t final_seen[ACCURACYCOIN_MAX_PAGE_ROWS];
        uint8_t stage_seen[ACCURACYCOIN_CAMPAIGN_STAGE_COUNT][ACCURACYCOIN_MAX_PAGE_ROWS];
        uint8_t last_row_by_stage[ACCURACYCOIN_CAMPAIGN_STAGE_COUNT] = {0u};
        unsigned stage_counts[ACCURACYCOIN_CAMPAIGN_STAGE_COUNT] = {0u};
        int start_evidence_complete = 0;
        int final_snapshot_complete = 0;
        int page_metadata_valid = 0;
        int page_report_valid = 0;
        int page_complete = 0;
        accuracycoin_controls_evidence controls_evidence;
        const accuracycoin_page *page = &directory->pages[page_number - 1u];
        size_t first_row = page->first_row;
        size_t row_count = page->row_count;
        accuracycoin_outer_outcome *outer = &outcomes[page_number - 1u];
        memset(&controls_evidence, 0, sizeof controls_evidence);
        for (size_t i = 0u; i < row_count; i++)
            if (directory->rows[first_row + i].kind == (uint8_t)ACCURACYCOIN_KIND_TEST)
                draw_only_page = 0;
        memset(final_raw, 0, sizeof final_raw);
        memset(final_observed, 0, sizeof final_observed);
        memset(final_seen, 0, sizeof final_seen);
        memset(stage_seen, 0, sizeof stage_seen);
        (void)snprintf(page_dir, sizeof page_dir, "page-%02u", (unsigned)page_number);
        (void)snprintf(attempt_id, sizeof attempt_id, "page-%02u-cold-01", (unsigned)page_number);
        if (!campaign_path(raw_path, output_dir, page_dir, "raw.tsv") ||
            !campaign_path(diag_path, output_dir, page_dir, "diagnostics.tsv") ||
            !campaign_path(page_report_path, output_dir, page_dir, "report.tsv") ||
            !campaign_path(page_metadata_path, output_dir, page_dir, "metadata.txt")) {
            evidence_invalid = 1;
            (void)snprintf(error, sizeof error, "campaign page path is too long");
            goto collect_done;
        }

        if (strcmp(outer->outcome, "EXIT_ZERO") != 0)
            infrastructure_failed = 1;
        if (!campaign_read_text(page_metadata_path, metadata, sizeof metadata, 0,
                                &metadata_present)) {
            evidence_invalid = 1;
            (void)snprintf(error, sizeof error, "page %u metadata is unreadable or truncated",
                           (unsigned)page_number);
            metadata_present = 0;
        }
        if (metadata_present != 0) {
#define META_STRING(KEY, DEST)                                                                     \
    do {                                                                                           \
        (DEST)[0] = '\0';                                                                          \
        if (!campaign_value(metadata, KEY, DEST, sizeof DEST))                                     \
            page_metadata_valid = 0;                                                               \
    } while (0)
            page_metadata_valid = 1;
            META_STRING("report_schema", value);
            if (strcmp(value, "1") != 0)
                page_metadata_valid = 0;
            META_STRING("inventory_schema", value);
            if (strcmp(value, "1") != 0)
                page_metadata_valid = 0;
            META_STRING("source_revision", source_revision);
            META_STRING("source_dirty", source_dirty);
            META_STRING("library_version", library_version);
            META_STRING("behaviour_revision", behaviour_revision);
            META_STRING("input_start_policy", policy);
            META_STRING("stop_cause", stop_cause);
            META_STRING("verdict", verdict);
            if (strcmp(verdict, "complete") != 0 && strcmp(verdict, "guest-timeout") != 0 &&
                strcmp(verdict, "bounded-incomplete") != 0 &&
                strcmp(verdict, "startup-timeout") != 0 && strcmp(verdict, "driver-failure") != 0 &&
                strcmp(verdict, "guest-failure") != 0 && strcmp(verdict, "output-failure") != 0 &&
                strcmp(verdict, "startup-failure") != 0)
                page_metadata_valid = 0;
            META_STRING("attempt_id", value);
            if (strcmp(value, attempt_id) != 0)
                page_metadata_valid = 0;
            META_STRING("origin", value);
            if (strcmp(value, "cold-page") != 0)
                page_metadata_valid = 0;
            META_STRING("requested_row", value);
            if (strcmp(value, "all") != 0)
                page_metadata_valid = 0;
            META_STRING("rom_pin", value);
            if (strcmp(value, directory->pin) != 0)
                page_metadata_valid = 0;
            META_STRING("rom_sha256", value);
            if (strcmp(value, directory->rom_sha256) != 0)
                page_metadata_valid = 0;
            META_STRING("inventory_sha256", value);
            if (strcmp(value, directory->inventory_sha256) != 0)
                page_metadata_valid = 0;
            META_STRING("mode", value);
            if (strcmp(value, "observe") != 0)
                page_metadata_valid = 0;
            META_STRING("page", value);
            if (!campaign_parse_u64(value, &parsed_value) || parsed_value != page_number)
                page_metadata_valid = 0;
            META_STRING("boot_frame_budget", value);
            if (!campaign_parse_u64(value, &boot_cap) || boot_cap > UINT32_MAX)
                page_metadata_valid = 0;
            META_STRING("navigation_frame_budget_per_page", value);
            if (!campaign_parse_u64(value, &navigation_cap) || navigation_cap > UINT32_MAX)
                page_metadata_valid = 0;
            META_STRING("execution_frame_budget", value);
            if (!campaign_parse_u64(value, &execution_cap) || execution_cap > UINT32_MAX)
                page_metadata_valid = 0;
            META_STRING("total_frame_budget", value);
            if (!campaign_parse_u64(value, &total_cap))
                page_metadata_valid = 0;
            {
                uint64_t expected_total = boot_cap;
                uint64_t nav_bound = navigation_cap * (uint64_t)(page_number - 1u);
                if (UINT64_MAX - expected_total < nav_bound)
                    page_metadata_valid = 0;
                else
                    expected_total += nav_bound;
                if (UINT64_MAX - expected_total < 2u)
                    page_metadata_valid = 0;
                else
                    expected_total += 2u;
                if (UINT64_MAX - expected_total < execution_cap)
                    page_metadata_valid = 0;
                else
                    expected_total += execution_cap;
                if (total_cap != expected_total)
                    page_metadata_valid = 0;
            }
            META_STRING("boot_frames_used", value);
            if (!campaign_parse_u64(value, &boot_used) || boot_used > boot_cap)
                page_metadata_valid = 0;
            META_STRING("navigation_frames_used", value);
            if (!campaign_parse_u64(value, &navigation_used) ||
                navigation_used > navigation_cap * (uint64_t)(page_number - 1u))
                page_metadata_valid = 0;
            META_STRING("start_frames_used", value);
            if (!campaign_parse_u64(value, &start_used) || start_used > 2u)
                page_metadata_valid = 0;
            META_STRING("execution_frames", value);
            if (!campaign_parse_u64(value, &execution_used) || execution_used > execution_cap)
                page_metadata_valid = 0;
            META_STRING("frames_used", value);
            if (!campaign_parse_u64(value, &total_used) || total_used > total_cap ||
                UINT64_MAX - boot_used < navigation_used ||
                UINT64_MAX - (boot_used + navigation_used) < start_used ||
                UINT64_MAX - (boot_used + navigation_used + start_used) < execution_used ||
                total_used != boot_used + navigation_used + start_used + execution_used)
                page_metadata_valid = 0;
            META_STRING("host_elapsed_ns", value);
            if (!campaign_parse_u64(value, &host_elapsed))
                page_metadata_valid = 0;
            META_STRING("selection_observed", value);
            if (!campaign_parse_bool(value, &selection_observed))
                page_metadata_valid = 0;
            META_STRING("selected_page", value);
            if (!campaign_parse_u64(value, &parsed_value) || parsed_value > UINT8_MAX)
                page_metadata_valid = 0;
            selected_page = (int)parsed_value;
            META_STRING("selected_cursor", value);
            if (!campaign_parse_u64(value, &parsed_value) || parsed_value > UINT8_MAX)
                page_metadata_valid = 0;
            selected_cursor = (int)parsed_value;
            META_STRING("start_attributed", value);
            if (!campaign_parse_bool(value, &start_attributed))
                page_metadata_valid = 0;
            META_STRING("a_press_observed", value);
            if (!campaign_parse_bool(value, &a_press))
                page_metadata_valid = 0;
            META_STRING("a_release_observed", value);
            if (!campaign_parse_bool(value, &a_release))
                page_metadata_valid = 0;
            META_STRING("release_frames", value);
            if (!campaign_parse_u64(value, &parsed_value) || parsed_value > INT_MAX)
                page_metadata_valid = 0;
            release_frames = (int)parsed_value;
            META_STRING("run_state_observed", value);
            if (!campaign_parse_bool(value, &run_state))
                page_metadata_valid = 0;
            META_STRING("result_transition_observed", value);
            if (!campaign_parse_bool(value, &transition))
                page_metadata_valid = 0;
            META_STRING("draw_cursor_transition_observed", value);
            if (!campaign_parse_bool(value, &draw_cursor_transition_observed))
                page_metadata_valid = 0;
            META_STRING("menu_returned_to_header_observed", value);
            if (!campaign_parse_bool(value, &menu_returned_to_header_observed))
                page_metadata_valid = 0;
            META_STRING("startup_dmc_pretest_observed", value);
            if (!campaign_parse_bool(value, &pretest_dmc_observed))
                page_metadata_valid = 0;
            META_STRING("startup_dmc_pretest", value);
            if (!campaign_parse_hex_byte(value, &pretest_dmc))
                page_metadata_valid = 0;
            META_STRING("startup_vblank_pretest_observed", value);
            if (!campaign_parse_bool(value, &pretest_vblank_observed))
                page_metadata_valid = 0;
            META_STRING("startup_vblank_pretest", value);
            if (!campaign_parse_hex_byte(value, &pretest_vblank))
                page_metadata_valid = 0;
            META_STRING("startup_ppu_reset_pretest_observed", value);
            if (!campaign_parse_bool(value, &pretest_ppu_observed))
                page_metadata_valid = 0;
            META_STRING("startup_ppu_reset_pretest", value);
            if (!campaign_parse_hex_byte(value, &pretest_ppu))
                page_metadata_valid = 0;
            META_STRING("active_key", active_key);
            META_STRING("active_raw", active_raw);
            if (source_revision[0] == '\0' || strcmp(source_revision, "unknown") == 0 ||
                (strcmp(source_dirty, "0") != 0 && strcmp(source_dirty, "1") != 0) ||
                library_version[0] == '\0' || behaviour_revision[0] == '\0' ||
                strlen(policy) == 0u ||
                strcmp(policy, "cold-boot;page-header-A;one-frame-A-press;one-frame-release") != 0)
                page_metadata_valid = 0;
            if (page_metadata_valid != 0) {
                if (have_identity == 0) {
                    (void)snprintf(campaign_source, sizeof campaign_source, "%s", source_revision);
                    (void)snprintf(campaign_dirty, sizeof campaign_dirty, "%s", source_dirty);
                    (void)snprintf(campaign_library, sizeof campaign_library, "%s",
                                   library_version);
                    (void)snprintf(campaign_behaviour, sizeof campaign_behaviour, "%s",
                                   behaviour_revision);
                    (void)snprintf(campaign_policy, sizeof campaign_policy, "%s", policy);
                    campaign_boot_cap = (uint32_t)boot_cap;
                    campaign_navigation_cap = (uint32_t)navigation_cap;
                    campaign_execution_cap = (uint32_t)execution_cap;
                    have_identity = 1;
                } else if (strcmp(campaign_source, source_revision) != 0 ||
                           strcmp(campaign_dirty, source_dirty) != 0 ||
                           strcmp(campaign_library, library_version) != 0 ||
                           strcmp(campaign_behaviour, behaviour_revision) != 0 ||
                           strcmp(campaign_policy, policy) != 0 || campaign_boot_cap != boot_cap ||
                           campaign_navigation_cap != navigation_cap ||
                           campaign_execution_cap != execution_cap) {
                    page_metadata_valid = 0;
                }
            }
            if (page_metadata_valid == 0) {
                evidence_invalid = 1;
                (void)snprintf(error, sizeof error,
                               "page %u metadata is incompatible with campaign identity",
                               (unsigned)page_number);
            }
            if (page_metadata_valid != 0) {
                if (UINT64_MAX - campaign_total_cap < total_cap) {
                    evidence_invalid = 1;
                    (void)snprintf(error, sizeof error,
                                   "page %u aggregate guest-frame cap overflows",
                                   (unsigned)page_number);
                } else {
                    campaign_total_cap += total_cap;
                }
            }
            if (page_metadata_valid != 0 && host_elapsed <= UINT64_MAX - campaign_host_elapsed_ns)
                campaign_host_elapsed_ns += host_elapsed;
            else if (page_metadata_valid != 0) {
                evidence_invalid = 1;
                (void)snprintf(error, sizeof error, "page %u elapsed-time total overflows",
                               (unsigned)page_number);
            }
#undef META_STRING
        }

        if (!campaign_read_text(raw_path, metadata, sizeof metadata, 0, &raw_present)) {
            evidence_invalid = 1;
            (void)snprintf(error, sizeof error, "page %u raw samples are unreadable or truncated",
                           (unsigned)page_number);
            raw_present = 0;
        }
        if (raw_present != 0) {
            char *line = metadata;
            int first = 1;
            while (*line != '\0') {
                char *end = strchr(line, '\n');
                char *fields[7];
                uint64_t parsed_page;
                uint64_t parsed_row;
                uint64_t parsed_address;
                uint8_t raw_byte = 0u;
                int observed;
                int stage;
                size_t local_index;
                if (end == NULL) {
                    evidence_invalid = 1;
                    (void)snprintf(error, sizeof error, "page %u raw sample is truncated",
                                   (unsigned)page_number);
                    break;
                }
                if (first != 0) {
                    static const char raw_header[] =
                        "stage\tpage\trow\tkey\tresult_address\tobserved\traw\n";
                    size_t header_length = sizeof raw_header - 1u;
                    if ((size_t)(end - line) + 1u != header_length ||
                        memcmp(line, raw_header, header_length) != 0) {
                        evidence_invalid = 1;
                        (void)snprintf(error, sizeof error, "page %u raw sample header is invalid",
                                       (unsigned)page_number);
                    }
                    first = 0;
                    line = end + 1;
                    continue;
                }
                if (!campaign_split(line, fields, 7u) ||
                    !campaign_parse_u64(fields[1], &parsed_page) ||
                    !campaign_parse_u64(fields[2], &parsed_row) || parsed_page != page_number ||
                    parsed_row == 0u || parsed_row > row_count) {
                    evidence_invalid = 1;
                    (void)snprintf(error, sizeof error,
                                   "page %u raw row is malformed or outside inventory order",
                                   (unsigned)page_number);
                    break;
                }
                stage = campaign_stage(fields[0]);
                if (stage < 0) {
                    evidence_invalid = 1;
                    (void)snprintf(error, sizeof error, "page %u raw stage is unsupported",
                                   (unsigned)page_number);
                    break;
                }
                local_index = (size_t)parsed_row - 1u;
                const accuracycoin_test *row = &directory->rows[first_row + local_index];
                char expected_key[ACCURACYCOIN_NAME_CAPACITY + 16u];
                char expected_address[16];
                (void)snprintf(expected_key, sizeof expected_key, "accuracycoin/%s", row->name);
                (void)snprintf(expected_address, sizeof expected_address, "0x%04x",
                               (unsigned)row->result_address);
                if (strcmp(fields[3], expected_key) != 0 ||
                    strcmp(fields[4], expected_address) != 0 ||
                    !campaign_parse_u64(fields[5], &parsed_address) || parsed_address > 1u ||
                    (strcmp(fields[5], "0") != 0 && strcmp(fields[5], "1") != 0)) {
                    evidence_invalid = 1;
                    (void)snprintf(error, sizeof error,
                                   "page %u raw row key, address or observation flag differs",
                                   (unsigned)page_number);
                    break;
                }
                observed = strcmp(fields[5], "1") == 0;
                if (row->kind == (uint8_t)ACCURACYCOIN_KIND_DRAW) {
                    if (observed != 0 || strcmp(fields[6], "-") != 0) {
                        evidence_invalid = 1;
                        (void)snprintf(error, sizeof error,
                                       "page %u DRAW row contains scored raw data",
                                       (unsigned)page_number);
                        break;
                    }
                } else if (observed != 0) {
                    if (!campaign_parse_hex_byte(fields[6], &raw_byte)) {
                        evidence_invalid = 1;
                        (void)snprintf(error, sizeof error, "page %u raw byte is malformed",
                                       (unsigned)page_number);
                        break;
                    }
                } else if (strcmp(fields[6], "-") != 0) {
                    evidence_invalid = 1;
                    (void)snprintf(error, sizeof error, "page %u unobserved row invents a raw byte",
                                   (unsigned)page_number);
                    break;
                }
                if (fprintf(observations_file, "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", attempt_id,
                            fields[0], fields[1], fields[2], fields[3], fields[4], fields[5],
                            fields[6]) < 0) {
                    evidence_invalid = 1;
                    break;
                }
                if (parsed_row != (uint64_t)last_row_by_stage[stage] + 1u) {
                    evidence_invalid = 1;
                    (void)snprintf(error, sizeof error, "page %u raw rows are out of source order",
                                   (unsigned)page_number);
                    break;
                }
                if (stage_seen[stage][local_index] != 0u) {
                    evidence_invalid = 1;
                    (void)snprintf(error, sizeof error, "page %u repeats a raw row stage",
                                   (unsigned)page_number);
                    break;
                }
                stage_seen[stage][local_index] = 1u;
                last_row_by_stage[stage] = (uint8_t)parsed_row;
                stage_counts[stage]++;
                if (stage == 4) {
                    final_seen[local_index] = 1u;
                    final_observed[local_index] = (uint8_t)observed;
                    final_raw[local_index] = raw_byte;
                }
                line = end + 1;
            }
            if (first != 0) {
                evidence_invalid = 1;
                (void)snprintf(error, sizeof error, "page %u raw samples are empty",
                               (unsigned)page_number);
            }
            for (size_t stage_index = 0u; stage_index < ACCURACYCOIN_CAMPAIGN_STAGE_COUNT;
                 stage_index++) {
                if (stage_counts[stage_index] != 0u && stage_counts[stage_index] != row_count) {
                    evidence_invalid = 1;
                    (void)snprintf(error, sizeof error, "page %u raw stage is incomplete",
                                   (unsigned)page_number);
                }
            }
            final_snapshot_complete = stage_counts[4] == row_count;
        }
        if (page_metadata_valid != 0 && start_attributed != 0) {
            if (stage_counts[0] != row_count || stage_counts[1] != row_count ||
                stage_counts[2] != row_count || start_used != 2u) {
                evidence_invalid = 1;
                (void)snprintf(error, sizeof error,
                               "page %u lacks complete press/release snapshots",
                               (unsigned)page_number);
            }
        }
        if (page_metadata_valid != 0 && strcmp(verdict, "startup-timeout") == 0 &&
            (start_attributed != 0 || boot_used != boot_cap || selection_observed != 0 ||
             a_press != 0 || a_release != 0)) {
            evidence_invalid = 1;
            (void)snprintf(error, sizeof error, "page %u startup timeout has later-stage evidence",
                           (unsigned)page_number);
        }
        if (!campaign_read_text(diag_path, metadata, sizeof metadata, 0, &diagnostics_present)) {
            evidence_invalid = 1;
            diagnostics_present = 0;
            (void)snprintf(error, sizeof error, "page %u diagnostics are unreadable or truncated",
                           (unsigned)page_number);
        }
        if (diagnostics_present != 0 && !campaign_valid_diagnostics(metadata, &controls_evidence)) {
            evidence_invalid = 1;
            diagnostics_present = 0;
            (void)snprintf(error, sizeof error, "page %u diagnostics are malformed",
                           (unsigned)page_number);
        }
        if (page_metadata_valid != 0 && strcmp(outer->outcome, "EXIT_ZERO") == 0 &&
            (diagnostics_present == 0 || raw_present == 0)) {
            evidence_invalid = 1;
            (void)snprintf(error, sizeof error, "page %u omitted raw or diagnostic evidence",
                           (unsigned)page_number);
        }
        if (page_metadata_valid != 0 && diagnostics_present != 0) {
            int expected_draw_cursor_transition =
                draw_only_page != 0 && selected_cursor == 0xff &&
                controls_evidence.press_page_index == page_number - 1u &&
                controls_evidence.press_cursor == 0u && controls_evidence.press_automate != 0u;
            int expected_menu_return = controls_evidence.last_menu_ready == 0x0au &&
                                       controls_evidence.last_page_index == page_number - 1u &&
                                       controls_evidence.last_cursor == 0xffu &&
                                       controls_evidence.last_automate == 0u;
            if ((int)controls_evidence.saw_run_state != run_state ||
                (int)controls_evidence.saw_result_transition != transition ||
                controls_evidence.press_page_index != page_number - 1u ||
                draw_cursor_transition_observed != expected_draw_cursor_transition ||
                menu_returned_to_header_observed != expected_menu_return) {
                evidence_invalid = 1;
                (void)snprintf(error, sizeof error,
                               "page %u control diagnostics conflict with metadata",
                               (unsigned)page_number);
            }
            if (page_metadata_valid != 0 && start_attributed != 0) {
                if (!selection_observed || selected_page != (int)(page_number - 1u) ||
                    selected_cursor != 0xff || !a_press || !a_release || release_frames != 1 ||
                    !run_state ||
                    (draw_only_page != 0 ? !draw_cursor_transition_observed : !transition) ||
                    !pretest_dmc_observed || !pretest_vblank_observed || !pretest_ppu_observed) {
                    evidence_invalid = 1;
                    (void)snprintf(error, sizeof error,
                                   "page %u has unattributed menu/start evidence",
                                   (unsigned)page_number);
                } else {
                    start_evidence_complete = 1;
                }
            }
        }
        if (page_metadata_valid != 0 && final_snapshot_complete != 0) {
            unsigned active_rows = 0u;
            const accuracycoin_test *cursor_row = NULL;
            for (size_t i = 0u; i < row_count; i++) {
                const accuracycoin_test *row = &directory->rows[first_row + i];
                if (row->kind == (uint8_t)ACCURACYCOIN_KIND_TEST && final_observed[i] != 0u &&
                    final_raw[i] == 0x03u)
                    active_rows++;
                if (i == controls_evidence.last_cursor)
                    cursor_row = row;
            }
            if (strcmp(verdict, "guest-timeout") == 0) {
                char current_key[ACCURACYCOIN_NAME_CAPACITY + 16u] = "-";
                if (cursor_row != NULL && cursor_row->kind == (uint8_t)ACCURACYCOIN_KIND_TEST)
                    (void)snprintf(current_key, sizeof current_key, "accuracycoin/%s",
                                   cursor_row->name);
                if (active_rows == 0u || cursor_row == NULL ||
                    cursor_row->kind != (uint8_t)ACCURACYCOIN_KIND_TEST ||
                    final_observed[controls_evidence.last_cursor] == 0u ||
                    final_raw[controls_evidence.last_cursor] != 0x03u ||
                    controls_evidence.last_automate != 1u || strcmp(active_key, current_key) != 0 ||
                    strcmp(active_raw, "0x03") != 0 ||
                    strcmp(stop_cause, "execution-cap-active-test") != 0 ||
                    execution_used != execution_cap) {
                    evidence_invalid = 1;
                    (void)snprintf(error, sizeof error,
                                   "page %u timeout lacks current-row automation evidence",
                                   (unsigned)page_number);
                }
            } else if (active_rows != 0u || strcmp(active_key, "-") != 0 ||
                       strcmp(active_raw, "-") != 0) {
                evidence_invalid = 1;
                (void)snprintf(error, sizeof error,
                               "page %u has active identity without guest timeout",
                               (unsigned)page_number);
            }
        }

        if (!campaign_read_text(page_report_path, metadata, sizeof metadata, 0,
                                &page_report_present)) {
            evidence_invalid = 1;
            page_report_present = 0;
            (void)snprintf(error, sizeof error, "page %u report is unreadable or truncated",
                           (unsigned)page_number);
        }
        if (page_metadata_valid != 0 && start_attributed != 0 && final_snapshot_complete != 0 &&
            page_report_present != 0) {
            FILE *child_report = fopen(page_report_path, "rb");
            char line[ACCURACYCOIN_CAMPAIGN_LINE_CAPACITY];
            int valid = child_report != NULL;
            if (valid != 0 &&
                (fgets(line, sizeof line, child_report) == NULL ||
                 !campaign_strip_line_ending(line) ||
                 strcmp(line, "key\tpage\trow\tstatus\tobserved\traw\tframes\tcause\tattempt_id") !=
                     0))
                valid = 0;
            for (size_t i = 0u; valid != 0 && i < row_count; i++) {
                char expected[ACCURACYCOIN_CAMPAIGN_LINE_CAPACITY];
                char raw_text[5];
                const char *classified_cause = NULL;
                const accuracycoin_test *row = &directory->rows[first_row + i];
                int observed = row->kind == (uint8_t)ACCURACYCOIN_KIND_TEST;
                accuracycoin_result_status result = accuracycoin_classify_final_row(
                    row, observed, final_raw[i],
                    strcmp(verdict, "guest-timeout") == 0 && i == controls_evidence.last_cursor,
                    &classified_cause);
                if (observed != 0)
                    (void)snprintf(raw_text, sizeof raw_text, "0x%02x", (unsigned)final_raw[i]);
                else
                    (void)snprintf(raw_text, sizeof raw_text, "-");
                (void)snprintf(expected, sizeof expected,
                               "accuracycoin/%s\t%u\t%u\t%s\t%d\t%s\t%llu\t%s\t%s\n", row->name,
                               (unsigned)page_number, (unsigned)row->row,
                               accuracycoin_status_text(result), observed, raw_text,
                               (unsigned long long)execution_used, classified_cause, attempt_id);
                if (result == ACCURACYCOIN_RESULT_INVALID) {
                    fprintf(stderr,
                            "nesturbator-run: AccuracyCoin page %u has invalid row %s (%s)\n",
                            (unsigned)page_number, row->name,
                            classified_cause != NULL ? classified_cause : "invalid-result");
                    valid = 0;
                    break;
                }
                if (fgets(line, sizeof line, child_report) == NULL ||
                    !campaign_strip_line_ending(line)) {
                    fprintf(stderr,
                            "nesturbator-run: AccuracyCoin page %u report ended before row %lu\n",
                            (unsigned)page_number, (unsigned long)i + 1u);
                    valid = 0;
                    break;
                }
                if (!campaign_strip_line_ending(expected) || strcmp(line, expected) != 0) {
                    fprintf(stderr,
                            "nesturbator-run: AccuracyCoin page %u report row %lu differs from "
                            "final bytes\nexpected: %sactual: %s",
                            (unsigned)page_number, (unsigned long)i + 1u, expected, line);
                    valid = 0;
                    break;
                }
            }
            if (valid != 0 && fgets(line, sizeof line, child_report) != NULL)
                valid = 0;
            if (child_report != NULL && (ferror(child_report) != 0 || fclose(child_report) != 0))
                valid = 0;
            page_report_valid = valid;
            if (valid == 0) {
                evidence_invalid = 1;
                (void)snprintf(error, sizeof error, "page %u report does not match final raw bytes",
                               (unsigned)page_number);
            }
        }

        if (page_metadata_valid != 0 && strcmp(outer->outcome, "EXIT_ZERO") == 0) {
            if (strcmp(verdict, "startup-timeout") == 0 &&
                strcmp(stop_cause, "boot-cap-exhausted") == 0 && start_attributed == 0) {
                page_complete = 1;
            } else if ((strcmp(verdict, "complete") == 0 || strcmp(verdict, "guest-timeout") == 0 ||
                        strcmp(verdict, "bounded-incomplete") == 0) &&
                       start_evidence_complete != 0 && final_snapshot_complete != 0 &&
                       page_report_valid != 0 &&
                       (strcmp(verdict, "complete") != 0 ||
                        menu_returned_to_header_observed != 0)) {
                page_complete = 1;
            } else {
                evidence_invalid = 1;
                (void)snprintf(error, sizeof error,
                               "page %u successful child lacks terminal evidence (start=%d "
                               "final=%d report=%d)",
                               (unsigned)page_number, start_evidence_complete,
                               final_snapshot_complete, page_report_valid);
            }
        }
        if (page_complete == 0)
            infrastructure_failed = 1;
        if (start_attributed != 0)
            attempted_pages++;
        if (start_attributed != 0)
            for (size_t i = 0u; i < row_count; i++)
                if (directory->rows[first_row + i].kind == (uint8_t)ACCURACYCOIN_KIND_TEST)
                    attempted_scored_rows++;

        (void)snprintf(path, sizeof path, "%s", attempt_id);
        char pretest_dmc_text[5] = "-";
        char pretest_vblank_text[5] = "-";
        char pretest_ppu_text[5] = "-";
        if (pretest_dmc_observed != 0)
            (void)snprintf(pretest_dmc_text, sizeof pretest_dmc_text, "0x%02x",
                           (unsigned)pretest_dmc);
        if (pretest_vblank_observed != 0)
            (void)snprintf(pretest_vblank_text, sizeof pretest_vblank_text, "0x%02x",
                           (unsigned)pretest_vblank);
        if (pretest_ppu_observed != 0)
            (void)snprintf(pretest_ppu_text, sizeof pretest_ppu_text, "0x%02x",
                           (unsigned)pretest_ppu);
        if (fprintf(attempts_file,
                    "%s\t%s\t%u\tall\t%s\t%s\t%d\t%d\t%d\t%d\t%s\t%s\t%s\t%llu\t%llu\t%llu\t%llu\t%"
                    "llu\t%s\t%s\t%s\t%d\n",
                    path, origin, (unsigned)page_number, outer->outcome, outer->exit_code,
                    selection_observed, selected_page, selected_cursor, start_attributed,
                    pretest_dmc_text, pretest_vblank_text, pretest_ppu_text,
                    (unsigned long long)boot_used, (unsigned long long)navigation_used,
                    (unsigned long long)start_used, (unsigned long long)execution_used,
                    (unsigned long long)total_used, stop_cause, active_key, active_raw,
                    page_complete) < 0) {
            evidence_invalid = 1;
            (void)snprintf(error, sizeof error, "cannot write attempts table");
            goto collect_done;
        }

        for (size_t local_index = 0u; local_index < row_count; local_index++) {
            size_t global_index = first_row + local_index;
            const accuracycoin_test *row = &directory->rows[global_index];
            const char *cause = NULL;
            int observed = row->kind == (uint8_t)ACCURACYCOIN_KIND_TEST &&
                           final_seen[local_index] != 0u && final_observed[local_index] != 0u;
            uint8_t raw = final_raw[local_index];
            accuracycoin_result_status result;
            uint64_t frames_for_row = execution_used;
            const char *row_attempt = attempt_id;
            if (row->kind == (uint8_t)ACCURACYCOIN_KIND_DRAW) {
                result = ACCURACYCOIN_RESULT_DRAW;
                cause = "inventory-only-draw";
                draw_rows++;
            } else if (page_report_valid != 0 && final_seen[local_index] != 0u &&
                       final_observed[local_index] != 0u && page_metadata_valid != 0) {
                result = accuracycoin_classify_final_row(row, 1, raw,
                                                         strcmp(verdict, "guest-timeout") == 0 &&
                                                             local_index ==
                                                                 controls_evidence.last_cursor,
                                                         &cause);
                if (result == ACCURACYCOIN_RESULT_INVALID) {
                    evidence_invalid = 1;
                    (void)snprintf(error, sizeof error, "page %u has invalid classified result",
                                   (unsigned)page_number);
                    result = ACCURACYCOIN_RESULT_NOT_RUN;
                    cause = "invalid-observation";
                }
            } else {
                result = ACCURACYCOIN_RESULT_NOT_RUN;
                if (strcmp(outer->outcome, "OUTER_TIMEOUT") == 0)
                    cause = "outer-timeout";
                else if (strcmp(outer->outcome, "CHILD_FAILURE") == 0)
                    cause = "child-failure";
                else if (page_metadata_valid != 0 && start_attributed == 0)
                    cause = stop_cause;
                else
                    cause = final_seen[local_index] != 0u ? "untrusted-final-snapshot"
                                                          : "missing-final-snapshot";
            }
            if (row->kind == (uint8_t)ACCURACYCOIN_KIND_TEST) {
                if ((unsigned)result < 5u)
                    status_counts[(unsigned)result]++;
                if (observed != 0)
                    observed_rows++;
                if (result == ACCURACYCOIN_RESULT_PASS || result == ACCURACYCOIN_RESULT_FAIL ||
                    result == ACCURACYCOIN_RESULT_SKIP)
                    terminal_rows++;
            }
            char raw_text[5];
            if (observed != 0)
                (void)snprintf(raw_text, sizeof raw_text, "0x%02x", (unsigned)raw);
            else
                (void)snprintf(raw_text, sizeof raw_text, "-");
            if (fprintf(report_file, "accuracycoin/%s\t%u\t%u\t%s\t%s\t%d\t%s\t%llu\t%s\t%s\n",
                        row->name, (unsigned)row->page, (unsigned)row->row,
                        row->kind == (uint8_t)ACCURACYCOIN_KIND_DRAW ? "draw" : "test",
                        accuracycoin_status_text(result), observed, raw_text,
                        (unsigned long long)frames_for_row, cause != NULL ? cause : "unknown",
                        row_attempt) < 0) {
                evidence_invalid = 1;
                (void)snprintf(error, sizeof error, "cannot write final report");
                goto collect_done;
            }
        }
    }

collect_done:
    if (draw_rows != directory->draw_count) {
        evidence_invalid = 1;
        (void)snprintf(error, sizeof error, "campaign DRAW inventory was contaminated");
    }
    unsigned classified_total = status_counts[0] + status_counts[1] + status_counts[2] +
                                status_counts[3] + status_counts[4];
    if (classified_total != directory->scored_count) {
        evidence_invalid = 1;
        (void)snprintf(error, sizeof error,
                       "campaign status counts do not sum to scored inventory");
    }
    if (summary_file == NULL) {
        evidence_invalid = 1;
        (void)snprintf(error, sizeof error, "campaign summary output is unavailable");
    } else if (
        fprintf(
            summary_file,
            "AccuracyCoin bounded "
            "campaign\nrom_pin=%s\nrom_sha256=%s\ninventory_sha256=%s\nscored_total=%u\npass=%u/"
            "%u\nfail=%u\nskip=%u\nnot-run=%u\ntimeout=%u\ndraw_inventory_only=%u\npages_launched=%"
            "u/%u\npages_start_attributed=%u/%u\nattempted_scored_rows=%u/"
            "%u\nobserved_scored_rows=%u/%u\nterminal_scored_rows=%u/%u\nouter_infrastructure=%s\n",
            directory->pin, directory->rom_sha256, directory->inventory_sha256,
            (unsigned)directory->scored_count, status_counts[ACCURACYCOIN_RESULT_PASS],
            (unsigned)directory->scored_count, status_counts[ACCURACYCOIN_RESULT_FAIL],
            status_counts[ACCURACYCOIN_RESULT_SKIP], status_counts[ACCURACYCOIN_RESULT_NOT_RUN],
            status_counts[ACCURACYCOIN_RESULT_TIMEOUT], draw_rows, ACCURACYCOIN_PAGE_COUNT,
            ACCURACYCOIN_PAGE_COUNT, attempted_pages, ACCURACYCOIN_PAGE_COUNT,
            attempted_scored_rows, (unsigned)directory->scored_count, observed_rows,
            (unsigned)directory->scored_count, terminal_rows, (unsigned)directory->scored_count,
            infrastructure_failed || evidence_invalid ? "failed" : "complete") < 0) {
        evidence_invalid = 1;
        (void)snprintf(error, sizeof error, "cannot write summary");
    }
    if (summary_file != NULL) {
        for (size_t i = 0u; i < directory->row_count; i++)
            if (directory->rows[i].kind == (uint8_t)ACCURACYCOIN_KIND_DRAW &&
                fprintf(summary_file, "draw\t%u\t%s\n", (unsigned)directory->rows[i].page,
                        directory->rows[i].name) < 0) {
                evidence_invalid = 1;
                (void)snprintf(error, sizeof error, "cannot write campaign DRAW inventory");
            }
        if (fflush(summary_file) != 0) {
            evidence_invalid = 1;
            (void)snprintf(error, sizeof error, "cannot flush campaign summary");
        }
    }
    if (report_file != NULL && fflush(report_file) != 0) {
        evidence_invalid = 1;
        (void)snprintf(error, sizeof error, "cannot flush campaign report");
    }
    if (observations_file != NULL && fflush(observations_file) != 0) {
        evidence_invalid = 1;
        (void)snprintf(error, sizeof error, "cannot flush campaign observations");
    }
    if (attempts_file != NULL && fflush(attempts_file) != 0) {
        evidence_invalid = 1;
        (void)snprintf(error, sizeof error, "cannot flush campaign attempts");
    }
    if (metadata_file != NULL) {
        if (fprintf(
                metadata_file,
                "report_schema=1\ninventory_schema=1\nrom_pin=%s\nrom_sha256=%s\ninventory_sha256=%"
                "s\nsource_revision=%s\nsource_dirty=%s\nlibrary_version=%s\nbehaviour_revision=%"
                "s\nmode=campaign\ninput_start_policy=%s\nboot_frame_budget=%u\nnavigation_frame_"
                "budget_per_page=%u\nexecution_frame_budget=%u\ntotal_frame_budget=%llu\npages_"
                "launched=%u\npages_start_attributed=%u\nattempted_scored_rows=%u\npages_completed="
                "%u\nscored_total=%u\npass=%u\nfail=%u\nskip=%u\nnot_run=%u\ntimeout=%u\ndraw=%"
                "u\nobserved_scored_rows=%u\nterminal_scored_rows=%u\nhost_elapsed_guest_ns=%"
                "llu\ntrustworthy_observation=%s\ninfrastructure_verdict=%s\n",
                directory->pin, directory->rom_sha256, directory->inventory_sha256,
                have_identity ? campaign_source : "-", have_identity ? campaign_dirty : "-",
                have_identity ? campaign_library : "-", have_identity ? campaign_behaviour : "-",
                have_identity ? campaign_policy : "-", campaign_boot_cap, campaign_navigation_cap,
                campaign_execution_cap, (unsigned long long)campaign_total_cap,
                ACCURACYCOIN_PAGE_COUNT, attempted_pages, attempted_scored_rows,
                infrastructure_failed || evidence_invalid ? 0u : ACCURACYCOIN_PAGE_COUNT,
                (unsigned)directory->scored_count, status_counts[0], status_counts[1],
                status_counts[2], status_counts[3], status_counts[4], draw_rows, observed_rows,
                terminal_rows, (unsigned long long)campaign_host_elapsed_ns,
                infrastructure_failed || evidence_invalid ? "0" : "1",
                infrastructure_failed || evidence_invalid ? "failed" : "complete") < 0 ||
            fflush(metadata_file) != 0)
            evidence_invalid = 1;
    }
    {
        int close_failed = 0;
        if (inventory_file != NULL && fclose(inventory_file) != 0)
            close_failed = 1;
        if (attempts_file != NULL && fclose(attempts_file) != 0)
            close_failed = 1;
        if (observations_file != NULL && fclose(observations_file) != 0)
            close_failed = 1;
        if (report_file != NULL && fclose(report_file) != 0)
            close_failed = 1;
        if (summary_file != NULL && fclose(summary_file) != 0)
            close_failed = 1;
        if (metadata_file != NULL && fclose(metadata_file) != 0)
            close_failed = 1;
        if (close_failed != 0) {
            evidence_invalid = 1;
            (void)snprintf(error, sizeof error, "cannot close campaign output");
        }
    }
    if (evidence_invalid || infrastructure_failed) {
        fprintf(stderr, "nesturbator-run: AccuracyCoin campaign collection failed: %s\n", error);
        return RUNNER_EXIT_FAILURE;
    }
    printf("accuracycoin campaign complete (%u scored, %u DRAW; pass %u fail %u skip %u not-run %u "
           "timeout %u)\n",
           (unsigned)directory->scored_count, draw_rows, status_counts[0], status_counts[1],
           status_counts[2], status_counts[3], status_counts[4]);
    return fflush(stdout) == 0 && ferror(stdout) == 0 ? RUNNER_EXIT_DONE : RUNNER_EXIT_FAILURE;
}
