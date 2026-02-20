#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ast/ast.h"
#include "exec/exec.h"
#include "io_backend/io_backend.h"
#include "parser/parser.h"
#include "special/special.h"

/* Boucle principale d'execution du shell */
static int run_shell(struct parser *parser)
{
    int status = 0;
    while (!parser->exit)
    {
        struct ast *ast = parser_input(parser);
        if (!ast)
        {
            if (parser->parse_error)
                return 2;
            break;
        }
        status = exec_ast(ast);
        parser->last_code = status;
        if (parser->spe)
            parser->spe->exit_code = status;
        ast_free(ast);
    }
    return status;
}

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
    parser->spe = create_special();
    exec_set_parser(parser);
    int status = run_shell(parser);
    int should_exit = parser->exit;
    int exit_code = parser->ex_code;
    free_special(parser->spe);
    parser_free(parser);
    io_backend_close();
    if (should_exit)
        _exit(exit_code);
    return status;
}
