#ifndef REQUEST_H
#define REQUEST_H

#define MAX_URI_LENGTH 255
#define MAX_METHOD_LENGTH 10
#define MAX_PROTOCOL_LENGTH 10
#define INVALID_VERSION_NUMBER -1
#define INVALID_REQUEST_HEADER -1
#define CRLF "\r\n"
typedef enum
{
  METHOD_OPTION,
  METHOD_GET,
  METHOD_HEAD,
  METHOD_POST,
  METHOD_PUT,
  METHOD_DELETE,
  METHOD_TRACE,
  METHOD_CONNECT
} RequestMethod;

typedef enum
{
  INFO_CONTINUE = 100,
  INFO_SWITCH_PROTOCOLS = 101
} InfoStatusCode;

typedef enum
{
  SUCCESS_OK = 200,
  SUCCESS_CREATED,
  SUCCESS_ACCEPTED,
  SUCCESS_NONAUTH_INFO,
  SUCCESS_NO_CONTENT,
  SUCCESS_RESET_CONTENT,
  SUCCESS_PARTIAL_CONTENT
} SuccessStatusCode;

typedef enum
{
  REDIRECT_MULTI_CHOICES = 300,
  REDIRECT_MOVED,
  REDIRECT_FOUND,
  REDIRECT_SEE_OTHER,
  REDIRECT_NOT_MODIFIED,
  REDIRECT_USE_PROXEY,
  REDIRECT_UNUSED,
  REDIRECT_TEMP,
} RedirectStatusCode;

typedef enum
{
  CLIENT_BAD_REQUEST = 400,
  CLIENT_UNAUTH,
  CLIENT_PAYMENT_REQ,
  CLIENT_FORBIDDEN,
  CLIENT_NOT_FOUND,
  CLIENT_METHOD_NOT_ALLOWED,
  CLIENT_NOT_ACCEPTABLE,
  CLIENT_PROXY_AUTH_REQ,
  CLIENT_REQUEST_TIMEOUT,
  CLIENT_CONFLICT,
  CLIENT_GONE,
  CLIENT_LENGTH_REQ,
  CLIENT_PRECONDITION_FAILED,
  CLIENT_REQUEST_ENTITY_LARGE,
  CLIENT_REQUEST_URI_LENGTH,
  CLIENT_UNSUPPORTED_MEDIA_TYPE,
  CLIENT_REQUESTED_RANGE_NOT_SATISFIABLE,
  CLIENT_EXPCTATION_FAILED

} ClientErrorStatusCode;

typedef enum
{
  SERVER_INTERNAL_ERROR = 500,
  SERVER_NOT_IMPLEMENTED,
  SERVER_BAD_GATEWAY,
  SERVER_SERVICE_UNAVAILABLE,
  SERVER_GATEWAY_TIMEOUT,
  SERVER_HTTP_VERSION_NOT_SUPPORTED
} ServerErrorStatusCode;

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
char *read_request(int client_fd);
int parse_request(Request *request, char *header_buff);
int process_request(int client_fd, Request *request);
void send_response(int, char *, char *);
#endif
