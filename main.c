#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "request.h"
#include "util.h"

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct {
  int fd;
  struct sockaddr_in addr;
  socklen_t addr_len;
} Connection;

typedef struct {
  Connection connection;
  Request request;
} Client;

void process_header(char *buff) {
  Request request;
  char t_buff[strlen(buff)];
  strcpy(t_buff, buff);

  char *line;
  line = strtok(t_buff, "\n");

  char method[20];
  char uri[255];
  char protocol[20];

  sscanf(line, "%s %s %s", method, uri, protocol);

  request.method = get_method(method);
  request.uri.path = uri;
  request.protocol.type = get_protocol(protocol);
  get_protocol_version(protocol, &request.protocol);
}

Connection init_server() {

  Connection server;

  server.addr.sin_family = AF_INET;
  server.addr.sin_addr.s_addr = INADDR_ANY;
  server.addr.sin_port = htons(PORT);
  server.addr_len = sizeof(server.addr);

  int opt = 1;

  server.fd = socket(AF_INET, SOCK_STREAM, 0);
  if (setsockopt(server.fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &opt,
                 sizeof(opt)) < 0) {
    perror("Failed to create socket");
    exit(EXIT_FAILURE);
  }

  if (bind(server.fd, (struct sockaddr *)&server.addr, server.addr_len) < 0) {
    perror("Failed to bin socket to address");
    exit(EXIT_FAILURE);
  }

  if (listen(server.fd, 3) < 0) {
    perror("Socket failed to listen for incoming clients");
    exit(EXIT_FAILURE);
  }

  return server;
}

void handle_connection(int srv_fd) {
  Connection client;
  client.addr_len = sizeof(client.addr);
  // int client;
  struct sockaddr_in cli_addr;
  socklen_t addr_len;

  int valread;
  char buffer[BUFFER_SIZE];

  if ((client.fd = accept(srv_fd, (struct sockaddr *)&client.addr,
                          (socklen_t *)&client.addr_len)) < 0) {
    perror("Failed to accept new client");
    exit(EXIT_FAILURE);
  }
  printf("\nClient %s:%d Connected\n", inet_ntoa(client.addr.sin_addr),
         ntohs(client.addr.sin_port));

  while (true) {

    valread = read(client.fd, buffer, BUFFER_SIZE);
    if (valread < 0) {
      perror("Error reading request");
      break;
    }

    if (valread == 0) {
      printf("Client disconnected...\n");
      break;
    }

    printf("\nClient Request:\n%s\n", buffer);

    // process_header(buffer);
    char *response = "HTTP/1.0 200 OK\nContent-Type: "
                     "text/plain\nContent-Length: 12\n\nHello World\n";

    send(client.fd, response, strlen(response), 0);

    printf("Sent response...\n");

    if (strstr(buffer, "Connection: keep-alive") == NULL) {
      printf("Closing conection...\n");
      break;
    }
  }
  close(client.fd);
}

int main() {
  char cwd[BUFFER_SIZE];
  getcwd(cwd, sizeof(cwd));

  Connection server = init_server();

  // Print the address the socket is listening on
  getsockname(server.fd, (struct sockaddr *)&server.addr,
              (socklen_t *)&server.addr_len);
  printf("Server listening on %s:%d\n", inet_ntoa(server.addr.sin_addr),
         ntohs(server.addr.sin_port));

  // Working loop
  while (true) {
    handle_connection(server.fd);
  }

  printf("Closing Server\n");
  exit(EXIT_SUCCESS);
}
