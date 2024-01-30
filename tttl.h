#ifndef _TTTL_H_
#define _TTTL_H_

#include <time.h>
#include <stdio.h>
#ifndef __cplusplus
/* improve compatibility for C++ project */
#include <stdatomic.h>
#endif // __cplusplus
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#ifndef TTTL_MAX_CALLBACKS
#define TTTL_MAX_CALLBACKS 64
#endif // TTTL_MAX_CALLBACKS

#ifndef TTTL_MAX_FILE_LINE
#define TTTL_MAX_FILE_LINE (1<<12) // 4096
#endif // TTTL_MAX_FILE_LINE

#define PATH_SEPARATOR '/'

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

#ifndef __cplusplus
typedef struct TTTL_Mutex {
    _Atomic int value;
} TTTL_Mutex_t;
#else
/*
 * Because C++ doesn't have the Keyword '_Atomic',
 * here has to hide members of the structure.
 *
 * DON'T WORRY!!!
 * All associated implementations are in the .c file.
 *
 * If you use this log library in your C++ project,
 * you just create a .c file in your project.
 * And put the "#define TTTL_IMPLEMENTATION" line
 * before including the "tttl.h". Then compile
 * the .c file into .o file and link it to your project.
 */
typedef struct TTTL_Mutex {} TTTL_Mutex_t;
#endif //__cplusplus

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
void tttl_log_event_init_time(struct TTTL_LogEvent *self);
bool tttl_log_event_is_same_day(struct TTTL_LogEvent *self);
/* private:
 *     bool tttl_log_event_maybe_update_log_file(struct TTTL_LogEvent *self);
 */

typedef void (*TTTL_LogFn)(struct TTTL_LogEvent *ev);
typedef void *(*TTTL_Fn)(void *data);

typedef struct TTTL_Callback {
    enum TTTL_LogLevel level;
    TTTL_LogFn logfn;
    TTTL_Fn fn;
    void *data;
} TTTL_Callback_t;

typedef struct TTTL_LogFile {
    const char *file_path;
    char *file_name;
    int index;
    int max_line;
    int count_line;
    FILE *fp;
} TTTL_LogFile_t;

void tttl_log_file_close(struct TTTL_LogFile *self);

typedef struct TTTL_Log {
    enum TTTL_LogLevel level;
    int mode;
    bool quiet;
    struct TTTL_Mutex mutex;
    struct TTTL_Callback callbacks[TTTL_MAX_CALLBACKS];
    void *data;
} TTTL_Log_t;

void tttl_log_log(enum TTTL_LogLevel level,
                  const char* file,
                  int32_t line,
                  const char* fmt, ...);
void tttl_log_set_quiet(bool enable);
void tttl_log_set_mode(enum TTTL_LogMode mode);
void tttl_log_set_level(enum TTTL_LogLevel level);
bool tttl_log_add_callback(struct TTTL_Callback callback);
bool tttl_log_add_file_callback(const char* file_path, enum TTTL_LogLevel level);
void tttl_log_close();

#ifdef TTTL_IMPLEMENTATION

// Event implementation

void tttl_log_event_set_data(struct TTTL_LogEvent *self, void *data) {
    self->data = data;
}

void tttl_log_event_init_time(struct TTTL_LogEvent *self) {
    if (!self->time) {
        time_t t = time(NULL);
        self->time = localtime(&t);
    }
}

bool tttl_log_event_is_same_day(struct TTTL_LogEvent *self) {
    time_t t = time(NULL);
    struct tm *time = self->time;
    struct tm *now = localtime(&t);

    if (!strncmp(time->tm_zone, now->tm_zone, sizeof(now->tm_zone)) &&
        time->tm_year == now->tm_year &&
        time->tm_mon == now->tm_mon &&
        time->tm_mday == now->tm_mday) {

        // tttl_free(now);
        return true;
    }
    // tttl_free(now);
    return false;
}

