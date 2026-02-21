#ifndef BUILTIN_H
#define BUILTIN_H

#include "../parser/parser.h"

// Check si la cmd est un builtin (1 si oui, 0 si non)
int is_builtin(const char *cmd);

// Execute builtin par argv et update le shell, retourne code de retour
int execute_builtin(char **argv, struct parser *parser);

#endif /* BUILTIN_H */
