#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
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

// TODO: handle 404 and other error cases
// TODO: handle icon request
int handle_connection(int srv_fd) {
  Connection client;
  client.addr_len = sizeof(client.addr);

  int valread;
  char rbuff[BUFFER_SIZE];
  char wbuff[BUFFER_SIZE];
  Request request;

  if ((client.fd = accept(srv_fd, (struct sockaddr *)&client.addr,
                          (socklen_t *)&client.addr_len)) < 0) {
    perror("Failed to accept new client");
    exit(EXIT_FAILURE);
  }

  printf("\nClient %s:%d Connected\n", inet_ntoa(client.addr.sin_addr),
         ntohs(client.addr.sin_port));

  while (true) {

    valread = read(client.fd, rbuff, BUFFER_SIZE);
    if (valread < 0) {
      perror("Error reading request");
      break;
    }

    if (valread == 0) {
      printf("Client disconnected...\n");
      break;
    }

    printf("\nClient Request:\n%s\n", rbuff);

    processRequestHeader(&request, rbuff);

    FILE *file;
    char *file_contents = NULL;
    size_t file_size = 0;

    file = fopen(request.uri.path, "rb");

    if (file == NULL) {
      perror("Error opening file");
      break;
    }

    while (fgets(wbuff, BUFFER_SIZE, file) != NULL) {
      size_t buff_size = strlen(wbuff);
      char *new_contents = realloc(file_contents, file_size + buff_size + 1);

      if (new_contents == NULL) {
        perror("Error allocating memory");
        fclose(file);
        if (file_contents != NULL)
          free(file_contents);
        break;
      }

      file_contents = new_contents;
      memcpy(file_contents + file_size, wbuff, buff_size);
      file_size += buff_size;
      file_contents[file_size] = '\0';
    }

    printf("%s\n", request.uri.path);

    fclose(file);

    char *pos_resp;
    char *img_resp = "HTTP/1.1 200 OK\nContent-Type: "
                     "image/png\r\n\r\n";

    char *txt_resp = "HTTP/1.1 200 OK\nContent-Type: "
                     "text/html\nContent-Length:";

    if (strstr(request.uri.path, "png")) {
      pos_resp = img_resp;
    } else {
      pos_resp = txt_resp;
    }

    char *response = malloc(strlen(pos_resp) + file_size + 10);

    memcpy(response, pos_resp, strlen(pos_resp));
    sprintf(response + strlen(pos_resp), " %ld\n\n", file_size);
    memcpy(response + strlen(response), file_contents, file_size);
    printf("%s\n", response);

    free(file_contents);

    send(client.fd, response, strlen(response), 0);

    free(response);
    free(request.uri.path);

    if (strstr(rbuff, "Connection: keep-alive") == NULL) {
      printf("Closing conection...\n");
      break;
    }
  }
  close(client.fd);
  return 0;
}

int main(int argc, char *argv[]) {

  char root[255];

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--root")) {
      if (i + 1 < argc) {
        strcpy(root, argv[i + 1]);
        i++;
      } else
        printf("Error: Missing value for root directory");
    } else
      printf("Unknown argument: %s\n", argv[i]);
  }

  chdir(root);

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
