#ifndef AST_CREATE_H
#define AST_CREATE_H

#include "ast.h"

/* Cree un noeud commande simple avec le tableau de mots */
struct ast *create_cmd(char **words);

/* Cree un noeud liste de commandes */
struct ast *create_list(struct ast **cmds, size_t count);

/* Cree un noeud pipeline */
struct ast *ast_pipeline_create(struct ast **cmds, size_t count);

/* Cree un noeud operateur && */
struct ast *create_and(struct ast *left, struct ast *right);

/* Cree un noeud operateur || */
struct ast *create_or(struct ast *left, struct ast *right);

/* Cree un noeud redirection */
struct ast *create_redir(enum redir_type type, struct ast *left, char *file,
                         int file_desc);

#endif /* AST_CREATE_H */
