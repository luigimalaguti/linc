#include "utinc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ==================================================
// Global Variables
// ==================================================

static struct utinc utinc = {0};

// ==================================================
// Utils Functions
// ==================================================

#if defined(CLOCK_MONOTONIC_RAW)
#define CLOCK_GETTIME(ts) clock_gettime(CLOCK_MONOTONIC_RAW, ts)
#else
#define CLOCK_GETTIME(ts) clock_gettime(CLOCK_MONOTONIC, ts)
#endif

int64_t utinc_timestamp(void) {
    struct timespec ts;
    CLOCK_GETTIME(&ts);
    int64_t timestamp = (int64_t)ts.tv_sec * 1000000000L + (int64_t)ts.tv_nsec;
    return timestamp;
}

static void utinc_print_chars(char c, int length, bool newline) {
    for (int i = 0; i < length; i++) {
        printf("%c", c);
    }
    if (newline == true) {
        printf("\n");
    }
}

#define UTINC_COLOR_RESET "\x1b[0m"
#define UTINC_COLOR_BOLD "\x1b[1m"
#define UTINC_COLOR_LOW "\x1b[2m"
#define UTINC_COLOR_RED "\x1b[31m"
#define UTINC_COLOR_GREEN "\x1b[32m"
#define UTINC_COLOR_YELLOW "\x1b[33m"
#define UTINC_COLOR_BLUE "\x1b[34m"
#define UTINC_COLOR_CYAN "\x1b[36m"
#define UTINC_COLOR_GRAY "\x1b[90m"

#define UTINC_LINE_WIDTH 80

static void utinc_print_file_header(const char *title) {
    printf("\n");
    printf("%s", UTINC_COLOR_BOLD);
    utinc_print_chars('#', UTINC_LINE_WIDTH, true);
    printf("🗃️  %s\n", title);
    printf("%s", UTINC_COLOR_RESET);
    utinc_print_chars('=', UTINC_LINE_WIDTH, true);
    printf("\n");
}

void utinc_test_file_init(const char *title) {
    memset(&utinc, 0, sizeof(utinc));
    utinc.file_start_time = utinc_timestamp();
    utinc_print_file_header(title);
}

static void utinc_print_file_footer(int64_t execution_time) {
    utinc_print_chars('=', UTINC_LINE_WIDTH, true);
    printf("%s", UTINC_COLOR_BOLD);
    printf("📊 %s\n", "TEST FILE SUMMARY");
    printf("%s", UTINC_COLOR_RESET);
    utinc_print_chars('-', UTINC_LINE_WIDTH, true);
    printf("Total Test Suites: %s%d%s\n", UTINC_COLOR_CYAN, utinc.test_suites_count, UTINC_COLOR_RESET);
    if (utinc.test_suites_passed > 0) {
        printf("    Passed: %s%d%s ✅\n", UTINC_COLOR_GREEN, utinc.test_suites_passed, UTINC_COLOR_RESET);
    }
    if (utinc.test_suites_failed > 0) {
        printf("    Failed: %s%d%s ❌\n", UTINC_COLOR_RED, utinc.test_suites_failed, UTINC_COLOR_RESET);
    }
    printf(" Total Test Cases: %s%d%s\n", UTINC_COLOR_CYAN, utinc.test_cases_count, UTINC_COLOR_RESET);
    if (utinc.test_cases_passed > 0) {
        printf("    Passed: %s%d%s ✅\n", UTINC_COLOR_GREEN, utinc.test_cases_passed, UTINC_COLOR_RESET);
    }
    if (utinc.test_cases_failed > 0) {
        printf("    Failed: %s%d%s ❌\n", UTINC_COLOR_RED, utinc.test_cases_failed, UTINC_COLOR_RESET);
    }
    utinc_print_chars('-', UTINC_LINE_WIDTH, true);
    printf("%sCompleted in %.3f ms%s\n", UTINC_COLOR_GRAY, execution_time / 1e6, UTINC_COLOR_RESET);
    utinc_print_chars('=', UTINC_LINE_WIDTH, true);
    if (utinc.test_cases_failed == 0) {
        printf("🎉 %sRESULT: ALL TESTS PASSED%s\n", UTINC_COLOR_BOLD UTINC_COLOR_GREEN, UTINC_COLOR_RESET);
    } else {
        printf("💥 %sRESULT: %d TEST%s IN %d SUITE%s FAILED%s\n",
               UTINC_COLOR_BOLD UTINC_COLOR_RED,
               utinc.test_cases_failed,
               utinc.test_cases_failed == 1 ? "" : "S",
               utinc.test_suites_failed,
               utinc.test_suites_failed == 1 ? "" : "S",
               UTINC_COLOR_RESET);
    }
    printf("%s", UTINC_COLOR_BOLD);
    utinc_print_chars('#', UTINC_LINE_WIDTH, true);
    printf("%s", UTINC_COLOR_RESET);
    printf("\n");
}

