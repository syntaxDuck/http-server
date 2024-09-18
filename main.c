#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>

#include "response.h"
#include "request.h"
#include "util.h"

#define PORT 8080
#define BUFFER_SIZE 1024

#define POSITIVE_RESP 200
#define RESOURCE_NOT_FOUND 404

int init_server_socket()
{

  struct sockaddr_in socket_address;
  socklen_t address_length;

  socket_address.sin_family = AF_INET;
  socket_address.sin_addr.s_addr = INADDR_ANY;
  socket_address.sin_port = htons(PORT);
  address_length = sizeof(socket_address);

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

  if (bind(socket_fd, (struct sockaddr *)&socket_address, address_length) < 0)
  {
    perror("Failed to bind socket to address");
    exit(EXIT_FAILURE);
  }

  return socket_fd;
}

// TODO: handle 404 and other error cases
// TODO: handle icon request
int socket_handle_connection(int server_fd)
{
  int client_fd;
  struct sockaddr_in client_socket_addr;
  socklen_t addr_len = sizeof(client_socket_addr);

  Request request;

  if ((client_fd = accept(server_fd, (struct sockaddr *)&client_socket_addr,
                          (socklen_t *)&addr_len)) < 0)
  {
    perror("Failed to accept new client");
    exit(EXIT_FAILURE);
  }

  printf("\nClient %s:%d Connected\n", inet_ntoa(client_socket_addr.sin_addr),
         ntohs(client_socket_addr.sin_port));

  while (true)
  {

    process_request(client_fd, &request);
    process_response(client_fd, request);

    // if (strstr(read_buff, "Connection: keep-alive") == NULL)
    // {
    //   printf("Closing conection...\n");
    //   break;
    // }
    break;
  }
  close(client_fd);
  return 0;
}

int parse_args(int argc, char *argv[])
{
  char root[255];
  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--root") == 0) // Check if the argument is "--root"
    {
      if (i + 1 < argc) // Check if there's another argument after "--root"
      {
        strcpy(root, argv[i + 1]); // Copy the next argument to 'root'
        i++;                       // Skip the next argument (since it's the root path)
      }
      else
      {
        printf("Error: Missing value for root directory\n");
        exit(EXIT_FAILURE); // Exit with an error code if no value for root
      }
    }
    else
    {
      printf("Unknown argument: %s\n", argv[i]);
    }
  }

  chdir(root); // Change to the specified root directory
  return 0;
}

int main(int argc, char *argv[])
{
  int socket_fd = init_server_socket();
  struct sockaddr_in socket_addr;
  socklen_t addr_len = sizeof(socket_addr);

  if (listen(socket_fd, 3) < 0)
  {
    perror("Socket failed to listen for incoming clients");
    exit(EXIT_FAILURE);
  }

  // Print the address the socket is listening on
  getsockname(socket_fd, (struct sockaddr *)&socket_addr, &addr_len);
  printf("Server listening on %s:%d\n", inet_ntoa(socket_addr.sin_addr),
         ntohs(socket_addr.sin_port));

  // Working loop
  while (true)
  {
    socket_handle_connection(socket_fd);
  }

  printf("Closing Server\n");
  exit(EXIT_SUCCESS);
}
