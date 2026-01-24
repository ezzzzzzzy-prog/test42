#include <assert.h>
#include <string.h>

#include "../../src/io_backend.h"
#include "../../src/lexer.h"

int main(void)
{
    char *argv[] = { "42sh", "-c", "if echo hello; fi", NULL };
    int argc = 3;

    assert(io_backend_init(argc, argv) == 0);
    struct lexer *lex = new_lex();
    assert(lex);

    struct token *tok;

    // if
    tok = pop(lex);
    assert(tok);
    assert(tok->type == TOK_IF);
    free_tok(tok);

    // echo
    tok = pop(lex);
    assert(tok);
    assert(tok->type == TOK_WORD);
    assert(tok->val);
    assert(strcmp(tok->val, "echo") == 0);
    free_tok(tok);

    // hello
    tok = pop(lex);
    assert(tok);
    assert(tok->type == TOK_WORD);
    assert(tok->val);
    assert(strcmp(tok->val, "hello") == 0);
    free_tok(tok);

    // ;
    tok = pop(lex);
    assert(tok);
    assert(tok->type == TOK_SEMI);
    free_tok(tok);

    // fi
    tok = pop(lex);
    assert(tok);
    assert(tok->type == TOK_FI);
    free_tok(tok);

    // EOF
    tok = pop(lex);
    assert(tok);
    assert(tok->type == TOK_EOF);
    free_tok(tok);

    lexer_free(lex);
    io_backend_close();

    return 0;
}
