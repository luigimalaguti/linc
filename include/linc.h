#ifndef LINC_INCLUDE_LINC_H_
#define LINC_INCLUDE_LINC_H_

#include <stdint.h>

// ==================================================
// Common Macros
// ==================================================

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

// ==================================================
// Common Types
// ==================================================

enum linc_level {
    LINC_LEVEL_TRACE = 0,  // Lowest level, for detailed debugging information
    LINC_LEVEL_DEBUG = 1,  // Debugging information, useful for developers
    LINC_LEVEL_INFO = 2,   // General information about application state
    LINC_LEVEL_WARN = 3,   // Warning messages, indicating potential issues
    LINC_LEVEL_ERROR = 4,  // Error messages, indicating something went wrong
    LINC_LEVEL_FATAL = 5,  // Critical errors that cause the application to terminate
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
