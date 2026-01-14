#include <stdlib.h>
#include "ast.h"

struct ast *create_cmd(char **words)
{
    struct ast_cmd *cmd = malloc(sizeof(*cmd));
    if (!cmd)
        return NULL;

    cmd->base.type = AST_COMMAND;
    cmd->words = words;
    return (struct ast *)cmd;
}

struct ast *create_list(struct ast **cmds, size_t count)
{
    struct ast_list *list = malloc(sizeof(*list));
    if (!list)
        return NULL;

    list->base.type = AST_LIST;
    list->commands = cmds;
    list->count = count;
    list->sep = ";";
    return (struct ast *)list;
}

struct ast *create_if(struct ast *cond,
                      struct ast *then_body,
                      struct ast *else_body)
{
    struct ast_if *node = malloc(sizeof(*node));
    if (!node)
        return NULL;

    node->base.type = AST_IF;
    node->condition = cond;
    node->then_body = then_body;
    node->else_body = else_body;
    return (struct ast *)node;
}
struct ast *ast_pipeline_create(struct ast **cmds, size_t count)
{
    struct ast_pipeline *p = calloc(1, sizeof(*p));
    if (!p)
        return NULL;

    p->base.type = AST_PIPELINE;
    p->cmds = cmds;
    p->count = count;
    return &p->base;
}
struct ast *create_negation(struct ast *child)
{
    struct ast_negation *n = malloc(sizeof(*n));
    if (!n)
        return NULL;

    n->base.type = AST_NEGATION;
    n->child = child;
    return (struct ast *)n;
}
struct ast *create_and(struct ast *left, struct ast *right)
{
    struct ast_and_or *and = malloc(sizeof(*and));
    if (!and)
        return NULL;

    and->base.type = AST_AND;
    and->left = left;
    and->right = right;
    return (struct ast *)and;
}

struct ast *create_or(struct ast *left, struct ast *right)
{
    struct ast_and_or *or = malloc(sizeof(*or));
    if (!or)
        return NULL;

    or->base.type = AST_OR;
    or->left = left;
    or->right = right;
    return (struct ast *)or;
}

struct ast *create_redir(struct ast *left, struct ast *right)
{
    struct ast_redirection *redir = malloc(sizeof(*redir));
    if (!redir)
        return NULL;

    redir->base.type = AST_REDIRECTION;
    redir->left = left;
    redir->right = right;
    return (struct ast *)redir;
}


void ast_free(struct ast *ast)
{
    if (!ast)
        return;

    switch (ast->type)
    {
    case AST_COMMAND:
    {
        struct ast_cmd *cmd = (struct ast_cmd *)ast;
        for (size_t i = 0; cmd->words && cmd->words[i]; i++)
            free(cmd->words[i]);
        free(cmd->words);
        break;
    }

    case AST_LIST:
    {
        struct ast_list *list = (struct ast_list *)ast;
        for (size_t i = 0; i < list->count; i++)
            ast_free(list->commands[i]);
        free(list->commands);
        break;
    }

    case AST_IF:
    {
        struct ast_if *ifn = (struct ast_if *)ast;
        ast_free(ifn->condition);
        ast_free(ifn->then_body);
        ast_free(ifn->else_body);
        break;
    }
    case AST_PIPELINE:
        {
            struct ast_pipeline *p = (struct ast_pipeline *)ast;
            for (size_t i = 0; i < p->count; i++)
                ast_free(p->cmds[i]);
            free(p->cmds);
            break;
        }

    case AST_NEGATION:
    {
        struct ast_negation *n = (struct ast_negation *)ast;
        ast_free(n->child);
        break;
    }
    case AST_AND:
    case AST_OR:
    {
        struct ast_and_or *and_or = (struct ast_and_or *)ast;
        ast_free(and_or->left);
        ast_free(and_or->right);
        break;
    }
    case AST_REDIRECTION:
    {
        struct ast_redirection *redir = (struct ast_redirection *)ast;
        ast_free(redir->left);
        ast_free(redir->right);
        break;
    }
    }
    free(ast);
}

