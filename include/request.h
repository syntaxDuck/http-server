#ifndef REQUEST_H

typedef enum {
  GET,
  SET,
} Request_Method;

typedef struct {
  char *path;
} URI;

typedef enum {
  HTTP,
} Protocol_Type;

typedef struct {
  Protocol_Type type;
  int major_version;
  int minor_version;
} Protocol;

typedef struct {
  Request_Method method;
  URI uri;
  Protocol protocol;
  int error;
} Request;

int get_method(Request_Method *method, char *string);
int get_protocol(Protocol *protocol, char *string);
int get_protocol_version(Protocol *protocol, char *string);
int get_resource(char *uri);
int process_header(Request *request, char *buff);
#endif // !REQUEST_H
