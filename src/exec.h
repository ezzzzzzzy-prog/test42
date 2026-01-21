#ifndef EXEC_H
#define EXEC_H

#include "ast.h"
#include "parser.h"
extern struct parser *g_parser;
int exec_ast(struct ast *ast);
void exec_set_parser(struct parser *parser);

#endif /* EXEC_H */
