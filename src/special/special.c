#include "special.h"

#include <stdlib.h>
#include <string.h>

struct special *create_special(void)
{
    struct special *spe = malloc(sizeof(struct special));
    if (!spe)
        return NULL;
    spe->exit_code = 0;
    return spe;
}

void free_special(struct special *spe)
{
    if (!spe)
    {
        return;
    }
    free(spe);
}
