#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

void get_request_type(char **string) {
  char *pos = strstr(*string, "/");
  int offset = pos - *string;

  char request_type[offset + 1];
  strncpy(request_type, *string, offset);
  request_type[offset] = '\0';

  printf("%s\n", request_type);

  *string = ++pos;
}

void get_protocol(char **string) {
  char *pos = strstr(*string, "/");
  int offset = pos - *string;

  char protocol[offset + 1];
  strncpy(protocol, *string, offset);
  protocol[offset] = '\0';

  printf("%s\n", protocol);

  *string = ++pos;
}

void get_protocol_version(char **string) {
  char minor_version[10];

  char *pos = strstr(*string, ".");
  int offset = pos - *string;
  char major_version[offset + 1];

  strncpy(major_version, *string, offset);
  major_version[offset] = '\0';
  strcpy(minor_version, *string + offset + 1);

  printf("%s\n", major_version);
  printf("%s\n", minor_version);

  *string = ++pos;
}

void process_header(char *buff) {
  char t_buff[strlen(buff)];
  strcpy(t_buff, buff);

  char *line;
  line = strtok(t_buff, "\n");

  char **pos = &line;

  get_request_type(pos);
  get_protocol(pos);
  get_protocol_version(pos);
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
