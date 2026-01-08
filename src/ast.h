#ifndef AST_H
#define AST_H

#include <stddef.h>

enum ast_type
{
    AST_COMMANDE,
    AST_IF,
    AST_PIPES,
    AST_REDIRECTIONS,
    AST_LISTCOMMANDES
};

struct ast
{
    enum ast_type type;
};

struct ast_commande
{
    struct ast base;
    char **argv;
};

struct ast_if
{
    struct ast base;
    struct ast *condition;
    struct ast *then_body;
    struct ast *else_body;
};

struct ast_listcommandes
{
    struct ast base;
    struct ast **commands;
    size_t count;
};

struct ast_command *ast_command_new(char **argv);

struct ast_if *ast_if_new(struct ast *condition, struct ast *then_body, struct ast *else_body);

struct ast_listcommandes *ast_listcommandes_new(void);

void ast_listcommandes_add(struct ast_listcommandes *list, struct ast *cmd);

void ast_free(struct ast *ast);

#endif /* AST_H */
