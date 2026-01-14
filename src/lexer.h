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
    TOK_DGT,
    TOK_FOR,
    TOK_WHILE,
    TOK_UNTIL,
    TOK_DO,
    TOK_DONE,
    TOK_IN,
    TOK_OR,
    TOK_NOT,
    TOK_AND
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
