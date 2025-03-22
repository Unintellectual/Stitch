#define STITCH_IMPLEMENTATION
#define STITCH_STRIP_PREFIX
#include "stitch.h"

int main(void)
{
    const char *test_current_dir = "/";

    const char *current_dir = get_current_dir_temp();
    if (current_dir == NULL) return 1;
    stitch_log(INFO, "Current Dir: %s", current_dir);

    stitch_log(INFO, "Setting Current Dir to %s", test_current_dir);
    if (!set_current_dir(test_current_dir)) return 1;
    
    current_dir = get_current_dir_temp();
    if (current_dir == NULL) return 1;
    stitch_log(INFO, "Current Dir: %s", get_current_dir_temp());
    return 0;
}
