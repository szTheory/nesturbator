/* Production-helper controls for the pinned AccuracyCoin directory/protocol. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "accuracycoin.h"
#include "check.h"

typedef struct fixture_bytes {
    uint8_t *bytes;
    size_t size;
} fixture_bytes;

#define FIXTURE_SIZE (16u + 32768u + 8192u)
#define PAGE_TABLE_OFFSET 0x110u
#define PRG_END_OFFSET (16u + 32768u)

static uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | (uint16_t)((uint16_t)bytes[1] << 8));
}

static void write_le16(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
}

static size_t page_offset(const uint8_t *bytes, unsigned page)
{
    uint16_t address = read_le16(bytes + PAGE_TABLE_OFFSET + (size_t)(page - 1u) * 2u);
    return 16u + (size_t)(address - 0x8000u);
}

static size_t page_limit(const uint8_t *bytes, unsigned page)
{
    return page < ACCURACYCOIN_PAGE_COUNT ? page_offset(bytes, page + 1u) : PRG_END_OFFSET;
}

static size_t page_title_terminator(const uint8_t *bytes, unsigned page)
{
    size_t cursor = page_offset(bytes, page);
    size_t limit = page_limit(bytes, page);
    while (cursor < limit && bytes[cursor] != 0xffu)
        cursor++;
    return cursor;
}

static int row_offsets(const uint8_t *bytes, unsigned page, unsigned row,
                       size_t *name_offset, size_t *field_offset, size_t *encoded_length)
{
    size_t cursor = page_title_terminator(bytes, page) + 1u;
    size_t limit = page_limit(bytes, page);
    if (cursor == 0u || cursor > limit)
        return 0;
    for (unsigned current = 1u; cursor < limit && bytes[cursor] != 0xffu; current++) {
        size_t start = cursor;
        while (cursor < limit && bytes[cursor] != 0xffu)
            cursor++;
        if (cursor >= limit || limit - (cursor + 1u) < 4u)
            return 0;
        if (current == row) {
            *name_offset = start;
            *field_offset = cursor + 1u;
            *encoded_length = cursor - start + 1u;
            return 1;
        }
        cursor += 5u;
    }
    return 0;
}

static void expect_inventory_reason(const uint8_t *bytes, size_t size,
                                    accuracycoin_reason expected)
{
    accuracycoin_directory parsed;
    accuracycoin_reason actual = ACCURACYCOIN_REASON_OK;
    int accepted = accuracycoin_parse_for_pin(bytes, size, ACCURACYCOIN_PIN_NEW, &parsed, &actual);
    CHECK(!accepted);
    CHECK_EQ_U64(actual, expected);
}

static int contains_encoded_token(const uint8_t *bytes, size_t name_offset, size_t encoded_length)
{
    for (size_t i = 0u; i + 1u < encoded_length; i++)
        if (bytes[name_offset + i] >= 0xf0u && bytes[name_offset + i] <= 0xf4u)
            return 1;
    return 0;
}

static int read_fixture(const char *path, fixture_bytes *out)
{
    FILE *file = fopen(path, "rb");
    long length;
    uint8_t *bytes;
    if (file == NULL || fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        if (file != NULL)
            fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)length);
    if (bytes == NULL || fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(bytes);
        fclose(file);
        return 0;
    }
    if (fclose(file) != 0) {
        free(bytes);
        return 0;
    }
    out->bytes = bytes;
    out->size = (size_t)length;
    return 1;
}

static void check_page3(const accuracycoin_directory *directory)
{
    static const char *const expected[] = {"$03   SLO indirect,X", "$07   SLO zeropage",
                                            "$0F   SLO absolute", "$13   SLO indirect,Y",
                                            "$17   SLO zeropage,X", "$1B   SLO absolute,Y",
                                            "$1F   SLO absolute,X"};
    const accuracycoin_page *page = &directory->pages[2];
    CHECK(strcmp(page->title, "Unofficial Instructions: SLO") == 0);
    CHECK_EQ_U64(page->row_count, 7u);
    for (unsigned i = 0u; i < 7u; i++) {
        const accuracycoin_test *row = &directory->rows[page->first_row + i];
        CHECK(strcmp(row->name, expected[i]) == 0);
        CHECK_EQ_U64(row->page, 3u);
        CHECK_EQ_U64(row->row, i + 1u);
        CHECK_EQ_U64(row->kind, ACCURACYCOIN_KIND_TEST);
    }
}

static void check_all_token_expansions(const accuracycoin_directory *directory)
{
    static const char *const expected[] = {" indirect", " zeropage", " absolute", " immediate",
                                           "Unofficial Instructions: "};
    for (size_t token = 0u; token < sizeof expected / sizeof expected[0]; token++) {
        int found = 0;
        for (unsigned page = 0u; page < ACCURACYCOIN_PAGE_COUNT; page++)
            if (strstr(directory->pages[page].title, expected[token]) != NULL)
                found = 1;
        for (unsigned row = 0u; row < directory->row_count; row++)
            if (strstr(directory->rows[row].name, expected[token]) != NULL)
                found = 1;
        CHECK(found);
    }
}

static int find_unused_result_address(const accuracycoin_directory *directory, uint16_t *address)
{
    for (uint16_t candidate = 0x0400u; candidate <= 0x04ffu; candidate++) {
        int used = 0;
        for (unsigned row = 0u; row < directory->row_count; row++)
            if (directory->rows[row].kind == ACCURACYCOIN_KIND_TEST &&
                directory->rows[row].result_address == candidate)
                used = 1;
        if (!used) {
            *address = candidate;
            return 1;
        }
    }
    return 0;
}

static void test_inventory_mutations(const uint8_t *source, size_t size,
                                     const accuracycoin_directory *directory)
{
    uint8_t *mutant = (uint8_t *)malloc(size);
    size_t name_a;
    size_t field_a;
    size_t encoded_a;
    size_t name_b;
    size_t field_b;
    size_t encoded_b;
    uint16_t unused_address;
    accuracycoin_directory ignored;
    accuracycoin_reason reason = ACCURACYCOIN_REASON_OK;
    CHECK(mutant != NULL);
    if (mutant == NULL)
        return;

    CHECK(!accuracycoin_parse_pinned(NULL, 0u, &ignored, &reason));
    CHECK_EQ_U64(reason, ACCURACYCOIN_REASON_IMAGE_SIZE);
    CHECK(!accuracycoin_parse_pinned(source, size - 1u, &ignored, &reason));
    CHECK_EQ_U64(reason, ACCURACYCOIN_REASON_IMAGE_SIZE);

    /* Repeated and missing pages are rejected by pointer order, even when
     * their aggregate row count would otherwise be preserved. */
    memcpy(mutant, source, size);
    write_le16(mutant + PAGE_TABLE_OFFSET + 2u, read_le16(mutant + PAGE_TABLE_OFFSET));
    expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_PAGE_POINTER);
    memcpy(mutant, source, size);
    write_le16(mutant + PAGE_TABLE_OFFSET + 4u, read_le16(mutant + PAGE_TABLE_OFFSET + 2u));
    expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_PAGE_POINTER);
    memcpy(mutant, source, size);
    write_le16(mutant + PAGE_TABLE_OFFSET + 2u, 0x7fffu);
    expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_PAGE_POINTER);

    /* Empty fields and unsupported encodings are distinguished explicitly. */
    memcpy(mutant, source, size);
    mutant[page_offset(mutant, 1u)] = 0xffu;
    expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_NAME_SIZE);
    memcpy(mutant, source, size);
    mutant[page_offset(mutant, 1u)] = 0xf5u;
    expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_NAME_ENCODING);
    memcpy(mutant, source, size);
    mutant[page_title_terminator(mutant, 1u) + 1u] = 0xffu;
    expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_EMPTY_PAGE);
    memcpy(mutant, source, size);
    mutant[page_offset(mutant, 1u) + 1u] = 0x41u;
    for (size_t i = page_offset(mutant, 1u) + 2u; i < page_limit(mutant, 1u); i++)
        mutant[i] = (uint8_t)'A';
    expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_NAME_SIZE);

    /* Four F4 tokens expand to 100 bytes. Twenty-seven literal bytes complete
     * the exact 127-byte decoded capacity; one additional byte is rejected. */
    memcpy(mutant, source, size);
    size_t title = page_offset(mutant, 1u);
    for (unsigned i = 0u; i < 4u; i++)
        mutant[title + i] = 0xf4u;
    memset(mutant + title + 4u, 'A', 27u);
    mutant[title + 31u] = 0xffu;
    mutant[title + 32u] = 0xffu;
    expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_EMPTY_PAGE);
    memcpy(mutant, source, size);
    title = page_offset(mutant, 1u);
    for (unsigned i = 0u; i < 4u; i++)
        mutant[title + i] = 0xf4u;
    memset(mutant + title + 4u, 'A', 28u);
    mutant[title + 32u] = 0xffu;
    expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_NAME_SIZE);

    /* A page title without a terminator at its bounded page end must not
     * consume the next page. */
    memcpy(mutant, source, size);
    uint16_t page2_address = read_le16(mutant + PAGE_TABLE_OFFSET + 2u);
    uint16_t shortened_page3 = (uint16_t)(page2_address + 16u);
    CHECK(shortened_page3 < read_le16(mutant + PAGE_TABLE_OFFSET + 6u));
    write_le16(mutant + PAGE_TABLE_OFFSET + 4u, shortened_page3);
    memset(mutant + page_offset(mutant, 2u), 'A', 16u);
    expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_ROW_FIELD);

    /* A row name without its terminator is likewise bounded by the next
     * page pointer rather than consuming the following page's bytes. */
    memcpy(mutant, source, size);
    CHECK(row_offsets(source, 3u, 1u, &name_a, &field_a, &encoded_a));
    uint16_t row_bound = (uint16_t)(0x8000u + (name_a + 4u - 16u));
    CHECK(row_bound < read_le16(mutant + PAGE_TABLE_OFFSET + 6u));
    write_le16(mutant + PAGE_TABLE_OFFSET + 6u, row_bound);
    memset(mutant + name_a, 'A', 4u);
    expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_ROW_FIELD);

    /* Relocating the final page close to PRG end makes its first row's fields
     * genuinely truncated while every read remains inside the image. */
    memcpy(mutant, source, size);
    CHECK(read_le16(mutant + PAGE_TABLE_OFFSET + 40u) < 0xfff9u);
    write_le16(mutant + PAGE_TABLE_OFFSET + 42u, 0xfff9u);
    size_t tail = 16u + (size_t)(0xfff9u - 0x8000u);
    mutant[tail] = 'X';
    mutant[tail + 1u] = 0xffu;
    mutant[tail + 2u] = 'Y';
    mutant[tail + 3u] = 0xffu;
    mutant[tail + 4u] = 0x00u;
    mutant[tail + 5u] = 0x04u;
    mutant[tail + 6u] = 0x00u;
    expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_ROW_FIELD);

    /* One bad routine and one out-of-range score address fail before the
     * canonical inventory fingerprint check. */
    memcpy(mutant, source, size);
    CHECK(row_offsets(mutant, 3u, 1u, &name_a, &field_a, &encoded_a));
    write_le16(mutant + field_a + 2u, 0x7fffu);
    expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_ROUTINE_ADDRESS);
    memcpy(mutant, source, size);
    CHECK(row_offsets(mutant, 3u, 1u, &name_a, &field_a, &encoded_a));
    write_le16(mutant + field_a, 0x03ffu);
    expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_SCORE_ADDRESS);

    /* Equal scored result addresses and decoded names fail rather than
     * receiving an invented suffix. The name collision uses F0-F4 data. */
    memcpy(mutant, source, size);
    CHECK(row_offsets(mutant, 3u, 1u, &name_a, &field_a, &encoded_a));
    CHECK(row_offsets(mutant, 3u, 2u, &name_b, &field_b, &encoded_b));
    write_le16(mutant + field_b, read_le16(mutant + field_a));
    expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_DUPLICATE_ADDRESS);
    int collision_found = 0;
    for (unsigned first = 0u; first < directory->row_count && !collision_found; first++) {
        const accuracycoin_test *source_row = &directory->rows[first];
        if (source_row->kind != ACCURACYCOIN_KIND_TEST)
            continue;
        CHECK(row_offsets(source, source_row->page, source_row->row, &name_a, &field_a,
                          &encoded_a));
        if (!contains_encoded_token(source, name_a, encoded_a))
            continue;
        for (unsigned second = first + 1u; second < directory->row_count; second++) {
            const accuracycoin_test *target_row = &directory->rows[second];
            if (target_row->kind != ACCURACYCOIN_KIND_TEST)
                continue;
            CHECK(row_offsets(source, target_row->page, target_row->row, &name_b, &field_b,
                              &encoded_b));
            if (encoded_a != encoded_b)
                continue;
            memcpy(mutant, source, size);
            memcpy(mutant + name_b, source + name_a, encoded_a);
            expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_DUPLICATE_KEY);
            collision_found = 1;
            break;
        }
    }
    CHECK(collision_found);

    /* Substituting one row name or an unused result address preserves counts
     * but must still fail the independently retained fingerprint. */
    memcpy(mutant, source, size);
    CHECK(row_offsets(mutant, 3u, 1u, &name_a, &field_a, &encoded_a));
    mutant[name_a] = (uint8_t)'%';
    expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_INVENTORY_FINGERPRINT);
    CHECK(find_unused_result_address(directory, &unused_address));
    memcpy(mutant, source, size);
    CHECK(row_offsets(mutant, directory->rows[0].page, directory->rows[0].row,
                      &name_a, &field_a, &encoded_a));
    write_le16(mutant + field_a, unused_address);
    expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_INVENTORY_FINGERPRINT);

    /* DRAW is a separate five-row sentinel class: turning one DRAW into an
     * otherwise unused scored address changes the denominator and is refused. */
    size_t draw_index = 0u;
    while (draw_index < directory->row_count &&
           directory->rows[draw_index].kind != ACCURACYCOIN_KIND_DRAW)
        draw_index++;
    CHECK(draw_index < directory->row_count);
    if (draw_index < directory->row_count) {
        memcpy(mutant, source, size);
        const accuracycoin_test *draw = &directory->rows[draw_index];
        CHECK(row_offsets(mutant, draw->page, draw->row, &name_a, &field_a, &encoded_a));
        CHECK(find_unused_result_address(directory, &unused_address));
        write_le16(mutant + field_a, unused_address);
        expect_inventory_reason(mutant, size, ACCURACYCOIN_REASON_DRAW_COUNT);
    }
    free(mutant);
}

