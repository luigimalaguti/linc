#include "http_sink.h"

#include <inttypes.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int sink_network_open(void *data) {
    struct sink_network *sink_network = data;
    struct addrinfo hints, *res, *rp;
    int sockfd = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(sink_network->host, sink_network->port, &hints, &res) != 0) {
        return -1;
    }

    for (rp = res; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1) {
            continue;
        }

        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            sink_network->sockfd = sockfd;
            sink_network->is_connected = 1;
            freeaddrinfo(res);
            return 0;
        }

        close(sockfd);
    }

    freeaddrinfo(res);
    return -1;
}

int sink_network_close(void *data) {
    struct sink_network *sink_network = data;
    if (sink_network->is_connected) {
        close(sink_network->sockfd);
        sink_network->is_connected = 0;
    }
    return 0;
}

int sink_network_write(void *data, struct linc_metadata *metadata) {
    struct sink_network *sink_network = data;

    if (!sink_network->is_connected) {
        if (sink_network_open(sink_network) != 0) {
            return -1;
        }
    }

    char timestamp_string[24];
    linc_timestamp_string(metadata->timestamp, timestamp_string, sizeof(timestamp_string));

    char log[816];
    int log_size = snprintf(log,
                            sizeof(log),
                            "{"
                            "\"timestamp\": \"%s\","
                            "\"level\": \"%s\","
                            "\"thread_id\": \"%" PRIxPTR
                            "\","
                            "\"module_name\": \"%s\","
                            "\"filename\": \"%s\","
                            "\"line\": \"%" PRIu32
                            "\","
                            "\"func\": \"%s\","
                            "\"message\": \"%s\""
                            "}",
                            timestamp_string,
                            linc_level_string(metadata->level),
                            metadata->thread_id,
                            metadata->module_name,
                            metadata->filename,
                            metadata->line,
                            metadata->func,
                            metadata->message);

    int written = snprintf(sink_network->request,
                           sizeof(sink_network->request),
                           "POST %s HTTP/1.1\r\n"
                           "Host: %s\r\n"
                           "Connection: %s\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: %d\r\n"
                           "\r\n"
                           "%s",
                           sink_network->path,
                           sink_network->host,
                           sink_network->keep_alive ? "keep-alive" : "close",
                           log_size,
                           log);

    if (send(sink_network->sockfd, sink_network->request, written, 0) < 0) {
        close(sink_network->sockfd);
        sink_network->is_connected = 0;
        return -1;
    }

    char resp[512];
    int r = recv(sink_network->sockfd, resp, sizeof(resp) - 1, 0);
    if (r <= 0) {
        close(sink_network->sockfd);
        sink_network->is_connected = 0;
        return -1;
    }
    resp[r] = '\0';

    return 0;
}

int sink_network_flush(void *data) {
    (void)data;
    return 0;
}
