#include "internal/shared.h"

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// ==================================================
// Internal Functions
// ==================================================

/**
 * @brief Checks if a module is enabled and if the specified log level is allowed.
 *
 * This function validates whether a logging operation should proceed based on two criteria:
 *
 * 1. The module must be enabled (module->enabled must be true)
 * 2. The requested log level must be equal to or higher than the module's configured level
 *
 * The function also handles the special case where the module's level is set to
 * LINC_LEVEL_INHERIT, which is treated as a valid level (effectively allowing all levels).
 *
 * @param module     Pointer to the linc_module structure to check.
 * @param log_level  The logging level to verify against the module's configuration.
 *                   Must be a valid level between LINC_LEVEL_TRACE and LINC_LEVEL_FATAL.
 *
 * @return Returns different values based on the outcome:
 *     - 1: If the module is enabled and the log level is explicitly allowed (not inherited)
 *     - 0: If the module is enabled and the level is inherited (LINC_LEVEL_INHERIT)
 *     - -1: If the module is disabled, the log level is invalid, or the log level is below the module's level
 */
static int linc_check_module(struct linc_module *module, enum linc_level level) {
    if (module == NULL || level < LINC_LEVEL_TRACE || level > LINC_LEVEL_FATAL) {
        return -1;
    }

    bool is_module_enabled = module->enabled == true;
    bool is_level_inherit = module->level == LINC_LEVEL_INHERIT;
    // NOTE: If module.level is LINC_LEVEL_INHERIT, then log_level >= module.level is always true.
    bool is_level_valid = level >= module->level;

    if (!is_module_enabled || !is_level_valid) {
        return -1;
    } else if (is_level_inherit) {
        return 0;
    }
    return 1;
}

// ==================================================
// Public Functions
// ==================================================

void linc_log(const char *module_name,
              enum linc_level level,
              const char *file,
              uint32_t line,
              const char *func,
              const char *format,
              ...) {
    const char *name = module_name == NULL ? LINC_DEFAULT_MODULE_NAME : module_name;
    struct linc_module *module = linc_get_module(name);

    int module_check = linc_check_module(module, level);
    if (module_check < 0 || (module_check == 0 && level < linc_get_level())) {
        return;
    }

    struct linc_metadata *metadata = (struct linc_metadata *)calloc(1, sizeof(struct linc_metadata));
    if (metadata == NULL) {
        return;
    }
    metadata->timestamp = linc_timestamp();
    metadata->level = level;
    metadata->thread_id = (uintptr_t)pthread_self();
    metadata->module_name = module->name;
    metadata->file = file;
    metadata->line = line;
    metadata->func = func;

    if (format != NULL) {
        va_list args;
        va_start(args, format);
        vsnprintf(metadata->message, sizeof(metadata->message), format, args);
        va_end(args);
    }

    // In the meantime, process the log entry in the current thread.
    linc_temp_worker(metadata);
}
