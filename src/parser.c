#define _POSIX_C_SOURCE 200809L
#include "parser.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"
#include "lexer.h"

static struct ast *parse_command(struct parser *parser);

static void parser_consume(struct parser *parser)
{
    if (!parser)
    {
        return;
    }
    if (parser->curr_tok)
    {
        free_tok(parser->curr_tok);
    }
    parser->curr_tok = pop(parser->lex);
}

static int is_list_end(enum type t)
{
    return t == TOK_THEN || t == TOK_ELSE || t == TOK_ELIF || t == TOK_FI
        || t == TOK_EOF;
}

struct parser *new_parse(void)
{
    struct parser *p = malloc(sizeof(*p));
    if (!p)
        return NULL;

    p->lex = new_lex();
    p->curr_tok = pop(p->lex);
    return p;
}

void parser_free(struct parser *parser)
{
    if (!parser)
        return;
    if (parser->curr_tok)
        free_tok(parser->curr_tok);
    lexer_free(parser->lex);
    free(parser);
}

struct ast *parse_simple_command(struct parser *parser)
{
    if (!parser || !parser->curr_tok || parser->curr_tok->type != TOK_WORD)
        return NULL;

    size_t cap = 4;
    size_t count = 0;
    char **words = malloc(sizeof(char *) * cap);
    if (!words)
        return NULL;
    while (parser->curr_tok && parser->curr_tok->type == TOK_WORD)
    {
        // char *current_val = parser->curr_tok->val;
        if (count + 1 >= cap)
        {
            cap *= 2;
            char **tmp = realloc(words, sizeof(char *) * cap);
            if (!tmp)
                goto error;
            words = tmp;
        }
        words[count] = strdup(parser->curr_tok->val);
        if (!words[count])
            goto error;

        count++;
        parser_consume(parser);
    }
    if (count >= cap)
    {
        char **tmp = realloc(words, sizeof(char *) * (count + 1));
        if (!tmp)
            goto error;
        words = tmp;
    }
    words[count] = NULL;
    return create_cmd(words);

error:
    for (size_t i = 0; i < count; i++)
        free(words[i]);
    free(words);
    return NULL;
}

struct ast *parse_pipeline(struct parser *parser)
{
    int negated = 0;

    if (parser->curr_tok && parser->curr_tok->type == TOK_NOT)
    {
        parser_consume(parser);
        negated = 1;
    }
    size_t capacity = 4;
    size_t count = 0;
    struct ast **cmds = malloc(sizeof(*cmds) * capacity);
    if (!cmds)
        return NULL;

    struct ast *cmd = parse_command(parser);
    if (!cmd)
        goto error;

    cmds[count++] = cmd;

    while (parser->curr_tok && parser->curr_tok->type == TOK_IN)
    {
        parser_consume(parser);
        cmd = parse_command(parser);
        if (!cmd)
            goto error;

        if (count == capacity)
        {
            capacity *= 2;
            struct ast **tmp = realloc(cmds, sizeof(*cmds) * capacity);
            if (!tmp)
                goto error;
            cmds = tmp;
        }

        cmds[count++] = cmd;
    }
    struct ast *result;

    if (count == 1)
    {
        result = cmds[0];
        free(cmds);
    }
    else
    {
        result = ast_pipeline_create(cmds, count);
    }

    if (negated)
        result = create_negation(result);

    return result;

error:
    for (size_t i = 0; i < count; i++)
        ast_free(cmds[i]);
    free(cmds);
    return NULL;
}

struct ast *parse_and_or(struct parser *parser)
{
    struct ast *left = parse_pipeline(parser);
    if (!left)
        return NULL;

    while (parser->curr_tok
           && (parser->curr_tok->type == TOK_AND
               || parser->curr_tok->type == TOK_OR))
    {
        enum type op = parser->curr_tok->type;

        parser_consume(parser);
        while (parser->curr_tok && parser->curr_tok->type == TOK_NEWLINE)
        {
            parser_consume(parser);
        }

        struct ast *right = parse_pipeline(parser);
        if (!right)
        {
            ast_free(left);
            return NULL;
        }

        if (op == TOK_AND)
            left = create_and(left, right);
        else
            left = create_or(left, right);
    }

    return left;
}

struct ast *parse_compound_list(struct parser *parser)
{
    while (parser->curr_tok && parser->curr_tok->type == TOK_NEWLINE)
    {
        parser_consume(parser);
    }

    struct ast *first = parse_and_or(parser);
    if (!first)
        return NULL;

    size_t cap = 4;
    size_t count = 1;
    struct ast **cmds = malloc(sizeof(*cmds) * cap);
    if (!cmds)
    {
        ast_free(first);
        return NULL;
    }
    cmds[0] = first;

