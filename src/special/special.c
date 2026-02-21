#include "special.h"

#include <stdlib.h>
#include <string.h>

// Alloue et reset le code de sortie a 0
struct special *create_special(void)
{
    struct special *spe = malloc(sizeof(struct special));
    if (!spe)
        return NULL;
    spe->exit_code = 0;
    return spe;
}

// Libere la struct spe
void free_special(struct special *spe)
{
    if (!spe)
    {
        return;
    }
    free(spe);
}
