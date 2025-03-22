#define STITCH_IMPLEMENTATION
#define STITCH_STRIP_PREFIX
#define STITCH_DA_INIT_CAP 4
#include "stitch.h"

typedef struct {
    int *items;
    size_t count;
    size_t capacity;
} Numbers;

int main(void)
{
    Numbers xs = {0};
    da_append(&xs, 69);
    da_append(&xs, 420);
    da_append(&xs, 1337);
    stitch_log(INFO, "count = %zu, capacity = %zu", xs.count, xs.capacity);
    stitch_da_resize(&xs, 1);
    stitch_log(INFO, "count = %zu, capacity = %zu", xs.count, xs.capacity);
    stitch_da_resize(&xs, 10);
    stitch_log(INFO, "count = %zu, capacity = %zu", xs.count, xs.capacity);
    return 0;
}
