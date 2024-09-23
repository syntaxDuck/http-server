#include "request.h"
#include "response.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

char *load_uri(Request request, long *content_size)
{
  FILE *file;

  file = fopen(request.uri.path, "rb");
  if (file == NULL)
  {
    perror("Error opening file");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  *content_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *contents = malloc(*content_size);
  if (contents == NULL)
  {
    perror("Error allocating memory");
    fclose(file);
    return NULL;
  }

  fread(contents, 1, *content_size, file);
  fclose(file);

  return contents;
}

char *build_header(Request request, char *content_type, long content_size)
{
  // Prepare and send HTTP response
  char *response_header = malloc(BUFFER_SIZE);
  if (response_header == NULL)
  {
    perror("Error allocating memory");
    return NULL;
  }
  snprintf(response_header, BUFFER_SIZE,
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: %s\r\n"
           "Content-Length: %ld\r\n\r\n",
           content_type,
           content_size);

  printf("%s\n", response_header);
  return response_header;
}

int process_response(int client_fd, Request request)
{
  long content_size;
  char *contents = load_uri(request, &content_size);
  printf("%s\n", request.uri.path);

  // Add 2 to the content size to accommodate the CRLF
  char *header = build_header(request, "", content_size + 2);
  free(request.uri.path);

  send(client_fd, header, strlen(header), 0);
  send(client_fd, contents, content_size, 0);
  send(client_fd, CRLF, content_size, 0);
  free(contents);
  free(header);
  return 0;
}