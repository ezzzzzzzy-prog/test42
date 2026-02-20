#ifndef DEBUG_H
#define DEBUG_H

#include "../ast/ast.h"

/* Affiche recursivement l'AST avec indentation pour le debug */
void ast_dump(struct ast *ast, int depth);

#endif /* DEBUG_H */
