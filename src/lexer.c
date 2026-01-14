#include "lexer.h"
#include "io_backend.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char *my_strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (!copy)
        return NULL;
    memcpy(copy, s, len);
    return copy;
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
    if(!lex)
    {
        return;
    }
    if(lex->curr_tok)
    {
        free_tok(lex->curr_tok);
    }
    free(lex);
}

static struct token *new_tok(enum type type, char *val)
{
    struct token *tok = malloc(sizeof(struct token));
    if(!tok)
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
    if(c == '!')
        return new_tok(TOK_NOT,NULL);
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
        return new_tok(TOK_PIPE, NULL);
    }
    if (c == '>')
    {
        int next = io_backend_peek();
        if (next == '>')
        {
            io_backend_next();
            return new_tok(TOK_DGT, NULL);
        }
        return new_tok(TOK_GT, NULL);
    }

    if (c == '<')
        return new_tok(TOK_LT, NULL);

    if (c == '#')
    {
        while (c != EOF && c != '\n')
            c = io_backend_next();
        return new_tok(c == '\n' ? TOK_NEWLINE : TOK_EOF, NULL);
    }

    if (c == '\'')
    {
        char buf[512];
        int i = 0;
        while ((c = io_backend_next()) != EOF && c != '\'')
        {
            if (i < 511)
                buf[i++] = c;
        }
        buf[i] = '\0';
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
    if(c == '"')
    {
	    char buff[512];
	    int i = 0;
	    while((c = io_backend_next()) != EOF && c != '"')
	    {
		    if(c == '\\')
		    {
			    int next = io_backend_next();
			    if(next != EOF)
			    {
				    c = next;
			    }
		    }
		    if(i < 511)
		    {
			    buff[i++] = c;
		    }
	    }
	    buff[i] = '\0';
	    return new_tok(TOK_WORD, my_strdup(buff));
    }
    char buf[512];
    int i = 0;
    while ( c != EOF && !isspace(c) && c != ';' && c != '|' && c != '<' 
            && c != '>' && c != '!')
    {
        if (i < 511)
            buf[i++] = c;
        c = io_backend_peek();
        if (isspace(c) || c == ';' || c == '|' || c == '<' || c == '>' 
                || c == '!')
            break;
        c = io_backend_next();
    }

    buf[i] = '\0';

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
    if(!lex)
    {
        return NULL;
    }
    if(!lex->curr_tok)
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
