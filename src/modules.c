#include "internal/shared.h"
#include "linc.h"

#include <string.h>

// ==================================================
// Internal Functions
// ==================================================

static int linc_check_name_module(struct linc_module_list *modules, const char *name) {
    if (name == NULL) {
        return -1;
    }

    size_t name_length = strnlen(name, LINC_DEFAULT_MODULE_NAME_LENGTH + LINC_ZERO_CHAR_LENGTH);
    if (name_length == 0 || name_length > LINC_DEFAULT_MODULE_NAME_LENGTH) {
        return -1;
    }

    for (size_t i = 0; i < modules->count; i++) {
        if (strncmp(modules->list[i].name, name, LINC_DEFAULT_MODULE_NAME_LENGTH) == 0) {
            return -1;
        }
    }

    return 0;
}

static int linc_check_level_module(enum linc_level level) {
    if (level < LINC_LEVEL_INHERIT || level > LINC_LEVEL_FATAL) {
        return -1;
    }
    return 0;
}

static struct linc_module *linc_add_module(struct linc_module_list *modules,
                                           const char *name,
                                           enum linc_level level,
                                           bool enabled) {
    bool is_failed = false;
    is_failed |= modules == NULL;
    is_failed |= modules->count >= LINC_DEFAULT_MAX_MODULES;
    is_failed |= linc_check_name_module(modules, name) < 0;
    is_failed |= linc_check_level_module(level) < 0;

    if (is_failed == true) {
        return NULL;
    }

    struct linc_module *module = &modules->list[modules->count];
    strncpy(module->name, name, LINC_DEFAULT_MODULE_NAME_LENGTH);
    module->name[LINC_DEFAULT_MODULE_NAME_LENGTH] = '\0';
    module->level = level;
    module->enabled = enabled;
    modules->count += 1;

    return module;
}

void linc_default_module(struct linc_module_list *modules) {
    modules->count = 0;
    linc_add_module(modules, LINC_DEFAULT_MODULE_NAME, LINC_LEVEL_INHERIT, true);
}

// ==================================================
// Public Functions
// ==================================================

linc_module linc_register_module(const char *name, enum linc_level level, bool enabled) {
    struct linc_module_list *modules = linc_get_modules();
    struct linc_module *module = linc_add_module(modules, name, level, enabled);
    if (module == NULL) {
        return NULL;
    }
    return module;
}

linc_module linc_get_module(const char *name) {
    struct linc_module_list *modules = linc_get_modules();
    if (name == NULL) {
        return NULL;
    }

    size_t name_length = strnlen(name, LINC_DEFAULT_MODULE_NAME_LENGTH + LINC_ZERO_CHAR_LENGTH);
    if (name_length == 0 || name_length > LINC_DEFAULT_MODULE_NAME_LENGTH) {
        return NULL;
    }

    for (size_t i = 0; i < modules->count; i++) {
        if (strncmp(modules->list[i].name, name, LINC_DEFAULT_MODULE_NAME_LENGTH) == 0) {
            return &modules->list[i];
        }
    }

    return NULL;
}

int linc_set_module_level(linc_module module, enum linc_level level) {
    linc_init();
    if (module == NULL || linc_check_level_module(level) < 0) {
        return -1;
    }
    module->level = level;
    return 0;
}

int linc_set_module_enabled(linc_module module, bool enabled) {
    linc_init();
    if (module == NULL) {
        return -1;
    }
    module->enabled = enabled;
    return 0;
}
