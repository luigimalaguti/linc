#include "linc.h"
#include "test/utils.h"

#include <assert.h>
#include <stdio.h>

void test_invalid_module(void) {
    const char *title = "Testing an invalid add module. Should NOT add module";
    test_group_header(title);

    int result = 0;
    result = linc_add_module("");
    assert(result == -1);

    result = 0;
    result = linc_add_module(NULL);
    assert(result == -1);

    result = 0;
    result = linc_add_module("01234567890123456789");
    assert(result == -1);

    result = 0;
    result = linc_add_module("main");
    assert(result == -1);

    test_group_footer();
}

void test_valid_module(void) {
    const char *title = "Testing a valid add module. SHOULD add module";
    test_group_header(title);

    int result = linc_add_module("testing");
    assert(result == 1);

    INFO_M("testing",
           "Module added at index %d. NAME: %s, LEVEL: %d, ENABLED: %d",
           result,
           "testing",
           LINC_LEVEL_INHERIT,
           true);

    test_group_footer();
}

void test_lower_level_module(void) {
    const char *title = "Testing level < INHERIT of new module. Should NOT print logs";
    test_group_header(title);

    TRACE_M("testing", "%s", "Log from TRACE_M");
    linc_log("testing", LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    DEBUG_M("testing", "%s", "Log from DEBUG_M");
    linc_log("testing", LINC_LEVEL_DEBUG, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    test_group_footer();
}

void test_upper_level_module(void) {
    const char *title = "Testing level >= INHERIT of new module. SHOULD print logs";
    test_group_header(title);

    INFO_M("testing", "%s", "Log from INFO_M");
    linc_log("testing", LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    WARN_M("testing", "%s", "Log from WARN_M");
    linc_log("testing", LINC_LEVEL_WARN, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    ERROR_M("testing", "%s", "Log from ERROR_M");
    linc_log("testing", LINC_LEVEL_ERROR, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    FATAL_M("testing", "%s", "Log from FATAL_M");
    linc_log("testing", LINC_LEVEL_FATAL, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    test_group_footer();
}

void test_set_invalid_enabled_module(void) {
    const char *title = "Testing invalid enabled of new module. SHOULD print logs";
    test_group_header(title);

    int result = 0;
    result = linc_set_module_enabled(NULL, false);
    assert(result == -1);

    INFO_M("testing", "%s", "Log from INFO_M");
    linc_log("testing", LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    result = 0;
    result = linc_set_module_enabled("", false);
    assert(result == -1);

    INFO_M("testing", "%s", "Log from INFO_M");
    linc_log("testing", LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    result = 0;
    result = linc_set_module_enabled("01234567890123456789", false);
    assert(result == -1);

    INFO_M("testing", "%s", "Log from INFO_M");
    linc_log("testing", LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    result = 0;
    result = linc_set_module_enabled("not_existing", false);
    assert(result == -1);

    INFO_M("testing", "%s", "Log from INFO_M");
    linc_log("testing", LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    test_group_footer();
}

void test_set_not_enabled_module(void) {
    const char *title = "Testing enabled = FALSE of new module. Should NOT print logs";
    test_group_header(title);

    int result = linc_set_module_enabled("testing", false);
    assert(result == 0);

    TRACE_M("testing", "%s", "Log from TRACE_M");
    linc_log("testing", LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    DEBUG_M("testing", "%s", "Log from DEBUG_M");
    linc_log("testing", LINC_LEVEL_DEBUG, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    INFO_M("testing", "%s", "Log from INFO_M");
    linc_log("testing", LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    WARN_M("testing", "%s", "Log from WARN_M");
    linc_log("testing", LINC_LEVEL_WARN, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    ERROR_M("testing", "%s", "Log from ERROR_M");
    linc_log("testing", LINC_LEVEL_ERROR, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    FATAL_M("testing", "%s", "Log from FATAL_M");
    linc_log("testing", LINC_LEVEL_FATAL, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    test_group_footer();
}

void test_set_enabled_module(void) {
    const char *title = "Testing enabled = TRUE of new module. SHOULD print logs";
    test_group_header(title);

    int result = linc_set_module_enabled("testing", true);
    assert(result == 0);

    INFO_M("testing", "%s", "Log from INFO_M");
    linc_log("testing", LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    WARN_M("testing", "%s", "Log from WARN_M");
    linc_log("testing", LINC_LEVEL_WARN, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    ERROR_M("testing", "%s", "Log from ERROR_M");
    linc_log("testing", LINC_LEVEL_ERROR, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    FATAL_M("testing", "%s", "Log from FATAL_M");
    linc_log("testing", LINC_LEVEL_FATAL, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    test_group_footer();
}

void test_set_invalid_level_module(void) {
    const char *title = "Testing invalid level of new module. Should NOT print logs";
    test_group_header(title);

    int result = 0;
    result = linc_set_module_level(NULL, LINC_LEVEL_TRACE);
    assert(result == -1);

    TRACE_M("testing", "%s", "Log from TRACE_M");
    linc_log("testing", LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    result = 0;
    result = linc_set_module_level("", LINC_LEVEL_TRACE);
    assert(result == -1);

    TRACE_M("testing", "%s", "Log from TRACE_M");
    linc_log("testing", LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    result = 0;
    result = linc_set_module_level("01234567890123456789", LINC_LEVEL_TRACE);
    assert(result == -1);

    TRACE_M("testing", "%s", "Log from TRACE_M");
    linc_log("testing", LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    result = 0;
    result = linc_set_module_level("not_existing", LINC_LEVEL_TRACE);
    assert(result == -1);

    TRACE_M("testing", "%s", "Log from TRACE_M");
    linc_log("testing", LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    result = 0;
    result = linc_set_module_level("testing", -2);
    assert(result == -1);

    TRACE_M("testing", "%s", "Log from TRACE_M");
    linc_log("testing", LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    result = 0;
    result = linc_set_module_level("testing", 6);
    assert(result == -1);

    TRACE_M("testing", "%s", "Log from TRACE_M");
    linc_log("testing", LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    test_group_footer();
}

void test_set_valid_level_module(void) {
    const char *title = "Testing valid level of new module. SHOULD print logs";
    test_group_header(title);

    int result = 0;
    result = linc_set_module_level("testing", LINC_LEVEL_INHERIT);
    assert(result == 0);

    result = 0;
    result = linc_set_module_level("testing", LINC_LEVEL_TRACE);
    assert(result == 0);

    TRACE_M("testing", "%s", "Log from TRACE_M");
    linc_log("testing", LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    DEBUG_M("testing", "%s", "Log from DEBUG_M");
    linc_log("testing", LINC_LEVEL_DEBUG, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    INFO_M("testing", "%s", "Log from INFO_M");
    linc_log("testing", LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    test_group_footer();
}

void test_default_and_new_module(void) {
    const char *title = "Testing unchanged default module level. Should NOT print logs";
    test_group_header(title);

    TRACE("%s", "Log from TRACE");
    TRACE_M("main", "%s", "Log from TRACE_M");
    linc_log("main", LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    DEBUG("%s", "Log from DEBUG");
    DEBUG_M("main", "%s", "Log from DEBUG_M");
    linc_log("main", LINC_LEVEL_DEBUG, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    test_group_footer();
}

void test_set_module_level_of_default(void) {
    const char *title = "Testing set level of default module by module APIs. SHOULD print logs";
    test_group_header(title);

    int result = linc_set_module_level("main", LINC_LEVEL_TRACE);
    assert(result == 0);

    TRACE("%s", "Log from TRACE");
    TRACE_M("main", "%s", "Log from TRACE_M");
    linc_log("main", LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    DEBUG("%s", "Log from DEBUG");
    DEBUG_M("main", "%s", "Log from DEBUG_M");
    linc_log("main", LINC_LEVEL_DEBUG, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    test_group_footer();
}

void test_max_modules(void) {
    const char *title = "Testing adding max modules. Should add up to max modules";
    test_group_header(title);

    INFO_M("main", "Log message from module %s", "main");
    INFO_M("testing", "Log message from module %s", "testing");

    int already_added = 2;
    for (int i = already_added; i < LINC_MODULES_MAX_NUMBER; i++) {
        char module_name[2];
        snprintf(module_name, sizeof(module_name), "%d", i);
        int result = linc_add_module(module_name);
        assert(result == i);

        INFO_M(module_name, "Log message from module %s", module_name);
    }

    const char *overflow_buffer = "OVERFLOW";
    int result = linc_add_module(overflow_buffer);
    assert(result == -1);

    test_group_footer();
}

int main(void) {
    const char *title_header = "MODULES APIs TESTS";
    const char *title_footer = "MODULES APIs TESTS DONE";
    const char *description =
        "This test checks the MODULES APIs, add and set modules.\n\n"
        "ENVIRONMENT VARIABLES:\n"
        "  - LEVEL: INFO\n"
        "  - MODULES: [{ main, enabled, INHERIT }]";

    test_header(title_header, description);

    test_invalid_module();
    test_valid_module();
    test_lower_level_module();
    test_upper_level_module();
    test_set_invalid_enabled_module();
    test_set_not_enabled_module();
    test_set_enabled_module();
    test_set_invalid_level_module();
    test_set_valid_level_module();
    test_default_and_new_module();
    test_set_module_level_of_default();
    test_max_modules();

    test_footer(title_footer);
    return 0;
}