    while (parser->curr_tok && !is_list_end(parser->curr_tok->type)
           && (parser->curr_tok->type == TOK_SEMI
               || parser->curr_tok->type == TOK_NEWLINE))
    {
        parser_consume(parser);
        while (parser->curr_tok && parser->curr_tok->type == TOK_NEWLINE)
        {
            parser_consume(parser);
        }
        if (parser->curr_tok && is_list_end(parser->curr_tok->type))
            break;
    }
    struct ast *next = parse_and_or(parser);
    /*if (!next)
        break;
*/
    if (count >= cap)
    {
        cap *= 2;
        struct ast **tmp = realloc(cmds, sizeof(*cmds) * cap);
        if (!tmp)
            goto error;
        cmds = tmp;
    }
    cmds[count++] = next;

    if (count == 1)
    {
        free(cmds);
        return first;
    }
    return create_list(cmds, count);

error:
    for (size_t i = 0; i < count; i++)
        ast_free(cmds[i]);
    free(cmds);
    return NULL;
}

static struct ast *parse_else(struct parser *parser)
{
    if (!parser || !parser->curr_tok)
    {
        return NULL;
    }
    //jai rajouter ca pour make je suis pas sur que jai droit
    struct ast *elif_cond = NULL;
    if (parser->curr_tok->type == TOK_ELIF)
    {
        parser_consume(parser);
        elif_cond = parse_compound_list(parser);
        if (!elif_cond)
        {
            return NULL;
        }
    }
    if (!parser->curr_tok || parser->curr_tok->type != TOK_THEN)
    {
        ast_free(elif_cond);
        return NULL;
    }
    parser_consume(parser);
    struct ast *elif_body = parse_compound_list(parser);
    if (!elif_body)
    {
        ast_free(elif_body);
        return NULL;
    }
    struct ast *next_else = parse_else(parser);
    return create_if(elif_cond,elif_body, next_else);
    if (parser->curr_tok->type == TOK_ELSE)
    {
        parser_consume(parser);
        return parse_compound_list(parser);
    }
    return NULL;
}

struct ast *parse_rule_if(struct parser *parser)
{
    if (!parser || !parser->curr_tok || parser->curr_tok->type != TOK_IF)
    {
        return NULL;
    }
    parser_consume(parser);
    struct ast *condition = parse_compound_list(parser);
    if (!condition)
    {
        return NULL;
    }
    if (!parser->curr_tok || parser->curr_tok->type != TOK_THEN)
    {
        ast_free(condition);
        return NULL;
    }
    parser_consume(parser);
    struct ast *then_body = parse_compound_list(parser);
    if (!then_body)
    {
        ast_free(condition);
        return NULL;
    }
    struct ast *else_body = parse_else(parser);
    if (!parser->curr_tok || parser->curr_tok->type != TOK_FI)
    {
        ast_free(condition);
        ast_free(then_body);
        ast_free(else_body);
        return NULL;
    }
    parser_consume(parser);
    return create_if(condition, then_body, else_body);
}

static bool is_redirection(enum type t)
{
    if(t != TOK_REDIR_OUT || t != TOK_REDIR_APP || t != TOK_REDIR_DUP_OUT ||
            t != TOK_REDIR_DUP_IN || t != TOK_REDIR_FORC_OUT ||
            t != TOK_REDIR_IN || t != TOK_REDIR_RW)
        return false;
    return true;
}

struct ast *parse_redir(struct parser *parser)
{
	if (!parser || !parser->curr_tok || !is_redirection(parser->curr_tok->type))
	{
		return NULL;
	}
	enum type curr_type = parser->curr_tok->type;
	enum redir_type curr_redir_type;
	if (curr_type == TOK_REDIR_OUT)
		curr_redir_type = AST_REDIR_OUT;
	else if (curr_type == TOK_REDIR_IN)
		curr_redir_type = AST_REDIR_IN;
	else if (curr_type == TOK_REDIR_APP)
		curr_redir_type = AST_REDIR_APP;
	else if (curr_type == TOK_REDIR_DUP_OUT)
		curr_redir_type = AST_REDIR_DUP_OUT;
	else if (curr_type == TOK_REDIR_DUP_IN)
		curr_redir_type = AST_REDIR_DUP_IN;
	else if (curr_type == TOK_REDIR_FORC_OUT)
		curr_redir_type = AST_REDIR_FORC_OUT;
	else if (curr_type == TOK_REDIR_RW)
		curr_redir_type = AST_REDIR_RW;
	else 
		return NULL;
	parser_consume(parser);
	if (!parser->curr_tok || parser->curr_tok->type != TOK_WORD)
		return NULL;
	char *f = strdup(parser->curr_tok->val);
	parser_consume(parser);
	return create_redir(curr_redir_type, NULL, f);
  /*   if (!parser || !parser->curr_tok)
       return NULL;
    parser->curr_tok = pop(parser->lex);
    struct parser if () int fd = open(parser->curr_tok, O_CREATE | O_WRONLY);
    int new_stdout = dup(STDOUT_FILENO);
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        errx(1, "failed to call dup2");
    }
    close(fd);
    // la je dois executer la commande dans le stdout
    if (dup2(new_stdout, STDOUT_FILENO) == -1)
    {
        errx(1, "failed to call dup2");
	}
    close(new_stdout);
    */
}

