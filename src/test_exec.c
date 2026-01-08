#include "exec.h"
#include "builtin.h"
#include <stdio.h>

int main(void) {
    printf("Test execute_command\n");
    char *argv1[] = {"echo", "Hello world!", NULL};
    execute_command(argv1);	

    printf("\nTest execute_pipeline\n");
    execute_pipeline();

    printf("\nTest execute_redirect\n");
    execute_redirect();

    printf("\nTest execute_builtin\n");
    char *argv2[] = {"true", NULL};  // ou echo, cd, etc.
    execute_builtin(argv2);

    return 0;
}

