#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"
#include "special.h"

struct variable
{
        char *nom;
        char *value;
        struct variable *next;
};

struct parser
{
    struct lexer *lex;
    struct token *curr_tok;
    struct variable *var;
    struct special *spe;
};
struct parser *new_parse(void);

struct ast *parser_input(struct parser *parser);
struct ast *parse_rule_if(struct parser *parser);
struct ast *parse_rule_for(struct parser *parser);    // AJOUT
struct ast *parse_rule_while(struct parser *parser); // AJOUT
struct ast *parse_rule_until(struct parser *parser); // AJOUT
void parser_free(struct parser *parser);
void add_var(struct parser *parser, char *name, char *value);

#endif /* PARSER_H */
