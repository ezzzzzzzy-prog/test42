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

/*#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#include "../ast/ast.h"
#include "../lexer/lexer.h"



struct parser
{
	struct token *curr_tok;
	struct lexer *lex;
};


struct parser *new_parser(FILE *input);
void parser_free(struct parser *parser);

struct ast *parser_init(void);
struct ast *parse_simple_command(struct parser *parser);
struct ast *parse_and_or(void);
struct ast *parse_pipeline(void);
struct ast *parser_command(void);
struct ast *parse_rule_if(void);
struct ast *parser_input(void);
struct ast *free_parse(void);
*/
