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

