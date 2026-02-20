#ifndef AST_H
#define AST_H

#include <stddef.h>

/* Types de noeuds de l'AST */
enum ast_type
{
    AST_COMMAND, /* commande simple */
    AST_LIST, /* liste de commandes separees par ';' */
    AST_AND, /* operateur && */
    AST_OR, /* operateur || */
    AST_PIPELINE, /* pipeline de commandes separees par '|' */
    AST_REDIRECTION, /* redirection d'entree/sortie */
};

/* Types de redirection */
enum redir_type
{
    AST_REDIR_OUT, /* redirection sortie : > */
    AST_REDIR_APP, /* redirection sortie en ajout : >> */
    AST_REDIR_IN, /* redirection entree : < */
};

/* Forward declaration */
struct ast;

/* Commande simple : ex: echo hello world */
struct ast_cmd
{
    char **words; /* tableau argv NULL-termine, NULL si noeud assignment-only */
};

/* Liste de commandes : ex: cmd1 ; cmd2 ; cmd3 */
struct ast_list
{
    struct ast **commands; /* tableau des commandes */
    size_t count; /* nombre de commandes */
    const char *sep; /* separateur utilise (";") */
};

/* Pipeline : ex: cmd1 | cmd2 | cmd3 */
struct ast_pipeline
{
    struct ast **cmds; /* tableau des commandes du pipeline */
    size_t count; /* nombre de commandes */
};

/* Operateur binaire && ou || : ex: cmd1 && cmd2 */
struct ast_and_or
{
    struct ast *left; /* operande gauche */
    struct ast *right; /* operande droit */
};

/* Redirection : ex: [IONUMBER] ( > | >> | < ) WORD */
struct ast_redirection
{
    enum redir_type type; /* type de redirection */
    struct ast *left; /* commande dont on redirige les flux */
    char *file; /* fichier cible de la redirection */
    int redir_nb; /* fd explicite (ex: 2 pour "2>"), -1 si absent */
};

/* Noeud principal de l'AST avec union discriminee */
struct ast
{
    enum ast_type type; /* type du noeud */
    union
    {
        struct ast_cmd cmd; /* donnees commande simple */
        struct ast_list list; /* donnees liste */
        struct ast_pipeline pipeline; /* donnees pipeline */
        struct ast_and_or and_or; /* donnees and/or */
        struct ast_redirection redir; /* donnees redirection */
    } data;
};

/* Fonctions de creation des noeuds */
struct ast *create_cmd(char **words);
struct ast *create_list(struct ast **cmds, size_t count);
struct ast *ast_pipeline_create(struct ast **cmds, size_t count);
struct ast *create_and(struct ast *left, struct ast *right);
struct ast *create_or(struct ast *left, struct ast *right);
struct ast *create_redir(enum redir_type type, struct ast *left, char *file,
                         int file_desc);

/* Libere recursivement tout l'AST */
void ast_free(struct ast *ast);

#endif /* AST_H */
