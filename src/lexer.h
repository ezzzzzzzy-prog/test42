#ifndef LEXER_H
#define LEXER_H

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
    TOK_PIPE,
    TOK_GT,
    TOK_LT,
    TOK_DGT
};

struct token
{
    enum type type;
    char *val;
};

struct lexer
{
    struct token *curr_tok;
};

struct lexer *new_lex(void);
void lexer_free(struct lexer *lex);

struct token *peek(struct lexer *lex);
struct token *pop(struct lexer *lex);
void free_tok(struct token *tok);

#endif /* LEXER_H */

/*#ifndef LEXER_H
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
    TOK_PIPE,
    TOK_GT,
    TOK_LT,
    TOK_DGT
};

struct token
{
	enum type type;
	char *val;
};

struct lexer
{
	FILE *input;
	struct token *curr_tok;
	//int curr;
};

struct lexer *new_lex(FILE *input);
struct token *build(void);
void free_tok(struct token *tok);
void lexer_free(struct lexer *lex);
//struct token *list_token(void);
struct token *peek(struct lexer *lex);
struct token *pop(struct lexer *lex);

*/
