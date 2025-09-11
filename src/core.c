#include "internal/shared.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ==================================================
// Global State
// ==================================================

static struct linc linc;
struct linc_module *linc_default_module;
struct linc_sink *linc_default_sink;
static int64_t timestamp_offset = 0;

// ==================================================
// Utils Functions
// ==================================================

#if defined(CLOCK_MONOTONIC_RAW)
#define CLOCK_GETTIME(ts) clock_gettime(CLOCK_MONOTONIC_RAW, ts)
#else
#define CLOCK_GETTIME(ts) clock_gettime(CLOCK_MONOTONIC, ts)
#endif

static void linc_timestamp_offset(void) {
    if (timestamp_offset != 0) {
        return;
    }

    struct timespec mono, real;
    CLOCK_GETTIME(&mono);
    clock_gettime(CLOCK_REALTIME, &real);
    int64_t mono_ns = (int64_t)mono.tv_sec * 1000000000L + (int64_t)mono.tv_nsec;
    int64_t real_ns = (int64_t)real.tv_sec * 1000000000L + (int64_t)real.tv_nsec;
    timestamp_offset = real_ns - mono_ns;
}

// ==================================================
// Life cycle Management
// ==================================================

#if defined(__GNUC__)
#define LINC_AT_START __attribute__((constructor))
#define LINC_BOOTSTRAP(pthread_once, routine)
#else
#define LINC_AT_START
static pthread_once_t linc_once_init = PTHREAD_ONCE_INIT;
#define LINC_BOOTSTRAP(pthread_once, routine) pthread_once((pthread_once), (routine))
#endif

static void linc_shutdown(void) {
    for (size_t i = 0; i < linc.sinks.count; i++) {
        struct linc_sink *sink = &linc.sinks.list[i];
        sink->funcs.flush(sink->funcs.data);
        sink->funcs.close(sink->funcs.data);
    }

    memset(&linc, 0, sizeof(linc));
}

LINC_AT_START
static void linc_bootstrap(void) {
    memset(&linc, 0, sizeof(linc));

    linc_timestamp_offset();
    linc.level = LINC_DEFAULT_LEVEL;
    linc_default_module = linc_register_default_module(&linc.modules);
    linc_default_sink = linc_register_default_sink(&linc.sinks);

    atexit(linc_shutdown);
}

void linc_init(void) {
    LINC_BOOTSTRAP(&linc_once_init, linc_bootstrap);
}

// ==================================================
// State Management
// ==================================================

struct linc *linc_get_state(void) {
    LINC_BOOTSTRAP(&linc_once_init, linc_bootstrap);
    return &linc;
}

enum linc_level linc_get_level(void) {
    LINC_BOOTSTRAP(&linc_once_init, linc_bootstrap);
    return linc.level;
}

struct linc_module_list *linc_get_modules(void) {
    LINC_BOOTSTRAP(&linc_once_init, linc_bootstrap);
    return &linc.modules;
}

struct linc_sink_list *linc_get_sinks(void) {
    LINC_BOOTSTRAP(&linc_once_init, linc_bootstrap);
    return &linc.sinks;
}

// ==================================================
// Public Functions
// ==================================================

int linc_set_level(enum linc_level level) {
    LINC_BOOTSTRAP(&linc_once_init, linc_bootstrap);
    if (level < LINC_LEVEL_TRACE || level > LINC_LEVEL_FATAL) {
        return -1;
    }
    linc.level = level;
    return 0;
}

int64_t linc_timestamp(void) {
    struct timespec ts;
    CLOCK_GETTIME(&ts);
    int64_t mono = (int64_t)ts.tv_sec * 1000000000L + (int64_t)ts.tv_nsec;
    return mono + timestamp_offset;
}

int linc_timestamp_string(int64_t timestamp, char *buffer, size_t size) {
    if (buffer == NULL) {
        return -1;
    }
    time_t sec = timestamp / 1000000000L;
    int64_t n_sec = timestamp % 1000000000L;
    if (n_sec < 0) {
        n_sec += 1000000000L;
        sec -= 1;
    }
    int m_sec = n_sec / 1000000L;
    struct tm utc_tm;
    gmtime_r(&sec, &utc_tm);
    int written = snprintf(buffer,
                           size,
                           "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                           utc_tm.tm_year + 1900,
                           utc_tm.tm_mon + 1,
                           utc_tm.tm_mday,
                           utc_tm.tm_hour,
                           utc_tm.tm_min,
                           utc_tm.tm_sec,
                           m_sec);
    if (written < 0 || (size_t)written >= size) {
        return -1;
    }
    return 0;
}

