#include "request.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int get_method(Request_Method *method, char *string) {
  if (strcmp(string, "GET") == 0) {
    *method = GET;
    return 0;
  } else if (strcmp(string, "SET") == 0) {
    *method = SET;
    return 0;

  } else
    return -1;
}

int get_protocol(Protocol *protocol, char *string) {
  char *pos = strstr(string, "/");
  int offset = pos - string;

  char pt[offset + 1];
  strncpy(pt, string, offset);
  pt[offset] = '\0';

  if (strcmp(pt, "HTTP") == 0) {
    protocol->type = HTTP;
    return 0;
  } else
    return -1;
}

int get_protocol_version(Protocol *protocol, char *string) {
  char *pos1 = strstr(string, "/");
  char *pos2 = strstr(string, ".");
  int offset = pos2 - pos1 - 1;
  char major_version[offset + 1];

  if (strncpy(major_version, ++pos1, offset) < 0) {
    perror("Error copying string\n");
    return -1;
  }
  major_version[offset] = '\0';
  protocol->major_version = atoi(major_version);
  if (protocol->major_version < 0) {
    perror("Error invalid protocol major version\n");
    return -1;
  }

  string = pos2++;

  int chars_left = strlen(pos2);

  char minor_version[chars_left];
  major_version[chars_left - 1] = '\0';
  if (strcpy(minor_version, pos2) < 0) {
    perror("Error copying string\n");
    return -1;
  }
  protocol->minor_version = atoi(minor_version);
  if (protocol->minor_version < 0) {
    perror("Error invalid protocol minor version\n");
    return -1;
  }
  return 0;
}

int process_header(Request *request, char *buff) {
  Request req;
  char t_buff[strlen(buff)];
  strcpy(t_buff, buff);

  char *line;
  line = strtok(t_buff, "\n");

  char method[20];
  char uri[255];
  char protocol[20];

  sscanf(line, "%s %s %s", method, uri, protocol);

  if (get_method(&req.method, method) != 0) {
    return -1;
  }
  if (get_protocol(&req.protocol, protocol) != 0) {
    return -1;
  }
  if (get_protocol_version(&req.protocol, protocol) != 0) {
    return -1;
  }

  int uri_len = strlen(uri);
  // Remove back slashes for forward slashes
  for (int i = 0; i < uri_len; i++) {
    if (uri[i] == '\\')
      uri[i] = '/';
  }

  // ensure the uri isnt attempting to go to request
  // resources outside of the servers space
  if (strstr(uri, "..")) {
    perror("Error invalide uri requested");
    return -1;
  }

  memcpy(uri + 1, uri, strlen(uri));
  uri[0] = '.';

  // check if the resource exists
  if (access(uri, F_OK) != 0) {
    perror("Error requested resource not found");
    req.error = 404;
  } else {
    req.uri.path = uri;
  }

  *request = req;

  return 0;
}
