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

struct ast *create_if(struct ast *cond, struct ast *then_body,
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
    return (struct ast *) and;
}
struct ast *create_while(struct ast *cond, struct ast *body)
{
    struct ast_while *w = malloc(sizeof(*w));
    if (!w)
    {
        return NULL;
    }
    w->base.type = AST_WHILE;
    w->condition = cond;
    w->body = body;
    return (struct ast *)w;
}

struct ast *create_until(struct ast *cond, struct ast *body)
{
    struct ast_until *un = malloc(sizeof(*un));
    if (!un)
    {
        return NULL;
    }
    un->base.type = AST_UNTIL;
    un->condition = cond;
    un->body = body;
    return (struct ast *)un;
}

struct ast *create_for(char *var, char **words, struct ast *body)
{
    struct ast_for *fo = malloc(sizeof(*fo));
    if (!fo)
    {
        return NULL;
    }
    fo->base.type = AST_FOR;
    fo->var = var;
    fo->words = words;
    fo->body = body;
    return (struct ast *)fo;
}

struct ast *create_or(struct ast *left, struct ast *right)
{
    struct ast_and_or * or = malloc(sizeof(* or));
    if (! or)
        return NULL;

    or->base.type = AST_OR;
    or->left = left;
    or->right = right;
    return (struct ast *) or ;
}
struct ast *create_redir(enum redir_type type, struct ast *left, char *file,
                         int redir_nb)
{
    struct ast_redirection *redir = malloc(sizeof(*redir));
    if (!redir)
        return NULL;

    redir->base.type = AST_REDIRECTION;
    redir->type = type;
    redir->left = left;
    redir->file = file;
    redir->redir_nb = redir_nb;
    return (struct ast *)redir;
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

static void free_words(char **words)
{
    if (!words)
        return;
    for (size_t i = 0; words[i]; i++)
        free(words[i]);
    free(words);
}

static void free_cmd(struct ast *ast)
{
    struct ast_cmd *cmd = (struct ast_cmd *)ast;
    free_words(cmd->words);
}

static void free_while_until(struct ast *ast, int i)
{
    if (i)
    {
        struct ast_while *wh = (struct ast_while *)ast;
        ast_free(wh->condition);
        ast_free(wh->body);
    }
    else
    {
        struct ast_until *un = (struct ast_until *)ast;
        ast_free(un->condition);
        ast_free(un->body);
    }
}

static void free_for(struct ast *ast)
{
    struct ast_for *fo = (struct ast_for *)ast;
    free(fo->var);
    free_words(fo->words);
    ast_free(fo->body);
}

static void free_list(struct ast *ast)
{
    struct ast_list *list = (struct ast_list *)ast;
    for (size_t i = 0; i < list->count; i++)
        ast_free(list->commands[i]);
    free(list->commands);
}

static void free_if(struct ast *ast)
{
    struct ast_if *ifn = (struct ast_if *)ast;
    ast_free(ifn->condition);
    ast_free(ifn->then_body);
    ast_free(ifn->else_body);
}

static void free_pipeline(struct ast *ast)
{
    struct ast_pipeline *p = (struct ast_pipeline *)ast;
    for (size_t i = 0; i < p->count; i++)
        ast_free(p->cmds[i]);
    free(p->cmds);
}

static void free_negation(struct ast *ast)
{
    struct ast_negation *n = (struct ast_negation *)ast;
    ast_free(n->child);
}

static void free_and_or(struct ast *ast)
{
    struct ast_and_or *and_or = (struct ast_and_or *)ast;
    ast_free(and_or->left);
    ast_free(and_or->right);
}

static void free_redirection(struct ast *ast)
{
    struct ast_redirection *redir = (struct ast_redirection *)ast;
    free(redir->file);
    ast_free(redir->left);
}
static void free_subshell(struct ast *ast)
{
    struct ast_subshell *s = (struct ast_subshell *)ast;
    ast_free(s->body);
}

void ast_free(struct ast *ast)
{
    if (!ast)
        return;

    switch (ast->type)
    {
    case AST_COMMAND: {
        free_cmd(ast);
        break;
    }
    case AST_WHILE: {
        free_while_until(ast, 1);
        break;
    }
    case AST_UNTIL: {
        free_while_until(ast, 0);
        break;
    }
    case AST_FOR: {
        free_for(ast);
        break;
    }
    case AST_LIST: {
        free_list(ast);
        break;
    }

    case AST_IF: {
        free_if(ast);
        break;
    }
    case AST_PIPELINE: {
        free_pipeline(ast);
        break;
    }

    case AST_NEGATION: {
        free_negation(ast);
        break;
    }
    case AST_AND: {
        free_and_or(ast);
        break;
    }

    case AST_OR: {
        free_and_or(ast);
        break;
    }
    case AST_REDIRECTION: {
        free_redirection(ast);
        break;
    }
    case AST_SUBSHELL: {
        free_subshell(ast);
        break;
    }
    }
    free(ast);
}
