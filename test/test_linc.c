#include "linc.h"
#include "utinc.h"

const char *title =
    "LINC Main Test File\n\n"
    "By UTINC, Unit Testing In C";

DEFINE_CALLBACK(lowest_level, { linc_set_level(LINC_LEVEL_TRACE); })
DEFINE_CALLBACK(restore_level, { linc_set_level(LINC_LEVEL_INFO); })

TEST_RUNNER(title, {
    BEFORE_ALL(lowest_level);
    AFTER_ALL(restore_level);

    TEST_SUITE("Simple showcase", {
        TEST_CASE("Show LINC log", {
            TRACE("Hello from LINC!");
            DEBUG("This is LINC, a simple Logger In C.");
            INFO("It supports multiple levels, multiple modules, and multiple outputs.");
            WARN("It is fast and, most importantly, thread-safe!");
            ERROR("Try it out in your own project.");
            FATAL("It is open source and free to use.");
        });
    });
})
