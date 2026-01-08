#include <stdlib.h>
#include "io_backend.h"
#include "lexer.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
	if(io_backend_init(argc,argv) < 0)
	{
		return 1;
	}
/*	int c;
	while ((c = io_backend_next()) != EOF)
	{
		putchar(c);
	}
	io_backend_close();*/
	struct lexer *lex = new_lex(NULL);
	struct token *tok;

	while (1)
	{
		tok = build();

		printf("Token type: %d", tok->type);
		if (tok->val)
			printf(", value: '%s'", tok->val);
		printf("\n");

		if (tok->type == TOK_EOF)
		{
			free_tok(tok);
			break;
		}
		free_tok(tok);
	}

	lexer_free(lex);
	io_backend_close();
    return 0; 
}

