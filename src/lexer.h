#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

enum type
{
	TOK_EOF,
	TOK_WORD,
	TOK_IF,
	TOK_THEN,
	TOK_ELIF,
	TOK_ELSE,
	TOK_FI,
	TOK_SEMI,
	TOK_NEWLINE,
	TOK_SIMQUOTE
};

struct token
{
	enum type type;
	char *val; //pour les mots/words
};

struct lexer
{
	FILE *input;
	int curr;
};

struct lexer *new_lex(FILE *input);
struct token *build(void);
void free_tok(struct token *tok);
void lexer_free(struct lexer *lex);

#endif /* LEXER_H */ 
