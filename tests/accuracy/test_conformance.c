/* Production-helper controls for the pinned AccuracyCoin directory/protocol. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(_WIN32)
#include <direct.h>
#define ACCURACYCOIN_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define ACCURACYCOIN_MKDIR(path) mkdir(path, 0700)
#endif

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

static int row_offsets(const uint8_t *bytes, unsigned page, unsigned row, size_t *name_offset,
                       size_t *field_offset, size_t *encoded_length)
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

static void expect_inventory_reason(const uint8_t *bytes, size_t size, accuracycoin_reason expected)
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
                                           "$0F   SLO absolute",   "$13   SLO indirect,Y",
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
    uint16_t unused_address = 0u;
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
        CHECK(
            row_offsets(source, source_row->page, source_row->row, &name_a, &field_a, &encoded_a));
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
    CHECK(row_offsets(mutant, directory->rows[0].page, directory->rows[0].row, &name_a, &field_a,
                      &encoded_a));
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
    const char *cause = NULL;
    static const struct {
        const char *key;
        uint8_t raw;
    } pass_variants[] = {{"accuracycoin/DMA + $2002 Read", 0x05u},
                         {"accuracycoin/DMA + $2002 Read", 0x09u},
                         {"accuracycoin/APU Register Activation", 0x05u},
                         {"accuracycoin/APU Register Activation", 0x09u},
                         {"accuracycoin/DMA + $4016 Read", 0x05u},
                         {"accuracycoin/DMA + $4016 Read", 0x09u},
                         {"accuracycoin/Implicit DMA Abort", 0x05u},
                         {"accuracycoin/Implicit DMA Abort", 0x09u},
                         {"accuracycoin/Controller Clocking", 0x05u},
                         {"accuracycoin/Controller Clocking", 0x09u},
                         {"accuracycoin/Sprites On Scanline 0", 0x05u},
                         {"accuracycoin/Sprites On Scanline 0", 0x09u},
                         {"accuracycoin/Misaligned OAM2 Address", 0x05u},
                         {"accuracycoin/Misaligned OAM2 Address", 0x09u},
                         {"accuracycoin/$93   SHA indirect,Y", 0x05u},
                         {"accuracycoin/$93   SHA indirect,Y", 0x09u},
                         {"accuracycoin/$93   SHA indirect,Y", 0x0du},
                         {"accuracycoin/$93   SHA indirect,Y", 0x11u},
                         {"accuracycoin/$9F   SHA absolute,Y", 0x05u},
                         {"accuracycoin/$9F   SHA absolute,Y", 0x09u},
                         {"accuracycoin/$9F   SHA absolute,Y", 0x0du},
                         {"accuracycoin/$9F   SHA absolute,Y", 0x11u},
                         {"accuracycoin/$9B   SHS absolute,Y", 0x05u},
                         {"accuracycoin/$9B   SHS absolute,Y", 0x09u},
                         {"accuracycoin/$9B   SHS absolute,Y", 0x0du},
                         {"accuracycoin/$9B   SHS absolute,Y", 0x11u},
                         {"accuracycoin/PPU Read Buffer", 0x39u},
                         {"accuracycoin/PPU Read Buffer", 0x41u},
                         {"accuracycoin/Address $2004 behavior", 0x39u},
                         {"accuracycoin/Address $2004 behavior", 0x41u}};
    check_classification("accuracycoin/ROM is not writable", 1, 0xffu, 0, ACCURACYCOIN_RESULT_SKIP,
                         "upstream-skip");
    check_classification("accuracycoin/ROM is not writable", 1, 0x00u, 0,
                         ACCURACYCOIN_RESULT_NOT_RUN, "unstarted");
    check_classification("accuracycoin/ROM is not writable", 1, 0x01u, 0, ACCURACYCOIN_RESULT_PASS,
                         "terminal-pass");
    check_classification("accuracycoin/PPU Read Buffer", 1, 0x39u, 0, ACCURACYCOIN_RESULT_PASS,
                         "source-defined-pass-variant");
    check_classification("accuracycoin/PPU Read Buffer", 1, 0x41u, 0, ACCURACYCOIN_RESULT_PASS,
                         "source-defined-pass-variant");
    for (size_t i = 0u; i < sizeof pass_variants / sizeof pass_variants[0]; i++)
        check_classification(pass_variants[i].key, 1, pass_variants[i].raw, 0,
                             ACCURACYCOIN_RESULT_PASS, "source-defined-pass-variant");
    check_classification("accuracycoin/ROM is not writable", 1, 0x39u, 0,
                         ACCURACYCOIN_RESULT_INVALID, "unsupported-pass-variant");
    check_classification("accuracycoin/ROM is not writable", 1, 0x41u, 0,
                         ACCURACYCOIN_RESULT_INVALID, "unsupported-pass-variant");
    check_classification("accuracycoin/ROM is not writable", 1, 0x02u, 0, ACCURACYCOIN_RESULT_FAIL,
                         "terminal-fail");
    check_classification("accuracycoin/ROM is not writable", 1, 0x06u, 0, ACCURACYCOIN_RESULT_FAIL,
                         "terminal-fail");
    check_classification("accuracycoin/ROM is not writable", 1, 0x42u, 0, ACCURACYCOIN_RESULT_FAIL,
                         "terminal-fail");
    check_classification("accuracycoin/ROM is not writable", 1, 0x40u, 0,
                         ACCURACYCOIN_RESULT_INVALID, "invalid-result");
    check_classification("accuracycoin/ROM is not writable", 1, 0x05u, 0,
                         ACCURACYCOIN_RESULT_INVALID, "unsupported-pass-variant");
    check_classification("accuracycoin/ROM is not writable", 1, 0x0du, 0,
                         ACCURACYCOIN_RESULT_INVALID, "unsupported-pass-variant");
    check_classification("accuracycoin/ROM is not writable", 1, 0x03u, 0,
                         ACCURACYCOIN_RESULT_INVALID, "active-test");
    check_classification("accuracycoin/ROM is not writable", 1, 0x03u, 1,
                         ACCURACYCOIN_RESULT_TIMEOUT, "active-test-budget-exhausted");
    check_classification("accuracycoin/ROM is not writable", 1, 0x83u, 1,
                         ACCURACYCOIN_RESULT_INVALID, "unsupported-running-code");
    /* Three-state active argument (JAM-01): the numeric values 0 and 1 are the
     * old budget flag, so existing callers keep their results. */
    CHECK_EQ_U64(ACCURACYCOIN_ACTIVE_RUNNING, 0u);
    CHECK_EQ_U64(ACCURACYCOIN_ACTIVE_BUDGET, 1u);
    CHECK_EQ_U64(ACCURACYCOIN_ACTIVE_JAM, 2u);
    check_classification("accuracycoin/ROM is not writable", 1, 0x03u, ACCURACYCOIN_ACTIVE_JAM,
                         ACCURACYCOIN_RESULT_FAIL, "guest-jam");
    check_classification("accuracycoin/ROM is not writable", 1, 0x03u, ACCURACYCOIN_ACTIVE_RUNNING,
                         ACCURACYCOIN_RESULT_INVALID, "active-test");
    check_classification("accuracycoin/ROM is not writable", 1, 0x03u, ACCURACYCOIN_ACTIVE_BUDGET,
                         ACCURACYCOIN_RESULT_TIMEOUT, "active-test-budget-exhausted");
    /* A terminal byte is never overridden by the JAM state. */
    check_classification("accuracycoin/ROM is not writable", 1, 0x01u, ACCURACYCOIN_ACTIVE_JAM,
                         ACCURACYCOIN_RESULT_PASS, "terminal-pass");
    check_classification("accuracycoin/ROM is not writable", 1, 0x06u, ACCURACYCOIN_ACTIVE_JAM,
                         ACCURACYCOIN_RESULT_FAIL, "terminal-fail");
    check_classification("accuracycoin/ROM is not writable", 1, 0x00u, ACCURACYCOIN_ACTIVE_JAM,
                         ACCURACYCOIN_RESULT_NOT_RUN, "unstarted");
    check_classification("accuracycoin/ROM is not writable", 1, 0x83u, ACCURACYCOIN_ACTIVE_JAM,
                         ACCURACYCOIN_RESULT_INVALID, "unsupported-running-code");
    check_classification("accuracycoin/ROM is not writable", 0, 0x01u, 0,
                         ACCURACYCOIN_RESULT_INVALID, "missing-raw");
    check_classification("accuracycoin/ROM is not writable", 0, 0xffu, 0,
                         ACCURACYCOIN_RESULT_INVALID, "missing-raw");
    accuracycoin_test draw_row;
    memset(&draw_row, 0, sizeof draw_row);
    draw_row.kind = (uint8_t)ACCURACYCOIN_KIND_DRAW;
    CHECK(accuracycoin_classify_row(&draw_row, 1, 0x7fu, 0, &cause) == ACCURACYCOIN_RESULT_DRAW);
    CHECK(cause != NULL && strcmp(cause, "inventory-only-draw") == 0);
    CHECK(strcmp(accuracycoin_status_text(ACCURACYCOIN_RESULT_DRAW), "draw") == 0);
    draw_row.kind = 2u;
    CHECK(accuracycoin_classify_row(&draw_row, 1, 0x01u, 0, &cause) == ACCURACYCOIN_RESULT_INVALID);
    CHECK(cause != NULL && strcmp(cause, "invalid-row") == 0);
    CHECK_DONE();
}

