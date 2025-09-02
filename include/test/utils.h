#ifndef LINC_INCLUDE_TEST_UTILS_H_
#define LINC_INCLUDE_TEST_UTILS_H_

#define LINC_TEST_PAGE_WIDTH 80
#define LINC_TEST_PAGE_TEST_SECTION_CHAR '='
#define LINC_TEST_PAGE_INFO_SECTION_CHAR '-'

#define LINC_TEST_PAGE_GROUP_SECTION_REPEAT 5
#define LINC_TEST_PAGE_GROUP_START_SECTION_CHAR '>'
#define LINC_TEST_PAGE_GROUP_END_SECTION_CHAR '<'

void test_header(const char *title, const char *description);
void test_footer(const char *title);

void test_group_header(const char *title);
void test_group_footer(void);

#endif  // LINC_INCLUDE_TEST_UTILS_H_
