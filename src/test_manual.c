#include "exec.h"
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    printf("\n Test 1: echo simple \n");
    char *argv1[] = {"echo", "Hello", "world", NULL};
    exec_command(argv1);
    
    printf("\n Test 2: echo -n \n");
    char *argv2[] = {"echo", "-n", "Pas de saut de ligne", NULL};
    exec_command(argv2);
    printf("\n");
    
    printf("\n Test 3: echo -e \n");
    char *argv3[] = {"echo", "-e", "Ligne1\\nLigne2\\tAvec tabulation", NULL};
    exec_command(argv3);
    
    printf("\n Test 4: true \n");
    char *argv4[] = {"true", NULL};
    int ret = exec_command(argv4);
    printf("Code return: %d (devrait être 0)\n", ret);
    
    printf("\n Test 5: false \n");
    char *argv5[] = {"false", NULL};
    ret = exec_command(argv5);
    printf("Code return: %d (devrait être 1)\n", ret);
    
    printf("\n Test 6: ls (commande externe) \n");
    char *argv6[] = {"ls", "-la", NULL};
    exec_command(argv6);
    
    printf("\n Test 7: cd \n");
    char *argv7[] = {"cd", "/tmp", NULL};
    ret = exec_command(argv7);
    printf("Changement vers /tmp, code return: %d\n", ret);
    
    char *argv8[] = {"pwd", NULL};
    exec_command(argv8);
    
    return 0;
}

