#define STITCH_IMPLEMENTATION
#define STITCH_STRIP_PREFIX
#include "stitch.h"
#include "config.h"
#include "folders.h"

int main(void)
{
    Cmd cmd = {0};
#ifdef FOO
    stitch_log(INFO, "FOO feature is enabled");
#endif // FOO
#ifdef BAR
    stitch_log(INFO, "BAR feature is enabled");
#endif // BAR
    const char *output_path = BUILD_FOLDER"main";
    const char *input_path = SRC_FOLDER"main.c";
    cmd_append(&cmd, "gcc", "-Wall", "-Wextra", "-ggdb", "-I"BUILD_FOLDER, "-I.", "-o", output_path, input_path);
    if (!cmd_run_sync_and_reset(&cmd)) return 1;
    return 0;
}
