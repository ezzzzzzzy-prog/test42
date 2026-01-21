#ifndef BUILTIN_H
#define BUILTIN_H

#include "parser.h"
int is_builtin(const char *cmd);
int execute_builtin(char **argv, struct parser *parser);

#endif
