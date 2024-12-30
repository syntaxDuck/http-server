#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

#include "progargs.h"
#include "response.h"
#include "request.h"
#include "util.h"

#define DEFAULT_PORT 8080
#define MAX_ROOT_LENGTH 255
#define MAX_PORT_LENGTH 10

int server_fd;

int init_server_socket(int port)
{
    struct sockaddr_in socket_address;
    socklen_t address_length = sizeof(socket_address);
    int opt = 1;

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Set SO_REUSEADDR option
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("Failed to set SO_REUSEADDR");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    // Set SO_REUSEPORT option
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        perror("Failed to set SO_REUSEPORT");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = INADDR_ANY;
    socket_address.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *)&socket_address, address_length) < 0)
    {
        perror("Failed to bind socket to address");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    return socket_fd;
}

void handle_shutdown(int sig)
{
    printf("\nShutting down the server...\n");
    close(server_fd);
    exit(EXIT_SUCCESS);
}

int socket_handle_connection(int client_fd)
{
    Request request;

    process_request(client_fd, &request);
    process_valueonse(client_fd, request);

    close(client_fd);
    return 0;
}

int parse_args(int argc, char *argv[])
{
    char root[MAX_ROOT_LENGTH] = ".";
    int port = DEFAULT_PORT;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--root") == 0 || strcmp(argv[i], "-r") == 0)
        {
            if (i + 1 < argc)
            {
                strncpy(root, argv[i + 1], MAX_ROOT_LENGTH - 1); // Safe string copy
                root[MAX_ROOT_LENGTH - 1] = '\0';
                i++;
            }
            else
            {
                fprintf(stderr, "Error: Missing value for root directory\n");
                exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0)
        {
            if (i + 1 < argc)
            {
                port = atoi(argv[i + 1]);
                i++;
            }
            else
            {
                fprintf(stderr, "Error: Missing value for for port\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
        }
    }

    if (chdir(root) != 0)
    {
        perror("Failed to change directory");
        exit(EXIT_FAILURE);
    }

    return port;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle_shutdown);

    const ArgumentDefinition defs[] = {
        {"port", true},
        {"root", true}};

    const ArgumentDefinitions prog_args = {defs, 1};

    ParsedArguments *parsed_args = handle_arguments(argc, argv, prog_args);

    const int port = atoi(get_argument_value(parsed_args, "port"));

    server_fd = init_server_socket(port);

    if (listen(server_fd, 3) < 0)
    {
        perror("Socket failed to listen for incoming clients");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    while (true)
    {
        struct sockaddr_in client_socket_addr;
        socklen_t addr_len = sizeof(client_socket_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_socket_addr, &addr_len);
        if (client_fd < 0)
        {
            perror("Failed to accept new client");
            continue;
        }
        printf("Client connected: %s:%d\n", inet_ntoa(client_socket_addr.sin_addr), ntohs(client_socket_addr.sin_port));
        socket_handle_connection(client_fd);
    }

    printf("Closing Server\n");
    exit(EXIT_SUCCESS);
}
