#include <assert.h>
#include <string.h>

#include "../../src/io_backend/io_backend.h"
#include "../../src/lexer/lexer.h"

int main(void)
{
    char *argv[] = { "minishell", "-c", "echo hello; fi", NULL };
    int argc = 3;

    assert(io_backend_init(argc, argv) == 0);
    struct lexer *lex = new_lex();
    assert(lex);

    struct token *tok;

    tok = pop(lex);
    assert(tok && tok->type == TOK_WORD && strcmp(tok->val, "echo") == 0);
    free_tok(tok);

    tok = pop(lex);
    assert(tok && tok->type == TOK_WORD && strcmp(tok->val, "hello") == 0);
    free_tok(tok);

    tok = pop(lex);
    assert(tok && tok->type == TOK_SEMI);
    free_tok(tok);

    tok = pop(lex);
    assert(tok && tok->type == TOK_WORD);
    free_tok(tok);

    tok = pop(lex);
    assert(tok && tok->type == TOK_EOF);
    free_tok(tok);

    lexer_free(lex);
    io_backend_close();

    return 0;
}
