#include "request.h"
#include <stdlib.h>
#include <string.h>

Request_Method get_method(char *string) {
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
