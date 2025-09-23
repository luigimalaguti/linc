#ifndef LINC_EXAMPLES_NETWORK_SINK_HTTP_SINK_H
#define LINC_EXAMPLES_NETWORK_SINK_HTTP_SINK_H

#include "linc.h"

struct sink_network {
    int sockfd;
    const char *host;
    const char *port;
    const char *path;
    int keep_alive;
    int is_connected;
    char request[1024];
};

int sink_network_open(void *data);
int sink_network_close(void *data);
int sink_network_write(void *data, struct linc_metadata *metadata);
int sink_network_flush(void *data);

#endif  // LINC_EXAMPLES_NETWORK_SINK_HTTP_SINK_H
