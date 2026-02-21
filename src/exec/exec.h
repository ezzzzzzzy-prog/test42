#ifndef EXEC_H
#define EXEC_H

#include "../ast/ast.h"
#include "../parser/parser.h"

// Pointer global vers le parser (pour l'exec et les builtins)
extern struct parser *g_parser;

// Lance l execution recursive de l AST et renvoie le code de sortie
int exec_ast(struct ast *ast);

// Set le pointer global avant de lancer l exec
void exec_set_parser(struct parser *parser);

#endif /* EXEC_H */
