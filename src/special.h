#ifndef SPECIAL_H
#define SPECIAL_H

#include <sys/types.h>
#include <unistd.h>

struct special
{
    int exit_code;        // $? : code de retour de la dernière commande
    pid_t shell_pid;      // $$ : PID du shell
    uid_t uid;            // UID de l'utilisateur

    int argc_count;       // $# : nombre d'arguments positionnels
    char **args;          // $1..$n, $@, $*

    char *script_name;    // $0 (ex: "42sh")

    char *pwd;            // $PWD
    char *oldpwd;         // $OLDPWD
    
    pid_t last_bg_pid;    // $!
    char *shell_opts;     // $- (ex: "himBH")
};

struct special *create_special(char **args, int argc_count);
void free_special(struct special *spe);

#endif /* SPECIAL_H */
