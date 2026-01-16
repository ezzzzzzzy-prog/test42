#ifndef SPECIAL_H
#define SPECIAL_H

#include <sys/types.h>
#include <unistd.h>

struct special
{
        int exit_code; //c'est pour les $?
        pid_t shell_pid; // pour $$
        uid_t uid; // uid de l'utilisateur
        int argc_count; // $# (nombre d'arguments)
        char **args; // $1...$n et ya aussi $@ et aussi $*
        char *pwd; // pwd
        char *oldpwd; //oldpwd
};


struct special *create_special(char **args, int argc_count);
void free_special(struct special *spe);

#endif /*! SPECIAL_H !*/
