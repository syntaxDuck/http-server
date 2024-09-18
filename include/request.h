#ifndef REQUEST_H
#define REQUEST_H

#define MAX_URI_LENGTH 255
#define MAX_METHOD_LENGTH 10
#define MAX_PROTOCOL_LENGTH 10
#define INVALID_VERSION_NUMBER -1
#define INVALID_REQUEST_HEADER -1

typedef enum
{
  REQUEST_GET,
  REQUEST_SET,
} RequestMethod;

typedef enum
{
  PROTOCOL_HTTP,
} ProtocolType;

typedef enum
{
  ONE_TIME,
  KEEP_ALIVE,
} ConnectionType;

typedef struct
{
  char *path;
} Uri;

typedef struct
{
  ProtocolType type;
  int major_version;
  int minor_version;
} Protocol;

typedef struct
{
  RequestMethod method;
  Protocol protocol;
  ConnectionType type;
  char *user_agent;
  char *platform;
  char *accept;
  char *referer;
  char *accept_encoding;
  char *accept_language;
  Uri uri;
  int error;
} Request;

#define BUFFER_SIZE 1024

int header_parse_protocol(Protocol *protocol, char *protocol_str);
int header_parse_method(RequestMethod *method, char *method_str);
int header_parse_protocol_type(Protocol *protocol, char *protocol_str);
int header_parse_protocol_version(Protocol *protocol, char *version_str);

int get_resource(char *uri);
int read_request(int client_fd, char *buff);
int parse_request(Request *request, char *header_buff);
int process_request(int client_fd, Request *request);
#endif
