#include "exec.h"
#include "builtin.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int execute_command(char **argv)
{
    if (!argv || !argv[0])
        return -1;
    
    if (is_builtin(argv[0]))
    {
        return execute_builtin(argv);
    }
    
    pid_t pid = fork();
    
    if (pid == -1)
    {
        perror("fork");
        return -1;
    }
    
    if (pid == 0)
    {
        execvp(argv[0], argv);
        perror("execvp");
        exit(127);
    }
    
    int status;
    waitpid(pid, &status, 0);
    
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    
    return -1;
}

int execute_pipeline()
{
    printf("Pipeline exec (TODO)\n");
    return 0;
}

int execute_redirect()
{
    printf("Redirect exec (TODO)\n");
    return 0;
}
