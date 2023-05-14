<div align="center">
    <h1>tinytinytinylog.h (tttl.h)</h1>
    <p>A tiny log library (header-only) written in C.</p>
</div>

## Usage

```c
#define TTTL_IMPLEMENTATION // stb style header file needs
#define TTTL_LOG_USE_COLOR  // enable colorful output
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
```

Notice: The header file is [stb style](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt).
Don't forget to put `#define TTTL_IMPLEMENTATION` in your file before including this file.

```c
#define TTTL_IMLEMENTATION

#include "tttl.h"
```

## Api

```c
// set log system quiet (The log info will not be printed out into stdout.)
void tttl_log_set_quiet(bool enable);

// TODO: set mode: Async | Sync
// void tttl_log_set_mode(enum TTTL_LogMode mode);

// set global log level
void tttl_log_set_level(enum TTTL_LogLevel level);

// add custom callback
bool tttl_log_add_callback(struct TTTL_Callback callback);

// add file callback to make log system save the log info in files
bool tttl_log_add_file_callback(const char* file_path, enum TTTL_LogLevel level);

// close log system and release the resource
void tttl_log_close();
```
