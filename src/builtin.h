#ifndef BUILTIN_H
#define BUILTIN_H

#include "parser.h"
int is_builtin(const char *cmd);
int execute_builtin(char **argv, struct parser *parser);

int unset_variable(struct parser *parser, const char *name);
int unset_function(struct parser *parser, const char *name);

#endif
