#include "internal/shared.h"

#include <stdio.h>
#include <string.h>

// ==================================================
// Default Sink Functions
// ==================================================

static int linc_sink_stderr_open(void *data) {
    (void)data;
    return 0;
}

static int linc_sink_stderr_close(void *data) {
    (void)data;
    return 0;
}

static int linc_sink_stderr_write(void *data, const char *buffer, size_t length) {
    FILE *output_file = (FILE *)data;
    size_t written = fwrite(buffer, sizeof(char), length, output_file);
    return written == length ? 0 : -1;
}

static int linc_sink_stderr_flush(void *data) {
    FILE *output_file = (FILE *)data;
    return fflush(output_file);
}

// ==================================================
// Internal Functions
// ==================================================

static int linc_check_name_sink(struct linc_sink_list *sinks, const char *name) {
    if (name == NULL) {
        return -1;
    }

    size_t name_length = strnlen(name, LINC_DEFAULT_SINK_NAME_LENGTH + LINC_ZERO_CHAR_LENGTH);
    if (name_length == 0 || name_length > LINC_DEFAULT_SINK_NAME_LENGTH) {
        return -1;
    }

    for (size_t i = 0; i < sinks->count; i++) {
        if (strncmp(sinks->list[i].name, name, LINC_DEFAULT_SINK_NAME_LENGTH) == 0) {
            return -1;
        }
    }

    return 0;
}

static int linc_check_level_sink(enum linc_level level) {
    if (level < LINC_LEVEL_INHERIT || level > LINC_LEVEL_FATAL) {
        return -1;
    }
    return 0;
}

static int linc_check_color_mode_sink(enum linc_color_mode color_mode) {
    if (color_mode < LINC_COLOR_MODE_AUTO || color_mode > LINC_COLOR_MODE_ALWAYS) {
        return -1;
    }
    return 0;
}

static int linc_check_formatter_sink(enum linc_formatter formatter) {
    if (formatter < LINC_FORMATTER_TEXT || formatter > LINC_FORMATTER_JSON) {
        return -1;
    }
    return 0;
}

static int linc_check_funcs_sink(struct linc_sink_funcs funcs) {
    if (funcs.open == NULL || funcs.close == NULL || funcs.write == NULL || funcs.flush == NULL) {
        return -1;
    }
    return 0;
}

static struct linc_sink *linc_add_sink(struct linc_sink_list *sinks,
                                       const char *name,
                                       enum linc_level level,
                                       enum linc_color_mode color_mode,
                                       enum linc_formatter formatter,
                                       bool enabled,
                                       struct linc_sink_funcs funcs) {
    bool is_failed = false;
    is_failed |= sinks == NULL;
    is_failed |= sinks->count >= LINC_DEFAULT_MAX_SINKS;
    is_failed |= linc_check_name_sink(sinks, name) < 0;
    is_failed |= linc_check_level_sink(level) < 0;
    is_failed |= linc_check_color_mode_sink(color_mode) < 0;
    is_failed |= linc_check_formatter_sink(formatter) < 0;
    is_failed |= linc_check_funcs_sink(funcs) < 0;

    if (is_failed == true) {
        return NULL;
    }

    struct linc_sink *sink = &sinks->list[sinks->count];
    strcpy(sink->name, name);
    sink->level = level;
    sink->color_mode = color_mode;
    sink->formatter = formatter;
    sink->funcs = funcs;
    sink->enabled = enabled;
    sinks->count += 1;

    sink->funcs.open(sink->funcs.data);
    return sink;
}

void linc_default_sink(struct linc_sink_list *sinks) {
    sinks->count = 0;
    struct linc_sink_funcs funcs = {
        .data = stderr,
        .open = linc_sink_stderr_open,
        .close = linc_sink_stderr_close,
        .write = linc_sink_stderr_write,
        .flush = linc_sink_stderr_flush,
    };
    linc_add_sink(
        sinks, LINC_DEFAULT_SINK_NAME, LINC_LEVEL_TRACE, LINC_COLOR_MODE_AUTO, LINC_FORMATTER_TEXT, true, funcs);
}

// ==================================================
// Public Functions
// ==================================================

linc_sink linc_register_sink(const char *name,
                             enum linc_level level,
                             enum linc_color_mode color_mode,
                             enum linc_formatter formatter,
                             bool enabled,
                             struct linc_sink_funcs funcs) {
    struct linc_sink_list *sinks = linc_get_sinks();
    struct linc_sink *sink = linc_add_sink(sinks, name, level, color_mode, formatter, enabled, funcs);
    if (sink == NULL) {
        return NULL;
    }
    return sink;
}

linc_sink linc_get_sink(const char *name) {
    struct linc_sink_list *sinks = linc_get_sinks();
    if (name == NULL) {
        return NULL;
    }

    size_t name_length = strnlen(name, LINC_DEFAULT_SINK_NAME_LENGTH + LINC_ZERO_CHAR_LENGTH);
    if (name_length == 0 || name_length > LINC_DEFAULT_SINK_NAME_LENGTH) {
        return NULL;
    }

    for (size_t i = 0; i < sinks->count; i++) {
        if (strncmp(sinks->list[i].name, name, LINC_DEFAULT_SINK_NAME_LENGTH) == 0) {
            return &sinks->list[i];
        }
    }

    return NULL;
}

int linc_set_sink_level(linc_sink sink, enum linc_level level) {
    linc_init();
    if (sink == NULL || linc_check_level_sink(level) < 0) {
        return -1;
    }
    sink->level = level;
    return 0;
}

int linc_set_sink_color_mode(linc_sink sink, enum linc_color_mode color_mode) {
    linc_init();
    if (sink == NULL || linc_check_color_mode_sink(color_mode) < 0) {
        return -1;
    }
    sink->color_mode = color_mode;
    return 0;
}

int linc_set_sink_formatter(linc_sink sink, enum linc_formatter formatter) {
    linc_init();
    if (sink == NULL || linc_check_formatter_sink(formatter) < 0) {
        return -1;
    }
    sink->formatter = formatter;
    return 0;
}

int linc_set_sink_enabled(linc_sink sink, bool enabled) {
    linc_init();
    if (sink == NULL) {
        return -1;
    }
    sink->enabled = enabled;
    return 0;
}
