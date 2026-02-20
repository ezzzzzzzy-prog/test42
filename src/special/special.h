#ifndef SPECIAL_H
#define SPECIAL_H

/* Variables speciales du shell (ex: $?) */
struct special
{
    int exit_code; /* code de retour de la derniere commande executee ($?) */
};

/* Cree et initialise une structure special */
struct special *create_special(void);

/* Libere une structure special */
void free_special(struct special *spe);

#endif /* SPECIAL_H */
