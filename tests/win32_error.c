#define STITCH_IMPLEMENTATION
#include "stitch.h"

int main(void) {
    stitch_log(STITCH_ERROR, "First 100 Win32 API Errors:");
    for (DWORD i = 0; i < 100; i++)
        stitch_log(STITCH_ERROR, "%lu (0x%X): \"%s\"", i, i, stitch_win32_error_message(i));
    return 0;
}
