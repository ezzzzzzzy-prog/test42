#include <assert.h>
#include <stdio.h>

#include "../../src/exec/exec.h"
#include "../../src/parser/parser.h"

int main(void)
{
    printf("Test exec_set_parser\n");
    struct parser *p = new_parse();
    assert(p);
    exec_set_parser(p);
    assert(g_parser == p);
    printf("OK: exec_set_parser\n");
    parser_free(p);
    return 0;
}
