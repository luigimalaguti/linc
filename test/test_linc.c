#include "linc.h"

#include <assert.h>

int main(void) {
    INFO("A simple log message.");
    DEBUG(NULL);
    WARN("By LINC, %s %s %s!", "Logger", "In", "C");

    return 0;
}
