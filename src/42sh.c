#include <stdlib.h>
#include <stdio.h>

#include "io_backend.h"
#include "parser.h"
#include "exec.h"
#include "ast.h"
#include <unistd.h>
#include <sys/wait.h>

/*int main(int argc, char **argv)
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
    struct ast *ast = parser_input(parser);
    if (!ast)
    {
        int status = 0;
        free_special(parser->spe); 
        parser_free(parser);
        io_backend_close();
        return status;
    }
    
    exec_set_parser(parser);
    int status = exec_ast(ast);
    
    ast_free(ast);
    free_special(parser->spe); 
    parser_free(parser);
    io_backend_close();
    return status;
}
*/
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

    free_special(parser->spe);
    parser_free(parser);
    //io_backend_close();
    if(parser->exit)
        _exit(parser->ex_code);
    io_backend_close();
    return status;
}

