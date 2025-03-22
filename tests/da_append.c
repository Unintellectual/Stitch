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
    for (int x = 0; x < 10; ++x) {
        da_append(&xs, x);
        stitch_log(INFO, "count = %zu, capacity = %zu", xs.count, xs.capacity);
    }
    return 0;
}
