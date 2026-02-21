#ifndef SPECIAL_H
#define SPECIAL_H

// Stocke les variables speciales (comme le $?)
struct special
{
    int exit_code; // Valeur de $?
};

// Alloue et init le special
struct special *create_special(void);

// Libere la structure
void free_special(struct special *spe);

#endif /* SPECIAL_H */