static int expect_start(const char *expected, uint32_t requested_page, uint8_t menu_ready,
                        uint8_t selected_page, uint8_t cursor, int a_press, int a_release,
                        int run_state, int transition)
{
    const char *actual =
        accuracycoin_validate_start(requested_page, menu_ready, selected_page, cursor, a_press,
                                    a_release, run_state, transition, 0, 0);
    return actual != NULL && strcmp(actual, expected) == 0;
}

static int test_driver(void)
{
    CHECK(expect_start("start-attributed", 3u, 0x0au, 2u, 0xffu, 1, 1, 1, 1));
    CHECK(expect_start("menu-not-ready", 3u, 0u, 2u, 0xffu, 1, 1, 1, 1));
    CHECK(expect_start("selected-page-mismatch", 3u, 0x0au, 3u, 0xffu, 1, 1, 1, 1));
    CHECK(expect_start("cursor-not-page-header", 3u, 0x0au, 2u, 0u, 1, 1, 1, 1));
    CHECK(expect_start("a-press-not-observed", 3u, 0x0au, 2u, 0xffu, 0, 1, 1, 1));
    CHECK(expect_start("a-release-not-observed", 3u, 0x0au, 2u, 0xffu, 1, 0, 1, 1));
    CHECK(expect_start("start-not-attributed", 3u, 0x0au, 2u, 0xffu, 1, 1, 1, 0));
    CHECK(expect_start("start-not-attributed", 3u, 0x0au, 2u, 0xffu, 1, 1, 0, 1));
    CHECK(expect_start("menu-not-ready", 0u, 0x0au, 0u, 0xffu, 1, 1, 1, 1));
    CHECK(strcmp(accuracycoin_validate_start(16u, 0x0au, 15u, 0xffu, 1, 1, 1, 0, 1, 1),
                 "start-attributed") == 0);
    CHECK(strcmp(accuracycoin_validate_start(16u, 0x0au, 15u, 0xffu, 1, 1, 1, 0, 1, 0),
                 "start-not-attributed") == 0);
    CHECK_DONE();
}

static int make_directory(const char *path)
{
    return ACCURACYCOIN_MKDIR(path) == 0 || errno == EEXIST;
}

/* JAM-01 report fixtures (D-14): `jam` is a consistent guest-jam page; the other
 * three each drop one part of the attribution and must be rejected. */
static int fixture_is_jam(const char *mutation)
{
    return strcmp(mutation, "jam") == 0 || strcmp(mutation, "jam-cause-dropped") == 0 ||
           strcmp(mutation, "jam-row-dropped") == 0 || strcmp(mutation, "jam-frame-missing") == 0;
}

static uint8_t fixture_final_raw(const accuracycoin_test *row, const char *mutation)
{
    if ((strcmp(mutation, "timeout") == 0 || strcmp(mutation, "multiple-timeout") == 0 ||
         fixture_is_jam(mutation)) &&
        row->page == 1u) {
        if (row->row == 1u)
            return 0xffu;
        if (row->row == 2u)
            return 0x03u;
        if (strcmp(mutation, "multiple-timeout") == 0 && row->row == 3u)
            return 0x03u;
        return 0x00u;
    }
    if (strcmp(mutation, "cap-minus") == 0)
        return 0x01u;
    if (strcmp(row->name, "PPU Read Buffer") == 0)
        return 0x41u;
    if (strcmp(row->name, "Controller Clocking") == 0)
        return 0xffu;
    if (row->page == 1u && row->row == 1u)
        return 0x02u;
    if (strcmp(mutation, "bad-variant") == 0 && row->page == 1u && row->row == 2u)
        return 0x39u;
    return 0x01u;
}