static bool tttl_log_event_maybe_update_log_file(struct TTTL_LogEvent *self) {
    struct TTTL_LogFile *log_file = self->data;

    if (log_file->file_name == NULL) {
        char buf[256];
        buf[strftime(buf, sizeof(buf), "%F", self->time)] = '\0';
        buf[sprintf(buf, "%s.log", buf)] = '\0';
        char *file_name = strdup(buf);
        sprintf(buf, "%s%c%s", log_file->file_path, PATH_SEPARATOR, file_name);
        FILE *fp = fopen(buf, "a");

        log_file->file_name = file_name;
        log_file->fp = fp;

        return true;
    }

    if (tttl_log_event_is_same_day(self)) {
        if (log_file->count_line >= log_file->max_line) {
            fclose(log_file->fp);
            log_file->index++;

            char buf[256];
            sprintf(buf, "%s%c%s.%d", log_file->file_path,
                    PATH_SEPARATOR, log_file->file_name,
                    log_file->index);
            FILE *fp = fopen(buf, "a");

            log_file->fp = fp;
            log_file->count_line = 0;
            return true;
        }
        return false;
    } else {
        tttl_free(log_file->file_name);
        fclose(log_file->fp);

        char buf[256];
        buf[strftime(buf, sizeof(buf), "%F", self->time)] = '\0';
        buf[sprintf(buf, "%s.log", buf)] = '\0';
        char *file_name = strdup(buf);
        sprintf(buf, "%s%c%s", log_file->file_path, PATH_SEPARATOR, log_file->file_name);
        FILE *fp = fopen(buf, "a");

        log_file->file_name = file_name;
        log_file->fp = fp;

        return true;
    }
}

// Log File implementation

void tttl_log_file_close(struct TTTL_LogFile *self) {
    assert(self != NULL);
    if (self->file_name != NULL) {
        tttl_free(self->file_name);
    }
    if (self->fp != NULL) {
        fclose(self->fp);
    }
    tttl_free(self);
}

// Log implementation

static void tttl_logfn_stdout(struct TTTL_LogEvent *ev) {
    char buf[32];
    buf[strftime(buf, sizeof(buf), "%F %T", ev->time)] = '\0';
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
    tttl_log_event_maybe_update_log_file(ev);
    struct TTTL_LogFile *log_file = ev->data;
    FILE *fp = log_file->fp;
    char buf[32];
    buf[strftime(buf, sizeof(buf), "%F %T", ev->time)] = '\0';
    fprintf(
        fp, "%s %-5s %s:%d ", buf,
        tttl_log_level_string[ev->level],
        ev->file, ev->line);
    vfprintf(fp, ev->fmt, ev->ap);
    fprintf(fp, "\n");
    fflush(fp);
    log_file->count_line++;
}

static void *tttl_fn_handle_logfn_file(void *data) {
    struct TTTL_LogFile *f = data;
    tttl_log_file_close(f);
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

    tttl_log_event_init_time(&ev);

    if (tttl_log.mode == TTTL_LOG_MODE_ASYNC)
        tttl_mutex_lock(&tttl_log.mutex);

    if (!tttl_log.quiet && level >= tttl_log.level) {
        tttl_log_event_set_data(&ev, stderr);
        va_start(ev.ap, fmt);
        tttl_logfn_stdout(&ev);
        va_end(ev.ap);
    }

    for (int i = 0; i < TTTL_MAX_CALLBACKS && tttl_log.callbacks[i].logfn; i++) {
        struct TTTL_Callback *callback = &tttl_log.callbacks[i];
        if (level >= callback->level) {
            tttl_log_event_set_data(&ev, callback->data);
            va_start(ev.ap, fmt);
            callback->logfn(&ev);
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

bool tttl_log_add_callback(struct TTTL_Callback callback) {
    for (int i = 0; i < TTTL_MAX_CALLBACKS; i++) {
        if (!tttl_log.callbacks[i].logfn) {
            tttl_log.callbacks[i] = callback;
            return true;
        }
    }

    return false;
}

bool tttl_log_add_file_callback(const char* file_path, enum TTTL_LogLevel level) {
    struct TTTL_LogFile *log_file = (struct TTTL_LogFile *)tttl_malloc(sizeof(struct TTTL_LogFile));

    log_file->file_path = file_path;
    log_file->file_name = NULL;
    log_file->index = 0;
    log_file->max_line = TTTL_MAX_FILE_LINE;
    log_file->count_line = 0;
    log_file->fp = NULL;

    struct TTTL_Callback callback = {
        .level = level,
        .logfn = tttl_logfn_file,
        .fn = tttl_fn_handle_logfn_file,
        .data = log_file,
    };

    tttl_log_add_callback(callback);

    return true;
}

void tttl_log_close() {
    for (int i = 0; i < TTTL_MAX_CALLBACKS && tttl_log.callbacks[i].fn; i++) {
        struct TTTL_Callback *callback = &tttl_log.callbacks[i];
        callback->fn(callback->data);
    }
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
