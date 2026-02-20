#include "ast.h"

#include <stdio.h>
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

static void free_ast_andor(struct ast_and_or *a)
{
    ast_free(a->left);
    ast_free(a->right);
}

static void free_ast_redir(struct ast_redirection *r)
{
    ast_free(r->left);
    free(r->file);
}

void ast_free(struct ast *ast)
{
    if (!ast)
        return;

    switch (ast->type)
    {
    case AST_COMMAND:
        free_ast_cmd(&ast->data.cmd);
        break;
    case AST_LIST:
        free_ast_list(&ast->data.list);
        break;
    case AST_PIPELINE:
        free_ast_pipeline(&ast->data.pipeline);
        break;
    case AST_AND:
    case AST_OR:
        free_ast_andor(&ast->data.and_or);
        break;
    case AST_REDIRECTION:
        free_ast_redir(&ast->data.redir);
        break;
    default:
        break;
    }

    free(ast);
}
