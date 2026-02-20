#ifndef LEXER_H
#define LEXER_H

/* Types de tokens produits par le lexer */
enum type
{
    TOK_EOF, /* fin de fichier */
    TOK_WORD, /* mot ou argument */
    TOK_SEMI, /* separateur ';' */
    TOK_NEWLINE, /* saut de ligne */
    TOK_PIPE, /* operateur pipe '|' */
    TOK_OR, /* operateur logique '||' */
    TOK_AND, /* operateur logique '&&' */
    TOK_REDIR_OUT, /* redirection sortie '>' */
    TOK_REDIR_APP, /* redirection ajout '>>' */
    TOK_REDIR_IN, /* redirection entree '<' */
    TOK_REDIR_NB /* numero de fd explicite ex: '2>' */
};

/* Token produit par le lexer */
struct token
{
    enum type type; /* type du token */
    char *val; /* valeur textuelle (NULL si non applicable) */
};

/* Etat du lexer avec lookahead d'un token */
struct lexer
{
    struct token *curr_tok; /* token courant en attente de consommation */
};

/* Cree et initialise un nouveau lexer */
struct lexer *new_lex(void);

/* Libere le lexer et ses ressources */
void lexer_free(struct lexer *lex);

/* Retourne le token courant sans le consommer */
struct token *peek(struct lexer *lex);

/* Retourne et consomme le token courant */
struct token *pop(struct lexer *lex);

/* Libere un token et sa valeur */
void free_tok(struct token *tok);

#endif /* LEXER_H */
