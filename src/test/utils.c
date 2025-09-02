#include "test/utils.h"

#include <stdio.h>
#include <string.h>

void test_header(const char *title, const char *description) {
    if (title == NULL || strlen(title) == 0 || strlen(title) > LINC_TEST_PAGE_WIDTH) {
        title = "No Title";
    }
    if (description == NULL || strlen(description) == 0) {
        description = "No Description";
    }

    printf("%c", '\n');
    // ================================================================================
    for (int i = 0; i < LINC_TEST_PAGE_WIDTH; i++) {
        printf("%c", LINC_TEST_PAGE_TEST_SECTION_CHAR);
    }
    printf("%c", '\n');

    //                                    LINC TESTS
    int title_padding = (LINC_TEST_PAGE_WIDTH - (int)strlen(title)) / 2;
    for (int i = 0; i < title_padding; i++) {
        printf("%c", ' ');
    }
    printf("%s\n", title);

    // --------------------------------------------------------------------------------
    for (int i = 0; i < LINC_TEST_PAGE_WIDTH; i++) {
        printf("%c", LINC_TEST_PAGE_INFO_SECTION_CHAR);
    }
    printf("%c", '\n');

    // Simple test, as a main function, to check if LINC works fine.
    // No tests are performed here, just a simple call to a function defined in the
    // LINC library.
    int paging = 0;
    for (int i = 0; i < (int)strlen(description); i++) {
        printf("%c", description[i]);
        if (description[i] == '\n') {
            paging = 0;
        } else {
            paging++;
        }
        if ((paging + 1) % LINC_TEST_PAGE_WIDTH == 0) {
            printf("%c", '\n');
        }
    }
    printf("%c", '\n');

    // --------------------------------------------------------------------------------
    for (int i = 0; i < LINC_TEST_PAGE_WIDTH; i++) {
        printf("%c", LINC_TEST_PAGE_INFO_SECTION_CHAR);
    }
    printf("%s", "\n\n");
}

void test_footer(const char *title) {
    if (title == NULL || strlen(title) == 0 || strlen(title) > LINC_TEST_PAGE_WIDTH) {
        title = "No Title";
    }
    printf("%c", '\n');

    // --------------------------------------------------------------------------------
    for (int i = 0; i < LINC_TEST_PAGE_WIDTH; i++) {
        printf("%c", LINC_TEST_PAGE_INFO_SECTION_CHAR);
    }
    printf("%c", '\n');

    //                                    LINC TESTS
    int title_padding = (LINC_TEST_PAGE_WIDTH - (int)strlen(title)) / 2;
    for (int i = 0; i < title_padding; i++) {
        printf("%c", ' ');
    }
    printf("%s\n", title);

    // ================================================================================
    for (int i = 0; i < LINC_TEST_PAGE_WIDTH; i++) {
        printf("%c", LINC_TEST_PAGE_TEST_SECTION_CHAR);
    }
    printf("%s", "\n\n");
}

void test_group_header(const char *title) {
    if (title == NULL || strlen(title) == 0 || strlen(title) > LINC_TEST_PAGE_WIDTH) {
        title = "No Title";
    }

    for (int i = 0; i < LINC_TEST_PAGE_GROUP_SECTION_REPEAT; i++) {
        printf("%c", LINC_TEST_PAGE_GROUP_START_SECTION_CHAR);
    }
    printf(" %s\n", title);
}

void test_group_footer(void) {
    for (int i = 0; i < LINC_TEST_PAGE_GROUP_SECTION_REPEAT; i++) {
        printf("%c", LINC_TEST_PAGE_GROUP_END_SECTION_CHAR);
    }
    printf("\n");
}
