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
  URI uri;
  Protocol protocol;
} HTTP_Request;

void remove_whitespace(char *str);
Request_Method get_request_method(char *string);
Protocol_Type get_protocol(char *string);
void get_protocol_version(char *string, Protocol *protocol);

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
  HTTP_Request request;
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