void utinc_test_file_finish(void) {
    int64_t finish_timestamp = utinc_timestamp();
    int64_t total_time = finish_timestamp - utinc.file_start_time;
    utinc_print_file_footer(total_time);
    if (utinc.test_cases_failed > 0) {
        exit(1);
    }
}

void utinc_test_suite_start(const char *description) {
    utinc.test_suites_count++;
    utinc.current_test_suites_failed = false;
    utinc.suite_start_time = utinc_timestamp();

    printf("🗂️  %s%s%s\n", UTINC_COLOR_BOLD, description, UTINC_COLOR_RESET);
    utinc_print_chars('-', UTINC_LINE_WIDTH, true);
}

void utinc_test_suite_end(void) {
    // Execute AFTER_ALL callback if registered
    if (utinc.after_all != NULL) {
        utinc.after_all();
    }

    double suite_time = utinc_timestamp() - utinc.suite_start_time;
    utinc_print_chars('-', UTINC_LINE_WIDTH, true);
    printf("%s(%.3f ms)%s\n\n", UTINC_COLOR_GRAY, suite_time / 1e6, UTINC_COLOR_RESET);
    if (utinc.current_test_suites_failed) {
        utinc.test_suites_failed++;
    } else {
        utinc.test_suites_passed++;
    }

    // Clear callbacks for next suite
    utinc.before_all = NULL;
    utinc.after_all = NULL;
    utinc.before_each = NULL;
    utinc.after_each = NULL;
}

void utinc_test_case_start(const char *description) {
    utinc.test_cases_count++;
    utinc.current_test_cases_failed = false;

    printf("🧪 %s\n", description);
    printf("%s", UTINC_COLOR_LOW UTINC_COLOR_GRAY);
    fflush(stdout);

    // Execute BEFORE_EACH callback if registered
    if (utinc.before_each != NULL) {
        utinc.before_each();
    }
}

void utinc_test_case_fail(const char *file, int line, const char *message) {
    utinc.current_test_cases_failed = true;
    utinc.current_test_suites_failed = true;

    printf("    ┌ %sAssertion failed:%s\n", UTINC_COLOR_RED, UTINC_COLOR_RESET);
    printf("    │ %sLocation:%s %s:%d\n", UTINC_COLOR_GRAY, UTINC_COLOR_RESET, file, line);
    printf("    └ %sMessage:%s %s\n", UTINC_COLOR_GRAY, UTINC_COLOR_RESET, message);
}

bool utinc_test_case_should_continue(void) {
    return !utinc.current_test_cases_failed;
}

void utinc_test_case_end(void) {
    // Execute AFTER_EACH callback if registered
    if (utinc.after_each != NULL) {
        utinc.after_each();
    }

    printf("%s", UTINC_COLOR_RESET);
    if (utinc.current_test_cases_failed) {
        utinc.test_cases_failed++;
        printf("↳  ❌ %s[FAILED]%s\n", UTINC_COLOR_RED, UTINC_COLOR_RESET);
    } else {
        utinc.test_cases_passed++;
        printf("↳  ✅ %s[PASSED]%s\n", UTINC_COLOR_GREEN, UTINC_COLOR_RESET);
    }
}

// ==================================================
// Callback Registration Functions
// ==================================================

void utinc_register_before_all(void (*callback)(void)) {
    utinc.before_all = callback;
    // Execute immediately if we're in a test suite
    if (callback != NULL) {
        callback();
    }
}

void utinc_register_after_all(void (*callback)(void)) {
    utinc.after_all = callback;
}

void utinc_register_before_each(void (*callback)(void)) {
    utinc.before_each = callback;
}

void utinc_register_after_each(void (*callback)(void)) {
    utinc.after_each = callback;
}
