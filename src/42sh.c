#include <stdlib.h>
#include <stdio.h>

#include "io_backend.h"
#include "parser.h"
#include "exec.h"
#include "ast.h"

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

    struct ast *ast = parser_input(parser);
    if (!ast)
    {
	    int status = 0;
        parser_free(parser);
        io_backend_close();
        return status;
    }

    int status = exec_ast(ast);

    ast_free(ast);
    parser_free(parser);
    io_backend_close();

    return status;
}
