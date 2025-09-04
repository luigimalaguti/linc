#include "linc.h"

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    int result = snprintf(buffer,
                          size,
                          "%04d-%02d-%02d %02d:%02d:%02d.%03d",
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
    return module_index;
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
    size_t written = fwrite(buffer, sizeof(char), length, output_file);
    return written == length ? 0 : -1;
}

static int sink_stdout_flush(void *data) {
    FILE *output_file = (FILE *)data;
    return fflush(output_file);
}

// ==================================================
// State Management
// ==================================================

static void linc_shutdown(void) {
    // Close all sinks
    for (size_t i = 0; i < linc_global.sinks_count; i++) {
        struct linc_sink *sink = &linc_global.sinks_list[i];
        sink->entry.flush(sink->entry.data);
        sink->entry.close(sink->entry.data);
    }

    // Clear global state
    memset(&linc_global, 0, sizeof(linc_global));
}

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
// Worker Functions
// ==================================================

static int linc_sinks_check_level(struct linc_sink *sink, enum linc_level level) {
    bool is_sink_enabled = sink->enabled == true;
    enum linc_level sink_level = sink->level == LINC_LEVEL_INHERIT ? linc_global.level : sink->level;
    bool is_level_valid = level >= sink_level;
    bool is_sink_writer_valid = sink->entry.write != NULL;
    if (is_sink_enabled == false || is_level_valid == false || is_sink_writer_valid == false) {
        return -1;
    }
    return 0;
}

static void linc_temp_worker(struct linc_entry *entry) {
    // Build timestamp string from entry timestamp
    char timestamp_string[LINC_TIMESTAMP_LENGTH];
    if (linc_timestamp_string(entry->timestamp, timestamp_string, LINC_TIMESTAMP_LENGTH) < 0) {
        strcpy(timestamp_string, LINC_TIMESTAMP_FALLBACK);
    }

    // Iterate over sinks and write log entry
    for (size_t i = 0; i < linc_global.sinks_count; i++) {
        // Check sink level
        struct linc_sink *sink = &linc_global.sinks_list[i];
        if (linc_sinks_check_level(sink, entry->level) < 0) {
            continue;
        }

        // Format log entry
        const char *file = entry->file == NULL ? "unknown" : entry->file;
        const char *func = entry->func == NULL ? "unknown" : entry->func;
        if (strlen(entry->message) == 0) {
            strcpy(entry->message, "(no message)");
        }
        char log_text_format[LINC_LOG_TEXT_MAX_LENGTH];
        int written = snprintf(log_text_format,
                               sizeof(log_text_format),
                               LINC_LOG_TEXT_FORMAT,
                               timestamp_string,
                               linc_level_string(entry->level),
                               entry->thread_id,
                               linc_global.modules_list[entry->module_index].name,
                               file,
                               entry->line,
                               func,
                               entry->message);
        if (written < 0) {
            strcpy(log_text_format, LINC_LOG_TEXT_FALLBACK);
            written = strlen(log_text_format);
        } else if ((size_t)written >= sizeof(log_text_format)) {
            written = sizeof(log_text_format) - 1;
        }

        // Write log entry to sink
        sink->entry.write(sink->entry.data, log_text_format, written);
    }

    // Free entry
    free(entry);
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
    int module_index = linc_modules_check_level(module_name, level);
    if (module_index < 0) {
        return;
    }

    // Create log entry
    struct linc_entry *entry = (struct linc_entry *)calloc(1, sizeof(struct linc_entry));
    if (entry == NULL) {
        return;
    }
    entry->timestamp = linc_timestamp();
    entry->level = level;
    entry->thread_id = (uintptr_t)pthread_self();
    entry->module_index = module_index;
    entry->file = file;
    entry->line = line;
    entry->func = func;

    // Format log message
    va_list args;
    va_start(args, format);
    if (format != NULL) {
        vsnprintf(entry->message, sizeof(entry->message), format, args);
    }
    va_end(args);

    // ==================================================
    // Temporary code
    // This code will be inserted into worker thread later
    // ==================================================
    linc_temp_worker(entry);
}

int linc_set_level(enum linc_level level) {
    LINC_BOOTSTRAP(&linc_once_init, linc_bootstrap);

    if (level < LINC_LEVEL_TRACE || level > LINC_LEVEL_FATAL) {
        return -1;
    }

    linc_global.level = level;
    return 0;
}