static int write_fixture_raw_stage(FILE *file, const accuracycoin_directory *directory,
                                   uint32_t page, const char *stage, const char *mutation)
{
    const accuracycoin_page *page_info = &directory->pages[page - 1u];
    for (size_t i = 0u; i < page_info->row_count; i++) {
        const accuracycoin_test *row = &directory->rows[page_info->first_row + i];
        int observed = row->kind == (uint8_t)ACCURACYCOIN_KIND_TEST;
        uint8_t raw = 0u;
        if (strcmp(stage, "a-press") == 0 && observed != 0)
            raw = 0x03u;
        else if (strcmp(stage, "final") == 0 && observed != 0)
            raw = fixture_final_raw(row, mutation);
        if (strcmp(mutation, "bad-draw") == 0 && strcmp(stage, "final") == 0 &&
            row->kind == (uint8_t)ACCURACYCOIN_KIND_DRAW && page == 16u) {
            observed = 1;
            raw = 0x01u;
        }
        char raw_text[5] = "-";
        if (observed != 0)
            (void)snprintf(raw_text, sizeof raw_text, "0x%02x", (unsigned)raw);
        if (fprintf(file, "%s\t%u\t%u\taccuracycoin/%s\t0x%04x\t%d\t%s\n", stage,
                    (unsigned)row->page, (unsigned)row->row, row->name,
                    (unsigned)row->result_address, observed, raw_text) < 0)
            return 0;
    }
    return 1;
}

