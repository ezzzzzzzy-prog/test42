#include "debug.h"

#include <stdio.h>

#include "../ast/ast.h"

/* Affiche l'indentation pour la profondeur donnee */
static void indent(int depth)
{
    for (int i = 0; i < depth; i++)
        printf("  ");
}

/* Affiche un noeud commande */
static void dump_cmd(struct ast *ast, int depth)
{
    indent(depth);
    printf("CMD:");
    if (ast->data.cmd.words)
    {
        for (int i = 0; ast->data.cmd.words[i]; i++)
            printf(" [%s]", ast->data.cmd.words[i]);
    }
    else
        printf(" (assignment only)");
    printf("\n");
}

/* Affiche un noeud liste */
static void dump_list(struct ast *ast, int depth)
{
    indent(depth);
    printf("LIST (%zu)\n", ast->data.list.count);
    for (size_t i = 0; i < ast->data.list.count; i++)
        ast_dump(ast->data.list.commands[i], depth + 1);
}

/* Affiche un noeud pipeline */
static void dump_pipeline(struct ast *ast, int depth)
{
    indent(depth);
    printf("PIPELINE (%zu)\n", ast->data.pipeline.count);
    for (size_t i = 0; i < ast->data.pipeline.count; i++)
        ast_dump(ast->data.pipeline.cmds[i], depth + 1);
}

/* Affiche un noeud and/or */
static void dump_and_or(struct ast *ast, int depth, const char *op)
{
    indent(depth);
    printf("%s\n", op);
    ast_dump(ast->data.and_or.left, depth + 1);
    ast_dump(ast->data.and_or.right, depth + 1);
}

/* Retourne le symbole de redirection */
static const char *redir_sym(struct ast *ast)
{
    if (ast->data.redir.type == AST_REDIR_APP)
        return ">>";
    if (ast->data.redir.type == AST_REDIR_IN)
        return "<";
    return ">";
}

/* Affiche un noeud redirection */
static void dump_redir(struct ast *ast, int depth)
{
    indent(depth);
    const char *sym = redir_sym(ast);
    if (ast->data.redir.redir_nb != -1)
        printf("REDIR %d%s %s\n", ast->data.redir.redir_nb, sym,
               ast->data.redir.file);
    else
        printf("REDIR %s %s\n", sym, ast->data.redir.file);
    ast_dump(ast->data.redir.left, depth + 1);
}

/* Affiche recursivement l'AST pour le debug */
void ast_dump(struct ast *ast, int depth)
{
    if (!ast)
    {
        indent(depth);
        printf("(null)\n");
        return;
    }
    switch (ast->type)
    {
    case AST_COMMAND:
        dump_cmd(ast, depth);
        break;
    case AST_LIST:
        dump_list(ast, depth);
        break;
    case AST_PIPELINE:
        dump_pipeline(ast, depth);
        break;
    case AST_AND:
        dump_and_or(ast, depth, "AND");
        break;
    case AST_OR:
        dump_and_or(ast, depth, "OR");
        break;
    case AST_REDIRECTION:
        dump_redir(ast, depth);
        break;
    default:
        indent(depth);
        printf("UNKNOWN\n");
        break;
    }
}
