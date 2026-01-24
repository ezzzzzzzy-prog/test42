#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/ast.h"

static struct ast *fake_cmd(void)
{
    struct ast_cmd *cmd = malloc(sizeof(*cmd));
    assert(cmd);

    cmd->base.type = AST_COMMAND;
    cmd->words = NULL;

    return (struct ast *)cmd;
}

int main(void)
{
    // if
    struct ast *cond = fake_cmd();
    struct ast *then_body = fake_cmd();
    struct ast *else_body = fake_cmd();

    struct ast *if_ast = create_if(cond, then_body, else_body);
    assert(if_ast);
    assert(if_ast->type == AST_IF);

    struct ast_if *if_node = (struct ast_if *)if_ast;
    assert(if_node->condition == cond);
    assert(if_node->then_body == then_body);
    assert(if_node->else_body == else_body);

    ast_free(if_ast);

    // while
    struct ast *while_cond = fake_cmd();
    struct ast *while_body = fake_cmd();

    struct ast *while_ast = create_while(while_cond, while_body);
    assert(while_ast);
    assert(while_ast->type == AST_WHILE);

    struct ast_while *w = (struct ast_while *)while_ast;
    assert(w->condition == while_cond);
    assert(w->body == while_body);

    ast_free(while_ast);

    // until
    struct ast *until_cond = fake_cmd();
    struct ast *until_body = fake_cmd();

    struct ast *until_ast = create_until(until_cond, until_body);
    assert(until_ast);
    assert(until_ast->type == AST_UNTIL);

    struct ast_until *u = (struct ast_until *)until_ast;
    assert(u->condition == until_cond);
    assert(u->body == until_body);

    ast_free(until_ast);

    // for
    char *var = strdup("i");

    char **words = malloc(sizeof(char *) * 2);
    words[0] = strdup("one");
    words[1] = NULL;

    struct ast *for_body = fake_cmd();

    struct ast *for_ast = create_for(var, words, for_body);
    assert(for_ast);
    assert(for_ast->type == AST_FOR);

    struct ast_for *f = (struct ast_for *)for_ast;
    assert(strcmp(f->var, "i") == 0);
    assert(strcmp(f->words[0], "one") == 0);
    assert(f->body == for_body);

    ast_free(for_ast);

    // Redir
    struct ast *redir_left = fake_cmd();
    char *file = strdup("out.txt");

    struct ast *redir_ast = create_redir(AST_REDIR_OUT, redir_left, file, -1);

    assert(redir_ast);
    assert(redir_ast->type == AST_REDIRECTION);

    struct ast_redirection *r = (struct ast_redirection *)redir_ast;
    assert(r->left == redir_left);
    assert(strcmp(r->file, "out.txt") == 0);

    ast_free(redir_ast);

    // subshell
    struct ast *sub_body = fake_cmd();

    struct ast *sub_ast = create_subshell(sub_body);
    assert(sub_ast);
    assert(sub_ast->type == AST_SUBSHELL);

    struct ast_subshell *s = (struct ast_subshell *)sub_ast;
    assert(s->body == sub_body);

    ast_free(sub_ast);

    return 0;
}
