#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io_backend.h"

static char *my_strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (!copy)
        return NULL;
    memcpy(copy, s, len);
    return copy;
}

static char *append_char(char *buf, int *s, int *cap, char c)
{
    if (*s + 1 >= *cap)
    {
        *cap *= 2;
        buf = realloc(buf, *cap);
    }
    buf[(*s)++] = c;
    buf[*s] = '\0';
    return buf;
}
void free_tok(struct token *tok)
{
    if (!tok)
    {
        return;
    }
    free(tok->val);
    free(tok);
}
struct lexer *new_lex(void)
{
    struct lexer *lex = malloc(sizeof(*lex));
    if (!lex)
        return NULL;
    lex->curr_tok = NULL;
    return lex;
}

void lexer_free(struct lexer *lex)
{
    if (!lex)
    {
        return;
    }
    if (lex->curr_tok)
    {
        free_tok(lex->curr_tok);
    }
    free(lex);
}

static struct token *new_tok(enum type type, char *val)
{
    struct token *tok = malloc(sizeof(struct token));
    if (!tok)
    {
        return NULL;
    }
    tok->type = type;
    tok->val = val;
    return tok;
}
static struct token *build(void)
{
    int c = io_backend_next();

    while (c == ' ' || c == '\t')
        c = io_backend_next();

    if (c == EOF)
        return new_tok(TOK_EOF, NULL);
    if (c == '!')
        return new_tok(TOK_NOT, NULL);
    if (c == '\n')
        return new_tok(TOK_NEWLINE, NULL);

    if (c == ';')
        return new_tok(TOK_SEMI, NULL);

    if (c == '|')
    {
        if (io_backend_peek() == '|')
        {
            io_backend_next();
            return new_tok(TOK_OR, NULL);
        }
        return new_tok(TOK_IN, NULL);
    }
    if (c == '>')
    {
        if (io_backend_peek() == '>')
        {
            io_backend_next();
            return new_tok(TOK_REDIR_APP, NULL);
        }
        if (io_backend_peek() == '&')
        {
            io_backend_next();
            return new_tok(TOK_REDIR_DUP_OUT, NULL);
        }
        if (io_backend_peek() == '|')
        {
            io_backend_next();
            return new_tok(TOK_REDIR_FORC_OUT, NULL);
        }
        return new_tok(TOK_REDIR_OUT, NULL);
    }

    if (c == '<')
    {
        if (io_backend_peek() == '&')
        {
            io_backend_next();
            return new_tok(TOK_REDIR_DUP_IN, NULL);
        }
        if (io_backend_peek() == '>')
        {
            io_backend_next();
            return new_tok(TOK_REDIR_RW, NULL);
        }
        return new_tok(TOK_REDIR_IN, NULL);
    }

    if (c == '#')
    {
        while (c != EOF && c != '\n')
            c = io_backend_next();
        return new_tok(c == '\n' ? TOK_NEWLINE : TOK_EOF, NULL);
    }
    if (c == '\'')
    {
        int cap = 64;
        int s = 0;
        char *buf = malloc(cap);
        // int i = 0;
        while ((c = io_backend_next()) != EOF && c != '\'')
        {
            buf = append_char(buf, &s, &cap, c);
        }
        return new_tok(TOK_WORD, my_strdup(buf));
    }
    if (c == '&')
    {
        if (io_backend_peek() == '&')
        {
            io_backend_next();
            return new_tok(TOK_AND, NULL);
        }
        return new_tok(TOK_WORD, my_strdup("&"));
    }
    int cap = 64;
    int s = 0;
    char *buf = malloc(cap);
    while (c != EOF && !isspace(c) && c != ';' && c != '|' && c != '<'
           && c != '>' && c != '!')
    {
        buf = append_char(buf, &s, &cap, c);
        c = io_backend_peek();
        if (isspace(c) || c == ';' || c == '|' || c == '<' || c == '>'
            || c == '!')
            break;
        c = io_backend_next();
    }

    if (strcmp(buf, "if") == 0)
        return new_tok(TOK_IF, NULL);
    if (strcmp(buf, "then") == 0)
        return new_tok(TOK_THEN, NULL);
    if (strcmp(buf, "elif") == 0)
        return new_tok(TOK_ELIF, NULL);
    if (strcmp(buf, "else") == 0)
        return new_tok(TOK_ELSE, NULL);
    if (strcmp(buf, "fi") == 0)
        return new_tok(TOK_FI, NULL);
    if (strcmp(buf, "while") == 0)
        return new_tok(TOK_WHILE, NULL);
    if (strcmp(buf, "until") == 0)
        return new_tok(TOK_UNTIL, NULL);
    if (strcmp(buf, "for") == 0)
        return new_tok(TOK_FOR, NULL);
    if (strcmp(buf, "do") == 0)
        return new_tok(TOK_DO, NULL);
    if (strcmp(buf, "done") == 0)
        return new_tok(TOK_DONE, NULL);
    if (strcmp(buf, "in") == 0)
        return new_tok(TOK_IN, NULL);

    return new_tok(TOK_WORD, my_strdup(buf));
}

struct token *peek(struct lexer *lex)
{
    if (!lex)
    {
        return NULL;
    }
    if (!lex->curr_tok)
    {
        lex->curr_tok = build();
    }
    return lex->curr_tok;
}

struct token *pop(struct lexer *lex)
{
    struct token *tok;

    if (!lex)
        return NULL;

    tok = peek(lex);
    lex->curr_tok = NULL;
    return tok;
}
