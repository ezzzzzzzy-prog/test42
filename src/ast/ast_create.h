#ifndef AST_CREATE_H
#define AST_CREATE_H

#include "ast.h"

// Cree node cmd simple via tableau de mots
struct ast *create_cmd(char **words);

// Cree node list (pour les ;)
struct ast *create_list(struct ast **cmds, size_t count);

// Cree node pour un pipeline
struct ast *ast_pipeline_create(struct ast **cmds, size_t count);

// Cree node pour le &&
struct ast *create_and(struct ast *left, struct ast *right);

// Cree node pour le ||
struct ast *create_or(struct ast *left, struct ast *right);

// Cree node redir avec type, file et fd
struct ast *create_redir(enum redir_type type, struct ast *left, char *file,
                         int file_desc);

#endif /* AST_CREATE_H */
