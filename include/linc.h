#ifndef LINC_INCLUDE_LINC_H
#define LINC_INCLUDE_LINC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// ==================================================
// Structures and Enums
// ==================================================

enum linc_level {
    LINC_LEVEL_INHERIT = -1,  // Inherit level from parent (used internally)
    LINC_LEVEL_TRACE = 0,     // Lowest level, for detailed debugging information
    LINC_LEVEL_DEBUG = 1,     // Debugging information, useful for developers
    LINC_LEVEL_INFO = 2,      // General information about application state
    LINC_LEVEL_WARN = 3,      // Warning messages, indicating potential issues
    LINC_LEVEL_ERROR = 4,     // Error messages, indicating something went wrong
    LINC_LEVEL_FATAL = 5,     // Critical errors that cause the application to terminate
};

enum linc_color_mode {
    LINC_COLOR_MODE_AUTO = 0,    // Enable color if output is a TTY
    LINC_COLOR_MODE_NEVER = 1,   // Never use colors
    LINC_COLOR_MODE_ALWAYS = 2,  // Always use colors (assume ANSI safe)
};

enum linc_formatter {
    LINC_FORMATTER_TEXT = 0,  // Plain text format
    LINC_FORMATTER_JSON = 1,  // JSON format for structured logging
};

struct linc_sink_funcs {
    void *data;                                                    // User-defined data pointer
    int (*open)(void *data);                                       // Function to open/init the sink
    int (*close)(void *data);                                      // Function to close/deinit the sink
    int (*write)(void *data, const char *message, size_t length);  // Function to write a log message
    int (*flush)(void *data);                                      // Function to flush the sink (if applicable)
};

typedef struct linc_module *linc_module;  // Opaque pointer to a module
typedef struct linc_sink *linc_sink;      // Opaque pointer to a sink

// ==================================================
// Macros and Definitions
// ==================================================

#if defined(__GNUC__)
#define LINC_PRINT_FMT(fmt, args) \
    __attribute__((format(printf, fmt, args)))  // GCC format attribute for printf-style functions
#else
#define LINC_PRINT_FMT(fmt, args)
#endif

#if !defined(LINC_DEFAULT_LEVEL)
#define LINC_DEFAULT_LEVEL LINC_LEVEL_INFO  // Default log level
#elif (LINC_DEFAULT_LEVEL < LINC_LEVEL_TRACE) || (LINC_DEFAULT_LEVEL > LINC_LEVEL_FATAL)
#error "LINC_DEFAULT_LEVEL must be between LINC_LEVEL_TRACE and LINC_LEVEL_FATAL"
#endif

#if !defined(LINC_DEFAULT_MODULE_NAME_LENGTH)
#define LINC_DEFAULT_MODULE_NAME_LENGTH 16  // Maximum length for module names
#elif (LINC_DEFAULT_MODULE_NAME_LENGTH < 1)
#error "LINC_DEFAULT_MODULE_NAME_LENGTH must be at least 1"
#endif

#if !defined(LINC_DEFAULT_MODULE_NAME)
#define LINC_DEFAULT_MODULE_NAME "main"  // Default module name
#elif (sizeof(LINC_DEFAULT_MODULE_NAME) > LINC_DEFAULT_MODULE_NAME_LENGTH) || (sizeof(LINC_DEFAULT_MODULE_NAME) < 1)
#error "LINC_DEFAULT_MODULE_NAME must be between 1 and LINC_DEFAULT_MODULE_NAME_LENGTH characters"
#endif

#if !defined(LINC_DEFAULT_MAX_MODULES)
#define LINC_DEFAULT_MAX_MODULES 8  // Maximum number of modules
#elif (LINC_DEFAULT_MAX_MODULES < 1)
#error "LINC_DEFAULT_MAX_MODULES must be at least 1"
#endif

#if !defined(LINC_DEFAULT_SINK_NAME_LENGTH)
#define LINC_DEFAULT_SINK_NAME_LENGTH 16  // Maximum length for sink names
#elif (LINC_DEFAULT_SINK_NAME_LENGTH < 1)
#error "LINC_DEFAULT_SINK_NAME_LENGTH must be at least 1"
#endif

#if !defined(LINC_DEFAULT_SINK_NAME)
#define LINC_DEFAULT_SINK_NAME "stderr"  // Default sink name
#elif (sizeof(LINC_DEFAULT_SINK_NAME) > LINC_DEFAULT_SINK_NAME_LENGTH) || (sizeof(LINC_DEFAULT_SINK_NAME) < 1)
#error "LINC_DEFAULT_SINK_NAME must be between 1 and LINC_DEFAULT_SINK_NAME_LENGTH characters"
#endif

#if !defined(LINC_DEFAULT_MAX_SINKS)
#define LINC_DEFAULT_MAX_SINKS 8  // Maximum number of sinks
#elif (LINC_DEFAULT_MAX_SINKS < 1)
#error "LINC_DEFAULT_MAX_SINKS must be at least 1"
#endif

#if !defined(LINC_DEFAULT_MAX_MESSAGE_LENGTH)
#define LINC_DEFAULT_MAX_MESSAGE_LENGTH 512  // Default maximum length for log messages
#elif (LINC_DEFAULT_MAX_MESSAGE_LENGTH < 1)
#error "LINC_DEFAULT_MAX_MESSAGE_LENGTH must be at least 1"
#endif

#define LINC_ZERO_CHAR_LENGTH 1     // Zero character length
#define LINC_NEWLINE_CHAR_LENGTH 1  // Newline character length

// ==================================================
// Functions
// ==================================================

int linc_set_level(enum linc_level level);

void linc_log(const char *module_name,
              enum linc_level level,
              const char *file,
              uint32_t line,
              const char *func,
              const char *format,
              ...) LINC_PRINT_FMT(6, 7);

#define TRACE(...) linc_log(NULL, LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define DEBUG(...) linc_log(NULL, LINC_LEVEL_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define INFO(...) linc_log(NULL, LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define WARN(...) linc_log(NULL, LINC_LEVEL_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ERROR(...) linc_log(NULL, LINC_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define FATAL(...) linc_log(NULL, LINC_LEVEL_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define TRACE_M(module, ...) linc_log(module, LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define DEBUG_M(module, ...) linc_log(module, LINC_LEVEL_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define INFO_M(module, ...) linc_log(module, LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define WARN_M(module, ...) linc_log(module, LINC_LEVEL_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ERROR_M(module, ...) linc_log(module, LINC_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define FATAL_M(module, ...) linc_log(module, LINC_LEVEL_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

linc_module linc_register_module(const char *name, enum linc_level level, bool enabled);
linc_sink linc_register_sink(const char *name,
                             enum linc_level level,
                             enum linc_color_mode color_mode,
                             enum linc_formatter formatter,
                             bool enabled,
                             struct linc_sink_funcs funcs);

linc_module linc_get_module(const char *name);
linc_sink linc_get_sink(const char *name);

int linc_set_module_level(linc_module module, enum linc_level level);
int linc_set_module_enabled(linc_module module, bool enabled);

int linc_set_sink_level(linc_sink sink, enum linc_level level);
int linc_set_sink_color_mode(linc_sink sink, enum linc_color_mode color_mode);
int linc_set_sink_formatter(linc_sink sink, enum linc_formatter formatter);
int linc_set_sink_enabled(linc_sink sink, bool enabled);

#endif  // LINC_INCLUDE_LINC_H
