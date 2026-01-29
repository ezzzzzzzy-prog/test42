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

static void free_ast_andor(struct ast_and_or *a)
{
    ast_free(a->left);
    ast_free(a->right);
}

static void free_ast_negation(struct ast_negation *n)
{
    ast_free(n->child);
}

static void free_ast_redir(struct ast_redirection *r)
{
    ast_free(r->left);
    free(r->file);
}

static void free_ast_subshell(struct ast_subshell *s)
{
    ast_free(s->body);
}

static void free_ast_function(struct ast_function *f)
{
	free(f->name);
	ast_free(f->body);
}

static void ast_free_switch1(struct ast *ast)
{
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
    default:
        return;
    }
}

static void ast_free_switch2(struct ast *ast)
{
    switch (ast->type)
    {
    case AST_AND:
    case AST_OR:
        free_ast_andor((struct ast_and_or *)ast);
        break;
    case AST_NEGATION:
        free_ast_negation((struct ast_negation *)ast);
        break;
    case AST_REDIRECTION:
        free_ast_redir((struct ast_redirection *)ast);
        break;
    case AST_SUBSHELL:
        free_ast_subshell((struct ast_subshell *)ast);
        break;
    case AST_FUNCTION:
    	free_ast_function((struct ast_function *)ast);
	break;
    case AST_BREAK:
    case AST_CONTINUE:
        break;
    default:
        ast_free_switch1(ast);
        return;
    }
}

void ast_free(struct ast *ast)
{
    if (!ast)
        return;
    ast_free_switch2(ast);
    free(ast);
}
