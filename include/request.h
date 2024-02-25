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
} Request;

Request_Method get_method(char *string);
Protocol_Type get_protocol(char *string);
void get_protocol_version(char *string, Protocol *protocol);
void get_resource(char *uri);
#endif // !REQUEST_H
