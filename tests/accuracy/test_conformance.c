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
    check_classification("accuracycoin/PPU Read Buffer", 1, 0x41u, 0,
                         ACCURACYCOIN_RESULT_PASS, "source-defined-pass-variant");
    check_classification("accuracycoin/ROM is not writable", 1, 0x41u, 0,
                         ACCURACYCOIN_RESULT_INVALID, "unsupported-pass-variant");
    check_classification("accuracycoin/ROM is not writable", 1, 0x02u, 0,
                         ACCURACYCOIN_RESULT_FAIL, "terminal-fail");
    check_classification("accuracycoin/ROM is not writable", 1, 0x42u, 0,
                         ACCURACYCOIN_RESULT_FAIL, "terminal-fail");
    check_classification("accuracycoin/ROM is not writable", 1, 0x03u, 0,
                         ACCURACYCOIN_RESULT_INVALID, "active-test");
    check_classification("accuracycoin/ROM is not writable", 1, 0x03u, 1,
                         ACCURACYCOIN_RESULT_TIMEOUT, "active-test-budget-exhausted");
    check_classification("accuracycoin/ROM is not writable", 1, 0x83u, 1,
                         ACCURACYCOIN_RESULT_INVALID, "unsupported-running-code");
    check_classification("accuracycoin/ROM is not writable", 0, 0x01u, 0,
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
