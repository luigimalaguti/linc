#include "linc.h"
#include "test/utils.h"

#include <assert.h>
#include <stdio.h>

struct sink_file {
    const char *filename;
    FILE *file;
};

struct sink_file sink_file_data = {
    .filename = "test/test_sink.log",
};

int sink_file_open(void *data) {
    struct sink_file *sink_file = (struct sink_file *)data;

    FILE *file = fopen(sink_file->filename, "w");
    if (!file) {
        return -1;
    }

    sink_file->file = file;
    return 0;
}

int sink_file_wrong_open(void *data) {
    (void)data;
    return -1;
}

int sink_file_close(void *data) {
    struct sink_file *sink_file = (struct sink_file *)data;

    if (sink_file->file == NULL) {
        return -1;
    }

    fclose(sink_file->file);
    sink_file->file = NULL;
    return 0;
}

int sink_file_write(void *data, const char *buffer, size_t length) {
    struct sink_file *sink_file = (struct sink_file *)data;

    if (sink_file->file == NULL) {
        return -1;
    }
    size_t written = fwrite(buffer, sizeof(char), length, sink_file->file);
    if (written != length) {
        return -1;
    }

    return 0;
}

int sink_file_flush(void *data) {
    struct sink_file *sink_file = (struct sink_file *)data;

    if (sink_file->file == NULL) {
        return -1;
    }

    fflush(sink_file->file);
    return 0;
}

void test_invalid_sink(void) {
    const char *title = "Testing an invalid add sink. Should NOT add sink";
    test_group_header(title);

    int result = 0;
    struct linc_sink_entry entry;
    entry.data = &sink_file_data;

    entry.open = NULL;
    entry.close = sink_file_close;
    entry.write = sink_file_write;
    entry.flush = sink_file_flush;
    result = linc_add_sink(entry);
    assert(result == -1);

    entry.open = sink_file_open;
    entry.close = NULL;
    entry.write = sink_file_write;
    entry.flush = sink_file_flush;
    result = linc_add_sink(entry);
    assert(result == -1);

    entry.open = sink_file_open;
    entry.close = sink_file_close;
    entry.write = NULL;
    entry.flush = sink_file_flush;
    result = linc_add_sink(entry);
    assert(result == -1);

    entry.open = sink_file_open;
    entry.close = sink_file_close;
    entry.write = sink_file_write;
    entry.flush = NULL;
    result = linc_add_sink(entry);
    assert(result == -1);

    entry.open = sink_file_wrong_open;
    entry.close = sink_file_close;
    entry.write = sink_file_write;
    entry.flush = sink_file_flush;
    result = linc_add_sink(entry);
    assert(result == -1);

    test_group_footer();
}

void test_valid_sink(void) {
    const char *title = "Testing a valid add sink. SHOULD add sink";
    test_group_header(title);

    struct linc_sink_entry entry;
    entry.data = &sink_file_data;
    entry.open = sink_file_open;
    entry.close = sink_file_close;
    entry.write = sink_file_write;
    entry.flush = sink_file_flush;

    int result = linc_add_sink(entry);
    assert(result == 1);

    INFO("%s", "Log from INFO");
    INFO_M("main", "%s", "Log from INFO_M");

    test_group_footer();
}

void test_sink_level_inherit(void) {
    const char *title = "Testing sink level INHERIT. SHOULD log";
    test_group_header(title);

    DEBUG("%s", "Log from DEBUG");
    DEBUG_M("main", "%s", "Log from DEBUG_M");
    INFO("%s", "Log from INFO");
    INFO_M("main", "%s", "Log from INFO_M");

    test_group_footer();
}

void test_set_invalid_sink_level(void) {
    const char *title = "Testing an invalid set sink level. Should NOT set sink level";
    test_group_header(title);

    int result = 0;
    result = linc_set_sink_level(-1, LINC_LEVEL_INFO);
    assert(result == -1);

    result = 0;
    result = linc_set_sink_level(3, LINC_LEVEL_INFO);
    assert(result == -1);

    result = 0;
    result = linc_set_sink_level(1, -2);
    assert(result == -1);

    result = 0;
    result = linc_set_sink_level(1, 6);
    assert(result == -1);

    test_group_footer();
}

