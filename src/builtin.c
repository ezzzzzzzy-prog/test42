#include "builtin.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

static int builtin_echo(char **argv)
{
    int n_flag = 0;
    int e_flag = 0;
    int idx = 1;
    
    while (argv[idx] && argv[idx][0] == '-')
    {
        if (!strcmp(argv[idx], "-n"))
        {
            n_flag = 1;
            idx++;
        }
        else if (!strcmp(argv[idx], "-e"))
        {
            e_flag = 1;
            idx++;
        }
        else if (!strcmp(argv[idx], "-E"))
        {
            e_flag = 0;
            idx++;
        }
        else
            break;
    }
    
    int first = 1;
    while (argv[idx])
    {
        if (!first)
            putchar(' ');
        first = 0;
        
        char *s = argv[idx];
        int i = 0;
        while (s[i])
        {
            if (e_flag && s[i] == '\\' && s[i + 1])
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
                i++;
            }
            else
            {
                putchar(s[i]);
                i++;
            }
        }
        idx++;
    }
    
    if (!n_flag)
        putchar('\n');
    
    fflush(stdout);
    return 0;
}

static int builtin_true(char **argv)
{
    if (argv)
        argv = argv;
    return 0;
}

static int builtin_false(char **argv)
{
    if (argv)
        argv = argv;
    return 1;
}

static int builtin_exit(char **argv)
{
    int end_code = 0;
    
    if (argv[1] != NULL)
    {
        end_code = string_to_int(argv[1]);
    }
    
    exit(end_code);
    return 0;
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

int execute_builtin(char **argv)
{
    if (argv == NULL || argv[0] == NULL)
        return -1;
    
    char *cmd = argv[0];
    
    if (strcmp(cmd, "echo") == 0)
        return builtin_echo(argv);
    
    if (strcmp(cmd, "true") == 0)
        return builtin_true(argv);
    
    if (strcmp(cmd, "false") == 0)
        return builtin_false(argv);
    
    if (strcmp(cmd, "exit") == 0)
        return builtin_exit(argv);
    
    if (strcmp(cmd, "cd") == 0)
        return builtin_cd(argv);
    
    return -1;
}
