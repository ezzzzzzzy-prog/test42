#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/ast/ast.h"
#include "../../src/ast/ast_create.h"

int main(void)
{
    /* create_cmd */
    char **words = malloc(sizeof(char *) * 2);
    words[0] = strdup("echo");
    words[1] = NULL;
    struct ast *cmd_ast = create_cmd(words);
    assert(cmd_ast && cmd_ast->type == AST_COMMAND);
    ast_free(cmd_ast);

    /* create_list */
    char **w1 = malloc(sizeof(char *) * 2);
    w1[0] = strdup("ls");
    w1[1] = NULL;
    char **w2 = malloc(sizeof(char *) * 2);
    w2[0] = strdup("cat");
    w2[1] = NULL;

    struct ast **cmds = malloc(sizeof(struct ast *) * 2);
    cmds[0] = create_cmd(w1);
    cmds[1] = create_cmd(w2);
    struct ast *list_ast = create_list(cmds, 2);
    assert(list_ast && list_ast->type == AST_LIST);
    ast_free(list_ast);

    /* ast_pipeline_create */
    char **w3 = malloc(sizeof(char *) * 2);
    w3[0] = strdup("echo");
    w3[1] = NULL;
    char **w4 = malloc(sizeof(char *) * 2);
    w4[0] = strdup("grep");
    w4[1] = NULL;

    struct ast **pipes = malloc(sizeof(struct ast *) * 2);
    pipes[0] = create_cmd(w3);
    pipes[1] = create_cmd(w4);
    struct ast *pipe_ast = ast_pipeline_create(pipes, 2);
    assert(pipe_ast && pipe_ast->type == AST_PIPELINE);
    ast_free(pipe_ast);

    /* create_and */
    char **w5 = malloc(sizeof(char *) * 2);
    w5[0] = strdup("true");
    w5[1] = NULL;
    char **w6 = malloc(sizeof(char *) * 2);
    w6[0] = strdup("echo");
    w6[1] = NULL;

    struct ast *and_ast = create_and(create_cmd(w5), create_cmd(w6));
    assert(and_ast && and_ast->type == AST_AND);
    ast_free(and_ast);

    /* create_or */
    char **w7 = malloc(sizeof(char *) * 2);
    w7[0] = strdup("false");
    w7[1] = NULL;
    char **w8 = malloc(sizeof(char *) * 2);
    w8[0] = strdup("echo");
    w8[1] = NULL;

    struct ast *or_ast = create_or(create_cmd(w7), create_cmd(w8));
    assert(or_ast && or_ast->type == AST_OR);
    ast_free(or_ast);

    return 0;
}
