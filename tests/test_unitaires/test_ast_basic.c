#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/ast.h"

static struct ast *fake(void)
{
    struct ast_cmd *cmd = malloc(sizeof(*cmd));
    assert(cmd);

    cmd->base.type = AST_COMMAND;
    cmd->words = NULL;

    return (struct ast *)cmd;
}

int main(void)
{
    // create_cmd
    char **words = malloc(sizeof(char *) * 2);
    words[0] = strdup("echo");
    words[1] = NULL;

    struct ast *cmd_ast = create_cmd(words);
    assert(cmd_ast);
    assert(cmd_ast->type == AST_COMMAND);

    ast_free(cmd_ast);

    // create_list
    struct ast **cmds = malloc(sizeof(struct ast *) * 2);
    cmds[0] = fake();
    cmds[1] = fake();

    struct ast *list_ast = create_list(cmds, 2);
    assert(list_ast);
    assert(list_ast->type == AST_LIST);

    ast_free(list_ast);

    // ast_pipeline_create
    struct ast **pipes = malloc(sizeof(struct ast *) * 2);
    pipes[0] = fake();
    pipes[1] = fake();

    struct ast *pipe_ast = ast_pipeline_create(pipes, 2);
    assert(pipe_ast);
    assert(pipe_ast->type == AST_PIPELINE);

    ast_free(pipe_ast);

    // create_negation
    struct ast *neg_ast = create_negation(fake());
    assert(neg_ast);
    assert(neg_ast->type == AST_NEGATION);

    ast_free(neg_ast);

    // create_and
    struct ast *and_ast = create_and(fake(), fake());
    assert(and_ast);
    assert(and_ast->type == AST_AND);

    ast_free(and_ast);

    // create_or
    struct ast *or_ast = create_or(fake(), fake());
    assert(or_ast);
    assert(or_ast->type == AST_OR);

    ast_free(or_ast);

    // create_break
    struct ast *brk = create_break();
    assert(brk);
    assert(brk->type == AST_BREAK);

    ast_free(brk);

    // create_continue
    struct ast *cont = create_continue();
    assert(cont);
    assert(cont->type == AST_CONTINUE);

    ast_free(cont);

    return 0;
}