static int write_fixture_page(const char *root, const accuracycoin_directory *directory,
                              uint32_t page, const char *mutation)
{
    char page_dir[4096];
    char raw_path[4096];
    char diagnostics_path[4096];
    char report_path[4096];
    char metadata_path[4096];
    char source_revision[64] = "fixture-source";
    uint32_t execution_cap = 3u;
    uint32_t execution_used = strcmp(mutation, "cap-minus") == 0 ? 2u : 3u;
    int is_jam = fixture_is_jam(mutation) && page == 1u;
    /* A timeout and a JAM share one layout: row 2 of page 1 is the running row. */
    int is_timeout =
        ((strcmp(mutation, "timeout") == 0 || strcmp(mutation, "multiple-timeout") == 0) &&
         page == 1u) ||
        is_jam != 0;
    int release_frames = strcmp(mutation, "held-a") == 0 && page == 3u ? 0 : 1;
    char rom_sha[65];
    char inventory_sha[65];
    char attempt_id[64];
    (void)snprintf(rom_sha, sizeof rom_sha, "%s", directory->rom_sha256);
    (void)snprintf(inventory_sha, sizeof inventory_sha, "%s", directory->inventory_sha256);
    (void)snprintf(attempt_id, sizeof attempt_id, "page-%02u-cold-01", (unsigned)page);
    if (strcmp(mutation, "wrong-source") == 0 && page == 2u)
        (void)snprintf(source_revision, sizeof source_revision, "different-source");
    if (strcmp(mutation, "wrong-rom-sha") == 0 && page == 1u)
        (void)snprintf(rom_sha, sizeof rom_sha, "different-rom-sha");
    if (strcmp(mutation, "wrong-inventory-sha") == 0 && page == 1u)
        (void)snprintf(inventory_sha, sizeof inventory_sha, "different-inventory-sha");
    if (strcmp(mutation, "wrong-attempt") == 0 && page == 5u)
        (void)snprintf(attempt_id, sizeof attempt_id, "page-%02u-cold-02", (unsigned)page);
    if (strcmp(mutation, "cap-plus") == 0 && page == 4u)
        execution_used = 4u;
    if (snprintf(page_dir, sizeof page_dir, "%s/page-%02u", root, (unsigned)page) < 0 ||
        snprintf(raw_path, sizeof raw_path, "%s/raw.tsv", page_dir) < 0 ||
        snprintf(diagnostics_path, sizeof diagnostics_path, "%s/diagnostics.tsv", page_dir) < 0 ||
        snprintf(report_path, sizeof report_path, "%s/report.tsv", page_dir) < 0 ||
        snprintf(metadata_path, sizeof metadata_path, "%s/metadata.txt", page_dir) < 0 ||
        !make_directory(page_dir))
        return 0;
    (void)remove(raw_path);
    (void)remove(diagnostics_path);
    (void)remove(report_path);
    (void)remove(metadata_path);

    FILE *raw_file = NULL;
    if (!(strcmp(mutation, "missing-raw") == 0 && page == 5u)) {
        raw_file = fopen(raw_path, "w");
        if (raw_file == NULL ||
            fprintf(raw_file, "stage\tpage\trow\tkey\tresult_address\tobserved\traw\n") < 0 ||
            !write_fixture_raw_stage(raw_file, directory, page, "before-start", mutation) ||
            !write_fixture_raw_stage(raw_file, directory, page, "a-press", mutation)) {
            if (raw_file != NULL)
                (void)fclose(raw_file);
            return 0;
        }
        if ((strcmp(mutation, "duplicate-stage") == 0 ||
             strcmp(mutation, "conflicting-stage") == 0) &&
            page == 1u) {
            const accuracycoin_test *row = &directory->rows[directory->pages[0].first_row];
            const char *raw = strcmp(mutation, "conflicting-stage") == 0 ? "0x02" : "0x03";
            if (fprintf(raw_file, "a-press\t1\t1\taccuracycoin/%s\t0x%04x\t1\t%s\n", row->name,
                        (unsigned)row->result_address, raw) < 0) {
                (void)fclose(raw_file);
                return 0;
            }
        }
        if (!write_fixture_raw_stage(raw_file, directory, page, "a-release", mutation) ||
            !write_fixture_raw_stage(raw_file, directory, page, "final", mutation)) {
            (void)fclose(raw_file);
            return 0;
        }
        if (fclose(raw_file) != 0)
            return 0;
    }

    FILE *diagnostics = fopen(diagnostics_path, "w");
    int is_draw_page = page == 16u;
    if (diagnostics == NULL ||
        fprintf(diagnostics,
                "stage\tframe\tmenu_ready\tpage_index\tcursor\tautomate_test_suite\trunning_all_"
                "tests\tresult_transition\n"
                "boot\t1\t10\t%u\t255\t0\t0\t0\n"
                "a-press\t2\t10\t%u\t%u\t1\t0\t%u\n"
                "a-release\t3\t10\t%u\t%u\t1\t0\t%u\n"
                "observe\t4\t10\t%u\t%u\t%u\t0\t0\n",
                (unsigned)(page - 1u), (unsigned)(page - 1u), 0u, is_draw_page ? 0u : 1u,
                (unsigned)(page - 1u), is_draw_page ? 0u : 0u, is_draw_page ? 0u : 1u,
                (unsigned)(page - 1u), is_timeout ? 1u : 255u, is_timeout ? 1u : 0u) < 0 ||
        fclose(diagnostics) != 0)
        return 0;

    int crlf_report = strcmp(mutation, "crlf-outcomes") == 0;
    int malformed_report = strcmp(mutation, "bad-report-line") == 0;
    const char *report_line_ending = crlf_report != 0 ? "\r\n" : "\n";
    FILE *report = fopen(report_path, "wb");
    if (report == NULL ||
        fprintf(report, "key\tpage\trow\tstatus\tobserved\traw\tframes\tcause\tattempt_id%s",
                report_line_ending) < 0) {
        if (report != NULL)
            (void)fclose(report);
        return 0;
    }
    const accuracycoin_page *page_info = &directory->pages[page - 1u];
    for (size_t i = 0u; i < page_info->row_count; i++) {
        const accuracycoin_test *row = &directory->rows[page_info->first_row + i];
        int observed = row->kind == (uint8_t)ACCURACYCOIN_KIND_TEST;
        uint8_t raw = observed != 0 ? fixture_final_raw(row, mutation) : 0u;
        if (is_timeout != 0 && observed != 0) {
            if (row->row == 1u)
                raw = 0xffu;
            else if (row->row == 2u)
                raw = 0x03u;
            else if (strcmp(mutation, "multiple-timeout") == 0 && row->row == 3u)
                raw = 0x03u;
            else
                raw = 0x00u;
        }
        const char *cause = NULL;
        int current_active_row = is_timeout != 0 && row->row == 2u;
        accuracycoin_result_status result;
        if (strcmp(mutation, "multiple-timeout") == 0 && row->row == 3u && raw == 0x03u) {
            result = ACCURACYCOIN_RESULT_NOT_RUN;
            cause = "noncurrent-running-marker";
        } else if (is_jam != 0 && current_active_row != 0 &&
                   strcmp(mutation, "jam-row-dropped") != 0) {
            result = accuracycoin_classify_row(row, observed, raw, ACCURACYCOIN_ACTIVE_JAM, &cause);
        } else {
            /* jam-row-dropped: the attributed row is reported as an ordinary budget
             * timeout, so no row carries the guest-jam cause. */
            result = accuracycoin_classify_row(row, observed, raw, current_active_row, &cause);
        }
        char raw_text[5] = "-";
        if (observed != 0)
            (void)snprintf(raw_text, sizeof raw_text, "0x%02x", (unsigned)raw);
        const char *line_ending = malformed_report != 0 && page == ACCURACYCOIN_PAGE_COUNT &&
                                          i + 1u == page_info->row_count
                                      ? ""
                                      : report_line_ending;
        if (fprintf(report, "accuracycoin/%s\t%u\t%u\t%s\t%d\t%s\t%u\t%s\tpage-%02u-cold-01%s",
                    row->name, (unsigned)row->page, (unsigned)row->row,
                    accuracycoin_status_text(result), observed, raw_text, execution_used,
                    cause != NULL ? cause : "invalid-result", (unsigned)page, line_ending) < 0) {
            (void)fclose(report);
            return 0;
        }
    }
    if (fclose(report) != 0)
        return 0;

    FILE *metadata = fopen(metadata_path, "w");
    if (metadata == NULL)
        return 0;
    unsigned nav_frames = (unsigned)(page - 1u) * 5u;
    unsigned total_cap = 120u + nav_frames + 2u + execution_cap;
    unsigned frames_used = 26u + nav_frames + 2u + execution_used;
    const char *verdict = is_timeout != 0 ? "guest-timeout" : "complete";
    const char *stop_cause =
        is_timeout != 0 ? "execution-cap-active-test" : "all-page-results-terminal";
    const char *jam_frame = "-";
    if (is_jam != 0) {
        verdict = "guest-jam";
        stop_cause =
            strcmp(mutation, "jam-cause-dropped") == 0 ? "execution-frame-failure" : "guest-jam";
        jam_frame = strcmp(mutation, "jam-frame-missing") == 0 ? "-" : "2";
    }
    char active_key[ACCURACYCOIN_NAME_CAPACITY + 16u] = "-";
    if (is_timeout != 0) {
        const accuracycoin_test *active = &directory->rows[page_info->first_row + 1u];
        (void)snprintf(active_key, sizeof active_key, "accuracycoin/%s", active->name);
    }
    if (strcmp(mutation, "wrong-active") == 0 && page == 1u)
        (void)snprintf(active_key, sizeof active_key, "accuracycoin/not-the-active-row");
    if (fprintf(
            metadata,
            "report_schema=%s\ninventory_schema=1\nrom_pin=%s\nrom_sha256=%s\ninventory_sha256=%"
            "s\nsource_revision=%s\nsource_dirty=0\nlibrary_version=0.1.0\nbehaviour_revision="
            "1\nmode=observe\npage=%u\nattempt_id=%s\norigin=cold-page\nrequested_row=all\ninput_"
            "start_policy=cold-boot;page-header-A;one-frame-A-press;one-frame-release\nboot_frame_"
            "budget=120\nnavigation_frame_budget_per_page=5\nexecution_frame_budget=%u\ntotal_"
            "frame_budget=%u\nboot_frames_used=26\nnavigation_frames_used=%u\nstart_frames_used="
            "2\nexecution_frames=%u\nframes_used=%u\nstartup_dmc_pretest_observed=1\nstartup_dmc_"
            "pretest=0x00\nstartup_vblank_pretest_observed=1\nstartup_vblank_pretest=0x00\nstartup_"
            "ppu_reset_pretest_observed=1\nstartup_ppu_reset_pretest=0x01\nselection_observed="
            "1\nselected_page=%u\nselected_cursor=255\na_press_observed=1\na_release_observed="
            "1\nrelease_frames=%d\nrun_state_observed=1\nresult_transition_observed=%d\ndraw_"
            "cursor_transition_observed=%d\nmenu_returned_to_header_observed=%d\nstart_attributed="
            "1\nstop_cause=%s\nactive_key=%s\nactive_raw=%s\nverdict=%s\njam_frame=%s\nhost_"
            "elapsed_"
            "ns=1000\n",
            strcmp(mutation, "bad-schema") == 0 && page == 1u ? "2" : "1", directory->pin, rom_sha,
            inventory_sha, source_revision, (unsigned)page, attempt_id, execution_cap, total_cap,
            nav_frames, execution_used, frames_used, (unsigned)(page - 1u), release_frames,
            is_draw_page ? 0 : 1, is_draw_page, !is_timeout, stop_cause, active_key,
            is_timeout != 0 ? "0x03" : "-", verdict, jam_frame) < 0 ||
        fclose(metadata) != 0)
        return 0;
    return 1;
}

