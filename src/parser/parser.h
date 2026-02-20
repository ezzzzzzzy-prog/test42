#ifndef PARSER_H
#define PARSER_H

#include "../ast/ast.h"
#include "../lexer/lexer.h"
#include "../special/special.h"

/* Variable shell stockee dans une liste chainee */
struct variable
{
    char *nom; /* nom de la variable */
    char *value; /* valeur de la variable */
    struct variable *next; /* maillon suivant dans la liste */
    int exported; /* 1 si exportee dans l'environnement */
};

/* Etat global du parser et du shell */
struct parser
{
    struct lexer *lex; /* lexer associe au parser */
    struct token *curr_tok; /* token courant (lookahead) */
    struct variable *var; /* liste des variables shell */
    struct special *spe; /* variables speciales ($?) */
    int parse_error; /* 1 si une erreur de parsing a eu lieu */
    int exit; /* 1 si le shell doit quitter */
    int ex_code; /* code de sortie demande par exit */
    int last_code; /* code de retour de la derniere commande */
};

/* Cree et initialise un nouveau parser */
struct parser *new_parse(void);

/* Libere le parser et toutes ses ressources */
void parser_free(struct parser *parser);

/* Parse une entree complete et retourne l'AST correspondant */
struct ast *parser_input(struct parser *parser);

/* Ajoute ou met a jour une variable shell */
void add_var(struct parser *parser, const char *name, const char *value);

/* Libere la liste chainee de variables */
void free_variable(struct variable *var);

#endif /* PARSER_H */
