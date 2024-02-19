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

typedef enum {
  REQUEST,
} Header;

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
  Protocol protocol;
} HTTP_Request;

void remove_whitespace(char *str);
Request_Method get_request_method(char **string);
Protocol get_protocol(char **string);
void get_protocol_version(char **string, Protocol *protocol);

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

Request_Method get_request_method(char **string) {
  char *pos = strstr(*string, " ");
  int offset = pos - *string;

  char method[offset + 1];
  strncpy(method, *string, offset);
  method[offset] = '\0';

  *string = ++pos;

  if (strcmp(method, "GET") == 0) {
    return GET;
  } else if (strcmp(method, "SET") == 0)
    return SET;
  else
    return -1;
}

Protocol get_protocol(char **string) {
  Protocol protocol;
  char *pos = strstr(*string, "/");
  int offset = pos - *string;

  char protocol_type[offset + 1];
  strncpy(protocol_type, *string, offset);
  protocol_type[offset] = '\0';
  remove_whitespace(protocol_type);

  if (strcmp(protocol_type, "HTTP"))
    protocol.type = HTTP;
  else
    protocol.type = -1;

  *string = ++pos;

  get_protocol_version(string, &protocol);
  return protocol;
}

void get_protocol_version(char **string, Protocol *protocol) {
  char *pos = strstr(*string, ".");
  int offset = pos - *string;
  char major_version[offset + 1];

  strncpy(major_version, *string, offset);
  major_version[offset] = '\0';
  protocol->major_version = atoi(major_version);

  *string = ++pos;

  int chars_left = strlen(*string);

  char minor_version[chars_left];
  strcpy(minor_version, *string + offset + 1);
  protocol->minor_version = atoi(minor_version);

  *string = pos + chars_left;
}

void process_header(char *buff) {
  HTTP_Request request;
  char t_buff[strlen(buff)];
  strcpy(t_buff, buff);

  char *line;
  line = strtok(t_buff, "\n");

  char **pos = &line;

  request.method = get_request_method(pos);
  request.protocol = get_protocol(pos);
}

int main() {
  int server_fd, new_socket;
  struct sockaddr_in address;
  int addresslen = sizeof(address);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  int opt = 1;

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &opt,
                 sizeof(opt)) < 0) {
    perror("Failed to create socket");
    exit(EXIT_FAILURE);
  }

  if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Failed to bin socket to address");
    exit(EXIT_FAILURE);
  }

  if (listen(sockfd, 3) < 0) {
    perror("Socket failed to listen for incoming clients");
    exit(EXIT_FAILURE);
  }

  printf("Listening at at IP: %d, Port: %d\n", address.sin_addr.s_addr, PORT);

  while (true) {

    int client;
    if ((client = accept(sockfd, (struct sockaddr *)&address,
                         (socklen_t *)&addresslen)) < 0) {
      perror("Failed to accept new client");
      exit(EXIT_FAILURE);
    }

    char buff[255];
    recv(client, buff, 255, 0);

    process_header(buff);

    write(client, "Hello World!\n", 18);
    close(client);
  }

  printf("Closing Server\n");
  exit(EXIT_SUCCESS);
}
