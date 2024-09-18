#ifndef RESPONSE_H
#define RESPONSE_H
#include "request.h"

int process_response(int client_fd, Request request);
char *build_response(Request request, char *content);
char *load_uri(Request request);
#endif