void test_set_valid_default_sink_level(void) {
    const char *title = "Testing a valid set default sink level. SHOULD set sink level";
    test_group_header(title);

    linc_set_module_level("main", LINC_LEVEL_DEBUG);

    int result = -1;
    result = linc_set_sink_level(0, LINC_LEVEL_TRACE);
    assert(result == 0);

    // Not printed because module level is DEBUG
    TRACE("%s", "Log from TRACE");
    TRACE_M("main", "%s", "Log from TRACE_M");

    // Printed because module level is DEBUG and sink level is TRACE
    DEBUG("%s", "Log from DEBUG");
    DEBUG_M("main", "%s", "Log from DEBUG_M");

    result = -1;
    result = linc_set_sink_level(0, LINC_LEVEL_INHERIT);
    assert(result == 0);

    // Not printed because module level is DEBUG and sink level is INHERIT (INFO)
    DEBUG("%s", "Log from DEBUG");
    DEBUG_M("main", "%s", "Log from DEBUG_M");

    linc_set_level(LINC_LEVEL_DEBUG);
    // Now printed because we set the default level to DEBUG
    DEBUG("%s", "Log from DEBUG");
    DEBUG_M("main", "%s", "Log from DEBUG_M");

    test_group_footer();
}

void test_set_valid_sink_level(void) {
    const char *title = "Testing a valid set sink level. SHOULD set sink level";
    test_group_header(title);

    int result = linc_set_sink_level(1, LINC_LEVEL_WARN);
    assert(result == 0);

    INFO("%s", "Log from INFO");
    ERROR("%s", "Log from ERROR");

    test_group_footer();
}

void test_set_invalid_color_mode(void) {
    const char *title = "Testing an invalid set color mode. Should NOT set color mode";
    test_group_header(title);

    int result = 0;
    result = linc_set_sink_color_mode(-1, LINC_COLOR_MODE_AUTO);
    assert(result == -1);

    result = 0;
    result = linc_set_sink_color_mode(3, LINC_COLOR_MODE_AUTO);
    assert(result == -1);

    result = 0;
    result = linc_set_sink_color_mode(1, -1);
    assert(result == -1);

    result = 0;
    result = linc_set_sink_color_mode(1, 3);
    assert(result == -1);

    test_group_footer();
}

void test_set_valid_color_mode(void) {
    const char *title = "Testing a valid set color mode. SHOULD set color mode";
    test_group_header(title);

    int result = linc_set_sink_color_mode(1, LINC_COLOR_MODE_ALWAYS);
    assert(result == 0);

    test_group_footer();
}

void test_set_invalid_formatter(void) {
    const char *title = "Testing an invalid set formatter. Should NOT set formatter";
    test_group_header(title);

    int result = 0;
    result = linc_set_sink_formatter(-1, LINC_FORMAT_JSON);
    assert(result == -1);

    result = 0;
    result = linc_set_sink_formatter(3, LINC_FORMAT_JSON);
    assert(result == -1);

    result = 0;
    result = linc_set_sink_formatter(1, -1);
    assert(result == -1);

    result = 0;
    result = linc_set_sink_formatter(1, 3);
    assert(result == -1);

    test_group_footer();
}

void test_set_valid_formatter(void) {
    const char *title = "Testing a valid set formatter. SHOULD set formatter";
    test_group_header(title);

    int result = linc_set_sink_formatter(1, LINC_FORMAT_JSON);
    assert(result == 0);

    test_group_footer();
}

int main(void) {
    const char *title_header = "SINKS APIs TESTS";
    const char *title_footer = "SINKS APIs TESTS DONE";
    const char *description =
        "This test checks the SINKS APIs, add and set sinks.\n\n"
        "ENVIRONMENT VARIABLES:\n"
        "  - LEVEL: INFO\n"
        "  - SINKS: [{ stderr, enabled, INHERIT, ... }]";
    test_header(title_header, description);

    test_invalid_sink();
    test_valid_sink();
    test_sink_level_inherit();
    test_set_invalid_sink_level();
    test_set_valid_default_sink_level();
    test_set_valid_sink_level();
    test_set_invalid_color_mode();
    test_set_invalid_formatter();
    test_set_valid_formatter();

    test_footer(title_footer);
    return 0;
}
