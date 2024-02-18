#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
  int sockfd;
  struct sockaddr_in server_addr;
  char *message = "GET / HTTP/10.32\r\nHost: localhost\r\n\r\n";
  char buffer[BUFFER_SIZE] = {0};

  // Create socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  // Specify server address and port
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Connect to server
  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1) {
    perror("connect");
    exit(EXIT_FAILURE);
  }

  // Send request to server
  if (send(sockfd, message, strlen(message), 0) == -1) {
    perror("send");
    exit(EXIT_FAILURE);
  }
  printf("Request sent\n");

  // Receive response from server
  ssize_t bytes_received;
  if ((bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0)) == -1) {
    perror("recv");
    exit(EXIT_FAILURE);
  }
  buffer[bytes_received] = '\0';
  printf("Response received:\n%s\n", buffer);

  // Close socket
  close(sockfd);

  return 0;
}
