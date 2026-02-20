#ifndef EXPANSION_H
#define EXPANSION_H

#include "../parser/parser.h"
#include "../special/special.h"

/*
 * Realise l'expansion d'un mot :
 *   - variables ($VAR, ${VAR})
 *   - variables speciales ($?)
 *   - single quotes (pas d'expansion)
 * Retourne une chaine allouee (a liberer par l'appelant), ou NULL si erreur.
 *
 * parser : etat du parser (acces aux variables shell)
 * spe    : variables speciales ($?)
 * word   : mot a expand
 */
char *expand(struct parser *parser, struct special *spe, const char *word);

/*
 * Supprime une variable shell du parser.
 * Retourne 0 en cas de succes, 1 si la variable est protegee ou inexistante.
 */
int unset_variable(struct parser *parser, const char *name);

/*
 * Supprime une fonction shell du parser (non implemente, retourne 1).
 */
int unset_function(struct parser *parser, const char *name);

#endif /* EXPANSION_H */
