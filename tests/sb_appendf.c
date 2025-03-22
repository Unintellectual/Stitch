#define STITCH_IMPLEMENTATION
#define STITCH_STRIP_PREFIX
#define STITCH_DA_INIT_CAP 4
#include "stitch.h"

int main(void)
{
    String_Builder sb = {0};
    stitch_log(INFO, "count = %zu, capacity = %zu, items = \"%.*s\"", sb.count, sb.capacity, (int)sb.count, sb.items);
    sb_appendf(&sb, "Hello, %d.", 69);
    stitch_log(INFO, "count = %zu, capacity = %zu, items = \"%.*s\"", sb.count, sb.capacity, (int)sb.count, sb.items);
    return 0;
}
