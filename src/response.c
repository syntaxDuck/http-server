#include "request.h"
#include "response.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

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

  // if (strstr(request.uri.path, "png")) {
  //   load_png(file, contents);
  // } else if (strstr())

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
  char *img_resp = "HTTP/1.1 200 OK\r\nContent-Type: "
                   "image/png\r\n\r\n";

  char *txt_resp = "HTTP/1.1 200 OK\r\nContent-Type: "
                   "text/html\r\nContent-Length:";

  if (strstr(request.uri.path, "png"))
  {
    pos_resp = img_resp;
  }
  else
  {
    pos_resp = txt_resp;
  }

  long content_len = strlen(content);

  char *response = malloc(strlen(pos_resp) + content_len + 100);

  memcpy(response, pos_resp, strlen(pos_resp));
  sprintf(response + strlen(pos_resp), " %ld\r\n\r\n", content_len + 2);
  memcpy(response + strlen(response), content, content_len);
  strcpy(response + strlen(response), "\r\n");

  return response;
}

int process_response(int client_fd, Request request)
{
  char *content = load_uri(request);
  printf("%s\n", request.uri.path);
  char *response = build_response(request, content);
  free(request.uri.path);
  free(content);

  printf("%s\n", response);

  send(client_fd, response, strlen(response), 0);
  free(response);
  return 0;
}