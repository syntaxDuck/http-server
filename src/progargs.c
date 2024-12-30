#include "progargs.h"
#include "stdbool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: Not sure if I like how a ProgramArgument is allocated on the heap and then later deallocated
//  after being added to ParsedArgs. Might wan't to just pass ProgramArgument by value so we arn't
//  unnecessarily throwing things on the heap

// TODO: Should probably make a system for default arguments for the argument definitons

static int get_value_from_flag(ProgramArgument *arg, const char *following_arg)
{
    char *equal_sign = strchr(arg->flag, '=');
    if (equal_sign)
    {
        *equal_sign = '\0';
        arg->value = equal_sign + 1;
    }
    else if (following_arg && following_arg[0] != '-')
    {
        arg->value = strdup(following_arg);
        return 1;
    }
    else
        arg->value = NULL;

    return 0;
}

static const ArgumentDefinition *get_argument_definition(const char *flag, ArgumentDefinitions arg_defs)
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

static ProgramArgument *parse_argument(char *argv[], int argc, int *current_arg)
{
    ProgramArgument *parsed_arg = malloc(sizeof(ProgramArgument));
    parsed_arg->flag = strdup(argv[*current_arg]);
    if (!parsed_arg->flag)
    {
        fprintf(stderr, "Error: memory allocation failed for flag\n");
        free(parsed_arg);
        return NULL;
    }

    int shift = 0;
    if (strncmp("--", parsed_arg->flag, 2) == 0)
    {
        shift = 2;
    }
    else if (parsed_arg->flag[0] == '-')
    {
        shift++;
    }
    else
    {
        fprintf(stderr, "Error: issue parsing flag %s\n", parsed_arg->flag);
        free(parsed_arg);
        return NULL;
    }

    char *temp = strdup(parsed_arg->flag + shift);
    if (!temp)
    {
        fprintf(stderr, "Error: issue allocating memory for temp flag\n");
        free(parsed_arg);
        return NULL;
    }

    free(parsed_arg->flag);
    parsed_arg->flag = temp;

    const char *following_arg = *current_arg + 1 < argc ? argv[*current_arg + 1] : NULL;

    if (get_value_from_flag(parsed_arg, following_arg))
    {
        *current_arg += 1;
    }

    return parsed_arg;
}

static void free_parsed_arg(ProgramArgument *arg)
{
    free(arg->flag);
    free(arg->value);
    free(arg);
}

static void add_parsed_arg(ParsedArguments *parsed_args, ProgramArgument *arg)
{

    parsed_args->args[parsed_args->count].flag = strdup(arg->flag);
    parsed_args->args[parsed_args->count].value = strdup(arg->value);
    parsed_args->count++;
    free_parsed_arg(arg);
}

static ParsedArguments *init_parsed_args(int capacity)
{
    ParsedArguments *parsed_args = malloc(sizeof(ParsedArguments));
    if (!parsed_args)
    {
        fprintf(stderr, "Error: Memory Allocation Failure for type -> ParsedArguments\n");
        return NULL;
    }
    parsed_args->args = malloc(sizeof(ProgramArgument) * capacity);
    if (!parsed_args->args)
    {
        fprintf(stderr, "Error: Memory Allocation Failure for type -> ArgumentDefinition\n");
        free(parsed_args);
        return NULL;
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

ParsedArguments *handle_arguments(int argc, char *argv[], const ArgumentDefinitions arg_defs)
{
    ParsedArguments *parsed_args = init_parsed_args(argc);

    // start at index 1 to avoid executable name arg
    for (int current_arg = 1; current_arg < argc; current_arg++)
    {

        ProgramArgument *arg = parse_argument(argv, argc, &current_arg);
        printf("Flag: %s\nValue: %s\n", arg->flag, arg->value);

        if (!arg)
        {
            free_parsed_args(parsed_args);
            return NULL;
        }
        const ArgumentDefinition *target_flag_def = get_argument_definition(arg->flag, arg_defs);

        // If unable to find flag definition
        if (!target_flag_def)
        {
            fprintf(stderr, "Error: Invalid argument %s provided\n", arg->flag);
            free_parsed_arg(arg);
            free_parsed_args(parsed_args);
            return NULL;
        }

        // If target flag definition requires a vlue and value is null
        if (target_flag_def->value_req && !arg->value)
        {
            fprintf(stderr, "Error: Flag %s provided without a value\n", arg->flag);
            free(arg);
            free_parsed_args(parsed_args);
            return NULL;
        }

        add_parsed_arg(parsed_args, arg);
    }

    return parsed_args;
}

char *get_argument_value(ParsedArguments *parsed_args, const char *arg)
{
    for (int i = 0; i < parsed_args->count; i++)
    {
        if (strcmp(arg, parsed_args->args[i].flag) == 0)
        {
            return parsed_args->args[i].value;
        }
    }
    return NULL;
}