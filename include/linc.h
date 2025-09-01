#ifndef LINC_INCLUDE_LINC_H_
#define LINC_INCLUDE_LINC_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// ==================================================
// Common Macros
// ==================================================

#define LINC_NUM_STRINGIFY(number) #number
#define LINC_NUM_TO_STR(number) LINC_NUM_STRINGIFY(number)

#if defined(__GNUC__)
#define LINC_ATSTART __attribute__((constructor))
#else
#define LINC_ATSTART
#endif

#define LINC_COLOR_RESET "\x1b[0m"
#define LINC_COLOR_BOLD "\x1b[1m"
#define LINC_COLOR_DIM "\x1b[2m"
#define LINC_COLOR_NORMAL "\x1b[22m"
#define LINC_COLOR_RED "\x1b[91m"
#define LINC_COLOR_GREEN "\x1b[92m"
#define LINC_COLOR_YELLOW "\x1b[93m"
#define LINC_COLOR_BLUE "\x1b[94m"
#define LINC_COLOR_MAGENTA "\x1b[95m"
#define LINC_COLOR_CYAN "\x1b[96m"
#define LINC_COLOR_WHITE "\x1b[97m"

#define LINC_TIMESTAMP_LENGTH 24                           // Length of timestamp string "YYYY-MM-DD HH:MM:SS.mmm"
#define LINC_TIMESTAMP_FALLBACK "0000-00-00 00:00:00.000"  // Fallback timestamp string
#define LINC_LOG_TEXT_LEVEL_LENGTH 5                       // Length of log level string
#define LINC_LOG_TEXT_THREAD_ID_LENGTH 16                  // Length of thread ID string
#define LINC_LOG_TEXT_FILE_LENGTH 64                       // Length of file name string
#define LINC_LOG_TEXT_FUNC_LENGTH 64                       // Length of function name string
#define LINC_LOG_TEXT_LINE_LENGTH 10                       // Length of line number string
#define LINC_LOG_TEXT_CHAR_SET 24                          // Extra characters for formatting, e.g., [ ], spaces, etc.

#define LINC_LOG_TEXT_MAX_LENGTH                                                                                    \
    (LINC_TIMESTAMP_LENGTH + LINC_LOG_TEXT_LEVEL_LENGTH + LINC_LOG_TEXT_THREAD_ID_LENGTH + LINC_MODULES_NAME_LENGTH \
     + LINC_LOG_TEXT_FILE_LENGTH + LINC_LOG_TEXT_LINE_LENGTH + LINC_LOG_TEXT_FUNC_LENGTH                            \
     + LINC_LOG_MESSAGE_MAX_LENGTH)
#define LINC_LOG_TEXT_FORMAT                                                                                          \
    "[ %s ] [ %-" LINC_NUM_TO_STR(LINC_LOG_TEXT_LEVEL_LENGTH) "s ] [ %0" LINC_NUM_TO_STR(                             \
        LINC_LOG_TEXT_THREAD_ID_LENGTH) PRIxPTR " ] [ %-" LINC_NUM_TO_STR(LINC_MODULES_NAME_LENGTH) "s ] %s:%" PRIu32 \
                                                                                                    " %s: %s\n"

#define LINC_LOG_TEXT_FALLBACK "[ LINC ERROR ] Internal logging error occurred\n"

// ==================================================
// Common Types
// ==================================================

#define LINC_LEVEL_DEFAULT 2  // Default log level

enum linc_level {
    LINC_LEVEL_INHERIT = -1,  // Inherit level from parent (used internally)
    LINC_LEVEL_TRACE = 0,     // Lowest level, for detailed debugging information
    LINC_LEVEL_DEBUG = 1,     // Debugging information, useful for developers
    LINC_LEVEL_INFO = 2,      // General information about application state
    LINC_LEVEL_WARN = 3,      // Warning messages, indicating potential issues
    LINC_LEVEL_ERROR = 4,     // Error messages, indicating something went wrong
    LINC_LEVEL_FATAL = 5,     // Critical errors that cause the application to terminate
};

// ==================================================
// Modules Configuration
// ==================================================