static int write_campaign_fixture(const char *root, const accuracycoin_directory *directory,
                                  const char *mutation)
{
    char path[4096];
    if (!make_directory(root) || snprintf(path, sizeof path, "%s/outer-outcomes.tsv", root) < 0)
        return 0;
    int crlf = strcmp(mutation, "crlf-outcomes") == 0;
    const char *line_ending = crlf != 0 ? "\r\n" : "\n";
    FILE *outer = fopen(path, "wb");
    if (outer == NULL ||
        fprintf(outer, "page\toutcome\texit_code\tdeadline_seconds%s", line_ending) < 0) {
        if (outer != NULL)
            (void)fclose(outer);
        return 0;
    }
    for (uint32_t page = 1u; page <= ACCURACYCOIN_PAGE_COUNT; page++)
        if (fprintf(outer, "%u\tEXIT_ZERO\t0\t30%s", (unsigned)page, line_ending) < 0) {
            (void)fclose(outer);
            return 0;
        }
    if (fclose(outer) != 0)
        return 0;
    for (uint32_t page = 1u; page <= ACCURACYCOIN_PAGE_COUNT; page++)
        if (!write_fixture_page(root, directory, page, mutation))
            return 0;
    return 1;
}

static int expect_fixture_rejected(const char *work, const char *case_name,
                                   const accuracycoin_directory *directory, const char *mutation)
{
    char root[4096];
    if (snprintf(root, sizeof root, "%s/%s", work, case_name) < 0 ||
        !write_campaign_fixture(root, directory, mutation)) {
        CHECK(0);
        return 0;
    }
    CHECK(accuracycoin_collect_campaign(directory, root) != 0);
    return 1;
}

static int expect_conflicting_observations_retained(const char *work,
                                                    const accuracycoin_directory *directory)
{
    char root[4096];
    char path[4096];
    char prefix[512];
    if (snprintf(root, sizeof root, "%s/conflicting-stage", work) < 0 ||
        !write_campaign_fixture(root, directory, "conflicting-stage")) {
        CHECK(0);
        return 0;
    }
    CHECK(accuracycoin_collect_campaign(directory, root) != 0);
    if (snprintf(path, sizeof path, "%s/observations.tsv", root) < 0 ||
        snprintf(prefix, sizeof prefix,
                 "page-01-cold-01\ta-press\t1\t1\taccuracycoin/%s\t0x%04x\t1\t",
                 directory->rows[directory->pages[0].first_row].name,
                 (unsigned)directory->rows[directory->pages[0].first_row].result_address) < 0) {
        CHECK(0);
        return 0;
    }
    FILE *observations = fopen(path, "r");
    CHECK(observations != NULL);
    unsigned copies = 0u;
    int retained_original = 0;
    int retained_conflict = 0;
    if (observations != NULL) {
        char line[1024];
        while (fgets(line, sizeof line, observations) != NULL) {
            if (strncmp(line, prefix, strlen(prefix)) == 0) {
                copies++;
                retained_original |= strstr(line, "0x03\n") != NULL;
                retained_conflict |= strstr(line, "0x02\n") != NULL;
            }
        }
        CHECK(ferror(observations) == 0);
        CHECK(fclose(observations) == 0);
    }
    CHECK_EQ_U64(copies, 2u);
    CHECK(retained_original);
    CHECK(retained_conflict);
    if (snprintf(path, sizeof path, "%s/metadata.txt", root) < 0) {
        CHECK(0);
        return 0;
    }
    FILE *metadata = fopen(path, "r");
    CHECK(metadata != NULL);
    char metadata_text[2048];
    size_t metadata_size =
        metadata != NULL ? fread(metadata_text, 1u, sizeof metadata_text - 1u, metadata) : 0u;
    if (metadata != NULL) {
        CHECK(ferror(metadata) == 0);
        CHECK(fclose(metadata) == 0);
    }
    metadata_text[metadata_size] = '\0';
    CHECK(strstr(metadata_text, "trustworthy_observation=0") != NULL);
    CHECK(strstr(metadata_text, "infrastructure_verdict=failed") != NULL);
    return 1;
}

