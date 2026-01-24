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

struct ast *ast_pipeline_create(struct ast **cmds, size_t count)
{
    struct ast_pipeline *p = calloc(1, sizeof(*p));
    if (!p)
        return NULL;

    p->base.type = AST_PIPELINE;
    p->cmds = cmds;

    p->count = count;

    return (struct ast *)p;
}

struct ast *create_negation(struct ast *child)
{
    struct ast_negation *nope = malloc(sizeof(*nope));
    if (!nope)
        return NULL;

    nope->base.type = AST_NEGATION;
    nope->child = child;

    return (struct ast *)nope;
}

struct ast *create_and(struct ast *left, struct ast *right)
{
    struct ast_and_or *and = malloc(sizeof(*and));
    if (!and)
        return NULL;

    and->base.type = AST_AND;
    and->left = left;

    and->right = right;

    return (struct ast *) and;
}

struct ast *create_or(struct ast *left, struct ast *right)
{
    struct ast_and_or *and_or = malloc(sizeof(*and_or));
    if (!and_or)
        return NULL;

    and_or->base.type = AST_OR;
    and_or->left = left;
    and_or->right = right;
    return (struct ast *)and_or;
}

struct ast *create_break(void)
{
    struct ast *br = malloc(sizeof(*br));
    if (!br)
        return NULL;

    br->type = AST_BREAK;

    return br;
}

struct ast *create_continue(void)
{
    struct ast *cont = malloc(sizeof(*cont));
    if (!cont)
        return NULL;

    cont->type = AST_CONTINUE;
    return cont;
}
