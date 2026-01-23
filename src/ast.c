#include "ast.h"

#include "stdio.h"
#include "stdlib.h"

struct ast *create_cmd(char **words)
{
    struct ast_cmd *cmd = malloc(sizeof(struct ast_cmd));
    if (!cmd)
    {
        return NULL;
    }
    cmd->base.type = AST_COMMAND;
    cmd->words = words;
    return (struct ast *)cmd;
}

struct ast *create_list(struct ast **cmd, size_t count)
{
    struct ast_list *list = malloc(sizeof(struct ast_list));
    if (!list)
    {
        return NULL;
    }
    list->base.type = AST_LIST;
    list->commands = cmd;
    list->count = count;
    list->sep = ";";
    return (struct ast *)list;
}

struct ast *create_if(struct ast *cond, struct ast *then_body,
                      struct ast *else_body)
{
    struct ast_if *res = malloc(sizeof(struct ast_if));
    if (!res)
    {
        return NULL;
    }
    res->base.type = AST_IF;
    res->condition = cond;
    res->then_body = then_body;
    res->else_body = else_body;
    return (struct ast *)res;
}

void ast_free(struct ast *ast)
{
    if (!ast)
    {
        return;
    }
    switch (ast->type)
    {
    case AST_COMMAND: {
        struct ast_cmd *cmd = (struct ast_cmd *)ast;
        if (cmd->words)
        {
            for (size_t i = 0; cmd->words[i] != NULL; i++)
            {
                free(cmd->words[i]);
            }
            free(cmd->words);
        }
        break;
    }
    case AST_LIST: {
        struct ast_list *list = (struct ast_list *)ast;
        for (size_t i = 0; i < list->count; i++)
        {
            ast_free(list->commands[i]);
        }
        free(list->commands);
        break;
    }
    case AST_IF: {
        struct ast_if *a_if = (struct ast_if *)ast;
        ast_free(a_if->condition);
        ast_free(a_if->then_body);
        ast_free(a_if->else_body);
        break
    }
    default:
        break;
    }
    free(ast);
}