int linc_add_module(const char *name) {
    LINC_BOOTSTRAP(&linc_once_init, linc_bootstrap);

    if (name == NULL || strlen(name) == 0 || strlen(name) >= LINC_MODULES_NAME_LENGTH) {
        return -1;
    }
    if (linc_global.modules_count >= LINC_MODULES_MAX_NUMBER) {
        return -1;
    }
    if (linc_modules_exists(name, linc_global.modules_list, linc_global.modules_count) >= 0) {
        return -1;
    }

    size_t module_index = linc_global.modules_count;
    strcpy(linc_global.modules_list[module_index].name, name);
    linc_global.modules_list[module_index].enabled = true;
    linc_global.modules_list[module_index].level = LINC_LEVEL_INHERIT;
    linc_global.modules_count++;

    return module_index;
}

int linc_set_module_enabled(const char *name, bool enabled) {
    LINC_BOOTSTRAP(&linc_once_init, linc_bootstrap);

    if (name == NULL || strlen(name) == 0 || strlen(name) >= LINC_MODULES_NAME_LENGTH) {
        return -1;
    }
    int module_index = linc_modules_exists(name, linc_global.modules_list, linc_global.modules_count);
    if (module_index < 0) {
        return -1;
    }

    linc_global.modules_list[module_index].enabled = enabled;
    return 0;
}

int linc_set_module_level(const char *name, enum linc_level level) {
    LINC_BOOTSTRAP(&linc_once_init, linc_bootstrap);

    if (name == NULL || strlen(name) == 0 || strlen(name) >= LINC_MODULES_NAME_LENGTH) {
        return -1;
    }
    if (level < LINC_LEVEL_INHERIT || level > LINC_LEVEL_FATAL) {
        return -1;
    }
    int module_index = linc_modules_exists(name, linc_global.modules_list, linc_global.modules_count);
    if (module_index < 0) {
        return -1;
    }

    linc_global.modules_list[module_index].level = level;
    return 0;
}

int linc_add_sink(struct linc_sink_entry entry) {
    LINC_BOOTSTRAP(&linc_once_init, linc_bootstrap);

    if (entry.open == NULL || entry.close == NULL || entry.write == NULL || entry.flush == NULL) {
        return -1;
    }
    if (linc_global.sinks_count >= LINC_SINKS_MAX_NUMBER) {
        return -1;
    }
    if (entry.open(entry.data) < 0) {
        return -1;
    }

    size_t sink_index = linc_global.sinks_count;
    linc_global.sinks_list[sink_index].enabled = true;
    linc_global.sinks_list[sink_index].level = LINC_LEVEL_INHERIT;
    linc_global.sinks_list[sink_index].formatter = LINC_FORMAT_TEXT;
    linc_global.sinks_list[sink_index].color_mode = LINC_COLOR_MODE_AUTO;
    linc_global.sinks_list[sink_index].entry = entry;
    linc_global.sinks_count++;

    return sink_index;
}

int linc_set_sink_enabled(size_t index, bool enabled) {
    LINC_BOOTSTRAP(&linc_once_init, linc_bootstrap);

    if (index >= linc_global.sinks_count) {
        return -1;
    }

    linc_global.sinks_list[index].enabled = enabled;
    return 0;
}

int linc_set_sink_level(size_t index, enum linc_level level) {
    LINC_BOOTSTRAP(&linc_once_init, linc_bootstrap);

    if (index >= linc_global.sinks_count) {
        return -1;
    }
    if (level < LINC_LEVEL_INHERIT || level > LINC_LEVEL_FATAL) {
        return -1;
    }

    linc_global.sinks_list[index].level = level;
    return 0;
}

int linc_set_sink_color_mode(size_t index, enum linc_color_mode color_mode) {
    LINC_BOOTSTRAP(&linc_once_init, linc_bootstrap);

    if (index >= linc_global.sinks_count) {
        return -1;
    }
    if (color_mode < LINC_COLOR_MODE_AUTO || color_mode > LINC_COLOR_MODE_ALWAYS) {
        return -1;
    }

    linc_global.sinks_list[index].color_mode = color_mode;
    return 0;
}

int linc_set_sink_formatter(size_t index, enum linc_formatter formatter) {
    LINC_BOOTSTRAP(&linc_once_init, linc_bootstrap);

    if (index >= linc_global.sinks_count) {
        return -1;
    }
    if (formatter < LINC_FORMAT_TEXT || formatter > LINC_FORMAT_JSON) {
        return -1;
    }

    linc_global.sinks_list[index].formatter = formatter;
    return 0;
}
