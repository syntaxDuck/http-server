#include "request.h"
#include "util.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int header_parse_method(RequestMethod *method, char *method_str)
{
  if (method_str == NULL)
  {
    fprintf(stderr, "Error: NULL pointer passed as methodString\n");
    return INVALID_REQUEST_HEADER;
  }

  str_to_upper(method_str);

  if (strcmp(method_str, "GET") == 0)
  {
    *method = REQUEST_GET;
  }
  else if (strcmp(method_str, "SET") == 0)
  {
    *method = REQUEST_SET;
  }
  else
  {
    return INVALID_REQUEST_HEADER;
  }

  return 0;
}

int header_parse_protocol(Protocol *protocol, char *protocol_str)
{
  if (header_parse_protocol_type(protocol, protocol_str) < 0)
  {
    return INVALID_REQUEST_HEADER;
  }
  if (header_parse_protocol_version(protocol, protocol_str) < 0)
  {
    return INVALID_REQUEST_HEADER;
  }
  return 0;
}

int header_parse_protocol_type(Protocol *protocol, char *protocol_str)
{
  if (protocol_str == NULL)
  {
    fprintf(stderr, "Error: NULL pointer passed as protocolString\n");
    return INVALID_REQUEST_HEADER;
  }

  str_to_upper(protocol_str);

  char *slash_pos = strstr(protocol_str, "/");
  if (slash_pos == NULL)
  {
    fprintf(stderr, "Error: Invalid protocol string\n");
    return INVALID_REQUEST_HEADER;
  }
  int offset = slash_pos - protocol_str;
  char *type = malloc(offset + 1);
  if (type == NULL)
  {
    fprintf(stderr, "Error: unable to allocate memory\n");
    return INVALID_REQUEST_HEADER;
  }

  strncpy(type, protocol_str, offset);
  type[offset] = '\0';

  ProtocolType result;
  if (strcmp(type, "HTTP") == 0)
  {
    protocol->type = PROTOCOL_HTTP;
  }
  else
  {
    return INVALID_REQUEST_HEADER;
  }

  free(type);
  return 0;
}

int header_parse_protocol_version(Protocol *protocol, char *version_str)
{
  if (version_str == NULL)
  {
    fprintf(stderr, "Error: NULL pointer passed as versionString\n");
    return INVALID_REQUEST_HEADER;
  }

  while (*version_str && !isdigit(*version_str))
    version_str++;

  int major_version = INVALID_VERSION_NUMBER, minor_version = INVALID_VERSION_NUMBER;
  if (sscanf(version_str, "%d.%d", &major_version, &minor_version) != 2)
  {
    fprintf(stderr, "Error: Invalid protocol version format\n");
    return INVALID_REQUEST_HEADER;
  }

  protocol->major_version = major_version;
  if (protocol->major_version < 0)
  {
    fprintf(stderr, "Error: Invalid protocol major version");
    return INVALID_REQUEST_HEADER;
  }

  protocol->minor_version = minor_version;
  if (protocol->minor_version < 0)
  {
    fprintf(stderr, "Error: Invalid protocol minor version");
    return INVALID_REQUEST_HEADER;
  }
  return 0;
}

int process_request(int client_fd, Request *request)
{
  char read_buff[BUFFER_SIZE];
  if (read_request(client_fd, read_buff) < 0)
  {
    return -1;
  }

  printf("\nClient Request:\n%s\n", read_buff);
  if (parse_request(request, read_buff) < 0)
  {
    return -1;
  }

  return 0;
}

int parse_request(Request *request, char *buff)
{

  if (buff == NULL)
  {
    fprintf(stderr, "Error: NULL pointer passed as buff");
    return INVALID_REQUEST_HEADER;
  }

  if (strlen(buff) == 0)
  {
    fprintf(stderr, "Error: Empty string passes as buff");
    return INVALID_REQUEST_HEADER;
  }

  char method[MAX_METHOD_LENGTH];
  char uri[MAX_URI_LENGTH];
  char protocol[MAX_PROTOCOL_LENGTH];
  char buffer_cpy[BUFFER_SIZE];

  strncpy(buffer_cpy, buff, sizeof(buffer_cpy));

  // if (getcwd(uri, MAX_URI_LENGTH) == NULL) {
  //   fprintf(stderr, "Error: Unable to get cwd");
  //   return INVALID_REQUEST_HEADER;
  // }

  char *line = strtok(buffer_cpy, "\n");

  if (sscanf(line, "%s %s %s", method, uri, protocol) != 3)
  {
    fprintf(stderr, "Error: Invalid request header format\n");
    return INVALID_REQUEST_HEADER;
  }

  if (header_parse_method(&request->method, method) < 0)
  {
    return INVALID_REQUEST_HEADER;
  }

  if (header_parse_protocol(&request->protocol, protocol) < 0)
  {
    return INVALID_REQUEST_HEADER;
  }

  request->uri.path = strdup(uri + 1);

  if (request->uri.path == NULL)
  {
    fprintf(stderr, "Error: Unable to allocate memory for request URI\n");
    return INVALID_REQUEST_HEADER;
  }

  return 0;
}

int read_request(int client_fd, char *read_buff)
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