static int test_report(const char *new_rom_path, const char *old_rom_path, const char *work)
{
    fixture_bytes new_bytes = {0};
    fixture_bytes old_bytes = {0};
    accuracycoin_directory new_directory;
    accuracycoin_directory old_directory;
    accuracycoin_reason reason = ACCURACYCOIN_REASON_OK;
    char path[4096];
    CHECK(make_directory(work));
    CHECK(read_fixture(new_rom_path, &new_bytes));
    CHECK(read_fixture(old_rom_path, &old_bytes));
    if (new_bytes.bytes == NULL || old_bytes.bytes == NULL)
        goto done;
    CHECK(accuracycoin_parse_pinned(new_bytes.bytes, new_bytes.size, &new_directory, &reason));
    CHECK(accuracycoin_parse_pinned(old_bytes.bytes, old_bytes.size, &old_directory, &reason));
    if (snprintf(path, sizeof path, "%s/good", work) < 0 ||
        !write_campaign_fixture(path, &new_directory, "good")) {
        CHECK(0);
        goto done;
    }
    CHECK_EQ_U64(accuracycoin_collect_campaign(&new_directory, path), 0u);
    snprintf(path, sizeof path, "%s/good/summary.txt", work);
    FILE *summary = fopen(path, "r");
    char summary_text[2048];
    size_t summary_size =
        summary != NULL ? fread(summary_text, 1u, sizeof summary_text - 1u, summary) : 0u;
    if (summary != NULL)
        (void)fclose(summary);
    summary_text[summary_size] = '\0';
    CHECK(strstr(summary_text, "scored_total=146") != NULL);
    CHECK(strstr(summary_text, "pass=144/146") != NULL);
    CHECK(strstr(summary_text, "fail=1") != NULL);
    CHECK(strstr(summary_text, "skip=1") != NULL);
    CHECK(strstr(summary_text, "draw_inventory_only=5") != NULL);
    if (snprintf(path, sizeof path, "%s/crlf-outcomes", work) < 0 ||
        !write_campaign_fixture(path, &new_directory, "crlf-outcomes")) {
        CHECK(0);
        goto done;
    }
    CHECK_EQ_U64(accuracycoin_collect_campaign(&new_directory, path), 0u);
    CHECK(expect_fixture_rejected(work, "unterminated-report-line", &new_directory,
                                  "bad-report-line"));
    CHECK(expect_fixture_rejected(work, "bad-schema", &new_directory, "bad-schema"));
    CHECK(expect_fixture_rejected(work, "held-a", &new_directory, "held-a"));
    CHECK(expect_fixture_rejected(work, "wrong-source", &new_directory, "wrong-source"));
    CHECK(expect_fixture_rejected(work, "bad-draw", &new_directory, "bad-draw"));
    CHECK(expect_fixture_rejected(work, "bad-variant", &new_directory, "bad-variant"));
    CHECK(expect_fixture_rejected(work, "cap-plus", &new_directory, "cap-plus"));
    CHECK(expect_fixture_rejected(work, "duplicate-stage", &new_directory, "duplicate-stage"));
    CHECK(expect_conflicting_observations_retained(work, &new_directory));
    CHECK(expect_fixture_rejected(work, "wrong-active", &new_directory, "wrong-active"));
    CHECK(expect_fixture_rejected(work, "wrong-rom-sha", &new_directory, "wrong-rom-sha"));
    CHECK(expect_fixture_rejected(work, "wrong-inventory-sha", &new_directory,
                                  "wrong-inventory-sha"));
    CHECK(expect_fixture_rejected(work, "wrong-attempt", &new_directory, "wrong-attempt"));
    CHECK(expect_fixture_rejected(work, "missing-raw", &new_directory, "missing-raw"));
    snprintf(path, sizeof path, "%s/cap-minus", work);
    CHECK(write_campaign_fixture(path, &new_directory, "cap-minus"));
    CHECK_EQ_U64(accuracycoin_collect_campaign(&new_directory, path), 0u);
    snprintf(path, sizeof path, "%s/timeout", work);
    CHECK(write_campaign_fixture(path, &new_directory, "timeout"));
    CHECK_EQ_U64(accuracycoin_collect_campaign(&new_directory, path), 0u);
    /* JAM-01 (D-14): a consistent guest-jam page is accepted; each variant that
     * drops one part of the attribution is rejected. */
    snprintf(path, sizeof path, "%s/jam", work);
    CHECK(write_campaign_fixture(path, &new_directory, "jam"));
    CHECK_EQ_U64(accuracycoin_collect_campaign(&new_directory, path), 0u);
    CHECK(expect_fixture_rejected(work, "jam-cause-dropped", &new_directory, "jam-cause-dropped"));
    CHECK(expect_fixture_rejected(work, "jam-row-dropped", &new_directory, "jam-row-dropped"));
    CHECK(expect_fixture_rejected(work, "jam-frame-missing", &new_directory, "jam-frame-missing"));
    snprintf(path, sizeof path, "%s/multiple-timeout", work);
    CHECK(write_campaign_fixture(path, &new_directory, "multiple-timeout"));
    CHECK_EQ_U64(accuracycoin_collect_campaign(&new_directory, path), 0u);
    snprintf(path, sizeof path, "%s/multiple-timeout/report.tsv", work);
    FILE *multiple_timeout = fopen(path, "r");
    char multiple_timeout_text[32768];
    size_t multiple_timeout_size =
        multiple_timeout != NULL
            ? fread(multiple_timeout_text, 1u, sizeof multiple_timeout_text - 1u, multiple_timeout)
            : 0u;
    if (multiple_timeout != NULL)
        (void)fclose(multiple_timeout);
    multiple_timeout_text[multiple_timeout_size] = '\0';
    CHECK(strstr(multiple_timeout_text, "accuracycoin/RAM "
                                        "Mirroring\t1\t2\ttest\ttimeout\t1\t0x03\t3\tactive-test-"
                                        "budget-exhausted\tpage-01-cold-01") != NULL);
    CHECK(strstr(multiple_timeout_text, "accuracycoin/PC "
                                        "Wraparound\t1\t3\ttest\tnot-run\t1\t0x03\t3\tnoncurrent-"
                                        "running-marker\tpage-01-cold-01") != NULL);
    snprintf(path, sizeof path, "%s/wrong-pin", work);
    CHECK(write_campaign_fixture(path, &new_directory, "good"));
    CHECK(accuracycoin_collect_campaign(&old_directory, path) != 0);
    snprintf(path, sizeof path, "%s/no-output-directory", work);
    CHECK(accuracycoin_collect_campaign(&new_directory, path) != 0);
done:
    free(new_bytes.bytes);
    free(old_bytes.bytes);
    CHECK_DONE();
}

#define MIGRATION_FIELD_COUNT 12u
#define PROTECTED_ROW_CAPACITY 24u

static size_t split_tsv(char *line, char **fields, size_t capacity)
{
    size_t count = 1u;
    size_t length = strlen(line);
    while (length > 0u && (line[length - 1u] == '\n' || line[length - 1u] == '\r'))
        line[--length] = '\0';
    fields[0] = line;
    for (size_t i = 0u; i < length; i++) {
        if (line[i] == '\t') {
            line[i] = '\0';
            if (count >= capacity)
                return capacity + 1u;
            fields[count++] = line + i + 1u;
        }
    }
    return count;
}

static int field_has_prefix(const char *field, const char *prefix)
{
    return strncmp(field, prefix, strlen(prefix)) == 0;
}

static int allowed_disposition(const char *value)
{
    static const char *const values[] = {"unchanged", "renamed", "changed-meaning", "removed"};
    for (size_t i = 0u; i < sizeof values / sizeof values[0]; i++)
        if (strcmp(value, values[i]) == 0)
            return 1;
    return 0;
}

