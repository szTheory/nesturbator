/* The runner's owned SHA-256 against the FIPS 180-4 example answers
   (NIST "SHA-256" example values for "abc" and the two-block message). */
#include <string.h>

#include "sha256.h"
#include "../check.h"

static int digest_is(const void *msg, size_t len, size_t step, const char *want)
{
    nesturbator_run_sha256 s;
    uint8_t d[32];
    char hex[65];
    const unsigned char *p = (const unsigned char *)msg;
    nesturbator_run_sha256_init(&s);
    if (step == 0u) {
        nesturbator_run_sha256_update(&s, p, len);
    } else {
        for (size_t i = 0; i < len; i += step) {
            nesturbator_run_sha256_update(&s, p + i, len - i < step ? len - i : step);
        }
    }
    nesturbator_run_sha256_final(&s, d);
    nesturbator_run_sha256_hex(d, hex);
    return strcmp(hex, want) == 0;
}

int main(void)
{
    static const char two_blocks[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    CHECK(
        digest_is("", 0u, 0u, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
    CHECK(digest_is("abc", 3u, 0u,
                    "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"));
    CHECK_EQ_U64(sizeof two_blocks - 1u, 56);
    CHECK(digest_is(two_blocks, 56u, 0u,
                    "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"));
    /* Byte-at-a-time updates give the same digest. */
    CHECK(digest_is(two_blocks, 56u, 1u,
                    "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"));
    CHECK_DONE();
}
