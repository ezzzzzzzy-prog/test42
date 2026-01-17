/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/lexer.h"
#include "../../src/io_backend.h"

int main(int argc, char **argv)
{
        if (io_backend_init(argc,argv) <0)
                return 1;
        struct lexer *l = new_lex();
        struct token *t;
        printf("%-20s | %-10s | %s\n", "Type du Token", "Longueur", "Aperçu");

        while((t = pop(l))->type != TOK_EOF)
        {
                printf("%-20d | %-10zu | %.20s...\n", t->type, t->val ? strlen(t->val) : 0, t->val ? t->val : "");
                free_tok(t);
        }
        printf("Fin du fichier atteinte avec succes.\n");
        free_tok(t);
        lexer_free(l);
        io_backend_close();
        return 0;
}
*/
