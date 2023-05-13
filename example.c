#define TTTL_IMPLEMENTATION
#define TTTL_LOG_USE_COLOR
#include "tttl.h"

int main(void) {
    tttl_log_set_level(TTTL_LOG_TRACE);
    tttl_log_add_file_callback("./", TTTL_LOG_TRACE);

    ltrace("Hello %s", "world");
    ldebug("Hello %s", "world");
    linfo("Hello %s", "world");
    lwarn("Hello %s", "world");
    lerror("Hello %s", "world");
    lfatal("Hello %s", "world");

    tttl_log_set_quiet(true);

    for (int i = 0; i < (TTTL_MAX_FILE_LINE + 100); i++) {
        linfo("Hello %d", i);
    }

    tttl_log_close();
}
