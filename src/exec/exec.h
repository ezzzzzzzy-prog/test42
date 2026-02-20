#ifndef EXEC_H
#define EXEC_H

#include "../ast/ast.h"
#include "../parser/parser.h"

/* Pointeur global vers le parser courant (accessible depuis exec et builtins)
 */
extern struct parser *g_parser;

/*
 * Execute recursivement un noeud AST.
 * Retourne le code de retour de la commande executee.
 */
int exec_ast(struct ast *ast);

/*
 * Initialise le pointeur global vers le parser.
 * Doit etre appele avant tout appel a exec_ast.
 */
void exec_set_parser(struct parser *parser);

#endif /* EXEC_H */
