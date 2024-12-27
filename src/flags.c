#include "flags.h"
#include "stdbool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: Not sure if I like how a ProgramArgument is allocated on the heap and then later deallocated
//  after being added to ParsedArgs. Might wan't to just pass ProgramArgument by value so we arn't
//  unnecessarily throwing things on the heap

static char *get_value_from_flag(char *flag)
{
    if (strncmp(flag, "--", 2) == 0)
    {
        char *equal_sign = strchr(flag, "=");
        if (equal_sign)
        {
            *equal_sign = '\0';
            return equal_sign + 1;
        }
    }

    return NULL;
}

static ArgumentDefinition *get_argument_definition(const char *flag, ArgumentDefinitions arg_defs)
{
    for (int j = 0; j < arg_defs.count; j++)
    {
        if (strcmp(arg_defs.def[j].flag, flag) == 0)
        {
            return &arg_defs.def[j];
        }
    }

    return NULL;
}

static ProgramArgument *parse_argument(char *argv[], int argc, int current_arg)
{
    ProgramArgument *parsed_arg = malloc(sizeof(ProgramArgument));
    parsed_arg->flag = strdup(argv[current_arg]);
    if (!parsed_arg->flag)
    {
        fprintf(stderr, "Error: memory allocation failed for flag\n");
        free(parsed_arg);
        return NULL;
    }

    parsed_arg->value = get_value_from_flag(parsed_arg->flag);

    if (!parsed_arg->value && current_arg + 1 < argc && argv[current_arg + 1][0] != '-')
    {
        parsed_arg->value = argv[++current_arg];
    }

    return parsed_arg;
}

static void free_parsed_arg(ProgramArgument *arg)
{
    free(arg->flag);
    free(arg);
}

ParsedArguments *init_parsed_args(int capacity)
{
    ParsedArguments *parsed_args = malloc(sizeof(ParsedArguments));
    if (!parsed_args)
    {
        fprintf(stderr, "Error: Memory Allocation Failure for type -> ParsedArguments\n");
        exit(EXIT_FAILURE);
    }
    parsed_args->args = malloc(sizeof(ProgramArgument) * capacity);
    if (!parsed_args->args)
    {
        fprintf(stderr, "Error: Memory Allocation Failure for type -> ArgumentDefinition\n");
        free(parsed_args);
        exit(EXIT_FAILURE);
    }
    parsed_args->count = 0;
    return parsed_args;
}

void free_parsed_args(ParsedArguments *parsed_args)
{
    for (int i = 0; i < parsed_args->count; i++)
    {
        free(parsed_args->args[i].flag);
        free(parsed_args->args[i].value);
    }
    free(parsed_args->args);
    free(parsed_args);
}

void add_parsed_arg(ParsedArguments *parsed_args, ProgramArgument *arg)
{
    parsed_args->args[parsed_args->count] = *arg;
    parsed_args->count++;
    free_parsed_arg(arg);
}

ParsedArguments *handle_arguments(int argc, char *argv[], const ArgumentDefinitions arg_defs)
{
    ParsedArguments *parsed_args = init_parsed_args(argc);
    for (int current_arg = 0; current_arg < argc; current_arg++)
    {

        ProgramArgument *arg = parse_argument(argv, argc, current_arg);
        if (!arg)
        {
            free_parsed_args(parsed_args);
            exit(EXIT_FAILURE);
        }

        ArgumentDefinition *target_flag_def = get_argument_definition(arg->flag, arg_defs);

        // If unable to find flag definition
        if (!target_flag_def)
        {
            fprintf(stderr, "Error: Invalid argument %s provided\n", arg->flag);
            free_parsed_arg(arg);
            free_parsed_args(parsed_args);
            exit(EXIT_FAILURE);
        }

        // If target flag definition requires a vlue and value is null
        if (target_flag_def->value_req && !arg->value)
        {
            fprintf(stderr, "Error: Flag %s provided without a value\n", arg->flag);
            free(arg);
            free_parsed_args(parsed_args);
            exit(EXIT_FAILURE);
        }

        add_parsed_arg(parsed_args, arg);
    }

    return parsed_args;
}