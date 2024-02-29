#include "request.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

RequestMethod parseRequestMethod(char *methodString) {
  if (methodString == NULL) {
    fprintf(stderr, "Error: NULL pointer passed as methodString\n");
    return REQUEST_INVALID;
  }

  for (int i = 0; methodString[i]; i++) {
    methodString[i] = toupper(methodString[i]);
  }

  if (strcmp(methodString, "GET") == 0) {
    return REQUEST_GET;
  } else if (strcmp(methodString, "SET") == 0) {
    return REQUEST_SET;

  } else {
    return REQUEST_INVALID;
  }
}

ProtocolType parseProtocolType(char *protocolString) {
  if (protocolString == NULL) {
    fprintf(stderr, "Error: NULL pointer passed as protocolString\n");
    return PROTOCOL_INVALID;
  }

  for (int i = 0; protocolString[i]; i++) {
    protocolString[i] = toupper(protocolString[i]);
  }

  char *slashPos = strstr(protocolString, "/");
  if (slashPos == NULL) {
    fprintf(stderr, "Error: Invalid protocol string\n");
    return PROTOCOL_INVALID;
  }
  int offset = slashPos - protocolString;
  char *protocolType = malloc(offset + 1);
  if (protocolType == NULL) {
    fprintf(stderr, "Error: unable to allocate memory\n");
    return PROTOCOL_INVALID;
  }

  strncpy(protocolType, protocolString, offset);
  protocolType[offset] = '\0';

  ProtocolType result;
  if (strcmp(protocolType, "HTTP") == 0) {
    result = PROTOCOL_HTTP;
  } else {
    result = PROTOCOL_INVALID;
  }

  free(protocolType);
  return result;
}

int parseProtocolVersion(Protocol *protocol, char *versionString) {
  if (versionString == NULL) {
    fprintf(stderr, "Error: NULL pointer passed as versionString\n");
    return PROTOCOL_INVALID;
  }

  while (*versionString && !isdigit(*versionString))
    versionString++;

  int majorVersion = -1, minorVersion = -1;
  if (sscanf(versionString, "%d.%d", &majorVersion, &minorVersion) != 2) {
    fprintf(stderr, "Error: Invalid protocol version format\n");
    return INVALID_VERSION_NUMBER;
  }

  protocol->majorVersion = majorVersion;
  if (protocol->majorVersion < 0) {
    fprintf(stderr, "Error: Invalid protocol major version");
    return INVALID_VERSION_NUMBER;
  }

  protocol->minorVersion = minorVersion;
  if (protocol->minorVersion < 0) {
    fprintf(stderr, "Error: Invalid protocol minor version");
    return INVALID_VERSION_NUMBER;
  }

  return 0;
}

int processRequestHeader(Request *request, char *buff) {

  if (buff == NULL) {
    fprintf(stderr, "Error: NULL pointer passed as buff");
    return INVALID_REQUEST_HEADER;
  }

  if (strlen(buff) == 0) {
    fprintf(stderr, "Error: Empty string passes as buff");
    return INVALID_REQUEST_HEADER;
  }

  char method[MAX_METHOD_LENGTH];
  char uri[MAX_URI_LENGTH];
  char protocol[MAX_PROTOCOL_LENGTH];

  // if (getcwd(uri, MAX_URI_LENGTH) == NULL) {
  //   fprintf(stderr, "Error: Unable to get cwd");
  //   return INVALID_REQUEST_HEADER;
  // }

  if (sscanf(buff, "%s %s %s", method, uri + strlen(uri), protocol) != 3) {
    fprintf(stderr, "Error: Invalid request header format\n");
    return INVALID_REQUEST_HEADER;
  }

  request->method = parseRequestMethod(method);
  request->protocol.type = parseProtocolType(protocol);

  if (request->method < 0 || request->protocol.type < 0) {
    return INVALID_REQUEST_HEADER;
  }

  if (parseProtocolVersion(&request->protocol, protocol) < 0)
    return INVALID_REQUEST_HEADER;

  request->uri.path = strdup(uri + 1);

  if (request->uri.path == NULL) {
    fprintf(stderr, "Error: Unable to allocate memory for request URI\n");
    return INVALID_REQUEST_HEADER;
  }

  return 0;
}
