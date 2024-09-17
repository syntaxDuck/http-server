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

int read_client_request(int client_fd, char *read_buff)
{
  int read_status = read(client_fd, read_buff, BUFFER_SIZE);
  if (read_status < 0)
  {
    perror("Error reading request");
    return -1;
  }

  if (read_status == 0)
  {
    printf("Client disconnected...\n");
    return -1;
  }

  return 0;
}

char *load_uri(Request request)
{
  FILE *file;
  char *contents = NULL;
  size_t file_size = 0;
  char write_buff[BUFFER_SIZE];

  file = fopen(request.uri.path, "rb");

  if (file == NULL)
  {
    perror("Error opening file");
    return NULL;
  }

  while (fgets(write_buff, BUFFER_SIZE, file) != NULL)
  {
    size_t buff_size = strlen(write_buff);
    char *block = realloc(contents, file_size + buff_size + 1);

    if (block == NULL)
    {
      perror("Error allocating memory");
      fclose(file);
      if (contents != NULL)
        free(contents);
      break;
    }

    contents = block;
    memcpy(contents + file_size, write_buff, buff_size);
    file_size += buff_size;
    contents[file_size] = '\0';
  }
  fclose(file);

  return contents;
}

char *build_response(Request request, char *content)
{
  // TODO: This needs to be refactored
  char *pos_resp;
  char *img_resp = "HTTP/1.1 200 OK\nContent-Type: "
                   "image/png\r\n\r\n";

  char *txt_resp = "HTTP/1.1 200 OK\nContent-Type: "
                   "text/html\nContent-Length:";

  if (strstr(request.uri.path, "png"))
  {
    pos_resp = img_resp;
  }
  else
  {
    pos_resp = txt_resp;
  }

  long content_len = strlen(content);

  char *response = malloc(strlen(pos_resp) + content_len + 10);

  memcpy(response, pos_resp, strlen(pos_resp));
  sprintf(response + strlen(pos_resp), " %ld\n\n", content_len);
  memcpy(response + strlen(response), content, content_len);

  return response;
}

// TODO: handle 404 and other error cases
// TODO: handle icon request
int socket_handle_connection(int server_fd)
{
  int client_fd;
  struct sockaddr_in client_socket_addr;
  socklen_t addr_len = sizeof(client_socket_addr);

  char read_buff[BUFFER_SIZE];
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

    if (read_client_request(client_fd, read_buff) < 0)
    {
      break;
    }

    printf("\nClient Request:\n%s\n", read_buff);

    process_client_request(&request, read_buff);

    printf("%s\n", request.uri.path);

    char *uri_content = load_uri(request);
    long content_len = strlen(uri_content);

    char *response = build_response(request, uri_content);
    free(uri_content);

    printf("%s\n", response);

    send(client_fd, response, strlen(response), 0);

    free(response);

    if (strstr(read_buff, "Connection: keep-alive") == NULL)
    {
      printf("Closing conection...\n");
      break;
    }
  }
  close(client_fd);
  return 0;
}

int main(int argc, char *argv[])
{

  char root[255];

  for (int i = 0; i < argc; i++)
  {
    if (strcmp(argv[i], "--root"))
    {
      if (i + 1 < argc)
      {
        strcpy(root, argv[i + 1]);
        i++;
      }
      else
        printf("Error: Missing value for root directory");
    }
    else
      printf("Unknown argument: %s\n", argv[i]);
  }

  chdir(root);

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
