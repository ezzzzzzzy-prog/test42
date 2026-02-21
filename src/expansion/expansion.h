#ifndef EXPANSION_H
#define EXPANSION_H

#include "../parser/parser.h"
#include "../special/special.h"

// Realise l expansion du mot (vars et quotes) et renvoie une chaine allouee
char *expand(struct parser *parser, struct special *spe, const char *word);

// Delete une variable du shell (0 si ok, 1 si erreur ou protege)
int unset_variable(struct parser *parser, const char *name);

// Delete une fonction shell (non implemente)
int unset_function(struct parser *parser, const char *name);

#endif /* EXPANSION_H */
