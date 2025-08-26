#include "linc.h"

#include <inttypes.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==================================================
// Global Variables
// ==================================================

static struct linc linc_global;
static pthread_once_t linc_once_init = PTHREAD_ONCE_INIT;

// ==================================================
// Private Functions
// ==================================================

static int linc_modules_check(const char *module_name, struct linc_module *modules, size_t count) {
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

// ==================================================
// State Management
// ==================================================

static void linc_shutdown(void) {}

static void linc_bootstrap(void) {
    memset(&linc_global, 0, sizeof(linc_global));

    linc_global.modules_count = 1;
    linc_global.modules_list[0].enabled = true;
    linc_global.modules_list[0].level = LINC_LEVEL_DEFAULT;
    strcpy(linc_global.modules_list[0].name, LINC_MODULES_DEFAULT_NAME);

    atexit(linc_shutdown);
}

// ==================================================
// Public Functions
// ==================================================

void linc_log(const char *module,
              enum linc_level level,
              const char *file,
              uint32_t line,
              const char *func,
              const char *format,
              ...) {
    pthread_once(&linc_once_init, linc_bootstrap);

    const char *module_name = module == NULL ? LINC_MODULES_DEFAULT_NAME : module;
    int module_index = linc_modules_check(module_name, linc_global.modules_list, linc_global.modules_count);
    if (module_index < 0 || linc_global.modules_list[module_index].enabled == false
        || level < linc_global.modules_list[module_index].level) {
        return;
    }

    const char *timestamp = "0000-00-00 00:00:00.000";
    const char *level_strings[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
    uintptr_t thread_id = (uintptr_t)pthread_self();

    char message[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    printf("[ %s ] [ %-5s ] [ %016" PRIxPTR " ] [ %s ] %s:%" PRIu32 " %s: %s\n",
           timestamp,
           level_strings[level],
           thread_id,
           module_name,
           file,
           line,
           func,
           message);
}