static int test_inventory(const char *new_path, const char *old_path)
{
    fixture_bytes new_bytes = {NULL, 0u};
    fixture_bytes old_bytes = {NULL, 0u};
    accuracycoin_directory new_directory;
    accuracycoin_directory old_directory;
    accuracycoin_reason reason = ACCURACYCOIN_REASON_OK;
    CHECK(read_fixture(new_path, &new_bytes));
    CHECK(read_fixture(old_path, &old_bytes));
    if (new_bytes.bytes != NULL) {
        CHECK(accuracycoin_parse_pinned(new_bytes.bytes, new_bytes.size, &new_directory, &reason));
        CHECK_EQ_U64(reason, ACCURACYCOIN_REASON_OK);
        CHECK(strcmp(new_directory.pin, "6bee919529289c555803b612b8c9e4f7eaaeeabb") == 0);
        CHECK(strcmp(new_directory.rom_sha256,
                     "8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc") == 0);
        CHECK(strcmp(new_directory.inventory_sha256,
                     "7d022800d01ad81159109bf7457af94ab6f80e4a682fce716ec476f7c454a4a5") == 0);
        CHECK_EQ_U64(new_directory.row_count, 151u);
        CHECK_EQ_U64(new_directory.scored_count, 146u);
        CHECK_EQ_U64(new_directory.draw_count, 5u);
        CHECK_EQ_U64(ACCURACYCOIN_PAGE_COUNT, 22u);
        check_page3(&new_directory);
        check_all_token_expansions(&new_directory);
        unsigned shared_draws = 0u;
        for (unsigned row = 0u; row < new_directory.row_count; row++)
            if (new_directory.rows[row].kind == ACCURACYCOIN_KIND_DRAW &&
                new_directory.rows[row].result_address == 0x03ffu)
                shared_draws++;
        CHECK_EQ_U64(shared_draws, 5u);
        test_inventory_mutations(new_bytes.bytes, new_bytes.size, &new_directory);
    }
    if (old_bytes.bytes != NULL) {
        CHECK(accuracycoin_parse_pinned(old_bytes.bytes, old_bytes.size, &old_directory, &reason));
        CHECK_EQ_U64(reason, ACCURACYCOIN_REASON_OK);
        CHECK(strcmp(old_directory.pin, "673ef550db296136d52229961e7d39366116882a") == 0);
        CHECK(strcmp(old_directory.rom_sha256,
                     "4fe8c2bc9abc6f4d418da47b73f62cba89fcacd950fae763097a1681d650e839") == 0);
        CHECK(strcmp(old_directory.inventory_sha256,
                     "3271803cdc63c4b4a83cf65453b817dc66ec65ed97863ad19261b56f36821c84") == 0);
        CHECK_EQ_U64(old_directory.row_count, 149u);
        CHECK_EQ_U64(old_directory.scored_count, 144u);
        CHECK_EQ_U64(old_directory.draw_count, 5u);
    }
    free(new_bytes.bytes);
    free(old_bytes.bytes);
    CHECK_DONE();
}

