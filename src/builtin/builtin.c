#define _POSIX_C_SOURCE 200809L
#include "builtin.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../exec/exec.h"
#include "../expansion/expansion.h"

extern struct parser *g_parser;

// Check si le nom correspond a un builtin
int is_builtin(const char *cmd)
{
    if (!cmd)
        return 0;

    if (strcmp(cmd, "echo") == 0)
        return 1;

    if (strcmp(cmd, "exit") == 0)
        return 1;

    if (strcmp(cmd, "cd") == 0)
        return 1;

    if (strcmp(cmd, "kill") == 0)
        return 1;

    return 0;
}

// Custom atoi pour gerer les codes de retour
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

// Affiche un str + gerer les backslashs pour echo -e
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

// Parse les options -n, -e et -E pour echo
static int parse_echo_flags(char **argv, int *idx, int *n_flag, int *e_flag)
{
    while (argv[*idx] && argv[*idx][0] == '-' && argv[*idx][1])
    {
        char *arg = argv[*idx];
        for (int i = 1; arg[i]; i++)
        {
            if (arg[i] == 'n')
                *n_flag = 1;
            else if (arg[i] == 'e')
                *e_flag = 1;
            else if (arg[i] == 'E')
                *e_flag = 0;
            else
                return 0;
        }
        *idx = *idx + 1;
    }
    return 0;
}

// Builtin echo : affiche du texte
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

        if (!first)
            putchar(' ');
        first = 0;
        echo_print(to_print ? to_print : "", e_flag);

        if (expanded)
            free(expanded);
        idx++;
    }
    if (!n_flag)
        putchar('\n');
    fflush(stdout);
    return 0;
}

// Builtin exit : quitte le shell
static int builtin_exit(char **argv, struct parser *parser)
{
    if (argv[1] == NULL)
    {
        parser->exit = 1;
        parser->ex_code = parser->last_code;
        return 0;
    }
    if (argv[2] != NULL)
    {
        printf("minishell: exit: too many arguments\n");
        return 1;
    }
    // Verif que l arg est bien un nombre
    for (int i = 0; argv[1][i]; i++)
    {
        if ((i == 0 && argv[1][i] == '-') || (i == 0 && argv[1][i] == '+'))
            continue;
        if (argv[1][i] < '0' || argv[1][i] > '9')
        {
            fprintf(stderr, "minishell: exit: %s: numeric argument required\n",
                    argv[1]);
            parser->exit = 1;
            parser->ex_code = 2;
            return 2;
        }
    }
    parser->exit = 1;
    parser->ex_code = string_to_int(argv[1]) & 0xFF;
    return parser->ex_code;
}

// Builtin cd : change le directory actuel
static int builtin_cd(char **argv)
{
    char *way = argv[1];

    if (way == NULL)
    {
        way = getenv("HOME");
        if (way == NULL)
        {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }
    }

    if (chdir(way) != 0)
    {
        fprintf(stderr, "cd: %s: No such file or directory\n", way);
        return 1;
    }

    return 0;
}

// Parse le numero du signal pour kill (ex: -9)
static int parse_kill_signal(const char *arg, int *sig)
{
    int i = 1;
    while (arg[i])
    {
        if (arg[i] < '0' || arg[i] > '9')
        {
            fprintf(stderr, "kill: invalid signal specification\n");
            return 0;
        }
        i++;
    }
    *sig = atoi(&arg[1]);
    return 1;
}

// Envoie le signal a chaque PID de la liste
static int send_signal(char **argv, int idx, int sig)
{
    while (argv[idx])
    {
        pid_t pid = atoi(argv[idx]);
        if (pid <= 0)
        {
            fprintf(stderr, "kill: (%s) - invalid pid\n", argv[idx]);
            return 1;
        }
        if (kill(pid, sig) < 0)
        {
            perror("kill");
            return 1;
        }
        idx++;
    }
    return 0;
}

// Builtin kill : envoie des signaux aux pids
static int builtin_kill(char **argv)
{
    if (!argv || !argv[1])
    {
        fprintf(stderr, "kill: usage: kill [-signal] pid\n");
        return 1;
    }
    int sig = SIGTERM;
    int idx = 1;
    if (argv[1][0] == '-')
    {
        if (!parse_kill_signal(argv[1], &sig))
            return 1;
        idx = 2;
    }
    return send_signal(argv, idx, sig);
}

// Dispatch l execution vers le bon builtin
int execute_builtin(char **argv, struct parser *parser)
{
    if (argv == NULL || argv[0] == NULL)
        return -1;

    char *cmd = argv[0];

    if (strcmp(cmd, "echo") == 0)
        return builtin_echo(argv, parser);

    if (strcmp(cmd, "exit") == 0)
        return builtin_exit(argv, parser);

    if (strcmp(cmd, "cd") == 0)
        return builtin_cd(argv);

    if (strcmp(cmd, "kill") == 0)
        return builtin_kill(argv);

    return -1;
}
