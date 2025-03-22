#define STITCH_IMPLEMENTATION
#include "stitch.h"

void log_test_messages(void)
{
    stitch_log(STITCH_INFO, "Info Test Message");
    stitch_log(STITCH_WARNING, "Warning Test Message");
    stitch_log(STITCH_ERROR, "Error Test Message");
    stitch_log(STITCH_NO_LOGS, "YOU SHOULD NEVER SEE THIS");
}

int main(void)
{
    log_test_messages();
    stitch_minimal_log_level = STITCH_WARNING;
    log_test_messages();
    stitch_minimal_log_level = STITCH_ERROR;
    log_test_messages();
    stitch_minimal_log_level = STITCH_NO_LOGS;
    log_test_messages();
    return 0;
}
