#include "linc.h"

#include <inttypes.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

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
    memset(&linc_global, 0, sizeof(linc_global));

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

    const char *module_name = module == NULL ? LINC_MODULES_DEFAULT_NAME : module;
    int is_level_valid = linc_modules_check_level(module_name, level);
    if (is_level_valid < 0) {
        return;
    }

    const char *timestamp = "0000-00-00 00:00:00.000";
    uintptr_t thread_id = (uintptr_t)pthread_self();

    char message[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    printf("[ %s ] [ %-5s ] [ %016" PRIxPTR " ] [ %-" LINC_NUM_TO_STR(LINC_MODULES_NAME_LENGTH) "s ] %s:%" PRIu32
                                                                                                " %s: %s\n",
           timestamp,
           linc_level_string(level),
           thread_id,
           module_name,
           file,
           line,
           func,
           message);
}
