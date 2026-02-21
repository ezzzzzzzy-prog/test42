#ifndef AST_H
#define AST_H

#include <stddef.h>

// Types de nodes de l AST
enum ast_type
{
    AST_COMMAND, // cmd simple
    AST_LIST, // liste avec ;
    AST_AND, // &&
    AST_OR, // ||
    AST_PIPELINE, // pipes |
    AST_REDIRECTION, // redirs < > >>
};

// Types de redirs
enum redir_type
{
    AST_REDIR_OUT, // >
    AST_REDIR_APP, // >>
    AST_REDIR_IN, // <
};

struct ast;

// Cmd simple : ex echo hello
struct ast_cmd
{
    char **words; // argv fini par NULL
};

// Liste de cmds : ex cmd1 ; cmd2
struct ast_list
{
    struct ast **commands; // tableau de nodes
    size_t count; // nb de cmds
    const char *sep; // le separateur (";")
};

// Pipeline : ex cmd1 | cmd2
struct ast_pipeline
{
    struct ast **cmds; // tableau de nodes du pipe
    size_t count; // nb de nodes
};

// Operateur && ou ||
struct ast_and_or
{
    struct ast *left; // gauche
    struct ast *right; // droite
};

// Redir : ex 2> file
struct ast_redirection
{
    enum redir_type type; // quel type de redir
    struct ast *left; // node a rediriger
    char *file; // fichier cible
    int redir_nb; // fd (ex: 2 pour 2>), -1 si defaut
};

// Node principal avec union pour le dispatch
struct ast
{
    enum ast_type type; // type du node
    union
    {
        struct ast_cmd cmd;
        struct ast_list list;
        struct ast_pipeline pipeline;
        struct ast_and_or and_or;
        struct ast_redirection redir;
    } data;
};

// Fonctions de creation
struct ast *create_cmd(char **words);
struct ast *create_list(struct ast **cmds, size_t count);
struct ast *ast_pipeline_create(struct ast **cmds, size_t count);
struct ast *create_and(struct ast *left, struct ast *right);
struct ast *create_or(struct ast *left, struct ast *right);
struct ast *create_redir(enum redir_type type, struct ast *left, char *file,
                         int file_desc);

// Free tout l arbre recursivement
void ast_free(struct ast *ast);

#endif /* AST_H */
