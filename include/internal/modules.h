#ifndef LINC_INCLUDE_INTERNAL_MODULES_H
#define LINC_INCLUDE_INTERNAL_MODULES_H

#include "linc.h"

// ==================================================
// Structures and Enums
// ==================================================

struct linc_module {
    char name[LINC_DEFAULT_MODULE_NAME_LENGTH + LINC_ZERO_CHAR_LENGTH];  // Module name
    enum linc_level level;                                               // Minimum log level for this module
    bool enabled;                                                        // Is module enabled
};

struct linc_module_list {
    struct linc_module list[LINC_DEFAULT_MAX_MODULES];  // List of registered modules
    size_t count;                                       // Number of registered modules
};

// ==================================================
// Internal Functions
// ==================================================

void linc_default_module(struct linc_module_list *modules);

// ==================================================
// Public Functions (linc.h)
// ==================================================

// linc_module linc_register_module(const char *name, enum linc_level level, bool enabled);
// linc_module linc_get_module(const char *name);
// int linc_set_module_level(linc_module module, enum linc_level level);
// int linc_set_module_enabled(linc_module module, bool enabled);

#endif  // LINC_INCLUDE_INTERNAL_MODULES_H
