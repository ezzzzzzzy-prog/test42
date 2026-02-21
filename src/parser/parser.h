#ifndef PARSER_H
#define PARSER_H

#include "../ast/ast.h"
#include "../lexer/lexer.h"
#include "../special/special.h"

// liste chainee des vars
struct variable
{
    char *nom;
    char *value;
    struct variable *next;
    int exported;
};

// Etat global (shell + parsing)
struct parser
{
    struct lexer *lex;
    struct token *curr_tok;
    struct variable *var;
    struct special *spe;
    int parse_error;
    int exit; // Flag de sortie
    int ex_code; // Code de sortie
    int last_code; // Retour derniere cmd
};

// Allocation et free du parser
struct parser *new_parse(void);
void parser_free(struct parser *parser);

// Recupere l AST complet depuis l entree
struct ast *parser_input(struct parser *parser);

// Utils pour les variables
void add_var(struct parser *parser, const char *name, const char *value);
void free_variable(struct variable *var);

#endif /* PARSER_H */
