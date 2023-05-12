#ifndef _TTTL_H_
#define _TTTL_H_

#include <time.h>
#include <stdio.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <threads.h>

#ifndef TTTL_MAX_CALLBACKS
#define TTTL_MAX_CALLBACKS 64
#endif // TTTL_MAX_CALLBACKS

#define tttl_malloc(s) malloc(s)
#define tttl_free(p) free(p)

enum TTTL_MutexState { TTTL_MUTEX_LOCK = 0, TTTL_MUTEX_UNLOCK = 1 };

enum TTTL_LogLevel { TTTL_LOG_TRACE = 0, TTTL_LOG_DEBUG, TTTL_LOG_INFO,
                     TTTL_LOG_WARN, TTTL_LOG_ERROR, TTTL_LOG_FATAL };

enum TTTL_LogMode { TTTL_LOG_MODE_SYNC = 0, TTTL_LOG_MODE_ASYNC = 1 };

#ifdef TTTL_WITH_PREFIX

#define tttl_trace(...) tttl_log_log(TTTL_LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define tttl_debug(...) tttl_log_log(TTTL_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define tttl_info(...) tttl_log_log(TTTL_LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define tttl_warn(...) tttl_log_log(TTTL_LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define tttl_error(...) tttl_log_log(TTTL_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define tttl_fatal(...) tttl_log_log(TTTL_LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#else

#define ltrace(...) tttl_log_log(TTTL_LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define ldebug(...) tttl_log_log(TTTL_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define linfo(...) tttl_log_log(TTTL_LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define lwarn(...) tttl_log_log(TTTL_LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define lerror(...) tttl_log_log(TTTL_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define lfatal(...) tttl_log_log(TTTL_LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#endif // TTTL_WITH_PREFIX

static const char *tttl_log_level_string[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef TTTL_LOG_USE_COLOR
static const char *tttl_log_level_color[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif // TTTL_LOG_USE_COLOR
       //
typedef struct TTTL_Mutex {
    atomic_uint value;
} TTTL_Mutex_t;

void tttl_mutex_lock(struct TTTL_Mutex *self);
void tttl_mutex_unlock(struct TTTL_Mutex *self);

typedef struct TTTL_LogEvent {
    enum TTTL_LogLevel level;
    struct tm *time;
    va_list ap;
    const char *fmt;
    const char *file;
    int32_t line;
    void *data;
} TTTL_LogEvent_t;

void tttl_log_event_set_data(struct TTTL_LogEvent *self, void *data);

typedef void (*TTTL_LogFn)(struct TTTL_LogEvent *ev);

typedef struct TTTL_Callback {
    enum TTTL_LogLevel level;
    TTTL_LogFn fn;
    void *data;
} TTTL_Callback_t;

typedef struct TTTL_Log {
    enum TTTL_LogLevel level;
    int mode;
    bool quiet;
    struct TTTL_Mutex mutex;
    struct TTTL_Callback callback[TTTL_MAX_CALLBACKS];
    void *data;
} TTTL_Log_t;

void tttl_log_log(enum TTTL_LogLevel level,
                  const char* file,
                  int32_t line,
                  const char* fmt, ...);
void tttl_log_set_quiet(bool enable);
void tttl_log_set_mode(enum TTTL_LogMode mode);
void tttl_log_set_level(enum TTTL_LogLevel level);
void tttl_log_add_callback(struct TTTL_Callback callback);
void tttl_log_add_file_callback(const char* file_path);

#ifdef TTTL_IMPLEMENTATION

// Event implementation

void tttl_log_event_set_data(struct TTTL_LogEvent *self, void *data) {
    self->data = data;
}

// Log implementation

static void tttl_logfn_stdout(struct TTTL_LogEvent *ev) {
    char buf[16];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';
#ifdef TTTL_LOG_USE_COLOR
    fprintf(
        (FILE *)ev->data, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
        buf, tttl_log_level_color[ev->level],
        tttl_log_level_string[ev->level],
        ev->file, ev->line);
#else
    fprintf(
        (FILE *)ev->data, "%s %-5s %s:%d ",
        buf, tttl_log_level_string[ev->level],
        ev->file, ev->line);
#endif // TTTL_LOG_USE_COLOR
    vfprintf((FILE *)ev->data, ev->fmt, ev->ap);
    fprintf((FILE *)ev->data, "\n");
    fflush((FILE *)ev->data);
}

static void tttl_logfn_file(struct TTTL_LogEvent *ev) {
    char buf[16];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';
    fprintf(
        (FILE *)ev->data, "%s %-5s %s:%d ",
        buf, tttl_log_level_string[ev->level],
        ev->file, ev->line);
    vfprintf((FILE *)ev->data, ev->fmt, ev->ap);
    fprintf((FILE *)ev->data, "\n");
    fflush((FILE *)ev->data);
}

static struct TTTL_Log tttl_log = {
    .level = TTTL_LOG_DEBUG,
    .mode = TTTL_LOG_MODE_SYNC,
    .quiet = false,
    .mutex = { .value = TTTL_MUTEX_UNLOCK },
};

void tttl_log_log(enum TTTL_LogLevel level,
                  const char* file,
                  int32_t line,
                  const char* fmt, ...) {
    struct TTTL_LogEvent ev = {
        .level = level,
        .fmt = fmt,
        .file = file,
        .line = line,
    };
    if (tttl_log.mode == TTTL_LOG_MODE_ASYNC)
        tttl_mutex_lock(&tttl_log.mutex);

    if (!tttl_log.quiet && level >= tttl_log.level) {
        tttl_log_event_set_data(&ev, stderr);
        va_start(ev.ap, fmt);
        tttl_logfn_stdout(&ev);
        va_end(ev.ap);
    }

    for (int i = 0; i < TTTL_MAX_CALLBACKS && tttl_log.callback[i].fn; i++) {
        struct TTTL_Callback *callback = &tttl_log.callback[i];
        if (level >= callback->level) {
            tttl_log_event_set_data(&ev, callback->data);
            va_start(ev.ap, fmt);
            callback->fn(&ev);
            va_end(ev.ap);
        }
    }

    if (tttl_log.mode == TTTL_LOG_MODE_ASYNC)
        tttl_mutex_unlock(&tttl_log.mutex);
}

void tttl_log_set_quiet(bool enable) {
    tttl_log.quiet = enable;
}

void tttl_log_set_mode(enum TTTL_LogMode mode) {
    tttl_log.mode = mode;
}

void tttl_log_set_level(enum TTTL_LogLevel level) {
    tttl_log.level = level;
}

void tttl_log_add_file_callback(const char *file_path) {
}

// Mutex implementation

void tttl_mutex_lock(struct TTTL_Mutex *self) {
    assert(self != NULL);
    if (self->value == TTTL_MUTEX_UNLOCK)
        self->value = TTTL_MUTEX_LOCK;
}

void tttl_mutex_unlock(struct TTTL_Mutex *self) {
    assert(self != NULL);
    if (self->value == TTTL_MUTEX_LOCK)
        self->value = TTTL_MUTEX_UNLOCK;
}

#endif // TTTL_IMPLEMENTATION

#undef tttl_malloc
#undef tttl_free

#endif // _TTTL_H_
