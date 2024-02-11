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

  printf("Listening at on port %d\n", PORT);

  while (true) {

    int client;
    if ((client = accept(sockfd, (struct sockaddr *)&address,
                         (socklen_t *)&addresslen)) < 0) {
      perror("Failed to accept new client");
      exit(EXIT_FAILURE);
    }

    write(client, "Hello World!\n", 18);
    close(client);
  }

  printf("Closing Server\n");
  exit(EXIT_SUCCESS);
}
