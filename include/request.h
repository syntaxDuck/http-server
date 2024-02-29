#ifndef REQUEST_H

#define MAX_URI_LENGTH 255
#define MAX_METHOD_LENGTH 10
#define MAX_PROTOCOL_LENGTH 10
#define INVALID_VERSION_NUMBER -1
#define INVALID_REQUEST_HEADER -1

typedef enum {
  REQUEST_GET,
  REQUEST_SET,
  REQUEST_INVALID = -1,
} RequestMethod;

typedef enum {
  PROTOCOL_HTTP,
  PROTOCOL_INVALID = -1,
} ProtocolType;

typedef struct {
  char *path;
} Uri;

typedef struct {
  ProtocolType type;
  int majorVersion;
  int minorVersion;
} Protocol;

typedef struct {
  int x;
} RequestDataType;

typedef struct {
  RequestMethod method;
  Uri uri;
  Protocol protocol;
  int error;
} Request;

RequestMethod parseRequestMethod(char *methodString);
ProtocolType parseProtocolType(char *protocolString);
int parseProtocolVersion(Protocol *protocol, char *versionString);

int getResource(char *uri);
int processRequestHeader(Request *request, char *headerBuffer);
#endif // !REQUEST_H
