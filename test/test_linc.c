#include "linc.h"
#include "test/utils.h"

#include <assert.h>

int main(void) {
    const char *title_header = "LINC TESTS";
    const char *title_footer = "LINC TESTS DONE";
    const char *description =
        "Simple test, as a main function, to check if LINC works fine.\n"
        "No tests are performed here, just a simple call to a function defined in the\n"
        "LINC library.";
    test_header(title_header, description);

    INFO("Simple log from %s.", __FILE__);

    test_footer(title_footer);
    return 0;
}
