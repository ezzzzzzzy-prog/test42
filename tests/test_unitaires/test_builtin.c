#include <assert.h>
#include <stdio.h>

#include "../../src/builtin/builtin.h"

int main(void)
{
    printf("Test is_builtin\n");
    assert(is_builtin("echo") == 1);
    assert(is_builtin("cd") == 1);
    assert(is_builtin("exit") == 1);
    assert(is_builtin("kill") == 1);
    assert(is_builtin("ls") == 0);
    assert(is_builtin(NULL) == 0);
    printf("OK: is_builtin\n");
    return 0;
}
