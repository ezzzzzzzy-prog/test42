#define _POSIX_C_SOURCE 200809L
#include "io_backend.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Stream global pour la lecture (file, stdin ou buffer memoire)
static FILE *f = NULL;

// Initialise la source d entree (stdin, chaine -c ou fichier)
int io_backend_init(int argc, char **argv)
{
    if (argc == 1)
    {
        f = stdin;
        return 0;
    }

    if (argc >= 3 && strcmp(argv[1], "-c") == 0)
    {
        f = fmemopen(argv[2], strlen(argv[2]), "r");
        return f ? 0 : -1;
    }

    if (argv[1][0] == '-')
    {
        fprintf(stderr, "minishell: invalid option\n");
        fprintf(stderr, "usage: minishell [-c string] [script]\n");
        return -1;
    }

    f = fopen(argv[1], "r");
    if (!f)
    {
        perror("minishell");
        return -1;
    }
    return 0;
}

// Uitilse et renvoie le prochain caractere
int io_backend_next(void)
{
    if (!f)
    {
        return EOF;
    }
    int res = fgetc(f);
    return res;
}

// Renvoie le prochain caractere sans le consommer (via ungetc)
int io_backend_peek(void)
{
    if (!f)
        return EOF;

    int c = fgetc(f);
    if (c != EOF)
        ungetc(c, f);
    return c;
}

// Ferme le flux sauf si c est stdin
void io_backend_close(void)
{
    if (f && f != stdin)
    {
        fclose(f);
    }
    f = NULL;
}
