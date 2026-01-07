#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

enum type
{
	TOK_EOF;
	TOK_WORD;
	TOK_IF;
	TOK_THEN;
	TOK_ELIF;
	TOK_ELSE;
	TOK_FI;
	TOK_SEMI;
	TOK_NEWLINE;
	TOK_SIMQUOTE
};

struct tok
{
	enum type type;
	char *val; //pour les mots/words
};

struct lexer
{
	FILE *input;
	int curr;
};

struct lexer *new(FILE *input);
static char *build(struct lexer *lex)
void free_tok(struct token *tok);

#endif