static int test_migration_source(const char *migration_path, const char *scoreboard_path)
{
    static const char expected_header[] =
        "old_pin\told_key\told_result_symbol\told_routine_symbol\tnew_pin\tnew_key\t"
        "new_result_symbol\tnew_routine_symbol\tdisposition\tsource_reference\t"
        "assertion_change\tnew_observation_reference";
    static const char old_pin[] = "673ef550db296136d52229961e7d39366116882a";
    static const char new_pin[] = "6bee919529289c555803b612b8c9e4f7eaaeeabb";
    FILE *migration = fopen(migration_path, "r");
    FILE *scoreboard = fopen(scoreboard_path, "r");
    char line[2048];
    char *fields[MIGRATION_FIELD_COUNT + 1u];
    char protected_keys[PROTECTED_ROW_CAPACITY][ACCURACYCOIN_NAME_CAPACITY + 16u];
    char migration_keys[PROTECTED_ROW_CAPACITY][ACCURACYCOIN_NAME_CAPACITY + 16u];
    size_t protected_count = 0u;
    size_t migration_count = 0u;
    CHECK(migration != NULL);
    CHECK(scoreboard != NULL);
    if (migration == NULL || scoreboard == NULL) {
        if (migration != NULL)
            fclose(migration);
        if (scoreboard != NULL)
            fclose(scoreboard);
        return 1;
    }
    if (fgets(line, sizeof line, migration) == NULL) {
        CHECK(0);
        goto done;
    }
    size_t header_length = strlen(line);
    while (header_length > 0u &&
           (line[header_length - 1u] == '\n' || line[header_length - 1u] == '\r'))
        line[--header_length] = '\0';
    CHECK(strcmp(line, expected_header) == 0);
    CHECK(split_tsv(line, fields, MIGRATION_FIELD_COUNT) == MIGRATION_FIELD_COUNT);

    while (fgets(line, sizeof line, scoreboard) != NULL) {
        char *score_fields[6];
        char *end = NULL;
        unsigned long raw;
        size_t count = split_tsv(line, score_fields, 5u);
        CHECK(count == 5u);
        if (count != 5u)
            continue;
        CHECK(field_has_prefix(score_fields[0], "accuracycoin/"));
        CHECK(strcmp(score_fields[1], "pass") == 0);
        raw = strtoul(score_fields[2], &end, 0);
        CHECK(end != score_fields[2] && end != NULL && *end == '\0');
        CHECK(raw == 0x01u ||
              (raw == 0x41u && strcmp(score_fields[0], "accuracycoin/PPU Read Buffer") == 0));
        CHECK(protected_count < PROTECTED_ROW_CAPACITY);
        if (protected_count >= PROTECTED_ROW_CAPACITY)
            continue;
        for (size_t i = 0u; i < protected_count; i++)
            CHECK(strcmp(protected_keys[i], score_fields[0]) != 0);
        (void)snprintf(protected_keys[protected_count], sizeof protected_keys[0], "%s",
                       score_fields[0]);
        protected_count++;
    }
    CHECK(ferror(scoreboard) == 0);

    while (fgets(line, sizeof line, migration) != NULL) {
        size_t count = split_tsv(line, fields, MIGRATION_FIELD_COUNT);
        int found_protected = 0;
        CHECK(count == MIGRATION_FIELD_COUNT);
        if (count != MIGRATION_FIELD_COUNT)
            continue;
        CHECK(strcmp(fields[0], old_pin) == 0);
        CHECK(strcmp(fields[4], new_pin) == 0);
        CHECK(field_has_prefix(fields[2], "result_"));
        CHECK(field_has_prefix(fields[3], "TEST_"));
        CHECK(field_has_prefix(fields[6], "result_"));
        CHECK(field_has_prefix(fields[7], "TEST_"));
        CHECK(allowed_disposition(fields[8]));
        CHECK(strstr(fields[9], "github.com/100thCoin/AccuracyCoin/blob/") != NULL);
        CHECK(strstr(fields[9], old_pin) != NULL);
        CHECK(strstr(fields[9], new_pin) != NULL);
        CHECK(fields[10][0] != '\0');
        CHECK(strncmp(fields[11], "new/page-", 9u) == 0);
        CHECK(strstr(fields[11], "/report.tsv#attempt=page-") != NULL);
        CHECK(strstr(fields[11], ";row=") != NULL);
        CHECK(strstr(fields[11], ";raw=0x") != NULL);
        CHECK(strstr(fields[11], ";frames=") != NULL);
        CHECK(strstr(fields[11], ";status=pass") != NULL);
        CHECK(strcmp(fields[1], fields[5]) == 0);
        for (size_t i = 0u; i < protected_count; i++)
            if (strcmp(fields[1], protected_keys[i]) == 0)
                found_protected = 1;
        CHECK(found_protected);
        CHECK(migration_count < PROTECTED_ROW_CAPACITY);
        if (migration_count >= PROTECTED_ROW_CAPACITY)
            continue;
        for (size_t i = 0u; i < migration_count; i++)
            CHECK(strcmp(migration_keys[i], fields[1]) != 0);
        (void)snprintf(migration_keys[migration_count], sizeof migration_keys[0], "%s", fields[1]);
        migration_count++;
    }
    CHECK(ferror(migration) == 0);
    CHECK_EQ_U64(protected_count, 17u);
    CHECK_EQ_U64(migration_count, protected_count);
    for (size_t i = 0u; i < protected_count; i++) {
        int found = 0;
        for (size_t j = 0u; j < migration_count; j++)
            if (strcmp(protected_keys[i], migration_keys[j]) == 0)
                found = 1;
        CHECK(found);
    }

done:
    CHECK(fclose(migration) == 0);
    CHECK(fclose(scoreboard) == 0);
    CHECK_DONE();
}

/* JAM-01 end-to-end control (D-12): the real observe path on the pinned ROM with
 * one byte patched in memory only. The pin and SHA checks run on the unpatched
 * bytes; the patched copy goes to nesturbator_load_cartridge and nowhere else, so
 * no modified ROM exists on disk and nesturbator-run keeps refusing one.
 *
 * NROM file offset of a CPU address: 16-byte iNES header plus the offset into the
 * 32 KiB PRG window at $8000 [NESdev Wiki, NROM]. This duplicates the static
 * rom_offset() in runner/accuracycoin.c on purpose: a small local copy with its
 * own bounds check is simpler than exporting a runner internal for one test. */
static int nrom_file_offset(uint16_t address, size_t image_size, size_t *offset)
{
    if (address < 0x8000u)
        return 0;
    *offset = 16u + (size_t)(address - 0x8000u);
    return *offset < PRG_END_OFFSET && *offset < image_size;
}

#define JAM_OPCODE 0x02u /* an undocumented KIL opcode: the CPU stops */

/* The last TEST-kind row of a page (0-based page index), chosen by rule from the
 * parsed inventory, never a literal offset. */
static const accuracycoin_test *last_test_row(const accuracycoin_directory *directory,
                                              unsigned page_index)
{
    const accuracycoin_page *page = &directory->pages[page_index];
    const accuracycoin_test *row = NULL;
    for (unsigned i = 0u; i < page->row_count; i++)
        if (directory->rows[page->first_row + i].kind == ACCURACYCOIN_KIND_TEST)
            row = &directory->rows[page->first_row + i];
    return row;
}

/* A copy of the ROM with JAM_OPCODE at the file offset of a CPU address, after
 * asserting the original byte is not already JAM_OPCODE. NULL on any failure. */
static uint8_t *patched_copy(const fixture_bytes *rom, uint16_t address, size_t *offset_out)
{
    size_t offset = 0u;
    uint8_t *copy;
    if (!nrom_file_offset(address, rom->size, &offset) || rom->bytes[offset] == JAM_OPCODE)
        return NULL;
    copy = (uint8_t *)malloc(rom->size);
    if (copy == NULL)
        return NULL;
    memcpy(copy, rom->bytes, rom->size);
    copy[offset] = JAM_OPCODE;
    *offset_out = offset;
    return copy;
}

