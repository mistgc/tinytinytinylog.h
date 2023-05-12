#define TTTL_IMPLEMENTATION
#define TTTL_LOG_USE_COLOR
#include "tttl.h"

int main(void) {
    tttl_log_set_level(TTTL_LOG_TRACE);

    ltrace("Hello %s", "world");
    ldebug("Hello %s", "world");
    linfo("Hello %s", "world");
    lwarn("Hello %s", "world");
    lerror("Hello %s", "world");
    lfatal("Hello %s", "world");
}
