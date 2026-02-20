#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/ast/ast.h"
#include "../../src/ast/ast_create.h"

int main(void)
{
    /* Test ast_free sur un noeud commande simple */
    char **words = malloc(sizeof(char *) * 2);
    assert(words);
    words[0] = strdup("echo");
    words[1] = NULL;
    struct ast *cmd = create_cmd(words);
    assert(cmd);
    ast_free(cmd);

    /* Test ast_free sur un arbre and */
    char **w1 = malloc(sizeof(char *) * 2);
    w1[0] = strdup("ls");
    w1[1] = NULL;
    char **w2 = malloc(sizeof(char *) * 2);
    w2[0] = strdup("cat");
    w2[1] = NULL;
    struct ast *and_node = create_and(create_cmd(w1), create_cmd(w2));
    assert(and_node);
    ast_free(and_node);

    /* Test ast_free sur NULL */
    ast_free(NULL);

    return 0;
}
