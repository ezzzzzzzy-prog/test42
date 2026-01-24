#include "ast.h"

#include <stdlib.h>

static void free_ast_cmd(struct ast_cmd *c)
{
    if (c->words)
    {
        for (size_t i = 0; c->words[i]; i++)
            free(c->words[i]);
        free(c->words);
    }
}

static void free_ast_list(struct ast_list *l)
{
    for (size_t i = 0; i < l->count; i++)
        ast_free(l->commands[i]);
    free(l->commands);
}

static void free_ast_pipeline(struct ast_pipeline *p)
{
    for (size_t i = 0; i < p->count; i++)
        ast_free(p->cmds[i]);
    free(p->cmds);
}

static void free_ast_if(struct ast_if *i)
{
    ast_free(i->condition);
    ast_free(i->then_body);
    ast_free(i->else_body);
}

static void free_ast_while(struct ast_while *w)
{
    ast_free(w->condition);
    ast_free(w->body);
}

static void free_ast_until(struct ast_until *u)
{
    ast_free(u->condition);
    ast_free(u->body);
}

static void free_ast_for(struct ast_for *f)
{
    free(f->var);
    if (f->words)
    {
        for (size_t i = 0; f->words[i]; i++)
            free(f->words[i]);
        free(f->words);
    }
    ast_free(f->body);
}

void ast_free(struct ast *ast)
{
    if (!ast)
        return;

    switch (ast->type)
    {
    case AST_COMMAND:
        free_ast_cmd((struct ast_cmd *)ast);
        break;
    case AST_LIST:
        free_ast_list((struct ast_list *)ast);
        break;
    case AST_PIPELINE:
        free_ast_pipeline((struct ast_pipeline *)ast);
        break;
    case AST_IF:
        free_ast_if((struct ast_if *)ast);
        break;
    case AST_WHILE:
        free_ast_while((struct ast_while *)ast);
        break;
    case AST_UNTIL:
        free_ast_until((struct ast_until *)ast);
        break;
    case AST_FOR:
        free_ast_for((struct ast_for *)ast);
        break;
    case AST_AND:
    case AST_OR:
    {
        struct ast_and_or *a = (struct ast_and_or *)ast;
        ast_free(a->left);
        ast_free(a->right);
        break;
    }
    case AST_NEGATION:
    {
        struct ast_negation *n = (struct ast_negation *)ast;
        ast_free(n->child);
        break;
    }
    case AST_REDIRECTION:
    {
        struct ast_redirection *r = (struct ast_redirection *)ast;
        ast_free(r->left);
        free(r->file);
        break;
    }
    case AST_SUBSHELL:
    {
        struct ast_subshell *s = (struct ast_subshell *)ast;
        ast_free(s->body);
        break;
    }
    case AST_BREAK:
    case AST_CONTINUE:
        break;
    }
    free(ast);
}
