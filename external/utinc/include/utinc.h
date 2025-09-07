#ifndef LINC_INCLUDE_TEST_UTINC_H
#define LINC_INCLUDE_TEST_UTINC_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// ==================================================
// Statistics and State
// ==================================================

struct utinc {
    int test_suites_count;
    int test_cases_count;

    int test_suites_passed;
    int test_suites_failed;
    int test_cases_passed;
    int test_cases_failed;

    bool current_test_suites_failed;
    bool current_test_cases_failed;

    int64_t file_start_time;
    int64_t suite_start_time;

    // Callback function pointers
    void (*before_all)(void);
    void (*after_all)(void);
    void (*before_each)(void);
    void (*after_each)(void);
};

// ==================================================
// Functions
// ==================================================

void utinc_test_file_init(const char *title);
void utinc_test_file_run(void);
void utinc_test_file_finish(void);

void utinc_test_suite_start(const char *description);
void utinc_test_suite_end(void);

void utinc_test_case_start(const char *description);
void utinc_test_case_fail(const char *file, int line, const char *message);
bool utinc_test_case_should_continue(void);
void utinc_test_case_end(void);

// Callback registration functions
void utinc_register_before_all(void (*callback)(void));
void utinc_register_after_all(void (*callback)(void));
void utinc_register_before_each(void (*callback)(void));
void utinc_register_after_each(void (*callback)(void));

int64_t utinc_timestamp(void);

// ==================================================
// Test File Macro
// ==================================================

#define UTINC_MAIN(title)            \
    int main(void) {                 \
        utinc_test_file_init(title); \
        utinc_test_file_run();       \
        utinc_test_file_finish();    \
        return 0;                    \
    }                                \
    void utinc_test_file_run(void)

// ==================================================
// Test Suite Macros
// ==================================================

#define UTINC_SUITE(description) \
    do {                         \
        utinc_test_suite_start(description);

#define END_UTINC_SUITE()   \
    utinc_test_suite_end(); \
    }                       \
    while (false)

// ==================================================
// Test Case Macros
// ==================================================

#define UTINC_CASE(description)             \
    do {                                    \
        utinc_test_case_start(description); \
        do

#define END_UTINC_CASE()   \
    while (0);             \
    utinc_test_case_end(); \
    }                      \
    while (false)

// ==================================================
// Test Runner Setup and Teardown
// ==================================================

// Define callback function and register it
#define UTINC_BEFORE_ALL(func_name) utinc_register_before_all(func_name)
#define UTINC_AFTER_ALL(func_name) utinc_register_after_all(func_name)
#define UTINC_BEFORE_EACH(func_name) utinc_register_before_each(func_name)
#define UTINC_AFTER_EACH(func_name) utinc_register_after_each(func_name)

// Helper macro to define callback functions inline (for simple cases)
#define UTINC_DEFINE_CALLBACK(name, code) void name(void) code

// ==================================================
// Test Runner Assert Macros
// ==================================================

#define UTINC_ASSERT(condition, message)                       \
    do {                                                       \
        if (utinc_test_case_should_continue() == false) break; \
        if ((condition) == false) {                            \
            utinc_test_case_fail(__FILE__, __LINE__, message); \
        }                                                      \
    } while (false)

#define UTINC_ASSERT_EQUAL(expected, actual, message)               \
    do {                                                            \
        if (utinc_test_case_should_continue() == false) break;      \
        if ((expected) != (actual)) {                               \
            char fail_message[256];                                 \
            snprintf(fail_message,                                  \
                     sizeof(fail_message),                          \
                     "%s (expected: %d, actual: %d)",               \
                     message,                                       \
                     (int)(expected),                               \
                     (int)(actual));                                \
            utinc_test_case_fail(__FILE__, __LINE__, fail_message); \
        }                                                           \
    } while (false)

#define UTINC_ASSERT_NULL(pointer, message)                    \
    do {                                                       \
        if (utinc_test_case_should_continue() == false) break; \
        if ((pointer) != NULL) {                               \
            utinc_test_case_fail(__FILE__, __LINE__, message); \
        }                                                      \
    } while (false)

#define UTINC_ASSERT_NOT_NULL(pointer, message)                \
    do {                                                       \
        if (utinc_test_case_should_continue() == false) break; \
        if ((pointer) == NULL) {                               \
            utinc_test_case_fail(__FILE__, __LINE__, message); \
        }                                                      \
    } while (false)

#define UTINC_ASSERT_STRING_EQUAL(expected, actual, message)        \
    do {                                                            \
        if (utinc_test_case_should_continue() == false) break;      \
        if (strcmp((expected), (actual)) != 0) {                    \
            char fail_message[512];                                 \
            snprintf(fail_message,                                  \
                     sizeof(fail_message),                          \
                     "%s (expected: '%s', actual: '%s')",           \
                     message,                                       \
                     (expected),                                    \
                     (actual));                                     \
            utinc_test_case_fail(__FILE__, __LINE__, fail_message); \
        }                                                           \
    } while (false)

#endif  // LINC_INCLUDE_TEST_UTINC_H
