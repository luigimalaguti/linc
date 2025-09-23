#ifndef LINC_EXAMPLES_FILE_SINK_SERVER_H
#define LINC_EXAMPLES_FILE_SINK_SERVER_H

#include <arpa/inet.h>

typedef struct server_socket {
    int domain;    // Address family (e.g., PF_INET)
    int type;      // Socket type (e.g., SOCK_STREAM)
    int protocol;  // Protocol (e.g., 0 for default)
} server_socket_t;

typedef struct server_address {
    int family;   // Address family (e.g., AF_INET)
    int address;  // IP address in
    int port;     // Port number
} server_address_t;

typedef struct server_opts {
    int reusable;  // Allow reuse of the address
    int backlog;   // Maximum length of the queue of pending connections
} server_opts_t;

typedef struct server {
    int socket_fd;
    struct sockaddr_in address_in;
    int running;
} server_t;

extern server_t server;

typedef struct client {
    int socket_fd;
    struct sockaddr_in address_in;
} client_t;

int server_init(server_t *server,
                server_socket_t *socket_config,
                server_address_t *address_config,
                server_opts_t *opts_config);
int server_start(server_t *server);
int server_stop(server_t *server);
void server_signal_handler(int signal);

#endif  // LINC_EXAMPLES_FILE_SINK_SERVER_H
