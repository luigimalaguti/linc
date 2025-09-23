#include "http_sink.h"
#include "linc.h"

#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define THREAD_LOGS 10

struct sink_network sink_network_data = {
    .sockfd = -1,
    .host = "0.0.0.0",
    .port = "3000",
    .path = "/api/logs",
    .keep_alive = 1,
};

struct linc_sink_funcs funcs = {
    .data = &sink_network_data,
    .open = sink_network_open,
    .close = sink_network_close,
    .write = sink_network_write,
    .flush = sink_network_flush,
};

linc_module app_module;
linc_module db_module;
linc_module service_module;

void *app_thread(void *arg) {
    (void)arg;

    for (int i = 0; i < THREAD_LOGS; i++) {
        int level = rand() % 5;
        linc_log(app_module, level, __FILE__, __LINE__, __func__, "App log %d", i);
        sleep(1);
    }

    pthread_exit(0);
}

void *db_thread(void *arg) {
    (void)arg;

    for (int i = 0; i < THREAD_LOGS; i++) {
        int level = rand() % 5;
        linc_log(db_module, level, __FILE__, __LINE__, __func__, "DB log %d", i);
        sleep(1);
    }

    pthread_exit(0);
}

void *service_thread(void *arg) {
    (void)arg;

    for (int i = 0; i < THREAD_LOGS; i++) {
        int level = rand() % 5;
        linc_log(service_module, level, __FILE__, __LINE__, __func__, "Service log %d", i);
        sleep(1);
    }

    pthread_exit(0);
}

int main(void) {
    srand(time(NULL));

    linc_register_sink("network", LINC_LEVEL_TRACE, true, funcs);
    linc_set_sink_level(linc_default_sink, LINC_LEVEL_TRACE);
    linc_set_module_enabled(linc_default_module, false);
    app_module = linc_register_module("app", LINC_LEVEL_DEBUG, true);
    db_module = linc_register_module("db", LINC_LEVEL_INFO, true);
    service_module = linc_register_module("service", LINC_LEVEL_WARN, true);

    pthread_t threads[3];

    pthread_create(&threads[0], NULL, app_thread, NULL);
    pthread_create(&threads[0], NULL, db_thread, NULL);
    pthread_create(&threads[0], NULL, service_thread, NULL);

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