static nesturbator *jam_instance(const uint8_t *image, size_t size)
{
    nesturbator_config config;
    nesturbator *inst = NULL;
    memset(&config, 0, sizeof config);
    config.size = (uint32_t)sizeof config;
    config.abi = NESTURBATOR_ABI_VERSION;
    if (nesturbator_create(&config, &inst) != NESTURBATOR_OK)
        return NULL;
    if (nesturbator_load_cartridge(inst, image, size) != NESTURBATOR_OK) {
        nesturbator_destroy(inst);
        return NULL;
    }
    return inst;
}

static int test_jam(const char *new_rom_path, const char *old_rom_path, const char *old_scoreboard,
                    const char *work)
{
    static uint16_t video[256u * 240u];
    static int16_t audio[1024u];
    fixture_bytes rom = {0};
    fixture_bytes old_rom = {0};
    accuracycoin_directory directory;
    accuracycoin_directory old_directory;
    accuracycoin_reason reason = ACCURACYCOIN_REASON_OK;
    accuracycoin_run_budgets budgets;
    nesturbator *inst = NULL;
    uint8_t *patched = NULL;
    char output_dir[4096];
    size_t offset = 0u;
    int exit_code;
    const accuracycoin_test *row = NULL;

    CHECK(make_directory(work));
    CHECK(read_fixture(new_rom_path, &rom));
    CHECK(read_fixture(old_rom_path, &old_rom));
    if (rom.bytes == NULL || old_rom.bytes == NULL)
        goto done;
    CHECK(accuracycoin_parse_pinned(rom.bytes, rom.size, &directory, &reason));
    if (reason != ACCURACYCOIN_REASON_OK)
        goto done;
    CHECK(accuracycoin_parse_pinned(old_rom.bytes, old_rom.size, &old_directory, &reason));
    if (reason != ACCURACYCOIN_REASON_OK)
        goto done;

    budgets.boot_frames = 32u;
    budgets.navigation_frames_per_page = 5u;
    budgets.execution_frames = 1800u;
    budgets.total_frames = 32u + 5u * 2u + 2u + 1800u;

    /* Leg 1: the last TEST row of page 3. A JAM in the execution stage is a
     * finished observation: exit 0, report written. */
    row = last_test_row(&directory, 2u);
    CHECK(row != NULL);
    if (row == NULL)
        goto done;
    patched = patched_copy(&rom, row->routine_address, &offset);
    CHECK(patched != NULL);
    if (patched == NULL)
        goto done;
    inst = jam_instance(patched, rom.size);
    CHECK(inst != NULL);
    if (inst == NULL)
        goto done;
    CHECK(snprintf(output_dir, sizeof output_dir, "%s/observe-page3", work) > 0);
    CHECK(make_directory(output_dir));
    printf("jam-patched-key=accuracycoin/%s\n", row->name);
    printf("jam-patched-offset=%llu\n", (unsigned long long)offset);
    fflush(stdout);
    exit_code =
        accuracycoin_run_observe_page(&directory, 3u, &budgets, output_dir, inst, video, audio);
    printf("jam-observe-exit=%d\n", exit_code);
    fflush(stdout);
    CHECK_EQ_U64(exit_code, 0u);
    nesturbator_destroy(inst);
    inst = NULL;
    free(patched);
    patched = NULL;

    /* Leg 2: a JAM before the execution stage. The reset vector is read from the
     * copy through the same bounds-checked mapping and its target patched, so the
     * very first instruction jams. No report is written and the exit is 1. */
    {
        size_t lo_offset = 0u;
        size_t hi_offset = 0u;
        uint16_t reset;
        CHECK(nrom_file_offset(0xFFFCu, rom.size, &lo_offset));
        CHECK(nrom_file_offset(0xFFFDu, rom.size, &hi_offset));
        if (lo_offset == 0u || hi_offset == 0u)
            goto done;
        reset = (uint16_t)(rom.bytes[lo_offset] | (rom.bytes[hi_offset] << 8));
        patched = patched_copy(&rom, reset, &offset);
        CHECK(patched != NULL);
        if (patched == NULL)
            goto done;
    }
    inst = jam_instance(patched, rom.size);
    CHECK(inst != NULL);
    if (inst == NULL)
        goto done;
    CHECK(snprintf(output_dir, sizeof output_dir, "%s/observe-boot", work) > 0);
    CHECK(make_directory(output_dir));
    exit_code =
        accuracycoin_run_observe_page(&directory, 3u, &budgets, output_dir, inst, video, audio);
    printf("jam-boot-exit=%d\n", exit_code);
    fflush(stdout);
    CHECK_EQ_U64(exit_code, 1u);
    nesturbator_destroy(inst);
    inst = NULL;
    free(patched);
    patched = NULL;

    /* Leg 3: strict page mode on the old ROM, page 2. The patched row is the last
     * TEST row of the old inventory's page 2; strict mode names the JAM on stderr
     * and returns 1 (it used to fail silently). */
    row = last_test_row(&old_directory, 1u);
    CHECK(row != NULL);
    if (row == NULL)
        goto done;
    patched = patched_copy(&old_rom, row->routine_address, &offset);
    CHECK(patched != NULL);
    if (patched == NULL)
        goto done;
    inst = jam_instance(patched, old_rom.size);
    CHECK(inst != NULL);
    if (inst == NULL)
        goto done;
    exit_code =
        accuracycoin_run_strict_page(&old_directory, 2u, old_scoreboard, inst, video, audio, NULL);
    printf("jam-strict-exit=%d\n", exit_code);
    fflush(stdout);
    CHECK_EQ_U64(exit_code, 1u);

done:
    if (inst != NULL)
        nesturbator_destroy(inst);
    free(patched);
    free(rom.bytes);
    free(old_rom.bytes);
    CHECK_DONE();
}

int main(int argc, char **argv)
{
    if (argc >= 2 && strcmp(argv[1], "inventory") == 0 && argc == 4)
        return test_inventory(argv[2], argv[3]);
    if (argc == 2 && strcmp(argv[1], "classifier") == 0)
        return test_classifier();
    if (argc == 2 && strcmp(argv[1], "driver") == 0)
        return test_driver();
    if (argc == 5 && strcmp(argv[1], "report") == 0)
        return test_report(argv[2], argv[3], argv[4]);
    if (argc == 6 && strcmp(argv[1], "jam") == 0)
        return test_jam(argv[2], argv[3], argv[4], argv[5]);
    if (argc == 4 && strcmp(argv[1], "migration") == 0)
        return test_migration_source(argv[2], argv[3]);
    fprintf(stderr, "usage: accuracycoin.conformance inventory NEW_ROM OLD_ROM | classifier | "
                    "driver | report NEW_ROM OLD_ROM WORK | migration TSV SCOREBOARD | "
                    "jam NEW_ROM OLD_ROM OLD_SCOREBOARD WORK\n");
    return 2;
}
