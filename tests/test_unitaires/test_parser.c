#include <assert.h>
#include <string.h>

#include "../../src/ast/ast.h"
#include "../../src/io_backend/io_backend.h"
#include "../../src/lexer/lexer.h"
#include "../../src/parser/parser.h"

int main(void)
{
    char *argv[] = { "minishell", "-c", "echo hello", NULL };
    int argc = 3;

    assert(io_backend_init(argc, argv) == 0);

    struct parser *p = new_parse();
    assert(p);

    struct ast *ast = parser_input(p);
    assert(ast);
    assert(ast->type == AST_COMMAND);

    assert(ast->data.cmd.words);
    assert(ast->data.cmd.words[0]);
    assert(strcmp(ast->data.cmd.words[0], "echo") == 0);
    assert(ast->data.cmd.words[1]);
    assert(strcmp(ast->data.cmd.words[1], "hello") == 0);
    assert(ast->data.cmd.words[2] == NULL);

    ast_free(ast);
    parser_free(p);
    io_backend_close();

    return 0;
}
