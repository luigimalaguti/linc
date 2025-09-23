#include "linc.h"
#include "server.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

struct sink_file {
    const char filename[64];
    FILE *fp;
};

int sink_file_open(void *data) {
    struct sink_file *sink_file = (struct sink_file *)data;
    if (sink_file->fp != NULL) {
        return -1;
    }
    sink_file->fp = fopen(sink_file->filename, "w");
    if (sink_file->fp == NULL) {
        return -1;
    }
    return 0;
}

int sink_file_close(void *data) {
    struct sink_file *sink_file = (struct sink_file *)data;
    if (sink_file->fp == NULL) {
        return -1;
    }
    fclose(sink_file->fp);
    sink_file->fp = NULL;
    return 0;
}

int sink_file_write(void *data, struct linc_metadata *metadata) {
    struct sink_file *sink_file = (struct sink_file *)data;
    if (sink_file->fp == NULL) {
        return -1;
    }
    char log[816];
    int written = linc_stringify_metadata(metadata, log, sizeof(log), false);
    if (written < 0) {
        return -1;
    }
    fwrite(log, sizeof(char), written, sink_file->fp);
    return 0;
}

int sink_file_flush(void *data) {
    struct sink_file *sink_file = (struct sink_file *)data;
    if (sink_file->fp == NULL) {
        return -1;
    }
    fflush(sink_file->fp);
    return 0;
}

struct sink_file sink_file_data = {
    .filename = "server_log.log",
    .fp = NULL,
};

struct linc_sink_funcs funcs = {
    .data = &sink_file_data,
    .open = sink_file_open,
    .close = sink_file_close,
    .write = sink_file_write,
    .flush = sink_file_flush,
};

int main(void) {
    linc_register_sink("file", LINC_LEVEL_TRACE, true, funcs);
    linc_set_sink_level(linc_default_sink, LINC_LEVEL_WARN);
    linc_set_module_level(linc_default_module, LINC_LEVEL_TRACE);

    server_socket_t server_socket;
    server_address_t server_address;
    server_opts_t server_opts;

    server_socket.domain = PF_INET;
    server_socket.type = SOCK_STREAM;
    server_socket.protocol = 0;

    server_address.family = AF_INET;
    server_address.address = INADDR_ANY;
    server_address.port = 3000;

    server_opts.reusable = 1;
    server_opts.backlog = 128;

    if (server_init(&server, &server_socket, &server_address, &server_opts) < 0) {
        FATAL("Failed to initialize server.");
        server_stop(&server);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, server_signal_handler);  // Ctrl+C
    signal(SIGTERM, server_signal_handler);

    if (server_start(&server) < 0) {
        FATAL("Failed to start server.");
        server_stop(&server);
        exit(EXIT_FAILURE);
    }

    server_stop(&server);

    return 0;
}
