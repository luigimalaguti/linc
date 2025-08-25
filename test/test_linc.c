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

    INFO("%s", "Log from INFO");
    INFO_M("main", "%s", "Log from INFO_M");
    linc_log("main", LINC_LEVEL_INFO, "test/test_linc.c", 26, "main", "%s", "Log from linc_log");

    test_footer();
    return 0;
}
