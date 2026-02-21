#ifndef LEXER_H
#define LEXER_H

// Liste des types de tokens
enum type
{
    TOK_EOF, // Fin de l entree
    TOK_WORD, // cmd ou un arg
    TOK_SEMI, // Separateur de commande ';'
    TOK_NEWLINE, // Fin de ligne '\n'
    TOK_PIPE, // Barre '|'
    TOK_OR, // Operateur '||'
    TOK_AND, // Operateur '&&'
    TOK_REDIR_OUT, // Redirection '>'
    TOK_REDIR_APP, // Redirection '>>'
    TOK_REDIR_IN, // Redirection '<'
    TOK_REDIR_NB // Numero de FD pour redir (ex: '2' dans '2>')
};

// Structure d un token : type + sa valeur chaine
struct token
{
    enum type type;
    char *val;
};

// stocke le token
struct lexer
{
    struct token *curr_tok;
};

// Alloue un nv lexer
struct lexer *new_lex(void);

// Nettoie le lexer + le token en attente
void lexer_free(struct lexer *lex);

// Regarde le prochain token sans l enlever
struct token *peek(struct lexer *lex);

// Recupere le prochain token et passe a la suite
struct token *pop(struct lexer *lex);

// Libere un token + sa valeur
void free_tok(struct token *tok);

#endif /* LEXER_H */