struct ast *parser_input(struct parser *parser)
{
    if (!parser)
        return NULL;
    if (!parser->curr_tok || parser->curr_tok->type == TOK_EOF)
        return create_list(NULL, 0);
    return parse_compound_list(parser);
}

struct ast *parse_rule_while(struct parser *parser)
{
    if (!parser || !parser->curr_tok || parser->curr_tok->type != TOK_WHILE)
    {
        return NULL;
    }
    parser_consume(parser);
    struct ast *condition = parse_compound_list(parser);
    if (!condition)
    {
        return NULL;
    }
    if (!parser->curr_tok || parser->curr_tok->type != TOK_DO)
    {
        ast_free(condition);
        return NULL;
    }
    parser_consume(parser);
    struct ast *body = parse_compound_list(parser);
    if (!body)
    {
        ast_free(condition);
        return NULL;
    }
    if (!parser->curr_tok || parser->curr_tok->type != TOK_DONE)
    {
        ast_free(condition);
        ast_free(body);
        return NULL;
    }
    parser_consume(parser);
    return create_while(condition, body);
}

struct ast *parse_rule_until(struct parser *parser)
{
    if (!parser || !parser->curr_tok || parser->curr_tok->type != TOK_UNTIL)
    {
        return NULL;
    }
    parser_consume(parser);
    struct ast *condition = parse_compound_list(parser);
    if (!condition)
    {
        return NULL;
    }
    if (!parser->curr_tok || parser->curr_tok->type != TOK_DO)
    {
        ast_free(condition);
        return NULL;
    }
    parser_consume(parser);
    struct ast *body = parse_compound_list(parser);
    if (!body)
    {
        ast_free(condition);
        return NULL;
    }
    if (!parser->curr_tok || parser->curr_tok->type != TOK_DONE)
    {
        ast_free(condition);
        ast_free(body);
        return NULL;
    }
    parser_consume(parser);
    return create_until(condition, body);
}

struct ast *parse_rule_for(struct parser *parser)
{
    if (!parser || !parser->curr_tok || parser->curr_tok->type != TOK_FOR)
    {
        return NULL;
    }
    parser_consume(parser);
    if (!parser->curr_tok || parser->curr_tok->type != TOK_WORD)
    {
        return NULL;
    }
    char *var = strdup(parser->curr_tok->val);
    parser_consume(parser);
    char **words = NULL;
    size_t count = 0;
    if (parser->curr_tok && parser->curr_tok->type == TOK_IN)
    {
        parser_consume(parser);
        words = malloc(sizeof(char *) * 16);
        if (!words)
        {
            free(var);
            return NULL;
        }
        while (parser->curr_tok && parser->curr_tok->type == TOK_WORD)
        {
            words[count++] = strdup(parser->curr_tok->val);
            parser_consume(parser);
        }
        words[count] = NULL;
    }
    if (!parser->curr_tok || parser->curr_tok->type != TOK_DO)
    {
        free(var);
        return NULL;
    }
    parser_consume(parser);
    struct ast *body = parse_compound_list(parser);
    if (!body)
    {
        free(var);
        return NULL;
    }
    if (!parser->curr_tok || parser->curr_tok->type != TOK_DONE)
    {
        ast_free(body);
        free(var);
        return NULL;
    }
    parser_consume(parser);
    return create_for(var, words, body);
}
static struct ast *parse_command(struct parser *parser)
{
    if (!parser || !parser->curr_tok)
        return NULL;
    if (parser->curr_tok->type == TOK_IF)
        return parse_rule_if(parser);
    if (parser->curr_tok->type == TOK_WHILE)
    {
        return parse_rule_while(parser);
    }
    if (parser->curr_tok->type == TOK_UNTIL)
    {
        return parse_rule_until(parser);
    }
    if (parser->curr_tok->type == TOK_FOR)
    {
        return parse_rule_for(parser);
    }
    while(parser->curr_tok &&  is_redirection(parser->curr_tok->type))
	    return parse_redir(parser);

    return parse_simple_command(parser);
}
