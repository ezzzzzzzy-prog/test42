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
    int afficher_newline = 1;
    int interpret_slash = 0;
    int i = 1;
    
    while (argv[i] != NULL && argv[i][0] == '-')
    {
        if (strcmp(argv[i], "-n") == 0)
        {
            afficher_newline = 0;
            i++;
        }
        else if (strcmp(argv[i], "-e") == 0)
        {
            interpret_slash = 1;
            i++;
        }
        else if (strcmp(argv[i], "-E") == 0)
        {
            interpret_slash = 0;
            i++;
        }
        else
        {
            break;
        }
    }
    
    while (argv[i] != NULL)
    {
        char *mot = argv[i];
        
        if (interpret_slash)
        {
            int j = 0;
            while (mot[j] != '\0')
            {
                if (mot[j] == '\\' && mot[j + 1] != '\0')
                {
                    if (mot[j + 1] == 'n')
                    {
                        putchar('\n');
                        j = j + 2;
                    }
                    else if (mot[j + 1] == 't')
                    {
                        putchar('\t');
                        j = j + 2;
                    }
                    else if (mot[j + 1] == '\\')
                    {
                        putchar('\\');
                        j = j + 2;
                    }
                    else
                    {
                        putchar(mot[j]);
                        j++;
                    }
                }
                else
                {
                    putchar(mot[j]);
                    j++;
                }
            }
        }
        else
        {
            printf("%s", mot);
        }
        
        i++;
        
        if (argv[i] != NULL)
            putchar(' ');
    }
    
    if (afficher_newline)
        putchar('\n');
    
    fflush(stdout);
    
    return 0;
}

static int builtin_true(char ** argv)
{
    (void)argv;
    return 0;
}

static int builtin_false(char ** argv)
{
    (void)argv;
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
