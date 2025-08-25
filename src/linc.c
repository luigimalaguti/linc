#include "linc.h"

#include <inttypes.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

void linc_log(const char *module,
              linc_level_t level,
              const char *file,
              uint32_t line,
              const char *func,
              const char *format,
              ...) {
    const char *timestamp = "0000-00-00 00:00:00.000";
    const char *level_strings[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
    uintptr_t thread_id = (uintptr_t)pthread_self();
    const char *module_name = module == NULL ? "main" : module;

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
