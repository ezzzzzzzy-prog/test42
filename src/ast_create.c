#include <stdlib.h>

#include "ast.h"

struct ast *create_if(struct ast *cond, struct ast *then_body,
                      struct ast *else_body)
{
    struct ast_if *n = malloc(sizeof(*n));
    if (!n)
        return NULL;

    n->base.type = AST_IF;
    n->condition = cond;
    n->then_body = then_body;
    n->else_body = else_body;

    return (struct ast *)n;
}

struct ast *create_while(struct ast *cond, struct ast *body)
{
    struct ast_while *w = malloc(sizeof(*w));
    if (!w)
        return NULL;

    w->base.type = AST_WHILE;
    w->condition = cond;
    w->body = body;
    return (struct ast *)w;
}

struct ast *create_until(struct ast *cond, struct ast *body)
{
    struct ast_until *u = malloc(sizeof(*u));
    if (!u)
        return NULL;

    u->base.type = AST_UNTIL;

    u->condition = cond;
    u->body = body;

    return (struct ast *)u;
}

struct ast *create_for(char *var, char **words, struct ast *body)
{
    struct ast_for *f = malloc(sizeof(*f));
    if (!f)
        return NULL;

    f->base.type = AST_FOR;
    f->var = var;
    f->words = words;

    f->body = body;
    return (struct ast *)f;
}

struct ast *create_redir(enum redir_type type, struct ast *left, char *file,
                         int redir_nb)
{
    struct ast_redirection *r = malloc(sizeof(*r));

    if (!r)
        return NULL;

    r->base.type = AST_REDIRECTION;
    r->type = type;
    r->left = left;
    r->file = file;
    r->redir_nb = redir_nb;

    return (struct ast *)r;
}

struct ast *create_subshell(struct ast *body)
{
    struct ast_subshell *s = malloc(sizeof(*s));
    if (!s)
        return NULL;

    s->base.type = AST_SUBSHELL;
    s->body = body;
    return (struct ast *)s;
}
