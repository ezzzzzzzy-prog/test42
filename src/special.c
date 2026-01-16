#include "special.h"
#include <stdlib.h>
#include <string.h>

struct special *create_special(char **args, int argc_count)
{
    struct special *spe = malloc(sizeof(struct special));
    if(!spe)
    {
            return NULL;
    }
    spe->exit_code = 0;
    spe->shell_pid = getpid();
    spe->uid = getuid();
    spe->pwd = getcwd(NULL, 0);
    spe->oldpwd = NULL;
    spe->argc_count = argc_count;
    spe->args = args;
    return spe;
}


void free_special(struct special *spe)
{
        if(!spe)
        {
                return;
        }
        free(spe->pwd);
        free(spe->oldpwd);
        free(spe);
}
