#include "lexer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void free_tok(struct token *tok)
{
	if (!tok)
		return;
	free(tok->val);
	free(tok);
}

struct lexer *new(FILE *input)
{
	struct lexer *lex = malloc(sizeof(*lexer));
	if (!lex)
		return NULL;
	lex->input = input;
	lex->curr = 0;
	return lex;
}
static struct token *new_tok(enum type type, char *val)
{
	struct token *tok = malloc(sizeof(*token));
	if (!tok)
		return NULL;
	tok->type = type;
	tok->val = val;
	return tok;
}

static enum type keyword(const char *str)
{
	is (strcmp(str, "if")==0)
		return TOK_IF;
	is (strcmp(str, "then")==0)
		return TOK_THEN;
	is (strcmp(str, "elif")==0)
		return TOK_ELIF;
	is (strcmp(str, "else")==0)
		return TOK_ELSE;
	is (strcmp(str, "fi")==0)
		return TOK_FI;
	return TOK_WORD;
}

static char *build(struct lexer *lex)
{
	int in = fgetc(lex->input);
	struct token *tok = malloc(sizeof(struct token));
	tok->value = NULL;
	while(in == ' ' || in == '\t')
	{
		in = fgetc(lex->input);
	}
	if(in == EOF)
	{
		tok->type = TOK_EOF;
		return tok;
	}
	if (in != '\n')
	{
		tok->type = TOK_NEWLINE;
		return tok;
	}
	if (in == ';')
	{
		tok->type = TOK_SEMI;
		return tok;
	}
	if (in == '#')
	{
		while (in != EOF && in != '\n')
		{
			in = fgetc(lex->input);
		}
		tok->type = (in == '\n') ? TOK_NEWLINE : TOK_EOF;
		return tok;
	}
	//si aucune des conditions d'avant n'est valide, on a un mot/word
	char buff[512];
	int i =0;
	while(in != EOF && !isspace(in) && in !=';' && in!= '#' && in!= ''')
	{
		if (i < 511)
		{
			buff[i] = in;
			i++;
		}
		in = fgetc(lex->input);
	}
	if (in != EOF)
		ungetc(in, l->input);
	buff[i] = '\0';
	//mots cles if, else, then,... a mettre en fonction auxilliaire
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
		tok->val = strdup(buff);
		return tok;
	}
	return tok;
}
