#include "linc.h"

#include <stdio.h>

void test_header(void) {
    printf("%s\n", "==================================================");
    printf("%s\n", "                    LINC TESTS                    ");
    printf("%s\n", "--------------------------------------------------");
    printf("%s\n", "Simple test, as a main function, to check if LINC ");
    printf("%s\n", "works fine.");
    printf("%s\n", "No tests are performed here, just a simple call to");
    printf("%s\n", "a function defined in the LINC library.");
    printf("%s\n", "==================================================");
    printf("\n");
}

void test_footer(void) {
    printf("\n");
    printf("%s\n", "==================================================");
    printf("%s\n", "                 LINC TESTS DONE                  ");
    printf("%s\n", "==================================================");
}

int main(void) {
    test_header();

    // These logs should not appear in the output
    // WHY -> Because the default log level is INFO
    DEBUG("%s", "Log from DEBUG");
    DEBUG_M("main", "%s", "Log from DEBUG_M");
    linc_log("main", LINC_LEVEL_DEBUG, "test/test_linc.c", 31, "main", "%s", "Log from linc_log");

    // These logs should not appear in the output
    // WHY -> Because the default module is "main"
    INFO_M("not_existing", "%s", "Log from INFO_M");
    linc_log("not_existing", LINC_LEVEL_INFO, "test/test_linc.c", 36, "main", "%s", "Log from linc_log");

    // These logs should appear in the output
    // WHY -> Because the default log level is INFO and default module is "main"
    INFO("%s", "Log from INFO");
    INFO_M("main", "%s", "Log from INFO_M");
    linc_log("main", LINC_LEVEL_INFO, "test/test_linc.c", 42, "main", "%s", "Log from linc_log");

    test_footer();
    return 0;
}
