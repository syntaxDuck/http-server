#ifndef PROGARGS_H
#define PROGARGS_H
#include <stdbool.h>

typedef struct
{
    const char *flag;
    bool value_req;
} ArgumentDefinition;

typedef struct
{
    const ArgumentDefinition *def;
    int count;
} ArgumentDefinitions;

typedef struct
{
    char *flag;
    char *value;
} ProgramArgument;

typedef struct
{
    ProgramArgument *args;
    int count;
} ParsedArguments;

#endif