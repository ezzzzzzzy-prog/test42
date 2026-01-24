#include <stdio.h>

#include "io_backend.h"
#include "lexer.h"
#include "parser.h"

static void print_ast(struct ast *ast, int indent)
{
    if (!ast)
        return;

    for (int i = 0; i < indent; i++)
        printf("  ");

    if (ast->type == AST_COMMAND)
    {
        struct ast_command *cmd = (struct ast_command *)ast;
        printf("COMMAND: ");
        for (int i = 0; cmd->argv[i] != NULL; i++)
            printf("%s ", cmd->argv[i]);
        printf("\n");
    }
    else if (ast->type == AST_IF)
    {
        struct ast_if *if_node = (struct ast_if *)ast;
        printf("IF:\n");

        for (int i = 0; i < indent + 1; i++)
            printf("  ");
        printf("Condition:\n");
        print_ast(if_node->condition, indent + 2);

        for (int i = 0; i < indent + 1; i++)
            printf("  ");
        printf("Then:\n");
        print_ast(if_node->then_body, indent + 2);

        if (if_node->else_body)
        {
            for (int i = 0; i < indent + 1; i++)
                printf("  ");
            printf("Else:\n");
            print_ast(if_node->else_body, indent + 2);
        }
    }
    else if (ast->type == AST_LIST)
    {
        struct ast_list *list = (struct ast_list *)ast;
        printf("LIST (%zu commands):\n", list->count);
        for (size_t i = 0; i < list->count; i++)
            print_ast(list->commands[i], indent + 1);
    }
}

int main(int argc, char **argv)
{
    if (io_backend_init(argc, argv) < 0)
        return 1;

    struct lexer *lex = new_lex(NULL);
    struct parser *p = parser_new(lex);

    struct ast *ast = parse_input(p);

    if (ast)
    {
        printf("=== AST créé avec succès ===\n");
        print_ast(ast, 0);
        ast_free(ast);
    }
    else
    {
        printf("Erreur de parsing\n");
    }

    parser_free(p);
    lexer_free(lex);
    io_backend_close();

    return 0;
}
