#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/ast.h"

int main(void)
{
    struct ast_cmd *cmd = malloc(sizeof(*cmd));
    assert(cmd);

    cmd->base.type = AST_COMMAND;

    cmd->words = malloc(sizeof(char *) * 2);
    assert(cmd->words);

    cmd->words[0] = strdup("echo");
    cmd->words[1] = NULL;
    ast_free((struct ast *)cmd);

    return 0;
}
