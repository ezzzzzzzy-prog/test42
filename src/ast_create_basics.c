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
    struct ast_negation *n = malloc(sizeof(*n));
    if (!n)
        return NULL;

    n->base.type = AST_NEGATION;
    n->child = child;
    return (struct ast *)n;
}

struct ast *create_and(struct ast *left, struct ast *right)
{
    struct ast_and_or *a = malloc(sizeof(*a));
    if (!a)
        return NULL;

    a->base.type = AST_AND;
    a->left = left;
    a->right = right;
    return (struct ast *)a;
}

struct ast *create_or(struct ast *left, struct ast *right)
{
    struct ast_and_or *o = malloc(sizeof(*o));
    if (!o)
        return NULL;

    o->base.type = AST_OR;
    o->left = left;
    o->right = right;
    return (struct ast *)o;
}

struct ast *create_break(void)
{
    struct ast *b = malloc(sizeof(*b));
    if (!b)
        return NULL;

    b->type = AST_BREAK;
    return b;
}

struct ast *create_continue(void)
{
    struct ast *c = malloc(sizeof(*c));
    if (!c)
        return NULL;

    c->type = AST_CONTINUE;
    return c;
}
