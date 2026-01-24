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

static int skip(void)
{
    int c = io_backend_next();

    while (c == ' ' || c == '\t')
        c = io_backend_next();
    return c;
}

static struct token *simple_tok(int c)
{
    if (c == EOF)
        return new_tok(TOK_EOF, NULL);
    if (c == '!')
        return new_tok(TOK_NOT, NULL);
    if (c == '\n')
        return new_tok(TOK_NEWLINE, NULL);

    if (c == ';')
        return new_tok(TOK_SEMI, NULL);
    return NULL;
}

static struct token *pipe_tok(void)
{
    if (io_backend_peek() == '|')
    {
        io_backend_next();
        return new_tok(TOK_OR, NULL);
    }
    return new_tok(TOK_PIPE, NULL);
}

static struct token *redir_out_tok(void)
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
static struct token *redir_in_tok(void)
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

static struct token *comment_tok(int c)
{
    while (c != EOF && c != '\n')
        c = io_backend_next();
    return new_tok(c == '\n' ? TOK_NEWLINE : TOK_EOF, NULL);
}

static struct token *word_tok(char *buf)
{
    if (strcmp(buf, "if") == 0)
    {
        free(buf);
        return new_tok(TOK_IF, NULL);
    }
    if (strcmp(buf, "then") == 0)
    {
        free(buf);
        return new_tok(TOK_THEN, NULL);
    }
    if (strcmp(buf, "elif") == 0)
    {
        free(buf);
        return new_tok(TOK_ELIF, NULL);
    }
    if (strcmp(buf, "else") == 0)
    {
        free(buf);
        return new_tok(TOK_ELSE, NULL);
    }
    if (strcmp(buf, "fi") == 0)
    {
        free(buf);
        return new_tok(TOK_FI, NULL);
    }
    if (strcmp(buf, "while") == 0)
    {
        free(buf);
        return new_tok(TOK_WHILE, NULL);
    }
    if (strcmp(buf, "until") == 0)
    {
        free(buf);
        return new_tok(TOK_UNTIL, NULL);
    }
    if (strcmp(buf, "for") == 0)
    {
        free(buf);
        return new_tok(TOK_FOR, NULL);
    }
    if (strcmp(buf, "do") == 0)
    {
        free(buf);
        return new_tok(TOK_DO, NULL);
    }
    if (strcmp(buf, "done") == 0)
    {
        free(buf);
        return new_tok(TOK_DONE, NULL);
    }
    if (strcmp(buf, "in") == 0)
    {
        free(buf);
        return new_tok(TOK_IN, NULL);
    }
    struct token *tok = new_tok(TOK_WORD, my_strdup(buf));
    free(buf);
    return tok;
}
static int double_quote(char **buf, int *s, int *cap)
{
    int c = io_backend_next(); 

    while (c != EOF)
    {
        if (c == '\\')
        {
            int n = io_backend_next();

            if (n == '"' || n == '\\' || n == '$' || n == '`')
                *buf = append_char(*buf, s, cap, n);
            else if (n == '\n')
            {
                // IGNORE
            }
            else if ( n == ' ')
            {
                *buf = append_char(*buf, s, cap, '\\'); 
                *buf = append_char(*buf, s, cap, ' '); 
            }
            else
                *buf = append_char(*buf, s, cap, n);
        }
        else if (c == '"')
            return io_backend_next();
        else
            *buf = append_char(*buf, s, cap, c);
        c = io_backend_next();
    }
    return c;
}
static int single_quote(char **buf, int *s, int *cap)
{
    int c = io_backend_next();
    while (c != EOF && c != '\'')
    {
        *buf = append_char(*buf, s, cap, c);
        c = io_backend_next();
    }
    return io_backend_next();
}
static void backslash(char **buf, int *s, int *cap)
{
    int n = io_backend_next();
    if (n == '\n')
        return;
    *buf = append_char(*buf, s, cap, n);
}
static struct token *read_tok(int c)
{
    int cap = 64;
    int s = 0;
    char *buf = malloc(cap);
    buf[0] = '\0';
    while (c != EOF && !isspace(c) && c != ';' && c != '|' && c != '<'
           && c != '>' && c != '!')
    {
        if (c == '"')
        {
            c = double_quote(&buf, &s, &cap);
            continue;
        }
        else if (c == '\'')
        {
            c = single_quote(&buf, &s, &cap);
            continue;
        }
        else if (c == '\\')
            backslash(&buf, &s, &cap);
        else
            buf = append_char(buf, &s, &cap, c);

        c = io_backend_peek();
        if (c == EOF || isspace(c) || c == ';' || c == '|' || c == '<'
            || c == '>' || c == '!')
            break;

        c = io_backend_next();
    }

    return word_tok(buf);
}

static struct token *redir_nb(int c)
{
    int n = c;
    char buf[32];
    int i = 0;

    buf[i++] = n;
    while (isdigit(io_backend_peek()))
    {
        buf[i++] = io_backend_next();
    }
    buf[i] = '\0';

    if (io_backend_peek() == '<' || io_backend_peek() == '>')
    {
        return new_tok(TOK_REDIR_NB, my_strdup(buf));
    }

    return word_tok(my_strdup(buf));
}
static struct token *build(void)
{
    int c = skip();
    struct token *tok;

    if ((tok = simple_tok(c)))
        return tok;

    if (isdigit(c))
        return redir_nb(c);

    if (c == '|')
        return pipe_tok();
    if (c == '>')
        return redir_out_tok();

    if (c == '<')
        return redir_in_tok();
    if (c == '#')
        return comment_tok(c);
    if (c == '&')
    {
        if (io_backend_peek() == '&')
        {
            io_backend_next();
            return new_tok(TOK_AND, NULL);
        }
        return new_tok(TOK_WORD, my_strdup("&"));
    }
    if (c == '(')
        return new_tok(TOK_SUB_LP, NULL);
    if (c == ')')
        return new_tok(TOK_SUB_RP, NULL);
    if (c == '{')
        return new_tok(TOK_SUB_LB, NULL);
    if (c == '}')
        return new_tok(TOK_SUB_RB, NULL);

    return read_tok(c);
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