static void check_classification(const char *key, int observed, uint8_t raw, int expired,
                                 accuracycoin_result_status expected_status,
                                 const char *expected_cause)
{
    const char *cause = NULL;
    accuracycoin_result_status status = accuracycoin_classify(key, observed, raw, expired, &cause);
    CHECK_EQ_U64(status, expected_status);
    CHECK(cause != NULL && strcmp(cause, expected_cause) == 0);
}

static int test_classifier(void)
{
    check_classification("accuracycoin/ROM is not writable", 1, 0xffu, 0,
                         ACCURACYCOIN_RESULT_SKIP, "upstream-skip");
    check_classification("accuracycoin/ROM is not writable", 1, 0x00u, 0,
                         ACCURACYCOIN_RESULT_NOT_RUN, "unstarted");
    check_classification("accuracycoin/ROM is not writable", 1, 0x01u, 0,
                         ACCURACYCOIN_RESULT_PASS, "terminal-pass");
    check_classification("accuracycoin/PPU Read Buffer", 1, 0x39u, 0,
                         ACCURACYCOIN_RESULT_PASS, "source-defined-pass-variant");
    check_classification("accuracycoin/PPU Read Buffer", 1, 0x41u, 0,
                         ACCURACYCOIN_RESULT_PASS, "source-defined-pass-variant");
    check_classification("accuracycoin/ROM is not writable", 1, 0x39u, 0,
                         ACCURACYCOIN_RESULT_INVALID, "unsupported-pass-variant");
    check_classification("accuracycoin/ROM is not writable", 1, 0x41u, 0,
                         ACCURACYCOIN_RESULT_INVALID, "unsupported-pass-variant");
    check_classification("accuracycoin/ROM is not writable", 1, 0x02u, 0,
                         ACCURACYCOIN_RESULT_INVALID, "zero-error-code");
    check_classification("accuracycoin/ROM is not writable", 1, 0x06u, 0,
                         ACCURACYCOIN_RESULT_FAIL, "terminal-fail");
    check_classification("accuracycoin/ROM is not writable", 1, 0x42u, 0,
                         ACCURACYCOIN_RESULT_FAIL, "terminal-fail");
    check_classification("accuracycoin/ROM is not writable", 1, 0x40u, 0,
                         ACCURACYCOIN_RESULT_INVALID, "invalid-result");
    check_classification("accuracycoin/ROM is not writable", 1, 0x05u, 0,
                         ACCURACYCOIN_RESULT_INVALID, "unsupported-pass-variant");
    check_classification("accuracycoin/ROM is not writable", 1, 0x03u, 0,
                         ACCURACYCOIN_RESULT_INVALID, "active-test");
    check_classification("accuracycoin/ROM is not writable", 1, 0x03u, 1,
                         ACCURACYCOIN_RESULT_TIMEOUT, "active-test-budget-exhausted");
    check_classification("accuracycoin/ROM is not writable", 1, 0x83u, 1,
                         ACCURACYCOIN_RESULT_INVALID, "unsupported-running-code");
    check_classification("accuracycoin/ROM is not writable", 0, 0x01u, 0,
                         ACCURACYCOIN_RESULT_INVALID, "missing-raw");
    check_classification("accuracycoin/ROM is not writable", 0, 0xffu, 0,
                         ACCURACYCOIN_RESULT_INVALID, "missing-raw");
    CHECK_DONE();
}

int main(int argc, char **argv)
{
    if (argc >= 2 && strcmp(argv[1], "inventory") == 0 && argc == 4)
        return test_inventory(argv[2], argv[3]);
    if (argc == 2 && strcmp(argv[1], "classifier") == 0)
        return test_classifier();
    fprintf(stderr, "usage: accuracycoin.conformance inventory NEW_ROM OLD_ROM | classifier\n");
    return 2;
}
