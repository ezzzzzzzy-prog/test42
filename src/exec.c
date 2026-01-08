#include "exec.h"
#include <stdlib.h>
#include <stdio.h>

int exec_command(const char *cmd)
{
    if (!cmd)
        return -1;

    printf("Execute command: %s\n", cmd);
    int ret = system(cmd);
    return ret;
}

int exec_pipeline(/* type à définir plus tard */)
{
    printf("Pipeline exec\n");
    return 0;
}

int exec_redirect(/* type à définir plus tard */)
{
    printf("Redirect exec\n");
    return 0;
}

int exec_builtin(/* type à définir plus tard */)
{
    printf("Builtin exec\n");
    return 0;
}
