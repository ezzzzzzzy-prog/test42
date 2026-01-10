#include "parser.h"
#include "../ast/ast.h"
#include "../lexer/lexer.h"


struct parser *new_parse(FILE *input)
{
	struct parser *res= malloc(sizeof(struct parser));
	res->lex = new_lex();
	res->curr_tok = peek(res->lex);
	return res;
}

void parser_free(struct parser *parser)
{
	if(!parser)
	{
		return;
	}
	lexer_free(parser->lex);
	free(parser);
}

struct ast *parse_simple_command(struct parser *parser)
{
	if(!parser || !parser->curr_tok)
	{
		return NULL;
	}
	if(parser->curr_tok->type != TOK_WORD)
	{
		return NULL;
	}
	char **words = malloc(10 * sizeof(char *));
	if(!words)
	{
		return NULL;
	}
	int count = 0;
	int capacity = 10;
    while (parser->curr_tok && parser->curr_tok->type == TOK_WORD)
	{
		if(count >= capacity)
		{
			capacity *= 2;
			char **new_words = realloc(words, capacity *sizeof(char *));
			if(!new_words)
			{
				for(int i = 0; i < count; i++)
				{
					free(words[i]);
				}
				free(words);
				return NULL;
			}
			words = new_words;
		}
		words[count] = my_strdup(parser->curr_tok->val);
		if(!words[count])
		{
			for(int i = 0; i < count; i++)
			{
				free(words[i]);
			}
			free(words);
			return NULL;
		}
		count++;
		pop(parser->lex);
		parser->curr_tok = peek(parser->lex);
	}
	if(count >= capacity)
	{
		char **new_words = realloc(words, (count +1 ) *sizeof(char *));
		if(!new_words)
		{
			for(int i = 0; i < count ; i++)
			{
				free(words[i]);
			}
			free(words);
			return NULL;
		}
		words = new_words;
	}
	words[count] = NULL;
	struct ast *res = create_cmd(words);
    if (!res)
    {
        for (int i = 0; i < count; i++)
            free(words[i]);
        free(words);
    }
	return res;
}

struct ast *parse_compound_list(struct parser *parser)
{
	while(parser->curr_tok && parser->curr_tok->type == TOK_NEWLINE)
	{
		pop(parser->lex);
        parser->curr_tok = peek(parser->lex);
	}
	struct ast *res  = parse_and_or(parser);
	if(!res)
	{
		return NULL;
	}
	struct ast **commands = malloc(10 * sizeof(struct ast*));
	if(!commands)
	{
		ast_free(res);
		return NULL;
	}
    int count = 1;
    int capacity = 10;
    commands[0] = res;
	while (parser->curr_tok &&
      (parser->curr_tok->type == TOK_SEMI ||
       parser->curr_tok->type == TOK_NEWLINE))
	{
		pop(parser->lex);
        parser->curr_tok = peek(parser->lex);
        while(parser->curr_tok && parser->curr_tok->type == TOK_NEWLINE)
		{
			pop(parser->lex);
            parser->curr_tok = peek(parser->lex);
		}
		struct ast *next_ast = parse_and_or(parser);
		if(!next_ast)
		{
			break;
		}
		if(count >= capacity)
                {
                        capacity *= 2;
                        struct ast **cm = realloc(commands, capacity * sizeof(*cm));
                        if(!cm)
                        {
                                for(int i = 0; i < count; i++)
                                {
                                        ast_free(commands[i]);
                                }
                                free(commands);
                                return NULL;
                        }
                        commands = cm;
                }
		commands[count++] = next_ast;
	}
	if(parser->curr_tok && parser->curr_tok->type == TOK_SEMI)
	{
        pop(parser->lex);
        parser->curr_tok = peek(parser->lex);
	}
	while(parser->curr_tok && parser->curr_tok->type == TOK_NEWLINE)
	{
        pop(parser->lex);
        parser->curr_tok = peek(parser->lex);
	}
	if(count == 1)
	{
		free(commands);
		return res;
	}
	else
	{
		struct ast *list = create_list(commands,count);
		if(!list)
		{
			for(size_t i = 0; i < count ; i++)
			{
				ast_free(commands[i]);
			}
			free(commands);
			return NULL;
		}
		return list;
	}
}


struct ast *parse_pipeline(struct parser *parser)
{
	if(!parser)
	{
		return NULL;
	}
	return parser_command(parser);
}


struct ast *parse_and_or(struct parser *parser)
{
	if(!parser)
	{
		return NULL;
	}
	return parse_pipeline(parser);
}

struct ast *parser_command(struct parser *parser)
{
	struct ast *res = parse_simple_command(parser);
	if(res)
	{
		return res;
	}
	return NULL;
}

struct ast *parse_list(struct parse *parse)
{
	 while(parser->curr_tok)
        {
                free_tok(parser->curr_tok);
                parser->curr_tok = pop(parser->lex);
        }
        struct ast *res  = parse_and_or(parser);
        if(!res)
        {
                return NULL;
        }
        struct ast **commands = malloc(10 * sizeof(struct ast*));
        if(!commands)
        {
                ast_free(res);
                return NULL;
        }
        while (parser->curr_tok && parser->curr_tok->type == TOK_SEMI)
        {
                free_tok(parser->curr_tok);
                parser->curr_tok = pop(parser->lex);
                while(parser->curr_tok && parser->curr_tok->type == TOK_NEWLINE)
                {
                        free_tok(parser->curr_tok);
                        parser->curr_tok = pop(parser->lex);
                }
                struct ast *next_ast = parse_and_or(parser);
                if(!next_ast)
                {
                        break;
                }
                if(count >= capacity)
                {
                        capacity *= 2;
                        char **cm = realloc(words, capacity *sizeof(char *));
                        if(!cm)
                        {
                                for(int i = 0; i < count; i++)
                                {
                                        ast_free(commands[i]);
                                }
                                free(commands);
                                return NULL;
                        }
                        commands = cm;
                }
                commandes[count++] = next;
        }
        if(parser->curr_tok && parser->curr_tok->type == TOK_SEMI)
        {
                free_tok(parser->curr_tok);
                parser->curr_tok = pop(parser->lex);
        }
        while(parser->curr_tok)
        {
                free_tok(parser->curr_tok);
                parser->curr_tok = pop(parser->lex);
        }
        if(count == 1)
        {
                free(commands);
                return res;
        }
        else
        {
                struct ast *list = create_list(commands,count);
                if(!list)
                {
                        for(size_t i = 0; i < count ; i++)
                        {
                                ast_free(commandsi[i]);
                        }
                        free(commands);
                        return NULL;
                }
                return list;
        }

}
struct ast *parser_input(struct parser *parser)
{
	if(!parser)
	{
		return NULL;
	}
    return parse_compound_list(parser);
}
