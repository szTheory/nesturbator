/* The runner's save name rule (D-18): the last component of the ROM path,
   split at '/' or '\' on every platform, minus its last extension unless the
   dot is the first character, plus ".sav" inside the save directory. */
#include <string.h>

#include "save.h"
#include "../check.h"

static int path_is(const char *dir, const char *rom, const char *want)
{
    char buf[256];
    return nesturbator_run_save_path(buf, sizeof buf, dir, rom) == 1 && strcmp(buf, want) == 0;
}

int main(void)
{
    char small[16];
    CHECK(path_is("dir", "a/b/game.nes", "dir/game.sav"));
    CHECK(path_is("dir", "a\\b\\game.nes", "dir/game.sav"));
    CHECK(path_is("dir", "a/b\\game.nes", "dir/game.sav"));
    CHECK(path_is("dir", "game.v1.nes", "dir/game.v1.sav"));
    CHECK(path_is("dir", "game", "dir/game.sav"));
    CHECK(path_is("dir", ".hidden", "dir/.hidden.sav"));
    CHECK(path_is("dir", "a/.hidden.nes", "dir/.hidden.sav"));
    CHECK(path_is("/abs/saves", "../up/game.nes", "/abs/saves/game.sav"));
    /* "dir/game.sav" is 12 characters, so 13 bytes hold it and 12 do not. */
    CHECK(nesturbator_run_save_path(small, 13u, "dir", "game.nes") == 1);
    CHECK(nesturbator_run_save_path(small, 12u, "dir", "game.nes") == 0);
    CHECK(strcmp(small, "dir/game.sav") == 0);
    CHECK_DONE();
}
