#define _POSIX_C_SOURCE 200809L
#include "builtin.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "exec.h"
#include "expansion.h"

extern struct parser *g_parser;

int is_builtin(const char *cmd)
{
    if (!cmd)
        return 0;

    if (strcmp(cmd, "echo") == 0)
        return 1;
    if (strcmp(cmd, "true") == 0)
        return 1;
    if (strcmp(cmd, "false") == 0)
        return 1;
    if (strcmp(cmd, "exit") == 0)
        return 1;
    if (strcmp(cmd, "cd") == 0)
        return 1;
    if (strcmp(cmd, "unset") == 0)
        return 1;
    if (strcmp(cmd, "export") == 0)
        return 1;
    if (strcmp(cmd, "break") == 0)
        return 1;
    if (strcmp(cmd, "continue") == 0)
        return 1;
    return 0;
}

static int string_to_int(const char *str)
{
    int result = 0;
    int signe = 1;
    int j = 0;

    if (str[0] == '-')
    {
        signe = -1;
        j = 1;
    }

    while (str[j] != '\0')
    {
        if (str[j] >= '0' && str[j] <= '9')
        {
            result = result * 10 + (str[j] - '0');
            j++;
        }
        else
        {
            break;
        }
    }

    return result * signe;
}
static void echo_print(const char *s, int f)
{
    for (int i = 0; s[i]; i++)
    {
        if (f && s[i] == '\\' && s[i + 1])
        {
            i++;
            if (s[i] == 'n')
                putchar('\n');
            else if (s[i] == 't')
                putchar('\t');
            else if (s[i] == '\\')
                putchar('\\');
            else if (s[i] == '"')
                putchar('"');
            else
            {
                putchar('\\');
                putchar(s[i]);
            }
        }
        else
        {
            putchar(s[i]);
        }
    }
}

static int parse_echo_flags(char **argv, int *idx, int *n_flag, int *e_flag)
{
    while (argv[*idx] && argv[*idx][0] == '-')
    {
        if (!strcmp(argv[*idx], "-n"))
        {
            *n_flag = 1;
            *idx = *idx + 1;
        }
        else if (!strcmp(argv[*idx], "-e"))
        {
            *e_flag = 1;
            *idx = *idx + 1;
        }
        else if (!strcmp(argv[*idx], "-E"))
        {
            *e_flag = 0;
            *idx = *idx + 1;
        }
        else
            break;
    }
    return 0;
}

/*
static void print_echo_arg(char **argv, int idx, struct parser *parser, int
e_flag)
{
    char *expanded = expand(parser, parser->spe, argv[idx]);
    if (expanded)
    {
        echo_print(expanded, e_flag);
        free(expanded);
    }
    else
        echo_print(argv[idx], e_flag);
}
*/

static int builtin_echo(char **argv, struct parser *parser)
{
    int n_flag = 0;
    int e_flag = 0;
    int idx = 1;

    parse_echo_flags(argv, &idx, &n_flag, &e_flag);
    int first = 1;

    while (argv[idx])
    {
        char *expanded = expand(parser, parser->spe, argv[idx]);
        char *to_print = expanded ? expanded : argv[idx];

        if (to_print && *to_print)
        {
            if (!first)
                putchar(' ');
            first = 0;
            echo_print(to_print, e_flag);
        }

        if (expanded)
            free(expanded);
        idx++;
    }
    if (!n_flag)
        putchar('\n');
    fflush(stdout);
    return 0;
}

static int builtin_break(char **argv)
{
    if (argv && argv[0])
        return 0;
    return EXEC_BREAK;
}

static int builtin_continue(char **argv)
{
    if (argv && argv[0])
        return 0;
    return EXEC_CONTINUE;
}

static int builtin_true(char **argv)
{
    return argv ? 0 : 0;
}

static int builtin_false(char **argv)
{
    return argv ? 1 : 1;
}

