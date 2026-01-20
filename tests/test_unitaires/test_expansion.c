#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "expansion.h"
#include "special.h"
#include "parser.h"

int ok = 0;
int total = 0;

void test(const char *result, const char *expect, const char *msg)
{
    total++;
    if ((!result && !expect) || (result && expect && strcmp(result, expect) == 0))
    {
        ok++;
        printf("OK: %s\n", msg);
    }
    else
    {
        printf("FAIL: %s\n", msg);
        printf("  attendu: %s\n", expect ? expect : "(null)");
        printf("  obtenu: %s\n", result ? result : "(null)");
    }
    if (result)
        free((void *)result);
}

struct special *make_test_special()
{
    struct special *s = malloc(sizeof(struct special));
    s->exit_code = 42;
    s->shell_pid = getpid();
    s->uid = getuid();
    s->argc_count = 3;
    
    s->args = malloc(sizeof(char*) * 3);
    s->args[0] = strdup("alpha");
    s->args[1] = strdup("beta");
    s->args[2] = strdup("gamma");
    
    s->script_name = strdup("42sh");
    s->pwd = strdup("/home/test");
    s->oldpwd = strdup("/tmp");
    s->last_bg_pid = 9999;
    s->shell_opts = strdup("himBH");
    
    return s;
}

void free_test_special(struct special *s)
{
    if (!s)
        return;
    for (int i = 0; i < s->argc_count; i++)
        free(s->args[i]);
    free(s->args);
    free(s->script_name);
    free(s->pwd);
    free(s->oldpwd);
    free(s->shell_opts);
    free(s);
}

int main(void)
{
    struct parser p;
    p.var = NULL;
    
    struct special *spe = make_test_special();
    char pid[32];
    char uid[32];
    
    snprintf(pid, sizeof(pid), "%d", spe->shell_pid);
    snprintf(uid, sizeof(uid), "%d", spe->uid);
    
    printf("Tests expansion\n\n");
    
    printf("Variables speciales\n");
    test(expand(&p, spe, "$?"), "42", "$?");
    test(expand(&p, spe, "$$"), pid, "$$");
    test(expand(&p, spe, "$#"), "3", "$#");
    test(expand(&p, spe, "$0"), "42sh", "$0");
    test(expand(&p, spe, "$!"), "9999", "$!");
    test(expand(&p, spe, "$-"), "himBH", "$-");
    
    printf("\nArguments\n");
    test(expand(&p, spe, "$1"), "alpha", "$1");
    test(expand(&p, spe, "$2"), "beta", "$2");
    test(expand(&p, spe, "$3"), "gamma", "$3");
    test(expand(&p, spe, "$9"), "", "$9");
    test(expand(&p, spe, "$@"), "alpha beta gamma", "$@");
    test(expand(&p, spe, "$*"), "alpha beta gamma", "$*");
    
    printf("\nVariables env\n");
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)))
        test(expand(&p, spe, "$PWD"), cwd, "$PWD");
    else
        printf("SKIP: $PWD (getcwd failed)\n");
    test(expand(&p, spe, "$OLDPWD"), "/tmp", "$OLDPWD");
    test(expand(&p, spe, "$UID"), uid, "$UID");
    
    printf("\nRANDOM\n");
    char *r1 = expand(&p, spe, "$RANDOM");
    char *r2 = expand(&p, spe, "$RANDOM");
    total++;
    if (r1 && r2 && strcmp(r1, r2) != 0)
    {
        ok++;
        printf("OK: $RANDOM different\n");
    }
    else
    {
        printf("FAIL: $RANDOM\n");
    }
    free(r1);
    free(r2);
    
    printf("\nQuotes\n");
    test(expand(&p, spe, "'$1'"), "$1", "simple quotes");
    test(expand(&p, spe, "\"$1\""), "alpha", "double quotes");
    test(expand(&p, spe, "'\"$1\"'"), "\"$1\"", "mix quotes");
    
    printf("\nBackslash\n");
    test(expand(&p, spe, "\\$1"), "$1", "backslash");
    test(expand(&p, spe, "\"\\$1\""), "$1", "backslash dquotes");
    test(expand(&p, spe, "\\\\$1"), "\\alpha", "double backslash");
    
    printf("\nCas limites\n");
    test(expand(&p, spe, "$"), "$", "dollar seul");
    test(expand(&p, spe, "${}"), "", "accolades vides");
    test(expand(&p, spe, "${1}"), "alpha", "${1}");
    test(expand(&p, spe, "$1$2"), "alphabeta", "concat");
    test(expand(&p, spe, "pre$1post"), "prealphapost", "texte autour");
    
    printf("\nVariables inexistantes\n");
    test(expand(&p, spe, "$INEXISTANT"), "", "var inexistante");
    test(expand(&p, spe, "${INEXISTANT}"), "", "var inexistante accolades");
    
    printf("\nResultat: %d/%d\n", ok, total);
    
    free_test_special(spe);
    
    return (ok == total) ? 0 : 1;
}

