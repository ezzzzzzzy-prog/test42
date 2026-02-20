#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/expansion/expansion.h"
#include "../../src/parser/parser.h"
#include "../../src/special/special.h"

int ok = 0;
int total = 0;

static void test(char *result, const char *expect, const char *msg)
{
    total++;
    if (result && expect && strcmp(result, expect) == 0)
    {
        ok++;
        printf("OK: %s\n", msg);
    }
    else
    {
        printf("FAIL: %s (attendu='%s', obtenu='%s')\n", msg,
               expect ? expect : "(null)", result ? result : "(null)");
    }
    free(result);
}

int main(void)
{
    struct parser p;
    p.var = NULL;
    p.spe = NULL;

    struct special *spe = create_special();
    assert(spe);
    spe->exit_code = 42;
    p.spe = spe;

    printf("Tests expansion\n\n");

    test(expand(&p, spe, "$?"), "42", "$?");
    test(expand(&p, spe, "hello"), "hello", "mot simple");
    test(expand(&p, spe, "pre${?}post"), "pre42post", "${?}");

    char *sq = expand(&p, spe, "'$?'");
    total++;
    if (sq && strcmp(sq, "$?") == 0)
    {
        ok++;
        printf("OK: single quotes\n");
    }
    else
    {
        printf("FAIL: single quotes (obtenu='%s')\n", sq ? sq : "(null)");
    }
    free(sq);

    test(expand(&p, spe, "$INEXISTANT"), "", "var inexistante");
    test(expand(&p, spe, "${INEXISTANT}"), "", "var inexistante accolades");

    printf("\nResultat: %d/%d\n", ok, total);

    free_special(spe);
    return (ok == total) ? 0 : 1;
}
