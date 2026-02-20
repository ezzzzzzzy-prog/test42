#ifndef BUILTIN_H
#define BUILTIN_H

#include "../parser/parser.h"

/*
 * Retourne 1 si cmd est le nom d'un builtin connu, 0 sinon.
 */
int is_builtin(const char *cmd);

/*
 * Execute le builtin correspondant a argv[0].
 * argv   : tableau d'arguments NULL-termine
 * parser : etat du shell (variables, exit, last_code...)
 * Retourne le code de retour du builtin.
 */
int execute_builtin(char **argv, struct parser *parser);

#endif /* BUILTIN_H */
