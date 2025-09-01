#include "linc.h"

#include <inttypes.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <time.h>

// ==================================================
// Global Variables
// ==================================================

static struct linc linc_global;

#if !defined(__GNUC__)
static pthread_once_t linc_once_init = PTHREAD_ONCE_INIT;
#define LINC_BOOTSTRAP(pthread_once, routine) pthread_once((pthread_once), (routine))
#else
#define LINC_BOOTSTRAP(pthread_once, routine)
#endif

static int64_t timestamp_offset = 0;

// ==================================================
// General Functions
// ==================================================

static int64_t linc_timestamp(void) {
    struct timespec ts;
#ifdef CLOCK_MONOTONIC_RAW
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
#else
    clock_gettime(CLOCK_MONOTONIC, &ts);
#endif
    int64_t mono = (int64_t)ts.tv_sec * 1000000000L + (int64_t)ts.tv_nsec;
    return mono + timestamp_offset;
}

static int linc_timestamp_string(int64_t timestamp, char *buffer, size_t size) {
    time_t sec = timestamp / 1000000000L;
    int16_t msec = (timestamp / 1000000L) % 1000;
    struct tm utc_tm;
    gmtime_r(&sec, &utc_tm);
    int result =
        snprintf(buffer,
                 size,
                 "%04" PRId16 "-%02" PRId16 "-%02" PRId16 " %02" PRId16 ":%02" PRId16 ":%02" PRId16 ".%03" PRId16,
                 utc_tm.tm_year + 1900,
                 utc_tm.tm_mon + 1,
                 utc_tm.tm_mday,
                 utc_tm.tm_hour,
                 utc_tm.tm_min,
                 utc_tm.tm_sec,
                 msec);
    if (result < 0) {
        return -1;
    }
    return 0;
}

// ==================================================
// Module Functions
// ==================================================

static int linc_modules_exists(const char *module_name, struct linc_module *modules, size_t count) {
    if (module_name == NULL || modules == NULL) {
        return -1;
    }
    for (size_t i = 0; i < count; i++) {
        if (strcmp(modules[i].name, module_name) == 0) {
            return i;
        }
    }
    return -1;
}

static int linc_modules_check_level(const char *module_name, enum linc_level level) {
    if (level < LINC_LEVEL_TRACE || level > LINC_LEVEL_FATAL) {
        return -1;
    }
    int module_index = linc_modules_exists(module_name, linc_global.modules_list, linc_global.modules_count);
    if (module_index < 0) {
        return -1;
    }
    bool is_module_enabled = linc_global.modules_list[module_index].enabled == true;
    enum linc_level module_level = linc_global.modules_list[module_index].level == LINC_LEVEL_INHERIT
                                     ? linc_global.level
                                     : linc_global.modules_list[module_index].level;
    bool is_level_valid = level >= module_level;
    if (is_module_enabled == false || is_level_valid == false) {
        return -1;
    }
    return 0;
}

// ==================================================
// Sink Functions
// ==================================================

static const char *linc_level_string(enum linc_level level) {
    switch (level) {
        case LINC_LEVEL_TRACE:
            return "TRACE";
        case LINC_LEVEL_DEBUG:
            return "DEBUG";
        case LINC_LEVEL_INFO:
            return "INFO";
        case LINC_LEVEL_WARN:
            return "WARN";
        case LINC_LEVEL_ERROR:
            return "ERROR";
        case LINC_LEVEL_FATAL:
            return "FATAL";
        default:
            return "UNKN";
    }
}

static int sink_stdout_open(void *data) {
    (void)data;
    return 0;
}

static int sink_stdout_close(void *data) {
    (void)data;
    return 0;
}

static int sink_stdout_write(void *data, const char *buffer, size_t length) {
    FILE *output_file = (FILE *)data;
    return fwrite(buffer, sizeof(char), length, output_file);
}

static int sink_stdout_flush(void *data) {
    FILE *output_file = (FILE *)data;
    return fflush(output_file);
}

// ==================================================
// State Management
// ==================================================

static void linc_shutdown(void) {}

LINC_ATSTART
static void linc_bootstrap(void) {
    // Clear global state
    memset(&linc_global, 0, sizeof(linc_global));

    // Timestamp
    struct timespec mono, real;
#ifdef CLOCK_MONOTONIC_RAW
    clock_gettime(CLOCK_MONOTONIC_RAW, &mono);
#else
    clock_gettime(CLOCK_MONOTONIC, &mono);
#endif
    clock_gettime(CLOCK_REALTIME, &real);
    int64_t mono_ns = (int64_t)mono.tv_sec * 1000000000L + (int64_t)mono.tv_nsec;
    int64_t real_ns = (int64_t)real.tv_sec * 1000000000L + (int64_t)real.tv_nsec;
    timestamp_offset = real_ns - mono_ns;

    // Bootstrap default module
    linc_global.modules_count = 1;
    linc_global.modules_list[0].enabled = true;
    linc_global.modules_list[0].level = LINC_LEVEL_INHERIT;
    strcpy(linc_global.modules_list[0].name, LINC_MODULES_DEFAULT_NAME);

    // Bootstrap default sink
    struct linc_sink_entry entry = {
        .data = stderr,
        .open = sink_stdout_open,
        .close = sink_stdout_close,
        .write = sink_stdout_write,
        .flush = sink_stdout_flush,
    };
    linc_global.sinks_count = 1;
    linc_global.sinks_list[0].enabled = true;
    linc_global.sinks_list[0].level = LINC_LEVEL_INHERIT;
    linc_global.sinks_list[0].formatter = LINC_FORMAT_TEXT;
    linc_global.sinks_list[0].color_mode = LINC_COLOR_MODE_AUTO;
    linc_global.sinks_list[0].entry = entry;

    // Bootstrap default settings
    linc_global.level = LINC_LEVEL_DEFAULT;

    // At exit shutdown function
    atexit(linc_shutdown);
}

// ==================================================
// Client Functions
// ==================================================

void linc_log(const char *module,
              enum linc_level level,
              const char *file,
              uint32_t line,
              const char *func,
              const char *format,
              ...) {
    LINC_BOOTSTRAP(&linc_once_init, linc_bootstrap);

    // Validate module and level
    const char *module_name = module == NULL ? LINC_MODULES_DEFAULT_NAME : module;
    int is_level_valid = linc_modules_check_level(module_name, level);
    if (is_level_valid < 0) {
        return;
    }

    // Temprorary code
    // This code will be inserted into worker thread later

    int64_t timestamp = linc_timestamp();
    char timestamp_string[LINC_TIMESTAMP_LENGTH];
    if (linc_timestamp_string(timestamp, timestamp_string, LINC_TIMESTAMP_LENGTH) < 0) {
        strcpy(timestamp_string, LINC_TIMESTAMP_FALLBACK);
    }
    uintptr_t thread_id = (uintptr_t)pthread_self();

    char message[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    printf("[ %s ] [ %-5s ] [ %016" PRIxPTR " ] [ %-" LINC_NUM_TO_STR(LINC_MODULES_NAME_LENGTH) "s ] %s:%" PRIu32
                                                                                                " %s: %s\n",
           timestamp_string,
           linc_level_string(level),
           thread_id,
           module_name,
           file,
           line,
           func,
           message);
}
