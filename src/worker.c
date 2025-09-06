#include "internal/shared.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==================================================
// Internal Functions
// ==================================================

/**
 * @brief Checks if a sink is enabled and if the specified log level is allowed.
 *
 * This function validates whether a logging operation should proceed based on two criteria:
 *
 * 1. The sink must be enabled (sink->enabled must be true)
 * 2. The requested log level must be equal to or higher than the sink's configured level
 *
 * The function also handles the special case where the sink's level is set to
 * LINC_LEVEL_INHERIT, which is treated as a valid level (effectively allowing all levels).
 *
 * @param sink       Pointer to the linc_sink structure to check.
 * @param log_level  The logging level to verify against the sink's configuration.
 *                   Must be a valid level between LINC_LEVEL_TRACE and LINC_LEVEL_FATAL.
 *
 * @return Returns different values based on the outcome:
 *     - 1: If the sink is enabled and the log level is explicitly allowed (not inherited)
 *     - 0: If the sink is enabled and the level is inherited (LINC_LEVEL_INHERIT)
 *     - -1: If the sink is disabled, the log level is invalid, or the log level is below the sink's level
 */
static int linc_check_sink(struct linc_sink *sink, enum linc_level level) {
    if (sink == NULL || level < LINC_LEVEL_TRACE || level > LINC_LEVEL_FATAL) {
        return -1;
    }

    bool is_sink_enabled = sink->enabled == true;
    bool is_level_inherit = sink->level == LINC_LEVEL_INHERIT;
    // NOTE: If sink.level is LINC_LEVEL_INHERIT, then log_level >= sink.level is always true.
    bool is_level_valid = level >= sink->level;

    if (!is_sink_enabled || !is_level_valid) {
        return -1;
    } else if (is_level_inherit) {
        return 0;
    }
    return 1;
}

void linc_temp_worker(struct linc_metadata *metadata) {
    char timestamp_string[LINC_LOG_TIMESTAMP_LENGTH + LINC_ZERO_CHAR_LENGTH];
    if (linc_timestamp_string(metadata->timestamp, timestamp_string, sizeof(timestamp_string)) < 0) {
        strcpy(timestamp_string, "0000-00-00 00:00:00.000");
    }

    struct linc_sink_list *sinks = linc_get_sinks();
    for (size_t i = 0; i < sinks->count; i++) {
        struct linc_sink *sink = &sinks->list[i];
        int sink_check = linc_check_sink(sink, metadata->level);
        if (sink_check < 0 || (sink_check == 0 && metadata->level < linc_get_level())) {
            continue;
        }

        const char *file = metadata->file == NULL ? "unknown" : metadata->file;
        const char *func = metadata->func == NULL ? "unknown" : metadata->func;
        char log_text_format[LINC_LOG_MAX_LENGTH + LINC_NEWLINE_CHAR_LENGTH + LINC_ZERO_CHAR_LENGTH];
        int written = snprintf(
            log_text_format,
            sizeof(log_text_format),
            "[ %s ] [ %-" LINC_STRINGIFY(LINC_LOG_LEVEL_LENGTH) "s ] [ %0" LINC_STRINGIFY(LINC_LOG_THREAD_ID_LENGTH)
                PRIxPTR " ] [ %-" LINC_STRINGIFY(LINC_DEFAULT_MODULE_NAME_LENGTH) "s ] %s:%" PRIu32 " %s: %s\n",
            timestamp_string,
            linc_level_string(metadata->level),
            metadata->thread_id,
            metadata->module_name,
            file,
            metadata->line,
            func,
            metadata->message);

        if (written < 0) {
            strcpy(log_text_format, "[ LINC ERROR ] Internal logging error\n");
            written = strlen(log_text_format);
        } else if ((size_t)written >= sizeof(log_text_format)) {
            written = sizeof(log_text_format) - 1;
        }

        sink->funcs.write(sink->funcs.data, log_text_format, written);
    }

    free(metadata);
}
