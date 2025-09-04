#include "linc.h"
#include "test/utils.h"

#include <assert.h>
#include <string.h>

void test_right_values(void) {
    const char *title = "Testing level >= INFO and module \"main\". SHOULD print logs";
    test_group_header(title);

    INFO("%s", "Log from INFO");
    INFO_M("main", "%s", "Log from INFO_M");
    linc_log("main", LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    WARN("%s", "Log from WARN");
    WARN_M("main", "%s", "Log from WARN_M");
    linc_log("main", LINC_LEVEL_WARN, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    ERROR("%s", "Log from ERROR");
    ERROR_M("main", "%s", "Log from ERROR_M");
    linc_log("main", LINC_LEVEL_ERROR, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    FATAL("%s", "Log from FATAL");
    FATAL_M("main", "%s", "Log from FATAL_M");
    linc_log("main", LINC_LEVEL_FATAL, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    INFO_M(NULL, "%s", "Log from INFO_M");
    linc_log(NULL, LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    test_group_footer();
}

void test_lower_level(void) {
    const char *title = "Testing level < INFO. Should NOT print logs";
    test_group_header(title);

    TRACE("%s", "Log from TRACE");
    TRACE_M("main", "%s", "Log from TRACE_M");
    linc_log("main", LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    DEBUG("%s", "Log from DEBUG");
    DEBUG_M("main", "%s", "Log from DEBUG_M");
    linc_log("main", LINC_LEVEL_DEBUG, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    test_group_footer();
}

void test_invalid_level(void) {
    const char *title = "Testing invalid level. Should NOT print logs";
    test_group_header(title);

    linc_log("main", LINC_LEVEL_INHERIT, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");
    linc_log("main", -2, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");
    linc_log("main", 6, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    test_group_footer();
}

void test_invalid_module(void) {
    const char *title = "Testing invalid module. Should NOT print logs";
    test_group_header(title);

    INFO_M("not_existing", "%s", "Log from INFO_M");
    linc_log("not_existing", LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");
    linc_log("", LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    test_group_footer();
}

void test_invalid_set_level(void) {
    const char *title = "Testing change default level to invalid value. Should NOT print logs";
    test_group_header(title);

    int result = 0;
    result = linc_set_level(LINC_LEVEL_INHERIT);
    assert(result == -1);

    TRACE("%s", "Log from TRACE");
    TRACE_M("main", "%s", "Log from TRACE_M");
    linc_log("main", LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    DEBUG("%s", "Log from DEBUG");
    DEBUG_M("main", "%s", "Log from DEBUG_M");
    linc_log("main", LINC_LEVEL_DEBUG, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    result = 0;
    result = linc_set_level(6);
    assert(result == -1);

    TRACE("%s", "Log from TRACE");
    TRACE_M("main", "%s", "Log from TRACE_M");
    linc_log("main", LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    DEBUG("%s", "Log from DEBUG");
    DEBUG_M("main", "%s", "Log from DEBUG_M");
    linc_log("main", LINC_LEVEL_DEBUG, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    result = 0;
    result = linc_set_level(-2);
    assert(result == -1);

    TRACE("%s", "Log from TRACE");
    TRACE_M("main", "%s", "Log from TRACE_M");
    linc_log("main", LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    DEBUG("%s", "Log from DEBUG");
    DEBUG_M("main", "%s", "Log from DEBUG_M");
    linc_log("main", LINC_LEVEL_DEBUG, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    test_group_footer();
}

void test_valid_set_level(void) {
    const char *title = "Testing change default level to TRACE. SHOULD print logs";
    test_group_header(title);

    int result = linc_set_level(LINC_LEVEL_TRACE);
    assert(result == 0);

    TRACE("%s", "Log from TRACE");
    TRACE_M("main", "%s", "Log from TRACE_M");
    linc_log("main", LINC_LEVEL_TRACE, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    DEBUG("%s", "Log from DEBUG");
    DEBUG_M("main", "%s", "Log from DEBUG_M");
    linc_log("main", LINC_LEVEL_DEBUG, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    INFO("%s", "Log from INFO");
    INFO_M("main", "%s", "Log from INFO_M");
    linc_log("main", LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    WARN("%s", "Log from WARN");
    WARN_M("main", "%s", "Log from WARN_M");
    linc_log("main", LINC_LEVEL_WARN, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    ERROR("%s", "Log from ERROR");
    ERROR_M("main", "%s", "Log from ERROR_M");
    linc_log("main", LINC_LEVEL_ERROR, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    FATAL("%s", "Log from FATAL");
    FATAL_M("main", "%s", "Log from FATAL_M");
    linc_log("main", LINC_LEVEL_FATAL, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    INFO_M(NULL, "%s", "Log from INFO_M");
    linc_log(NULL, LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, "%s", "Log from linc_log");

    test_group_footer();
}

void test_very_long_messages(void) {
    const char *title = "Testing very long messages. SHOULD truncate gracefully";
    test_group_header(title);

    char long_message[LINC_LOG_MESSAGE_MAX_LENGTH + 100];
    memset(long_message, 'A', sizeof(long_message) - 1);
    long_message[sizeof(long_message) - 1] = '\0';

    INFO("Long message: %s", long_message);

    test_group_footer();
}

void test_null_parameters(void) {
    const char *title = "Testing NULL parameters. SHOULD handle gracefully";
    test_group_header(title);

    // INFO(NULL, NULL);  // -> Compiler error: Too many arguments for format
    // INFO_M("main", NULL, NULL); // -> Compiler error: Too many arguments for format
    linc_log("main", LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, NULL);
    // linc_log("main", LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, NULL, NULL); // -> Compiler error: Too many
    // arguments for format

    INFO("%s", "");
    // INFO("%s", NULL); // -> Compiler error: '%s' directive argument is null
    INFO_M("main", "%s", "");
    // INFO_M("main", "%s", NULL); // -> Compiler error: '%s' directive argument is null
    linc_log("main", LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, "%s", "");
    // linc_log("main", LINC_LEVEL_INFO, __FILE__, __LINE__, __func__, "%s", NULL); // -> Compiler error: '%s' directive
    // argument is null

    linc_log("main", LINC_LEVEL_INFO, NULL, 0, NULL, "Message with NULL file/func and line 0");
    linc_log("main", LINC_LEVEL_INFO, NULL, -1, NULL, "Message with NULL file/func and line -1");

    test_group_footer();
}

int main(void) {
    const char *title_header = "DEFAULT STATE TESTS";
    const char *title_footer = "DEFAULT STATE TESTS DONE";
    const char *description =
        "Testing default values of the linc library.\n\n"
        "ENVIRONMENT VARIABLES:\n"
        "  - LEVEL: INFO\n"
        "  - MODULES: [{ main, enabled, INHERIT }]";

    test_header(title_header, description);

    test_right_values();
    test_lower_level();
    test_invalid_level();
    test_invalid_module();
    test_invalid_set_level();
    test_valid_set_level();
    test_very_long_messages();
    test_null_parameters();

    test_footer(title_footer);
    return 0;
}
