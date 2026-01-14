#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

struct parser
{
    struct lexer *lex;
    struct token *curr_tok;
};
struct parser *new_parse(void);

struct ast *parser_input(struct parser *parser);
struct ast *parse_rule_if(struct parser *parser);
void parser_free(struct parser *parser);

#endif /* PARSER_H */
