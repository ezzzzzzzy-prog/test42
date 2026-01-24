#include "../../src/builtin.h"
#include <stdio.h>

int main(void)
{
    printf("\n Test 1: echo simple \n");
    char *test1[] = {"echo", "Salut", "les", "copines", NULL};
    execute_builtin(test1);
    
    printf("\n Test 2: echo -n (pas de newline) \n");
    char *test2[] = {"echo", "-n", "Pas de \\n", NULL};
    execute_builtin(test2);
    printf(" <-- pas de saut de ligne\n");
    
    printf("\n Test 3: echo -e (avec \\n et \\t) \n");
    char *test3[] = {"echo", "-e", "Ligne1\\nLigne2\\tTabulation", NULL};
    execute_builtin(test3);
    
    printf("\n Test 4: true \n");
    char *test4[] = {"true", NULL};
    int retour = execute_builtin(test4);
    printf("Retour: %d (devrait être 0)\n", retour);
    
    printf("\n Test 5: false \n");
    char *test5[] = {"false", NULL};
    retour = execute_builtin(test5);
    printf("Retour: %d (devrait être 1)\n", retour);
    
    printf("\n Test 6: cd /tmp \n");
    char *test6[] = {"cd", "/tmp", NULL};
    retour = execute_builtin(test6);
    printf("Retour: %d (devrait être 0)\n", retour);
    
  //  printf("\n Test 7: exit 42 (code ASCII) \n");
  //  char *test7[] = {"exit", "42", NULL};
  // printf("Appel de exit 42...\n");
    
    return 0;
}

