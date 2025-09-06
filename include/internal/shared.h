#ifndef LINC_INCLUDE_INTERNAL_SHARED_H
#define LINC_INCLUDE_INTERNAL_SHARED_H

#include "internal/modules.h"
#include "internal/sinks.h"
#include "linc.h"

// ==================================================
// Macros
// ==================================================

#define LINC_PARAM_STR(param) #param                 // Expands the parameter and converts it to a string
#define LINC_STRINGIFY(param) LINC_PARAM_STR(param)  // Stringifies the parameter

// [ 0000-00-00 00:00:00.000 ] [ LEVEL ] [ 012abc ] [ module ] filename.c:12345 function: Log message
//
// 0000-00-00 00:00:00.000                      -> LINC_LOG_TIMESTAMP_LENGTH
// LEVEL                                        -> LINC_LOG_LEVEL_LENGTH
// 012abc                                       -> LINC_LOG_THREAD_ID_LENGTH
// module                                       -> LINC_DEFAULT_MODULE_NAME_LENGTH
// filename.c                                   -> LINC_LOG_FILE_LENGTH
// 12345                                        -> LINC_LOG_LINE_LENGTH
// function                                     -> LINC_LOG_FUNC_LENGTH
// Log message                                  -> LINC_DEFAULT_MAX_MESSAGE_LENGTH
//
// [ -- ] [ -- ] [ -- ] [ -- ] --:-- --: --     -> LINC_LOG_EXTRA_FMT_LENGTH

#define LINC_LOG_TIMESTAMP_LENGTH 23  // Length of timestamp string "YYYY-MM-DD HH:MM:SS.mmm"
#define LINC_LOG_LEVEL_LENGTH 5       // Length of log level string
#define LINC_LOG_THREAD_ID_LENGTH 16  // Length of thread ID string
#define LINC_LOG_FILE_LENGTH 64       // Length of file name string
#define LINC_LOG_LINE_LENGTH 10       // Length of line number string
#define LINC_LOG_FUNC_LENGTH 64       // Length of function name string

#define LINC_LOG_EXTRA_FMT_LENGTH 24  // Extra characters for formatting, e.g., [ ], spaces, etc.

#define LINC_LOG_MAX_LENGTH                                                                                          \
    (LINC_LOG_TIMESTAMP_LENGTH + LINC_LOG_LEVEL_LENGTH + LINC_LOG_THREAD_ID_LENGTH + LINC_DEFAULT_MODULE_NAME_LENGTH \
     + LINC_LOG_FILE_LENGTH + LINC_LOG_LINE_LENGTH + LINC_LOG_FUNC_LENGTH + LINC_DEFAULT_MAX_MESSAGE_LENGTH          \
     + LINC_LOG_EXTRA_FMT_LENGTH)

// ==================================================
// Structures and Enums
// ==================================================

struct linc {
    enum linc_level level;            // Default log level
    struct linc_module_list modules;  // List of registered modules
    struct linc_sink_list sinks;      // List of registered sinks
};

struct linc_metadata {
    int64_t timestamp;                                                      // Timestamp in nanoseconds since epoch
    enum linc_level level;                                                  // Level of the log
    uintptr_t thread_id;                                                    // Thread ID where the log was generated
    const char *module_name;                                                // Module name where the log was generated
    const char *file;                                                       // Source file where the log was generated
    uint32_t line;                                                          // Line number in the source file
    const char *func;                                                       // Function name where the log was generated
    char message[LINC_DEFAULT_MAX_MESSAGE_LENGTH + LINC_ZERO_CHAR_LENGTH];  // Log message content
};

// ==================================================
// Internal Functions
// ==================================================

int64_t linc_timestamp(void);
int linc_timestamp_string(int64_t timestamp, char *buffer, size_t size);
const char *linc_level_string(enum linc_level level);

void linc_init(void);
struct linc *linc_get_state(void);
enum linc_level linc_get_level(void);
struct linc_module_list *linc_get_modules(void);
struct linc_sink_list *linc_get_sinks(void);

// In the meantime, define the worker function here to be used by the client.
void linc_temp_worker(struct linc_metadata *metadata);

// ==================================================
// Public Functions (linc.h)
// ==================================================

// int linc_set_level(enum linc_level level);

#endif  // LINC_INCLUDE_INTERNAL_SHARED_H
