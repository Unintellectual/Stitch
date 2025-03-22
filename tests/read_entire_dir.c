#define STITCH_IMPLEMENTATION
#define STITCH_STRIP_PREFIX
#include "stitch.h"
#include "shared.h"

int main(void)
{

    Stitch_File_Paths children = {0};
    if (!stitch_read_entire_dir(TESTS_FOLDER, &children)) return 1;
    stitch_log(INFO, "Tests:");
    for (size_t i = 0; i < children.count; ++i) {
        if (*children.items[i] != '.') {
            stitch_log(INFO, "    %s", children.items[i]);
        }
    }
    return 0;
}