const char *linc_level_string(enum linc_level level) {
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

static const char *linc_level_color_ansi(enum linc_level level) {
    switch (level) {
        case LINC_LEVEL_TRACE:
            return LINC_COLOR_DIM LINC_COLOR_BLUE;
        case LINC_LEVEL_DEBUG:
            return LINC_COLOR_CYAN;
        case LINC_LEVEL_INFO:
            return LINC_COLOR_GREEN;
        case LINC_LEVEL_WARN:
            return LINC_COLOR_YELLOW;
        case LINC_LEVEL_ERROR:
            return LINC_COLOR_RED;
        case LINC_LEVEL_FATAL:
            return LINC_COLOR_BOLD LINC_COLOR_MAGENTA;
        default:
            return LINC_COLOR_WHITE;
    }
}

int linc_stringify_metadata(struct linc_metadata *metadata, char *buffer, size_t length, bool use_colors) {
    if (metadata == NULL || buffer == NULL) {
        return -1;
    }

    char timestamp_string[LINC_LOG_TIMESTAMP_LENGTH + LINC_ZERO_CHAR_LENGTH];
    if (linc_timestamp_string(metadata->timestamp, timestamp_string, sizeof(timestamp_string)) < 0) {
        strcpy(timestamp_string, "0000-00-00 00:00:00.000");
    }

    const char *module_name;
    if (metadata->module_name == NULL) {
        module_name = "unknown";
    } else {
        size_t name_length = strnlen(metadata->module_name, LINC_DEFAULT_MODULE_NAME_LENGTH + LINC_ZERO_CHAR_LENGTH);
        if (name_length == 0 || name_length > LINC_DEFAULT_MODULE_NAME_LENGTH) {
            return -1;
        }
        module_name = metadata->module_name;
    }
    const char *filename;
    if (metadata->filename == NULL) {
        filename = "unknown";
    } else {
        size_t filename_length = strnlen(metadata->filename, LINC_LOG_FILE_LENGTH + LINC_ZERO_CHAR_LENGTH);
        if (filename_length == 0 || filename_length > LINC_LOG_FILE_LENGTH) {
            return -1;
        }
        filename = metadata->filename;
    }
    const char *func;
    if (metadata->func == NULL) {
        func = "unknown";
    } else {
        size_t func_length = strnlen(metadata->func, LINC_LOG_FUNC_LENGTH + LINC_ZERO_CHAR_LENGTH);
        if (func_length == 0 || func_length > LINC_LOG_FUNC_LENGTH) {
            return -1;
        }
        func = metadata->func;
    }

    int written = snprintf(
        buffer,
        length,
        "[ %s%s%s ] "
        "[ %s%-" LINC_STRINGIFY(LINC_LOG_LEVEL_LENGTH) "s%s ] "
        "[ %s%0" LINC_STRINGIFY(LINC_LOG_THREAD_ID_LENGTH) PRIxPTR "%s ] "
        "[ %s%-" LINC_STRINGIFY(LINC_DEFAULT_MODULE_NAME_LENGTH) "s%s ] "
        "%s%s%s:"
        "%s%" PRIu32 "%s "
        "%s%s%s: "
        "%s\n",
        use_colors ? LINC_COLOR_BOLD : "",
        timestamp_string,
        use_colors ? LINC_COLOR_RESET : "",
        use_colors ? linc_level_color_ansi(metadata->level) : "",
        linc_level_string(metadata->level),
        use_colors ? LINC_COLOR_RESET : "",
        use_colors ? LINC_COLOR_BOLD : "",
        metadata->thread_id,
        use_colors ? LINC_COLOR_RESET : "",
        use_colors ? LINC_COLOR_BOLD : "",
        module_name,
        use_colors ? LINC_COLOR_RESET : "",
        use_colors ? LINC_COLOR_CYAN : "",
        filename,
        use_colors ? LINC_COLOR_RESET : "",
        use_colors ? LINC_COLOR_YELLOW : "",
        metadata->line,
        use_colors ? LINC_COLOR_RESET : "",
        use_colors ? LINC_COLOR_MAGENTA : "",
        func,
        use_colors ? LINC_COLOR_RESET : "",
        metadata->message);
    if (written < 0 || (size_t)written >= length) {
        return -1;
    }
    return written;
}
