#include "ast_create.h"

#include <stdlib.h>

#include "ast.h"

// Init un node pour une cmd simple
struct ast *create_cmd(char **words)
{
    struct ast *ast = malloc(sizeof(*ast));
    if (!ast)
        return NULL;

    ast->type = AST_COMMAND;
    ast->data.cmd.words = words;

    return ast;
}

// Cree un node list pour stocker les suites de cmds (;)
struct ast *create_list(struct ast **cmds, size_t count)
{
    struct ast *ast = malloc(sizeof(*ast));
    if (!ast)
        return NULL;

    ast->type = AST_LIST;
    ast->data.list.commands = cmds;
    ast->data.list.count = count;
    ast->data.list.sep = ";";

    return ast;
}

// Alloc un node pipeline pour pipes
struct ast *ast_pipeline_create(struct ast **cmds, size_t count)
{
    struct ast *ast = calloc(1, sizeof(*ast));
    if (!ast)
        return NULL;

    ast->type = AST_PIPELINE;
    ast->data.pipeline.cmds = cmds;
    ast->data.pipeline.count = count;

    return ast;
}

// Cree un node pour ET
struct ast *create_and(struct ast *left, struct ast *right)
{
    struct ast *ast = malloc(sizeof(*ast));
    if (!ast)
        return NULL;

    ast->type = AST_AND;
    ast->data.and_or.left = left;
    ast->data.and_or.right = right;

    return ast;
}

// Cree un node pour OU
struct ast *create_or(struct ast *left, struct ast *right)
{
    struct ast *ast = malloc(sizeof(*ast));
    if (!ast)
        return NULL;

    ast->type = AST_OR;
    ast->data.and_or.left = left;
    ast->data.and_or.right = right;

    return ast;
}

// Init un node redir avec le type et le fichier
struct ast *create_redir(enum redir_type type, struct ast *left, char *file,
                         int redir_nb)
{
    struct ast *ast = malloc(sizeof(*ast));
    if (!ast)
        return NULL;

    ast->type = AST_REDIRECTION;
    ast->data.redir.type = type;
    ast->data.redir.left = left;
    ast->data.redir.file = file;
    ast->data.redir.redir_nb = redir_nb;

    return ast;
}
