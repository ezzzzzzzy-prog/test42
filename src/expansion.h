#ifndef EXPANSION_H
#define EXPANSION_H

#include "parser.h"
#include "special.h"

char *expand(struct parser *parser, struct special *spe,const char *word);

#endif /* EXPANSION_H */