#define LINC_MODULES_DEFAULT_NAME "main"  // Default module name
#define LINC_MODULES_MAX_NUMBER 8         // Maximum number of modules
#define LINC_MODULES_NAME_LENGTH 16       // Maximum length for module names

struct linc_module {
    char name[LINC_MODULES_NAME_LENGTH];  // Name of the module
    enum linc_level level;                // Minimum log level to output (most for client side)
    bool enabled;                         // Whether the module is enabled
};

// ==================================================
// Sinks Configuration
// ==================================================

#define LINC_SINKS_MAX_NUMBER 8  // Maximum number of sinks

enum linc_color_mode {
    LINC_COLOR_MODE_AUTO = 0,    // Enable color if output is a TTY
    LINC_COLOR_MODE_NEVER = 1,   // Never use colors
    LINC_COLOR_MODE_ALWAYS = 2,  // Always use colors (assume ANSI safe)
};

enum linc_formatter {
    LINC_FORMAT_TEXT = 0,  // Plain text format
    LINC_FORMAT_JSON = 1,  // JSON format for structured logging
};

struct linc_sink_entry {
    void *data;                // Pointer to sink-specific data (e.g., file handle, network socket)
    int (*open)(void *data);   // Function to open/init the sink
    int (*close)(void *data);  // Function to close/deinit the sink
    int (*write)(void *data, const char *buffer, size_t length);  // Function to write log data to the sink
    int (*flush)(void *data);                                     // Function to flush any buffered log data
};

struct linc_sink {
    enum linc_level level;            // Minimum log level to output (most for worker side)
    enum linc_color_mode color_mode;  // Color mode for the sink
    enum linc_formatter formatter;    // Formatter type for the sink
    bool enabled;                     // Whether the sink is enabled
    struct linc_sink_entry entry;     // Sink entry with function pointers and data
};

// ==================================================
// Log Entry Configuration
// ==================================================

#define LINC_LOG_MESSAGE_MAX_LENGTH 512  // Maximum length for log messages

struct linc_entry {
    int64_t timestamp;                          // Timestamp in nanoseconds since epoch
    enum linc_level level;                      // Log level of the entry
    uintptr_t thread_id;                        // Thread ID where the log was generated
    size_t module_index;                        // Index of the module in the modules list
    const char *file;                           // Source file where the log was generated
    uint32_t line;                              // Line number in the source file
    const char *func;                           // Function name where the log was generated
    char message[LINC_LOG_MESSAGE_MAX_LENGTH];  // Log message content
};

// ==================================================
// Linc Configuration
// ==================================================

struct linc {
    enum linc_level level;                                     // Global minimum log level (client side for modules)
    struct linc_module modules_list[LINC_MODULES_MAX_NUMBER];  // List of modules
    size_t modules_count;                                      // Number of modules in the list
    struct linc_sink sinks_list[LINC_SINKS_MAX_NUMBER];        // List of sinks
    size_t sinks_count;                                        // Number of sinks in the list
};

// ==================================================
// APIs linc
// ==================================================

#define TRACE(format, ...) linc_log(NULL, LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define DEBUG(format, ...) linc_log(NULL, LINC_LEVEL_DEBUG, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define INFO(format, ...) linc_log(NULL, LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define WARN(format, ...) linc_log(NULL, LINC_LEVEL_WARN, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define ERROR(format, ...) linc_log(NULL, LINC_LEVEL_ERROR, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define FATAL(format, ...) linc_log(NULL, LINC_LEVEL_FATAL, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

#define TRACE_M(module, format, ...) \
    linc_log(module, LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define DEBUG_M(module, format, ...) \
    linc_log(module, LINC_LEVEL_DEBUG, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define INFO_M(module, format, ...) \
    linc_log(module, LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define WARN_M(module, format, ...) \
    linc_log(module, LINC_LEVEL_WARN, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define ERROR_M(module, format, ...) \
    linc_log(module, LINC_LEVEL_ERROR, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define FATAL_M(module, format, ...) \
    linc_log(module, LINC_LEVEL_FATAL, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

void linc_log(const char *module,
              enum linc_level level,
              const char *file,
              uint32_t line,
              const char *func,
              const char *format,
              ...);

#endif  // LINC_INCLUDE_LINC_H_
