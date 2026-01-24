#include <assert.h>
#include <string.h>

#include "../../src/ast.h"
#include "../../src/io_backend.h"
#include "../../src/lexer.h"
#include "../../src/parser.h"

int main(void)
{
    char *argv[] = { "42sh", "-c", "echo hello", NULL };
    int argc = 3;

    assert(io_backend_init(argc, argv) == 0);

    struct lexer *lex = new_lex(NULL);
    assert(lex);

    struct parser *p = parser_new(lex);
    assert(p);

    struct ast *ast = parse_input(p);
    assert(ast);
    assert(ast->type == AST_COMMAND);

    struct ast_command *cmd = (struct ast_command *)ast;

    assert(cmd->argv);
    assert(cmd->argv[0]);
    assert(strcmp(cmd->argv[0], "echo") == 0);
    assert(cmd->argv[1]);
    assert(strcmp(cmd->argv[1], "hello") == 0);
    assert(cmd->argv[2] == NULL);

    ast_free(ast);
    parser_free(p);
    lexer_free(lex);
    io_backend_close();

    return 0;
}