static int builtin_exit(char **argv, struct parser *parser)
{
    int end_code = 0;
    if (argv[1] == NULL)
    {
        parser->exit = 1;
        parser->ex_code = parser->last_code;
        return 0;
    }

    if (argv[2] != NULL)
    {
        fprintf(stderr, "exit too many args\n");
        return 1;
    }

    end_code = string_to_int(argv[1]);
    parser->exit = 1;
    parser->ex_code = end_code;

    return end_code;
}

static int builtin_cd(char **argv)
{
    char *way = argv[1];

    if (way == NULL)
    {
        way = getenv("HOME");

        if (way == NULL)
        {
            fprintf(stderr, "cd: HOME non défini\n");
            return 1;
        }
    }

    if (chdir(way) != 0)
    {
        perror("cd");
        return 1;
    }

    return 0;
}

static int builtin_export(char **argv, struct parser *parser)
{
    if (!argv[1])
    {
        return 0;
    }
    char *arg = argv[1];
    char *equal = strchr(arg, '=');
    char *name;
    char *value;
    if (equal)
    {
        *equal = '\0';
        name = arg;
        value = equal + 1;
    }
    else
    {
        name = arg;
        value = "";
    }
    add_var(parser, name, value);
    struct variable *v = parser->var;
    while (v)
    {
        if (strcmp(v->nom, name) == 0)
        {
            v->exported = 1;
            setenv(name, value, 1);
            break;
        }
        v = v->next;
    }
    return 0;
}

static int parse_unset_flags(char **argv, int *idx, int *v_flag, int *f_flag)
{
    while (argv[*idx] && argv[*idx][0] == '-' && argv[*idx][1])
    {
        if (strcmp(argv[*idx], "-v") == 0)
        {
            *v_flag = 1;
            *idx = *idx + 1;
        }
        else if (strcmp(argv[*idx], "-f") == 0)
        {
            *f_flag = 1;
            *idx = *idx + 1;
        }
        else if (strcmp(argv[*idx], "--") == 0)
        {
            *idx = *idx + 1;
            return 0;
        }
        else
        {
            fprintf(stderr, "unset: invalid option: %s\n", argv[*idx]);
            return 2;
        }
    }
    return 0;
}

static int builtin_unset(char **argv, struct parser *parser)
{
    int v_flag = 0;
    int f_flag = 0;
    int idx = 1;
    int error_count = 0;
    int ret = 0;

    ret = parse_unset_flags(argv, &idx, &v_flag, &f_flag);
    if (ret != 0)
        return ret;

    if (argv[idx] == NULL)
        return 0;

    while (argv[idx])
    {
        const char *name = argv[idx];

        if (f_flag)
        {
            if (unset_function(parser, name) != 0)
                error_count++;
        }
        else if (v_flag || !f_flag)
        {
            if (unset_variable(parser, name) != 0)
                error_count++;
        }

        idx++;
    }

    return (error_count > 0) ? 1 : 0;
}

int execute_builtin(char **argv, struct parser *parser)
{
    if (argv == NULL || argv[0] == NULL)
        return -1;

    char *cmd = argv[0];

    if (strcmp(cmd, "echo") == 0)
        return builtin_echo(argv, parser);

    if (strcmp(cmd, "true") == 0)
        return builtin_true(argv);

    if (strcmp(cmd, "false") == 0)
        return builtin_false(argv);

    if (strcmp(cmd, "exit") == 0)
        return builtin_exit(argv, parser);

    if (strcmp(cmd, "cd") == 0)
        return builtin_cd(argv);

    if (strcmp(cmd, "unset") == 0)
        return builtin_unset(argv, parser);
    if (strcmp(cmd, "export") == 0)
        return builtin_export(argv, parser);
    if (strcmp(cmd, "break") == 0)
        return builtin_break(argv);
    if (strcmp(cmd, "continue") == 0)
        return builtin_continue(argv);

    return -1;
}
