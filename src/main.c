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

#define PORT 8080
#define BUFFER_SIZE 1024

typedef enum {
  GET,
  SET,
} Request_Method;

typedef struct {
  char *path;
} URI;

typedef enum {
  HTTP,
} Protocol_Type;

typedef struct {
  Protocol_Type type;
  int major_version;
  int minor_version;
} Protocol;

typedef struct {
  Request_Method method;
  URI uri;
  Protocol protocol;
} Request;

typedef struct {
  int fd;
  struct sockaddr_in addr;
  socklen_t addr_len;
} Connection;

typedef struct {
  Connection connection;
  Request request;
} Client;

void remove_whitespace(char *str);
Request_Method get_request_method(char *string);
Protocol_Type get_protocol(char *string);
void get_protocol_version(char *string, Protocol *protocol);
void get_resource(char *uri);

void remove_whitespace(char *str) {
  int len = strlen(str);
  char new_str[len];

  int j = 0;
  for (int i = 0; i < len; i++) {
    if (!isspace(str[i])) {
      new_str[j++] = str[i];
      str[i] = '\0';
    }
  }

  strcpy(str, new_str);
}

Request_Method get_request_method(char *string) {
  if (strcmp(string, "GET") == 0) {
    return GET;
  } else if (strcmp(string, "SET") == 0)
    return SET;
  else
    return -1;
}

Protocol_Type get_protocol(char *string) {
  char *pos = strstr(string, "/");
  int offset = pos - string;

  char protocol[offset + 1];
  strncpy(protocol, string, offset);
  protocol[offset] = '\0';

  if (strcmp(protocol, "HTTP") == 0)
    return HTTP;
  else
    return -1;
}

void get_protocol_version(char *string, Protocol *protocol) {
  char *pos1 = strstr(string, "/");
  char *pos2 = strstr(string, ".");
  int offset = pos2 - pos1 - 1;
  char major_version[offset + 1];

  strncpy(major_version, ++pos1, offset);
  major_version[offset] = '\0';
  protocol->major_version = atoi(major_version);

  string = pos2++;

  int chars_left = strlen(pos2);

  char minor_version[chars_left];
  major_version[chars_left - 1] = '\0';
  strcpy(minor_version, pos2);
  protocol->minor_version = atoi(minor_version);
}

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

  request.method = get_request_method(method);
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

int main() {
  char cwd[BUFFER_SIZE];
  getcwd(cwd, sizeof(cwd));

  Connection server = init_server();

  // Print the address the socket is listening on
  getsockname(server.fd, (struct sockaddr *)&server.addr,
              (socklen_t *)&server.addr_len);
  printf("Server listening on %s:%d\n", inet_ntoa(server.addr.sin_addr),
         ntohs(server.addr.sin_port));

  char buffer[BUFFER_SIZE];
  int valread;

  // Working loop
  while (true) {

    Connection client;
    client.addr_len = sizeof(client.addr);
    // int client;
    struct sockaddr_in cli_addr;
    socklen_t addr_len;

    if ((client.fd = accept(server.fd, (struct sockaddr *)&client.addr,
                            (socklen_t *)&client.addr_len)) < 0) {
      perror("Failed to accept new client");
      exit(EXIT_FAILURE);
    }
    printf("\nClient %s:%d Connected\n", inet_ntoa(client.addr.sin_addr),
           ntohs(client.addr.sin_port));

    // Client loop
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

  printf("Closing Server\n");
  exit(EXIT_SUCCESS);
}
