#include "server.h"

#include "linc.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

server_t server;

void *handle_request(void *arg) {
    client_t *client = (client_t *)arg;
    int socket = client->socket_fd;

    char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 76\r\n"
        "\r\n"
        "<html><body><h1>Hello, world!</h1><p>This is a web "
        "server.</p></body></html>";

    int request_size = 1024;
    char request[request_size];
    int bytes_received = recv(socket, request, request_size - 1, 0);
    if (bytes_received < 0) {
        ERROR("Failed to receive request");
        return NULL;
    }
    request[bytes_received] = '\0';
    INFO("Received request:\n%s", request);

    int bytes_sent = send(socket, response, strlen(response), 0);
    if (bytes_sent < 0) {
        ERROR("Failed to send response");
        return NULL;
    }

    TRACE("Client connection handled, closing socket...");
    close(socket);
    return NULL;
}

int server_init(server_t *server,
                server_socket_t *socket_config,
                server_address_t *address_config,
                server_opts_t *opts_config) {
    if (server == NULL) {
        ERROR("Invalid server or server configuration provided.");
        exit(EXIT_FAILURE);
    }

    server->running = 0;
    server->socket_fd = -1;

    server->address_in.sin_family = address_config->family;
    server->address_in.sin_addr.s_addr = htonl(address_config->address);
    server->address_in.sin_port = htons(address_config->port);
    memset(server->address_in.sin_zero, '\0', sizeof server->address_in.sin_zero);

    if ((server->socket_fd = socket(socket_config->domain, socket_config->type, socket_config->protocol)) < 0) {
        ERROR("Failed to create socket: %s", strerror(errno));
        return -1;
    }

    if (setsockopt(server->socket_fd, SOL_SOCKET, SO_REUSEADDR, &opts_config->reusable, sizeof(int)) < 0) {
        close(server->socket_fd);
        server->socket_fd = -1;
        ERROR("Failed to set socket options: %s", strerror(errno));
        return -1;
    }

    if (bind(server->socket_fd, (struct sockaddr *)&server->address_in, sizeof server->address_in) < 0) {
        close(server->socket_fd);
        server->socket_fd = -1;
        ERROR("Failed to bind socket: %s", strerror(errno));
        return -1;
    }

    if (listen(server->socket_fd, opts_config->backlog) < 0) {
        close(server->socket_fd);
        server->socket_fd = -1;
        ERROR("Failed to listen on socket: %s", strerror(errno));
        return -1;
    }

    char address_string[INET_ADDRSTRLEN];
    if (inet_ntop(server->address_in.sin_family, &server->address_in.sin_addr, address_string, sizeof address_string)
        == NULL) {
        close(server->socket_fd);
        server->socket_fd = -1;
        ERROR("Failed to convert address to string: %s", strerror(errno));
        return -1;
    }
    INFO("Server listening on %s:%d with backlog %d", address_string, address_config->port, opts_config->backlog);

    return 0;
}

int server_start(server_t *server) {
    if (server == NULL) {
        ERROR("Server instance is not initialized.");
        return -1;
    }
    if (server->socket_fd < 0) {
        ERROR("Server socket is not initialized.");
        return -1;
    }

    server->running = 1;
    INFO("Server started, entering main loop");

    while (server->running) {
        TRACE("Waiting for client connection...");

        client_t client;
        struct sockaddr *client_address_in = (struct sockaddr *)&client.address_in;
        socklen_t client_address_len = sizeof(client.address_in);
        memset(client.address_in.sin_zero, '\0', sizeof(client.address_in.sin_zero));

        client.socket_fd = accept(server->socket_fd, client_address_in, &client_address_len);
        if (client.socket_fd < 0) {
            if (server->running) {
                ERROR("Failed to accept client connection: %s", strerror(errno));
                continue;
            }
            INFO("Server is stopping, no longer accepting connections.");
            break;
        }

        INFO(
            "Accepted connection from %s:%d", inet_ntoa(client.address_in.sin_addr), ntohs(client.address_in.sin_port));

        TRACE("Handling client connection...");
        handle_request(&client);
    }
    INFO("Server stopped.");

    return 0;
}

int server_stop(server_t *server) {
    if (server == NULL) {
        return -1;
    }

    server->running = 0;
    if (server->socket_fd >= 0) {
        close(server->socket_fd);
        server->socket_fd = -1;
    }

    return 0;
}

void server_signal_handler(int signal) {
    (void)signal;
    TRACE(NULL);
    INFO("Signal handler called with signal: %d", signal);
    INFO("Stopping server due to signal");
    server_stop(&server);
}
