#define STITCH_IMPLEMENTATION
#define STITCH_STRIP_PREFIX
#include "stitch.h"
#include "src_build/folders.h"

int main(int argc, char **argv)
{
    STITCH_GO_REBUILD_URSELF_PLUS(argc, argv, "stitch.h", "src_build/folders.h");

    if (!stitch_mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    Cmd cmd = {0};
    Stitch_String_Builder sb = {0};

    const char *conf_path = BUILD_FOLDER"config.h";
    int exists = file_exists(conf_path);
    if (exists < 0) return 1;
    if (exists == 0) {
        stitch_log(INFO, "Generating initial %s", conf_path);
        sb_append_cstr(&sb, "#ifndef CONFIG_H_\n");
        sb_append_cstr(&sb, "#define CONFIG_H_\n");
        sb_append_cstr(&sb, "// #define FOO // Enables FOO feature\n");
        sb_append_cstr(&sb, "// #define BAR // Enables BAR feature\n");
        sb_append_cstr(&sb, "#endif // CONFIG_H_\n");
        if (!stitch_write_entire_file(conf_path, sb.items, sb.count)) return 1;
        sb.count = 0;
        stitch_log(INFO, "==================================");
        stitch_log(INFO, "EDIT %s TO CONFIGURE YOUR BUILD!!!", conf_path);
        stitch_log(INFO, "==================================");
    }

    // TODO: add support for MSVC
    // TODO: build and run this how_to on GitHub CI
    const char *output_path = BUILD_FOLDER"stitch_configed";
    const char *input_path = SRC_BUILD_FOLDER"stitch_configed.c";
    cmd_append(&cmd,
               "gcc",
               "-Wall", "-Wextra", "-ggdb",
               "-I.", "-I"BUILD_FOLDER, "-I"SRC_BUILD_FOLDER,
               "-o", output_path, input_path);
    if (!cmd_run_sync_and_reset(&cmd)) return 1;

    cmd_append(&cmd, output_path);
    if (!cmd_run_sync_and_reset(&cmd)) return 1;

    return 0;
}
