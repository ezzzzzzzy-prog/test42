#include "ast.h"
#include <stdlib.h>


void ast_free(struct ast *ast)
{
    if (!ast)
        return;

    switch (ast->type)
    {
    case AST_COMMAND: {
        struct ast_cmd *c = (struct ast_cmd *)ast;
        if (c->words)
        {
            for (size_t i = 0; c->words[i]; i++)
                free(c->words[i]);
            free(c->words);
        }
        break;
    }

    case AST_LIST: {
        struct ast_list *l = (struct ast_list *)ast;
        for (size_t i = 0; i < l->count; i++)
            ast_free(l->commands[i]);
        free(l->commands);
        break;
    }

    case AST_PIPELINE: {
        struct ast_pipeline *p = (struct ast_pipeline *)ast;
        for (size_t i = 0; i < p->count; i++)
            ast_free(p->cmds[i]);
        free(p->cmds);
        break;
    }

    case AST_IF: {
        struct ast_if *i = (struct ast_if *)ast;
        ast_free(i->condition);
        ast_free(i->then_body);
        ast_free(i->else_body);
        break;
    }

    case AST_WHILE: {
        struct ast_while *w = (struct ast_while *)ast;
        ast_free(w->condition);
        ast_free(w->body);
        break;
    }

    case AST_UNTIL: {
        struct ast_until *u = (struct ast_until *)ast;
        ast_free(u->condition);
        ast_free(u->body);
        break;
    }

    case AST_FOR: {
        struct ast_for *f = (struct ast_for *)ast;
        free(f->var);
        if (f->words)
        {
            for (size_t i = 0; f->words[i]; i++)
                free(f->words[i]);
            free(f->words);
        }
        ast_free(f->body);
        break;
    }

    case AST_AND:
    case AST_OR: {
        struct ast_and_or *a = (struct ast_and_or *)ast;
        ast_free(a->left);
        ast_free(a->right);
        break;
    }

    case AST_NEGATION: {
        struct ast_negation *n = (struct ast_negation *)ast;
        ast_free(n->child);
        break;
    }

    case AST_REDIRECTION: {
        struct ast_redirection *r = (struct ast_redirection *)ast;
        ast_free(r->left);
        free(r->file);
        break;
    }

    case AST_SUBSHELL: {
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
