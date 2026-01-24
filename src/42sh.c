#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ast.h"
#include "exec.h"
#include "io_backend.h"
#include "parser.h"

int main(int argc, char **argv)
{
    if (io_backend_init(argc, argv) < 0)
        return 2;

    struct parser *parser = new_parse();
    if (!parser)
    {
        io_backend_close();
        return 2;
    }

    parser->spe = create_special(argv, argc);
    exec_set_parser(parser);

    int status = 0;
    while (!parser->exit)
    {
        struct ast *ast = parser_input(parser);
        if (!ast)
            break;
        status = exec_ast(ast);
        parser->last_code = status;
        ast_free(ast);
    }

    int should_exit = parser->exit;
    int exit_code = parser->ex_code;
    free_special(parser->spe);
    parser_free(parser);
    io_backend_close();

    if (should_exit)
        _exit(exit_code);
    return status;
}
