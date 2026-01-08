#include "lexer.h"
#include "io_backend.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


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

struct lexer *new_lex(FILE *input)
{
	struct lexer *lex = malloc(sizeof(struct lexer));
	if (!lex)
	{
		return NULL;
	}
	lex->input = input;
	lex->curr = 0;
	return lex;
}

void lexer_free(struct lexer *lex)
{
	free(lex);
}

static struct token *new_tok(enum type type, char *val)
{
	struct token *tok = malloc(sizeof(struct token));
	if (!tok)
		return NULL;
	tok->type = type;
	tok->val = val;
	return tok;
}
/*
static enum type keyword(const char *str)
{
	if(strcmp(str, "if")==0)
		return TOK_IF;
	if (strcmp(str, "then")==0)
		return TOK_THEN;
	if (strcmp(str, "elif")==0)
		return TOK_ELIF;
	if (strcmp(str, "else")==0)
		return TOK_ELSE;
	if (strcmp(str, "fi")==0)
		return TOK_FI;
	return TOK_WORD;
}*/

struct token *build(void)
{
	int in = io_backend_next();
//	struct token *tok = malloc(sizeof(struct token));
//	tok->value = NULL;
	while(in == ' ' || in == '\t')
	{
		in = io_backend_next();
	}
	if(in == EOF)
	{
		//tok->type = TOK_EOF;
		return new_tok(TOK_EOF, NULL);
	}
	if (in == '\n')
	{
		//tok->type = TOK_NEWLINE;
		return new_tok(TOK_NEWLINE, NULL);
	}
	if (in == ';')
	{
		//tok->type = TOK_SEMI;
		return new_tok(TOK_SEMI, NULL);
	}
	if (in == '#')
	{
		while (in != EOF && in != '\n')
		{
			in = io_backend_next();
		}
		if(in == '\n')
		{
			return new_tok(TOK_NEWLINE, NULL);
		}
		else
		{
			return new_tok(TOK_EOF, NULL);
		}
		//tok->type = (in == '\n') ? TOK_NEWLINE : TOK_EOF;
		//return tok;
	}
	//si aucune des conditions d'avant n'est valide, on a un mot/word
	if (in == '\'')
	{
		char buff[512];
		int i = 0;
		in = io_backend_next();
		while(in != EOF && in != '\'')
		{
			if (i < 511)
			{
				buff[i] = in;
				i++;
			}
			in = io_backend_next();
		}
		buff[i] = '\0';
		return new_tok(TOK_WORD, my_strdup(buff));
	}
	char buff[512];
	int i =0;
	while(in != EOF && !isspace(in) && in !=';' && in!= '#' && in!= '\'')
	{
		if (i < 511)
		{
			buff[i] = in;
			i++;
		}
		in = io_backend_next();
	}
	buff[i] = '\0';
	struct token *tok = malloc(sizeof(struct token));
	if(!tok)
	{
		return NULL;
	}
	tok->val = NULL;
	if(strcmp(buff, "if") == 0)
		tok->type = TOK_IF;
	else if(strcmp(buff, "then") == 0)
		tok->type = TOK_THEN;
	else if(strcmp(buff, "elif") == 0)
		tok->type = TOK_ELIF;
	else if(strcmp(buff, "else") == 0)
		tok->type = TOK_ELSE;
	else if(strcmp(buff, "fi") == 0)
		tok->type = TOK_FI;
	else
	{
		tok->type = TOK_WORD;
		tok->val = my_strdup(buff);
	}
	return tok;
}
